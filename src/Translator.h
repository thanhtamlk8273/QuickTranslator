#ifndef HEADER_H_TRANSLATOR
#define HEADER_H_TRANSLATOR
#include <unicode/unistr.h>
#include "Dictionary.h"

class Translator
{
private:
	Dictionary* vp_dic;
	Dictionary* names_dic;
	Dictionary* hanviets_dic;
    icu::UnicodeString getLatinString(icu::UnicodeString&& s);
public:
    Translator(): vp_dic(nullptr), names_dic(nullptr), hanviets_dic(nullptr) {}
	Translator(Dictionary* _vp_dic, Dictionary* _names_dic, Dictionary* _hanviets_dic) :
		       vp_dic(_vp_dic), names_dic(_names_dic), hanviets_dic(_hanviets_dic)
	{}
	icu::UnicodeString translateALine(icu::UnicodeString& s);
	std::vector<std::pair<icu::UnicodeString, icu::UnicodeString>>  TranslateALine(icu::UnicodeString& s);
};

#endif
