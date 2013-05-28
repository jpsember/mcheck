#include "Headers.h"
#include "OrdSet.h"

bool OrdSet::contains(int n, int &loc) const
{
	loc = -1;
	int i = -1;
	while (++i < length()) {
		if (itemAt(i) > n)
			break;
		if (itemAt(i) == n) {
			loc = i;
			return true;
		}
	}
	return false;
}

void OrdSet::remove(int n) {
	int loc;
	if (contains(n,loc)) 
		Array<int>::remove(loc,1);
}

bool OrdSet::equals(const OrdSet &s) const
{
	if (length() != s.length()) return false;
	for (int i = 0; i < length(); i++)
		if (itemAt(i) != s.itemAt(i)) return false;
	return true;
}

#if DEBUG
String OrdSet::debInfo() const
{
	String s("(");
	for (int i = 0; i < length(); i++) {
		if (i != 0)
			s << " ";
		s << itemAt(i);
	}
	s << ")";
	return s;
}
#endif

void OrdSet::add(int n) {
	int i = 0;
	while (i < length()) {
		if (itemAt(i) == n) return;
		if (itemAt(i) > n) break;
		i++;
	}
	insert(i,n);
}

void OrdSet::include(const OrdSet &src) {
	int i = 0, j = 0;
	for (; i < src.length(); i++) {
		int n = src.itemAt(i);
		// find insertion point for this element
		while (j < length() && n > itemAt(j))
			j++;
		if (j == length() || n != itemAt(j)) {
			insert(j,n);
			j++;
		}
	}
}

void OrdSet::calcUnion(const OrdSet &s0, const OrdSet &s1, OrdSet &dest)
{
	dest.clear();
	int i = 0, j = 0;
	while (true) {
		int next = 0;
		if (j == s1.length()) {
			if (i == s0.length()) break;
			next = s0.itemAt(i++);
		} else if (i == s0.length()) {
			next = s1.itemAt(j++);
		} else {
			next = s0.itemAt(i);
			if (s0.itemAt(i) > s1.itemAt(j)) {
				next = s1.itemAt(j++);
			} else {
				next = s0.itemAt(i++);
			}
		}
		if (dest.length() == 0 || dest.itemAt(dest.length()-1) != next)
			dest.add(next);
	}
}

