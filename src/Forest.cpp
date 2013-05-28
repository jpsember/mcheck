#include "globals.h"

Forest::Forest() {
	graph_.setFlags(
		GFLAG_HALFEDGES 
		|GFLAG_ALLOWLOOPS
		|GFLAG_MULTIGRAPH
		);
}

int Forest::newNode()
{
	int n = graph_.newNode();
	nodesUsed_.set(n);
	return n;
}

void Forest::clear() {
	trees_.clear();
	treesRB_.clear();
	graph_.clear();
}

#if DEBUG
void Test_Forest()
{
	pr(("Test_Forest\n"));
	Utils::srand(1965);

	Forest f;
	//pr(("%s",f.s() ));
	int n = f.newNode();
	pr((" created node=%d\n",n));

	int tree;
	tree = f.newTree(n);
	pr((" constructed tree %s\n",f.s(tree) ));
	//pr(("%s",f.s() ));

	{
		int t2 = f.newTree(n);
		//pr(("%s",f.s() ));
		f.deleteTree(tree);
		tree = t2;
		pr((" deleted original tree, created duplicate with id %d, %s\n",tree,f.s(tree) ));
	}

	Array<int> s;
	s.add(n);
	for (int i = 0; i < 5; i++) {
		int parent = s[i];
		int k = Utils::rand(5);
		for (int j = 0; j < k; j++) {
			int c = f.newNode();
			f.insertChild(parent,c);
			s.add(c);
		}
	}

	pr(("Added lots of nodes:\n %s\n",f.s(tree) ));
	f.deleteChild(1,1);
	f.deleteChild(2,2);
	pr(("Deleted some children:\n %s\n",f.s(tree) ));

	f.setRoot(tree,4);
	pr(("Changed root to 4:\n %s\n",f.s(tree) ));

	int tree2 = f.newTree(1);
	pr((" created new tree %d\n",tree2));

	pr(("%s",f.s() ));

	pr((" performing garbage collection\n"));
	f.garbageCollect();
	pr(("%s",f.s() ));

	{
		int c = f.newNode(),
			c2 = f.newNode();
		pr((" two new nodes are %d and %d\n",c,c2));
		f.insertChild(2,c,0);
		f.insertChild(2,c2,0);
	}
	pr(("%s",f.s() ));

	pr(("\n"));
}

const char *Forest::s(int tree)
{
	String &s = Debug::str();
	Utils::pushSink(&s);
	printTree(tree);
	Utils::popSink();
	return s.chars();
}

const char *Forest::sNode(int node)
{
	String &s = Debug::str();
	Utils::pushSink(&s);
	printRootedTree(node);
	Utils::popSink();
	return s.chars();
}


const char *Forest::s()
{
	String &s = Debug::str();
	Utils::pushSink(&s);
	Cout << "===Forest===\n";
	for (int i = 0; i < trees_.length(); i++) {
		int root = trees_[i];
		if (root < 0) continue;
		Cout << fmt(i) << ": ";
		printTree(i);
		Cout << "\n";
	}
	//Cout << "Underlying graph:\n" << graph_.s();
	Cout << "============\n";
	Utils::popSink();
	return s.chars();
}
#endif

void Forest::deleteChild(int parent, int position)
{
#undef p2
#define p2(a) //pr(a)

	p2(("deleteChild parent=%d position=%d child=%d\n",parent,position,child(parent,position)));
	graph_.deleteEdge(parent,position);
//	int c = child(parent,position);
//	Node &n = graph_.node(parent);
//	int child = n.edgeDest(position);
//	graph_.deleteEdge(parent,c);
}

void Forest::deleteTree(int treeId)
{
#undef p2
#define p2(a) //pr(a)

	p2(("delete tree %d (%d)\n",treeId,trees_[treeId]));
	ASSERT(trees_[treeId] >= 0);
	trees_.set(treeId,-1);
	trees_.free(treeId, treesRB_);
}


int Forest::newTree(int root)
{
#undef p2
#define p2(a) //pr(a)
	int treeId = trees_.alloc(root, treesRB_);
	p2(("create tree %d (%d)\n",treeId,trees_[treeId]));
	return treeId;
}


void Forest::insertChild(int parent, int child, int position, bool replaceExisting)
{
#undef p2
#define p2(a) //pr(a)
	p2(("insertChild parent=%d child=%d position=%d\n",parent,child,position));
	graph_.newEdge(parent,child,position,replaceExisting);
}

void Forest::printTree(int treeId)
{
	int root = trees_[treeId];
	printRootedTree(root);
}

void Forest::printRootedTree(int root)
{
	int nKids = nChildren(root);
	//Node &node = graph_.node(root);
	if (nKids > 0) {
		Cout << "(" << root;
		for (int i = 0; i < nKids; i++) {
			Cout << " ";
			printRootedTree(child(root,i));
		}
		Cout << ")";
	} else {
		Cout << root;
	}
}

void Forest::setRoot(int treeId, int root)
{
	trees_.set(treeId,root);
}

void Forest::getNodeList(int root, Array<int> &list)
{
	list.clear();
	BitStore nodeFlags;
	paint(root,nodeFlags);
	
	int node = 0;
	nodeFlags.start();
	while (!nodeFlags.done()) {
		if (nodeFlags.next())
			list.add(node);
		node++;
	}
}

void Forest::paint(int root, BitStore &nodeFlags)
{
	if (!nodeFlags.get(root)) {
		nodeFlags.set(root);
		Node &n = graph_.node(root);
		for (int i = 0; i < n.nTotal(); i++) {
			paint(n.edgeDest(i), nodeFlags);
		}
	}
}

void Forest::garbageCollect()
{
#undef p2
#define p2(a) //pr(a)

	p2(("Forest, garbage collection\n"));

	// flags for each node, for traversing operations
	BitStore nodeFlags;

	for (int tid = 0; tid < trees_.length(); tid++) {
		int root = trees_[tid];
		if (root < 0) continue;
		paint(root, nodeFlags);
	}

	for (int i = 0; i < nodesUsed_.length(); i++) {
		if (nodesUsed_.get(i)
			&& !nodeFlags.get(i)) {
				p2((" recycling node %d\n",i));
				nodesUsed_.set(i,false);
				graph_.deleteNode(i);
			}
	}
}

int Forest::rootNode(int treeId)
{
	int root = trees_[treeId];
	ASSERT(root >= 0);
	return root;
}

int Forest::child(int parent, int index)
{
	Node &node = graph_.node(parent);
	return node.edgeDest(index);
}

int Forest::nChildren(int parent)
{
	Node &node = graph_.node(parent);
	return node.nTotal();
}

bool Forest::rewrite(int &root, const char * *scripts,
		cbForest *cbFunc)
{
	bool mods = false;

	// rewrite the children
	int nc = nChildren(root);
	for (int i = 0; i < nc; i++) {
		int r = child(root,i);
		mods |= rewrite(r,scripts,cbFunc);
		// replace child with new child root in case it's changed
		insertChild(root,r,i,true);
	}

	for (int s = 0; scripts[s] != 0; s += 2) {
		int r = root;
		bool match = patternMatch(r, scripts[s], cbFunc);

		if (!match) continue;
		mods |= patternMatch(root, scripts[s+1], cbFunc);
	}

	return mods;
}


bool Forest::patternMatch(int &root, const char *script,cbForest *cbFunc)
{
#undef p2
#define p2(a) //pr(a)

	Stack<int> stack;

	p2(("Forest, procScript root=%d, script=%s\n",root,script));
	//cbFunc(CMD_PRINTTREE, root, -1);

	// current node
	int node = root;

	bool result = true;

	for ( ;  result && *script; script++) {
		char c = script[0];
		if (c == ' ') continue;
		char c2 = script[1];
		p2((" node=%2d script=[%s]\n",node,script));
		switch (c) {
			case 'm':
				{
					//const char *sym = &script[1];
					script++;
					int sym = cbFunc(CMD_GETSYM, node, 0);
					if (sym != c2) {
						result = false;
						break;
					}
				}
				break;
			case 'c':
				{
					script++;
					int ci = c2 - '0';
					node = child(node, ci);
					p2((" descended to child %d = %d\n",ci,node));
				}
				break;
			case 'd':
				{
					// pop node, attach as rightmost child
					int n = stack.pop();
					insertChild(node,n);
				}
				break;
			case 'u':
				{
					// push current node on stack
					stack.push(node);
					// create new node
					node = newNode();
				}
				break;
			case 'r':
				{
					script++;
					// set node as child of original
					int childNum = c2 - '0';
					node = child(root,childNum);
				}
				break;
			default:
				{
					// convert character to node type, create new node
					//int tokenType = cbFunc(CMD_GETCODE,c,-1);
					//ASSERT(tokenType >= 0);
					script++;
					node = newNode();
					cbFunc(CMD_INITNODE, node, c);
					//int sym = cbFunc(CMD_GETSYM, node, 0);

					//Token t(tokenType);
					//Formula::tokens().add(t,node);
				}
				break;
		}
	}
	root = node;
	return result;
}
