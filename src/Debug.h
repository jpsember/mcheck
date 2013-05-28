#if DEBUG
class Debug 
{
public:
	static void myAssert(const char *file, int line, bool flag, 
		const char *msg, bool exitIfFail);
	static void myNew_(const char *file, int line, const void *ptr);
	static void myDelete_(const char *file, int line, const void *ptr);
	static void describeMemItem(const void *ptr, const char *msg);
	static void openMemory();
	static void closeMemory();
	static void myConstruct(const char *file, int line, void *ptr);
	static void myDestroy(const char *file, int line, void *ptr);
	static bool memTracking();
	static void disableMemoryTracking(bool flag);
	static void restoreMemoryTracking();
	static void dumpMemory(String *strPtr = 0);
	static char *fileLoc(const char *file, int line, char *dest);
	static String fileLoc(const char *file, int line);
	static void warn(const char *str, const char *file, int line);
	static bool testResults(const char *goalFile, const String& results, 
		const char *file, int line);
	static String &str();
	static const char *startTimer();
	static const char *endTimer();
};

/*
	Debug printing
	---------------

	The macro 

		pr()

	will cause printf-style output in DEBUG mode.  
	In non-DEBUG mode, no code is generated.

	Note that to support the variable-length arguments of the printf(...)
	command, the parentheses around the arguments should be doubled up:

		pr(("Length of list=%d\n",listLength));

	will generate the same code as

		printf("Length of list=%d\n",listLength);

	Assertions
	----------

	The macros

		ASSERT(flag)
		ASSERT(flag, msg)

	can be used to test that flag is non-zero.  If an assertion fails,
	an AssertException is thrown.

	In non-DEBUG mode, no code is generated.

	Warnings
	--------

	The macro 

		WARN(msg)

	can be used to print a warning string to cout.  It will only be printed
	once per program execution.  For example, in Program.cpp,

			int a = 8;
			int b = 22;

			for (int i = 0; i < 3; i++ ) {
				cout i << ": " << a << ", " << b << "\n";
				if (a < b) {
					WARN("Not implemented yet");          // this is line 128
				}
				cout << a+b << "\n";
			}

	This produces

			0: 8, 22
			--> Warning (Program   128): Not implemented yet
			30
			1: 8, 22
			30
			2: 8, 22
			30
*/

void My_printf(const char *fmt, ...);
#define pr(a) My_printf a
#define WARN(a) Debug::warn(a, __FILE__, __LINE__)
#define Test(a,b) Debug::testResults(a,b,__FILE__,__LINE__)
#define ASSERT(flag) Debug::myAssert(__FILE__,__LINE__,flag,0,false)
#define ASSERT2(flag, msg) Debug::myAssert(__FILE__,__LINE__,flag,msg,false)
#define ASSERTEXIT(flag) Debug::myAssert(__FILE__,__LINE__,flag,0,true)

#else	// DEBUG

#define pr(a)
#define WARN(a)
#define ASSERT(flag)
#define ASSERT2(flag, msg)
#define ASSERTEXIT(flag)

#endif	// DEBUG

// This macro can be placed around parameters that
// are generating 'unreferenced' warnings
#define REF(a) {void *unused = &(a);}
class Sink;
// global pointer to active stream (0 if none)
extern Sink *activeSink;
#define Cout Utils::getActiveSink()

#include "Memory.h"
