#include "Headers.h"
#include "Files.h"
#include "DFA.h"

#if DEBUG // Only for development

void DFA::clearTransition(int state, int symbol)
{
	if (state < 0 || state >= numStates())
		throw Exception("Illegal state number in DFA::clearTransition");
	getState(state).clearTransition(symbol);
}

void DFA::write(Sink &w)
{
	w << (int)VERSION << (short)numStates() << (short)startState_
		<< (short)tokenNames_.length();

	for (int i = 0; i < numStates(); i++) {
		DFA::DFAState &s = getState(i);
		s.write(w);
	}

	for (int i = 0; i < tokenNames_.length(); i++)
		w << tokenNames_.itemAt(i);
}

void DFA::DFAState::write(Sink &w) const
{
#undef pt
#define pt(a) //pr(a)
	pt(("DFAState::write\n"));

	w << flags_;
	Array<int> syms;
	getSymbols(syms);
	w << (short)syms.length();
	pt((" flags=%x #syms=%d\n",flags_,syms.length() ));

	for (int i = 0; i < syms.length(); i++) {
		int s = syms.itemAt(i);
		int trans = getTransitionState(s);
		w << (char)s << (char)trans;
		pt((" sym %d, trans %d\n",(byte)s, (byte)trans));
	}
}

void DFA::writeText(Sink &dest)
{
//	dest.clear();
	String work;
	for (int i = 0; i < numStates(); i++) {
		DFAState &s = getState(i);
		if (s.finalFlag())
			dest << i << "\n";
		for (int j = MIN_ASCII; j <= MAX_ASCII; j++) {
			char c = (char)j;
			int dState = getTransitionState(i,c);
			if (dState >= 0) {
				String cStr;
				const char esc[] = {
					7,'a',
					8,'b',
					9,'t',
					10,'n',
					11,'v',
					12,'f',
					13,'r',
					34,'"',
					//39,'\'',
					92,'\\',
					'A',
				};
				for (int j = 0; esc[j] != 'A'; j+=2) {
					if (c == esc[j]) {
						cStr.append('\\');
						cStr.append(esc[j+1]);
						break;
					}
				}
				if (cStr.length() == 0) {
					if (c <= 32) {
						Utils::toHex(c,2,work);
						cStr.append(work);
					} else
						cStr.append(c);
				}

				dest << i 
						<< " " << dState 
						<< " " 
						<< cStr
						<< "\n";
			}
		}
	}

#if DEBUG && 0
		// shuffle the order of the lines in this string
		// (but keep the first line, which is a transition from state 0)
		StringArray sa;
		dest.split(sa,'\n');
		Utils::srand(1965);
		for (int i = 1; i < sa.length(); i++) {
			int j = Utils::rand(sa.length());
			if (j == 0 || j == i) continue;
			// swap i with j
			String temp(sa.itemAt(i));
			sa.set(i,sa.itemAt(j));
			sa.set(j,temp);
		}
		String::join(sa, "\n", dest);
		// Make sure we end the last line with a \n
		dest.append("\n");
#endif
}

#if 0	// This is some old code which has now been replaced I think...
/*	Main program
*/
int DFA_main(CmdArgs &ca) {

	int error = false;

#if DEBUG

	// if the input files for testing the 379 assignments don't
	// exist, create them.

	if (!FileObject::exists("input1.txt")) 
		DFA::constructTestFile("corpus.txt","input1.txt");
	if (!FileObject::exists("input2.txt")) 
		DFA::constructTestFile("corpus2.txt","input2.txt");
#endif

	try {

		// construct a CmdArgs object for parsing the
		// command line arguments

		String dfaPath;
		String testString;

#if DEBUG
		// if running from IDE, no cmd line arguments will exist;
		// add some to test the 379 test data
		if (!ca.hasNext()) {
#if 1
			// A man moves to town and hunts around for an apartment to rent
			dfaPath.set("input1.txt");
			testString.set("townanapartmentand");
#else
			// <- == - -> = :: ~ : 
			dfaPath.set("input2.txt");
			testString.set("-><--=::==~<-->");
#endif
		} else
#endif

//		try {
			dfaPath = ca.nextValue();
			testString = ca.nextValue();
			ca.done();
//		} catch (CmdArgException &e) {
			//REF(e);
			//throw Exception("Usage: dfa <dfa definition path> <test string>");
		//}

		DFA dfa;
		TextReader r(dfaPath);
		dfa.readText(r);
//		dfa.readFromFile(dfaPath);

		// now apply the DFA to extract tokens from the test string
		int pos = 0;
		while (pos < testString.length()) {
			int tokenLength;
			if (dfa.recognize(testString, pos, tokenLength) < 0) {
				cout << "illegal token\n";
				break;
			}

			cout << "dfa: state=" << dfa.state() 
				<< " token=" << testString.subStr(pos,tokenLength) << "\n";

			pos += tokenLength;
		}

	} catch (Exception &e) {
			cout << e;
			error = true;
	}
	return error ? 1 : 0;
}
#endif

#if DEBUG
String DFA::dumpGroups(Array<Group> groups) const {
	String s;

	s << "Groups:\n";

	for (int i = 0; i < groups.length(); i++) {
		Group &g = groups.itemAt(i);
		s << fmt(i,3) << ": ";
		for (int j = 0; j < g.length(); j++)
			s << g.itemAt(j) << " ";
		s << "\n";
	}
	return s;

}

void Test_DFA() {
// If 0, outputs to screen, no test file written
// If 1, compares (internal) output to test file on disk,
//		report error if different
#define TO_STR 0

	int tests = 1;
	int t0 = 0, t1 = tests-1;
//	t0 = 1; t1 =t0;

	for (int t = t0; t <= t1; t++) {
String r;//		StringStream r;

		Utils::pushSink(TO_STR ? &r : 0);

		switch (t) {
		case 0:
			{
				Cout << "test DFA\n";
				DFA dfa;

				static int trans[] = {
					0,'a',1,
						1,'a',1,
						1,'b',2,
						2,'a',2,
						2,'b',2,
						0,'b',3,
						-1
				};
				dfa.addState(0);
				dfa.addState(1);
				dfa.addState(2);
				dfa.addState(3);
				dfa.makeStateFinal(1);
				dfa.makeStateFinal(2);
				dfa.setStartState(0);

				for (int i = 0; trans[i] >= 0; i+=3)
					dfa.addTransition(trans[i+0],trans[i+2],trans[i+1]);

				Cout << dfa.debInfo();

				DFA min;
				dfa.minimize(min);
				Cout << min.debInfo();
			}
			break;
		}
		Utils::popSink();
#if TO_STR
		String path("CFG");
		path << t;
		Test(path.chars(), r);
#endif
	}
}
#endif

void DFA::min_addSinkTransitions(OrdSet &symbols, int sinkState) {
	for (int i = 0; i < numStates(); i++) {
		DFAState &s = getState(i);
		for (int j = 0; j < symbols.length(); j++) {
			int sym = symbols.itemAt(j);
			if (s.getTransitionState(sym) < 0)
				s.addTransition(sinkState, sym);
		}
	}
}

void DFA::min_removeSinkTransitions(OrdSet &symbols, int sinkState)
{
#undef pt
#define pt(a) //pr(a)

	pt(("min_removeSinkTransitions sinkState=%d\n",sinkState));

	// Remove all transitions to the 'sink' state
	for (int i = 0; i < numStates(); i++) {
		DFAState &s = getState(i);
		for (int j = 0; j < symbols.length(); j++) {
			int sym = symbols.itemAt(j);
			if (s.getTransitionState(sym) == sinkState) {
				pt((" clearing transition %d -> %d on %d\n",i,sinkState,sym));
				s.clearTransition(sym);
			}
		}
	}
}

void DFA::min_addInitialGroups(Array<Group> &groups) const 
{
#if !AUGMENT
	// add these initial groups:
	//	0) nonfinal states
	//	1+0..1+k-1) final state with code #k

	Group nonFinal;
	groups.add(nonFinal);
	for (int i = 0; i < numStates(); i++) {
		DFAState &s = getState(i);
		if (s.finalFlag()) {
			Group final;
			final.add(i);
			s.setGroup(groups.length());
			groups.add(final);
		}
		else {
			groups.itemAt(0).add(i);
			s.setGroup(0);
		}
	}
#else
	// add two initial groups, one with nonfinal states, the other
	// with final states; set group index for each state

	Group nonFinal, final;
	for (int i = 0; i < numStates(); i++) {
		DFAState &s = getState(i);
		if (s.finalFlag()) {
			final.add(i);
			s.setGroup(0);
		}
		else {
			nonFinal.add(i);
			s.setGroup(1);
		}
	}
	groups.add(final);
	groups.add(nonFinal);
#endif
}

bool DFA::min_findGroupToSplit(OrdSet &symbols, Array<Group> &groups,
															 int &splitGroup, int &splitSym, int &splitDest)
{
#undef pt
#define pt(a) //pr(a)

	pt(("min_findGroupToSplit\n%s\n",dumpGroups(groups).chars() ));
	for (int g0 = 0; g0 < groups.length(); g0++) {
		int g = Utils::mod(g0 + min_counter_, groups.length());
		
		pt((" group #%d\n",g));

		Group &gr = groups.itemAt(g);
		
		int s0 = gr.itemAt(0);
		DFAState &sp0 = getState(s0);

		for (int j = 1; j < gr.length(); j++) {
			for (int i = 0; i < symbols.length(); i++) {
				int sym = symbols.itemAt(i);

				pt(("  testing symbol %d\n",sym));

				int sj = gr.itemAt(j);
				DFAState &spj = getState(sj);

				int t0 = sp0.getTransitionState(sym),
					tj = spj.getTransitionState(sym);

				pt(("   s0=%d, sj=%d, t0=%d, tj=%d\n",s0,sj,t0,tj));
				DFAState &tp0 = getState(t0),
					tpj = getState(tj);

				if (tp0.group() != tpj.group()) {

					pt(("     ...found, on symbol %d state %d-->%d(grp %d), while %d-->%d(grp %d)\n",
						sym, s0, t0, tp0.group(),
								sj, tj, tpj.group() ));

					splitGroup = g;
					splitSym = sym;
					splitDest = tp0.group();
					pt(("        ... setting splitGroup=%d, sym=%d, dest=%d\n",
						splitGroup,splitSym,splitDest));
					// resume with a different group next time, 
					// not always group #0
					min_counter_ += g0;
					return true;
				}
			}
		}
	}
	return false;
}

void DFA::min_splitGroup(GroupList &groups, int splitGroup,
	int splitSym, int splitDest) 
{
#undef pt
#define pt(a) //pr(a)

	pt(("min_splitGroup %d on symbol %d (dest %d)\n",splitGroup,splitSym,splitDest));

	Group ng;
	Group *gr = &groups.itemAt(splitGroup);
	//pt((" group=%s\n",gr->debInfo(true).chars() ));

	int newGroupIndex = groups.length();

	// construct a list of items to move, then move them
	// (since we need to leave them where they are for this test)
	Array<int> list;

	// we may be deleting from this list, so search in reverse order
	for (int i = gr->length() - 1; i >= 0; i--) {
		pt((" i=%d, gr len=%d\n",i,gr->length() ));

		int testState = gr->itemAt(i);
		pt((" state #%d=%d\n",i,testState));

		DFAState &s = getState(testState);
		if (getState(s.getTransitionState(splitSym)).group() != 
			splitDest) 
		{
			list.add(i);
		}
	}

	for (int i = 0; i < list.length(); i++) {
		int j = list.itemAt(i);
		int testState = gr->itemAt(j);
		DFAState &s = getState(testState);
		pt(("  moving state %d from group %d to %d\n",testState,splitGroup,
				newGroupIndex));
		ng.add(testState);
		s.setGroup(newGroupIndex);
		gr->remove(j,1);
	}

	pt((" adding new group\n"));
	groups.add(ng);
	pt((" after split:\n%s\n",dumpGroups(groups).chars() ));
}

void DFA::min_constructFromGroups(DFA &orig,
																	OrdSet &symbols, GroupList &groups,
																	int startGroup)
{
#undef pt
#define pt(a) //pr(a)

	pt(("min_constructFromGroups, startGroup=%d\n",startGroup));

	reset();
	pt(("  adding %d states...\n",groups.length() ));
	for (int i = 0; i < groups.length(); i++) {
		addState(i);
	}

	for (int i = 0; i < groups.length(); i++) {
		Group &g = groups.itemAt(i);
		pt((" group #%d = %s\n",i,g.debInfo(true).chars() ));
		DFAState &s = getState(i);

		pt(("  representative original state is %d\n",g.itemAt(0) ));
		DFAState &ref = orig.getState(g.itemAt(0));
		for (int j = 0; j < symbols.length(); j++) {
			int sym = symbols.itemAt(j);
			pt(("   transition on %d --> %d\n",sym,ref.getTransitionState(sym)));

			DFAState &sDest = orig.getState(ref.getTransitionState(sym));
			pt(("  original transition %d(%d)-->%d\n",
				g.itemAt(0), sym, ref.getTransitionState(sym) ));

			s.addTransition(sDest.group(),sym);
		}

		// determine if any of the states in this group were final
		// states.
		for (int j = 0; j < g.length(); j++) {
			DFAState &s2 = orig.getState(g.itemAt(j));

			if (s2.finalFlag()) {
				pt((" group %d state %d was final, code = %d\n",i,j,s2.finalCode()));

				s.setFinalFlag(s2.finalCode());
				break;
			}
		}
	}
	setStartState(startGroup);
}

bool DFA::min_lookForDeleteStates(int s) {
#undef pt
#define pt(a) //pr(a)

	pt(("min_lookForDeleteStates, state %d\n",s));
	pt(("%s\n",debInfo().chars() ));

	bool mods = false;

	DFAState &sp = getState(s);
	if (!sp.flag(DFAState::F_VISITED|DFAState::F_DELETE)) {
		sp.setFlag(DFAState::F_VISITED);

		Array<int> neighbors;
		sp.getSymbols(neighbors);
		for (int i = 0; i < neighbors.length(); i++) {
			int sym = neighbors.itemAt(i);
			int ns = sp.getTransitionState(sym);
			DFAState &s2 = getState(ns);
			if (s2.flag(DFAState::F_DELETE)) {
				pt((" state %d has transition on %d to deleted %d\n",
					s,sym,ns));
				sp.clearTransition(sym);
				mods = true;
			} else {
				if (min_lookForDeleteStates(ns))
					mods = true;
			}
		}

		// if state is not final, and has no transitions to other
		// states, delete it.
		if (!sp.finalFlag()) {
			bool retain = false;

			sp.getSymbols(neighbors);
			for (int i = 0; i < neighbors.length(); i++) {
				int sym = neighbors.itemAt(i);
				int ns = sp.getTransitionState(sym);
				if (ns != s) {
					retain = true;
					break;
				}
			}
			if (!retain) {
				pt((" marking state %d for deletion\n",s));
				sp.setFlag(DFAState::F_DELETE);
				mods = true;
			}
		}
	}
	return mods;
}

void DFA::min_clearFlags(int flags) {
	for (int i = 0; i < numStates(); i++) {
		DFAState &s = getState(i);
		s.clearFlag(flags);
	}
}

#if 0
/*	Modify state number conversion table to renumber a state
		> oldToNew					new state numbers, indexed by old
		> newToOld					old state numbers, indexed by new
		> oldIndex					current state number
		> newIndex					new number it should have
*/
static void setOrder(Array<int> &oldToNew, Array<int> &newToOld,
										 int oldIndex, int newIndex)
{
	// determine swaps to old->new
	int onA = oldIndex;
	int onB = newToOld.itemAt(newIndex);

	int noA = newIndex;
	int noB = oldToNew.itemAt(oldIndex);

	{
		int temp = oldToNew.itemAt(onA);
		oldToNew.set(onA, oldToNew.itemAt(onB));
		oldToNew.set(onB, temp);
	}
	{
		int temp = newToOld.itemAt(noA);
		newToOld.set(noA, newToOld.itemAt(noB));
		newToOld.set(noB, temp);
	}
}
#endif

void DFA::min_copyToOutput(DFA &out) const
{
//	WARN("Can use Renumber class now...");

	// construct tables to convert old->new, and new->old 
	// for renumbering

	Renumber ren;
	
//	Array<int> oldToNew, newToOld;

	//int j = 0;
	for (int i = 0; i < numStates(); i++) {
		DFAState &s = getState(i);
		ren.addOldItem(!s.flag(DFAState::F_DELETE));
#if 0
		oldToNew.add(j);
		if (!s.flag(DFAState::F_DELETE)) {
			newToOld.add(i);
			j++;
		} 
#endif
	}

	{
		// swap states around so start is first, and final states
		// are stored in states 1+k, where k is the final code.

		ren.renameItem(startState(),0);
//		setOrder(oldToNew, newToOld, startState(), 0);

#if 0
		// Not much use of this, since there are more than one final
		// states for a given regular expression
		for (int i = 0; i < numStates(); i++) {
			DFAState &s = getState(i);
			int n = s.finalCode();
			if (n < 0) continue;
			setOrder(oldToNew, newToOld, i, n+1);
		}
#endif
	}
	Array<int> syms;
	for (int i = 0; i < numStates(); i++) {
		DFAState &s = getState(i);

		//if (ren.newIndex(i) < 0) continue;
//		if (s.flag(DFAState::F_DELETE)) {
			//continue;
		//}

		int j = ren.newIndex(i);
		if (j < 0) continue;

//		int j = oldToNew.itemAt(i);
		out.addState(j);
		DFAState &s2 = out.getState(j);

		if (j == 0)
			out.setStartState(j);
		if (s.finalFlag())
			s2.setFinalFlag(s.finalCode());

		s.getSymbols(syms);
		for (int k = 0; k < syms.length(); k++) {
			int symbol = syms.itemAt(k);
			int dest = s.getTransitionState(symbol);
			ASSERT(dest >= 0);
			
			s2.addTransition(ren.newIndex(dest), /*oldToNew.itemAt(dest),*/symbol);
		}
	}
}

void DFA::minimize(DFA &minimized, bool verbose)
{
#undef pt
#define pt(a) //pr(a)

	pt(("Minimize:\n%s\n",debInfo().chars()));

	ASSERT(startState() >= 0);
	Array<Group> groups;
	OrdSet symbols;
	DFA min;

	// construct a list of all possible symbols
	min_getSymbolSet(symbols);

	// Add a 'sink' state that we can add a transition to for 
	// any undefined transitions.  We must remove the sink state
	// later, or the scan will never terminate until the input runs out!

	int sinkState = numStates();
	addState(sinkState);

	min_addSinkTransitions(symbols, sinkState);
	pt(("After adding sink state:\n%s\n",debInfo().chars()));

	min_addInitialGroups(groups);
	pt(("%s\n",dumpGroups(groups).chars() ));

	while (true) {
		pt(("Split loop; finding candidate for splitting...\n"));

		int splitGroup,splitSym,splitDest;

		if (!min_findGroupToSplit(symbols, groups, 
			splitGroup, splitSym, splitDest)) break;

		pt((" splitting group %d on symbol %d\n",splitGroup,splitSym));
		min_splitGroup(groups, splitGroup, splitSym, splitDest);
	}

	pt(("startState_ = %d\n",startState_));

	// determine which groups the start and sink belong to
	int startGroup = getState(startState_).group();
	pt((" startGroup=%d\n",startGroup));
	pt(("sinkState=%d\n",sinkState));
//	int sinkGroup = getState(sinkState).group();
//	pt((" sinkGroup=%d\n",sinkGroup));

	// construct the minimal DFA from the group information
	min.min_constructFromGroups(*this, symbols, groups, startGroup);

	min_removeSinkTransitions(symbols, sinkState);
	states_.remove(sinkState);

	// Perform a depth-first search from the start state,
	// removing any transitions to states marked for deletion.
	// If a state becomes a non-final state with no non-self
	// transitions, mark it for deletion.
	// Stop repeating if we didn't find any states to delete

	while (true) {
		min.min_clearFlags(DFAState::F_VISITED);
		if (!min.min_lookForDeleteStates(startGroup))
			break;
	}

	// delete any states that weren't visited.
	pt(("After looking for delete states,\n%s\n",min.debInfo().chars() ));

	for (int i = 0; i < min.numStates(); i++) {
		DFAState &s = min.getState(i);
		if (!s.flag(DFAState::F_VISITED)) {
			s.setFlag(DFAState::F_DELETE);
			pt((" ... node %d not reachable, deleting\n",i));
		}
	}

	// now copy to the minimal DFA, renaming states and
	// copying only those states not marked as deleted

	min.min_copyToOutput(minimized);

	if (verbose) {
		cout << "(Original DFA: " << numStates() << " states,\n"
			<<  "  minimal DFA: " << minimized.numStates() << " states)\n";
	}
	pt((" minimized:\n%s\n",minimized.debInfo().chars() ));

	if (numStates() < minimized.numStates()) {
		pr(("*** Minimize failure: %d states grew to %d\n",
			numStates(),minimized.numStates() ));
	}
}

#if DEBUG
void DFA::constructTestFile(const String &inputPath, 
														const String &outputPath)
{
	cout << "Constructing DFA test file from " << inputPath << " --> "
		<< outputPath << "\n";

	DFA dfa;
	dfa.addState(0);
	dfa.setStartState(0);

	// read file, split into words
	StringArray sa;
	{
		String str;
		TextReader::readToString(inputPath, str);
		str.split(sa);
	}

	// incorporate each word into the DFA
	for (int i = 0; i < sa.length(); i++) {
		String &str = sa.itemAt(i);

		dfa.setState(0);
		for (int j = 0; j < str.length(); j++) {
			char c = str.charAt(j);
			int newState = dfa.getTransitionState(dfa.state(),c);
			if (newState < 0) {
				newState = dfa.numStates();
				dfa.addState(newState);
				dfa.addTransition(dfa.state(), newState, c);
			}
			dfa.setState(newState);
		}
		dfa.makeStateFinal(dfa.state());
	}

	// dump the DFA to a string, which we will then write to a file
	{
		TextWriter tw(outputPath);
		dfa.writeText(tw);
		/*
		String s;
		dfa.dump(s);
		
		TextWriter::writeString(s,outputPath);
		*/
	}
}
#endif
#endif // DEBUG
