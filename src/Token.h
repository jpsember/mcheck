#ifndef _TOKEN
#define _TOKEN

class Token {
	// --------------------------------
public:
	/*	Constructor
			> type						type of token (T_xxx)
			> lineNum					compile session line number where token came from,
												 or < 0 if not from source
			> linePos					position in raw (un-PreProcessed) line number 
												 for the start of the token, if lineNum >= 0
	*/
	Token(int type = T_EOF, int lineNum = -1, int linePos = -1);
private:
	// initializer
	void construct();
	// --------------------------------
public:
	void set(int type, int lineNum=-1, int linePos=-1, const String *srcStr = 0,
		int offset=0, int len=0);

	//	Determine the type (T_xxx) of the token
	int type() const { return type_; }

	//	Determine if token's type matches t
	bool type(int t) const {return type() == t;}

	/*	Return a string describing a type of token.
			> token						token type (T_xxx)
			> dfa							if not 0, ptr to DFA containing token names
			< pointer to string, or 0 if no name defined
	*/
	static void getName(int token, String &dest, const DFA *dfa = 0);
#if DEBUG 
	String debInfo(const DFA *dfa = 0) const;
#endif
	void setStr(const String &s) {
		str_ = s;
	}
	/*	Get the string containing the token's text
	*/
	String &str() {return str_;}

	int lineNumber() const {return lineNumber_;}
	int linePos() const {return linePos_;}
//	int userData() const {return userData_;}
//	void setUserData(int d) {userData_ = d;}
private:
	int type_;
	// line number where token occurred (May not be a particular source
	// line, but instead the line # this compile session.  This can be
	// used to indicate which source line buffer it's from.)
	int lineNumber_;
	// position in original (raw) source line for start of this token
	int linePos_;
	// text of token extracted from source
	String str_;
	// application usage
//	int userData_;
};

#endif // _TOKEN
