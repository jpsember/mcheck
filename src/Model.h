#ifndef _MODEL
#define _MODEL

/*	
	Class:  Kripke models


*/

class Model {
public:
	Model(Vars &symbols);

	/*	Clear the model to its initial empty state
	*/
	void clear();

	void parse(Scanner &scanner);

	/*	Add a state to the model
			> name						name of state to add
			< id of state
	*/
	int addState(int name);

	/*	Determine if prop. variable is set in a particular state
			> state						name of state
			> vn							id of variable
	*/
	bool propVar(int state, int vn) const;

	/*	Add a propositional variable to a state
			> state						name of state					
			> var							variable to set true (0..MAX_PROP_VARS-1)
	*/
	void addPropVar(int state, int var);

	/*	Determine number of transitions from a state
			> src							name of state
	*/
	int degree(int src) const {
		return states_[stateId(src,true)].trans_.length();
	}

	/*	Get next state
			> current					name of current state
			> neighborInd			index of neighbor in list (0..degree-1)
			< next state			name of next state
	*/
	int next(int current, int neighborInd) const {
		int currentId = stateId(current);
		return states_[currentId].trans_.itemAt(neighborInd);
	}

	/*	Add a transition from one state to another
			> src							name of source state
			> dest						name of destination state
	*/
	void addTransition(int src, int dest);

	void print() const;
#if DEBUG
	const char *s() const;
#endif

	/*	Determine number of states
	*/
	int states() const {return states_.length();}

	/*	Determine if a model has been defined.
			It must have some states.
	*/
	bool defined() {return states() > 0;}

	/*	Determine if a variable has been used in the model
			> var							id of variable
			< true if used
	*/
	bool propVarUsed(int var) const {return varsUsed_.get(var);}

	/*	Determine if a state with a particular name exists
			> s								name of state
			< true if so
	*/
	bool stateUsed(int name) {return stateId(name) >= 0;}

	/*	Add a state to the list of initial states
			> s								name of state (it must exist)
	*/
	void setInitialState(int s) {
		//ASSERT(s >= 0 && s < states());
		initialStates_.add(s);
	}

	/*	Get set of initial states
			< OrdSet containing names of initial states
	*/
	const OrdSet &initialStates() const {
		return initialStates_;
	}

	/*	Convert state id to name
			> id							id of state
			< name of state
	*/
	int stateName(int id) const;

	/*	Convert state name to id
			> name						name of state
			> mustExist				if true, and doesn't exist, throws exception
													(DEBUG only)
			< id of state, or -1 if it doesn't exist
	*/
	int stateId(int name, bool mustExist = false) const;

	const Array<int> &getNames() {return names_;}
private:

	class KState {
	public:
	#if DEBUG
		const char *s() const;
	#endif
		/*	Add a transition from this state to another
				> state						id of state to transition to
		*/
		void addTransition(int state) {
			ASSERT(state >= 0); 
			trans_.add(state);
		}

		/*	Set propositional variable true in state
				> vn							prop. var index
		*/
		void setPropVar(int vn) {
			pv_.set(vn);
		}
		// flags indicating which prop. vars are true in the state
		BitStore pv_;
		// list of state names this state can transition to.
		// Note that these are NAMES and not IDS.
		OrdSet trans_;
	};

	// array of states
	Array<KState> states_;

	// flags indicating which prop. vars are used in this model
	BitStore varsUsed_;

	Vars &symbols_;

	// initial states
	OrdSet initialStates_;

	// hash table containing strings of state names, with pointers to
	// state ids
	HashTable tbl_;

	// names associated with each state
	Array<int> names_;
	// ids of each state; these are pointed to by the hash table entries
	SArray<int> ids_;
};
#endif // _MODEL
