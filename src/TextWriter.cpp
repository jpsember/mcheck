#include "Headers.h"
#include "Files.h"

TextWriter::TextWriter(const String &path)
{
	CONSTRUCT();
	try {
		open(path);
	} catch (IOException &e) {
		DESTROY();
		throw e;
	}
}

void TextWriter::close()
{
	if (f.is_open()) {
		// if stream was used to write to file, we won't have
		// checked the fail() flag until now.
		if (state != S_ERROR && f.fail())
			setError("closing file: write problem detected");
		f.close();
		if (f.fail())
			setError("closing file");
	}
	if (state != S_ERROR)
		state = S_CLOSED;
}

TextWriter::~TextWriter()
{
	close();
	DESTROY();
}

static const char *writeError = "Writing";


Sink& TextWriter::operator << (const char *s)
{
	ASSERT(state != S_START && state != S_CLOSED);
	f << s;
	if (f.bad())
		setError(writeError);
	return *this;
}

Sink& TextWriter::operator << (int i){
	ASSERT(state != S_START && state != S_CLOSED);
	f << i;
	if (f.bad())
		setError(writeError);
	return *this;
}

Sink& TextWriter::operator << (short i){
	ASSERT(state != S_START && state != S_CLOSED);
	f << i;
	if (f.bad())
		setError(writeError);
	return *this;
}

Sink& TextWriter::operator << (char c){
	ASSERT(state != S_START && state != S_CLOSED);
	f << c;
	if (f.bad())
		setError(writeError);
	return *this;
}

Sink& TextWriter::operator << (double d){
	ASSERT(state != S_START && state != S_CLOSED);
	f << d;
	if (f.bad())
		setError(writeError);
	return *this;
}

#if 0
void TextWriter::write(char c)
{
	ASSERT(state != S_START && state != S_CLOSED);
	f << c;

	if (f.bad())
		setError(writeError);
}


void TextWriter::write(const char *str)
{
	f << str;
	if (f.bad())
		setError(writeError);
}

void TextWriter::write(const String &s)
{
	f << s.chars();
}
#endif

#if DEBUG

String TextWriter::debInfo()
{
	String s;
	s << "TextWriter state=" << state << " Path " << path();
	if (error()) {
		s << " " << errorStr();
	}		
	return s;
}

void Test_TextWriter()
{
	const int TESTS = 3;

#define TO_STR 1
	for (int t = 0; t < TESTS; t++) {
	String r;
	Utils::pushSink(TO_STR ? &r : 0);

	switch (t) {
		case 0: 
			{
				String r;
				TextWriter w;
//				fstream &s = w.open("output.txt");

				Cout << "This text was written by TextWriter.cpp\n";
				Cout << "to test the text file writer class.\n";

				w.close();

				String str;

				TextReader::readToString(w.path(), str);
				Cout << "-----------\n" << str << "-----------\n";
			}
			break;

		case 1: 
			{
#if 0
				TextWriter w;
				fstream &s = w.open("output2.txt");

				s << "An error will be generated after this line...\n";
			//	s.clear(ios_base::badbit);
				s.clear(ofstream::badbit);
				s << "and won't be detected until\n";
				s << "the file is closed after this line.\n";
//						w.write("or until this line is printed\n");
				try {
					Cout << "expecting exception...\n";
					w.close();
					Cout << "SHOULDN'T BE PRINTED!\n";
				} catch (IOException &e) {
					Cout << "...caught:\n" << e.small();
				}
#endif
			}
			break;
		case 2:
			{
				Cout << "Printing (partial) file written previously:\n";
				TextReader::printFile("output2.txt");
			}
			break;
		}
	Utils::popSink();
#if TO_STR
	String p("TextWriter");
	p << t;
	Test(p.chars(),r);
#endif
	}
}
#endif

fstream &TextWriter::open(const char *p)
{
	ASSERT(state == S_START);

	filePath = p;

	String path2(p);
	path2.path_toSystem();

//#undef out
	f.open(path2.chars(),fstream::out);
	if (f.fail())
		setError("Opening file");
	state = S_OPEN;
	return f;
}

