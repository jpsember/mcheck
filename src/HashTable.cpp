#include "Headers.h"
#include "Ptr.h"
#include "HashTable.h"

/* Constructor
*/
HashTable::HashTable(bool smartPtrs)
{
	construct();
	smartPtrs_ = smartPtrs;
	resize();
}

/*	Initializer
*/
void HashTable::construct() {
	CONSTRUCT();

	used_ = 0;
	deleted_ = 0;
}

/* Destructor
*/
HashTable::~HashTable()
{
	DESTROY();
}

/*	Get data associated with key
	> key				key value
	< ptr to data associated with key, 0 if none
*/
void * HashTable::get(const char *key) const
{
	ASSERT(!smartPtrs());
	const void *out = 0;
	HashEntry *e = find(key, 0);
	if (e != 0)
		out = e->getData();
	return const_cast<void *>(out);
}

PtrBase * HashTable::getPtr_(const char *key) const
{
	ASSERT(smartPtrs());
	const PtrBase *out = 0;
	HashEntry *e = find(key, 0);
	if (e != 0)
		out = (PtrBase *)e->getData();
	return const_cast<PtrBase *>(out);
}

/*	Find entry in hash table.
	
		> key								key to look for
		> newIndexPtr				if not 0, index of next available entry 
												returned here, if entry not found
    < pointer to entry, 0 if not found
*/
HashTable::HashEntry *HashTable::find(const char *key, int *newIndexPtr) const
{
#undef pt
#define pt(a) //pr(a)

	HashEntry *found = 0;

	pt(("find key=%s\n",key));

	int newIndex = -1;

	// look for key.
	int i = Utils::hashFunction(key);
	ASSERT(i >= 0);

	int step = 1;
	while (true) {

		int capacity = entries.length();

		int pos = Utils::mod(i, capacity);
		if (i < 0)
			i += capacity;

		// perform quadratic probing
		i = pos + step*step;
		ASSERT(i >= 0);
		step++;

		HashEntry &ent = entries.itemAt(pos);
		if (	
				 !ent.exists()
			&& newIndex < 0
		) {
			newIndex = pos;
		}

		// have we reached the end of the entries?
		if (ent.none()) {
			break;
		}
		
		if (!ent.exists()) {
			continue;
		}

		if (!strcmp(key,ent.getKey())) {
			found = &ent;
			break;
		}
	}

	pt((" found=%p newIndex=%d\n",found,newIndex));

	if (newIndexPtr != 0)
		*newIndexPtr = newIndex;

	return found;
}

/*	Set data associated with key.  Existing data (if any) is replaced.
	> key				key value
	> data				data to associate with key; 0 to remove
						key from table
*/
void HashTable::setData(const char *key, const void *data)
{
#undef pt
#define pt(a) //pr(a)

	pt(("HashTable, setData key=%s data=%p\n",key,data));

	do {
		// find this key in the table

		int newIndex;
		HashTable::HashEntry *ent = find(key, &newIndex);
		pt(("  find returned %p, newIndex %d\n",ent,newIndex));

		// are we removing existing data?
		if (data == 0) {
			pt(("   removing existing data\n"));
			// if no key found, done.
			// otherwise, set it to deleted.
			if (ent != 0) {
				pt(("    setting S_DELETED\n"));
				ent->set(HashEntry::S_DELETED,0,0,smartPtrs_);
				deleted_++;
				used_--;
			}
		} else {
			// if key found, replace data.
			if (ent != 0) {
				pt(("  replacing existing S_VALID\n"));
				ent->set(HashEntry::S_VALID,key,data,smartPtrs_);
			} else {
				pt(("  no key found; storing at index %d\n",newIndex));
				ent = &entries.itemAt(newIndex);
				if (ent->deleted()) {
					deleted_--;
				}
				ent->set(HashEntry::S_VALID,key,data,smartPtrs_);
				used_++;
			}
		}

		resize();
		
	} while (false);
}


/*
	Resize the hash table 
*/
void HashTable::resize()
{
#undef pt
#define pt(a) //pr(a)

	const int INITIAL_CAPACITY = 250;

	pt(("Resizing hash table, used=%d, del %d, capacity %d\n",used_,deleted_, entries.length()));

	int minCapacity = (used_ + deleted_) * 2;
	if (minCapacity < INITIAL_CAPACITY)
		minCapacity = INITIAL_CAPACITY;

	int optCapacity = used_ * 5;
	if (optCapacity < minCapacity) {
		optCapacity = minCapacity;
	}

//	int maxCapacity = optCapacity * 2;

	int capacity = entries.length();
	if (!(capacity < minCapacity 
		// reenable to have table shrink:
//		|| capacity > maxCapacity
	))
		return;
	int newCapacity = optCapacity;
	pt(("resize from %d to %d\n",entries.length(),newCapacity));

	// construct a table of existing keys & data items
	Array<HashEntry> entList;
	pt((" resizing to capacity %d\n",newCapacity));
	entList.resize(newCapacity);
	pt((" storing empty items\n"));
	HashEntry emptyItem;

//	int newUsed = 0;
	for (int i = 0; i < entries.length(); i++) {
		HashEntry &e = entries.itemAt(i);
		// store pointers to existing entries
		if (e.exists()) {
			pt(("  adding existing item %d to new table\n",i));
			entList.add(e);
		}
	}

	entries.clear();
	for (int i = 0; i < newCapacity; i++)
		entries.add(emptyItem);

	for (int i = 0; i < entList.length(); i++) {
		HashEntry &en = entList.itemAt(i);
		int newIndex;
#if DEBUG
		HashTable::HashEntry *e = 
#endif
			find(en.getKey(), &newIndex);
		ASSERT(e == 0);
		pt((" storing item %d in table at %d\n",i,newIndex));
		entries.set(newIndex, en);
	}
	used_ = entList.length();
	deleted_ = 0;
}

/*	Store the keys for the hash table in an array.
*/
void HashTable::getKeys(StringArray &dest) const
{
	dest.clear();
	dest.ensureCapacity(used_);
	for (int i = 0; i < entries.length(); i++) {
		HashEntry &e = entries.itemAt(i);
		if (e.exists()) {
			dest.add(e.getKey());
		}
	}
}

void HashTable::getData(Array<void *> &dest) const
{
	dest.clear();
	dest.ensureCapacity(used_);
	for (int i = 0; i < entries.length(); i++) {
		HashEntry &e = entries.itemAt(i);
		if (e.exists()) {
			const void *d = e.getData();
			dest.add(const_cast<void *>(d));
		}
	}
}


#if DEBUG
/*	Get a description of this object
*/
String HashTable::debInfo() const
{
	String s;
	int capacity = entries.length();
	s << "HashTable used " << used_ << " of " << capacity << ", deleted " << deleted_;

#if 0
	for (int i = 0; i < capacity; i++)
		s << "Entry " << fmt(i,3) << ": " << entries[i].debInfo() << "\n";
#endif
	return s;
}
#endif


#if DEBUG
int HashTable_sortByKeys(const char *aKey, const void *aData,
			const char *bKey, const void *bData)
{
	return String::compareFunc(aKey,bKey);
}

void HashTable_dumpHex(String &s, const void *data)
{
	Utils::pushSink(&s);
	Utils::hexDump(data,8,8,true);
	Utils::popSink();
}

void HashTable_dumpChars(String &s, const void *data)
{
	char *src = (char *)data;
	s.append(src);
}

void HashTable_dumpInt(String &s, const void *data)
{
	int j = *(int *)data;
	s << fmt(j);
}

void HashTable_dumpString(String &s, const void *data)
{
	String &src = *(String *)data;
	s.append(src);
}
void HashTable_dumpHex(String &s, const void *data);

/*
		Dump hash table to a string, or to stdout
		> str								string to dump to, or 0 for dump to stdout
		> dumpData					pointer to function that dumps associated data,
												 or 0 for default function
		> compareFunc				sort comparison function for key+data pairs, 
												or 0 for no sorting
*/
void HashTable::dump(
		String *strPtr,
		void (* dumpData)(String &s, const void *data),
		int (* compareFunc)(const char *aKey, const void *aData,
			const char *bKey, const void *bData)
) {
#undef pt
#define pt(a) //pr(a)

	pt(("Dump hashTable %p, dumpData=%p, smartPtrs=%s\n",this,dumpData,bs(smartPtrs_) ));

	String dumpDest;
	bool toStr = (strPtr != 0);
	if (!toStr)
		strPtr = &dumpDest;
	String &str = *strPtr;

	if (dumpData == 0 && !smartPtrs_) {
		dumpData = &HashTable_dumpHex;
	}

	static const char *dashedLine = "--------------------------------------"
			"-------------------------------------------\n";

	str << debInfo() << "\n";
	str << dashedLine;

	StringArray keys;
	Array<void *> data;
	getKeys(keys);
	getData(data);

	if (compareFunc != 0) {
		for (int i = 0; i < keys.length(); i++) {
			for (int j = i+1; j < keys.length(); j++) {
				if (compareFunc(keys.itemAt(i).chars(),data.itemAt(i),
												keys.itemAt(j).chars(),data.itemAt(j)	) > 0) {
					String temp = keys.itemAt(i);
					keys.set(i,keys.itemAt(j));
					keys.set(j,temp);

					void * temp2 = data.itemAt(i);
					data.set(i,data.itemAt(j));
					data.set(j,temp2);
				}
			}
		}
	}

	String work;
	for (int i = 0; i < keys.length(); i++) {

		String s = keys.itemAt(i);
		s.truncate(12,true);
		s.pad(12);
		str << s << " --> ";

		if (dumpData != 0) {
			work.clear();
			dumpData(work, data.itemAt(i));
			str << work;
		} else {
			PtrBase *smart = (PtrBase *)data.itemAt(i);
			str << smart->debInfo();
		}
		str << "\n";
	}
	str << dashedLine;
	str << "\n";

	if (!toStr)
		Cout << dumpDest;
}

/*	Test of HashTable class
*/
#include "Files.h"
void Test_HashTable() {

	int tests = 7;
	int t0 = 0, t1 = tests-1;

	for (int test = t0; test <= t1; test++) switch (test) {

		case 0: 
			{
				// Store pointers to String objects.

				String r("Pointers to strings (non-smart)\n");

				HashTable tbl;
				
				r << "n\n constructing data strings...\n";

				String action("ACTION");
				String horror("HORROR");
				String drama("DRAMA");
				String comedy("COMEDY");
				String suspense("SUSPENSE");

				r << "\n\n adding items to hash table...\n";

				tbl.set("Dr No",&action);
				tbl.set("Halloween",&horror);
				tbl.set("Hard Target",&action);
				tbl.set("Silence of the Lambs",&suspense);
				tbl.set("Naked Gun",&comedy);

				r << "\n\n";

				tbl.dump(&r,HashTable_dumpString);
				if (test == 0)
					Test("Htbl0",r);
			}
			break;

		case 2:
			{
				String r("HashTable of int's (non-smart)\n");

				String corpus;
				TextReader::readToString("corpus.txt",corpus);
				corpus.truncate(200);
				r << "Corpus:\n" << corpus << "\n============\n";

				HashTable tbl;
				StringArray sa;
				corpus.split(sa);

				// associate with each word its last word position in the corpus.

				for (int i = 0; i < sa.length(); i++) {
					int *data = (int *)tbl.get(sa.itemAt(i));
					if (data == 0) {
						NewI(data,0);
						tbl.set(sa.itemAt(i), data);
					}
					*data = i;
				} 
				tbl.dump(&r,&HashTable_dumpInt);

				// free up all the counts
				Array<void *> counts;
				tbl.getData(counts);
				while (!counts.isEmpty()) {
					int *iPtr = (int *)counts.pop();
					Delete(iPtr);
				}
				Test("Htbl2",r);
			}
			break;

		case 3:
			{
				String r("HashTable of character strings (non-smart)\n");

				HashTable tbl;
				static const char *str1 = "ONE",
					*str2 = "TWO";

				tbl.set("beta",str2);
				tbl.set("alpha",str1);
				tbl.set("charlie",str2);

				tbl.dump(&r,&HashTable_dumpChars);
				Test("Htbl3",r);
			}
			break;


		case 4:
			{
				String r("Storing smart pointers to strings in HashTable\n");
				
				HashTable tbl(true);

				r << "Constructed hash table: " << tbl.debInfo() << "\n\n";

				{
					StringPtr s(new String("Telegraph"));
					StringPtr t(new String("Road"));
					StringPtr v(new String("Sultans"));
					StringPtr w(new String("Brothers"));

					r << "\nStringPtrs:\n" << s.debInfo() << "\n" << t.debInfo() << "\n" 
						<< v.debInfo() << "\n" << w.debInfo() << "\n\n";

					tbl.setPtr("Dire",s);
					tbl.setPtr("Straits",v);
					tbl.setPtr("In Arms",w);

					r << "\nStringPtrs:\n" << s.debInfo() << "\n" << t.debInfo() << "\n" 
						<< v.debInfo() << "\n" << w.debInfo() << "\n\n";

					r << "Stored some items: " << tbl.debInfo() << "\n";
					tbl.dump(&r);

					tbl.setPtr("Dire",StringPtr(new String("Down to the")));
					tbl.setPtr("In Arms",StringPtr(new String("waterline")));

					r << "Changed some items: " << tbl.debInfo() << "\n\n";
					tbl.dump(&r);

					tbl.del("Dire");
					tbl.del("Straits");

					r << "\nReplaced two strings with null\n";
					tbl.dump(&r);

					r << "\ngoing out of scope, should free up string refs\n";
				}
				r << "\n\ngoing out of scope, will free up tbl\n";
				Test("Htbl4",r);
			}
			break;

		case 5:
			{
				String r;
				r << "Using smart pointers to count frequency of words in corpus\n";

				StringArray sa;
				{
					String s(
						"the time has come the walrus said to speak of many things "
						"of shoes and ships and sealing wax of cabbages and kings "
						"of why the sea is boiling hot and whether pigs have wings"
						);
					s.split(sa);
				}
				HashTable tbl(true);

				for (int i = 0; i < sa.length(); i++) {
					String &w = sa.itemAt(i);

					int cnt = 0;

					Ptr<int> count;
					get(tbl, w, count);
//					Ptr<int> *count = (Ptr<int> *)tbl.getPtr(w);

					if (count.isNull()) {
						tbl.setPtr(w, Ptr<int>(new int(++cnt)));
					} else {
						cnt = ++(*count);
					}
					w.pad(10);
					r << fmt(cnt,4) << " " << w.subStr(0,10);
					if (((i+1) % 5) == 0)
						r << "\n";
				}
				r << "\n";
				tbl.dump(&r);

				Test("Htbl5",r);
			}
			break;
	
		case 6:
			{
				String r("2-character bigrams\n");

				String s;
				TextReader::readToString("corpus.txt",s);
				r << "Corpus: " << s << "\n";

				HashTable tbl;
			
				// associated data = String
				char prev = '\0';
				for (int i = 0; i < s.length(); i++) {
					char c = String::upper(s.charAt(i));
					if (c > ' ' && prev > ' ') {
					
						// construct the bigram
						String key = s.subStr(i-1,2);

						// determine pointer for this bigram.
						String *iPtr = (String *)tbl.get(key);

						// if not defined, allocate a new counter
						if (iPtr == 0) {
							New(iPtr);
							tbl.set(key,iPtr);
						}

						// increment its frequency
						iPtr->append('*');
					}
					prev = c;
				}
				r << "dumping table...\n";
				tbl.dump(&r, &HashTable_dumpString);
				r << "about to exit...\n";

				// free up all the Strings in the hash table
				StringArray keys; 
				Array<void *> counts;
				r << "Getting keys...\n";
				tbl.getKeys(keys);
				r << "getting data...\n";
				tbl.getData(counts);

				for (int i = 0; i < keys.length(); i++) {
					r << keys.itemAt(i) << " --> " << *(String *)counts.itemAt(i) << "\n";
				}

				r << "Freeing up data...\n";

				while (!counts.isEmpty()) {
					String *iPtr = (String *)counts.pop();

					Delete(iPtr);
				}
				Test("Htbl6",r);
			}
			break;
	}
}

#endif	// DEBUG

// HashEntry definitions

/*	Constructor
*/
void HashTable::HashEntry::construct()
{
	CONSTRUCT();

	flags_ = S_NONE;
	data_ = 0;
}

/*	Destructor
*/
HashTable::HashEntry::~HashEntry(void)
{
	DESTROY();
	removeExisting();
}

HashTable::HashEntry &HashTable::HashEntry::operator=(const HashEntry &s)
{
	if (&s != this) {
		removeExisting();

		flags_ = s.flags_;
		key_ = s.key_;

		if (smartPtrs()) {
				// if it's a null Pointer, just copy it
			if (s.data_ == 0)
				data_ = s.data_;
			else {
				// otherwise, make a copy of this pointer
				data_ = s.sPtr()->makeCopy_();
			}
		} else {
			data_ = s.data_;
		}
	}
	return *this;
}

/*	Remove existing key, clear data pointer to 0
*/
void HashTable::HashEntry::removeExisting()
{
	setState(S_NONE);

	if (smartPtrs()) {
		// if pointer to data exists, decrement its reference counter
		// and clear to nullPointer
		if (data_ != 0) {
			PtrBase *p = sPtr();
			Delete(p);
			data_ = 0;
		}
	} else {
		data_ = 0;
	}
}

/*	Set hash table entry
		> state				new state (S_xxx)
		> k						string for key (if S_VALID)
		> d						pointer to user data (if S_VALID)
*/
void HashTable::HashEntry::set(int state,  const char *key, const void *data, bool smartPtr)
{
	removeExisting();
	setSmart(smartPtr);

	if (state == S_VALID) {
		key_.set(key);
		setData(data);
	}
	setState(state);
}

void HashTable::HashEntry::setData(const void *data) {
	if (smartPtrs()) {
		if (data == 0) {
			data_ = 0;
		} else {
			PtrBase *smart = (PtrBase *)data;
			data_ = smart->makeCopy_();
		}
	} else {
		data_ = data;
	}
}


#if DEBUG
String HashTable::HashEntry::debInfo() const
{
	String s;
	if (state() == S_NONE) {
		s << "<NONE>";
	} else if (state() == S_DELETED) {
		s << "<DELETED>";
	} else {
		s.append(key_,0,12);
	}
	s.pad(16);

	String s2;
	s2 << "HashEntry " << s;
	if (state() == S_VALID) {
		s2 << " --> ";

		if (smartPtrs() ) {
			s2 << sPtr()->debInfo();
		} else {
			s << ptrStr(data_);
		}
	}
	return s2;
}
#endif
