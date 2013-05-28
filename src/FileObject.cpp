#include "Headers.h"
#include "Files.h"

void FileObject::writeString(const String &str, const String &path) {
	TextWriter w(path);
	w << str;
}

bool FileObject::exists(const char *path) {
	bool exists = true;
	fstream f;
	String path2(path);
	path2.path_toSystem();

	f.open(path2.chars(), fstream::in |  fstream::binary);

	if (f.fail()) {
		exists = false;

#if DEBUG && 0
	 {
			String desc;
			desc << "Failed to open file " << path << ": ";

			int n;
			n = f.rdstate();
			desc << "badBit=" << bs((n&ios::badbit) != 0) << " ";
			desc << "eofBit=" << bs((n&ios::eofbit) != 0) << " ";
			desc << "failBt=" << bs((n&ios::failbit) != 0) << " ";
			desc << "\n";

			Cout << desc;
		}
#endif
	}
	f.close();
	return exists;
}

FileObject::FileObject()
{
	CONSTRUCT();
	state = S_START;
}

/*	Determine if end of file reached
		< true if at end of file
*/
bool FileObject::eof() {
	return (state == S_EOF);
}

/*	Set error, throw an IOException.  
		If already in error state, does nothing.
		> msg					description of error
*/
void FileObject::setError(const char *msg)
{
	if (!error()) {
		errStr.append(msg);
		if (path().length() > 0) {
			errStr.append(", ");
			errStr.append(path());
		}

		state = S_ERROR;
		close();

		throw(IOException(errStr));
	}
}

Source &InputStreamWrapper::operator >>(String &s) {
	s.clear();
	while (true) {
		char c;
		s_.get(c);
		if (c == '\n' || eof()) break;
		s << c;
	}
	return *this;
}

