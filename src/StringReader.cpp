#include "Headers.h"
#include "Files.h"
#include "DFA.h"

StringReader::StringReader(const String &s)
{
	construct();
//	str_ = s;
	begin(s);
}

void StringReader::construct() {
	CONSTRUCT();
	cursor_ = 0;
	prevLen_ = -1;
	prevPos_ = -1;
	reading_ = false;
}

#if DEBUG
String StringReader::debInfo() {
	String s("StringReader ");
	s << "cursor=" << cursor_ << "\n";
	String text(str_);
	text.replace(cursor_,0,">>>");
	s << text.indent(5,"]");
	return s;
}

void Test_StringReader() {

#define TO_STR 1

	int tests = 1;
	int t0 = 0, t1 = tests-1;
	for (int t = t0; t <= t1; t++) {
		String r;
		Utils::pushSink(TO_STR ? &r : 0);
		switch (t) {
		case 0:
			{

				String sample(" the time has come, the walrus said, \n"
					" to speak of many things; \n"
					" of shoes, and ships and sealing wax...");

				StringReader s(sample);

				StringReader *tr = &s;

				while (!tr->eof()) {
					Cout << tr->debInfo();
					String line;
					*tr >> line;
//					tr->readLine(line);
					Cout << "Read line: ]" << line << "\n";
				}
			}
			break;
		}
		Utils::popSink();
#if TO_STR
		String path("StrReader");
		path << t;
		Test(path.chars(), r);
#endif
	}
}
#endif

Source& StringReader::operator >> (String &s)
{
//	ASSERT(!eof());
	s.clear();
	int i = cursor_;
	for (; i < str_.length(); i++) {
		if (str_.charAt(i) == '\n') {
			i++;
			break;
		}
	}
	s.set(str_.subStr(cursor_,i-cursor_));
	cursor_ = minVal(i, str_.length());
	return *this;
}

bool StringReader::readWS() {
	while (
			!eof()
			&& Utils::isWS( str_.charAt(cursor_))) {
		 cursor_++;
		 }
	return eof();
}

void StringReader::readWord(String &str) {
	readWS();
	updatePrevPos();

	str.clear();

	int i = 0;
	while (cursor_ + i < str_.length() && !Utils::isWS(str_.charAt(cursor_ + i)))
		i++;
	str.set(str_.subStr(cursor_,i));
	cursor_ += i;
	readWS();
	updatePrevLen(str.length());
}

Source& StringReader::operator >> (int &i)
{
	readWS();
	ASSERT(!eof());

	String str;

	readWord(str);
	i = Utils::parseInt(str);
	return *this;
}

Source& StringReader::operator >> (short &i)
{
	readWS();
	ASSERT(!eof());

	String str;
	readWord(str);
	i = (short)Utils::parseInt(str);
	return *this;
}

Source& StringReader::operator >> (char &c)
{
	ASSERT(!eof());
	c = str_.charAt(cursor_);
	cursor_++;
	return *this;
}

Source& StringReader::operator >> (double &d)
{
	readWS();
	ASSERT(!eof());

	String str;
	readWord(str);
	d = Utils::parseDouble(str);
	return *this;
}

Source& StringReader::operator >> (float &fl)
{
	readWS();
	ASSERT(!eof());
	String str;
	readWord(str);
	fl = (float)Utils::parseDouble(str);
	return *this;
}

void StringReader::getLastItem(int &pos, int &len) const
{
//	ASSERT(prevLen_ > 0);
	pos = 0;
	len = 0;
	if (prevLen_ > 0) {
		pos = prevPos_;
		len = prevLen_;
	}
}


void StringReader::getLastItem(String &dest) const
{
//	ASSERT(prevLen_ > 0);
	dest.clear();
	if (prevLen_ > 0)
		dest.set(str_.subStr(prevPos_,prevLen_));
}

void StringReader::extractWords(const String &s, StringArray &sa)
{
#undef pt
#define pt(a) //pr(a)

	pt(("StringReader::extractWords [%s]\n",s.chars() ));

	sa.clear();

	StringReader t;
	t.begin(s);

	String token;
	int j = 0;

	do {

		if (++j == 1000) break;

		t.readWord(token);
		pt((" read token %s\n",token.chars() ));

		if (token.length() == 0) {
			pt((" ... done\n"));
			break;
		}
		pt((" adding token\n"));
		sa.add(token);
	} while (true);

}

void StringReader::begin(const String &str, int startPos) {

	str_ = str;
	cursor_ = startPos;
	reading_ = true;
	ASSERT(cursor_ >= 0 && cursor_ <= str_.length());

//	readWS();
}

char StringReader::readChar(bool mustExist) {
	ASSERT(reading());
	if (eof() && mustExist)
		throw StringReaderException("Missing character");
	return (eof() ? (char)0 : str_.charAt(cursor_++));
}

bool StringReader::readExpChar(char expected, bool mustFind) {
	char c = peekChar();
	bool found = (c == expected);
	if (found)
		readChar();
	else if (mustFind) {
		String msg("Missing '");
		msg << expected << "'";
		throw StringReaderException(msg);
	}
	return found;
}

char StringReader::peekChar(int offset) const {
	ASSERT(reading());

	int i = cursor_ + offset;
	char c = (i >= str_.length()) ? (char)0 : str_.charAt(i);
	return c;
}

int StringReader::readUnknownChar()
{
	ASSERT(!eof());
	move(1);
	updatePrevLen(1);
	return T_UNKNOWN;
}


int StringReader::readToken(DFA *dfa)
{
	int len;
	int token = -1;

#if _WITHPP
	readWS();
#endif
	updatePrevPos();

	if (dfa != 0) {
		token = dfa->recognize(str_, cursor_, len);
		move(len);
		updatePrevLen(len);
	}
#if _WITHPP
	readWS();
#endif
	return token;
}

void StringReader::move(int amt) {
	ASSERT(reading());
	ASSERT(cursor_ + amt >= 0 && cursor_ + amt <= str_.length());
	cursor_ += amt;
}

