#ifndef _HASHTABLE
#define _HASHTABLE

#if DEBUG
void Test_HashTable();
// Functions to dump various types of data
void HashTable_dumpHex(String &s, const void *data);
void HashTable_dumpString(String &s, const void *data);
void HashTable_dumpChars(String &s, const void *data);
void HashTable_dumpInt(String &s, const void *data);
int HashTable_sortByKeys(const char *aKey, const void *aData,
			const char *bKey, const void *bData);
#endif

class HashTable
{
	// ------------------------------------
public:
	// constructor
	HashTable(bool smartPtrs = false);
	// destructor
	virtual ~HashTable();
private:	// cannot copy these objects
	// copy constructor
	HashTable(const HashTable &s) {}
	// assignment operator
	HashTable &operator=(const HashTable &s) {return *this;}
private:
	// initializer
	void construct();
	// ------------------------------------
	// declare HashEntry class to be defined later
	class HashEntry;

public:

	/*	access, smartPtrs() = false	*/
	void * get(const char *key) const;
	void * get(String &key) const {return get(key.chars());}

	void set(const char *key, const void *data) {
		ASSERT(!smartPtrs());
		setData(key, data);
	}
	void set(String &key, const void *data) { set(key.chars(), data); }

	/*	access, smartPtrs() = true	*/
	void setPtr(const char *key, const PtrBase &ptr) {
		ASSERT(smartPtrs());
		setData(key, &ptr);
	};
	void setPtr(String &key, const PtrBase &ptr) { 
		setPtr(key.chars(), ptr); 
	}
	
	void del(const char *key) {
		setData(key, 0);
	}
	void del(const String &key) {
		del(key.chars());
	}

	PtrBase *getPtr_(const char *key) const;
	PtrBase *getPtr_(String &key) const {return getPtr_(key.chars());}

	void getKeys(StringArray &dest) const;
	void getData(Array<void *> &dest) const;

	/*	Determine if this table is storing smart pointers.
	*/
	bool smartPtrs() const {return smartPtrs_;}
#if DEBUG
	String debInfo() const;
	void dump(
		String *str = 0,
		void (* dumpData)(String &s, const void *data) = 0,
		int (* compareFunc)(const char *aKey, const void *aData,
			const char *bKey, const void *bData) = &HashTable_sortByKeys
	);
#endif

	void clear() {
		entries.clear();
		used_ = deleted_ = 0;
		resize();
	}
private:
	/*	Common set point for smartPtrs() true or false	*/
	void setData(const char *key, const void *data);

	// resize the table
	void resize();
	// determine number of S_VALID items in table
	int length() const {return used_;}
	// find key in table, return location of free entry if not found
	HashEntry * find(const char *key, int *newIndexPtr) const;
	// table of entries
	Array<HashEntry> entries;
	// # keys currently used in table
	int used_;
	// # deleted keys in table
	int deleted_;
	// true if this table contains smart pointers
	bool smartPtrs_;


	/*
		Data structure for hash table entries

		Entries consist of a string (char *) and a pointer to
		user-defined data.
	*/
	class HashEntry {
		// ---------------------------------------
	public:
		// constructor
		HashEntry() {construct();}
		// destructor
		virtual ~HashEntry();
		// copy constructor
		HashEntry(const HashEntry &s) {
			construct();
			*this = s;
		}
		// assignment operator
		HashEntry &operator=(const HashEntry &s);
		// initializer:
	private:
		void construct();
		// ---------------------------------------
	public:
		enum {
			S_NONE,
			S_DELETED,
			S_VALID,
			
			S_STATE = 0x3,
			S_SMART = 0x4,
		};
		void set(int state,  const char *key, const void *data, bool smartPtr);
		const void *getData() const { return data_; }
		const char *getKey() const { return key_.chars(); }
		bool exists() const {	return (state() == S_VALID); }
		bool deleted() const { return (state() == S_DELETED); }
		bool none() const {	return (state() == S_NONE);}

	#if DEBUG
		String debInfo() const;
	#endif

	private:

		void setState(int state) {	flags_ = (flags_ & ~S_STATE) | state;	}
		void setSmart(bool smart) {	flags_ = (flags_ & ~S_SMART) | 
			(smart ? S_SMART : 0); }
		int state() const {	return flags_ & S_STATE; }
		void setData(const void *d);
		void removeExisting();
		bool smartPtrs() const {return (flags_ & S_SMART) != 0; }
		PtrBase *sPtr() const {return (PtrBase *)data_;}

		// hash key string
		String key_;
		// pointer to the user data associated with this key
		const void *data_;
		// state (S_xxx) with S_SMART flag added
		int flags_; 
	};
};

template <typename X>
void get(const HashTable &table, const char *sym, Ptr<X> &out)
{
	downcast(table.getPtr_(sym), out);
}

template <typename X>
void get(const HashTable &table, const String &sym, Ptr<X> &out)
{
	downcast(table.getPtr_(sym.chars()), out);
}

#endif // _HASHTABLE
