// Exception class

// This will be an example of a class with no dynamically allocated memory,
// thus it needs no destructor, copy constructor, or assignment operator.

// However, to keep track of memory leaks, we still need the calls to
// CONSTRUCT() and DESTROY() which are not provided by the compiler-generated
// methods, so we will still implement the five basic methods.

class Exception {
// The five basic methods
// ---------------------------------------
public:
	// constructor
	Exception() {construct();}
	Exception(const String &msg);
	// destructor
	virtual ~Exception() {
		//Cout << "Destroying exception " << ptrStr(this) << "\n";
		DESTROY();
	}
	// copy constructor
	Exception(const Exception &s) {
		construct();
		*this = s;
	}
	// assignment operator
	Exception &operator=(const Exception &s) {
		if (&s != this) {
			msg = s.msg;
		}
		return *this;
	}
private:
	void construct();
public:
// ---------------------------------------
	String &str() {return msg;}
#if DEBUG
	virtual String small();
#endif

private:

	String msg;
};

template <class T1>
T1& operator << (T1 &sout, Exception &s) {
	if (s.str().length() > 0)
		sout << "*** " << s.str() << "\n"; 
	return sout;
}

class ParseException : public Exception {
public:
	ParseException(const String &msg) : Exception(msg) {}
};

class NumberFormatException : public Exception {
public:	NumberFormatException(const String &msg) : Exception(msg) {}
};

class StringFormatException : public Exception {
public:	StringFormatException(const String &msg) : Exception(msg) {}
};

class AssertException : public Exception {
public:	AssertException(const String &msg) : Exception(msg) {}
};
