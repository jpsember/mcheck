//	Cmpt 755 Homework #1
//  (Also 379 Homework #1 testbed)
//

/*	Notes:
		This class is based on that used in HW #1, but has been modified
		to support token ids.

		[] Comments describing a function are placed above
				the declaration of the function (in the interface),
				instead of the definition (the implementation).
				It's problematic to duplicate this information.

		[] There are various DEBUG-only functions that are not
				enabled except on my Windows system.  This includes
				the 379 test generation code.

*/

#include "Headers.h"
#include "Files.h"
#include "DFA.h"
#include "Scanner.h"

int DFA::recognize(const String &str, int startPos, int &length) {

	ASSERT(startPos <= str.length());

	// keep track of the length of the longest token found, and
	// the final state it's associated with
	int maxLengthFound = 0;
	int bestFinalState = -1;
	int bestFinalCode = -1;

	setState(startState_);
	int i = startPos;

	while (i < str.length()) {

		char c = str.charAt(i);

		// if this is not a legal character for a token, no match.
		if (!isValid(c))
			break;

		int newState = getTransitionState(state(), c);
		// if there's no transition on this symbol from the state, no match.
		if (newState < 0)
			break;

		setState(newState);
		i++;

#if AUGMENT
		// if the current state has a <token id> transition, 
		// then we've reached a final state; choose the
		// first such number in the list, since we want the productions
		// that were listed first to have higher priority.

		int token = findTokenID();
		if (token >= 0) {
			maxLengthFound = i - startPos;
			bestFinalCode = token;
			bestFinalState = newState;
		}
#endif

		// With the augmented <token id>s, we will never
		// actually get to the (distinct) final state; so this is not
		// necessary.  We'll leave it in for compatibility with HW #1.

		// if this is a final state, update the longest token length.
		if (isFinalState(newState)) {
			maxLengthFound = i - startPos;
			bestFinalState = newState;
#if !AUGMENT
			bestFinalCode = getState(newState).finalCode();
#endif
		}
	}
	// don't know why this is here.
	setState(bestFinalState);
	length = maxLengthFound;
	return bestFinalCode;
}

void DFA::reset() {
	startState_ = -1;
	state_ = -1;
	states_.clear();
	min_counter_ = 0;
	tokenNames_.clear();
}

DFA::DFA() {
	reset();
}

void DFA::DFAState::clearTransition(int symbol) {
	if (useTable(symbol)) {
		lookup_.set(symbol - MIN_ASCII, 0);
	} else {
		for (int i = 0; i < ordList_.length(); i++) {
			int k = ordList_.itemAt(i);
			if (getSym(k) == symbol) {
				ordList_.removeArrayItem(i);
				break;
			}
		}
	}
}

void DFA::DFAState::addTransition(int stateD, int symbol) {
	if (useTable(symbol)) {
		lookup_.set(symbol - MIN_ASCII, (stateNum)(stateD+1));
	} else {
		// store symbol in ordered list as an integer,
		// which will be sorted by symbol, so store symbol in
		// bits 16..31.

		int tblCode = constructOrdListVal(symbol,stateD);
		ordList_.add(tblCode);

		if (!lookupUsed_
			&& ordList_.length() > MAX_ORDLIST_LENGTH) {
				lookupUsed_ = true;
				lookup_.ensureCapacity(MAX_ASCII + 1 - MIN_ASCII);
				for (int i = MIN_ASCII; i <= MAX_ASCII; i++)
					lookup_.add(0);

				for (int i = 0; i < ordList_.length(); i++) {
					int n = ordList_.itemAt(i);
					int sym = getSym(n);
//					int sym = n >> 16;
					if (sym <= MAX_ASCII) {
						// store in lookup table, and remove from set.
						lookup_.set(sym - MIN_ASCII, 
							(stateNum)(getTrans(n)+1)
						);
						ordList_.remove(n);
						i--;
					}
				}
			}
	}
}

int DFA::DFAState::getTransitionState(int symbol) const {
	int out = -1;
	if (useTable(symbol)) {
		out = ((int)(lookup_.itemAt(symbol - MIN_ASCII)))-1;
	} else {
		for (int i = 0; i < ordList_.length(); i++) {
			int n = ordList_.itemAt(i);
			if (getSym(n) == symbol) {
				out = getTrans(n);
				break;
			}
		}
	}
	return out;
}

DFA::DFAState::DFAState() {
	flags_ = 0;
	group_ = -1;
	// start off in list mode; we'll switch to lookup table mode
	// if the number of transitions on this state grow large.
	lookupUsed_ = false;
}

void DFA::DFAState::setFinalFlag(int code) {
	setFlag(F_FINAL);
	if (code >= 0) {
		ASSERT(code < (int)F_CODE);
		int current = finalCode();
		if (current < 0
			|| code < current
		)
			flags_ = (flags_ & ~F_CODE) | (code + 1);
	}
}

void DFA::makeStateFinal(int s, int code) {
	addState(s);
	DFAState &state = getState(s);
	state.setFinalFlag(code);
}

bool DFA::isFinalState(int state) const {
	return getState(state).finalFlag();
}

void DFA::addState(int s) {
	if (s < 0 || s >= MAX_STATES)
		throw Exception("Invalid state");
	// add new states if necessary between end of state
	// array and this state
	while (s >= numStates()) {
		states_.add(DFAState());
	}
}

void DFA::addTransition(int stateI, int stateD, int symbol, bool rangeCheck) {
	if (rangeCheck) {
		if (!isValid(symbol))
			throw Exception("Invalid transition symbol");
	}
	addState(stateI);
	addState(stateD);

	// ensure there's no transition on this symbol already
	int existDest = getTransitionState(stateI, symbol);
	if (existDest >= 0 && existDest != stateD)
		throw Exception("State already has transition symbol");
	DFAState &s = getState(stateI);
	s.addTransition(stateD, symbol);
}

int DFA::getTransitionState(int stateI, int symbol)
{
	if (stateI < 0 || stateI >= MAX_STATES)
		throw Exception("Invalid state");
	int stateD = -1;
	do {

    if (stateI >= numStates())
			break;

		stateD = getState(stateI).getTransitionState(symbol);
	} while (false);

	return stateD;
}

#if AUGMENT
int DFA::findTokenID() const {
	DFAState &s = getState(state_);
	return s.findTokenID();
}
#endif

#if AUGMENT
int DFA::DFAState::findTokenID() const {
	for (int i = 0; i < ordList_.length(); i++) {
		int n = getSym(ordList_.itemAt(i));
		if (n < TOKENID_START) continue;
		return n - TOKENID_START;
	}
	return -1;
}
#endif

void DFA::setStartState(int s) {
	if (s < 0 || s >= MAX_STATES)
		throw Exception("Invalid state");

	// If this state is a final state, that's a problem!
	// We don't want to recognize zero-length tokens.
	if (isFinalState(s))
		throw Exception("Start state cannot be a final state");

	startState_ = s;
	setState(s);
}
//#define PRG cout << __FILE__ << ":" << __LINE__ << "\n";

void DFA::read(Source &r)
{
#undef pt
#define pt(a) //pr(a)

	reset();
	int v;
	r >> v;
	pt((" version = %d\n",v));
	
	if (v != VERSION)
		throw IOException("Bad version in DFA");
	short sCnt;
	short sState;
	short tokenNameCount;
	r >> sCnt >> sState >> tokenNameCount;

	pt((" # states=%d, start=%d\n",sCnt,sState));

	for (int i = 0; i < sCnt; i++) {
		addState(i);
		DFAState &s = getState(i);
		pt((" reading state %d\n",i));
		s.read(r);
	}
	setStartState(sState);

	for (int i = 0; i < tokenNameCount; i++) {
		String n;
		r >> n;
		tokenNames_.add(n);
		pt((" token %d= %s\n",i,n.chars() ));
	}

	pt((" done reading\n"));
}

void DFA::DFAState::read(Source &r) {
#undef pt
#define pt(a) //pr(a)

	pt(("DFAState::read\n"));

	r >> flags_;

	short sCount;
	r >> sCount;
	pt((" flags=%x #syms=%d\n",flags_,sCount));
	for (short i = 0; i < sCount; i++) {
		char sym, dest;
		r >> sym >> dest;
		pt((" sym %d, trans %d\n",(byte)sym,(byte)dest));
		addTransition((byte)dest,(byte)sym);
	}
}

void DFA::DFAState::getSymbols(Array<int> &a) const
{
	a.clear();
	if (lookupUsed_) {
		for (int j = MIN_ASCII; j <= MAX_ASCII; j++) {
			int s2 = getTransitionState(j);
			if (s2 < 0) continue;
			a.add(j);
		}
	}
	for (int j = 0; j < ordList_.length(); j++) {
		int v = ordList_.itemAt(j);
		int sym = getSym(v);
		if (lookupUsed_ && sym <= MAX_ASCII) continue;
		a.add(sym);
	}
}


void DFA::readText(Source &r)
{
	reset();

//	TextReader r(path);
	StringReader tk;
	
	String lineStr;

	// storage for strings parsed from line; moved out
	// of main loop to minimize object construction/destruction
	String stateA;	
	String stateB;
	String symbol;

	// keep track of what will become the start state
	int startSt = -1;

	while (!r.eof()) {
		r >> lineStr;
//		r.readLine(lineStr);

		tk.begin(lineStr);	

		// is this line blank?
		if (tk.eof()) continue;

		/* parse line as
		
				<final state>

				<initial state> <next state> <transition symbol>
		*/

		const char *errMsg = "Problem with format of source line";

		int s0 = -1;
		int s1 = -1;

		try {

			tk.readWord(stateA);
			s0 = Utils::parseInt(stateA);

			// if no more data on this line, it's a <final state> 
			if (tk.eof()) {

				makeStateFinal(s0);

			} else {

				// read the next state & transition symbol

				tk.readWord(stateB);
				s1 = Utils::parseInt(stateB);

				tk.readWord(symbol);
				
				// check validity of input...
				if (symbol.length() != 1)
					throw Exception(errMsg);

				// add a transition symbol between these states; if a symbol
				// already exists, it will throw an exception (it's supposed
				// to be a DFA, not an NFA)

				// if the states don't exist, they will be created.

				addTransition(s0,s1,symbol.charAt(0));
			}
		} catch (StringReaderException &e) {
//			REF(e);
			throw Exception(errMsg);
		} catch (NumberFormatException &e) {
//			REF(e);
			throw Exception(errMsg);
		}

		// replace startState if necessary
		if (startSt < 0 || s0 == 0)
			startSt = s0;
	}
	if (startSt >= 0)
		setStartState(startSt);
}

#if DEBUG
String DFA::debInfo() const {
	String s("DFA");
	s << " # states=" << numStates();
	s << " startState=" << startState() << "\n";
	for (int i = 0; i < numStates(); i++) {
		DFAState &state = getState(i);
#if !AUGMENT
		String work;
		if (state.flag(DFAState::F_FINAL)) {
			work << "F";
			if (state.finalCode() >= 0)
				work << state.finalCode();
		} 
		work.pad(3);
		s << work;
#else
		s << (char)(state.flag(DFAState::F_FINAL) ? '*' : ' ');
#endif
		s << (char)(state.flag(DFAState::F_DELETE) ? 'D' : ' ');
		s << (char)(state.flag(DFAState::F_VISITED) ? 'V' : ' ');
		s << fmt(i,3) << " ";
		s << state.transitionInfo();
		s << '\n';
	}
	return s;
}

String DFA::DFAState::transitionInfo() const {
	String s;

	s << (char)(lookupUsed_ ? 'L' : ' ');

	Array<int> syms;
	getSymbols(syms);

	int pc = 0;
	for (int i = 0; i < syms.length(); i++) {
		int j = syms.itemAt(i);
		int s2 = getTransitionState(j);
		ASSERT(j >= MIN_ASCII);
#if AUGMENT
		if (j > MAX_ASCII)
			s << "T" << fmt(j - DFA::TOKENID_START,2);
		else 
#else
		if (j > MAX_ASCII)
			s << "#" << fmt(j,3);
		else
#endif
			s << "'" << (char)j << "'";
		s << " --> " << fmt(s2,3) << "  ";
		
		if (++pc == 16) {
			pc = 1;
			s << "\n     ";
		}
	}
	return s;
}

#endif	// DEBUG

// construct a list of all possible symbols
void DFA::min_getSymbolSet(OrdSet &symbols) const {
	Array<int> symList;
	for (int i = 0; i < numStates(); i++) {
		DFAState &s = getState(i);
		s.getSymbols(symList);
		for (int j = 0; j < symList.length(); j++)
			symbols.add(symList.itemAt(j));
	}
}

void DFA::setTokenNames(const StringArray &sa)
{
	tokenNames_ = sa;
}

void DFA::getTokenName(int type, String &dest) const
{
	dest.clear();
	if (type >= tokenNames_.length()) {
		dest << "T_" << type;
	} else
		dest << tokenNames_.itemAt(type);
}

void DFA::scanSource(Source &src, DFA &dfa, 
		Array<Token> &tokens, int scanOptions, Sink *sink,
		bool verboseSink, bool withLexeme, const String *commentToken
)
{
#undef pt
#define pt(a) //pr(a)

	pt(("DFA::scanSource\n"));

	tokens.clear();

	// build a scanner to extract tokens from a text file
	Scanner scanner(scanOptions,&dfa);	
#if DEBUG && 0
	scanner.setEcho(true,true);
#endif
	scanner.includeSource(src);

	String work;
	int prevLine = -1;
	String sourceLine;
	String tokenLine;

	while (true) {
		Token t;
		scanner.read(t);
	
		// it returns EOF if done
		if (t.type(T_EOF))
			break;

		if (t.type(T_UNKNOWN)) {
			scanner.addError("Unrecognized token");
			continue;
		} 

		tokens.add(t);

		pt(("found token: %s\n",t.debInfo().chars() ));

		if (sink) {

			if (withLexeme) {
				dfa.getTokenName(t.type(), work);
				if (commentToken && work.equals(*commentToken))
					continue;

				*sink << work << " " << t.str().cpp(false) << "\n";
				continue;
			}

			if (t.lineNumber() > prevLine) {
				scanner.flushErrors();
				if (prevLine >= 0) {
					if (verboseSink) {
						*sink << "# " << sourceLine << "\n";
					}
					*sink << tokenLine << "\n";
					if (verboseSink)
						*sink << "\n";
					tokenLine.clear();
					sourceLine.clear();
				}
				prevLine = t.lineNumber();
			}

			if (verboseSink) {
				sourceLine.pad(t.linePos());
				sourceLine << t.str();
			}

			// add token text to sink if necessary
			dfa.getTokenName(t.type(), work);
			if (verboseSink) {
				tokenLine.pad(2 + t.linePos());
			}
			tokenLine << work << " ";
		}
	}
	if (sink) {
		if (sourceLine.defined() || tokenLine.defined()) {
			if (verboseSink) {
				*sink << "# " << sourceLine << "\n";
			}
			*sink << tokenLine << "\n";
		}
	}
}
