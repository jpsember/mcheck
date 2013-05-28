#include "Headers.h"
#include "Files.h"

TextReader::TextReader() {
	construct();
}

TextReader::TextReader(const String &path)
{
	construct();
	try {
		open(path);
	} catch (IOException &e) {
		DESTROY();
		throw e;
	}
}

void TextReader::construct() {
	CONSTRUCT();
	lineNumber_ = 0;
}

fstream &TextReader::open(const char *p)
{
	ASSERT((state == S_START || state == S_CLOSED) && p != 0 && stringLength(p) > 0);

	filePath = p;
	
	String path2(p);
	path2.path_toSystem();
	f.clear();
	f.open(path2.chars(),fstream::in);

	if (f.fail())
		setError("Opening file");

	state = S_OPEN;
	return stream();
}

void TextReader::close()
{
	if (f.is_open()) {
		//pr(("<<< TextReader close, path %s\n",filePath.chars()));
		bool wasFailed = f.fail();
		f.close();
		if (!wasFailed && f.fail())
			setError("closing file");
	}
	if (state != S_ERROR)
		state = S_CLOSED;
}

TextReader::~TextReader()
{
	close();
	DESTROY();
}


void TextReader::readToString(const String& path, String &str)
{
	String s;
	TextReader r(path);
	str.clear();
	
	while (true) {
		char c;
		r >> c;
		if (!c || r.eof()) break;
		str.append(c);
	}
}

#if DEBUG
static const char *dashedLine =
	"---------------------------------------------------------------\n";

void TextReader::printFile(const String& path)
{
	try {
		Cout << "File: " << path << "\n" << dashedLine;
		String s;
		readToString(path,s);
		Cout << s;
		Cout << dashedLine;
	} catch (IOException &e) {
		Cout << e;
	}
}

void TextReader::dumpFile(const String& path)
{
	try {
		Cout << "Dumping file: " << path << "\n" << dashedLine;

		String s;
		readToString(path,s);

		bool lf = Utils::hexDump(s.chars(),s.length());
		if (!lf)
			Cout << "\n";
		Cout << dashedLine;
	} catch (IOException &e) {
		Cout << e;
	}
}

String TextReader::debInfo()
{
	String s;
	{
		s << "TextReader state=" << state << " Path " << path();
		if (error()) {
			s << " " << errorStr();
		}
	}
	return s;
}

#endif	// DEBUG


Source& TextReader::operator >> (String &s)
{
	ASSERT(state != S_START && state != S_CLOSED);
	s.clear();
	while (true) {
		char c;
		f.get(c);
		if (!f.good()) {
//			pr(("Problem reading, bad=%s, fail=%s, eof=%s\n",	bs(f.bad()),bs(f.fail()),bs(f.eof()) ));
			if (f.bad()) {
				setError("Reading");
			}
			ASSERT(f.eof());
			state = S_EOF;
			c = 0;
		}
		if (c == 0 || c == '\n') {
			s.append('\n');
			lineNumber_++;
			break;
		}
		s.append(c);
	}
	return *this;
}

Source& TextReader::operator >> (int &i)
{
	ASSERT(state != S_START && state != S_CLOSED);
	f >> i;
	if (!f.good())
		setError("Reading");
	return *this;
}

Source& TextReader::operator >> (short &i)
{
	ASSERT(state != S_START && state != S_CLOSED);
	f >> i;
	if (!f.good())
		setError("Reading");
	return *this;
}

Source& TextReader::operator >> (char &c)
{
	ASSERT(state != S_START && state != S_CLOSED);
	f.get(c);
	if (!f.good()) {
//		pr(("Problem reading, bad=%s, fail=%s, eof=%s\n",	bs(f.bad()),bs(f.fail()),bs(f.eof()) ));
		if (f.bad())
			setError("Reading");
		else {
			c = 0;
			state = S_EOF;
		}
	}
	return *this;
}

Source& TextReader::operator >> (double &d)
{
	ASSERT(state != S_START && state != S_CLOSED);
	f >> d;
	if (!f.good())
		setError("Reading");
	return *this;
}
/*
bool TextReader::eof() {
	return f.eof();
}
*/
Source& TextReader::operator >> (float &fl)
{
	f >> fl;
	if (!f.good())
		setError("Reading");
	return *this;
}

