#include "globals.h"

void CTLCheck::check(Vars &v, Model &m, Formula &f, BitStore *bs,
										 bool printFormulas, bool showProgress)
{
#undef pt
#define pt(a) //pr(a)

	f_ = f;
	model_ = &m;
	vars_ = &v;

	sfAlias_.clear();
	stateFlags_.clear();
	sfOrder_.clear();
	pvWarn_.clear();

	showProgress_ = showProgress;

	if (bs)
		bs->clear();

	ASSERT(f_.isCTL());

	prepareModel();
	if (model().defined()) {

		// reduce formula to minimal set of connectives
		f_.reduce();
		pt(("CTLCheck, checking formula\n    %s\n==> %s\n",f.s(),f_.s()));
		extractSubformulas();

		for (int i = 0; i < sfOrder_.length(); i++)
			processFormula(sfOrder_[i]);

		if (printFormulas) {
			for (int i = 0; i < model().states(); i++) {
				int name = model().stateName(i);
				Cout << "State #" << name << ":\n";
				for (int j = 0; j < sfOrder_.length(); j++) {
					int form = sfOrder_[j];
					if (getFlag(i,form)) {
						Cout << "  ";
						f_.print(form,false);
						Cout << "\n";
					}
				}
				Cout << "\n";
			}
		}

		if (bs) {
			int satFormula = sfOrder_.last();

			for (int i = 0; i < model().states(); i++)
				if (getFlag(i,satFormula))
					bs->set(i);
			pt((" satisfying states: %s\n",bs->s() ));
		}
	}
}

void CTLCheck::prepareModel()
{
	stateFlags_.clear();
	Model &m = model();

	// initialize state flags to empty
	{
		BitStore store;
		for (int i = 0; i < m.states(); i++)
			stateFlags_.add(store);
	}
}

void CTLCheck::extractSubformulas(int root)
{
#undef pt
#define pt(a) //pr(a)

	if (root < 0)
		root = f_.root();
	if (root < 0) return;
	pt(("extract root=%3d, [%s]\n",root,f_.s(root,true) ));

	// determine if this formula already exists in list
	for (int i = 0; i < sfOrder_.length(); i++) {
		if (sfOrder_[i] == root) {
			pt((" ...already exists...\n"));
			return;
		}
	}

	// process child nodes first
	int nc = f_.nChildren(root);
	for (int i = 0; i < nc; i++) {
		extractSubformulas(f_.child(root,i));
	}

	int alias = root;
	// determine if subformula is identical to an existing one
	for (int i = 0; i < sfOrder_.length(); i++) {
		if (Formula::equal(root, sfOrder_[i])) {
			alias = sfOrder_[i];
			break;
		}
	}

	sfAlias_.add(alias,root);

	// add root index to list describing order we'll do the checking in;
	// don't check aliased formulas.
	if (alias == root) {
		pt((" orig %3d [%s]\n",root,f_.s(root,true)));
		sfOrder_.add(root);
	}
}

int CTLCheck::childFormula(int node, int child)
{
	int c = f_.child(node,child);
	//pr(("childFormula node=%d child=%d c=%d alias=%d\n",node,child,c,sfAlias_[c]));
	return sfAlias_[c];
}

void CTLCheck::markState(int state, int root) {
	setFlag(state,root);
	if (showProgress_) {
		Cout << "  +" << fmt(model_->stateName(state),2) << ": ";
		f_.print(root,false);
		Cout << "\n";
	}
}

void CTLCheck::processFormula(int root)
{
#undef pt
#define pt(a) //pr(a)

	pt(("processFormula root=%d formula=%s\n",root,f_.s(root) ));

	Model &m = model();
	Token &t = f_.token(root);
	int type = f_.nType(root);
	//	pt((" type=%d, str=%s\n",type,Formula::ts(t) ));
	//pt((" %s\n",Formula::ts(t) ));

	switch (type) {
		case TK_PROPVAR: {
			int var = vars_->var(t.str(),true);
			//int var = Model::varToInt(t.str().charAt(0));
			if (!m.propVarUsed(var)) {
				if (!pvWarn_.get(var)) {
					pvWarn_.set(var);
					Cout << "Warning: Variable '" << t.str() << "' not used in model\n";
				}
			}
			for (int i = 0; i < m.states(); i++) {

				if (m.propVar(m.stateName(i),var)) {
					//pt((" setting flag %d, %d\n",i,root));
					pt(("     +%d\n",i));
//				pr(("  +#%2d: %s\n",i,f_.s(root,true)));
//
					pt((" setting flag %d, %d\n",i,root));
					markState(i,root);
	//				setFlag(i,root);
				}
			}
			}
			break;
		case TK_BOTTOM: 
			break;
		case TK_NEGATION:
			{
				int child = childFormula(root,0);
				for (int i = 0; i < m.states(); i++) {
					if (!getFlag(i,child)) {
						pt((" flag %d, child %d not set\n",i,child));
						pt(("     +%d\n",i));
//						pr(("  +#%2d: %s\n",i,f_.s(root,true)));
					markState(i,root);
	//					setFlag(i,root);
					}
				}
			}
			break;
		case TK_AND:
			{
				int ca = childFormula(root,0),
					cb = childFormula(root,1);

				for (int i = 0; i < m.states(); i++) {
					if (getFlag(i,ca) && getFlag(i,cb)) {
						pt(("     +%d\n",i));
//						pr(("  +#%2d: %s\n",i,f_.s(root,true)));
							markState(i,root);
//				setFlag(i,root);
					}
				}
			}
			break;
		case TK_EX:
			{
				int c = childFormula(root,0);

				for (int i = 0; i < m.states(); i++) {
					int in = m.stateName(i);
					for (int j = m.degree(in)-1; j >= 0; j--) {
						int xs = m.next(in,j);
						if (getFlag(m.stateId(xs),c)) {
							pt(("     +%d\n",i));
							markState(i,root);
//							setFlag(i,root);
							//pr(("  +#%2d: %s\n",i,f_.s(root,true)));
							break;
						}
					}
				}
			}
			break;
		case TK_AF:
			{
				int c = childFormula(root,0);

				bool changed = true;
				while (changed) {
					changed = false;
					for (int i = 0; i < m.states(); i++) {
						int iName = m.stateName(i);
						if (getFlag(i,root)) continue;
						if (getFlag(i,c)) {
							pt(("     +%d\n",i));
							changed = true;
							markState(i,root);
//pr(("  +#%2d: %s\n",i,f_.s(root,true)));
//							setFlag(i,root);
							continue;
						}

						bool allNb = true;
						for (int j = m.degree(iName)-1; j >= 0; j--) {
							int xs = m.next(iName,j);
							if (!getFlag(m.stateId(xs),root)) {
								allNb = false; break;
							}
						}
						if (allNb) {
							pt(("     +%d\n",i));
							changed = true;
						markState(i,root);
						//pr(("  +#%2d: %s\n",i,f_.s(root,true)));
						//	setFlag(i,root);
						}
					}
				}
			}
			break;
		case TK_EU:
			{
#undef pt
#define pt(a) //pr(a)
	pt(("processFormula root=%d [%s]\n",root,f_.s(root,true) ));

				int ca = childFormula(root,0);
				int cb = childFormula(root,1);

				bool changed = true;
				while (changed) {
					changed = false;
					for (int i = 0; i < m.states(); i++) {
						int in = m.stateName(i);

						if (getFlag(i,root)) continue;
						if (getFlag(i,cb)) {
							pt(("     +%d (second part true)\n",i));
							changed = true;
							markState(i,root);
//							setFlag(i,root);
	//						pr(("  +#%2d: %s\n",i,f_.s(root,true)));
							continue;
						}

						// first part must be true in this state
						if (!getFlag(i,ca)) continue;

						// check if second part is true in any successor
						for (int j = m.degree(in)-1; j >= 0; j--) {
							int xs = m.next(in,j);
							if (getFlag(m.stateId(xs, true),root)) {
								pt(("     +%d (neighbor %d has first true)\n",i,xs));
								changed = true;
								markState(i,root);
								break;
							}
						}
					}
				}
			}
			break;
		}
}

