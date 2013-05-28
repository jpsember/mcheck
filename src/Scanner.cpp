#include "Headers.h"
#include "Scanner.h"

//#include "RegExp.h"

// Maximum number of source line buffers to maintain.
// Theoretically, if an error was reported for a line
// that was several lines previous to the current one,
// this information would be lost for the purposes of
// printing out the error information.  In practice,
// this won't happen.
#define MAX_LINE_BUFFERS 5

/*	ErrorRecord class.
		
		For recording information about a single error, including
		its source file line, position, and descriptive information.
*/
class ErrorRecord {
public:
	ErrorRecord() {construct();}
	~ErrorRecord() {DESTROY();}
	ErrorRecord(const ErrorRecord &s) {
		construct();
		*this = s;
	}
	ErrorRecord &operator=(const ErrorRecord &s);
	void set(const String &s, int lineNum, int lineNumberThisFile,
			int linePos, const String &src) {
		msg_ = s;
		srcLine_ = src;
		lineNumber_ = lineNum;
		linePos_ = linePos;
		lineNumberThisFile_ = lineNumberThisFile;
	}
private:
	void construct();
public:
	int lineNumber() const {return lineNumber_;}
	int lineNumberThisFile() const {return lineNumberThisFile_;}
	int linePos() const {return linePos_;}
	const String& msg() const {return msg_;}
	const String& src() const {return srcLine_;}
private:

	String srcLine_;
	String msg_;
	int lineNumber_;
	int lineNumberThisFile_;
	int linePos_;
};

void ErrorRecord::construct() {
	CONSTRUCT();
	lineNumber_ = -1;
	linePos_ = -1;
}
ErrorRecord &ErrorRecord::operator =(const ErrorRecord &s) {
	if (this != &s) {
		msg_ = s.msg_;
		lineNumber_ = s.lineNumber_;
		linePos_ = s.linePos_;
	}
	return *this;
}

void Scanner::LineBuffer::construct() {
	CONSTRUCT();
	lineNumber_ = 0;
}

Scanner::Scanner(int ppOpts, DFA *dfa)
{
	construct();

#if _WITHPP
	NewI(pp_,ppOpts);
#endif
	dfa_ = dfa;
	New(tokenizer_);
	ppOpts_ = ppOpts;
}

void Scanner::construct() 
{
	CONSTRUCT();
#if _WITHPP
	pp_ = 0;
#endif
	skipType_ = -1;
	tokenizer_ = 0;
	lineNumber_ = 0;
	maxLineBuffers_ = MAX_LINE_BUFFERS;
	echo_ = false;
	echoFiltered_ = false;
	lastErrorLineNumber_ = -1;
	currentLineBuffer_ = -1;
	printedErrPath_ = false;
	crSent_ = false;
	ppOpts_ = 0;
	line_ = 0;
	col_ = 0;
	prevLine_ = 0;
	prevCol_ = 0;
	fileNumber_ = 0;
}

void Scanner::queueTextFile(String &path) {
	fileQueue_.add(path);
}

Scanner::~Scanner() {
	DESTROY();
	flushErrors();

	// delete all line buffers
	for (int i = 0; i < lineBuffers_.length(); i++) {
		LineBuffer *lb = lineBuffers_.itemAt(i);
		Delete(lb);
	}
#if _WITHPP
	Delete(pp_);
#endif
	Delete(tokenizer_);
}

Token Scanner::peek()
{
	// Read next token if we don't have one, or if we're
	// supposed to skip the one we've got.

#undef p2
#define p2(a) //pr(a)
	p2(("Scanner:peek()\n"));
	while (true) {
		p2((" nextToken type = %d\n",nextToken_.type() ));

		if (!nextToken_.type(skipType_)
			&& !nextToken_.type(T_EOF)) {
				p2(("  type is OTHER, breaking\n"));
				break;
			}

			p2((" reading token into buffer\n"));
		read_(nextToken_);
		if (nextToken_.type(T_EOF)) {
			p2(("   buffered token is EOF, breaking\n"));
			break;
		}
	}
	p2(("  peek, returning %s\n",nextToken_.debInfo().chars() ));
	return nextToken_;	
}

bool Scanner::peek(Token &token)
{
	token = peek();
	return !token.type(T_EOF);
}

bool Scanner::read(Token &token)
{

	// repeat while we get skip tokens
	do {
		prevLine_ = line_;
		prevCol_ = col_;

		// if we have another token buffered, use it
		if (nextToken_.type() != T_EOF) {
			token = nextToken_;
			// clear buffered token 
			nextToken_.set(T_EOF);
		} else {
			read_(token);
		}
		for (int i = 0; i < token.str().length(); i++) {
			char c = token.str().charAt(i);
			col_++;
			if (c == '\n') {
				line_++;
				col_ = 0;
			}
		}
	} while (token.type(skipType_));

	return token.type() != T_EOF;
}

/*	Move tokenizer into reading() state, continuing stacked
		readers if necessary.
		< true if reading() state attained; false if done source
*/
bool Scanner::startTokenizer() {
	while (true) {
		if (
				reader_.source() != 0
 		 && reader_.source()->eof()
		) {
				flushErrors();
				reader_.close();
				// close queued reader in case it was the one being read from
				qReader_.close();
		}

		if ( !reader_.source()) {
			if (!readers_.isEmpty()) {
				reader_ = readers_.pop();
				if (echo()) {
					Cout << "Reading: " << reader_.name() << "\n";
				}
				continue;
			}

			// if there are filenames queued up, open another one.

			if (fileQueue_.length() > fileNumber_) {
				String &path = fileQueue_[fileNumber_++];
				qReader_.open(path.chars());
				includeSource(qReader_,&path);
				continue;
			}
		}
		break;
	}
	return (reader_.source() != 0);
}

/*	Read next line of source to an appropriate	LineBuffer
*/
void Scanner::readLineToBuffer()
{
	// find line buffer for current source line.

	LineBuffer *lb = findLineBuffer(lineNumber_, currentLineBuffer_);

	// if no buffer for current line, add new one;
	// replace oldest line, if we already have all buffers in use
	if (!lb) {
		if (lineBuffers_.length() < maxLineBuffers_) {
			currentLineBuffer_ = lineBuffers_.length();
			LineBuffer *buff;
			NewI(buff, lineNumber_);
			lineBuffers_.add(buff);
		} else {
			int lowest = 0;
			for (int i = 0; i < lineBuffers_.length(); i++) {
				LineBuffer &b = *lineBuffers_.itemAt(i);
				if (i == 0 || b.lineNumber_ < lowest) {
					lowest = b.lineNumber_;
					currentLineBuffer_ = i;
				}
			}
		}
		lb = lineBuffers_.itemAt(currentLineBuffer_);
		lb->lineNumber_ = this->lineNumber_;
	}

	// read line of source to this buffer, 
	// apply preprocessor.

	*reader_.source() >> lb->raw_;
//	reader_->readLine(lb->raw_);
#if DEBUG && 0
	WARN("Reading all lines at once");
	while (!reader_->eof())
		reader_->readLine(lb->raw_);
#endif
}

/*	Apply the preprocessor to the active LineBuffer,
		echo output as necessary
*/
void Scanner::filterAndEcho() {
	LineBuffer  *lb = lineBuffers_.itemAt(currentLineBuffer_);
#if _WITHPP
	pp_->apply(lb->raw_,lb->filtered_);
#endif

	if (echo()) 
		printSourceLine(lb->raw_, reader_.lineNumber());
#if _WITHPP	
	if (echoFiltered()) 
		printSourceLine(lb->filtered_, echo() ? -1 : reader_.lineNumber());
#endif
}

/*	Read next token; throw exception if not of expected type
		> token						where to store token
		> type						expected type (T_xxx)
*/
void Scanner::read(Token &token, int type)
{
	peek(token);
	if (!token.type(type)) {
		throw StringReaderException(lineNumber(),"Bad or missing token");
	}
	read(token);
}

void Scanner::read() 
{
	Token t;
	read(t);
}

void Scanner::read(int type)
{
	Token token;
	peek(token);
	if (!token.type(type)) {
		throw StringReaderException(lineNumber(),"Bad or missing token");
	}
	read(token);
}

void Scanner::readPast(int type) {
	Token t;
	while (true) {
		read(t);
		if (t.type(T_EOF)
			|| t.type(type)) break;
	}
}

void Scanner::getTokenText(String &dest)
{
	int pos, len;
	tokenizer_->getLastItem(pos, len);
	tokenizer_->str().subStr(pos, len, dest);
}

void Scanner::read_(Token &token)
{
#undef pt
#define pt(a) //pr(a)

	pt(("Scanner::read_\n"));

	while (true) {

		// If tokenizer hasn't been started yet, read line from source

		if (!tokenizer_->reading()) {
			if (!startTokenizer()) {
				token = Token(T_EOF);
				break;
			}

			readLineToBuffer();
			reader_.incLineNumber();
			crSent_ = false;
			filterAndEcho();
			LineBuffer *lb = lineBuffers_.itemAt(currentLineBuffer_);
			tokenizer_->begin(
#if _WITHPP
				lb->filtered_
#else
				lb->raw_
#endif
			);
		}

		// if end of line, we must increment line and repeat.
#if _WITHPP
		tokenizer_->readWS();
#endif
		if (tokenizer_->eof()) {
#if _WITHPP			
			if ((ppOpts_ & SCAN_EXPLICITCR)
				&& !crSent_
			) {
				crSent_ = true;
				token = Token(T_CR);
				break;
			}
#endif
			flushErrors();
			lineNumber_++;
			tokenizer_->reset();
			continue;
		}

		pt((" peek= %d (%c)\n", (int) tokenizer_->peekChar(), tokenizer_->peekChar() ));

		int tokenType = 0;
		tokenType = tokenizer_->readToken(dfa_);
		if (tokenType < 0) {
			// construct T_UNKNOWN for the character.
			tokenType = tokenizer_->readUnknownChar();
		}
		
		int tokenPos = -1, tokenLen = -1;
		tokenizer_->getLastItem(tokenPos, tokenLen);

#if 0
		token = Token(
			tokenType, 
			lineNumber_, 
			pp_->filterToRaw(tokenPos)
		);

		token.setStr(tokenizer_->str().subStr(tokenPos,tokenLen));
#else
		// this saves 26% of the time
		token.set(
			tokenType,lineNumber_,
#if _WITHPP
			pp_->filterToRaw(tokenPos),
#else
			tokenPos,
#endif
			&tokenizer_->str(), tokenPos, tokenLen
			);
#endif
//pr(("--[%s]\n",token.str().s() ));

		break;
	}
	//pr((" Scanner read %s\n",token.str().chars() ));
}

void Scanner::printSourceLine(const String &s, int lineNumber,
															Sink *outPtr) const
{
	if (outPtr == 0) {
		outPtr = &Cout;
	}

	Sink &OUT = *outPtr;

		if (lineNumber >= 0)
			OUT << fmt(lineNumber,4) << ": ";
		else
			OUT << "      ";

#if DEBUG && 0
		OUT << "[";
#endif
		// change tabs to spaces
		for (int i = 0; i < s.length(); i++) {
			char c = s.charAt(i);
			if (c < ' ') {
				if (c == '\t')
					c = ' ';
			}
			OUT << c;
		}
#if DEBUG && 0
		OUT << "]";
#endif
		if (s.length() == 0
			|| s.charAt(s.length()-1) != '\n')
			OUT << "\n";
}

Scanner::LineBuffer *Scanner::findLineBuffer(int lineNumber, int &bufferIndex) const
{
	LineBuffer *lb = 0;

	for (int i = 0; i < lineBuffers_.length(); i++) {
		if (lineBuffers_.itemAt(i)->lineNumber_ == this->lineNumber_) {
			lb = lineBuffers_.itemAt(i);
			bufferIndex = i;
			break;
		}
	}
	return lb;
}

typedef ErrorRecord * ePtr;

static int sortErrorsFunc(const ePtr &e1, const ePtr &e2) {
	return (e1->linePos() - e2->linePos());
}

void Scanner::flushErrors() {

	Sink &OUT = Utils::getErrorSink();

	if (errors_.length() == 0) return;

	// We can assume that every error in the buffer is from
	// the same source line, and they should also be in sequential
	// order of positions.  Sort them to be sure.
	errors_.sort(sortErrorsFunc);

	for (int i = 0; i < errors_.length(); i++) {
		ErrorRecord &e = *errors_.itemAt(i);
//OUT << "error record line pos = " << e.linePos() << "\n";
		if (i == 0) {

			if (e.lineNumberThisFile() < 0) {
				OUT << "*** Error: ";
			OUT << e.msg() << "\n";
				continue;
			}

				// Print out the source line (don't do this if it's already been printed
				// (and filtered version hasn't been printed right below it)
				if (!echo() || echoFiltered()) {
					printSourceLine(e.src(), e.lineNumberThisFile(), &OUT);
				}

				// Print out the little pointers below the source
				int lastPos = -1;
				char lastChar = '\0';
				String marks;
				for (int j = 0; j < errors_.length(); j++) {
					ErrorRecord &f = *errors_.itemAt(j);
					int newPos = f.linePos();

					// If last position is same as this one, change to '^'
					if (lastPos == newPos || errors_.length() == 1) {
						lastChar = '^';
					} else {
						if (lastPos >= 0) {
							marks.pad(lastPos);
							marks.append(lastChar);
						}
						lastChar = (char)(j >= 9 ? '|' : ('1' + j));
					}
					lastPos = newPos;
				}
				if (lastPos >= 0) {
					marks.pad(lastPos);
					marks.append(lastChar);
				}
				printSourceLine(marks,-1,&OUT);
			}
			if (!printedErrPath_) {
				if (reader_.source() != 0) {
				//ASSERT(reader_.source() != 0);
					if (reader_.name().length() > 0) {
						OUT << "File " << reader_.name() << ", ";
					}
				}
//					OUT << "File " << reader_->path() << ":\n";
				printedErrPath_ = true;
			}
		if (errors_.length() > 1)
			OUT << fmt(i+1,2) << ": ";
		OUT << e.msg() << "\n";
	}

	while (!errors_.isEmpty()) {
		ErrorRecord *e = errors_.pop();
		Delete(e);
	}
}

void Scanner::showError(const String &msg) {
	addError(msg);
	flushErrors();
}

void Scanner::addError(const String &msg) {

	//	If this line number is greater than the previous error's,
	//  flush the error buffer.
	if (lineNumber_ > lastErrorLineNumber_) {
		flushErrors();
		lastErrorLineNumber_ = lineNumber_;
	}

	int pos = 0, len = 0;
	tokenizer_->getLastItem(pos,len);

	ErrorRecord *rec;
	New(rec);

	String msg2; //("Error: ");
	msg2 << msg;

	String raw;

	if (len > 0) 
		raw.set(lineBuffers_.itemAt(currentLineBuffer_)->raw_);
	
	rec->set(msg2,
		lineNumber_,

		len > 0 ? reader_.lineNumber() : -1,
#if _WITHPP
		pp_->filterToRaw(pos),
#else
		pos,
#endif
		raw
	);

	errors_.add(rec);
}

void Scanner::includeSource(Source &src, const String *name)
{
#undef pt
#define pt(a) //pr(a)

	pt(("Scanner::include\n",name ? name->chars() : "<unknown>"));

	// If current reader exists, push on stack.
	if (reader_.source()) {
		readers_.push(reader_);
		reader_ = SourceRec();
	}
	reader_.start(src, name);
	lineNumber_ = reader_.lineNumber();

	if (echo()) {
		Cout << "Reading: " << reader_.name() << "\n";
//		Cout << " linenumber= " << lineNumber() << "\n";
	}
	printedErrPath_ = false;
}

Source& Scanner::operator >> (String &s)
{
	Token t;
	read(t);
	if (t.type(T_EOF))
		throw StringReaderException(lineNumber(),"Unexpected EOF");
	getTokenText(s);
	return *this;
}


Source& Scanner::operator >> (int &i)
{
	Token t;
	read(t);
	if (t.type(T_EOF))
		throw StringReaderException(lineNumber(),"Unexpected EOF");
	String str;
	getTokenText(str);
	i = Utils::parseInt(str);
	return *this;
}

Source& Scanner::operator >> (short &i)
{
	Token t;
	read(t);
	if (t.type(T_EOF))
		throw StringReaderException(lineNumber(),"Unexpected EOF");
	String str;
	getTokenText(str);
	i = (short)Utils::parseInt(str);
	return *this;
}

Source& Scanner::operator >> (char &c)
{
	Token t;
	read(t);
	if (t.type(T_EOF))
		throw StringReaderException(lineNumber(),"Unexpected EOF");
	
	String str;
	getTokenText(str);
	if (str.length() != 1)
		throw StringReaderException(lineNumber(),"Bad or missing input");
	c = str.charAt(0);
	return *this;
}

Source& Scanner::operator >> (double &d)
{
	Token t;
	read(t);
	if (t.type(T_EOF))
		throw StringReaderException(lineNumber(),"Unexpected EOF");
	String str;
	getTokenText(str);
	d = Utils::parseDouble(str);
	return *this;
}

Source& Scanner::operator >> (float &fl)
{
	Token t;
	read(t);
	if (t.type(T_EOF))
		throw StringReaderException(lineNumber(),"Unexpected EOF");
	String str;
	getTokenText(str);
	fl = (float)Utils::parseDouble(str);
	return *this;
}

bool Scanner::eof() {
	Token t;
	return !peek(t);
}

void Scanner::SourceRec::start(Source &src, const String *name) {
	source_ = &src;
	name_.set(name ? name->chars() : "<unknown>");
	lineNumber_ = 0;
}

void Scanner::SourceRec::close() {
	source_ = 0;
}

#if DEBUG
void Test_Scanner()
{
	int tests = 1;
	int t0 = 0, t1 = tests-1;

//	t0 = 1; t1 =t0;

#define TO_STR 0

	for (int t = t0; t <= t1; t++) {
		String r;
		Utils::pushSink(TO_STR ? &r : 0);

		switch (t) {

		case 0:
			{
				DFA dfa;
				BinaryReader rd2("mesh.dfa");
				dfa.read(rd2);
//				RegExp::loadDFA(dfa, 0, "mesh.rxp", 0);
			
				Scanner s(
					0
#if _WITHPP
					|	SCAN_EXPLICITCR | SCAN_HASHCOMMENTS
#endif
					,
					&dfa);
				s.setEcho(true,true);
				TextReader rd("sample.smf");
				s.includeSource(rd,&rd.path());
				TextReader re;

				//s.includePath("sample.smf");

				Token t;
				enum {
					T_WHITESPACE,
				};

				for (int count = 0; ; count++) {

					ASSERT(count < 50);
					try {
						if (count == 3) {
							re.open("sample2.smf");
							s.includeSource(re,&re.path() );
//							s.includePath("sample2.smf");
						}

						do {
							s.read(t);
							Cout << "Read token: " << t.debInfo(&dfa) << "\n";
						}	while (t.type(T_WHITESPACE));

						if (t.type(T_UNKNOWN)) 
							throw(StringReaderException("Unexpected character"));

						if (t.type(T_EOF)) break;

					} catch (StringReaderException &e) {
						s.addError(e.str());
					}
				}
			}
			break;
		}
		Utils::popSink();
#if TO_STR
		String p("Scanner");
		p << t;
		Test(p.chars(),r);
#endif
	}

}
#endif

