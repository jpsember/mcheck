#include "globals.h"

#undef p2
#define p2(a) //pr(a)

Forest *Formula::forestPtr_;
SArray<Token> *Formula::tokensPtr_;
bool Formula::filterParen_;

int Formula::root()
{
	int r = treeId_;
	if (r >= 0) {
		r = forest().rootNode(r);
	}
	return r;
}

SArray<Token> &Formula::tokens() {
	ASSERT(tokensPtr_ != 0);
	return *tokensPtr_;
}

//static int cnt;
void Formula::construct() {
	typeFlags_ = 0;
	treeId_ = -1;
}

Formula::Formula() {
	construct();
}
Formula::~Formula() {
	if (treeId_ >= 0) {
//		pr(("Destroying formula %p, tree %d\n",this,treeId_));
		forest().deleteTree(treeId_);
	}
}
Formula::Formula(const Formula &s)
{
	construct();
	*this = s;
}
Formula& Formula::operator=(const Formula &s)
{
	if (&s != this) {
		// delete existing tree if necessary
		if (treeId_ >= 0)
			forest().deleteTree(treeId_);

		typeFlags_ = s.typeFlags_;
		treeId_ = s.treeId_;
		if (treeId_ >= 0) {
			treeId_ = forest().newTree(root());
		}
	}
	return *this;
}

#undef p2
#define p2(a) //pr(a)
Formula Formula::deepCopy()
{
	p2(("deepCopy %s\n",s(-1,true) ));

	Formula f;
	f = *this;

	int newRoot = deepCopyAux(root());
	forest().setRoot(f.treeId_, newRoot);

	p2((" returning %s\n",f.s() ));

	return f;
}

int Formula::deepCopyAux(int src)
{
	p2(("deepCopy src=%d\n",src));

		int dest = forest().newNode();
		Token t = token(src);
		tokens().add(t, dest);
		p2((" dest node=%d\n",dest));

		for (int i = 0; i < nChildren(src); i++) {
			int dcChild = deepCopyAux(child(src,i));
			p2((" child %d deep copy = %d\n",i,dcChild));
			forest().insertChild(dest, dcChild);
		}
		return dest;
}

Forest &Formula::forest() {
	ASSERT(forestPtr_ != 0);
	return *forestPtr_;
}

void Formula::addFormula(Formula &f, int fStart, int ourParent, int childIndex,
												 bool replaceFlag)
{
#undef pt
#define pt(a) //pr(a)

	pt(("addFormula (%s),fStart %d to (%s), ourParent %d\n",f.s(),fStart,this->s(),ourParent));
	pt((" this tree=%s\n",tree_.s()));pt(("  add tree=%s\n",f.tree_.s() ));

	if (ourParent < 0) 
		ourParent = root();

	if (fStart < 0)
		fStart = f.root();

	if (fStart >= 0) {
		// if current formula is empty, make added root this one's root
		if (ourParent < 0) {
			treeId_ = forest().newTree(fStart);
			pt((" adding as root\n"));
		} else {
			forest().insertChild(ourParent,fStart,childIndex,replaceFlag);
		}
	}
	pt((" formula now %s, tree %s\n",s(),tree_.s()));
}

void Formula::printToken(Token &t)
{
	int type = t.type();
#if DEBUG
	if (type < 0 || type >= TK_TOTAL)
		pr(("printToken, type=%d!\n",type));

	ASSERT(type >= 0 && type < TK_TOTAL);
#endif

	static const char *strs[] = {
	0, // TK_WS,
	0, // TK_MODELOP,
	0, // TK_MODELCL,
	0, // TK_INITIALSTATE,
	0,	// TK_COMPARE
	0,	// TK_COMPAREMID

	"->", // TK_IMPLIES,
	"!", // TK_NEGATION,
	"&", // TK_AND,
	"|", // TK_OR,
	0, // TK_PROPVAR,
	0, // TK_INT,
	"AU", // TK_AU,
	"EU", // TK_EU,
	0, // TK_UCL,
	0, // TK_PAROP,
	0, // TK_PARCL,
	"U", // TK_U,
	"R", // TK_R,
	"W", // TK_W,
	"T", // TK_TRUE,
	"B", // TK_BOTTOM,
	"X", // TK_X, 
	"F", // TK_F,
	"G", // TK_G,
	"AG ", // TK_AG,
	"EG ", // TK_EG,
	"AF ", // TK_AF,
	"EF ", // TK_EF,
	"AX ", // TK_AX,
	"EX ", // TK_EX,
	(const char *)99	// TK_TOTAL
	};

	ASSERT(strs[TK_TOTAL] == (const char *)99);

	if (type == TK_PROPVAR) {
		Cout << 
//			' ' <<
			t.str()
//			<< ' '
			;
	} else {
		const char *s = strs[type];
		if (s)
			Cout << s;
	}
}

void Formula::printNode(int root, int priority) {
	int nt = nType(root);

	static int pri[] = {
	0, // TK_WS,
	0, // TK_MODELOP,
	0, // TK_MODELCL,
	0, // TK_INITIALSTATE,
	0,	// TK_COMPARE
	0,	// TK_COMPAREMID

	10, // TK_IMPLIES,
	50, // TK_NEGATION,
	20, // TK_AND,
	20, // TK_OR,
	80, // TK_PROPVAR,
	0, // TK_INT,
	80, // TK_AU,
	80, // TK_EU,
	0, // TK_UCL,
	0, // TK_PAROP,
	0, // TK_PARCL,
	30, // TK_U,
	30, // TK_R,
	30, // TK_W,
	80, // TK_TRUE,
	80, // TK_BOTTOM,
	50, // TK_X, 
	50, // TK_F,
	50, // TK_G,
	50, // TK_AG,
	50, // TK_EG,
	50, // TK_AF,
	50, // TK_EF,
	50, // TK_AX,
	50, // TK_EX,
	99	// TK_TOTAL
	};
	ASSERT(pri[TK_TOTAL] == 99);

	ASSERT(nt >= 0 && nt < TK_TOTAL);
	int newPri = pri[nt];

	// add parenthesis if new priority is less than old,
	// or new is equal to old and it's a binary operator

	bool par = (newPri < priority)
		| (newPri == priority && nChildren(root) > 1);

	if (newPri == 30 && priority == 30)
		par = true;

	if (filterParen_)
		par = true;

	priority = newPri;

	if (par) {
		Cout << "(";
	}

	switch (nt) {
		case TK_X:
		case TK_G:
		case TK_F:
			printToken(tokens()[root]);
			Cout << " ";
			printNode(child(root,0),priority);
			break;

		case TK_IMPLIES:
		case TK_AND:
		case TK_OR:
		case TK_U:
		case TK_R:
		case TK_W:
			printNode(child(root,0),priority);
			Cout << " ";
			printToken(tokens()[root]);
			Cout << " ";
			printNode(child(root,1),priority);
			break;
		case TK_AU:
			Cout << "A[";
			printNode(child(root,0),30);
			Cout << " U ";
			printNode(child(root,1),30);
			Cout << "]";
			break;
		case TK_EU:
			Cout << "E[";
			printNode(child(root,0),30);
			Cout << " U ";
			printNode(child(root,1),30);
			Cout << "]";
			break;

		default: 
			printToken(tokens()[root]);
			for (int i = 0; i < nChildren(root); i++)
				printNode(child(root,i),priority);
			break;
	}
	if (par)
		Cout << ")";
}

void Formula::print(int root, bool verbose, int priority) {

	static const char *types[] = {"C/L ","LTL ","CTL ","CTL*"};
	if (verbose) {
		Cout << types[type()] << ": ";
	}

	if (root < 0)
		root = this->root();
	printNode(root, priority);
	if (verbose)
		Cout << "\n";

#if 0
	int nt = nType(root);

	// determine priority of this operation.
	static int pri[] = {
			TK_IMPLIES,10,
			TK_AND,20,
			TK_OR,20,
			TK_U,30,
			TK_R,30,
			TK_W,30,
			TK_AU,80,
			TK_EU,80,
			TK_AG,50,
			TK_EG,50,
			TK_AF,50,
			TK_EF,50,
			TK_AX,50,
			TK_EX,50,
			TK_X,50,
			TK_NEGATION,50,
			TK_PROPVAR,80,
			TK_BOTTOM,80,
			TK_TRUE,80,
		-1
	};
	int newPri = -1;
	for (int i = 0; pri[i] >= 0; i+=2) {
		if (pri[i] == nt) {
			newPri = pri[i+1];
			break;
		}
	}
	bool par = (newPri < priority); 
	if (newPri == 30 && priority == 30)
		par = true;

	if (filterParen_)
		par = true;

//WARN("always par");par = true;
	priority = newPri;

	//pr(("print nt=%d, pri=%d, newPri=%d\n",nt,priority,newPri));
	if (par) {
		//priority = newPri;
		Cout << "(";
	}

	switch (nt) {
		case TK_X:
			printToken(tokens()[root]);
			Cout << " ";
			print(child(root,0),false,priority);
			break;

		case TK_IMPLIES:
		case TK_AND:
		case TK_OR:
		case TK_U:
		case TK_R:
		case TK_W:
//			Cout << "(";

			print(child(root,0),false,priority);
			Cout << " ";
			printToken(tokens()[root]);
			Cout << " ";
			print(child(root,1),false,priority);
//			Cout << ")";
			break;
		case TK_AU:
			Cout << "A[";
			print(child(root,0),false,30);
			Cout << " U ";
			print(child(root,1),false,30);
			Cout << "]";
			break;
		case TK_EU:
			Cout << "E[";
			print(child(root,0),false,30);
			Cout << " U ";
			print(child(root,1),false,30);
			Cout << "]";
			break;

		default: 
			//Cout << '<';
			printToken(tokens()[root]);
			//Cout << '>';
			//Cout << " ";
			for (int i = 0; i < nChildren(root); i++)
				print(child(root,i),false,priority);
			break;
	}
	if (par)
		Cout << ")";
	if (verbose)
		Cout << "\n";
#endif
}

#if DEBUG
const char *Formula::ts(Token &t)
{
	String &s = Debug::str();
	Utils::pushSink(&s);
	printToken(t);
	Utils::popSink();
	return s.chars();
}


const char *Formula::s(int base, bool abbrev)
{
	String &s = Debug::str();
	Utils::pushSink(&s);

	if (base < 0)
		base = root();
	if (base >= 0)
		print(base, !abbrev);

	Utils::popSink();
	return s.chars();
}
#endif

int Formula::addToken(Token &t, int parent, int attachIndex) {
#undef pt
#define pt(a) //pr(a)

	pt(("Formula::addToken %s parent=%d attach=%d\n",t.str().s(),parent,attachIndex));

		if (parent < 0) {
			parent = root();
		}

		int tnum = -1;

		// if formula is empty, create a new node and make it the root
		if (parent < 0) {
			tnum = forest().newNode();
			treeId_ = forest().newTree(tnum);
			pt((" adding as root\n"));
		} else {
			tnum = forest().newNode();
			forest().insertChild(parent,tnum,attachIndex);
			//tnum = tree_.addChild(parent,attachIndex);
			pt((" adding as child\n"));
		}
		pt((" adding token to slot %d\n",tnum));
		tokens().add(t, tnum);
		return tnum;
}

void Formula::parse(Scanner &s) {
	p2(("Formula parse\n"));
	parse_fa(s,*this);
	convertToDAG();
	calcType();
//	forest().printTree(this->treeId_);
}

void Formula::parse_fa(Scanner &s, Formula &f) {

	p2(("parse fa\n"));

	Formula fb;
	parse_fb(s,fb);
	if (s.peek().type(TK_IMPLIES)) {
		Token impl;
		s.read(impl);

		f.addToken(impl);

		Formula fa;
		parse_fa(s,fa);			
		f.addFormula(fb);
		f.addFormula(fa);
	} else {
		f.addFormula(fb);
	}
}


void Formula::parse_fb(Scanner &s, Formula &f) {
#if 1	// left associative
/*
	<fb> ::=  <fc> <fb'>
	
	<fb'>::=	"&" <fc> <fb'>
					| "|" <fc> <fb'>
					| e
					
*/
	p2(("parse_fb\n"));
	Formula fc;
	parse_fc(s,fc);
	p2((" parsed fc, now parsing fbp\n"));
	parse_fbp(s,f,fc);
	p2((" result is %s\n",fc.s()));
#else
	Formula fc;
	parse_fc(s,fc);

	Token t;
	s.peek(t);
	if (t.type(TK_AND) || t.type(TK_OR)) {
		s.read(t);
		f.addToken(t);

		Formula fb;
		parse_fb(s,fb);
		f.addFormula(fc);
		f.addFormula(fb);
	} else
		f.addFormula(fc);
#endif
}
void Formula::parse_fbp(Scanner &s, Formula &f, Formula &fin) {
/*
	<fb'>::=	"&" <fc> <fb'>
					| "|" <fc> <fb'>
					| e
					
*/
	Token t;
	s.peek(t);
	if (t.type(TK_AND) || t.type(TK_OR)) {
		s.read(t);
		f.addToken(t);
		f.addFormula(fin);

		Formula fc;
		parse_fc(s,fc);

		f.addFormula(fc);
		Formula fbp;
		parse_fbp(s, fbp, f);
		f = fbp;
	} else
		f = fin;
}

void Formula::parse_fc(Scanner &s, Formula &f) {
	Formula fd;
	parse_fd(s,fd);
	Token t;
	s.peek(t);
	if (t.type(TK_U) || t.type(TK_R) || t.type(TK_W)) {
		s.read(t);
		f.addToken(t);
		Formula fc;
		parse_fc(s,fc);
		f.addFormula(fd);
		f.addFormula(fc);
	} else
		f.addFormula(fd);
}

void Formula::parse_fd(Scanner &s, Formula &f) {
	p2(("parse_fd\n"));
	Token t;
	s.peek(t);
	p2((" token=%s\n",t.str().s() ));
	if (t.type(TK_NEGATION) || (t.type() >= TK_X && t.type() <= TK_EX)) {
		s.read(t);
		f.addToken(t);
		Formula fd;
		parse_fd(s,fd);
		f.addFormula(fd);
	} else
		parse_fe(s,f);
}


void Formula::parse_fe(Scanner &s, Formula &f) {
	Token t;
	s.peek(t);
	p2(("parse_fe, token %s\n",t.str().s() ));

	switch (t.type()) {
		case TK_AU:
		case TK_EU:
			{
			s.read(t);

			t.setStr(t.type(TK_AU) ? "AU" : "EU");
			f.addToken(t);
			// <fd> "U" <fc> 

			Formula fd;
			Formula fc;
			parse_fd(s,fd);
			s.read(TK_U);
			parse_fc(s,fc);

			f.addFormula(fd);
			f.addFormula(fc);
			s.read(TK_UCL);
			}
			break;

		case TK_PAROP:
			s.read();
			parse_fa(s,f);
			s.read(TK_PARCL);
			break;
		case TK_PROPVAR:
		case TK_TRUE:
		case TK_BOTTOM:
			s.read(t);
			f.addToken(t);
			break;

		default:
			throw ParseException("Unrecognized token in formula");
	}
}

void Formula::calcType(int startNode) {
	if (startNode < 0) {
		typeFlags_ = 0;
		startNode = root();
		//startNode = tree_.root();
	}
	if (startNode >= 0) {
		int tk = nType(startNode);

		static int tcodes[] = {
			// LTL only:
			TK_U,TK_R,TK_W,TK_X,TK_F,TK_G,
			-1,
			// CTL only:
			TK_AU,TK_EU,TK_AG,TK_EG,TK_AF,TK_EF,TK_AX,TK_EX,
			-2
		};
		int flag = BF_LTL;
		for (int i = 0; tcodes[i] != -2; i++) {
			if (tcodes[i] == -1) {
				flag = BF_CTL;
				continue;
			}
			if (tcodes[i] == tk) {
				typeFlags_ |= flag;
				break;
			}
		}

		// recursively examine child nodes
		int j = nChildren(startNode);
		for (int i = 0; i < j; i++)
			calcType(child(startNode,i));
	} 
}

int Formula::nType(int node) {
	ASSERT(node >= 0);
	return tokens()[node].type();
}

Token &Formula::token(int node) {
	ASSERT(node >= 0);
	return tokens()[node];
}

const String &Formula::str(int node) {
	if (node < 0)
		node = root();
	return tokens()[node].str();
}


int Formula::child(int parent, int childIndex) {
	ASSERT(parent >= 0);
//	if (parent < 0) parent = root();
	return forest().child(parent,childIndex);
}

int Formula::nChildren(int node) {
	//if (node < 0) node = root(); 
	return forest().nChildren(node);
}

int Formula::cbFunc(int cmd, int arg1, int arg2)
{
//	pr(("cbFunc cmd=%d arg1=%d arg2=%d\n",cmd,arg1,arg2));

	static char ids[] = {
	'>',TK_IMPLIES,
	'!',TK_NEGATION,
	'&',TK_AND,
	'|',TK_OR,
	'V',TK_PROPVAR,
	'C',TK_AU,
	'D',TK_EU,
	'U',TK_U,
	'R',TK_R,
	'W',TK_W,
	'T',TK_TRUE,
	'B',TK_BOTTOM,
	'X',TK_X,
	'F',TK_F,
	'G',TK_G,
	'E',TK_AG,
	'H',TK_EG,
	'I',TK_AF,
	'J',TK_EF,
	'K',TK_AX,
	'L',TK_EX,
	-1
	};

	int out = -1;
	switch (cmd) {
		case Forest::CMD_PRINTTREE:
			{
				printNode(arg1);
//				if (globalPtr) {
//					globalPtr->print(arg1,false);
					Cout << "\n";
//				}
			}
			break;
		case Forest::CMD_INITNODE:
			{
				int code = cbFunc(Forest::CMD_GETCODE,arg2);
				Token t(code);
				tokens().add(t,arg1);
			}
			break;

		case Forest::CMD_GETSYM:
			{
				Token &t = tokens()[arg1];

				for (int i = 0; ids[i]; i+=2) {
					if (ids[i+1] == t.type()) {
						out = ids[i+0];
						break;
					}
				}
				//pr(("getsym, arg1=%d\n",arg1));
				//ASSERT(out >= 0);
			}
			break;
		case Forest::CMD_GETCODE:
			{
				for (int i = 0; ids[i]; i+=2) {
					if (ids[i+0] == arg1) {
						out = ids[i+1];
						break;
					}
				}
			}
			break;
		default:
			ASSERT(false);
			break;
	}
	return out;
}

bool Formula::equal(int root1, int root2)
{
	Forest &f = forest();

	bool match = false;
	do {
		Token &t1 = tokens()[root1], &t2 = tokens()[root2];
		if (t1.type() != t2.type()) break;
		if (t1.type(TK_PROPVAR)
			&& !t1.str().equals(t2.str())) break;
		
		for (int i = 0; i < f.nChildren(root1); i++) {
			if (!equal(f.child(root1,i),f.child(root2,i))) {
				return false;
			}
		}
		match = true;
	} while (false);
	return match;
}

static const char *scriptsCTL[] = {
		// get rid of double negation
		"m! c0 m!",		"c0 c0",

		// ->		(symbol = >)
		"m>",		"r1 u ! d u r0 u & d d u ! d",
		
		// or		(symbol = |)
		"m|",		"r1 u ! d u r0 u ! d u & d d u ! d",

		// CTL-specific:

		// AU		(symbol = C)
		"mC",		"r1 u I d u r1 u ! d u r0 u ! d u & "
						"d d u r1 u ! d u D d d u ! d u & d d",

		// TRUE	(symbol = T)
		"mT",		"B u ! d",

		// AG		(symbol = E)
		"mE",		"r0 u ! d u B u ! d u D d d u ! d",

		// EG		(symbol = H)
		"mH",		"r0 u ! d u I d u ! d",

		// EF		(symbol = J)
		"mJ",		"r0 u B u ! d u D d d",

		// AX		(symbol = K)
		"mK",		"r0 u ! d u L d u ! d",
#if 0
		// LTL-specific:
		
		// W		(symbol = W)
		"mW",		"r0 u ! d u B u ! d u U d d u r1 u r0 u U d d u ! d u & d d u ! d",

		// F		(symbol = F)
		"mF",		"r0 u B u ! d u U d d",

		// R		(symbol = R)
		"mR",		"r1 u ! d u B u ! d u U d d u r1 u r0 u & d d u r1 u U d d u ! "
						"d u & d d u ! d",

		// G		(symbol = G)
		"mG",		"r0 u ! d u B u ! d u U d d u ! d",
#endif
	0
}, 

// negation normal form
*scriptsLTL[] = 
{
		// get rid of double negation
		"m! c0 m!",		"c0 c0",

		// LTL-specific:

		// ->		
		"m>",		"r1 u r0 u ! d u | d d",
		// W		
		"mW",   "r1 u r0 u | d d u r1 u R d d",
		// F
		"mF",		"r0 u T u U d d",
		// G
		"mG",		"r0 u B u R d d",
		// !(a & b)   =>  !a | !b
		"m! r0 m&",	"r0 c1 u ! d u r0 c0 u ! d u | d d",
		// !(a | b)   =>  !a ^ !b
		"m! r0 m|",	"r0 c1 u ! d u r0 c0 u ! d u & d d",

		// !(a U b)
		"m! r0 mU",	"r0 c1 u ! d u r0 c0 u ! d u R d d",
		// !(a R b)
		"m! r0 mR", "r0 c1 u ! d u r0 c0 u ! d u U d d",
		// ! X a
		"m! r0 mX", "r0 c0 u ! d u X d",
	0
};

void Formula::reduce(int s) {
#undef p2
#define p2(a) //pr(a)

	p2(("Formula::reduce %s\n",forest().s(r)));

//	globalPtr = this;

	int r = root();

	// repeat until fixed point reached.
	while (true) {
		bool mods = false;

		const char * *scr = 0;
		
		if (isCTL()
#if FAVOR_LTL
					// treat as LTL if both for debug purposes
			&& !isLTL()
#endif

			) scr = scriptsCTL;
		else if (isLTL())
		scr = scriptsLTL;
		
		if (scr != 0) {
			mods = forest().rewrite(r, scr, cbFunc);
		}
		p2(("rewritten=> %s\n",forest().sNode(r) ));
		if (!mods) break;
	}

	forest().setRoot(treeId_,r);

	//WARN("not cvt to dag");
	convertToDAG();

//	globalPtr = 0;

}

Formula Formula::negate(int root)
{
//	pr(("Negate formula root=%d\n",root));
//	printNode(root);Cout << "\n";

	Formula f;
	int n = forest().newNode();
	Token t(TK_NEGATION);
	tokens().add(t,n);
	f.treeId_ = forest().newTree(n);
	forest().insertChild(n,root);
#if 0
	switch (nType(root)) {
		case TK_NEGATION:
			f.treeId_ = forest().newTree(child(root,0));
			break;
		case TK_TRUE:
			{
				int n = forest().newNode();
				Token t(TK_BOTTOM);
				tokens().add(t,n);
				f.treeId_ = forest().newTree(n);
			}
			break;
		case TK_BOTTOM:
			{
				int n = forest().newNode();
				Token t(TK_TRUE);
				tokens().add(t,n);
				f.treeId_ = forest().newTree(n);
			}
			break;
		default:
			{
				int n = forest().newNode();
				Token t(TK_NEGATION);
				tokens().add(t,n);
				f.treeId_ = forest().newTree(n);
				forest().insertChild(n,root);
			}
			break;
	}
#endif
	return f;
}

#if DEBUG
void Formula::printForest()
{
	Cout << "---------- Forest --------------------\n";
	for (int i = 0; i < forest().trees(); i++) {
		int root = forest().tree(i);
		if (root < 0) continue;
		Cout << "Tree #" << fmt(i) << ": ";
		Formula::printNode(root);
		Cout << "    nodes= " << forest().sNode(root) << "\n";
	}
	Cout << "\n";
}
#endif

void Formula::convertToDAG()
{
	//WARN("not cvt to dag");return;
	Forest &f = forest();

	/*
		build list of nodes in formula
		compare nodes i to nodes j (j > i), and if equal, alias j = i
		modify nodes to redirect aliased nodes
	*/
	
	Array<int> nl;
	Array<int> alias;

	//	build list of formula nodes; put list in nl
	f.getNodeList(root(), nl);

	// compare nodes j to i, building alias list in alias

	for (int i = 0; i < nl.length(); i++) {
		int al = nl[i];
		int orig = al;
		p2(("Node #%d = %d\n",i,al));
		for (int j = 0; j < i; j++) {
			int nn = nl[j];
			if (alias[nn] != nn) continue;
			if (!equal(al,nn)) continue;
			al = nn; break;
		}
		alias.add(al,orig);
	}

	// redirect all children nodes to aliases
	for (int i = 0; i < nl.length(); i++) {
		int n = nl[i];
		if (alias[n] != n) continue;
		for (int j = 0; j < f.nChildren(n); j++) {
			int c = alias[f.child(n, j)];
			f.insertChild(n,c,j,true);
		}
	}
}

void Formula::printReduced()
{
	Formula f2 = deepCopy();
	f2.reduce();
	//Utils::pad(6);
	f2.print(-1,false);
	Cout << "\n";
}

int Formula::getLiteralCode(int root)
{
	bool neg = false;

	{
		Token &t = Formula::token(root);
		if (t.type(TK_NEGATION)) {
			neg = true;
			root = Formula::child(root,0);
		}
	}

	int val = 0;

	Token &t = Formula::token(root);
	switch (t.type()) {
		case TK_PROPVAR:
			val = 2 + Vars::globalPtr()->var(t.str(), true);
			break;
		case TK_TRUE:
			val = 1;
			break;
		case TK_BOTTOM:
			val = -1;
			break;
	}
	if (neg)
		val = -val;
	return val;
}

