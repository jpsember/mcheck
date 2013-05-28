#ifndef _FORMULA
#define _FORMULA

#include "Graph.h"
#include "Forest.h"

#define FAVOR_LTL 1

class Formula {
public:
	
	/*	Set global pointers to forest containing formulas, and
			the array to hold their tokens
	*/
	static void setAuxDataStructures(Forest &forest, SArray<Token> &tokens) {
		forestPtr_ = &forest;
		tokensPtr_ = &tokens;
	}

	enum Constants {
		// type of formula
		TYPE_SIMPLE,
		TYPE_LTL,
		TYPE_CTL,
		TYPE_CTL_STAR,

	};

	Formula();
	virtual ~Formula();
	// copy constructor
	Formula(const Formula &s);
	// assignment operator
	Formula& operator=(const Formula &s);
	/*	Make a deep copy of the formula by duplicating all the nodes
	*/		
	Formula deepCopy();
private:
	static int deepCopyAux(int node);

	void construct();
public:

	bool isCTL() const {
		return type() == TYPE_CTL || type() == TYPE_SIMPLE;
	}

	bool isLTL() const {
		return type() == TYPE_LTL || type() == TYPE_SIMPLE;
	}

	void printReduced();

	void parse(Scanner &scanner);

#if DEBUG
	/*	Get token description
	*/
	static const char *ts(Token &t);

	/*	Get string describing formula
			> base						index of start node (-1 to use root)
			> brief						true to print abbreviated data
	*/
	const char *s(int base = -1, bool brief = false);
#endif

	/*	Print formula to active sink
			> base						start node (-1 to use root of formula)
			> verbose					if true, prints type of formula
			> priority				previous priority (for parentheses filtering)
	*/
	void print(int base = -1, bool verbose = true, int priority = -1);

	/*	Get type of formula
			< TYPE_xxx
	*/
	int type() const {return typeFlags_;}

	/*	Reduce formula so it uses only a minimal sufficient set of
			connectives.
			
			For CTL, this means only using B,!,&,AF,EU,EX.
			For LTL, reduce according to 'Model Checking, p. 132'; also
					see notes Apr 7 p.1
			For CTL*,no change performed.

			> startNode				node to start at; should be -1 to start at root node
	*/
	void reduce(int startNode = -1);

	/*	Get token type from formula
	 		> node						node of tree
			< type of token
	*/
	static int nType(int node);

	/*	Get token from formula
			> node						node of tree
			< Token
	*/
	static Token &token(int node);

	/*	Get token type from formula
	//		> node						node of tree, or -1 for root
			< type of token
	*/
	int rootType() {return nType(root());}

	/*	Get token string from formula
			> node						node of tree, or -1 for root
			< string
	*/
	const String &str(int node = -1);

	/*	Get child node from formula
			> node						node to get child of
			> childIndex			index of child
			< child node
	*/
	static int child(int node, int childIndex);

	/*	Get # children for node in formula
			> node						node to get child count of 
			< # children
	*/
	static int nChildren(int node); // = -1);

	/*	Get root of formula
			< root node, or -1 if formula is empty
	*/
	int root();

	/*	Determine if two formulas are identical
	*/
	static bool equal(int root1, int root2);

	/*	Enable/disable parentheses filtering
			> f								true to enable, false to disable
	*/
	static void filterParen(bool f) {
		filterParen_ = !f;
	}

	/*	Get forest containing formula
			< Forest
	*/
	static Forest &forest();

	/*	Print a formula starting at a particular node
			> root						root node of tree describing formula
			> priority				previous priority (for parenthesizing)
	*/
	static void printNode(int root, int priority = -1);

	/*	Construct the negation of a formula
			> root						root node of formula
			< negated formula
	*/
	static Formula negate(int root);

	/*	Construct the negation of the formula
			< negated formula
	*/
	Formula negate() {return negate(root());}

#if DEBUG
	static void printForest();
#endif

	/*	Convert formula to a directed acyclic graph to eliminate
			duplicate subformulas
	*/
	void convertToDAG();

	// callback for forest
	static int cbFunc(int cmd, int arg1=-1, int arg2=-1);

	/*	Determine if formula represents a literal (negated or not)
			> root						root node of formula
			< 1 if TRUE
			  2+n if prop var of symbol #n
				-1 if FALSE
				-(2+n) if negated prop var of symbol #n
				0 other

			Thus if r > 0, its negated value is simply -r
	*/
	static int getLiteralCode(int root);

private:

	/*	Print a token to current sink
			> t								token
	*/
	static void printToken(Token &t);

	enum pConstants {
		BF_LTL = 0x01,
		BF_CTL = 0x02,

	};

	/*	Determine type of formula (LTL, CTL, etc) by examining
			parse tree.
			> startNode				node to start from, or -1 to start at root
												(in which case LTL, CTL flags are cleared first)
	*/
	void calcType(int startNode = -1);

	/*	Add a token to the tree
			> t								token to add
			> parent					id of parent to attach to,
													or -1 to attach to root; if root doesn't exist,
													stores as root
			> attachIndex			insertion position in parent's child list; -1 to
													attach as rightmost child
	*/
	int addToken(Token &t, int parent = -1, int attachIndex = -1);

	/*	Add a formula to the tree
			> fAdd						formula containing subformula to be added
			> fStart					node of subformula's root (-1 for entire formula)
			> attachParent		id of parent to attach to,
													or -1 to attach to root; if root doesn't exist,
													stores as root
			> attachIndex		  insertion position in parent's child list; -1 to
													attach as rightmost child
			> replaceFlag			if true, doesn't insert child, but replaces existing
	*/
	void addFormula(Formula &fAdd, int fStart = -1, 
		int attachParent = -1, int attachIndex = -1, bool replaceFlag = false);

	static void parse_fa(Scanner &s, Formula &f);
	static void parse_fb(Scanner &s, Formula &f);
	static void parse_fbp(Scanner &s, Formula &f, Formula &fin);
	static void parse_fc(Scanner &s, Formula &f);
	static void parse_fd(Scanner &s, Formula &f);
	static void parse_fe(Scanner &s, Formula &f);

	/*	Get array of tokens
			< array of tokens
	*/
	static SArray<Token> &tokens();

	public: // public for debug only!
	// id of tree containing formula (-1 if formula is empty)
	int treeId_;
private:
	// BF_xxx set for LTL, CTL specific connectives
	int typeFlags_;

	// ptr to forest containing nodes of formula
	static Forest *forestPtr_;
	// ptr to array of tokens
	static SArray<Token> *tokensPtr_;

	// if true, doesn't filter extraneous parentheses
	static bool filterParen_;
};

#endif // _FORMULA

