#ifndef _FILES
#define _FILES

// Files.h : stream wrappers and associated classes
#include <fstream>


// from <string.h>:
void *  memmove ( void * dest, const void * src, size_t num );

// support preprocessing, whitespace removal?
#define _WITHPP 0

/*	IOException class.  These objects are thrown by stream
		classes when errors occur (file not found, read/write error).
*/
class IOException : public Exception {
public:
	// constructor
	IOException(const String &msg) : Exception(msg) {}
};

/*	FileObject class, the base class for stream wrapper classes:
			TextReader, TextWriter, BinaryReader, BinaryWriter
*/
class FileObject {
public:
	// constructor
	FileObject();
	// destructor
	virtual ~FileObject() {
		DESTROY();
	}
	// We can't copy or assign objects of this class
private:
	// copy constructor
	FileObject(const FileObject &s) {}
	// assignment operator
//	FileObject& operator=(const FileObject &s) {return (FileObject &)0;}

public:
	String &path() { return filePath;	}
	bool error() const { return (state == S_ERROR);	}
	String &errorStr() { return errStr;}
	fstream &stream() { return f; }
	virtual bool eof();
	virtual void close() = 0;
#if DEBUG
	virtual String debInfo() = 0;
#endif
	bool isOpen() {return f.is_open(); }
protected:
	enum {
		S_START,
		S_OPEN,
		S_EOF,
		S_CLOSED,
		S_ERROR = 999
	};
	void setError(const char *msg);
	// the stream
	fstream f;
	// state of object (S_xxx)
	int state;
	// path of file
	String filePath;
	// string describing error
	String errStr;
public:
	static bool exists(const char *path);
	static bool exists(const String &path) {return exists(path.chars());}

	static void writeString(const String &str, const String &path);
};

class TextReader : public FileObject, public Source {
public:
	TextReader(); // { construct(); }
	TextReader(const String &path);
	virtual ~TextReader();
	fstream & open(const String& path) {return open(path.chars()); }
	fstream & open(const char *path);
	void close();
	static void readToString(const String& path, String &str);
#if DEBUG
	static void dumpFile(const String& path);
	static void printFile(const String& path);
	String debInfo();
#endif
	int lineNumber() {
		return lineNumber_;
	}
// Source interface:

	/*	Read string up to \n or end of file; don't include the \n
	*/
	virtual Source& operator >> (String &s);
	virtual Source& operator >> (int &i);
	virtual Source& operator >> (short &s);
	virtual Source& operator >> (char &c);
	virtual Source& operator >> (double &d);
	virtual Source& operator >> (float &f);
	virtual bool eof() {return f.eof();}
protected:
	virtual void construct();
	int lineNumber_;
};

#if DEBUG
void Test_TextReader();
void Test_TextWriter();
void Test_BinaryReader();
void Test_StringReader();
#endif

class TextWriter : public FileObject, public Sink {
public:
	TextWriter() { CONSTRUCT(); }
	TextWriter(const String &path);
	~TextWriter();
	// sink interface:
	virtual Sink& operator << (const char *s);
	virtual Sink& operator << (const String &s) {
		return this->operator << (s.chars());}
	virtual Sink& operator << (int i);
	virtual Sink& operator << (short i);
	virtual Sink& operator << (char c);
	virtual Sink& operator << (double d);
	virtual Sink& operator << (float f) {
		return this->operator << ((double)f);}
#if 0	
	void setStream(fstream &fnew) {
		f = fnew;
	}
#endif
	fstream &open(const String& path) {return open(path.chars()); }
	fstream &open(const char *str);
	void close();
#if DEBUG
	String debInfo();
#endif
};

class BinaryReader : public FileObject, public Source {
// -------------------------------------------------------------
public:
	BinaryReader() { construct(); }
	BinaryReader(const String &path);
	~BinaryReader();
private:
	// no copying allowed.
	BinaryReader(const BinaryReader &s) {}
	BinaryReader &operator=(const BinaryReader &s) { return *this; }
	void construct();
// -------------------------------------------------------------
public:
// Source interface:
	virtual Source& operator >> (String &s);
	virtual Source& operator >> (int &i);
	virtual Source& operator >> (short &i);
	virtual Source& operator >> (char &c);
	virtual Source& operator >> (double &d);
	virtual Source& operator >> (float &f);
	virtual bool eof() {return f.eof();}
	fstream &open(const String& path) {return open(path.chars()); }
	fstream &open(const char *path);
	void close();
	static void readToCharArray(const String& path, CharArray &a);
	void read(char *dest, int length);

	// determine current offset into file
	int offset();

	// seek to a particular offset in the file
	void seek(int offset);
#if DEBUG
	static void dumpFile(const String& path);
	String debInfo();
#endif
	int length() const {return length_;}

private:
	long length_;
};

class BinaryWriter : public FileObject, public Sink {
public:
	BinaryWriter() { CONSTRUCT(); }
	BinaryWriter(const String &path);
	~BinaryWriter();
	fstream &open(const String& path) {return open(path.chars()); }
	fstream &open(const char *path);
	void close();
	static void writeCharArray(const String& path, CharArray &a, 
																		int offset = 0, int length = -1);
	void write(const char *data, int length);

	// sink interface:
	virtual Sink& operator << (const char *s);
	virtual Sink& operator << (const String &s) {
		return this->operator << (s.chars());}
	virtual Sink& operator << (int i);
	virtual Sink& operator << (char c);
	virtual Sink& operator << (short i);
	virtual Sink& operator << (double d);
	virtual Sink& operator << (float f);
#if DEBUG
	String debInfo();
#endif
};

enum {
	T_TOKEN_TYPES_START = 250,

	T_EOF = T_TOKEN_TYPES_START,
	T_UNKNOWN,
#if _WITHPP
	T_CR,
#endif

	T_TOKEN_TYPES_END,
};

class DFA;
class StringReader : public Source {
public:
	StringReader() {construct();}
	StringReader(const String &s);
	virtual ~StringReader() {DESTROY();}

	// Source interface:
	/*	Read string up to \n or end of file; don't include the \n
	*/
	virtual Source& operator >> (String &s);
	virtual Source& operator >> (int &i);
	virtual Source& operator >> (short &s);
	virtual Source& operator >> (char &c);
	virtual Source& operator >> (double &d);
	virtual Source& operator >> (float &f);
	virtual bool eof() {return !reading_ || cursor_ == str_.length();}

#if DEBUG
	String debInfo();
#endif

	/*	Read past any whitespace
			< true if end of file afterwards
	*/
	virtual bool readWS();

	/*	Read a whitespace-delimited phrase from string
			> str							string to read to
	*/
	virtual void readWord(String &str);

	/*	Start tokenizing a new string; move from
			state=initial to state=reading

			> str							pointer to string to tokenize
			> startPos				position within string to start at
	*/
	void begin(const String &str, int startPos = 0);

	/*	Determine if tokenizer is in the reading state	*/
	bool reading() const {return reading_;}

	/*	Put tokenizer back into its 'pre-begun' state	*/
	void reset() {	reading_ = 0;	}

	/*	Examine next character without consuming it
			> offset					# characters ahead to examine (0 for next)
			< char, or 0 if no such character exists
	*/
	char peekChar(int offset = 0) const;

	/*	Read next character
			> mustExist				if true, and none remain, throws exception
			< char, or 0 if none remain
	*/
	char readChar(bool mustExist = false);

	/*	Read a sequence of characters
			> count						# characters to read
	*/
	void readChars(int n, bool mustExist = false) { 
		while (n-- > 0) readChar(mustExist); }

	/*	Read a character if it matches expected value
			> expected				character expected; if found, reads it
			> mustFind				if true, throws exception if not found
			< true if found
	*/
	bool readExpChar(char expected, bool mustFind = false);

	/*	Adjust cursor fwd or bwd
			> amt							number of chars to advance (+), or backup (-)
	*/
	void move(int amt = 1);

	/*	Read token from string.
			> dfa							If not 0, DFA for recognizing token
			< token id, or -1 if unknown
	*/
	int readToken(DFA *dfa = 0);

	/*	Read an expected token from the string; throw an exception
			if it's not there.
			> type						type of token expected
			> dfa							If not 0, DFA for recognizing token
	*/
	void readToken(int expected, DFA *dfa = 0);

	/*	Read next character as an unknown token of length 1.
	*/
	int readUnknownChar();

	/*	Determine position and length of last token or word read
			from string.  Error if no token or word was read!
			> pos							Position returned here
			> len							Length returned here
	*/
	void getLastItem(int &pos, int &len) const;

	/*	Get the last token or word read
			> dest						String to store to
	*/
	void getLastItem(String &dest) const;

	/*	Get the current position of the cursor
	*/
	int cursor() const {return cursor_; }

	//	Return a (read-only) reference to the string being read
	const String &str() const {
		ASSERT(reading());
		return str_;
	}

	/*	Return an array of words (Strings) extracted from a string.
			> s								string to extract from
			> sa							where to store extracted tokens

	*/
	static void extractWords(const String &s, StringArray &sa);

protected:
	// position of last word or identifier read, 0 if none
	int prevPos_;
	// length of last word or identifier read, 0 if none
	int prevLen_;
	// Update the prevPos_ field to the current cursor position
	void updatePrevPos() { prevPos_ = cursor_; }
	void updatePrevLen(int len) { prevLen_ = len; }
	virtual void construct();

	// string being read
  String str_;
	bool reading_;
	int cursor_;
};

class StringReaderException : public Exception {
public:
	StringReaderException(const String &m) : Exception(m) {}
	StringReaderException(int ln, const String &m) {
		String w;
		w << "Line " << (ln+1) << ": " << m;
		str() << w;
	}
};

class ByteBufferWriter : public Sink {
public:
	Array<char> &data() {
		return data_;
	}

	void write(Sink &sink);
	void write(const char *data, int length) ;

	// sink interface:
	virtual Sink& operator << (const char *s) {
		write(s,String::strLength(s)+1);
		return *this;
	}
	virtual Sink& operator << (const String &s) {
		return this->operator << (s.chars());}
	virtual Sink& operator << (int i) {
		write((const char *)&i, sizeof(i));
		return *this;

	}
	virtual Sink& operator << (char c) {
		data_.add(c);
		return *this;
	}
	virtual Sink& operator << (short i){
		write((const char *)&i, sizeof(i));
		return *this;
	}
	virtual Sink& operator << (double d){
		write((const char *)&d, sizeof(d));
		return *this;
	}
	virtual Sink& operator << (float f){
		write((const char *)&f, sizeof(f));
		return *this;
	}
#if DEBUG
	/*	Dump contents of byte buffer to C++ source file form
	*/
	void dumpCPP(Sink &s) const;
#endif

private:
	Array<char> data_;
};
class ByteBufferReader : public Source {
public:
	 virtual ~ByteBufferReader() {}
	ByteBufferReader(const char *data) {
		offset_ = 0;
		data_ = data;
	}
	ByteBufferReader(const unsigned char *data) {
		offset_ = 0;
		data_ = (const char *)data;
	}
	virtual Source& operator >> (String &s) {
		s.set(data_ + offset_);
		offset_ += s.length() + 1;
		return *this;
	}
	virtual Source& operator >> (int &i) {
		i = *(int *)(data_ + offset_);
		offset_ += sizeof(int);
		return *this;
	}
	virtual Source& operator >> (short &s) {
		s = *(short *)(data_ + offset_);
		offset_ += sizeof(short);
		return *this;
	}
	virtual Source& operator >> (char &c) {
		c = data_[offset_++];
		return *this;
	}
	virtual Source& operator >> (double &d)
	{
		d = *(double *)(data_ + offset_);
		offset_ += sizeof(double);
		return *this;
	}
	virtual Source& operator >> (float &f) {
		f = *(float *)(data_ + offset_);
		offset_ += sizeof(float);
		return *this;
	}
	virtual bool eof() {return false;}
private:
	int offset_;
	const char *data_;
};

class OutputStreamWrapper : public Sink {
public:
	OutputStreamWrapper(ostream &s) : s_(s) {}

	// sink interface:
	virtual Sink& operator << (const char *s) {
		for (int i = 0; s[i]; i++)
			*this << s[i];
		return *this;
	}
	virtual Sink& operator << (const String &s) {
		return this->operator << (s.chars());}
	virtual Sink& operator << (int i) {
		s_ << i;
		return *this;
	}
	virtual Sink& operator << (char c) {
		s_ << c;
		return *this;
	}
	virtual Sink& operator << (short i){
		s_ << i;
		return *this;
	}
	virtual Sink& operator << (double d){
		s_ << d;
		return *this;
	}
	virtual Sink& operator << (float f){
		s_ << f;
		return *this;
	}
private:
	ostream &s_;
};

class InputStreamWrapper : public Source {
public:
	InputStreamWrapper(istream &s) : s_(s) {}
	 virtual ~InputStreamWrapper() {}

	// Source interface:

	/*	Read string up to \n or end of file; don't include the \n
	*/
	virtual Source& operator >> (String &s);

	virtual Source& operator >> (int &i) {
		s_ >> i;
		return *this;
	}
	virtual Source& operator >> (short &s) {
		s_ >> s;
		return *this;
	}
	virtual Source& operator >> (char &c) {
		s_ >> c;
		return *this;
	}

	virtual Source& operator >> (double &d) {
		s_ >> d;
		return *this;
	}

	virtual Source& operator >> (float &f) {
		s_ >> f;
		return *this;
	}
	virtual bool eof() {return s_.eof(); }
private:
	istream &s_;
};


#endif // _FILES
