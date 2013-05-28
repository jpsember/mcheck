#include "Headers.h"
#include <string.h>

#if 0
String::String(const char *s)
{
	ASSERT(s != 0);
	construct();
	append(s,0,stringLength(s));
}
#endif

char String::emptyString[1];

#if DEBUG
void String::lock(int amt) {
	lock_ += amt;
	ASSERT(lock_ >= 0 && lock_ < 100);
	mutable_ = (lock_ == 0);
}
#endif

String::String(const char *s, int offset, int len)
{
	ASSERT(s != 0);
	construct();
	append(s,offset,len);
}

/*	Construct a string that will generate this string in a c++
		source file.  Useful for generating test cases.
*/
String String::cpp(bool withQuotes) const {
	String src;
	if (withQuotes)
		src << "\"";
	for (int i = 0; i < length(); i++) {
		char c = charAt(i);
		switch (c) {
			case '\0':
				src << "\\0";
				break;
			case '\t':
				src << "\\t";
				break;
			case '\n':
				src << "\\n";
				if (i + 1 < length())
					src << "\"\n\"";
				break;
			default:
				if (c < ' ') {
					src << "!!!!! #" << (int)c << " !!!!!";
				} else
					src << c;
				break;
		}
	}
	if (withQuotes)
		src << "\"";
	return src;
}

#if DEBUG

/*	Indent a string by a certain number of spaces
*/
String String::indent(int amt, const char *prompt, int truncLen) const {
	String s;
	String front;
	String line;
	front.pad(amt);
	if (prompt != 0)
		front << prompt;

	bool first = true;
	
	for (int i = 0; i <= length(); i++) {
		char c = 0;
		if (i < length())
			c = charAt(i);

		if (c == 0) {
			if (first) break;
			c = '\n';
		}

		if (first) {
			line.clear();
			first = false;
		}

		line.append(c);
		if (c =='\n') {
			if (truncLen >= 0)
				line.truncate(truncLen,true);
			s << front << line;
			first = true;
		}
	}
	return s;
}

#endif

void String::trimWS() {
	// count whitespace at start
	int i = 0;
	while (i < length()
		&& Utils::isWS(charAt(i)))
		i++;
	remove(0,i);
	i = length() - 1;
	while (i >= 0
		&& Utils::isWS(charAt(i)))
		i--;
	remove(i+1);
}

/*	Append a substring of a zero-terminated string to this String
	> str				string to append
	> offset			start of substring
	> ln   			length of substring, -1 to add all remaining chars
*/
void String::append(const char *str, int offset, int ln)
{
	ASSERT(str != 0);
	ASSERT(mutable_);

	if (ln < 0) 
		ln = stringLength(str) - offset;

	ASSERT(offset >= 0 && ln >= 0);
	if (ln > 0) {
		// determine actual number of characters we're adding
		for (int i = 0; i < ln; i++) {
			if (str[offset + i] == 0) {
				ln = i;
				break;
			}
		}

		int len = length();
		ensureCapacity(len + ln, false);
		charArray_.insert(charArray_.length() - 1, str + offset, ln);
	}
}

/*	Append another String to this one
	> sb				String to append
*/
void String::append(const String &sb, int offset, int length)
{
	if (length < 0)
		length = sb.length() - offset;
	append(sb.chars(),offset,length);
}

/*	Pad String with a sequence of characters to some length n.
	If length >= n, no effect.
	> len				length to pad to
	> c					character to pad with
*/
void String::pad(int len, char c)
{
	ASSERT(mutable_);
	while (length() < len)
		append(c);
}

/*	Append a character to the String
	> c					character to append (must not be zero)
*/
void String::append(char c)
{
	ASSERT(mutable_);
	ASSERT(c != 0);

	// replace terminating zero with new character
	charArray_.set(length(),c);
	// add terminating zero
	charArray_.add(0);
}

/*	Allocate a copy of a zero-terminated character array.
	Caller should free it with freeArray().
	> s					character array to copy
	< copy of array
*/
char * String::allocCharArray(const char *s)
{
	ASSERT(s != 0);
	return allocCharArray(s,0,stringLength(s));
}

/*	Allocate a copy of a substring of a zero-terminated character array.
	Caller should free it with freeArray().
	> s					character array to copy
	> offset			index of start of substring
	> len				length of substring, -1 to copy all remaining
	< copy of array
*/
char * String::allocCharArray(const char *s, int offset, int len)
{
	ASSERT(s != 0);
	if (len < 0)
		len = stringLength(s) - offset;
	ASSERT(len >= 0);

	char *a;
	NewArray(a,len+1);
	for (int i = 0; i < len; i++) {
		a[i] = s[offset + i];
		if (a[i] == 0) {
			break;
		}
	}
	a[len] = 0;
	// describe array using its contents
	DESCARRAY(a, a);

	return a;
}

/*	Determine length of a string.  Equivalent to strlen(), except
	return type is an int.
	> s					string
	< length			length of string
*/
int String::strLength(const char *s)
{
	ASSERT(s != 0);
	return (int)strlen(s);
}

int String::indexOf(const char *s, char c, bool searchFromEnd)
{
	ASSERT(s != 0);

	int foundPos = -1;
	int i;
	for (i = 0; s[i]; i++) {
		if (s[i] != c) continue;
		foundPos = i;
		if (!searchFromEnd) 
			break;
	}
	return foundPos;
}

/*	Return a substring of this string
		> offset					offset of first character 
		> length					number of characters, or -1 to extract
												remainder of string; if not this many
												characters exist at offset, the
												remainder of the string is returned
		< substring
*/
String String::subStr(int offset, int len) const
{
	String out;
	subStr(offset,len,out);
	return out;
}

/*	Return a substring of this string
		> offset					offset of first character 
		> length					number of characters, or -1 to extract
												remainder of string; if not this many
												characters exist at offset, the
												remainder of the string is returned
		> dest						destination for substring
*/
void String::subStr(int offset, int len, String &dest) const
{
	if (len < 0) {
		len = length() - offset;
	}
	ASSERT(offset >= 0);
	if (len + offset > length()) 
		len = length() - offset;
	dest.set(chars(),offset,len);
}

void String::replace(int start, int delLen, const char *s, int insLen) {
	ASSERT(mutable_);
	if (delLen < 0)
		delLen = length() - start;
	if (insLen < 0)
		insLen = stringLength(s);

	ASSERT(start + delLen <= length());

	charArray_.replace(start,delLen,s,insLen);
}

/*	Truncate length if necessary; optionally place ellipses (...)
	at end of string
	> maxLength			maximum length of result
	> addEllipses		if true, and current length exceeds truncation
						length, end of string is replaced with "...";
						if maxLength < 3, no ellipses will be stored;
						in any case, resulting length is <= maxLength
*/
void String::truncate(int maxLength, bool addEllipses) 
{
	ASSERT(mutable_);
	if (length() > maxLength) {
		if (addEllipses && maxLength <= 3) {
			addEllipses = false;
		}
		if (addEllipses) {
			maxLength -= 3;
		}
		{
			charArray_.remove(maxLength);
			charArray_.add(0);
		}
		if (addEllipses)
			append("...");
	}
}

/*	Pad string with a sequence of characters to some length n.
	If length >= n, no effect.
	> s					string to pad
	> len				length to pad to
	> c					character to pad with
*/
void String::pad(char *s, int len, char c)
{
	ASSERT(s != 0);
	int i = stringLength(s);
	while (i < len) 
		s[i++] = c;
	s[i] = 0;
}

/*	Copy a string
	> dest				destination string
	> src				source string
	> maxLen			if >= 0, maximum length of destination string
						(excluding terminating zero); if < 0, entire
						source string is copied
*/
void String::stringCopy(char *dest, const char *src, int maxLen)
{
	ASSERT(dest != 0 && src != 0);
	if (maxLen < 0) 
		strcpy(dest,src);
	else {
		strncpy(dest,src,maxLen);
		dest[maxLen] = 0;
	}
}

void String::lower() 
{
	ASSERT(mutable_);
	for (int i = 0; i < length(); i++)
		setChar(i,String::lower(charAt(i)));		
}

void String::upper() 
{
	ASSERT(mutable_);
	for (int i = 0; i < length(); i++)
		setChar(i,String::upper(charAt(i)));		
}

int String::compare(const char *ca, const char *cb, bool ignoreCase)
{
	ASSERT(ca != 0 && cb != 0);

	int result = 0;

	while (true) {
		char c1 = *ca++,
			c2 = *cb++;
		if (ignoreCase) {
			c1 = String::upper(c1);
			c2 = String::upper(c2);
		}
		if (c1 == c2) {
			if (!c1) 
				break;
			continue;
		}
		result = c1 - c2;
		break;
	}
	return result;
}

#if DEBUG
/*	Get a description of this object
*/
String String::debInfo() const
{
	String s;
	s << "String ";
	s << "len=" << fmt(length(),3);
	if (!mutable_)
		s << " LOCKED";
	s << " [";
	for (int i = 0; i < length(); i++) {
		char c = charAt(i);
		if (c < ' ')
			c = '_';
		s << c;
	}
	s.truncate(70,true);
	s << "]";

	return s;
}
#endif

void String::upper(char *c) {
	while (*c) {
		*c = upper(*c);
		c++;
	}
}

void String::lower(char *c) {
	while (*c) {
		*c = lower(*c);
		c++;
	}
}

#if DEBUG
void String::verifyAbstract() const {
	for (int i = 0; i < length(); i++) {
		if (charAt(i) == '\\') {
			pr(("*** Path hasn't been converted to abstract form: %s\n",chars() ));
			break;
		}
	}
}
#endif

int String::path_nameStart() const
{
	verifyAbstract();
	int n = path_extStart();
	while (n > 0
		&& charAt(n-1) != separatorChar
	) n--;
	return n;
}

int String::path_extStart() const
{
	verifyAbstract();

	int n = path_findExtensionPosition();
	if (n < 0)
		n = length();
	return n;
}


void String::path_addSeparator() {
	verifyAbstract();
	if (length() == 0 || charAt(length()-1) != separatorChar) {
		append(separatorChar);
	}
}
void String::path_toAbstract() {
#if !UNIX 
	for (int i = 0; i < length(); i++) {
		if (charAt(i) == '\\')
			setChar(i,separatorChar);
	}
#endif
}
void String::path_toSystem() {
#if !UNIX 
	for (int i = 0; i < length(); i++) {
		if (charAt(i) == separatorChar)
			setChar(i,'\\');
	}
#endif
}

void String::path_nameOnly() {
	verifyAbstract();
	// clear all text up to and including last occurrence of
	// ':' or '/'
	int i;
	for (i = length()-1; i >= 0; i--) {
		if (charAt(i) == separatorChar
			|| charAt(i) == ':')
			break;
	}
	if (i >= 0)
		remove(0,i+1);
}

void String::path_extOnly() {
	verifyAbstract();
	int extPos = path_findExtensionPosition();
	if (extPos < 0)
		extPos = length()-1;
	remove(0,extPos + 1);
}

int String::path_findExtensionPosition() const
{
	verifyAbstract();
	// search for last '.' that occurs past all separators
	int i;
	int found = -1;
	for (i = 0; i < length(); i++) {
		if (charAt(i) == separatorChar)
			found = -1;
		else if (charAt(i) == '.')
			found = i;
	}
	return found;
}

void String::path_setExt(const String &ext)
{
	verifyAbstract();
	path_removeExt();
	if (ext.length() > 0) {
		append('.');
		append(ext);
	}
}
void String::path_removeExt()
{
	verifyAbstract();
	int pos = path_findExtensionPosition();
	if (pos >= 0)
		remove(pos);
}

#if DEBUG
String String::ptrString(const void *ptr)
{
	char work[20];
	sprintf(work,"%p",ptr);
	String s;
	s << work;
	return s;
}
#endif

void String::split(StringArray &array, char splitChar) const
{
	int wordLen = 0;
	int wordStart = 0;
	for (int i = 0; i < length(); i++) {
		char c = charAt(i);
		if (splitChar == '\0'
			&& (c == ' ' || c == '\t' || c == '\n'))
			c = 0;

		if (c != splitChar) {
			if (wordLen == 0)
				wordStart = i;
			wordLen++;
			continue;
		}
		if (wordLen > 0)
			array.add(String(chars(),wordStart,wordLen));
		wordLen = 0;
	}
	if (wordLen > 0)
		array.add(String(chars(),wordStart,wordLen));
}

void String::join(StringArray &array, const char *joinStr, String &dest) {
	dest.clear();
	for (int i = 0; i < array.length(); i++) {
		if (i > 0 && joinStr != 0)
			dest.append(joinStr);
		dest.append(array.itemAt(i));
	}
}

#if DEBUG
#include "Files.h"

/*	Test class
*/
void Test_String()
{
	const int tests = 4;
	int t0 = 0, t1 = tests-1;

	t0 = 0; t1 = t0;

#define TO_STR 1

	for (int t = t0; t <= t1; t++) {
		String r;
		Utils::pushSink(TO_STR ? &r : 0);
		switch (t) {
			case 0:
				{
					Cout << "Testing construction, freeing up\n";

					String s1("Strawberry");
					String t1("t1");
					String t2("t2");
					String t3("t3");
					String t4("t4");
					String t5("t5");
					String s2, s3;
					s2.append("Banana");

					Cout << s1.debInfo() << "\n";
					Cout << s2.debInfo() << "\n";
					Cout << s3.debInfo().chars() << "\n";

				} break;

			case 1:
				{
#if 0
					Cout << "Constructing StringArray\n";
					StringArray a;
					Cout << a.debInfo();

					Cout << "Adding some strings to it\n";

					static char *s[] = {"B.C","Alberta","Ontario","Quebec",
						"New Brunswick","Nova Scotia",
						"Guatemala","El Salvador","Chile","Paraguay","Colombia",
						"Ecuador","Argentina","Brazil","Nicaragua","Mexico",
						0
					};
					static char *s2[] = {
						"U.S.",
							"Spain","France","Italy","U.K.","Germany","Austria","Sweden",
							0};

						for (int i = 0; s[i]; i++)
							a.add(s[i]);

						Cout << a.debInfo();

						Cout << "Removing Quebec\n";
						a.remove(3,1);
						Cout << a.debInfo();

						Cout << "Adding Panama\n";
						a.add("Panama");
						Cout << a.debInfo();

						Cout << "Replacing Alberta with Idaho\n";
						a.set(1,"Idaho");
						Cout << a.debInfo();

						Cout << "Adding more strings\n";
						for (int i = 0; s2[i]; i++)
							a.add(s2[i]);
						Cout << a.debInfo();

						Cout << "Resizing to minimum\n";
						a.resize();
						Cout << a.debInfo();

						Cout << "Sorting\n";
						a.sort(&String::compareFunc);
						Cout << a.debInfo();

						//	Cout << "Dumping memory\n"; MEM_DUMP();

						Cout << "Destructing StringArray\n";
#endif
				} break;
			case 2:
				{
#if 0
					Cout << "Split string\n";

					StringArray a;
					String s;
					TextReader::readToString("test.txt",s);
					s.split(a);
					Cout << "Split string:\n" << s << "\n" << a.debInfo();
#endif
				} break;
			case 3:
				{
					Cout << "Paths\n";
					const char *st[] = {
						"c:\\sys\\bottom\\top\\underneath.txt",
							"c:\\sys.bot\\whoosis",
							"c:/sys/bottom/top/underneath.txt",
							"whatfor\\this.which\\foo",
							"whatfor\\this.which\\foo.bar",
							"c:",
							"\\",
							0
					};
					for (int i = 0; st[i]; i++) {
						String w(st[i]);
						Cout << "\nOperating on [" << w << "]:\n";

						String w2(w), w3(w), w4(w), w5(w),
							w6(w),w7(w),w8(w),w9(w);
						w2.path_addSeparator();
						w3.path_toAbstract();
						w4.path_toSystem();
						w5.path_nameOnly();
						w6.path_extOnly();
						w7.path_setExt("");
						w8.path_setExt("bin");
						w9.path_removeExt();

						Cout 
							<< w2 << "\n"
							<< w3 << "\n"
							// don't print system-specific path if going to file
#if !TO_STR
							<< w4 << "\n"
#endif
							<< w5 << "\n"
							<< w6 << "\n"
							<< w7 << "\n"
							<< w8 << "\n"
							<< w9 << "\n";
					}
				}
				break;
		}
		Utils::popSink();
#if TO_STR
		String path("String");
		path << t;
		Test(path.chars(), r);
#endif
	}
}
#endif

String String::fixedWidth(int width) const
{
	String s;
	s << *this;
	s.truncate(width-1, true);
	s.pad(width);
	return s;
}

#if DEBUG 
String deb(const String &s, int width) {
	return s.fixedWidth(width);
#if 0
	String out(s);
	out.truncate(width-1,true);
	out.pad(width);
	return out;
#endif
}
#endif

Sink& String::operator << (const char *s) {
	ASSERT(s != 0);
	append(s);
	return *this;
}
Sink& String::operator << (const String &s) {
	append(s.chars());
	return *this;
}

Sink& String::operator << (int i) {
	char work[20];
	Utils::intToStr(i,work);
	append(work);
	return *this;
}

Sink& String::operator << (char c) {
	append(c);
	return *this;
}

Sink& String::operator << (double d){
	char work[30];
	sprintf(work,"%f",d);
	append(work);
	return *this;
}

Sink& String::operator << (float f){
	return this->operator <<((double)f);
}

ostream &operator << (ostream &sout, const String &s)
{
	sout << s.chars(); return sout;
}

void String::ensureCapacity(int newCapacity, bool expectFutureGrowth)
{
	ASSERT(mutable_);
	charArray_.ensureCapacity(newCapacity+1, expectFutureGrowth);
}

bool String::startsWith(const char *s, bool ignoreCase) const
{
	const char *s2 = chars();
	if (ignoreCase) {
		for (int i = 0; s[i]; i++) {
			if (s[i] != s2[i]) return false;
		}
		return true;
	} else {
		for (int i = 0; s[i]; i++) {
			if (upper(s[i]) != upper(s2[i])) return false;
		}
		return true;
	}
}
