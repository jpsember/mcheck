#include "globals.h"

#undef pt
#define pt(a) //pr(a)

#if DOS
#pragma warning(disable : 4311)
#pragma warning(disable : 4312)
#endif

Vars *Vars::globalPtr_;

int Vars::var(String &str, bool addIfMissing)
{
	pt(("Vars::var %s (add=%s)\n",str.s(),bs(addIfMissing) ));

	int index = -1;
	void *ptr = tbl_.get(str);
	pt((" hash table ptr is %p\n",ptr));
	index = (*((int*)(&ptr)))-1;
//	index = ((int) ptr) - 1;
	pt((" cast to int %d\n",index));
	if (index < 0) {
		if (addIfMissing) {
//			size_++;
			index = strs_.length();
			tbl_.set(str,(void *)(index+1));
			strs_.add(str);
		}
	}
	pt((" returning %d\n",index));
	return index;
}

String &Vars::var(int index)
{
	pt(("Vars::var %d (size=%d)\n",index,strs_.length() ));
	String &s = strs_[index];
	pt((" returning %s\n",s.s() ));
	return s;
}

