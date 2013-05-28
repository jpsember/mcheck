#ifndef _MEMORY
#define _MEMORY

/*
	Memory tracking
	---------------

	To avoid memory leaks, we keep track of which items have been
	allocated or constructed.

	1) initializing 

	The macros
		MEM_OPEN();
		{
		}
		MEM_CLOSE();

	should be placed at the start and end of the main() method.
	There should be a set of braces between them so items constructed
	after MEM_OPEN() go out of scope and are destructed before MEM_CLOSE().
		
	2) tracking items on the free store

	There are these cases to consider:
		I)	allocating a single primitive (built-in) type with new:

							int *j = new int;
								:
								:
							delete j;
							j = 0;

				Becomes:

							int *j;
							New(j);
							:
							:
							Delete(j);


				If the value *j is to be initialized, use the NewI macro:

							int *j = new int(7);

				Becomes:

							int *j;
							NewI(j,7);

				A separate macro is required since the same macro cannot
				take different numbers of arguments, unlike function names.

		II)	allocating a class instance with new (this is treated the
				same way as in (I)):

							String *j = new String("Jeff");
								:
								:
							delete j;
							j = 0;

				Becomes:

							String *j;
							NewI(j,"Jeff");
								:
								:
							Delete(j);

		III) allocating an array of primitives or class instances:

							int *array = new int[80];
								:
								:
							delete [] array;
							array = 0;

				Becomes:

							int *array;
							NewArray(array, 80);
							  :
								:
							DeleteArray(array);

				These special macros are necessary to distinguish between
				allocation of a single item and allocating an array.

	The usual 'new' operator should only be used when constructing
	smart pointers:

				Ptr<String> s(new String("Cabbage"));

	(This creates a smart pointer, s, which points to the String "Cabbage".)
	The smart pointer takes care of calling New, and will call Delete
	automatically when the smart pointer's reference count becomes zero.


	In every class constructor & destructor, call CONSTRUCT()
	and DESTROY():

				MyClass::MyClass() {
						CONSTRUCT();
						 :
						 :
				}

				MyClass::~MyClass() {
						DESTROY();
				}
	
	The information printed in the memory dump for each pointer
	includes the file and line number where the CONSTRUCT or
	New / NewI / NewArray macro was called.

	Additional descriptive information can be added for a pointer
	by using the DESC macro:
	
		DESC(ptr,msg)    // description = msg

	For arrays, use a different macro:

		DESCARRAY(buffer,"state names");

	This is necessary because the pointer stored by the NewArray
	call is actually modified from that stored by the New call, so
	to look up the pointer to describe it, the pointer must be
	modified by the DESCARRAY call.

	3) disabling tracking

	To avoid storing tracking information, the tracking functions
	can be temporarily disabled by using the MEM_DISABLE(x) and
	MEM_RESTORE() macros. They should surround a block, so local
	objects can be destroyed while the disable state is still in effect:
	 
		MEM_DISABLE(f)	:	disables tracking if f is true
		{
			... put code between blocks ...
		}
		MEM_RESTORE()	:	restores value of tracking in effect
							before MEM_DISABLE called

	These act like a stack, so the two calls should be balanced.
	
	This ability to disable tracking is used so the hash tables that
	track memory usage are not themselves changing the information
	being stored.

	4) printing tracking-related debug information

	The macro MEM_DUMP() displays memory usage.

	In non-DEBUG mode, none of these macros generate code.
*/


//	Functions to support the memory tracking calls.

#undef _FL_
#if DEBUG
#define _FL_     const char *file, int line,
#else
#define _FL_
#endif

#if DEBUG
// When passing an array address to the debug memory tracker,
// we flip a high bit of its address so an imbalanced pair of calls
// will be detected:
//     New/DeleteArray    is imbalanced, and the DeleteArray address
//                         probably won't be found, since it doesn't
//                         match the New address.
#define ARRAY_MOD_VALUE 0x8000
#endif

template <typename X, typename Y>
void myNew(_FL_   X *&ptr, Y initValue) {
	ptr = new X(initValue);
#if DEBUG
	Debug::myNew_(file,line,ptr);
#endif
}

template <typename X>
void myNew(_FL_ 	 X *&ptr) {
	ptr = new X;
	//out << "alloc new item, ptr " << ptr << "\n";
#if DEBUG
	Debug::myNew_(file,line,ptr);
#endif
}

template <typename X>
void myDelete(_FL_  X *&ptr) {
	ASSERT(ptr != 0);
#if DEBUG
	Debug::myDelete_(file,line,ptr);
#endif
	delete ptr;
	ptr = 0;
}

template <typename X>
void myNewArray(_FL_  X *&ptr, int length) {
	ptr = new X[length];
#if DEBUG
	Debug::myNew_(file,line,((char *)ptr) + ARRAY_MOD_VALUE);
#endif
}

template <typename X>
void myDeleteArray(_FL_  X *&ptr) {
	ASSERT(ptr != 0);
#if DEBUG
	Debug::myDelete_(file,line,((char *)ptr) + ARRAY_MOD_VALUE);
#endif
	//pr(("...delete ptr %p\n",ptr));
	delete [] ptr;
	ptr = 0;
}

#undef _FL_

#if DEBUG

#define NewI(ptr, val) myNew(__FILE__,__LINE__,ptr,val)
#define New(ptr) myNew(__FILE__,__LINE__,ptr)
#define Delete(ptr) myDelete(__FILE__,__LINE__,ptr)
#define NewArray(ptr, length) myNewArray(__FILE__,__LINE__,ptr,length)
#define DeleteArray(ptr) myDeleteArray(__FILE__,__LINE__,ptr)
#define DESCARRAY(a, b) Debug::describeMemItem(((char *)(a)) + ARRAY_MOD_VALUE, (b))

#define CONSTRUCT() Debug::myNew_(__FILE__,__LINE__,this)
#define DESTROY() Debug::myDelete_(__FILE__,__LINE__,this)

#define DESC(ptr, msg) Debug::describeMemItem(ptr,msg)

#define MEM_OPEN() Debug::openMemory()
#define MEM_CLOSE() Debug::closeMemory()
#define MEM_DISABLE(a) Debug::disableMemoryTracking(a)
#define MEM_RESTORE() Debug::restoreMemoryTracking()
#define MEM_DUMP() Debug::dumpMemory()

#else

#define NewI(ptr, val) myNew(ptr,val)
#define New(ptr) myNew(ptr)
#define Delete(ptr) myDelete(ptr)
#define NewArray(ptr, length) myNewArray(ptr,length)
#define DeleteArray(ptr) myDeleteArray(ptr)
#define DESCARRAY(a, b) 

#define CONSTRUCT()
#define DESTROY()
#define DESC(ptr, msg) 
#define MEM_OPEN()
#define MEM_CLOSE()
#define MEM_DISABLE(a)
#define MEM_RESTORE()
#define MEM_DUMP()

#endif	// DEBUG

#endif // _MEMORY
