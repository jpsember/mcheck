#ifndef _PTR
#define _PTR

// Enable this for debug printing
#undef p2
#define p2(a) //pr(a)

/*	Ptr<T> : Smart pointers, with reference counting
*/
#include "PtrBase.h"

// A smart pointer, Ptr, is a templated class that subclasses
// PtrBase.
template <typename X> 
class Ptr : public PtrBase {
public:
	// --------------------------
	/*	Constructor
			> ptr									If not null, this points to an object of
														class X that has been allocated using new.
														This constructor will call allocPtr(..) with
														the item.
	*/
	Ptr(X *ptr = 0);
	/*	Destructor.  If reference count drops to zero, object being 
			pointed to is freed (freePtr(..) is called).
	*/
	virtual ~Ptr();
	/*	Copy constructor
			> s										pointer to make a copy of
	*/
	Ptr(const Ptr &s) : PtrBase() {
		construct();
		*this = s;
	}

	/*	Assignment operator
			> s										pointer to assign to
	*/
	virtual Ptr& operator=(const Ptr &s);

private:
	/*	Initializer
	*/
	void construct();
	// --------------------------
public:
	/*	Get object being pointed to
	*/
	X& operator*() const {
		ASSERT(!isNull());
		return *((X *)counter_->ptr_);
	}
	/*	Get standard c++ pointer to object
	*/
	X* operator->() const {
		ASSERT(!isNull());
		return (X *)(counter_->ptr_);
	}

#if DEBUG
	String debInfo() const;
#endif

	/*	Get another copy of this pointer; increments reference count.
			Used to construct an appropriate subclass of PtrBase().

			< pointer to copy of this pointer; must be Delete'd
				by user when done with it
	*/
	virtual PtrBase *makeCopy_() const;
protected:
	/*	Free up our reference to the data.  If count reaches zero, 
			data itself is freed.
	*/
	void release();
	/*	Acquire a reference to a piece of data.
			> c								RefCount object (includes the pointer to the
												data as well as the reference count)
	*/
	void acquire(RefCount *c);
};

/*	Convert a pointer to PtrBase to a Ptr<X>.
*/
template <typename X>
void downcast(const PtrBase *ptr, Ptr<X> &dest) {
	if (ptr == 0)
		dest = Ptr<X>(0);
	else
		dest = *(Ptr<X> *)ptr;
}


/*	Constructor
*/
template <typename X>
Ptr<X>::Ptr(X *ptr) : PtrBase()
{
	construct();

	// If this isn't a null pointer, allocate it
	if (ptr != 0) {
		RefCount *c;
		NewI(c,ptr);
		DESC(c,"Ptr.RefCount");
		acquire(c);
	}
}

/*	Initializer
*/
template <typename X>
inline void Ptr<X>::construct() {
  CONSTRUCT();
	DESC(this,"Ptr");
}

/*	Destructor
*/
template <typename X>
Ptr<X>::~Ptr() {
	DESTROY();
	release();
}

/*	Assignment operator
*/
template <typename X>
Ptr<X> &Ptr<X>::operator=(const Ptr<X> &s) {
	p2(("Assigning Ptr %p = %p\n",this,&s));

	if (this != &s) {
		// release old pointer, decrement count
		release();
		// acquire new, increment count
    acquire(s.counter_);
  }
  return *this;
}

/*	Acquire pointer
		> c									Counter structure to acquire
*/
template <typename X>
void Ptr<X>::acquire(RefCount *c) {
	p2(("Acquire counter %p (old %p)\n",c,counter_));
	// increment the count
	counter_ = c;
	if (c) {
		c->count_++;
		p2((" count is now %d\n",c->count_));

		// If we're the first one to acquire the pointer,
		// perform the NEW() tracking call.  This is
		// only necessary if memory tracking is being performed.
#if DEBUG
		if (c->count_ == 1) {
			X *ptr = (X *)c->ptr_;
			Debug::myNew_(__FILE__,__LINE__,ptr);
			DESC(ptr,"Ptr.RefCount.ptr");
		}
#endif
	}
}

/*	Release pointer
*/
template <typename X>
void Ptr<X>::release()
{ 
	p2(("Release counter %p\n",counter_));
	// decrement the count, delete if it is 0
  if (counter_) {
		counter_->count_--;
		p2((" count is now %d\n",counter_->count_));

		// if we're the last one to release the pointer,
		// delete the pointed-to item
		if (counter_->count_ == 0) {
			// if pointer wasn't null, free it up
			if (counter_->ptr_) {
				X *ptr = (X *)counter_->ptr_;
				Delete(ptr);
			}
			Delete(counter_);
    }

		// otherwise, leave it alone but clear our pointer to it
    counter_ = 0;
	}
}

/*	Allocate a copy of a pointer.

		< pointer to Ptr<X>; must be deleted (and DELETEd) 
			when user is done with it
*/
template <typename X>
Ptr<X> *makeCopy(const Ptr<X> &s) {
	PtrBase *c = s.makeCopy_();
	return (Ptr<X> *)c;
}

template <typename X>
PtrBase *Ptr<X>::makeCopy_() const {
	Ptr *copy = new Ptr(*this);
#if DEBUG
	Debug::myNew_(__FILE__,__LINE__,copy);
#endif
	return copy;
}

#if DEBUG
void Test_Ptr();

template <typename X>
String& operator << (String &sout, const Ptr<X> &p) {
	sout << "Ptr";
	if (p.isNull()) 
//	if (!p.counter_)
	{
		sout << " <null>";
	} else {
		sout << " C=" << fmt(p.getCount()); //.counter_->count_);
//		ASSERT(p.counter_->ptr_ != 0);
//		sout << " D=" << ptrStr(p.counter_->ptr_);
	}
	return sout;
}

template <typename X>
String Ptr<X>::debInfo() const
{
	String s;
	if (!isNull()) {
		s << *this;
	} else {
		s << "(" << PtrBase::debInfo() << ") ";
	}
	return s;
}
#endif	// DEBUG

//	Define some standard smart pointer types
typedef Ptr<String> StringPtr;

#undef p2

#endif // _PTR
