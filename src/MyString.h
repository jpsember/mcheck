// sources and sinks:  base class for reading, writing
// data to or from files, strings, stdout, cout, etc.
class Sink {
public:
	virtual Sink& operator << (const char *s) = 0;
	virtual Sink& operator << (const String &s) = 0;
	virtual Sink& operator << (int i) = 0;
	virtual Sink& operator << (short i) = 0;
	virtual Sink& operator << (char c) = 0;
	virtual Sink& operator << (double d) = 0;
	virtual Sink& operator << (float f) = 0;
	virtual ~Sink() {}
};
class Source {
public:
	virtual Source& operator >> (String &s) = 0;
	virtual Source& operator >> (int &i) = 0;
	virtual Source& operator >> (short &i) = 0;
	virtual Source& operator >> (char &c) = 0;
	virtual Source& operator >> (double &d) = 0;
	virtual Source& operator >> (float &f) = 0;
	virtual bool eof() = 0;
	virtual ~Source() {}
};

typedef Array<String> StringArray;

class String : public Sink
{
public:
#if 0
	/*	Constructor
		> s					zero-terminated string
	*/
	String(const char *s = emptyString);
#endif

	/*	Constructor.  Constructs a String from a substring of a
		character array.
		> s					zero-terminated string
		> offset		offset to first character in substring
		> len				length of substring; if < 0, uses all remaining
								characters
	*/
	String(const char *s = emptyString, int offset = 0, int len = -1);
#if 0
	// copy constructor
	String(const String &s) {
		construct();
		*this = s;
	}
	// assignment operator
	String& operator=(const String &s) {
		ASSERT(mutable_);
		if (&s != this) {
			clear();
			append(s);
		#if DEBUG
			mutable_ = s.mutable_;
		#endif
		}
		return *this;
	}
#endif
private:
	void construct() {
		#if DEBUG
			mutable_ = true;
			lock_ = 0;
		#endif
			charArray_.add('\0');
	}

	// ---------------------------------------
public:
	/*	Determine length of string
			< number of characters (doesn't include terminating zero)
	*/
	int length() const {
		// subtract the terminating zero
		return charArray_.length() - 1;
	}

	/*	Read character from String
		> i					offset into string (0...n-1)
		< char
	*/
	char charAt(int i) const {
		ASSERT(i < length());
		return charArray_.itemAt(i);
	}
	void setChar(int i, char c) {
		ASSERT(mutable_ && i < length());
		charArray_.set(i,c);
	}
#if DEBUG
	const char *s() const {return chars(); }
#endif

	const char *chars() const { 
	  return charArray_.array();
	}
	String cpp(bool withQuotes = true) const;
#if DEBUG
	String debInfo() const;
	String indent(int amt = 4, 
		const char *prompt = 0, int truncLen = -1) const;
	void lock(int amt);
#else
	void lock(int amt) {}
#endif

	/*	Determine if string is nonempty
	*/
	bool defined() const {return length() > 0;}

	void append(const char *str, int offset=0, int length = -1);
	void append(const String &sb, int offset=0, int length = -1);
	void append(char c);
	void set(const char *str, int offset=0, int length = -1) {
		clear();
		append(str,offset,length);
	}
	void set(const String &sb) { 
		set(sb.chars(), 0, sb.length() ); 
	}
	String subStr(int offset, int len = -1) const;
	void subStr(int offset, int len, String &dest) const;
	void clear() {
		remove(0);
	}
	void truncate(int maxLength, bool addEllipses = false);
	void pad(int len, char c = ' ');
	void lower();
	void upper();
	void remove(int start, int len = -1) {
		replace(start,len,0,0);
	}
	void insert(int start, const char *s) {replace(start,0,s,-1);}
	void replace(int start, int delLen, const char *s, int insLen = -1);
	void replace(int start, int delLen, const String &s, int insLen = -1) {
		replace(start,delLen,s.chars(),insLen);
	}
	void ensureCapacity(int newCapacity, bool expectFutureGrowth = true);
	bool startsWith(const char *s, bool ignoreCase = false) const;
	bool startsWith(const String &s, bool ignoreCase = false) const {
		return startsWith(s.chars(),ignoreCase);
	}

	void trimWS();

	/*	Pop last character from string
	*/
	char popChar() {
		char c = charAt(length()-1); 
		truncate(length()-1); 
		return c;
	}

	/*	Return a copy of the string that's padded to a certain 
			length, and truncated to that length with ellipses if 
			necessary.  Ensures at least one space at end of string.
	*/
	String fixedWidth(int width = 12) const;

	//	These functions are for manipulating strings that are file paths.
	//	-----------------------------------------------------------------
	//	There are two characters of particular interest with paths:
	//	1) the separator, which occurs between folder names and filenames;
	//			in DOS and Windows, it's a backslash \ and in UNIX a fwd slash /;
	//	2) the extension marker, a period (.) as in "sauces.txt"
	//
	//	It's convenient to work with abstract paths, which will be the
	//	same for both Windows and UNIX system, much like the Java treatment
	//	of abstract paths.  The functions path_toAbstract and path_toSystem
	//	will convert a path string accordingly.

	
	/*	Make sure path ends with separator; add one if not	*/
	void path_addSeparator();
	/*	Convert system-specific path to abstract path				*/
	void path_toAbstract();
	/*	Convert abstract path to system-specific one				*/
	void path_toSystem();
	/*	Remove directories from path, leaving only the name	*/
	void path_nameOnly();
	String path_getName() const {
		String s(*this);
		s.path_nameOnly();
		return s;
	}

	/*	Determine position where name starts (past directory)
			< index of start of name, or end of string
	*/
	int path_nameStart() const;
	/*	Determine position where extension starts, including the dot
			< index of dot, or end of string if none found
	*/
	int path_extStart() const;

	/*	Remove the directory and filename, leaving just the extension;
			don't include '.'; if no extension exists, returns empty string	*/
	void path_extOnly();
	//	Get extension from path
	String path_getExt() const {
		String s(*this);
		s.path_extOnly();
		return s;
	}

	/*	Set extension of path.  If none exists, new is added; else old
			is replaced
	*/
	void path_setExt(const String &ext);
	/*	Remove any existing extension (plus the '.') from a path	*/
	void path_removeExt();

	// sink interface:
	virtual Sink& operator << (const char *s);
	virtual Sink& operator << (const String &s); 
	virtual Sink& operator << (int i);
	virtual Sink& operator << (short i) {return this->operator<<((int)i);}
	virtual Sink& operator << (char c);
	virtual Sink& operator << (double d);
	virtual Sink& operator << (float f);
private:
	void verifyAbstract() const
#if !DEBUG
	{}
#else
		;
#endif

	/*	Find the position of the extension '.' marker
			< position of last '.' to right of all separators, or -1 if none
	*/
	int path_findExtensionPosition() const;

	enum {
		separatorChar = '/',
		extensionChar = '.',
	};

	Array<char> charArray_;
#if DEBUG
	bool mutable_;
	int lock_;
#endif
	// utility functions
public:
	static char emptyString[1];
	static char * allocCharArray(const char * s);
	static char * allocCharArray(const char *s, int offset, int len = -1);
	static int strLength(const char *s);

	/*	Find the index of a particular character
			> s								string to examine
			> c								character to look for
			> searchFromEnd		if true, returns index of last character found,
														else index of first character found
			< index of first (last) character found, or -1 if not found
	*/
	static int indexOf(const char *s, char c, bool searchFromEnd = false);
	int indexOf(char c, bool searchFromEnd = false) {
		return indexOf(chars(), c, searchFromEnd);
	}
	static void pad(char *s, int len, char c = ' ');
	static void stringCopy(char *dest, const char *src, int maxLen = -1);
	static char upper(char c) {return (char)toupper(c);}
	static void upper(char *c);
	static void lower(char *c);
	static char lower(char c) {return (char)tolower(c);}
	static int compare(const char *a, const char *b, bool ignoreCase = false);
	static int compareFunc(const char *a, const char *b) {
		return compare(a,b,true);
	}
	static int compareFunc(const String &a, const String &b) {
		return compare(a.chars(),b.chars(),true);
	}
#if DEBUG
	static String ptrString(const void *ptr);
#endif
	int compare(const String &s, bool ignoreCase = false) const {
		return compare(chars(), s.chars(), ignoreCase);
	}
	bool equals(const String &s, bool ignoreCase = false) const {
		return compare(chars(), s.chars(), ignoreCase) == 0;
	}
	void split(StringArray &array, char splitChar = '\0') const;
	/*	Join strings from an array to a string
			> array						
			> joinStr					if not null, string to insert between strings
			> dest						where to store result
	*/
	static void join(StringArray &array, const char *joinStr,
		String &dest);
};
ostream &operator << (ostream &sout, const String &s);

// Non-class versions of some utility functions:
#define stringLength(a) String::strLength(a)
#define strCopy(dest,src) String::stringCopy(dest,src,-1)
#define strCopyN(dest,src,n) String::stringCopy(dest,src,n)
#define ptrStr(a) String::ptrString(a)

#if DEBUG
String deb(const String &s, int width = 10);
void Test_String();
#endif
