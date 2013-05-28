/*	Array: dynamic array, uses templates
*/

#if DEBUG && 0
extern bool DEBFLAG;
#define pd(a) {if (DEBFLAG) pr(a); }
#else
#define pd(a)
#endif

template <typename T>
class Array {
public:
	// --------------------------------
	// constructor
	Array(int initialSize = 0);
	// destructor
	virtual ~Array();
	// copy constructor
	Array(const Array &s);
	// assignment operator
	Array& operator=(const Array &s);
	// initializer
private:
	void construct();
	// --------------------------------
public:
	T &operator[](int i) const {return itemAt(i);}
	T &itemAt(int i) const {
		ASSERT(i >= 0 && i < used_);
		return buffer_[i];
	}
	void add(const T &item);
	T &last() const {return itemAt(length()-1);}

	/*	Add an item to a particular slot; increase buffer if necessary
	*/
	void add(const T &item, int loc);

	void set(int i, const T &item) {
		ASSERT(i >= 0 && i < used_);
		buffer_[i] = item;
	}
	T &pop();
	bool isEmpty() const {
		return length() == 0;
	}
	int length() const {return used_;}
	void clear() {remove();}
	void remove(int offset = 0, int count = -1) {
		replace(offset,count,0,0);
	}
	void insert(int offset, const T &i) {
		replace(offset,0,&i,1);
	}
	void insert(int offset, const T *a, int insCount) {
		replace(offset,0,a,insCount);
	}
	void truncate(int len) {ASSERT(len >= 0); if (used_ > len) used_ = len; }
	void replace(int offset, int delCount, const T *src, int insCount);
	void ensureCapacity(int newCapacity, bool expectFutureGrowth = true) {
		ASSERT(newCapacity < 1000000);
		if (newCapacity > capacity_)
			growCapacity(newCapacity, expectFutureGrowth);
	}
	void resize(int newCapacity = 0);
	void sort(int (compareFunc)(const T &a, const T &b));
	const T *array() const;
	T *allocBuffer(int len) {
		ASSERT(len >= 0); ensureCapacity(len); used_ = len;
		return const_cast<T*>(buffer_);
	}
#if DEBUG && 0
	const char *s(bool small = false) const;
	String debInfo(bool small = false) const;
#endif
#if DEBUG
	void setTrace(bool f) {
		trace_ = f;
	}
#endif
	void lock() {
#if DEBUG
		lockValue_++;
		ASSERT(lockValue_ < 20);
#endif
	}
	void unlock() {
#if DEBUG
		lockValue_--;
		ASSERT(lockValue_ >= 0);
#endif
	}
	void lock(int growBy) {
		ensureCapacity(length() + growBy,true);
		lock();
	}

	/*	Add an item to a slot from the recycle bin, or to the end
			of the array if recycle bin is empty
			> item						item to add
			> recycleBin			recycle bin containing indices of free slots
			< index of new item
	*/
	int alloc(const T &item, Array<int> &recycleBin);

	/*	Free an item, adding its slot to the recycle bin
			> item						item to free
			> recycleBin			recycle bin containing indices of free slots
	*/
	void free(int item, Array<int> &recycleBin) {
		recycleBin.add(item);
	}

  int used_;
  
protected:
	int capacity_;
	T *buffer_;
	static const int INITIAL_SIZE = 20;
private:
	void growCapacity(int newCapacity, bool expectFutureGrowth = true);

#if DEBUG
	int lockValue_;
	bool trace_;
#endif
};

typedef Array<int> RecycleBin;

template<typename T>
int Array<T>::alloc(const T &item, Array<int> &recycleBin)
{
	int newIndex;
	if (recycleBin.isEmpty()) {
		newIndex = length();
		add(item);
	} else {
		newIndex = recycleBin.pop();
		set(newIndex, item);
	}
	return newIndex;
}

template<typename T>
Array<T>::Array(int initialSize)
{
	construct();
	ensureCapacity(initialSize, false);
}

template<typename T>
void Array<T>::add(const T &item, int loc) {
	if (loc >= length()) {
		ensureCapacity(loc+1);
		used_ = loc+1;
	}
	set(loc, item);
}

template<typename T>
void Array<T>::construct() {

	CONSTRUCT();
	DESC(this,"Array");

	capacity_ = 0;
	used_ = 0;
	buffer_ = 0;
#if DEBUG
	lockValue_ = 0;
	trace_ = false;
#endif
}

template<typename T>
Array<T>::~Array()
{
	if (buffer_ != 0) {
		DeleteArray(buffer_);
	}
	DESTROY();
}

template<typename T>
Array<T>::Array(const Array<T> &s)
{
	construct();
	*this = s;
}


template<typename T>
Array<T>& Array<T>::operator=(const Array<T> &s)
{
	if (&s != this) {
		clear();
		ensureCapacity(s.length());
		for (int i = 0; i < s.length(); i++)
			add(s.itemAt(i));
	}
	return *this;
}

template<typename T>
void Array<T>::add(const T &item)
{
	ensureCapacity(used_+1);
	used_++;
	set(used_-1, item);
}

template<typename T>
T &Array<T>::pop()
{
	ASSERT(used_ > 0);
	T &n = itemAt(used_-1);
	used_--;
	return n;
}

/*
template<typename T>
int Array<T>::length() const {
	return used_;
}
*/
template<typename T>
void Array<T>::replace(int offset, int delCount, const T *src, int insCount)
{
#undef pt
#define pt(a) //pr(a)

	if (delCount < 0) {
		delCount = length() - offset;
	}
	ASSERT(offset >= 0 && offset + delCount <= length() && insCount >= 0);

	pt(("replace [%d...%d] with %d new items\n",offset,offset+delCount-1,insCount));

	if (delCount > insCount) {
		// remove (delCount - insCount) items from (offset + insCount).

		int shift = delCount - insCount;
		for (int i = offset + delCount; i < used_; i++) {
			//                              ^ was <=!!!
			pt((" shiftBwd %d -> %d\n",i,i-shift));
			buffer_[i - shift] = buffer_[i];
		}

		used_ -= shift;
		delCount = insCount;
	}

	if (delCount < insCount) {
		// shift (size - offset) items forward from (offset + delCount)
		ensureCapacity(used_ + insCount - delCount);
		int shift = insCount - delCount;
		for (int i = used_ - 1; i >= offset + delCount; i--) {
			pt((" shiftFwd %d -> %d\n",i,i+shift));
			buffer_[i + shift] = buffer_[i];
		}
		used_ += shift;
		delCount = insCount;
	}

	for (int i = 0; i < insCount; i++) {
		pt((" copying %d -> %d\n",i,i+offset));
		buffer_[i + offset] = src[i];
	}
}


template<typename T>
const T *Array<T>::array() const
{
	return buffer_;
}

template<typename T>
void Array<T>::growCapacity(int newCapacity, bool expectFutureGrowth)
{
	{
		if (expectFutureGrowth) {
			newCapacity = maxVal(newCapacity * 2, INITIAL_SIZE);
		}
		resize(newCapacity);
	}
}

template<typename T>
void Array<T>::resize(int newCapacity)
{
	pd(("resize newCap=%d, used_=%d\n",newCapacity,used_));
		newCapacity = maxVal(newCapacity, used_);
		if (newCapacity != capacity_) {
#if DEBUG
			if (trace_) {
				pr(("resizing array %p from %d to %d\n",this,capacity_,newCapacity));
			}
#endif
			T *nBuff;
			NewArray(nBuff,newCapacity);
			ASSERT(newCapacity >= used_);

			for (int i = 0; i < used_; i++) {
				nBuff[i] = buffer_[i];
			}
			ASSERT2(lockValue_ == 0,"Attempt to resize locked array");

			if (buffer_ != 0) {
				pd((" deleting old %p\n",buffer_));
				DeleteArray(buffer_);
			}
			buffer_ = nBuff;
			capacity_ = newCapacity;
		}
}

template<typename T>
void Array<T>::sort(int (compareFunc)(const T &a, const T &b))
{
	for (int i = 0; i < length(); i++) {
		for (int j = i+1; j < length(); j++) {
			if (compareFunc(itemAt(i),itemAt(j)) > 0) {
				T temp = itemAt(i);
				set(i,itemAt(j));
				set(j,temp);
			}
		}
	}
}

#if DEBUG && 0
template<typename T>
const char *Array<T>::s(bool small) const
{
	String w = debInfo(small);
	String &s = Debug::str();
	s.set(w);
	return s.chars();
}

/*	Construct descriptive string.
		> work							String to store within
*/
template<typename T>
String Array<T>::debInfo(bool small) const
{
	String s;

	if (!small) {
		s << "Array ";

		s << " size=" << length() << " (capacity " << capacity_ << ")";
		s << "\n";
	}
	for (int i = 0; i < length(); i++) {
		if (small) {
			if (i > 0)
				s << " ";
			s << itemAt(i);
		}	else
			s << "#" << i <<": " << itemAt(i) << "\n";
	}

	return s;
}
#endif

typedef Array<char> CharArray;
#if DEBUG
void Test_Array();
#endif

template <typename X>
class Stack : public Array<X> {
public:
	// --------------------------------
	// constructor
	Stack() : Array<X>() {
		construct();
	}
	// destructor
	virtual ~Stack();
	// copy constructor
	Stack(const Stack &s) : Array<X>(s) {construct();}
	// assignment operator
	Stack& operator=(const Stack &s) {
		if (this != &s) {
			Array<X>::operator= (s);
		}
		return *this;
	}
	// initializer
private:
	void construct();
	// --------------------------------
public:
	const char *s(bool small = false) const {
		return Array<X>::s(small);
	}

	X &operator[](int i) const {return itemAt(i);}
	X &peek(int offset = 0) const {return itemAt(length() - 1 - offset); }
	void push(const X &s) {
		Array<X>::add(s);
	}
	X pop() {
		ASSERT2(!isEmpty(),"Pop of empty stack");
		return Array<X>::pop();
	}
	void pop(int amt) {
          ASSERT(amt >= 0 && amt <= length());
     Array<X>::used_ -= amt;
        }

	void clear() {
		Array<X>::clear();
	}
	bool isEmpty() const {
		return Array<X>::isEmpty();
	}

	/*	Peek at top of stack
	const X &top() const {
		return itemAt(length()-1);
	}
	*/
	int length() const {
		return Array<X>::length();
	}

	X &itemAt(int i) const {
		return Array<X>::itemAt(i);
	}
private:
};

template <typename X>
Stack<X>::~Stack() {
	DESTROY();
}

template <typename X>
void Stack<X>::construct() {
	CONSTRUCT();
}

