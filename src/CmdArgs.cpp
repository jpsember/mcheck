#include "Headers.h"
#include "CmdArgs.h"

#undef pt
#define pt(a) //pr(a)

CmdArgs::CmdArgs(int argc, char **argv,
								 const char *supportedOptions) 
{
	construct();
	if (supportedOptions)
		supportedOptions_.set(supportedOptions);
	addArguments(argc, (const char **)argv, true);
	if (strings_.length() > 0)
		strings_[0].path_toAbstract();
}

String CmdArgs::nextPath() {
	String w;
	w.set(nextValue());
	w.path_toAbstract();
	return w;
}

static void addWord(const String &str, StringArray &sa, int pos, int len, bool addZeroLen = false)
{
	if (addZeroLen || len > 0)
		sa.add(str.subStr(pos,len));
}

void CmdArgs::addArguments(const String &str)
{
	StringArray sa;

	{
		char strDelim = 0;
		int len = 0;
		int pos = 0;
		for (int i = 0; i < str.length(); i++) {
			bool addChar = false;
			char c = str.charAt(i);

			if (strDelim && c == strDelim) {
				strDelim = 0;
				addWord(str,sa,pos,len,true);
				len = 0;
				addChar = false;
			} else if (!strDelim && (c == '\'' || c == '\"')) {
				addWord(str,sa,pos,len);
				len = 0;
				strDelim = c;
			} else if (Utils::isWS(c) && !strDelim) {
				addWord(str,sa,pos,len);
				len = 0;
			} else {
				addChar = true;
			}
			if (addChar) {
				if (len == 0) {
					pos = i;
				}
				len++;
			}
		}
		if (strDelim)
			throw CmdArgException("Missing quote in arguments");
		addWord(str,sa,pos,len);
	}

//	str.split(sa);

#if DEBUG && 0
	for (int i = 0; i < sa.length(); i++) {
		pr(("%d: [%s]\n",i,sa[i].s() ));
	}
#endif

	const char * *list;
	list = new const char *[sa.length() + 1];
	for (int i = 0; i < sa.length(); i++)
		list[i] = sa.itemAt(i).chars();
	addArguments(sa.length(), list);
	delete [] list;
}

void CmdArgs::addArguments(int argc, const char **argv, bool includesExe) {								 
	// parse the arguments as [] executable, [] option, or [] value.

	for (int i = 0; i < argc; i++) {
		const char *s = argv[i];

		// is it an option?  
		if ((!includesExe || i > 0)
			&& isOption(s)
			) {
				while (*++s) {
					String opt("-");
					char oc = *s;
					if (supportedOptions_.length() > 0
						&& String::indexOf(supportedOptions_.chars(),oc) < 0) {
						String e("Unsupported option: ");
						e << argv[i];
						throw CmdArgException(e); 
					}
					pt(("Adding option '%c'\n",oc));
					opt.append(oc);
					strings_.add(opt);
				}
			} else {
				pt(("Adding value '%s'\n",s));
				strings_.add(s);
			}
	}
	if (strings_.length() == 0)
		throw CmdArgException("Missing executable name");

	// Start with first argument (skip the executable); if
	// we've already parsed some, don't change

	int nextArg = (includesExe ? 1 : 0);
	argNumber_ = max(argNumber_, nextArg);
}

void CmdArgs::construct() {
	CONSTRUCT();
	argNumber_ = 0;
}

bool CmdArgs::peekOption(char c)
{
	if (hasNext()) {
		String &s = strings_.itemAt(argNumber_);
		if (isOption(s)
			&& s.charAt(1) == c) {
				pt(("Peeked at option %c\n",c));
				nextOption();
				return true;
			}
	}
	return false;
}

bool CmdArgs::nextIsValue() const {
	return (hasNext() && !isOption(strings_.itemAt(argNumber_)));
}

String &CmdArgs::nextValue() {
	if (!nextIsValue()) {
		pt((" ... nextValue not found\n"));
		String e("Missing value in arguments");
		if (argNumber_ > 0)
			e << " after: " << strings_.itemAt(argNumber_-1);
		throw CmdArgException(e);
	}
	pt(("nextValue returning %s\n",strings_.itemAt(argNumber_).chars() ));

	return strings_.itemAt(argNumber_++);
}

char CmdArgs::nextOption() {
	if (nextIsValue()) {
		pt((" ... nextOption found value instead\n"));
		String e("Unexpected value in arguments: ");
		e << strings_.itemAt(argNumber_);
		throw CmdArgException(e);
	}
	pt(("nextOption returning %s\n",strings_.itemAt(argNumber_).chars() ));
	return strings_.itemAt(argNumber_++).charAt(1);
}

void CmdArgs::done() const {
	if (argNumber_ < strings_.length()) {
		String e("Unexpected arguments: ");
		for (int i = argNumber_; i < strings_.length(); i++) {
			e << strings_.itemAt(i) << " ";
		}
		throw CmdArgException(e);
	}

}
