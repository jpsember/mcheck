#include "Headers.h"

#include "Files.h"
#include <errno.h>

// for GetSystemMetrics, etc:
#if WINDOWS	
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#endif

class CoutSink : public Sink {
public:
	~CoutSink() {
		if (this == activeSink)
			activeSink = 0;
	}
	// sink interface:
	virtual Sink& operator << (const char *s) {cout << s;		return *this;}
	virtual Sink& operator << (const String &s) {cout << s.chars(); return *this;}
	virtual Sink& operator << (int i) {cout << i; return *this;}
	virtual Sink& operator << (short s) {cout << s; return *this;}
	virtual Sink& operator << (char c) {cout << c; return *this;}
	virtual Sink& operator << (double d){cout << d; return *this;}
	virtual Sink& operator << (float f){cout << f; return *this;}
};


CoutSink coutSink;
Sink *activeSink = &coutSink;
Stack<Sink *> streamStack;

#define SYS_MEM_DISABLE true
void Utils::pushSink(Sink *s) {
	MEM_DISABLE(SYS_MEM_DISABLE);
	streamStack.push(activeSink);
	useSink(s);
	MEM_RESTORE();
}
void Utils::popSink() {
	MEM_DISABLE(SYS_MEM_DISABLE);
	activeSink = streamStack.pop();
	MEM_RESTORE();
}

static OutputStreamWrapper *errWriter = 0;

Sink &Utils::getErrorSink()
{
	if (errWriter == 0) {
		errWriter = new OutputStreamWrapper(std::cerr);
	}
	return *errWriter;
}

Sink &Utils::getActiveSink() {
	if (activeSink == 0)
		activeSink = &coutSink;
	return *activeSink;
}

void Utils::useSink(Sink *s) {
	if (!s) {
//		if (coutStream == 0) 
//			coutStream = new CoutSink();
//		s = coutStream;
		s = &coutSink;
	}
	if (activeSink != s) {
		activeSink= s;
	}
}

void Utils::showMsg(const char *s) {
#if WINDOWS
	MessageBox(NULL,s,"Message",MB_OK);
#else
	Cout << s;
#endif
}

void Utils::intToStr(int val, char *dest)
{
	if (val < 0) {
		val = -val;
		*dest++ = '-';
	}
	bool leadZero = true;

	static int powers[] = {
		1,10,100,1000,10000,100000,
		1000000,10000000,100000000,1000000000 };

	int i = 9;
	while (i > 0 && powers[i] > val) i--;

	do {
		int power = powers[i];
		int d = abs(val / power);
		if (!(leadZero && d == 0) || !i) {
			leadZero = false;
			*dest++ = '0' + (char)d;
			val -= d * power;
		}
	} while (i--);
	*dest = 0;
}

void Utils::intToBinary(int val, int digits, String &dest, const char *syms)
{
	if (syms == 0)
		syms = "01";

	dest.clear();
	if (digits > 0) {
		uint bit = 1 << (digits - 1);
		while (bit != 0) {
			dest.append(syms[(val & bit) == 0 ? 0 : 1]);
			bit >>= 1;
		}
	}
}

/*	Determine decimal value of a character
	> c					char
	< 0..9, if it's '0'..'9'; else -1
*/
int Utils::decValue(char c) {
    int val = c - '0';
    if (val < 0 || val > 9) val = -1;
    return val;
}

bool Utils::isWS(char c) {
	return (c == ' '
		 || c == '\t'
		 || c == '\n'
		 || c == '\v'
		 || c == '\f'
		 || c == '\r' );
	//return (c <= ' ');
}

bool Utils::isAlpha(char c, bool allowUnderscore) 
{
	c = String::upper(c);
	return (c >= 'A' && c <= 'Z') || (allowUnderscore && c == '_');
}

/*	Determine octal value of a character
	> c					char
	< 0..7, if it's '0'..'7'; else -1
*/
int Utils::octValue(char c) {
    int val = c - '0';
    if (val < 0 || val > 7) val = -1;
    return val;
}

/*	Determine the hex value of a character.
	> c					char
	< 0..15, if it's '0'..'9','A'..'F','a'..'f'; else -1
*/
int Utils::hexValue(char c) {
	c = (char)toupper(c);
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

double Utils::parseDouble(const char *s) {
	bool error = false;
	int len = stringLength(s);
	if (len == 0 || len > 20)
		error = true;

	double val;
	int n = sscanf(s, "%lf", &val);
	if (n != 1) 
		error = true;
	if (error)
		throw NumberFormatException("Problem parsing double");
	return val;
}

int Utils::parseValue(const char *s, int minVal, int maxVal, const char *msg) {
	int v = parseValue(s);
	if (v < minVal || v > maxVal) {
		String str;
		if (msg == 0) {
			str << "Value must be from " << minVal << "..." << maxVal;
			msg = str.chars();
		}
		throw ParseException(msg);
	}
	return v;
}

int Utils::parseValue(const char *s) 
{
	if (stringLength(s) == 0)
		throw NumberFormatException(s);

  int value = 0;

	char c = String::upper(s[0]);
	char c2 = 0;
	if (s[0] != 0)
		c2 = String::upper(s[1]);

  switch (c) {
	case '\'':
		value = parseCharacter(s);
		break;
  case '$':
		value = parseHex(s + 1);
		break;
	case 'B':
		value = parseBinary(s + 1);
		break;
	case '0':
		if (c2 == 'X') {
			value = parseHex(s + 2);
		} else
			value = parseInt(s);
		break;
  default:
		value = parseInt(s);
		break;
	}
  return value;
}

int Utils::parseHex(const char *s) 
{
	if (!*s || stringLength(s) > 8)
			throw NumberFormatException(s);

    long value = 0;
    for (int i = 0; s[i]; i++) {
		int digit = hexValue(s[i]);
		value = (value << 4) | digit;
		if (digit < 0 || value > INT_MAXVAL) 
			throw NumberFormatException(s);
    }
    return (int)value;
}

void Utils::parseString(const char *s, String &out) {
	out.clear();

	const char delim = '"';

	enum {
		S_NOMINAL,
		S_ESCAPED,
		S_DONE,
		S_ERROR,
	};
	int state = S_NOMINAL;
	if (!*s || (*s != delim))
		state = S_ERROR;

	for (int i = 1; state != S_ERROR && s[i]; i++) {
		char c = s[i];
		switch (state) {
			default:
				state = S_ERROR;
				break;
			case S_NOMINAL:
				{
					if (c == '\\') {
						state = S_ESCAPED;
						break;
					}
					if (c == delim) {
						state = S_DONE;
						break;
					}
					out.append(c);
				}
				break;

			case S_ESCAPED:
				{
					state = S_NOMINAL;
					switch (c) {
					default: {
						String e;
						e << "Unrecognized escape sequence: " << c;
						throw StringFormatException(e);
							 }
						break;
					case 'n':
						c = '\n';
						break;
					case '\'':
					case delim:
					case '\\':
						break;
					case 't':
						c = '\t';
						break;
					case 'r':
						c = '\r';
						break;
					case 'a':
						c = '\a';
						break;
					case 'b':
						c = '\b';
						break;
					case 'v':
						c = '\v';
						break;
					}
					out.append(c);
				}
				break;
		}
	}
	if (state != S_DONE)
		throw StringFormatException(s);
}

int Utils::parseCharacter(const char *s)
{
	const char delim = '\'';

	enum {
		S_NOMINAL,
		S_ESCAPED,
		S_DONE,
		S_DONE2,
		S_ERROR,
	};
	int state = S_NOMINAL;
	int value = 0;
	if (!*s || *s != delim)
		state = S_ERROR;

	for (int i = 1; state != S_ERROR && s[i]; i++) {
		char c = s[i];
		switch (state) {
			case S_NOMINAL:
				{
					if (c == '\\') {
						state = S_ESCAPED;
						break;
					}
					value = c;
					state = S_DONE;
				}
				break;
			case S_DONE:
				state = (c == delim) ? S_DONE2: S_ERROR;
				break;
			case S_ESCAPED:
				{
					state = S_DONE;

					switch (c) {
					case 'n':
						value = '\n';
						break;
					case '"':
						value = '"';
						break;
					case delim:
						value = delim;
						break;
					case 't':
						value = '\t';
						break;
					case '\\':
						value = '\\';
						break;
					default:
						state = S_ERROR;
						break;
					}
				}
				break;
			default:
				state = S_ERROR;
				break;
		}
	}
	if (state != S_DONE2)
		throw NumberFormatException(s);
	return value;
}

int Utils::parseBinary(const char *s)
{
	bool error = false;

  long value = 0;
	int i = 0;
  for (; s[i]; i++) {
		int digit = s[i] - '0';
		value = (value << 1) | digit;
		if (digit < 0 || digit >= 2 || value > INT_MAXVAL) {
			error = true;
			break;
		}
	}
	if (i == 0)
		error = true;
	if (error)
		throw NumberFormatException(s);
  return (int)value;
}

/*	Parse a string as:
		[] integer value [-]'dddddd' (d = 0..9)

	> s					String containing value
	< value parsed
*/
int Utils::parseInt(const char *s)
{
#undef pt
#define pt(a) //pr(a)

#if 1
//	pr(("Utils::parseInt [%s], errno=%d\n",s,errno));
	char *tailPtr;
	errno = 0;
	long n = strtol(s,&tailPtr,10);
	if (*tailPtr || errno) {
		String err("Unable to parse integer: ");
		err << s;
		throw NumberFormatException(err);
	}
//static int count; if (++count > 1200)		pt(("parseInt %s n=%d tail=%s\n",s,n,tailPtr));
	return (int)n;
#else
	pt(("parseInt: [%s]\n",s));

	bool error = false;
	long value = 0;
	bool neg = false;
	if (s[0] == '-') {
		neg = true;
		s++;
	}
	int i;
	for (i = 0; s[i]; i++) {
		int digit = s[i] - '0';
		value = (value * 10) + digit;
		pt((" i=%d, d=%d, val=%d\n",i,digit,value));
		
		if (digit < 0 || digit >= 10 || value > INT_MAXVAL) {
			error = true;
			break;
		}
	}
	if (i == 0)
		error = true;
	if (error) {
		String err("Unable to parse integer: ");
		err << s;
		throw NumberFormatException(err);
	}
	int val = (int)value;
	return neg ? -val : val;
#endif
}

/*	Calculate the modulus of a number; never returns a negative value
		> val					value to take mod of
		> divisor			amount to divide by
		< remainder 
*/
int Utils::mod(int val, int divisor)
{
		int k = val % divisor;
		if (k < 0)
			k += divisor;
		return k;
}

bool Utils::isLegal(char c) {
	return ((unsigned int)c >= ' ' && (unsigned int)c <= 127);
}

String fmt(double val, int width, int decPlaces)
{
	char args[6];
	ASSERT(width >= 1 && width < 10 && decPlaces >= 0 && decPlaces <= width);
	args[0] = '%';
	args[1] = '0'+(char)width;
	args[2] = '.';
	args[3] = '0'+(char)decPlaces;
	args[4] = 'f';
	args[5] = 0;
	char work[20];
	sprintf(work,args,val);
	String s2(work);
	return s2;
}

String fmt(int val, int width, bool leftJust)
{
	char work[20];
	Utils::intToStr(val, work);
	String s2;
	if (leftJust) {
		s2 << work;
	}
	s2.pad(width - stringLength(work));
	if (!leftJust)
		s2.append(work);
	return s2;
}

/*	Calculate hash value for string
	> key			string to calculate value for
	< nonnegative hash value
*/
int Utils::hashFunction(const char *key) 
{
	unsigned int h = 0;
	for (int i = 0; key[i]; i++) {
		unsigned int ki = key[i];
		unsigned int highorder = h & 0xf8000000;   
		h = h << 5;
		h = h ^ (highorder >> 27);	
		h = h ^ ki  ;                  
	}
	return (int)(h & ~0x80000000);
}

bool Utils::isNumeric(char c)
{
	return (c >= '0' && c <= '9');
}

bool Utils::isAlphaNumeric(char c, bool startOfWord, 
													 bool allowUnderscoreAtStart)
{
	bool isAlpha = false;

	c = String::upper(c);
	if (
				  (c >= 'A' && c <= 'Z')
			|| (!startOfWord && (c >= '0' && c <= '9'))
			|| (c == '_' && !(startOfWord && !allowUnderscoreAtStart))
			) isAlpha = true;
	return isAlpha;
}

void Utils::getScreenSize(int &x, int &y, int &w, int &h) {
#if WINDOWS
    // Subtract a little for the width of the window frame.
    w = GetSystemMetrics(SM_CXSCREEN) - 20;
    h = GetSystemMetrics(SM_CYSCREEN) - 40; 
    x = 10;
    y = 10;
#if DEBUG
    h -= h/3;   // Leave room for debug window
#endif
#else
    w = 950;
    h = 600;
    x = 20;
    y = 40;
#endif
}

/*	Convert a number to a hex string
		> val								number to convert
		> digits						number of hex digits
		> dest							where to store the string
*/
void Utils::toHex(int val, int digits, String &dest) {
	static char chars[] =  {"0123456789ABCDEF"};

	dest.clear();
  for (int j = digits - 1; j >= 0; j--) {
  int d = (val >> (j << 2)) & 0xf;
		dest.append(chars[d]);
  }
}

String Utils::toHex(int val, int digits) {
	String s;
	toHex(val,digits,s);
	return s;
}

#if DEBUG
/* A C-program for MT19937: Real number version                */
/*   genrand() generates one pseudorandom real number (double) */
/* which is uniformly distributed on [0,1]-interval, for each  */
/* call. sgenrand(seed) set initial values to the working area */
/* of 624 words. Before genrand(), sgenrand(seed) must be      */
/* called once. (seed is any 32-bit integer except for 0).     */
/* Integer generator is obtained by modifying two lines.       */
/*   Coded by Takuji Nishimura, considering the suggestions by */
/* Topher Cooper and Marc Rieffel in July-Aug. 1997.           */

/* This library is free software; you can redistribute it and/or   */
/* modify it under the terms of the GNU Library General Public     */
/* License as published by the Free Software Foundation; either    */
/* version 2 of the License, or (at your option) any later         */
/* version.                                                        */
/* This library is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.            */
/* See the GNU Library General Public License for more details.    */
/* You should have received a copy of the GNU Library General      */
/* Public License along with this library; if not, write to the    */
/* Free Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA   */ 
/* 02111-1307  USA                                                 */

/* Copyright (C) 1997 Makoto Matsumoto and Takuji Nishimura.       */
/* Any feedback is very welcome. For any question, comments,       */
/* see http://www.math.keio.ac.jp/matumoto/emt.html or email       */
/* matumoto@math.keio.ac.jp                                        */

// #include<stdio.h>

/* Period parameters */  
#define N 624
#define M 397
#define MATRIX_A 0x9908b0df   /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */   
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

/* initializing the array with a NONZERO seed */
void Utils::srand(int seed) 
{
	ASSERT(seed != 0);
    /* setting initial seeds to mt[N] using         */
    /* the generator Line 25 of Table 1 in          */
    /* [KNUTH 1981, The Art of Computer Programming */
    /*    Vol. 2 (2nd Ed.), pp102]                  */
    mt[0]= seed & 0xffffffff;
    for (mti=1; mti<N; mti++)
        mt[mti] = (69069 * mt[mti-1]) & 0xffffffff;
}

int Utils::rand(int mod)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N) { /* generate N words at one time */
        int kk;

        if (mti == N+1)   /* if sgenrand() has not been called, */
					Utils::srand(1965); /* a default initial seed is used   */

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1];

        mti = 0;
    }
  
    y = mt[mti++];
    y ^= TEMPERING_SHIFT_U(y);
    y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
    y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
    y ^= TEMPERING_SHIFT_L(y);

		int z = ((int)y) & 0x7fffffff;
		return (mod > 0 ? z % mod : z);
}

#endif

void Utils::pad(int len, char c)
{
	for (int i = 0; i < len; i++)
		Cout << c;
}

#if DEBUG
/*	Return a string "T" or "F" for a boolean value.
	> f					boolean flag
	< string
*/
const char * Utils::boolStr(bool f) {
	const char *strs[] = {
		"F","T"};
	return strs[f ? 1 : 0];
}

String Utils::flagString(int val, int flag, const char *name) {
	ASSERT(name != 0);
	String s;
	if (val & flag)
		s << name << " ";
	return s;
}

/*	Dump a range of memory as a hex dump.
	> ptr				pointer into memory
	> len				length (in bytes)
	> rowSize		# bytes in each row to display
	> withText  true to include text on right side
	< true if data was big enough that a linefeed was printed
*/
bool Utils::hexDump(const void *ptr, int len, int rowSize, bool withText)
{
	bool lf = false;

	String work;

	char *bptr = (char *)ptr;
	CharArray vals;

	bool spaces4 = (rowSize == 16);

	int i = 0;
	while (i < len) {
		int rSize = rowSize;
		if (rSize + i > len)
		rSize = (int) (len - i);

		if (len > rowSize) {
			toHex(i,4,work);
			Cout << work << ": ";
		}
		vals.clear();
		for (int j = 0; j < rowSize; j++) {
			if (j < rSize) {
				int val = bptr[i + j];
				toHex(val,2,work);
				Cout << work;
				vals.add((char)val);
			} else
				Cout << "  ";
			Cout << ' ';
			if (spaces4) {
				if ( (j & 3) == 3)
					Cout << ' ';
			}
		}
		if (withText) {
			Cout << ' ';

			for (int j = 0; j < rSize; j++) {
				int v = vals.itemAt(j) & 0x7f;
				if (v < 0x20)
					v = '.';
				Cout << (char)v;
			}
		}
		if (len > rowSize) {
			Cout << '\n';
			lf = true;
		}
		i += rSize;
	}
	return lf;
}

/*	Test function for class
*/
void Test_Utils()
{
#define TO_STR 0

	const int tests = 5;
	int t0 = 0, t1 = tests-1;

t0 = 4, t1 = t0;

	for (int t = t0; t <= t1; t++) 
	{
		String r;
		Utils::pushSink(TO_STR ? &r : 0);
		Cout << "Utils Test " << t << "\n";

		switch (t) {
			case 0: 
			{
				static const char *strs[] = {
					"1965",
					"$c800",
					"b1000101010",
					"8238a992",
					"$cbg0",
					"b100101200",
					"0xcba2",
					"'\\n'",
					"'\\''",
					"'A'",
					"''",
					"'ab'",
					"'a\\'",
					"'\\\"'",
					0
				};
				for (int i = 0; strs[i] != 0; i++) {
					int val = 0;
					try {
						Cout << "parsing value " << strs[i];
						String s(strs[i]);
						val = Utils::parseValue(s);
						Cout << " = " << val << "\n";
					} catch (NumberFormatException &e) {
						Cout << "\n  " << e;
					}
				}
			} break;
			case 1:
				{
					const char *w = "This is a test string for the hex dump operation.";
					Utils::hexDump(w,stringLength(w));
					Cout << "\n";
				}
				break;
			case 2:
				{
					Utils::srand(1965);
					for (int i = 0; i < 256; i++) {
						Cout << fmt(Utils::rand(100),3);
						if ((i+1)%8 == 0) Cout << "\n";
					}

					Utils::srand(1965);
					for (int i = 0; i < 256; i++) {
						Cout << fmt(Utils::rand(100),3);
						if ((i+1)%8 == 0) Cout << "\n";
					}

					Utils::srand(42);
					for (int i = 0; i < 16; i++) {
						Cout << fmt(Utils::rand(),11);
						if ((i+1)%4 == 0) Cout << "\n";
					}

					Utils::srand(42);
					for (int i = 0; i < 16; i++) {
						Cout << fmt(Utils::rand(),11);
						if ((i+1)%4 == 0) Cout << "\n";
					}
				}
				break;
			case 3:
				{
					Cout << "    S M\n";
					String s("abcABZzy0129!@ _-+~ ");
					for (int i = 0; i < s.length(); i++) {
						char c = s.charAt(i);
						Cout << "'" << c << "' ";
						Cout <<  bs(Utils::isAlphaNumeric(c,true))
							<< " " << bs(Utils::isAlphaNumeric(c,false)) << "\n";
					}
				}
				break;
			case 4:
				{
					static int tv[] = {
						0,1,-1,
							2147483647,
					    0x80000000,
						-200,35,

					-2147483647,
						999
					};
					for (int i = 0; tv[i] != 999; i++) {

						char work[80];
						sprintf(work,"%d",tv[i]);
						String s;
						s << work;
						s.pad(14);
						Utils::intToStr(tv[i],work);
						s << work;
						s << "\n";
						Cout << s;
					}
				}
				break;
		}
		Utils::popSink();
#if TO_STR
		String path("utils");
		path << t;
		Test(path.chars(), r);
#endif
	}
}
#endif

#undef pt
#if DEBUG && 0
#define pt(a) {if (track) pr(a); }
#else
#define pt(a)
#endif

void Renumber::addOldItem(bool retain, int count)
{
	pt(("addOldItem retain=%s count=%d\n",bs(retain),count));
	pt((" oldToNew %s\n newToOld %s\n",oldToNew_.s(true),newToOld_.s(true) ));
	for (int i = 0; i < count; i++) {
		if (retain) {
			oldToNew_.add(newToOld_.length());
			newToOld_.add(oldToNew_.length()-1);
		} else {
			oldToNew_.add(-1);
		}
	}
	pt((" oldToNew %s\n newToOld %s\n",oldToNew_.s(true),newToOld_.s(true) ));
}

void Renumber::renameItem(int oldIndex, int newIndex)
{
	// determine swaps to old->new
	int onA = oldIndex;
	int onB = newToOld_[newIndex];

	int noA = newIndex;
	int noB = oldToNew_[oldIndex];
pt(("rename %d -> %d (%d %d %d %d)\n",oldIndex,newIndex,onA,onB,noA,noB));

	if (onA != onB) {
		int temp = oldToNew_[onA];
		oldToNew_.set(onA, oldToNew_[onB]);
		oldToNew_.set(onB, temp);
	}
	if (noA != noB) {
		int temp = newToOld_[noA];
		newToOld_.set(noA, newToOld_[noB]);
		newToOld_.set(noB, temp);
	}
}

#if DEBUG
const char *Renumber::s() const
{
	String w;
	w << "#old=" << fmt(oldItems()) << " #new=" << fmt(newItems()) << "\n";

	int k = minVal(20,newItems());
	for (int i = 0; i < k; i++)
		w << i << "(" << oldIndex(i) << ") ";
	String &s2 = Debug::str();
	s2.set(w);
	return s2.chars();
}
#endif

void Utils::printInt(const int &i) {
	Cout << i;
}

void Utils::printIntArray(const Array<int> &a, const char *title)
{
	if (title) {
		Cout << title << ": ";
	}
	Cout << '(';
	for (int i = 0; i < a.length(); i++) {
		if (i > 0)
			Cout << ' ';
		Cout << a[i];
	}
	Cout << ")\n";
	
}

#if DEBUG
const char *Utils::intArrayStr(const Array<int> &a, const char *title)
{
	String &s = Debug::str();
	Utils::pushSink(&s);
	printIntArray(a,title);
	Utils::popSink();
	return s.chars();
}
#endif

