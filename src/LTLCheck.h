#ifndef _LTLCHECK
#define _LTLCHECK

/*	LTL Model Checking class
*/

class LTLCheck {
public:
	LTLCheck(Vars &vars, int options = 0) {
		vars_ = &vars;
		options_ = options;
	}

	/*	Check a formula
			> model						model to check
			> f								specification (LTL formula)
	*/
	void check(Model &model, Formula &f);

	/*	Compare two LTL formulas
			> f1
			> f2
	*/
	void compare(Formula &f1, Formula &f2, bool printReduced = false);

#if DEBUG
	const char *s() const;
#endif

	enum {
		OPT_PRINTSTATES = 0x0001,
		OPT_PRINTFULLSEQ = 0x0002,
		OPT_PRINTBUCHI = 0x0004,
	};

private:
	bool option(int flag) const {
		return (options_ & flag) != 0;
	}

	/*	Construct a Buchi automaton for a formula
			> f								specification (LTL formula)
			> negate					true if formula should be negated
			> b								automaton to construct
	*/
	void constructAutomaton(Formula &f, bool negate, Buchi &b);


	/*	Create automaton states
	*/
	void createGraph();

	/*	Build a tableau (see p. 134).  Nodes are stored in nodeList_.
			> node						id of node in graph
	*/
	void expand(int node);

	/*	Node class for constructing automaton
	*/
	class Node {
	public:
		// list of predecessor nodes
		OrdSet incoming;
		// subformulas already processed
		OrdSet fOld;
		OrdSet fNew;
		// subformulas yet to be processed
		OrdSet fNext;
#if DEBUG
		const char *s();
#endif
	};

	Model &model() {return *model_;}

//	void processFormula(int root);

	/*	Construct a new state in the forest, and allocate
			its Node in the forest auxilliary data store

			< id of node
	*/
	int newNode();

	/*	Construct a new state in the forest, return reference to it
			< id							id of node stored here
			< reference to node
	*/
	Node &newNode(int &id) {
		id = newNode();
		return node(id);
	}

	/*	Construct a GBA (gen. buchi automaton) from the states
	*/
	void constructBuchi(Buchi &b);

	/*	Get reference to a particular node
			> n								id of node
			< reference to Node
	*/
	Node &node(int n) {
		return nodes_[n];
	}

	void printStateSet(bool skipNew = false);
	void printState(int n, bool skipNew = false);

	// specification being checked, in reduced form
	Formula f_;

	// model being checked
	Model *model_;

	// flags indicating which vars we've printed warnings about
	BitStore pvWarn_;

	// symbol table
	Vars *vars_;

	// automaton states
	Forest nForest_;

	// LTL -> automaton conversion:

	typedef SArray<int> NodeList;
	NodeList nodeList_;

	SArray<Node> nodes_;

	// id of special 'init' node
	int initNode_;

	int options_;
};

#endif // _LTLCHECK
