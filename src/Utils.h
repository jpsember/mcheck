/*
	Utility functions
*/
typedef unsigned int uint;
typedef unsigned short word;

class Utils {
public:
	static int decValue(char c);
	static int octValue(char c);
	static int hexValue(char c);
	static void toHex(int val, int digits, String &dest);
	static String toHex(int val, int digits);
	/*	Display a value as a binary string, high bit first
			> val							value to display
			> digits					# bits in val
			> dest						where to store the result
			> syms						if not 0, alternative to standard '0','1'
													representation
	*/
	static void intToBinary(int val, int digits, String &dest, const char *syms = 0);
	static void pushSink(Sink *s);
	static void popSink();
	static Sink &getActiveSink();
	static Sink &getErrorSink();
	static void useSink(Sink *s);
	static void printIntArray(const Array<int> &a, const char *title = 0);
	static void printInt(const int &i);
	static const char *intArrayStr(const Array<int> &a, const char *title = 0);

	static void pad(int len, char c = ' ');

	/*	Parse a string as a value.  String can be:
			[] base 10 number (dddd..), where d is a digit '0'..'9';
			[] hex value ($hhh..), where h is a digit or 'A'..'F','a'..'f'
			[] hex value (0xhhh..)
			[] binary value (bddd..), where d is '0'..'1'
			[] character value ('a', '9', '\'', '\"', '\n')
		Throws a NumberFormatException if not in one of these formats

		> s					String containing value
		< value parsed
	*/
	static int parseValue(const char *s); 
	static int parseValue(const String &s) { return parseValue(s.chars()); }

	/*	Parse a string as:
			[] hex value (hhh..), where h is a digit or 'A'..'F','a'..'f'

		> s					String containing value
		< value parsed
	*/
	static int parseHex(const char *s); 
	static int parseHex(const String &s) { return parseHex(s.chars()); }

	/*	Parse a string as:
			[] binary value (bddd..), where d is '0'..'1'

		> s					String containing value
		< value parsed
	*/
	static int parseBinary(const char *s);
	static int parseBinary(const String &s) { return parseBinary(s.chars()); }

	/*	Parse a string as:
			[] character 'a', '0', '\'', '\n'
		> s					String containing value
		< value parsed
	*/
	static int parseCharacter(const char *s);
	static int parseCharacter(const String &s) { return parseCharacter(s.chars()); }

	static int parseInt(const char *s);
	static int parseInt(const String &s) { return parseInt(s.chars());}
	static double parseDouble(const char *s);
	static double parseDouble(const String &s) { return parseDouble(s.chars());}
	static void parseString(const String &s, String &out) {
		parseString(s.chars(), out); }
	static void parseString(const char *s, String &out);
	static int mod(int val, int divisor);
	static int hashFunction(const char *key);
	static bool isAlphaNumeric(char c, bool startOfWord = false, 
		bool allowUnderscoreAtStart = true);
	static bool isAlpha(char c, bool allowUnderscore = false);
	static int parseValue(const char *s, int minVal, int maxVal, const char *msg = 0);
	static int parseValue(const String &s, int minVal, int maxVal, 
		const char *msg = 0) {return parseValue(s.chars(), minVal, maxVal, msg);}
	static bool isNumeric(char c);
	static bool isLegal(char c);
	static bool isWS(char c);
	static void intToStr(int val, char *dest);
	static void getScreenSize(int &x, int &y, int &w, int &h);
	static void showMsg(const char *s);
	static void showMsg(const String &s) {showMsg(s.chars()); }
#if DEBUG
	static String flagString(int val, int flag, const char *name);
	static void srand(int seed);
	static int rand(int mod = 0);
	static const char * boolStr(bool f);
	static bool hexDump(const void *ptr, int len,
		int rowSize = 16, bool withText = true);
#endif
	static const int INT_MAXVAL = 2147483647;
	static const int INT_MINVAL = 0x80000000;
	static int round(double n) {	return ((int)(n + (n >= 0 ? .5 : -.5))); }
};
#if DEBUG
#define fs(val,flag,str) Utils::flagString(val,flag,str)
void Test_Utils();
#endif
String fmt(int val, int width=5, bool leftJust = false);
String fmt(double val, int width=8, int decPlaces=3);

//	Macros for easier access to utility functions:

#define bs(a) Utils::boolStr(a)

/*	Class for renumbering items.
*/
class Renumber {
public:
#if DEBUG && 0
	Renumber() {track = false;}
#endif
	/*	Add an entry for an old item
			> retain					true if space should be reserved in new
												table for this item; if false, will be deleted
												from new table
			> count						# old items to add
	*/
	void addOldItem(bool retain = true, int count = 1);

	/*	Rename an item
			> oldIndex					current index 
			> newIndex					new index it should have
	*/
	void renameItem(int oldIndex, int newIndex);

	/*	Determine new index of item
			> oldIndex					current index
			< new index, or -1 if it's been deleted
	*/
	int newIndex(int oldIndex) const {
		return oldToNew_[oldIndex];
	}

	/*	Determine # old items
	*/
	int oldItems() const {return oldToNew_.length();}

	/*	Determine # new items
	*/
	int newItems() const {return newToOld_.length();}

	/*	Determine old index of new item #n
	*/
	int oldIndex(int newIndex) const {
		return newToOld_[newIndex];
	}

	/*	Change an index from old -> new
			> index						index to change
	*/
	void renumber(int &index) const {
		index = newIndex(index);
	}
#if DEBUG
	const char *s() const;
//	bool track;
#endif

private:
	Array<int> newToOld_, oldToNew_;
};
