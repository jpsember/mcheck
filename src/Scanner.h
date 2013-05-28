#ifndef _SCANNER
#define _SCANNER
#if _WITHPP
#include "PreProcessor.h"
#endif
#include "DFA.h"
#include "Token.h"
#include "Files.h"

class ErrorRecord;

class Scanner : public Source {
	// --------------------------------
public:
	/*	Constructor
			> ppOpts					Options for preprocessor
		  > dfa							DFA for recognizing tokens
	*/
	Scanner(int ppOpts = 0, DFA *dfa = 0);
	virtual ~Scanner();
private:
	class LineBuffer;

	// no copying, assignment allowed
	// assignment operator
	Scanner &operator=(const Scanner *&s) {return *this;}
private:
	// initializer
	void construct();
	// --------------------------------

public:

	void getCursor(int &line, int &col) const {
		line = prevLine_+1; col = prevCol_+1; }

	/*	Add a text file to the queue to be read from
			> path					filename
	*/
	void queueTextFile(String &path);
	
	/*	Include a source to read from
			> src						source to include
			> name					optional filename to display (for errors)
	*/
	void includeSource(Source &src, const String *name = 0);

	/*	Read next token from source.  If none remain, token will be T_NULL
			> token						where to store token
			< true if type <> T_NULL 
	*/
	bool read(Token &token);

	/*	Set type of token to skip.
			Used to skip whitespace or comments.
			Only one type of token can be skipped in this way.

			> type						type of token to skip, or -1 to turn off skipping
	*/
	void setSkip(int type) {
		skipType_ = type;
	}

	/*	Consume next token
	*/
	void read();

	/*	Peek at next token in source.  If none remain, token will be T_NULL
			> token						where to store token
			< true if type <> T_NULL 
	*/
	bool peek(Token &token);

	/*	Peek at next token in source.  If none remain, will be T_NULL
	*/
	Token peek();

	/*	Read next token; throw exception if not of expected type
			> token						where to store token
			> type						expected type (T_xxx)
	*/
	void read(Token &token, int type);

	/*	Consume all tokens up to and including the next token of
			a particular type, or until EOF

			> type						type to read past
	*/
	void readPast(int type);

	/*	Read next token; throw exception if not of expected type
			> type						expected type (T_xxx)
	*/
	void read(int type);

	/*	Enable/disable echoing to screen.  If enabled, source file is 
			echoed to stdout.
			> raw							echo raw source
			> filtered				echo preprocessor output
	*/
	void setEcho(bool raw, bool filtered = false) { 
		echo_ = raw;
		echoFiltered_ = filtered;
	}

	/*	Read current echo states	*/
	bool echo() const { return echo_; }
	bool echoFiltered() const { return echoFiltered_; }

	/*	Add an error report for the last token read
			> msg							message to display as error
			> rAction					action to take before continuing scanning (RECOVER_xxx)
	*/
	void addError(const String &s);

	/*	Get # lines read by this scanner
	*/
	int lineNumber() const {return lineNumber_;}

	/*	Flush any errors that still need reporting
	*/
	void flushErrors();		

	/*	Show an error associated with the last token read from
			the scanner
			> msg							message to display as error (usually
												e.str(), where e is the exception that was
												thrown)
	*/
	void showError(const String &msg);

	// Source interface:

	/*	Read string up to \n or end of file; don't include the \n
	*/
	virtual Source& operator >> (String &s);
	virtual Source& operator >> (int &i);
	virtual Source& operator >> (short &s);
	virtual Source& operator >> (char &c);
	virtual Source& operator >> (double &d);
	virtual Source& operator >> (float &f);
	virtual bool eof();

private:

	/*	Get text of last token read, for further parsing
			> str							where to store the text
	*/
	void getTokenText(String &dest);

	/*	Find buffered source line with a particular number
			> lineNumber			line number to search for
			> bufferIndex			if found, index of buffer stored here
			< pointer to LineBuffer if found, 0 if not
	*/
	LineBuffer *findLineBuffer(int lineNumber, int &bufferIndex) const;

	/*	Move tokenizer into reading() state, continuing stacked
			readers if necessary.
			< true if reading() state attained; false if done source
	*/
	bool startTokenizer();

	/*	Read next line of source to an appropriate	LineBuffer
	*/
	void readLineToBuffer();

	/*	Apply the preprocessor to the active LineBuffer,
			echo output as necessary
	*/
	void filterAndEcho();

	/*	Read next token from source, without checking the lookahead.
	*/
	void read_(Token &token);

	/*	Print a line of source, optionally preceded by line numbers
			> s								source line
			> lineNumber			line number within file (0 = first line)
	*/
	void printSourceLine(const String &s, int lineNumber,
		Sink *outPtr = 0) const;

	class SourceRec {
	public:
		SourceRec() {
			source_ = 0;
			lineNumber_ = 0;
		}
		Source *source() const {return source_;}
		int lineNumber() const {return lineNumber_;}
		const String &name() const {return name_;}
		void start(Source &src, const String *name);
		void incLineNumber() {lineNumber_++;}
		void close();
	private:
		String name_;
		Source *source_;
		int lineNumber_;
	};

#if _WITHPP
	PreProcessor *pp_;
#endif
	StringReader *tokenizer_;
	DFA *dfa_;
	int ppOpts_;
	// for explicit CRs, true if we've sent the T_CR for this line
	bool crSent_;

	Stack<SourceRec> readers_;
	SourceRec reader_;

	// lines read by this scanner (0 = first line)
	int lineNumber_;
	// array of line buffers
	Array<LineBuffer *> lineBuffers_;
	// buffer for storing a single token look-ahead
	Token nextToken_;
	// maximum number of line buffers to store
	int maxLineBuffers_;
	// true to echo output to screen
	bool echo_;
	// true to echo filtered output to screen
	bool echoFiltered_;
	// buffered error messages
	Array<ErrorRecord *> errors_;
	// lineNumber of last error reported, or -1 if none
	int lastErrorLineNumber_;
	// index of LineBuffer in use
	int currentLineBuffer_;
	// have we printed the current pathname for a previous error?
	bool printedErrPath_;		

	class LineBuffer {
		// --------------------------------
	public:
		LineBuffer(int lineNumber) {
			construct();
			lineNumber_ = lineNumber;
		}
		virtual ~LineBuffer() {DESTROY();}
	private: // no copying allowed
		LineBuffer(const LineBuffer &s) {}
		LineBuffer &operator=(const LineBuffer *&s) {return *this;}
	private:
		// initializer
		void construct();
		// --------------------------------
	public:
		// compile session line number this buffer currently contains
		int lineNumber_;
		// raw source string 
		String raw_;
#if _WITHPP
		// filtered (preprocessed) string
		String filtered_;
#endif
	};
	int skipType_;
	int line_;
	int col_;
	int prevLine_;
	int prevCol_;
	// queued text file index
	int fileNumber_;
	// array of queued text files
	StringArray fileQueue_;
	// reader for queued files
	TextReader qReader_;
};

#if DEBUG
void Test_Scanner();
#endif

#endif // _SCANNER
