#include "Headers.h"
#include "Files.h"
#include "Token.h"
#include "DFA.h"

void Token::set(int type, int lineNum, int linePos, const String *srcStr,
	int offset, int len
) {
	type_ = type;
	lineNumber_ = lineNum;
	linePos_ = linePos;
	if (srcStr)
		str_.set(srcStr->chars(),offset,len);
}

Token::Token(int type, int lineNum,	int linePos) {
	construct();
	type_ = type;
	lineNumber_ = lineNum;
	linePos_ = linePos;
}

void Token::construct() {
	type_ = T_EOF;
	lineNumber_ = -1;
	linePos_ = -1;
//	userData_ = -1;
}

void Token::getName(int token, String &dest, const DFA *dfa) {
	dest.clear();
	static const char *typeNames[] = {
			"T_EOF",
			"T_UNKNOWN",
	};

	if (token >= T_TOKEN_TYPES_START && token < T_TOKEN_TYPES_END) {
		dest.set(typeNames[token - T_TOKEN_TYPES_START]);
	}
	else if (dfa != 0) {
		dfa->getTokenName(token, dest);
	}
}

#if DEBUG
String Token::debInfo(const DFA *dfa) const {
	String s;
	getName(type(), s, dfa);
	if (!s.defined()) {
		s << "T_" << type();
	}
	s.pad(14);
	if (lineNumber_ >= 0)
		s << " (#" << fmt(lineNumber_,3) << ":" << fmt(linePos_,3) << ")";
	if (str_.length())
		s << " " << str_;
	return s;
}
#endif

