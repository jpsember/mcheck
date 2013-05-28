#ifndef _CMDARGS
#define _CMDARGS

// Class to manipulate command line arguments

// Arguments are of two types:
//  options:  -t, -I
//  values:   file.txt, 42
//
// If a command line argument is '-abcd', each character following '-'
// is parsed as a separate option: '-abcd' = '-a' '-b' '-c' '-d'
//
class CmdArgs {
	// --------------------------------------
private:	// no copying allowed
	// copy constructor
	CmdArgs(const CmdArgs &s) {}
	// assignment operator
	CmdArgs& operator=(const CmdArgs &s) {return *this;}
public:
	// constructor
	CmdArgs(int argc, char* *argv, const char *supportedOptions = 0);
	// destructor
	virtual ~CmdArgs() {DESTROY();}
private:
	void construct();
	// ---------------------------------------
public:
#if DEBUG
	String debInfo() const;

#endif
	//	Parse str into individual strings, add to command arguments
	//	> str							string to split into individual cmd line args
	void addArguments(const String &str);

	//	return the name of the executable (argv[0])
	String &execFilename() {
		return strings_.itemAt(0);
	}
	// determine if there are more arguments to process
	bool hasNext() const {return (argNumber_ < strings_.length()); }
	// determine if there is a next argument which is a value
	bool nextIsValue() const;
	// read next argument as an option; throw exception if it's 
	// not an option, or is missing
	char nextOption();
	// read the next argument if it matches a particular option;
	// if there are no more, or it's not a match, return false
	bool peekOption(char c);

	// parse next argument as integer
	int nextInt() {return Utils::parseInt(nextValue()); }

	// parse next argument as double
	double nextDouble() {return Utils::parseDouble(nextValue()); }

	// read next argument as a value; throw exception if it's 
	// not a value, or is missing
	String &nextValue();
	// indicate that processing is done; generate exception if more
	// arguments remain unprocessed
	void done() const;

	/*	Get next value as a path, and convert to abstract form.
	*/
	String nextPath();
#if DEBUG
	const char * debNext() { return hasNext() ? strings_[argNumber_].chars() : "<none>"; }
#endif
private:
	static bool isOption(const String &s) {
		return (s.length() >= 2 && s.charAt(0) == '-' && Utils::isAlpha(s.charAt(1),false));
	}
	void addArguments(int argc, const char **argv, bool includesExe = false);
	Array<String> strings_;
	int argNumber_;
	String supportedOptions_;
};

class CmdArgException : public Exception {
public:
	// constructor
	CmdArgException(const String &msg) : Exception(msg) {}
};

#endif // _CMDARGS
