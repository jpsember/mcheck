#ifndef _FOREST
#define _FOREST

/*
	Using the Forest class

	A forest contains any number of trees, which share a set of nodes
	connected within a graph structure.

	A tree has a root node, and zero or more child trees.

	If two trees share the same node, then changes made to the node
	by one tree will affect the other as well.

	Trees have their own ids, and nodes have ids; the two ids are
	distinct and should not be confused.

	1)	Construct a forest.

	2)	To create a tree, you must have a root node to associate with the
			tree.  Either use an existing node, or create a node with
			newNode().  Then call newTree() with this node.

	3)	Extend a tree by inserting children.  You must specify the parent
			node, and a position to insert the child at.  You can choose to
			replace a child tree with the new one instead of inserting the new
			one.  Inserting or replacing the root must be done using other 
			methods (see setRoot()).

	4)	A tree can be deleted.  This doesn't delete the nodes, just 
			deregisters the id associated with the tree.  A subsequent call
			to garbageCollect() WILL delete the nodes, though... it performs
			a search of the graph and retains only those nodes that are
			reachable from the root of some non-deleted tree.

	5)	Once a tree or node id is assigned, it doesn't change until
			the tree or node is deleted.  The id is an index >= 0 that can
			be used by an application for storing or associating data with
			the trees or nodes.

	The forest class uses a callback to communicate with the application.
	In particular, it performs conversion of character symbols to and from
	node data codes for tree rewriting.
*/

class Forest {
public:

	Forest();

	/*	Get # children of a node
			> parent					parent node
			< # children
	*/
	int nChildren(int parent);

	/*	Get particular child node
			> parent					node to get child of
			> index						index of child (0..nChildren()-1)
			< child node
	*/
	int child(int parent, int index);

#if DEBUG
	/*	Get string describing forest
	*/
	const char *s();

	/*	Get string describing tree within forest
			> tree						id of tree
	*/
	const char *s(int tree);

	/*	Get string describing tree within forest
			> node						id of root node
	*/
	const char *sNode(int node);
#endif

	/*	Get root of tree
			> treeId					id of tree
			< id of root node
	*/
	int rootNode(int treeId);

	/*	Allocate a new node in the graph
			< id of node
	*/
	int newNode();

	/*	Create a new tree
			> id of root node
			< id of tree
	*/
	int newTree(int root);

	/*	Set root of tree
			> treeId					id of tree
			> root						root node
	*/
	void setRoot(int treeId, int root);

	/*	Delete tree
			> id of tree to delete
	*/
	void deleteTree(int treeId);

	/*	Print a tree to current sink
			> treeId					id of tree to print
	*/
	void printTree(int treeId);

	/*	Insert a new child node for a particular parent
			> parent					parent of new child
			> child						child node
			> position				where to insert child, or -1 to add to end
			> replaceFlag			if true, replaces existing instead of inserting new
	*/
	void insertChild(int parent, int child, int position=-1, bool replaceFlag=false);

	/*	Delete a child from a parent
			> parent					parent node 
			> position				position of child in list
	*/
	void deleteChild(int parent, int position);
	
	/*	Perform garbage collection of unused nodes
	*/
	void garbageCollect();

	// callback function type
	typedef int (cbForest)(int cmd, int arg1, int arg2);

	enum {
		CMD_GETSYM,				// get symbol associated with node data
		CMD_GETCODE,			// get node data associated with symbol 
		CMD_INITNODE,			// initialize node data
		CMD_PRINTTREE,		// print tree (debugging only; can be ignored)
	};

	/*	Match a pattern to a tree
			> root						root of tree to apply pattern to
			> pattern					script of pattern to apply
			> cbFunc					callback function
			< modifies root if it has changed (pattern script has ability
					to modify the tree)
			< true if pattern recognized
	*/
	bool patternMatch(int &root, const char *pattern, cbForest *cbFunc);

	/*	Rewrite a tree
			> root						root of tree
			> scripts					array of string pointers (pairs of strings, 
													with 0 marking end of array)
			> cbFunc					callback function
			< modifies root if it has changed
			< true if changes were made

			Strings are stored in pairs of recognizer + rewriter.
			The recognizer script returns true if the tree is a match
			for the following rewriter script, which is then applied.
	*/
	bool rewrite(int &root, const char * *scripts, cbForest *cbFunc);

	/*	Clear forest of all trees & nodes.
	*/
	void clear();

	/*	Get # trees
	*/
	int trees() const {
		return trees_.length();
	}

	int tree(int n) const {
		return trees_[n];
	}

	/*	Paint tree by setting flags for every reachable node from tree
			> root						root node
			> nodeFlags				bit flags for nodes in forest; already painted
												nodes are set, to avoid infinite loops; thus
												clear this before calling paint() if making
												initial (non-recursive) call
	*/
	void paint(int root, BitStore &nodeFlags);

	/*	Get an ordered list of the nodes in a tree
			> root						root node
			> list						where to put ordered list
	*/
	void getNodeList(int root, Array<int> &list);

	/*	Print tree
			> root						id of root
	*/
	void printRootedTree(int root);
private:

	Graph graph_;

	// list of trees (root nodes, or -1 if not active)
	Array<int> trees_;
	// recycle bin for trees
	Array<int> treesRB_;

	// flags indicating whether node is used
	BitStore nodesUsed_;
};


#if DEBUG
void Test_Forest();
#endif

#endif // _FOREST
