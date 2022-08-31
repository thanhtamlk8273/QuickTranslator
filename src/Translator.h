#ifndef HEADER_H_TRANSLATOR
#define HEADER_H_TRANSLATOR
#include <unicode/unistr.h>
#include "Dictionary.h"

class Translator
{
private:
    Dictionary* vp_dic = nullptr;
    Dictionary* names_dic = nullptr;
    Dictionary* hanviets_dic = nullptr;
    icu::UnicodeString getLatinString(icu::UnicodeString&& s) const;
public:
    Translator() = default;
	Translator(Dictionary* _vp_dic, Dictionary* _names_dic, Dictionary* _hanviets_dic) :
		       vp_dic(_vp_dic), names_dic(_names_dic), hanviets_dic(_hanviets_dic)
	{}
    icu::UnicodeString translateALine(const icu::UnicodeString& s) const;
    std::vector<std::pair<icu::UnicodeString, icu::UnicodeString>>  TranslateALine(const icu::UnicodeString& s) const;
};

#endif
