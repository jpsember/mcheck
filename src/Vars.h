#ifndef _VARS
#define _VARS

class Vars {
public:
	Vars() { globalPtr_ = this; }

	/*	Convert variable index to string
			> propVar					index
			< string
	*/
	String &var(int index);

	/*	Convert variable string to index
			> var							string
			> addIfMissing		if true, and variable not found, adds it
			< index of variable, -1 if not found
	*/
	int var(String &str, bool addIfMissing = false);

	/*	Determine number of variables (1+max index)
	*/
	int length() const {return strs_.length();}
	static Vars *globalPtr() {ASSERT(globalPtr_ != 0);		return globalPtr_;}

	void clear() {
		tbl_.clear();
		strs_.clear();
	}
private:
	static Vars *globalPtr_;

	// hash table containing strings
	HashTable tbl_;
	// strings associated with each index
	StringArray strs_;
};

#endif // _VARS
