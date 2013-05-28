#include "globals.h"

/*	Find the point at which a sequence of integers starts to repeat.
		If repeat sequence is found, last item in list is deleted.
		> seq								sequence
		< index of start of repeating subsequence, or -1 if none found
*/
static int repeatPoint(Array<int> &seq)
{
#if 1
	int rep = -1;
	for (int i = seq.length()-2; i >= 0; i--) {
		int s = seq[i];
		if (s == seq.last()) {
			rep = i;
			seq.pop();
			break;
		}
	}
#else
	// find repeat point
	BitStore bs;
	int rep = -1;
	for (int i = seq.length()-1; i >= 0; i--) {
		int s = seq[i];
		if (bs[s]) {
			rep = i;
			break;
		}
		bs.set(s);
	}
#endif
	return rep;
}

void LTLCheck::constructAutomaton(Formula &f, bool negate, Buchi &b)
{
#undef pt
#define pt(a) //pr(a)

	if (!f.isLTL())
		throw ParseException("Cannot construct automaton for non-LTL formulas");

	if (negate) {
		f_ = f.negate();
	}
	else
		f_ = f;
	
	nodes_.clear();

	model_ = 0;
	pvWarn_.clear();
	nForest_.clear();
	nodeList_.clear();
	nodes_.clear();
	initNode_ = -1;

	// reduce formula to minimal set of connectives
	f_.reduce();
	pt(("LTLCheck, checking formula\n    %s\n==> %s\n",
		f.s(-1,true),f_.s(-1,true)));

	createGraph();

	Buchi bg;
	constructBuchi(bg);

	Buchi bg2;
	bg.convertGeneralized(bg2);
	bg2.reduce(b);
//	bg.convertGeneralized(b);

}

void LTLCheck::check(Model &model, Formula &f)
{
#undef pt
#define pt(a) //pr(a)

	Buchi ngb;
	constructAutomaton(f, true, ngb);
	if (option(OPT_PRINTBUCHI)) {
		Cout << "Formula automaton:\n";
		ngb.print();
		Cout << "\n";
	}

	ASSERT(model.defined() );
	model_ = &model;

	// convert model to Buchi automaton
	Buchi bModel;
	bModel.convertKripke(model,*vars_);
	pt((" buchi for kripke model:\n%s",bModel.s() ));
	if (option(OPT_PRINTBUCHI)) {
		Cout << "Model automaton:\n";
		bModel.print();
		Cout << "\n";
	}

	Buchi bProd;
	Buchi bProd0;
	bProd0.calcProduct(bModel,ngb);
#if 1	// reduce product aut?
	bProd0.reduce(bProd);
#else
	bProd = bProd0;
#endif

	if (option(OPT_PRINTBUCHI)) {
		Cout << "Product automaton:\n";
		bProd.print();
		Cout << "\n";
	}

	pt(("product:\n%s",prod.s() ));

	Array<int> seq;
	if (bProd.nonEmpty(seq)) {
		//Utils::printIntArray(seq,"Sequence");
		Cout << "Not satisfied; counterexample:\n";
		int rep = repeatPoint(seq);
		
		String w;
		w << "  ";
		for (int i = 1; i < seq.length(); i++) {
			int s = seq[i];
//			int s = seq[i] % bModel.nStates();
			if (i > 1) w << ' ';
			if (i == rep)
				w << "{";
			//Cout << s << ":";
			w << bProd.stateLabel(s);
		}
		if (rep >= 0)
			w << "}*";
		if (!option(OPT_PRINTFULLSEQ))
			w.truncate(75,true);
		Cout << w << "\n";
	} else
		Cout << "Satisfied.\n";
	Cout << "\n";
}

#if DEBUG
const char *LTLCheck::Node::s() 
{
	String &s = Debug::str();
	Utils::pushSink(&s);
	Cout << "----------- LTL Node ----------------\n";
	Cout << "  in:" << incoming.debInfo().chars() << "\n";
	for (int i = 0; i < 3; i++) {
		static const char *names[] = {" old"," new","next"};
		OrdSet &lst = (i == 0 ? fOld : (i == 1 ? fNew : fNext));
		Cout << names[i] << ":\n";
		for (int j = 0; j < lst.length(); j++) {
			int root = lst.itemAt(j);
			Cout << "     " << fmt(root,3) << ": ";
			Formula::printNode(root); Cout << "\n";
		}
	}
	Cout << "\n";
	Utils::popSink();
	return s.chars();

}
#endif

int LTLCheck::newNode()
{
	int node = nForest_.newNode();
	Node n;
	nodes_.add(n,node);
	return node;
}

void LTLCheck::printState(int nodeInd, bool skipNew)
{
		int pamt = 24;
		String s;
		Utils::pushSink(&s);

		Node &n = node(nodeInd);
		Cout << fmt(nodeInd,2) << ": ";
		for (int j = 0; j < n.incoming.length(); j++)
			Cout << fmt(n.incoming.itemAt(j),2) << ' ';

		int p = 24;
		s.pad(p); p += pamt;
		if (!skipNew) {
			Cout << "  New:";
			for (int j = 0; j < n.fNew.length(); j++)
				Cout << fmt(n.fNew.itemAt(j),2) << ' ';
			s.pad(p); p += pamt;
		}

		Cout << "  Old:";
		for (int j = 0; j < n.fOld.length(); j++)
				Cout << fmt(n.fOld.itemAt(j),2) << ' ';
		s.pad(p); p += pamt;
		Cout << "  Next:";
		for (int j = 0; j < n.fNext.length(); j++)
				Cout << fmt(n.fNext.itemAt(j),2) << ' ';
		s.pad(p); p += pamt;
		Utils::popSink();
		Cout << s << "\n";
}

void LTLCheck::printStateSet(bool skipNew)
{
	Cout << "------------- States ------------------------------------\n";
	for (int i = 0; i <= nodeList_.lastItem(); i++) {
		printState(nodeList_[i], skipNew);
	}
	// print the formulas associated with each node
	Cout << "------------- Formulas ----------------------------------\n";
	Array<int> list;
	f_.forest().getNodeList(f_.root(),list);
	for (int i = 0; i < list.length(); i++) {
		Cout << fmt(list[i],3) << ": ";
		f_.print(list[i],false);
		Cout << "\n";
	}
	Cout << "---------------------------------------------------------\n\n";
}

void LTLCheck::expand(int q)
{
#undef p2
#define p2(a) //pr(a)

	// the numbers correspond to line numbers from the 'simple on-the-fly...' paper

	Node &qr = node(q);
	//p2(("\n\nexpand q=%d, list=%s%s",q,nodeList_.s(Utils::printInt),qr.s() ));
	p2(("\n\nexpand q=%d\n",q));
	//printState(q);	printStateSet();

// 4
	if (qr.fNew.isEmpty()) {
		p2((" q.new is empty...\n"));

		// 5
		// skip the initial node
		for (int i = initNode_+1; i <= nodeList_.lastItem(); i++) {
			Node &t = node(nodeList_[i]);
			if (t.fOld.equals(qr.fOld)
				&& t.fNext.equals(qr.fNext)
			) {
				// 6
				p2(("  found node r=%d...\n",nodeList_[i]));
				t.incoming.include(qr.incoming);
				return;
			}
		}
		p2(("  ...no node r found\n"));
		int n2 = newNode();
		Node &n2p = node(n2);
		n2p.incoming.add(q);
		n2p.fNew = qr.fNext;

		p2(("  ...q' = %d\n",n2));
		nodeList_.add(q);
		//p2(("  ...just added node to list\n"));printStateSet();

		p2(("  ...expanding recursively\n"));
		expand(n2);
		return;

	}

// 12
	
		// New(q) is not empty
		int last = qr.fNew.length() - 1;
		int e = qr.fNew.itemAt(last);
		qr.fNew.removeArrayItem(last);

		p2((" e=%d, removed from q.new\n",e));

		// 13.5: not in original paper; more efficient to test for
		//  'old' added again (modifying lines 22 and 25)
		if (qr.fOld.contains(e)) {
			p2(("  q.old contains e, expanding q without e\n"));
			expand(q);
			return;
		}

		int litCode = Formula::getLiteralCode(e);
		if (litCode != 0) {
// 15			
			p2((" ...literal code=%d\n",litCode));

			// is e False, or is its negation in q.old?
			if (litCode == -1) {
				p2(("  ...BOTTOM\n"));
				return;	// BOTTOM or FALSE
			}

			// see if negative of this exists in q.old.
			for (int i = 0; i < qr.fOld.length(); i++) {
				int litCode2 = Formula::getLiteralCode(qr.fOld.itemAt(i));
				if (litCode2 == -litCode) {
					p2(("  ...found negation\n"));
					return;
				}
			}

// 18
			// add e to q.old
			if (litCode != 1) {	// don't add TRUE
				p2((" ...adding e=%d to q.old\n",e));
				qr.fOld.add(e);
			}
			expand(q);
			return;
		}

// 15
		int etype = Formula::nType(e);
		p2(("  etype=%d\n",etype));
		switch (etype) {
			case TK_U:
			case TK_R:
			case TK_OR:
				{
					{
						int id1;
						Node &n1 = newNode(id1);

						n1.incoming = qr.incoming;

						n1.fNew = qr.fNew;
						switch (etype) {
						case TK_U:
						case TK_OR:
							n1.fNew.add(f_.child(e,0));
							break;
						case TK_R:
							n1.fNew.add(f_.child(e,1));
							break;
						}

						n1.fOld = qr.fOld;
						n1.fOld.add(e);

						n1.fNext = qr.fNext;
						switch (etype) {
						case TK_U:
						case TK_R:
							n1.fNext.add(e);
							break;
						}
						expand(id1);
					}

					{
						int id2;
						Node &n2 = newNode(id2);

						n2.incoming = qr.incoming;

						n2.fNew = qr.fNew;
						switch (etype) {
						case TK_U:
						case TK_OR:
							n2.fNew.add(f_.child(e,1));
							break;
						case TK_R:
							n2.fNew.add(f_.child(e,0));
							n2.fNew.add(f_.child(e,1));
							break;
						}

						n2.fOld = qr.fOld;
						n2.fOld.add(e);

						n2.fNext = qr.fNext;
						expand(id2);
					}
				}
				break;

			case TK_AND:
					{
						int id1;
						Node &n1 = newNode(id1);

						n1.incoming = qr.incoming;

						n1.fNew = qr.fNew;
						n1.fNew.add(f_.child(e,0));
						n1.fNew.add(f_.child(e,1));

						n1.fOld = qr.fOld;
						n1.fOld.add(e);

						n1.fNext = qr.fNext;
						expand(id1);
					}
				break;

			case TK_X:
					{
						int id1;
						Node &n1 = newNode(id1);

						n1.incoming = qr.incoming;

						n1.fNew = qr.fNew;

						n1.fOld = qr.fOld;
						n1.fOld.add(e);

						n1.fNext = qr.fNext;
						n1.fNext.add(f_.child(e,0));

						expand(id1);
					}
				break;
#if DEBUG
			default:
				ASSERT(false); // ***shouldn't come here
				break;
#endif
		}
}

void LTLCheck::createGraph()
{
#undef pt
#define pt(a) //pr(a)
													pt(("createGraph\n"));

	nodeList_.clear();
	newNode(initNode_); 
	nodeList_.add(initNode_);

	int id;
	Node &np = newNode(id);
	np.incoming.add(initNode_);
	np.fNew.add(f_.root());
													pt(("first node:\n%s",np.s() ));

//	Cout << "CreateGraph, initial node created\n"; printStateSet();
	expand(id);

	//WARN("...printing state sets...\n");	printStateSet(true);
	if (option(OPT_PRINTSTATES))
		printStateSet(true);
}

void LTLCheck::constructBuchi(Buchi &b)
{
#undef pt
#define pt(a) //pr(a)

	pt(("constructBuchi\n"));

	b.clear();

	// construct a translation table for existing state numbers to
	// buchi state numbers
	Array<int> newNums;
	for (int i = 0; i <= nodeList_.lastItem(); i++) {
		int n = nodeList_[i];
		pt((" old state = %d, new is %d\n",n,i));
		newNums.add(i, n);
	}

	// add states
	for (int i = 0; i <= nodeList_.lastItem(); i++) {
		int s = b.addState(i == 0);

		// add flags for prop. vars
		Node &nd = node(nodeList_[i]);
		for (int j = 0; j < nd.fOld.length(); j++) {
			int f = nd.fOld.itemAt(j);
			int code = Formula::getLiteralCode(f);
			bool neg = (code < 0);
			code = abs(code);
			if (code < 2) continue;
			b.addPropVar(s,code-2,!neg);
		}
			
	}

	// add transitions 
	for (int i = 0; i <= nodeList_.lastItem(); i++) {
		int destNum = nodeList_[i];
		Node &dest = node(destNum);
		//pt((" dest.incoming=%s\n",dest.incoming.debInfo().s()));

		for (int j = 0; j < dest.incoming.length(); j++) {
			int srcNum = dest.incoming.itemAt(j);
			b.addTransition(newNums[srcNum],newNums[destNum]);
		}
	}

	// add accepting sets for (a U b) formulas

	{
		Array<int> nList;
		f_.forest().getNodeList(f_.root(),nList);

		for (int i = 0; i < nList.length(); i++) {
			int root = nList[i];

			if (f_.nType(root) != TK_U) continue;
			int childB = f_.child(root,1);

			pt((" processing formula %s\n",f_.s(root,true) ));

			BitStore set;
			for (int j = 0; j <= nodeList_.lastItem(); j++) {
				int si = nodeList_[j];
				Node &src = node(si);

				// see if (a U b) does not exist in src.old
				//  or b IS in src.old
				if (!src.fOld.contains(root)
					|| src.fOld.contains(childB)
				) {
					set.set(newNums[si]);
				}
			}
			b.addAcceptSet(set);
		}
	}

	pt(("%s",b.s(vars_) ));
}


void LTLCheck::compare(Formula &f1, Formula &f2, bool printReduced)
{
#undef p2
#define p2(a) //pr(a)

	Cout << "Comparing: ";
	f1.print(-1,false); 
	Cout << "\n";
	if (printReduced) {
		Utils::pad(11);
		f1.printReduced();
	}

	Cout << "\n     with: ";
	f2.print(-1,false); 
	Cout << "\n";
	if (printReduced) {
		Utils::pad(11);
		f2.printReduced();
	}
	Cout << "\n";

	Buchi b1, b2;

	bool equiv = true;

	for (int pass = 0; pass < 2; pass++) {

		if (pass == 0) {
			if (option(OPT_PRINTSTATES))
				Cout << "First automaton:\n";
			constructAutomaton(f1, false, b1);
			if (option(OPT_PRINTSTATES))
				Cout << "Second automaton:\n";
			constructAutomaton(f2, true, b2);
		} else {
			if (option(OPT_PRINTSTATES))
				Cout << "First automaton:\n";
			constructAutomaton(f2, false, b1);
			if (option(OPT_PRINTSTATES))
				Cout << "Second automaton:\n";
			constructAutomaton(f1, true, b2);
		}

		// label first automaton with description of its prop.var values
		b1.setPropVarLabels(*vars_);

		if (option(OPT_PRINTBUCHI)) {
			Cout << "First automaton:\n";
			b1.print();
			Cout << "\n";
			Cout << "Second automaton:\n";
			b2.print();
			Cout << "\n";
		}

		p2(("first automaton:\n%s",b1.s() ));
		p2(("second automaton:\n%s",b2.s() ));


		Buchi prod;
#if 1
		Buchi prod0;
		prod0.calcProduct(b1,b2);
		prod0.reduce(prod);
#else
		prod.calcProduct(b1,b2);
#endif
		p2(("product:\n%s",prod.s() ));
		if (option(OPT_PRINTBUCHI|OPT_PRINTSTATES)) {
			Cout << "Product automaton:\n";
			prod.print();
			Cout << "\n";
		}

		Array<int> seq;

		if (prod.nonEmpty(seq)) {
			//pr(("seq length=%d\n",seq.length()));

			if (equiv) {
				equiv = false;
				Cout << "Not equivalent.\n";
			}

			String buff;
			Utils::pushSink(&buff);
			{
				Cout << "\n";
				Cout << (pass == 0 ? " first" : "second") << " allows: ";

				// find repeat point
#if 0
				int rep = repeatPoint(seq);
				BitStore bs;
				int rep = -1;
				for (int i = seq.length()-1; i > 0; i--) {
					int s = seq[i];
					if (bs[s]) {
						rep = i;
						break;
					}
					bs.set(s);
				}
#endif
				int rep = repeatPoint(seq);

				for (int i = 1; i < seq.length(); i++) {
					int s = seq[i];
					const String &str = prod.stateLabel(s);

					if (i > 1) Cout << ' ';

					if (i == rep) {
						Cout << "{";
					}
					Cout << str;
				}
				Cout << "}*\n";
			}
			Utils::popSink();
			if (!option(OPT_PRINTFULLSEQ)) {
				buff.truncate(75,true);
				buff << '\n';
			}
			Cout << buff;
		}
	}
	if (equiv)
		Cout << "Equivalent.\n";
	Cout << "\n";
}

