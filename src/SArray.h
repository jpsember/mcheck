/*	SArray: dynamic array, uses templates, 
							with static item locations
*/

template <typename T> 
class SArray {
public:
	// --------------------------------
	// constructor
	SArray(int pageSize = 100);
	// destructor
	~SArray();
	// copy constructor
	SArray(const SArray &s);
	// assignment operator
	SArray& operator=(const SArray &s);
	// initializer
private:
	void construct();
	// --------------------------------
public:
	/*	Get item
			> id							id of item
	*/
	T &itemAt(int id) const;
  T &operator[](int i) const {return itemAt(i);}

	/*	Add an item
			> item						item to add
			< id of item
	*/
	int alloc(const T &item);

	/*	Add an item to a particular slot; allocate if necessary
			> item						item to add
			> id							id to store as (-1 to use arbitrary empty slot)
			< id of item
	*/
	int add(const T &item, int id = -1);

	/*	Free an item
			> id							id of item to free
	*/
	void free(int id);

	/*	Replace an item
			> id							id of item to replace
			> item						item to replace with
	*/
	void set(int id, const T &item);

	/*	Get id of last item in array
			< id of last item, or -1 if none exist
	*/
	int lastItem() const {return lastItem_;}

	/*	Get number of items (actually, 1 + id of last item)
			< number of items 
	*/
	int length() const {return 1 + lastItem();}

	/*	Determine if item exists
			> id							id to search for
			< true if it exists
	*/
	bool exists(int id) const {
		return (id >= 0 && valid_.get(id));
	};

	/*	Clear all items from array
	*/
	void clear();

#if DEBUG
	/*	Get debug description of array
	*/
	const char *s(void printItem(const T& item) = 0) const;
#endif

private:
	/*	Calculate the page and slot within the page for an item
	*/
	void calcPageAndSlot(int id, int &page, int &slot) const {
		page = id / pageSize_;
		slot = id % pageSize_;
	}

	// pointers to item pages
	Array<void *> pagePtrs_;

	// flags indicating whether item is valid
	BitStore valid_;

	// recycle bin for ids to reuse
	Array<int> recycleBin_;

	// id of last item
	int lastItem_;

	// # items in each page
	int pageSize_;
};

template<typename T>
void SArray<T>::construct() {
	lastItem_ = -1;
}

template<typename T> 
SArray<T>::SArray(int pageSize)
{
	construct();
	ASSERT(pageSize > 0);
	pageSize_ = pageSize;
}

template<typename T> 
SArray<T>::~SArray()
{
	clear();
}

template<typename T> 
void SArray<T>::clear() {
	while (!pagePtrs_.isEmpty()) {
		T *pg = (T *)pagePtrs_.pop();
		delete [] pg;
	}
	lastItem_ = -1;
	valid_.clear();
	recycleBin_.clear();
}

template<typename T> 
int SArray<T>::add(const T &item, int id)
{
	if (id < 0) {
		id = alloc(item);
	} else {
			
		int page, slot;
		calcPageAndSlot(id, page, slot);

		while (page >= pagePtrs_.length()) {
			T *newPage = new T[pageSize_];
			pagePtrs_.add((int *)newPage);
		}
		valid_.set(id);
		set(id, item);
	}
	return id;
}

template<typename T> 
int SArray<T>::alloc(const T &item)
{
	// determine id of new item

	int id;
	while (true) {
		if (recycleBin_.isEmpty()) {
			id = lastItem() + 1;
			break;
		} else {
			id = recycleBin_.pop();
			// in case user performed an 'add' to an item in
			// the recycle bin, make sure the recycled item is not valid
			if (!valid_.get(id))
				break;
		}
	}

	int page, slot;
	calcPageAndSlot(id, page, slot);

	if (page >= pagePtrs_.length()) {
		T *newPage = new T[pageSize_];
		pagePtrs_.add((int *)newPage);
	}
	valid_.set(id);
	set(id, item);
	return id;
}

template<typename T>
T &SArray<T>::itemAt(int id) const
{
	ASSERT(valid_.get(id));
	int page, slot;
	calcPageAndSlot(id, page, slot);
	T *pg = (T *)pagePtrs_[page];
	return pg[slot];
}

template<typename T>
void SArray<T>::set(int id, const T &item)
{
	ASSERT(valid_.get(id));
	int page, slot;
	calcPageAndSlot(id, page, slot);
	T *pg = (T *)pagePtrs_[page];
	pg[slot] = item;
	if (id > lastItem_)
		lastItem_ = id;
}

template<typename T>
SArray<T>::SArray(const SArray<T> &s)
{
	construct();
	*this = s;
}

template<typename T>
SArray<T>& SArray<T>::operator=(const SArray<T> &s)
{
	if (&s != this) {
		clear();

		pageSize_ = s.pageSize_;
		valid_ = s.valid_;
		recycleBin_ = s.recycleBin_;
		lastItem_ = s.lastItem_;

		for (int i = 0; i < s.pagePtrs_.length(); i++) {
			T *newPage = new T[pageSize_];
			T *srcPage = (T *)s.pagePtrs_.itemAt(i);
			for (int j = 0; j < pageSize_; j++)
				newPage[j] = srcPage[j];
			pagePtrs_.add(newPage);
		}
	}
	return *this;
}

template<typename T>
void SArray<T>::free(int id)
{
	ASSERT(valid_.get(id));
	valid_.set(id, false);
	recycleBin_.add(id);
	if (lastItem_ == id) {
		lastItem_--;
		while (lastItem_ >= 0
			&& !valid_.get(lastItem_)) lastItem_--;
	}
}

#if DEBUG
template<typename T>
const char *SArray<T>::s(void printItem(const T &item)) const
{
	String &str = Debug::str();

	Utils::pushSink(&str);
	
	Cout << "SArray ";
	Cout << "lastItem=" << lastItem() << " ";
	Cout << "#rb=" << recycleBin_.length() << " ";
	Cout << "\n";

	if (printItem != 0) {
		int printed = 0;
		bool first = true;
		for (int i = 0; i < valid_.length(); i++) {
			if (valid_.get(i)) {
				if (!first)
					Cout << " ";
				else {
					Cout << fmt(i) << ": ";
					first = false;
				}
				printItem(itemAt(i));
				printed++;
				if (printed == 10) {
					printed = 0;
					first = true;
					Cout << "\n";
				}
			}
		}
		if (!first)
			Cout << "\n";
	}
	Utils::popSink();
	return str.chars();
}
#endif

#if 0
static void printTestItem(const int &item) {
	Cout << item;
}

static void Test_SArray() {
	SArray<int> a;

	pr(("Test SArray\n"));

	Array<int> ids;
	SArray<int> b;

	for (int i = 0; ; i++) {
		pr(("%s",a.s(printTestItem) ));
		if (i < 20) {
			ids.add(a.alloc(i),i);
		}

		if (i == 12) {
			pr(("\n"));
			for (int j = 1; j < i; j += 3) {
				pr(("   freeing %d\n",ids[j]));
				a.free(ids[j]);
			}
		}

		if (i == 14)
			b = a;
		if (i >= 20) {
			if (a.lastItem() >= 0) {
				a.free(a.lastItem());
			} else
				break;
		}
	}
	pr(("%s",b.s(printTestItem) ));
}

#endif
