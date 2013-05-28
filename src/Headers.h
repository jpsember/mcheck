// Includes header files for application.  
// Supports Windows and Linux operating systems.
//

// Some conditional-compile flags of particular interest are:
//
//	WINDOWS		true if compiling for Windows; false if for Unix/Linux/DOS.
//	DEBUG			true if debug features (assertions, memory tracking) 
//								are to be included
// 

// Visual C++ defines _WINDOWS on the command line.

#ifdef _WINDOWS		// Include Windows-specific header files:

#define WINDOWS 1
#define UNIX 0
#define DEBUG 0
#define DOS 1
#define WITH_OPENGL 1
#define WITH_DEBWIND 1

#elif WIN32

#pragma warning(disable : 4996)

#define WITH_OPENGL 1
#define WINDOWS 0
#define UNIX 0
#define DEBUG _DEBUG
#define DOS 1

#else

#define WINDOWS 0
#define UNIX 1
#define DEBUG 1
#define DOS 0
#define WITH_OPENGL 0
#define WITH_DEBWIND 0

#endif

#include <limits.h>
#include <fstream>
#include <iostream>
#include <cstdlib>
using namespace std;

// Forward reference to class String to be defined later:
class String;
typedef unsigned char byte;

// Define some useful functions
template <typename X>
inline X maxVal(X a, X b) {
	return (a >= b) ? a : b;
}

template <typename X>
inline X minVal(X a, X b) {
	return (a <= b) ? a : b;
}

template <typename X>
void clamp(X &a, X b, X c) {
	if (a < b)
		a = b;
	if (a > c)
		a = c;
}

#include "Debug.h"
#include "Array.h"
#include "MyString.h"
#include "Utils.h"
#include "Exception.h"
#include "Ptr.h"
