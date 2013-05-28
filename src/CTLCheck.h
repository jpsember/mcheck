#ifndef _CTLCHECK
#define _CTLCHECK

/*	CTL Model Checking class
*/

class CTLCheck {
public:
	/*	Check a formula
			> vars						symbol table
			> model						model to check
			> f								specification (CTL formula)
			> bs							if not 0, where to store flags representing
													satisfying start states
			> printFormulas		if true, prints formulas satisfied in each state
			> showProgress		if true, displays formulas as they're marked for
													each state
	*/
	void check(Vars &vars, Model &model, Formula &f, BitStore *bs = 0,
		bool printFormulas = false, bool showProgress = false);

	CTLCheck() {
		showProgress_ = false;
	}

#if DEBUG
	const char *s() const;
#endif

private:

	Model &model() {return *model_;}

	/*	Prepare model for checking
	*/
	void prepareModel();

	void processFormula(int root);

	/*	Extract list of subformulas from the formula
			> root						current position in formula; -1 for start
	*/
	void extractSubformulas(int root = -1);

	/*	Mark state
			> state						id of state
	*/
	void markState(int state, int root);

	/*	Set flag for state
			> state						id of state
			> fi							bit to set
	*/
	void setFlag(int state, int fi) {
		BitStore &bf = stateFlags_[state];
		bf.set(fi);
	}

	/*	Read flag for state
			> state						id of state
			> fi							bit to read
	*/
	bool getFlag(int state, int fi) {
		BitStore &bf = stateFlags_[state];
		return bf.get(fi);
	}

	/*	Get child node; translate by alias if required
	*/
	int childFormula(int node, int child);

	// specification being checked, in reduced form
	Formula f_;

	// model being checked
	Model *model_;

	// aliases for subformulas, to detect identical ones;
	// if alias differs from index, it already exists
	Array<int> sfAlias_;

	// order of subformulas to check
	Array<int> sfOrder_;
	
	// flags for each state in model
	Array<BitStore> stateFlags_;

	// flags indicating which vars we've printed warnings about
	BitStore pvWarn_;

	// symbol table
	Vars *vars_;

	// true if we're to display formulas as they're marked in states
	bool showProgress_;
};

#endif // _CTLCHECK
