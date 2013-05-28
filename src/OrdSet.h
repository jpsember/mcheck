#ifndef _ORDSET
#define _ORDSET

//	Ordered set of integers
class OrdSet : private Array<int> {
public:
	void clear() {Array<int>::clear();}
	void add(int n);
	void remove(int n);
	void removeArrayItem(int pos) {
		Array<int>::remove(pos,1); }

	int length() const {return Array<int>::length(); }
	int itemAt(int pos) const {return Array<int>::itemAt(pos);}
	int operator[](int i) const {return itemAt(i);}

	/*	Calculate the union of two sets.
			> s1							first set
			> s2							second set
			< where to store union; may be same as one of the two sets
	*/
	static void calcUnion(const OrdSet &s1, const OrdSet &s2, OrdSet &dest);

	void include(const OrdSet &src);
	bool isEmpty() const {return (length() == 0);}
	bool equals(const OrdSet &s) const;
	bool contains(int n, int &loc) const;
	bool contains(int n) const {
		int loc;
		return contains(n, loc);
	};
#if DEBUG
	String debInfo() const;
#endif
};

#endif // _ORDSET
