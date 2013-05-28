#ifndef _BUCHI
#define _BUCHI

/*	Class for Buchi automata (including generalized Buchi automata)
*/
class Buchi {
public:

	/*	Add a state
			> initial					if true, makes this an initial state
			< id of new state
	*/
	int addState(bool initial = false);

	/*	Add a transition from one state to another
			> src							source state
			> dest						destination state
	*/
	void addTransition(int src, int dest);

	/*	Specify a prop. var. that must be defined
			> state						id of state
			> varNum					index of variable
			> value						true or false, the value it must have
	*/
	void addPropVar(int state, int varNum, bool value);

	/*	Add a set of accepting states
			> set							set to add
	*/
	void addAcceptSet(const BitStore &set) {
		acceptSets_.add(set);
	}

	/*	Clear automaton to freshly-constructed state
	*/
	void clear() {
		states_.clear();
		initialStates_.clear();
		acceptSets_.clear();
	}

	/*	Convert a generalized automaton to a non-generalized one.
			> dest						non-generalized automaton to construct
	*/
	void convertGeneralized(Buchi &dest);

	/*	Print automaton
	*/
	void print();

#if DEBUG
	const char *s();//Vars *v = 0);
#endif

	int nStates() const {return states_.length();}
	int nAcceptSets() const {return acceptSets_.length();}

	/*	Convert Kripke model to Buchi automaton
			> m								Kripke model to convert
			> v								Prop. Var. table (to determine # vars)
	*/
	void convertKripke(const Model &m, const Vars &v);

	/*	Calculate the product of two Buchi automata, one which
			recognizes the intersection of the respective automata.
			Neither input automata can be generalized.
			The labels of automata b1 are copied to the first 'row'
			of the product automaton.
			> b1							first buchi
			> b2							second buchi

	*/
	void calcProduct(const Buchi &b1, const Buchi &b2);

	/*	Determine if automaton is a generalized automaton
			< true if it has more than one set of accept states
	*/
	bool general() const {return acceptSets_.length() > 1;}

	/*	Determine if a state is in a particular accept set
			> state						state id
			> set							set number
			< true if so
	*/
	bool accepting(int state, int set=0) const;

	/*	Determine if language recognized by automaton is empty
			> sequence				if not empty, an infinite state sequence
												is stored here (up to the first repeated
												state)
			< true if sequence was found
	*/
	bool nonEmpty(Array<int> &sequence);

	/*	Add a label to a state, for display purposes
			> state						state number
			> label					
	*/
	void addStateLabel(int state, const String &label) {
		states_[state].label_.set(label);
	}

	const String& stateLabel(int state) const {
		return states_[state].label_;
	}

	/*	Set labels of automaton to description of propvar values
	*/
	void setPropVarLabels(Vars &v);

	/*	Reduce automaton by eliminating unreachable states
			> dest						where to store reduced automaton
	*/
	void reduce(Buchi &dest);
private:
	/*	Perform emptiness depth-first search, part 1
			> q								state to start from
			< true if infinite path found
	*/
	bool dfs1(int q);
	/*	Perform emptiness depth-first search, part 2
			> q								state to start from
			< true if infinite path found
	*/
	bool dfs2(int q);

	// dfs usage: bit is set if state is 'hashed'
	BitStore hashed_;	
	// dfs usage: bit is set if state is 'flagged'
	BitStore flagged_;
	// list of states on dfs stacks
	Array<int> dfsStack1_;
	Array<int> dfsStack2_;
	// true if state is on dfs stack
	BitStore stacked_;


	bool contradiction(int state) const {
		return contradictionStates_.get(state);
	}

	class State {
	public:
		// states this state has transitions to (this embodies '->', the
		// transition relation)
		Array<int> trans_;

		// flags indicating which prop. vars must be true (or false)
		// (if bit is set, indicates it must be true (or false))
		BitStore pvTrue_, pvFalse_;

		String label_;
	};

	// Q
	Array<State> states_;

	// I
	OrdSet initialStates_;

	// Set of sets of accepting states.  Each BitStore defines a subset
	// of the Q states which are accepting states.
	Array<BitStore> acceptSets_;

	// flags indicating whether a state has contradictions
	BitStore contradictionStates_;
};

#endif // _BUCHI
