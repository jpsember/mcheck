#include "Headers.h"
#include "Graph.h"

#if DEBUG
const char * Node::s() const {
	String &w = Debug::str();
	w << "Node "
	;
	{
		for (int i = 0; i < nTotal(); i++) {
			if (i > 0)
				w << " ";
			int n = edgeDest(i);
			int data = edgeData(i);

			w << n;
			if (data >= 0)
				w << "(" << data << ")";
		}
	}
	return w.chars();
}
#endif

bool Node::hasNeighbor(int n, int *loc) const {
	bool found = false;
	if (loc)
		*loc = -1;
	for (int i = 0; i < nTotal(); i++) {
		if (edgeDest(i) == n) {
			found = true;
			if (loc)
				*loc = i;
			break;
		}
	}
	return found;
}

bool Graph::isEdge(int a, int b) const {
	Node &na = node(a);
	return na.hasNeighbor(b);
}

void Graph::newEdge(int src, int dest, int pos, bool replaceFlag)
{
	Node &na = node(src);
	na.addNeighbor(dest,-1,pos,replaceFlag);
}

#if 0
void Graph::newEdgeAt(int a, int aPos, int b, int bPos, int abData, int baData) 
{
	if (!isEdge(a,b)) {
		Node &na = node(a), &nb = node(b);
		na.addNeighbor(b, abData, aPos);
		if (!halfEdges())
			nb.addNeighbor(a, baData, bPos);
		else ASSERT(baData < 0);
	}
}
#endif


void Node::addNeighbor(int n, int data, int insertPos, bool replaceFlag) {
	ASSERT(n >= 0);

	int d[2];
	d[0] = n;
	d[1] = data;

	if (insertPos < 0) 
		insertPos = neighbors_.length() / 2;
	if (replaceFlag) {
		neighbors_.set(insertPos*2,d[0]);
		neighbors_.set(insertPos*2+1,d[1]);
	} else
		neighbors_.insert(insertPos*2,d,2);
}

void Graph::deleteEdge(int a, int pos) {
	Node &na = node(a);
	na.removeIndex(pos);
#if 0
	{
		int loc;
		if (na.hasNeighbor(b, &loc)) {
			na.removeIndex(loc);
			if (!halfEdges()) {
				Node &nb = node(b);
				if (nb.hasNeighbor(a, &loc))
					nb.removeIndex(loc);
			}
		}
	}
#endif
}

Graph::Graph(int flags) {
	setFlags(flags);
}
void Node::removeNeighbor(int n) {
	for (int i = 0; ; i++) {
		if (edgeDest(i) == n) {
			neighbors_.remove(i*2,2);
			break;
		}
	}
}

void Graph::deleteNode(int id) {
#undef p2
#define p2(a) //pr(a)

	p2(("Graph::deleteNode %d\n",id));
	p2((" graph is \n%s",s() ));

	Node &n = node(id);

	p2((" node nTotal=%d\n",n.nTotal() ));

	// don't remove edges if halfedges is active
	if (!halfEdges()) {
		for (int i = n.nTotal() - 1; i >= 0; i--) {
			Node &m = node(n.edgeDest(i));
			p2(("  dest node is %s\n",m.s() ));
			m.removeNeighbor(id);
		}
	}
	nodesRC_.add(id);
	p2((" after del, graph is \n%s",s() ));
}

void Node::removeIndex(int loc) {
	neighbors_.remove(loc*2,2);
}

Node &Graph::node(int id) const {
	Node &n = node0(id);
//	ASSERT(!n.deleted());
	return n;
}

int Graph::newNode() {
	Node n2;
	int i = nodes_.alloc(n2,nodesRC_);
//	Node &n = nodes_.itemAt(i);
//	n.setID(i);
	return i;
}

#if DEBUG
const char *Graph::s() const {
	String w;
	w << "Graph: ";

	if (flag(GFLAG_HALFEDGES))
		w << " HALFEDGES";
	if (flag(GFLAG_ALLOWLOOPS))
		w << " ALLOWLOOPS";
	if (flag(GFLAG_MULTIGRAPH))
		w << " MULTIGRAPH";
	w << "\n";

	for (int i = 0; i < nodes_.length(); i++) {
		Node &n = node0(i);
//		if (n.deleted()) continue;
		w << fmt(i,3) << ": " << n.s() << "\n";
	}
	String &s3 = Debug::str();
	s3.set(w);
	return s3.chars();
}

void Test_Graph()
{
	const int tests = 1;
	int t0 = 0, t1 = tests-1;

	//t0 = 0; t1 = t0;

#define TO_STR 0

	for (int t = t0; t <= t1; t++) {
		String r;
		Utils::pushSink(TO_STR ? &r : 0);

		switch (t) {
			case 0:
				{
					Graph g(GFLAG_HALFEDGES);
					{
						Cout << "Creating graph...\n";
						for (int i = 0; i < 5; i++) {
							int n = g.newNode();
							for (int j = 0; j < i; j += Utils::rand(3)) {
								int data = Utils::rand(90);
								g.newEdge(j,n,data);
							}
						}
						Cout << g.s();
					}

					{
						Cout << "Deleting some nodes...\n";
						for (int i = 4; i >= 0; i-=5) {
							g.deleteNode(i);
						//	Cout << g.s();
						}
						Cout << "After deletion...\n";
						Cout << g.s();
					}

					{
						Cout << "Adding some back...\n";
						for (int i = 0; i < 3; i++) {
//							int n =
							g.newNode();
						}
						Cout << g.s();
					}

				} break;
		}
		Utils::popSink();
#if TO_STR
		String path("String");
		path << t;
		Test(path.chars(), r);
#endif
	}
}
#endif

void Graph::clear() {
	nodes_.clear();
	nodesRC_.clear();
}

