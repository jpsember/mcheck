#include "globals.h"

int Buchi::addState(bool initial)
{
	State s;
	int n = states_.length();
	states_.add(s);
	if (initial)
		initialStates_.add(n);
	return n;
}

void Buchi::addTransition(int src, int dest)
{
	ASSERT(src >= 0 && src < states_.length()
		&& dest >= 0 && dest < states_.length());

	states_[src].trans_.add(dest);
}

void Buchi::addPropVar(int state, int varNum, bool value)
{
	State &st = states_[state];
	if (value)
		st.pvTrue_.set(varNum);
	else
		st.pvFalse_.set(varNum);
}

void Buchi::print() {
	Vars *v = Vars::globalPtr();

	//Cout << "------------- Buchi automaton ----------------------\n";
	for (int i = 0; i < states_.length(); i++) {

		State &st = states_[i];

		String s; 
		s << (initialStates_.contains(i) ? '>' : ' ');
		s << fmt(i,3) << ": ";
		
		int maxVar = maxVal(st.pvTrue_.length(), st.pvFalse_.length() );

		bool printed = false;
		for (int j = 0; j < maxVar; j++) {

			bool f0 = st.pvTrue_.get(j),
				f1 = st.pvFalse_.get(j);

			if (!(f0 || f1)) 
				continue;

			if (printed)
				s << ',';

			printed = true;

			if (f0 && f1)
					s << '*';
			else if (f1) 
				s << '!';

			if (v == 0) {
				// convert var to 'a'...'z' to avoid confusion with numbers!
				char c = '?';
				if (j < 26)
					c = (char)(j + 'a');
				s << c;
			}	else
				s << v->var(j);
		}

		s.pad(20);
		for (int j = 0; j < st.trans_.length(); j++)
			s << st.trans_[j] << ' ';

		if (st.label_.defined()) {
			s.pad(40);
			s << st.label_;
		}
		Cout << s << "\n";
	}

	for (int i = 0; i < acceptSets_.length(); i++) {
		if (i == 0)
			Cout << " --- accept sets ---\n";
		BitStore &bs = acceptSets_[i];
		Cout << " (";
		bool first = true;
		for (int j = 0; j < states_.length(); j++) {
			if (bs.get(j)) {
				if (!first)
					Cout << ' ';
				first = false;
				Cout << j;
			}
		}
		Cout << ")\n";
	}
}

#if DEBUG
const char *Buchi::s(/*Vars *v*/) {

		String &s = Debug::str();
	Utils::pushSink(&s);
	print();
	Utils::popSink();
	return s.chars();

}
#endif

void Buchi::convertGeneralized(Buchi &d)
{
#undef pt
#define pt(a) //pr(a)

	pt(("convertGeneralized automaton\n"));

	d.clear();

	// multiplier factor (n+1)
	int qm = nAcceptSets() + 1;
	int rowSize = nStates();

	// add |Q| * (n+1) states
	for (int i = 0; i < qm * rowSize; i++) {
		d.addState();
	}

	// define initial states
	for (int i = 0; i < initialStates_.length(); i++) {
		int is = initialStates_[i];
		d.initialStates_.add(is + rowSize * 0);
	}

	// define accept set
	{
		BitStore set;
		for (int i = 0; i < nStates(); i++) {
			set.set(i + rowSize * (qm-1));
		}
		d.addAcceptSet(set);
	}

	// define transitions
	for (int i = 0; i < nStates(); i++) {
		State &st = states_[i];
		for (int tr = 0; tr < st.trans_.length(); tr++) {
			int j = st.trans_[tr];
			for (int x = 0; x < qm; x++) {
				int y = -1;
				if (x < qm-1
					&& acceptSets_[x].get(j))
					y = x+1;
				else if (x == qm-1)
					y = 0;
				else
					y = x;	// not sure about this last one...

//				pt((" (q=%d, q'=%d)  -->   (<q=%d,x=%d>, <q'=%d,y=%d>)\n",i,j,i,x,j,y));

				d.addTransition(i+rowSize*x,j+rowSize*y);
			}
		}
	}

	// define propVars 
	for (int i = 0; i < nStates(); i++) {
		State &st = states_[i];
		for (int j = 0; j < qm; j++) {
			State &sd = d.states_[i+rowSize*j];
			sd.pvFalse_ = st.pvFalse_;
			sd.pvTrue_ = st.pvTrue_;

			//if (j == 0)	pt((" i=%d pvTrue set to %s\n pvFalse set to %s\n",i,sd.pvTrue_.s(true),sd.pvFalse_.s(true) ));
		}
	}
}

void Buchi::convertKripke(const Model &m, const Vars &v)
{
#if DEBUG
	if (m.initialStates().length() == 0)
		Cout << "*** Warning: converting Kripke model with no initial states\n";
#endif

	int totVars = v.length();

	clear();

	// create an initial state
	addState(true);

	for (int i = 0; i < m.states(); i++) {
		int id = addState();
		String w;
		w << m.stateName(i);
		addStateLabel(id, w);
	}

	for (int i = 0; i < m.states(); i++) {
		int ds = i+1;
		int sname = m.stateName(i);

		// add complemented/uncom. version of EVERY variable to dest state
		for (int j = 0; j < totVars; j++) {
			addPropVar(ds, j, m.propVar(sname,j));
		}

		// add transitions
		for (int j = 0; j < m.degree(sname); j++)
			addTransition(ds, 1 + m.stateId(m.next(sname,j)));
	}

	const OrdSet &mInit = m.initialStates();

	for (int i = 0; i < mInit.length(); i++)
		addTransition(0, 1 + m.stateId(mInit[i]));

	// make every state an accepting state
	BitStore set;
	for (int i = 0; i < nStates(); i++)
		set.set(i);
	addAcceptSet(set);

	//pr(("converted to kripke\n%s",s() ));

}

/*	See p. 125 of Model Checking text
*/
void Buchi::calcProduct(const Buchi &b1, const Buchi &b2)
{
#undef p2
#define p2(a) //pr(a)

	clear();

	ASSERT(!b1.general() && !b2.general());

	int q1 = b1.nStates();
	int q2 = b2.nStates();
	p2(("calcProduct, q1=%d, q2=%d\n",q1,q2));

	int rowSize = q1;
	int pageSize = rowSize * q2;

	// add |Q1| * |Q2| * 3 states
	for (int i = 0; i < q1 * q2 * 3; i++) {

		int id = addState();
//		if (i < q1) {
			addStateLabel(id, b1.stateLabel(id % q1));
//		}
	}

	// construct propVars, and determine if they are a contradiction.

	// do this only for the first set, then copy to the other sets:

	for (int i = 0; i < q1; i++) {
		State &si = b1.states_[i];
		for (int j = 0; j < q2; j++) {
			State &sj = b2.states_[j];
			for (int k = 0; k < 2; k++) 
			{
				int d0 = (i + rowSize*j);
				int di = d0 + k * pageSize;

				State &d = states_[di];
				d.label_.set(si.label_);

				if (k == 0) {
					d.pvFalse_ = si.pvFalse_;
					d.pvTrue_ = si.pvTrue_;
					d.pvFalse_.bitwiseOr(sj.pvFalse_);
					d.pvTrue_.bitwiseOr(sj.pvTrue_);
					// determine if any contradictions exist
					BitStore test;
					test = d.pvTrue_;
					test.bitwiseAnd(d.pvFalse_);
					if (test.countBits() != 0)
						contradictionStates_.set(di);
				} else {
					State &src = states_[d0];
					d.pvFalse_ = src.pvFalse_;
					d.pvTrue_ = src.pvTrue_;
					contradictionStates_.set(di, contradictionStates_[d0]);
				}
			}
		}
	}

	// define initial states
	for (int i = 0; i < b1.initialStates_.length(); i++) {
		int i1 = b1.initialStates_[i];
		for (int j = 0; j < b2.initialStates_.length(); j++) {
			int j1 = b2.initialStates_[j];
			//for (int k = 0; k < 3; k++) {
				initialStates_.add((i1  + j1 * rowSize) + 0 * pageSize);
			//}
		}
	}

	// define accept set
	{
		BitStore set;
		for (int i = 0; i < q1; i++) {
			for (int j = 0; j < q2; j++) {
				set.set((i + j*rowSize) + 2 * pageSize);
			}
		}
		addAcceptSet(set);
	}

	// define transitions
	for (int ri = 0; ri < q1; ri++) {
		State &st = b1.states_[ri];
		for (int tr = 0; tr < st.trans_.length(); tr++) {
			int rm = st.trans_[tr];

			for (int qj = 0; qj < q2; qj++) {
				State &s2 = b2.states_[qj];
				for (int t2 = 0; t2 < s2.trans_.length(); t2++) {
					int qn = s2.trans_[t2];

					if (contradictionStates_[(rm + qn * rowSize)]) {
						p2((" state r=%d and q=%d have contradictions\n",rm,qn));
					//WARN("not proc cont"); if (0)
						continue;
					}

					for (int x = 0; x < 3; x++) {
						int y = x;
						switch (x) {
							case 0:
								if (b1.accepting(rm)) y = 1;
								break;
							case 1:
								if (b2.accepting(qn)) y = 2;
								break;
							case 2:
								y = 0;
								break;
						}
						//pr((" trans <%d,%d>,%d   to  <%d,%d>,%d\n",							ri,qj,x,rm,qn,y));

						addTransition((ri + qj * rowSize)+x*pageSize,
							(rm + qn * rowSize)+y*pageSize);
					}
				}
			}
		}
	}

}

bool Buchi::accepting(int state, int set) const {
	ASSERT(set >= 0 && set < nAcceptSets());
	return acceptSets_[set].get(state);
}

bool Buchi::nonEmpty(Array<int> &seq)
{
	seq.clear();
	flagged_.clear();
	hashed_.clear();
	dfsStack1_.clear();
	dfsStack2_.clear();

	bool result = false;

	for (int i = 0; i < initialStates_.length(); i++) {
		int q0 = initialStates_[i];
		result = dfs1(q0);
		if (result) break;
	}

	if (result) {
		seq = dfsStack1_;
		for (int j = 1; j < dfsStack2_.length(); j++)
			seq.add(dfsStack2_[j]);
	}

	return result;
}

bool Buchi::dfs1(int q)
{
	bool result = false;

	stacked_.set(q);
	dfsStack1_.add(q);

	hashed_.set(q);
	State &st = states_[q];
	for (int i = 0; i < st.trans_.length(); i++) {
		int q2 = st.trans_[i];
		if (!hashed_[q2]) {
			if (dfs1(q2)) {
				result = true;
				break;
			}
		}
	}
	if (!result) {
		if (accepting(q)) 
			result = dfs2(q);
	}
	if (!result) {
		dfsStack1_.pop();
		stacked_.set(q,false);
	}

	return result;
}

bool Buchi::dfs2(int q)
{
	dfsStack2_.add(q);
	flagged_.set(q);
	State &st = states_[q];
	bool result = false;

	for (int i = 0; i < st.trans_.length(); i++) {
		int q2 = st.trans_[i];
		if (stacked_[q2]) {
			dfsStack2_.add(q2);
			result = true;
			break;
		}
		if (!flagged_[q2]) {
			result = dfs2(q2);
			if (result) break;
		}
	}
	if (!result) {
		dfsStack2_.pop();
	}
	return result;
}


void Buchi::setPropVarLabels(Vars &v)
{
	for (int j = 0; j < nStates(); j++) {

		State &s = states_[j];
		String d;
		
		int litCnt = 0;

		for (int i = 0; i < v.length(); i++) {
			if (s.pvTrue_[i] && s.pvFalse_[i]) {
				litCnt = 1;
				d.set("B");
				break;
			}

			if (s.pvTrue_[i] || s.pvFalse_[i]) {
				if (litCnt == 1) {
					d.insert(0,"(");
				}
				if (litCnt > 0)
					d << " ^ ";
				litCnt++;
				if (s.pvFalse_[i])
					d << '!';
				d << v.var(i);
			}
		}
		if (litCnt == 0) {
			d << 'T';
			litCnt++;
		}
		if (litCnt > 1)
			d << ')';

		addStateLabel(j,d);
	}
}

void Buchi::reduce(Buchi &d)
{
#undef p2
#define p2(a) //pr(a)

	p2(("Reduce:\n%s",s()));

	d.clear();

	BitStore flagged;
	Stack<int> stk;

	for (int i = 0; i < initialStates_.length(); i++) {
		stk.push(initialStates_[i]);
	}

	while (!stk.isEmpty()) {
		int s = stk.pop();
		if (flagged[s]) continue;

		if (contradictionStates_[s]) continue;

		flagged.set(s);
		State &st = states_[s];
		for (int i = 0; i < st.trans_.length(); i++) {
			stk.push(st.trans_[i]);
		}
	}

	p2(("flagged=%s\n",flagged.s(true) ));

	Array<int> newId;
	Array<int> oldId;

	int j = 0;
	for (int i = 0; i < nStates(); i++) {
		newId.add(j);
//		p2((" i=%d, new ids = %s\n",i,Utils::intArrayStr(newId)));
		if (flagged[i]) {
			oldId.add(i);
			j++;
//			p2((" i=%d, j now %d, old ids = %s\n",i,j,Utils::intArrayStr(oldId)));
		}
	}

	for (int i = 0;  i < nStates(); i++) {
		if (!flagged[i]) continue;
		State &orig = states_[i];
		State s = orig;
		Array<int> &t = s.trans_;
		for (int j = 0; j < t.length(); j++) {
			int dest = t[j];
			if (!flagged[dest]) continue;
			t.set(j, newId[dest]);
		}
		d.states_.add(s);
	}

	for (int i = 0; i < initialStates_.length(); i++) {
		d.initialStates_.add(newId[initialStates_[i]]);
	}

	for (int j = 0; j < acceptSets_.length(); j++) {
		BitStore &src = acceptSets_[j];
		BitStore set;

		for (int i = 0; i < nStates(); i++) {
			if (!flagged[i]) continue;
			if (!src[i]) continue;
			int k = newId[i];
			set.set(k);
		}
		d.acceptSets_.add(set);
	}

	p2(("Reduced:\n%s",d.s()));
}
