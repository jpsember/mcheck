// debug.cpp
// 
// Miscellanous debugging functions
#include "Headers.h"
#include <stdio.h>
#include <stdarg.h>

#if DEBUG
#if !UNIX && 0
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#endif

#define DEBSTR_LEN (120+1)

#include "HashTable.h"
#include "Files.h"

#define pm(a) //printf a

// display all test case results, not just negative ones?
#define SHOW_TESTS 0

// -------------------------------------------------------------
/*	Class to record information about a memory allocation,
		used for the (debug-only) memory tracking.

		It's possible for two different items to simultaneously have
		the same address, if one is a member variable of the other.
		For this reason, when allocating a pointer that already exists,
		we save the old PtrEntry object using the 'link' variable, and
		restore the entry when removing the pointer from the memory map.
*/
class PtrEntry {
private:
	static const int STRLEN_FILELOC = 25,
		STRLEN_DESCR = 50;
public:
	PtrEntry(const void *p, const char *file, int line, const char *d);
	void setDesc(const char *s);
	static void dump(String &s, const void *data);
	static void constructTableKey(const void *ptr, char *dest);
	PtrEntry *getLink() {return link;}
	void setLink(PtrEntry *e) {link = e;}
	int time() const {return creationTime;}
private:
	// the pointer to the allocated memory
	const void *ptr;
	// order of creation of this pointer
	int creationTime;
	// link to older object with this address, 0 if none
	PtrEntry *link;
	// descriptive string: filename and line number
	char fileLoc[STRLEN_FILELOC+1];
	// optional additional descriptive string
	char descr[STRLEN_DESCR+1];
	
	static int currentTime;
};

/*	Constructor
		> p						address being allocated or removed
		> file				file, line where called from
		> line
		> d						optional description of pointer
*/
PtrEntry::PtrEntry(const void *p, const char *file, int line, const char *d) {

	MEM_DISABLE(true);
	{
		// ------------------
		// Initialize members
		creationTime = 0;
		ptr = 0;
		fileLoc[0] = 0;
		descr[0] = 0;
		link = 0;
		// ------------------

		ptr = p;

		Debug::fileLoc(file,line,fileLoc);
		if (d != 0)
			setDesc(d);
		creationTime = currentTime++;
	}
	MEM_RESTORE();
}

/*	Store a description
		> s						description to store; truncates if necessary
*/
void PtrEntry::setDesc(const char *s) {
	strCopyN(descr, s, STRLEN_DESCR);
}

/*	Construct a hash table key from allocation information.
		> ptr				pointer to allocated item
		> dest			char array to store key within (must support 8 chars + zero)
*/
void PtrEntry::constructTableKey(const void *ptr, char *dest)
{
		sprintf(dest,"%p",ptr);
}

// class variable: number of allocations so far
int PtrEntry::currentTime;

/*	Dump PtrEntry to string; suitable for passing to HashTable::dump()
		> s						String to dump to
		> data				pointer to PtrEntry 
*/
void PtrEntry::dump(String &s, const void *data)
{
	MEM_DISABLE(true);
	{
	PtrEntry &d = *(PtrEntry *)data;

	char work[DEBSTR_LEN];
	sprintf(work,"<%4d> %-16.16s | %-28.28s",
		d.creationTime,
		d.fileLoc,
		d.descr );
	s.set(work);
	}
	MEM_RESTORE();
}
// -------------------------------------------------------------

// Hash table to store warning items within
static HashTable *warnTbl;
static bool warnActive;
void Debug::warn(const char *str, const char *file, int line)
{
	MEM_DISABLE(true);
	if (!warnActive) {
		warnActive = true;

		String s;
		if (str != 0) {
			s.set(str);
		} else {
			char work[DEBSTR_LEN];
			s << "<unspecified " << fileLoc(file,line,work) << ">";
		}
		if (warnTbl == 0) {
			New(warnTbl);
//			warnTbl = _new_ HashTable();
		}

		void *flag = warnTbl->get(s);
		if (flag == 0) {
			static bool flag = true;
			char w2[DEBSTR_LEN];
			
			// use pr macro so warning is sent to DebWindow if
			// in graphics mode
			pr(("--> Warning (%s): %s\n",
				fileLoc(file,line,w2), s.chars() ));

//			Cout << "--> Warning (" << fileLoc(file,line,w2) << "): " <<
//				str << "\n";
			warnTbl->set(s,&flag);
		}
		warnActive = false;
	}
	MEM_RESTORE();
}

static void nameOnly(const char *file, char *dest, bool trimExt = false) {

	int endIndex = -1;
	int extStart = -1;

	for (int i = 0; file[i]; i++) {
		char c = file[i];
		if (c == '/' || c == '\\' || c == ':')
			endIndex = i;
		if (c == '.')
			extStart = i;
	}
	int maxLen = 16;
	if (trimExt && extStart > endIndex)
		maxLen = minVal(maxLen,extStart - (endIndex+1));

	strCopyN(dest,file+endIndex+1,maxLen);
//	dest.set(file,endIndex+1,maxLen);
}

// Hash table to store items allocated in memory. 
static HashTable *memTbl;

// Stack for disabling memory tracking
static const int //DESC_LEN = 80,
	DSTK_SIZE = 10;
static struct {
	bool state;
	int stackPtr;
	bool stack[DSTK_SIZE];
} DS;

/*	Add, modify, or remove an allocation entry in the memory map.  
	Existing entry, if found, is removed.
	> ptr				pointer to allocated item
	> entry				entry to add, or 0 to remove existing entry;
						a copy is made of this entry to store in the table
*/
static void setTableEntry(const void *ptr, PtrEntry *entry) 
{
#undef p2
#define p2(a) //pr(a)

	// don't record any memory manipulations that go on here
	MEM_DISABLE(true);

	ASSERTEXIT(memTbl != 0);

	PtrEntry *newEnt = 0;
	if (entry != 0) {
		NewI(newEnt, *entry);
	}
	p2(("setTableEntry ptr %p --> %p\n",ptr,newEnt));

	char key[20];
	PtrEntry::constructTableKey(ptr,key);

	// find existing entry
	PtrEntry *oldEntry = (PtrEntry *)memTbl->get(key);

	// If entry exists:
	//		if setting entry to 0, delete existing and replace it
	//			with its linked value
	//		else allocate new entry, link to existing
	//

	if (oldEntry != 0) {
		p2((" entry exists, %p --> %p\n",ptr,oldEntry));
		if (newEnt == 0) {
			PtrEntry *link = oldEntry->getLink();
			Delete(oldEntry);
			newEnt = link;
			p2(("  removing existing %p, replacing with linked %p --> %p\n",oldEntry,ptr,newEnt));
		} else {
			p2(("  linking new %p --> %p --> old %p\n",ptr,newEnt,oldEntry));
			newEnt->setLink(oldEntry);
		}
	}
	p2(("  set [%s]->[%p]\n",key,newEnt));
	memTbl->set(key, newEnt);

	MEM_RESTORE();
}

/*	Change the description of an allocated item
	> ptr				address of item
	> msg				new description
*/
void Debug::describeMemItem(const void *ptr, const char *msg)
{
#undef p2
#define p2(a) //pr(a)

	if (ptr != 0 && memTracking()) {
		// don't record any memory manipulations that go on here
		MEM_DISABLE(true);
		ASSERTEXIT(memTbl != 0);

		p2(("describeMemItem %p\n",ptr));

		char key[20];
		PtrEntry::constructTableKey(ptr,key);

		// find existing entry
		PtrEntry *ent = (PtrEntry *)memTbl->get(key);
		p2(("  key [%s] returned ent %p\n",key,ent));

		if (ent == 0) {
			pr(("*** describe %p failed (msg=%s)\n",ptr,msg));
		} else {
			ent->setDesc(msg);
		}

		MEM_RESTORE();
	}
}

#define pa(a) //{pr(("%5d|  ",LV.allocCount)); pr(a);}

/*	Add pointer to allocation list (debug only)
	> file				file called from
	> line				line called from
	> ptr				pointer to add
*/
void Debug::myNew_(const char *file, int line, const void *ptr)
{
	ASSERT(ptr != 0);
	if (memTracking()) {
		// don't record any memory manipulations that go on here
		MEM_DISABLE(true);

		pm(("alloc   %p (%s:%d)\n",ptr,file,line));
		pa((">>> %p, myNew %s:%d\n",ptr,file,line));

		PtrEntry ent(ptr, file, line, 0);
		setTableEntry(ptr, &ent);
		MEM_RESTORE();
	}
}

/*	Remove pointer from allocation list (debug only)
	> file				file called from
	> line				line called from
	> ptr				pointer to remove
*/
void Debug::myDelete_(const char *file, int line, const void *ptr) {

	//pm(("myDelete, %p (%s:%d)\n",ptr,file,line));
	if (ptr != 0 && memTracking()) {
		// don't record any memory manipulations that go on here
		MEM_DISABLE(true);

		pm(("delete  %p (%s:%d)\n",ptr,file,line ));
		pa(("<<< %p, myDelete %s:%d\n",ptr,file,line));

		// construct key
		char key[20];
		PtrEntry::constructTableKey(ptr,key);
		// find existing entry
		PtrEntry *ent = 0;

		ASSERTEXIT(memTbl != 0);
		ent = (PtrEntry *)memTbl->get(key);

		if (ent == 0) {
#if 1 // constructed static items were never added to map,
			// and this is causing problems with DebWindow
			pr(("*** myDelete %p, can't find key [%s]! (%s:%d)\n",ptr,key,
				file,line));
#endif
		} else {
			setTableEntry(ptr, 0);
		}
		MEM_RESTORE();
	}
}

void Debug::openMemory() {

	// Construct a hash table to store information about allocated
	// items in memory.  Disable memory tracking while manipulating
	// the memTbl.
	MEM_DISABLE(true);
	New(memTbl);
//	memTbl = _new_ HashTable();
	MEM_RESTORE();

	pm(("--- start memory tracking ---\n"));
}

void Debug::closeMemory() {

	pm(("--- end memory tracking -----\n"));
	MEM_DISABLE(true);
	// enclose in braces so items are destroyed
	// before MEM_RESTORE() is called.
	{
		ASSERTEXIT(memTbl != 0);
		StringArray sa;
		memTbl->getKeys(sa);
		if (sa.length() != 0) {
			if (sa.length() > 8)
				pr(("Possible memory leak, dumping to _memdump_.txt\n"));
			String s;
			dumpMemory(&s);
			TextWriter::writeString(s,"_memdump_.txt");
		}
		Delete(memTbl);
	}
	MEM_RESTORE();
}

void Debug::restoreMemoryTracking() {
	DS.stackPtr--;
	ASSERTEXIT(DS.stackPtr >= 0);
	DS.state = DS.stack[DS.stackPtr];
}

void Debug::disableMemoryTracking(bool flag) {
	ASSERTEXIT(DS.stackPtr < DSTK_SIZE);
	DS.stack[DS.stackPtr++] = DS.state;
	if (flag) {
		DS.state = true;
	}
}

// note no 'static' keyword here in the definition.

/*	Test an assertion
	> file				file called from
	> line				line called from
	> flag				test that this flag is nonzero
	> msg					if not 0, message to print if assertion fails
	> exitIfFail	if true, and assertion fails, program exits immediately
*/
void Debug::myAssert(const char *file, int line, bool flag, const char *msg,
										 bool exitIfFail) {
	if (!msg) {
		msg = "ASSERT";
	}
	if (!flag) {

		pr(("....assertion failure....\n"));
		if (exitIfFail) {
			cout << "*** Critical assertion failure (" << file << ": " << line << ") "
				 << msg << "\n";
			exit(1);
		}
		String m;
		m << "(" << Debug::fileLoc(file,line) << ") ";
		m << msg;

		throw AssertException(m);
	} else {
	//	printf(" <assertion passed: (%s:%d) %s>\n",file,line,msg);
	}
}

bool Debug::memTracking() {
	return (memTbl != 0) && !DS.state;
}

/*	Compare function for displaying memory hash table in order of
		creation time

		> a						pointer to hashed data
		> b						pointer to hashed data
		< int					result of a - b (0 = equal)
*/
//static int comparePtrEntries(const char *aKey, const void *aData,
//														 const char *bKey, const void *bData) {
//	PtrEntry *pa = (PtrEntry *)aData;
//	PtrEntry *pb = (PtrEntry *)bData;
//
//	return (pa->time() - pb->time());
//}

void Debug::dumpMemory(String *strPtr) {

	// disable memory tracking, since we don't want the table to be
	// modified while it's being printed!
	MEM_DISABLE(1);
	ASSERTEXIT(memTbl != 0);
	{
		memTbl->dump(
			strPtr,
			&PtrEntry::dump,
			0
		);
	}
	MEM_RESTORE();
}

/*	Convert a filename & line number to a shorter form.  Path information
		is stripped off.
			file[c:/home/projects/work.cpp] line=32 ==> work.cpp     32

		> file							filename
		> line							line number
		> dest							String to store result in; must fit ~20 chars
		< dest							
*/
char *Debug::fileLoc(const char *file, int line, char *dest)
{
	char name[DEBSTR_LEN];
	nameOnly(file,name,true);
	String::lower(name);
	sprintf(dest,"%-10.10s %4d",name,line);
	return dest;
}

String Debug::fileLoc(const char *file, int line)
{
	char w[DEBSTR_LEN];
	fileLoc(file,line,w);
	return String(w);
}

bool Debug::testResults(const char *goalFile, const String& results, 
		const char *file, int line)
{
	// construct path from goalFile id
	String path("testcases/_test_");
	path << goalFile;
	path.path_setExt("txt");
	path.lower();

	bool showTests = SHOW_TESTS;

	// if goal file doesn't exist, create it.
	if (!FileObject::exists(path.chars())) {
		Cout << "Creating goal:  (" << fileLoc(file,line) << ") " << path << "\n"
			<< results.indent() << "\n";
		showTests = false;
		TextWriter::writeString(results,path);
	}
	
	String goal;
	TextReader::readToString(path,goal);

	bool pass = (results.equals(goal));
	if (!pass) {
		Cout << "*** Failed test (" << fileLoc(file,line) << ") " << path << "\n"
			<< "  Got:\n" << results.indent() << "\n"
			<< "  Goal:\n" << goal.indent() << "\n\n";
	}
	else {
		Cout << "Passed test:    (" << fileLoc(file,line) << ") " << path << "\n";
		if (showTests)
			Cout << results.indent() << "\n";
	}
	return pass;
}

// allocate it on the stack, as we don't want it
// being freed on exit, since we'll be using it to
// print
static Array<char> *charBuffer = 0;
static bool inUse;
void My_printf(const char *fmt, ...)
{

	// this method is attempting to re-enter itself

	bool wasInUse = inUse;
	Array<char> *cb = 0;
	if (!inUse) {
		cb = charBuffer;
		inUse = true;
		charBuffer = 0;
	}

	if (cb == 0) 
		New(cb);

	int max_str_len = 5000;
	while (cb->length() < 1+max_str_len)
		cb->add(0);

	char *chars = const_cast<char *>(cb->array());

	va_list arg_ptr;
	va_start(arg_ptr, fmt);
#if UNIX
	vsnprintf(chars, max_str_len, fmt, arg_ptr);
#else
	_vsnprintf(chars, max_str_len, fmt, arg_ptr);
#endif
	va_end(arg_ptr);

//	if (activeSink == 0)
		//cout << chars;
	//else
	Utils::getActiveSink() << chars;
//	*activeSink << chars;

	if (wasInUse) {
		Delete(cb);
	} else {
		charBuffer = cb;
	}
	inUse = false;
}

static int strNum = 0;
static Array<String> strQueue;
String &Debug::str()
{	
	strQueue.ensureCapacity(100,false);
	if (strNum >= strQueue.length()) {
		strQueue.add(String());
	}
	String &out = strQueue.itemAt(strNum);
	//out.ensureCapacity(100,false);
	if (++strNum == 100)
		strNum = 0;
	out.clear();
	return out;
}

#if !UNIX && 0
static SYSTEMTIME timerStart, timerEnd;
static long ticks(const SYSTEMTIME &t) {
	long v = t.wMinute;
	v = v*60 + t.wSecond;
	v = v*1000 + t.wMilliseconds;

	//pr(("tMinute=%d sec=%d mil=%d v=%d\n",t.wMinute,t.wSecond,t.wMilliseconds,v));

	return v;
}

const char *Debug::startTimer() {
	GetSystemTime(&timerStart);
	return "(starting timer)";
}
const char *Debug::endTimer() {
	GetSystemTime(&timerEnd);

	long d = ticks(timerEnd) - ticks(timerStart);
	String &s = Debug::str();
	char work[20];
	sprintf(work,"%d.%03d sec",d/1000,d%1000);
	s << work;
	return s.chars();
}
#else
const char *Debug::startTimer() {return "";}
const char *Debug::endTimer() {
	return "*** timer not supported in linux";
}
#endif

#endif	// DEBUG

