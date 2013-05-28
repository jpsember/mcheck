#include "Headers.h"
#include "Files.h"

BinaryWriter::BinaryWriter(const String &path)
{
	CONSTRUCT();
	try {
		open(path);
	} catch (IOException &e) {
		DESTROY();
		throw e;
	}
}

void BinaryWriter::close()
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

BinaryWriter::~BinaryWriter()
{
	close();
	DESTROY();
}

static const char *writeError = "Writing";

void BinaryWriter::write(const char *data, int length) {
	f.write(data, length);
	if (f.bad())
		setError(writeError);
}

void BinaryWriter::writeCharArray(const String& path, CharArray &a, 
																		int offset, int length)
{
	if (length < 0)
		length = a.length() - offset;
	ASSERT(length >= 0);

	BinaryWriter w(path);
	w.write(a.array()+offset,length);
}

#if DEBUG
String BinaryWriter::debInfo()
{
	String s;
	s << "BinaryWriter"
		<< " state=" << state
		<< " Path " << path()
		;

	if (error()) {
		s << " " << errorStr();
	}
	
	return s;
}
#endif

fstream &BinaryWriter::open(const char *p)
{
	ASSERT(state == S_START);

	filePath = p;

	String path2(p);
	path2.path_toSystem();

	f.open(path2.chars(),fstream::out | fstream::binary);
	if (f.fail())
		setError("Opening file");
	state = S_OPEN;
	return f;
}

Sink& BinaryWriter::operator << (const char *s)
{
	ASSERT(state != S_START && state != S_CLOSED);
	f << s;
#if 1	// GCC doesn't like...
	f.put((char)0);
#else
	f << (char)0;
#endif

	if (f.bad())
		setError(writeError);
	return *this;
}

Sink& BinaryWriter::operator << (short i){
	write((char *)&i, sizeof(short));
	return *this;
}

Sink& BinaryWriter::operator << (int i){
	write((char *)&i, sizeof(int));
	return *this;
}

Sink& BinaryWriter::operator << (char c){
	ASSERT(state != S_START && state != S_CLOSED);
	f << c;
	if (f.bad())
		setError(writeError);
	return *this;
}

Sink& BinaryWriter::operator << (double d){
	write((char *)&d, sizeof(double));
	return *this;
}

Sink& BinaryWriter::operator << (float fl){
	write((char *)&fl, sizeof(float));
	return *this;
}

void ByteBufferWriter::write(const char *data, int length) {
		int currLen = data_.length();
		char *dest = data_.allocBuffer(currLen + length);
		memmove(dest + currLen, data, length);
}

void ByteBufferWriter::write(Sink &sink) {
	for (int i = 0; i < data_.length(); i++)
		sink << data_[i];
}

#if DEBUG
void ByteBufferWriter::dumpCPP(Sink &s) const
{
	String hd;
	s << "static unsigned char data[] = {";
	for (int i = 0; i < data_.length(); i++) {
		byte c = (byte)data_.itemAt(i);
		Utils::toHex(c,2,hd);
		if (i > 0)
			s << ",";
		if (i % 16 == 0)
			s << "\n\t";
		s << "0x" << hd;
	}
	s << "\n};\n";
}
#endif
