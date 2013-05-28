/*	Smart Pointer class
*/
#include "Headers.h"
#include "PtrBase.h"

PtrBase::PtrBase() {
	construct(); 
}

PtrBase::~PtrBase() {
	DESTROY();
}

bool PtrBase::isNull() const {
	return (counter_ == 0) || (counter_->ptr_ == 0);
}

void PtrBase::construct() {
	CONSTRUCT();
 	counter_ = 0;
}

PtrBase& PtrBase::operator=(const PtrBase &s) {
	ASSERT2(false, "**** This shouldn't be called?");
	return *this;
}

#if DEBUG


String PtrBase::debInfo() const {
	String s;
	s << "PtrBase ";
	if (isNull()) {
		s << "<null>";
	} else {
		s << counter_->debInfo();
	}
	return s;
}
#endif

#if DEBUG
String RefCount::debInfo() const {
	String s;
	s << "RefCount C#" << count_ << " ";
	if (!ptr_) {
		s << "<null pointer>";
	} else {
		// Don't print actual pointers, since test cases 
		// won't be constant
		s << "...ptr.."; //ptrStr(ptr_);
	}
	return s;
}
#endif
