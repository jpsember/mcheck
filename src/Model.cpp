#include "globals.h"

Model::Model(Vars &symbols) : symbols_(symbols) {
}

void Model::parse(Scanner &scan) {


#undef p2
#define p2(a) //pr(a)

	p2(("Model::parse\n"));

	bool initDef = false;
	BitStore statesDefined;

	scan.read(TK_MODELOP);

	Token t;

	while (true) {
		scan.peek(t);
	
		if (t.type(TK_MODELCL)) {
			scan.read(t);
			break;
		}

		bool initial = false;

		if (scan.peek().type(TK_INITIALSTATE)) {		
			scan.read();
			initial = true;
		}

		scan.read(t, TK_INT);
		int num = Utils::parseInt(t.str());
		p2(("state name=%d\n",num));
		
		int id = stateId(num);
		if (id >= 0
			&& statesDefined[id]) {
			throw StringReaderException(scan.lineNumber(),"Duplicate state definition");
		}
		
		if (id < 0) {
			id = addState(num);
		}
		statesDefined.set(id);
		if (initial) {
			setInitialState(num);
			initDef = true;
		}


		bool first = true;	//	require at least one transition?
		
		while (true) {
			scan.peek(t);
			if (!first && !t.type(TK_INT)) break;
			first = false;
			scan.read(t, TK_INT);
			int num2 = Utils::parseInt(t.str());
			p2((" transit to name=%d\n",num2));
			if (stateId(num2) < 0) {
				p2(("  doesn't exist, adding it\n"));
				addState(num2);
			}
			addTransition(num,num2);
		}

		p2((" propVars...\n"));
		first = true;

		while (true) {
			scan.peek(t);
			if (!first && !t.type(TK_PROPVAR)) break;
			first = false;
			scan.read(t, TK_PROPVAR);
			if (!t.str().equals("_")) {
				//pr((" searching for %s\n",t.str().s() ));
				int varNum = symbols_.var(t.str(),true);
				//pr(("  returned %d\n",varNum));
//			int varNum = varToInt(t.str().charAt(0));
				varsUsed_.set(varNum);
			
				addPropVar(num,varNum);
			}
		}
	} 

	// verify that no undefined transitions are occurring

	pt(("verify trans, first=%d, len=%d\n",firstState_,states_.length() ));

	for (int i = 0; i < ids_.length(); i++) {
		//if (!statesUsed_.get(i)) continue;
		KState &s = states_[i];
//		if (!s.used()) continue;
//		pt((" trans length = %d\n",s.trans_.length() ));
		for (int j = 0; j < s.trans_.length(); j++) {
			int destName = s.trans_.itemAt(j);
			int dest = stateId(destName);
			if (!statesDefined[dest]) {
				String s("Transition to unknown state: state ");
				s << stateName(i) << " to " << destName << "\n";
				throw StringReaderException(scan.lineNumber(),s);
			}
		}
	}

	// if no initial states were defined, make every state
	// an initial one
	if (!initDef) {
		for (int i = 0; i < ids_.length(); i++) 
//			if (statesUsed_.get(i)) {
			initialStates_.add(stateName(i));
//			}
	}

	p2((" done parsing\n"));
	p2((" parsed:\n%s",s() ));
}

int Model::addState(int name) {
	KState st;

	ASSERT(stateId(name) < 0);

	int id = ids_.add(0);
	ids_.set(id, id);

	states_.add(st);

	names_.add(name);

	char work[20];
	Utils::intToStr(name,work);
	tbl_.set(work, &ids_[id]);

//	states_.add(st,number);
//	if (firstState_ < 0 || firstState_ > number)
//		firstState_ = number;
	return id;
}

void Model::print() const {
	Cout << "Kripke model:\n";

#define MAX_TRANS 4
#define STW 4

	for (int i = 0; i < states(); i++) {
//		if (!statesUsed_.get(i)) continue;
		int name= stateName(i);
		if (initialStates_.contains(name))
			Cout << '>';
		else
			Cout << ' ';

		Cout << fmt(name,STW) << ":";
		int j = degree(name);
		int tot = 0;
		for (int k = 0; k < j; k++) {
			if (tot++ == MAX_TRANS) {
				tot = 1;
				Cout << "\n";
				Utils::pad(STW+1);
			}
			Cout << fmt(next(name,k),STW);
		}
		Utils::pad((MAX_TRANS - tot)*STW+1);

		for (int j = 0; j < symbols_.length(); j++) {
			if (propVar(name,j))
				Cout << symbols_.var(j) << " ";
		}
		Cout << "\n";
	}
}

void Model::clear() {
	states_.clear();
	varsUsed_.clear();
	initialStates_.clear();
	tbl_.clear();
	names_.clear();
	ids_.clear();
}

#if DEBUG
const char *Model::s() const
{
	String &s = Debug::str();
	Utils::pushSink(&s);
	print();
	Utils::popSink();
	return s.chars();
}
#endif

int Model::stateName(int id) const
{
	return names_[id];
}

int Model::stateId(int name, bool mustExist) const
{
	char work[20];
	Utils::intToStr(name, work);
	int *ptr = (int *)tbl_.get(work);
	if (ptr == 0) {
#if DEBUG
		ASSERT2(!mustExist, "stateId called with undefined state");
#endif
		return -1;
	}

	return *ptr;
}

void Model::addTransition(int src, int dest) {
#undef p2
#define p2(a) //pr(a)

	p2(("Model::addTransition srcName=%d destName=%d\n",src,dest));

	int srcId = stateId(src);
	ASSERT(srcId >= 0);
	p2((" srcId=%d\n",srcId));
	states_[srcId].addTransition(dest);
	p2((" done add trans\n"));
}

void Model::addPropVar(int state, int var) {
	int id = stateId(state, true);
	states_[id].setPropVar(var);
}

bool Model::propVar(int stateName, int vn) const {
		return states_[stateId(stateName,true)].pv_.get(vn);
}
