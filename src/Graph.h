#ifndef _GRAPH

class Graph;
class Node;

class Node {
	friend class Graph;
public:

	/*	Determine if node has a neighbor
			> n								id of node to look for
			> loc							if not 0, position of neighbor in list returned here,
													or -1 if not found
			< true if n is neighbor
	*/
	bool hasNeighbor(int n, int *loc = 0) const;

#if DEBUG
	/*	Get a string describing the object; uses the 
			DEBUG circular string queue
	*/
	const char *s() const;
#endif
	/*	Determine # neighbors (i.e. the degree) for this node
			< # neighbors
	*/
	int nTotal() const {return neighbors_.length() / 2; }

	/*	Get neighbor's id
			> i								neighbor index
			< id of neighbor
	*/
	int edgeDest(int i) const {return neighbors_.itemAt(i*2);}
	
	/*	Get an edge's id
			> i								neighbor index
			< id, or -1 if undefined
	*/
	int edgeData(int i) const {return neighbors_.itemAt(i*2+1);}

private:

	/*	Add neighbor to list
			> n								id of neighbor to add
			> edgeData				id to associate with edge, or -1 if undefined
			> insertPos				position in neighbor list to insert at (-1 to
													add to end)
			> replaceFlag			if true, replaces existing
	*/
	void addNeighbor(int n, int edgeData = -1, int insertPos = -1, bool replaceFlag = false);

	/*	Remove neighbor from list; must exist
	*/
	void removeNeighbor(int n); 

	/*	Remove a neighbor from a particular position in the list
			> loc							location of neighbor in list (0...n-1)
	*/
	void removeIndex(int loc);

	// list of node neighbors, with associated data;
	// stored as pairs of (neighbor data), where data is -1 if undefined
	Array<int> neighbors_;
};


enum {
	GFLAG_HALFEDGES		= 0x0001,	// half edges added/removed separately?
	GFLAG_ALLOWLOOPS	= 0x0002,	// edges from v to v allowed?
	GFLAG_MULTIGRAPH	= 0x0004,	// multiple edges from a to b allowed?
};

class Graph {
public:
	/*	Constructor
			> flags						flags describing type of graph (GFLAG_xxx)
	*/
	Graph(int flags = 0);

	/*	Set flags describing type of graph
			> flags						flags describing type of graph (GFLAG_xxx)
	*/
	void setFlags(int flags) {flags_ = flags; }

	/*	Determine if a particular flag is set in the graph
			> f								flag or combination of flags to test (GFLAG_xxx)
			< true if all of the flags were true
	*/
	bool flag(int f) const {return (flags_ & f) == f; }

#if DEBUG
	/*	Get a string describing the object; uses the 
			DEBUG circular string queue
	*/
	const char *s() const;
#endif

	/*	Add a new node to the graph
			< id of new node
	*/
	int newNode();

	/*	Delete a node, any edges leaving it, and if not HALFEDGES, any
			edges entering it

			> id							id of node to delete
	*/
	void deleteNode(int id);

	/*	Get reference to a node
			> id							id of node to get
			< reference to Node
	*/
	Node &node(int id) const;

	/*	Determine if graph is storing half edges
			< true if so
	*/
	bool halfEdges() const {return flag(GFLAG_HALFEDGES);}
#if 0
	/*	Add an edge between two nodes, if it doesn't already exist
			> a, b						ids of the two nodes
			> abData					edge data for a->b, or -1
			> baData					edge data for b->a, or -1; only used if
													HALFEDGES is false
	*/
	void newEdge(int a, int b, int abData = -1, int baData = -1) {
		newEdgeAt(a,-1,b,-1,abData,baData);
	}

	/*	Add edge between two nodes at particular location in 
			respective lists
			> a								id of first node
			> aPos						position in a's neighbor list to insert edge,
													or -1 to add to end
			> b								id of second node
			> bPos						position in b's neighbor list to insert edge,
													or -1 to add to end
			> abData					edge data for a->b, or -1
			> baData					edge data for b->a, or -1; only used if
													HALFEDGES is false
	*/
	void newEdgeAt(int a, int aPos, int b, int bPos, int abData = -1, int baData = -1);
#endif
	/*	Add a directed edge (HALFEDGES only)
			> src							source node
			> dest						destination node
			> pos							position in source's list to insert (-1 for end)
			> replaceFlag			if true, existing one is replaced
	*/
	void newEdge(int src, int dest, int pos = -1, bool replaceFlag = false);

	/*	Determine if an edge exists between two nodes
			> a, b						ids of the two nodes
			< true if edge exists from a->b
	*/
	bool isEdge(int a, int b) const;

	/*	Remove a half edge
			> a								source node
			> pos							position in list							
	*/
	void deleteEdge(int a, int pos);

	void clear();

private:
	/*	Get reference to a node
			> id							id of node to get; can be a deleted node
			< reference to Node
	*/
	Node &node0(int id) const {return nodes_.itemAt(id); }
	//	list of nodes in graph
	Array<Node> nodes_;
	RecycleBin nodesRC_;

	//	GFLAG_xxx
	int flags_;
};

#if DEBUG
void Test_Graph();
#endif


#endif // _GRAPH
