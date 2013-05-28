#include "Headers.h"
#include "Files.h"

#define p2(a) //printf a

void BinaryReader::construct() {
	CONSTRUCT();
	length_ = 0;
}

/*	Constructor
		> p						path to open
*/
BinaryReader::BinaryReader(const String &path) 
{
	construct();

	try {
		open(path);
	} catch (IOException &e) {
		DESTROY();
		throw e;
	}
}

void BinaryReader::seek(int offset) {
	f.seekg(offset, ios_base::beg);
}

/*	Open file for reading (binary mode)
		> p						path to open
*/
fstream &BinaryReader::open(const char *p)
{
	ASSERT(state == S_START);

	filePath = p;

	String path2(p);
	path2.path_toSystem();

	f.open(path2.chars(), fstream::in |  fstream::binary);
	if (f.fail())
		setError("Opening file");
	state = S_OPEN;

  // get length of file:
  f.seekg (0, ios::end);
  length_ = f.tellg();
  f.seekg (0, ios::beg);
	p2(("length is %d\n",length));

	p2(("open BinaryReader %s\n",filePath.chars() ));

#if 0
  // get length of file:
  f.seekg (0, ios::end);
  int length = f.tellg();
  f.seekg (0, ios::beg);
	p2(("length is %d\n",length));
	char work[20];
	f.read (work,3);
	p2((" first three chars are %d,%d,%d\n",work[0],work[1],work[2]));
	p2((" good = %d\n",f.good() ));

	int n;
	n = f.rdstate();
	p2((" n=%d\n",n));
	p2((" badBit=%d\n",n&ios::badbit));
	p2((" eofBit=%d\n",n&ios::eofbit));
	p2((" failBit=%d\n",n&ios::failbit));
#endif

	return f;
}

/*	Close file (if currently open)
*/
void BinaryReader::close()
{
	if (f.is_open()) {
		p2(("close BinaryReader %s\n",filePath.chars() ));
		bool wasFailed = f.fail();
		f.close();
		if (!wasFailed && f.fail())
			setError("closing file");
	}
	if (state != S_ERROR)
		state = S_CLOSED;
}

/*	Destructor
*/
BinaryReader::~BinaryReader()
{
	close();
	DESTROY();
}

#undef pt
#define pt(a) //pr(a)
#if 0
void BinaryReader::readUShort(int &us)
{
	char c0, c1;
	readChar(c0);
	readChar(c1); 

	us = (int)(((unsigned int)(byte)c0) | (((unsigned int)(byte)c1) << 8));
	pt(("Read US: %x\n",us));
}
#endif
int BinaryReader::offset() {
	int offset = f.tellg();
	return offset;
}

#if 0
void BinaryReader::readShort(int &s)
{
	char c0, c1;
	readChar(c0);
	readChar(c1);
	int us = ((unsigned int)c0) | (((unsigned int)c1) << 8);
	ASSERT(sizeof(short) == 2);
	s = (int)((short)us);
	pt(("Read  S: %x\n",s));
}

void BinaryReader::readFloat(double &fl)
{
	char c[4];
	for (int i = 0; i < 4; i++)
		readChar(c[i]);

	float f = *(float *)c;
	fl = f;
}

void BinaryReader::readByte(byte &c)
{
	readChar((char &)c);
}

void BinaryReader::readUInt(int &ui)
{
	int u0, u1;
	readUShort(u0);
	readUShort(u1);

	ui = (int)( ((unsigned int)u0)
		| (((unsigned int)u1) << 16));
}

void BinaryReader::readInt(int &ui)
{
	int x;
	readUInt(x);
	ui = x;
}

#endif

void BinaryReader::read(char *dest, int length)
{
	f.read(dest, length);
	if (!f.good()) 
		setError("Reading");
}

/*	Read a file into a CharArray
		> path				path of file to read
		> a						CharArray to read to
*/
void BinaryReader::readToCharArray(const String& path, CharArray &a)
{
	BinaryReader r(path);
	r.read(a.allocBuffer(r.length_),r.length_);
#if 0
	for (int i = 0; i < r.length_; i++) {
		char c;
		r >> c;
//		r.readChar(c);
		a.add(c);
	}
#endif
}

#if DEBUG
static const char *dashedLine =
	"---------------------------------------------------------------\n";
/*	Display a hex dump of the contents of a file
		> path				Path of file
*/
void BinaryReader::dumpFile(const String& path)
{

	try {
		Cout << "Dumping file: " << path << "\n" << dashedLine;

		CharArray a;
		readToCharArray(path,a);

		bool lf = Utils::hexDump(a.array(),a.length());
		if (!lf)
			Cout << "\n";
		Cout << dashedLine;
	} catch (IOException &e) {
		Cout << e;
	}
}

/*	Return a string describing the object
*/
String BinaryReader::debInfo()
{
	String s;
	s << "BinaryReader state= " << state << " Path " 
		<< path()
	;
	if (error())
		s << " " << errStr;
	return s;
}

void Test_BinaryReader() {
#define TO_STR 0


	String r;
	Utils::pushSink(TO_STR ? &r : 0);

	{
		BinaryWriter w("_test.bin");
		w << 'j'
		 << 's'
		 << '!'
		 << "Jeff"
		 << (short)8000
		 << (int)800000
		 << (double)8000.0
		 << (float)1234.56;
	}
	{
		BinaryReader r("_test.bin");

		char c1,c2,c3;
		String str;
		short s;
		int i;
		double d;
		float f;
		r >> c1 >> c2 >> c3 >> str >> s >> i >> d >> f;
		Cout << "j:" << c1 << "\n"
			<< "s:" << c2 << "\n"
			<< "!:" << c3 << "\n"
			<< "Jeff:" << str << "\n"
			<< "8000:" << s << "\n"
			<< "8000000:" << i << "\n"
			<< "8000.0:" << d << "\n"
			<< "1234.56:" << f << "\n"
			;
	}

	{
		ByteBufferWriter w;
		w << "This is a test";

		TextWriter tw("_bwtest.txt");
		Sink &sink = tw;

		w.dumpCPP(sink);
		// If we don't close the file, we can't open it for reading!
		tw.close();

		TextReader::dumpFile("_bwtest.txt");
	}

	Utils::popSink();
#if TO_STR
	Test("b_read2",r);
#endif
}

#endif	

Source& BinaryReader::operator >> (String &s)
{
	ASSERT(state != S_START && state != S_CLOSED);
	if (!f.good()) {
		throw IOException("Reading");
	}
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
		if (c == 0) break;
		s.append(c);
		if (c == '\n') {
			break;
		}
	}
	return *this;
}

Source& BinaryReader::operator >> (int &i)
{
	char c[4];
	read(c,4);
	i = *(int *)c;
	return *this;
}

Source& BinaryReader::operator >> (short &i)
{
	char c[2];
	read(c,2);
	i = *(short *)c;
	return *this;
}

Source& BinaryReader::operator >> (char &c)
{
	ASSERT(state != S_START && state != S_CLOSED);
	f.get(c);
	if (!f.good())
		setError("Reading");
	return *this;
}

Source& BinaryReader::operator >> (double &d)
{
	char c[sizeof(double)];
	read(c,sizeof(double));
	d = *(double *)c;
	return *this;
}

Source& BinaryReader::operator >> (float &fl)
{
	char c[sizeof(float)];
	read(c,sizeof(float));
	fl = *(float *)c;
	return *this;
}

