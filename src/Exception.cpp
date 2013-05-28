#include "Headers.h"

void Exception::construct() {
	CONSTRUCT();
	//cout << "Constructing exception " << ptrStr(this) << "\n";
}

Exception::Exception(const String& s)
{
	construct();
	msg = s;
#if 0
	if (msg.length() == 0)
		msg.set("Exception");
#endif
#if DEBUG && 0
	Cout << "(Constructed exception: " << msg << ")\n";
#endif
}

#if DEBUG
String Exception::small() {
	String s;
	s << *this;

	// Remove line number
	int startNum = -1;
	int length = 0;
	for (int i = 0; i < s.length(); i++) {
		char c = s.charAt(i);
		if (c >= '0' && c <= '9') {
			if (startNum < 0)
				startNum = i;
			length++;
			continue;
		}
		if (startNum >= 0) {
			break;
		}
	}
	if (startNum >= 0) {
		s.replace(startNum,length,"<line#>");
	}
	return s;
}
#endif

