#ifndef _DFA
#define _DFA

#define AUGMENT 0

#define DFA_BUILDER 0

#include "Files.h"
#include "OrdSet.h"
#include "CmdArgs.h"
#include "Token.h"

/*	Deterministic Finite State Automaton class, for extracting
		tokens from strings.
*/

/*
		To allow the recognizer to determine which token has
		been recognized, we augment each regular expression
		by adding a token identifier to the end:

		<reg.expr>  ==>  <reg.expr> <TOKENFLAG> <token id>

		When we are scanning input, and we encounter a 
		TOKENFLAG, we know that we've recognized a token
		(without having to match <TOKENFLAG> <token id> to
		the input).

		We can continue scanning after recording information
		about the token.

		There must be only one TOKENFLAG transition from any
		one state.
*/

class DFA {

	// --------------------------------------
public:
	// constructor
	DFA();
private:
	// declare the internal class to be defined later
	class DFAState;
public:
	enum {
		// maximum number of states (must fit in 8 bits, with
		// a 'null' value)
		MAX_STATES = 255,
#if AUGMENT
		// <token id> range:
		TOKENID_START = 128,
		TOKENID_MAX = 255,
#endif
		// range of legal ASCII values for terminal symbols in
		// regular expressions:
		MIN_ASCII = 0,
		MAX_ASCII = 127,
	};

	/*	Attempt to recognize a token from a string.  If recognized,
			the current state of the DFA is left at the appropriate final 
			state; if not, the state will be -1.

			> str							string being scanned
			> startPos				start position within string
			< length					length of recognized token is stored here,
												or 0 if none recognized
			< id of token recognized, or -1 if none
	*/
	int recognize(const String &str, int startPos, int &length);

	/*	Make a particular state a final state.  Creates
			it if it doesn't exist.
	  	> s								state number (0..MAX_STATES-1)
			> code						if >= 0, code to associate with this final state
												(if already has a code, retains lowest one)
	*/
	void makeStateFinal(int s, int code = -1);

	/*	Add a state if it doesn't already exist
			> s								state number (0..MAX_STATES-1)
	*/
	void addState(int s);

	/*	Add a transition between two states; adds the states
			to the DFA if they don't already exist
			> stateI					initial state
			> stateD					destination state
			> symbol					transition symbol; doesn't have to be a valid
												source ASCII char,
												since we augment the regular expressions with
												production numbers (see RegExp.cpp) which act
												like characters
			> rangeCheck			if true, check symbol for legality
												 and throw exception if bad
	*/
	void addTransition(int stateI, int stateD, int symbol,
		bool rangeCheck = true);

	/*	Clear a transition on a symbol
			> stateI					initial state
			> symbol					transition symbol
	*/
	void clearTransition(int state, int symbol);
		
	/*	Specify the start state for the DFA.  Also sets
			the current state to this value.
			> s								state
	*/
	void setStartState(int s);

	/*	Determine start state
			< start state, or -1 if not defined yet
	*/
	int startState() const { return startState_; }

	/*	Determine if a state is a final state
	*/
	bool isFinalState(int state) const;

	/*	Determine the current state of the DFA.
			< current state, or -1 if no start state defined or
				last recognition failed
	*/
	int state() const { return state_; }

	/*	Read DFA from a source
	*/
	void read(Source &s);

	/*	Write to a sink
	*/
	void write(Sink &s);

	/*	Read the definition of the DFA from a text file.
			The format is as defined in Homework #1.
			> s									source
	*/
	void readText(Source &s);
	/* Dump DFA to string in same format as assignment input
	  	> dest							sink to write to
	*/
	void writeText(Sink &dest);

	/*	Determine # states in DFA.
			< # states
	*/
	int numStates() const { return states_.length(); }

#if DEBUG
	//	Return a (debug only) descriptive string for this object
	String debInfo() const;

	// construct some DFA text files for testing the 379 assignments
	static void constructTestFile(const String &InputPath,
			const String &outputPath);

#endif

	/*	Construct a minimal DFA equivalent to this one.
			> minimal				where to store the minimal DFA
	*/
	void minimize(DFA &minimal, bool verbose = false);

	/*	Clear this DFA to its just-constructed state.
	*/
	void reset();

	/*	Store the names of the tokens in the DFA
			> sa							array of token name strings, 0...n-1
	*/
	void setTokenNames(const StringArray &sa);


	/*	Extract tokens from a source 
			> src							source to scan
			> dfa							dfa to use to recognize tokens
			> tokens					where to store the tokens
			> scanOptions			SCAN_xx : options for preprocessor
			> sink						if not 0, token names dumped here
			> verboseSink			if true, includes source in sink as 
												 hash comments (#...) and indents to match
												 source
			> showLexeme			print one token per line, with lexeme to right
			> commentToken		if not 0, ptr to token name of comments (to strip)
	*/
	static void scanSource(Source &src, DFA &dfa, 
		Array<Token> &tokens, int scanOptions = 0, Sink *sink = 0,
		bool verboseSink = false, bool showLexeme = false, const String *commentToken = 0);


	/*	Get the name of a token from the DFA
			> type						type of token
			> dest						token name stored here, or T_xxx if no
												such type is defined in this DFA
	*/
	void getTokenName(int type, String &dest) const;
private:
	enum {
		STATEFLAG_FINAL = 0x8000,

		// 0x9998
		// 0x9995 : added token name strings to file
		// 0x9993 : changed method of regexpr id augmentation
#if !AUGMENT
		VERSION = 0x9992,	// magic number for file version
#else
		VERSION = 0x9995,	// magic number for file version
#endif
		MAX_ORDLIST_LENGTH = 8,	// maximum # tokens in ordered list;
														//  if it grows past this point,
														//  a lookup table is constructed
	};

	//	names of tokens (T_xxx), or empty if none included
	StringArray tokenNames_;

	/*	Update the state
			> state						new state
	*/
	void setState(int state) { state_ = state; }

	/*	Determine what state, if any, is reached by transitioning
			from a state on a symbol
			> stateI					initial state
			> symbol					transition symbol
			< destination state, 0...k-1, or -1 if no transition exists
	*/
	int getTransitionState(int stateI, int symbol);

	/*	Determine if a symbol is within our range
	*/
	static bool isValid(int c) {	
		return (c >= MIN_ASCII && c < MAX_ASCII);
	}

#if AUGMENT
	/*	Determine what <token id>, if any, is associated with the
			current state. 

			< token id, or -1 if none found
	*/
	int findTokenID() const;
#endif

	/*	Get a reference to a state
	*/
	DFAState &getState(int n) const {return states_.itemAt(n); }

	// dynamic array of states
	Array<DFAState> states_;

	// start state of the DFA
	int startState_;

	// current state of the DFA
	int state_;

	// for use in minimize function
	typedef Array<int> Group;
	typedef Array<Group> GroupList;

	// Get an ordered set of all possible symbols for the DFA
	void min_getSymbolSet(OrdSet &symbols) const;
	void min_addInitialGroups(GroupList &groups) const;
	bool min_findGroupToSplit(OrdSet &symbols, GroupList &groups,
													int &splitGroup, int &splitSym, int &splitDest);
	void min_splitGroup(GroupList &groups, int splitGroup,
		int splitSym, int splitDest);
	void min_addSinkTransitions(OrdSet &symbols, int sinkState);
	void min_removeSinkTransitions(OrdSet &symbols, int sinkState);
	void min_constructFromGroups(DFA &orig,OrdSet &symbols, 
		GroupList &groups, int startGroup);
	bool min_lookForDeleteStates(int s);
	void min_clearFlags(int flags);
	void min_copyToOutput(DFA &out) const;
	// counter used to try to speed up group searching
	int min_counter_;
#if DEBUG
	String dumpGroups(Array<Group> groups) const;
#endif

	//	Class for DFA states; it's internal to the DFA class.
	class DFAState {
		// --------------------------------------
	public:
		//	constructor: initialize to non-final, no transitions
		DFAState();
		// --------------------------------------

		/*	Make state a final state
				> code					if >= 0, code to associate with final state
												(only the lowest code is retained)
		*/
		void setFinalFlag(int code = -1);

		//	determine if state is a final state
		bool finalFlag() const { return flag(F_FINAL); }

		void setFlag(int f) {
			flags_ |= f;
		}
		void clearFlag(int f) {
			flags_ &= ~f;
		}
		bool flag(int f) const {return (flags_ & f) != 0; }
		int flags() const {return flags_;}

		/*	Add a transition from this state on a particular symbol
				> stateD					destintion state
				> symbol					symbol to transition on
		*/
		void addTransition(int stateD, int symbol);

		/*	Remove any existing transition from this state on a particular
				symbol
		*/
		void clearTransition(int symbol);

		/*	Determine state to move to on an input symbol
				> symbol					input symbol
				< destination state, or -1 if no transition exists
		*/
		int getTransitionState(int symbol) const;

		/*	Read state from file
		*/
		void read(Source &r);

		/*	Write state to sink
		*/
		void write(Sink &w) const;

#if AUGMENT
		/*	Determine what <token id>, if any, is associated with this
				state
				< token id, or -1 if none found
		*/
		int findTokenID() const;
#endif

#if DEBUG
		/*	Get a string describing the transitions from this state.
		*/
		String transitionInfo() const;
#endif

		/*	Assign the state to a group (for minimization)
		*/		
		void setGroup(int g) { group_ = g; }

		/*	Get group that state is assigned to
		*/
		int group() const {return group_; }

		/*	Get a list of the symbols that have transitions from this state.
				> a							array to store symbols in
		*/
		void getSymbols(Array<int> &a) const;

		enum {
			// state flags

			F_CODE			= 0x0000ffff,	// 1+code to associate with final state
			F_FINAL			= 0x80000000,	// final state?

			// For minimizing:
			F_DELETE		= 0x40000000,	// marked for deletion?
			F_VISITED		= 0x20000000,	// reachable from start state?
		};

		/*	Determine the final state code associated with this state
				< code, or -1 if none
		*/
		int finalCode() const {return (flags() & F_CODE)-1; }

	private:

		// stateNum is the datatype for storing state numbers;
		// we allow 16 bits for a large number of states.
		typedef unsigned short stateNum;

		bool useTable(int symbol) const {
			return lookupUsed_ 
				&& symbol <= MAX_ASCII
				&& symbol >= MIN_ASCII
				;
		}
		enum {
			STATE_BITS = 8 * sizeof(stateNum),
			STATE_MASK = (1 << STATE_BITS)-1,
		};

		/*	Utilities to extract symbol / transition information from
				the ints stored in the transition lists
		*/
		int getSym(int ordListVal) const { return (ordListVal >> STATE_BITS);}
		int getTrans(int ordListVal) const {return (ordListVal & STATE_MASK);}
		int constructOrdListVal(int sym, int trans) const {
			return (sym << STATE_BITS) | (trans & STATE_MASK);
		}
		// F_xxx
		int flags_;	
		// group this state belongs to (for minimizing only)
		int group_;

		// Array of transitions.

		// Transitions are stored in one of two ways:  as an ordered
		// list of symbol+transition pairs, or as a lookup table
		// of transition states indexed by symbol (with -1 marking
		// no transition). 

		// Access via the ordered list is still O(1) if its length
		// is bounded by some constant (i.e. < 6).

		// If list_ is false, the lookup table is NOT USED.

		// The ordered list is always used to store <token id>s, 
		// whose values are too large to fit in the lookup table.

		// A transition is only stored in one of the two places, even
		// though both storage methods may be in use.

		// The lookup table contains k for a transition to state k,
		// or -1 if there's no transition on that symbol.

		Array<stateNum> lookup_;

		OrdSet ordList_;
		// true if lookup table has some entries
		bool lookupUsed_;
	};
};

#if DEBUG
void Test_DFA();
#endif

#endif // DFA
