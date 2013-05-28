#ifndef _PTRBASE
#define _PTRBASE

// reference counter for smart pointers
class RefCount {
public:
	// --------------------------
	// constructor
	/*	ptr								> ptr to data
	*/
	RefCount(void *ptr) {
		ASSERT(ptr != 0);
		construct();
		ptr_ = ptr;
	}

	// destructor
	virtual ~RefCount() {	DESTROY(); }
private: // no copying these objects
	// copy constructor
	RefCount(const RefCount &s) {}
	// assignment operator
	RefCount &operator=(const RefCount &s)  {return *this;}
	// initializer
	void construct() {
		CONSTRUCT();
		ptr_ = 0;
		count_ = 0;
	}
	// --------------------------
public:
#if DEBUG
	virtual String debInfo() const;
#endif
	// pointer to data, 0 if null pointer
  void *ptr_;
	// number of references to this pointer
  int count_;
};


#if DEBUG
template <class T1>
T1& operator << (T1 &sout, const RefCount &s) {
	sout << "PtrBase ";
	sout << "(" << s.debInfo() << ")";
	return sout;
}
//String& operator << (String &sout, const RefCount &s);
#endif

// Common base class for Ptr<X> class
class PtrBase {
	// -----------------------------------------
public:
	// constructor
	PtrBase();
	// destructor
	virtual ~PtrBase();
	// copy constructor
	PtrBase(const PtrBase &s) {
		construct();
		*this = s;
	}
	// assignment operator
	virtual PtrBase& operator=(const PtrBase &s);
	// initializer
private:
	void construct();
	// -----------------------------------------
public:
	bool isNull() const;
	int getCount() const {
		ASSERT(counter_ != 0); return counter_->count_;
	}
#if DEBUG
	/*	Return descriptive string
			< string
	*/
	virtual String debInfo() const;
//	friend String& operator << (String &sout, const PtrBase &s);
#endif

	/*	Get another copy of this pointer; increments reference count.
			Used to construct an appropriate subclass of PtrBase().  Should
			be overridden by the subclass.

			< pointer to copy of this pointer; must be delete'd (and DELETE'd) by user
			  when done with it
	*/
	virtual PtrBase *makeCopy_() const {
		// this method should not be called; must be overridden!
		ASSERT(false);
		return 0;
	}
protected:
	RefCount *counter_;
};

#if DEBUG
/*	Append object to string
		> sout							String to print to
		> p									object to print
		< resulting string
*/
//String& operator << (String &sout, const PtrBase &s);
#endif

#endif // _PTRBASE

