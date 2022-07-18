#include "Translator.h"

#include "unicode/utypes.h"
#include "unicode/uchar.h"
#include "unicode/locid.h"
#include "unicode/ustring.h"
#include "unicode/ucnv.h"
#include "unicode/ustdio.h"
#include "unicode/ucsdet.h"

#include "Dictionary.h"
#include <map>

static const std::map<UChar32, UChar> punctationTable = {
    {u'，', u','},
    {u'。', u'.'},
    {u'“', u'\"'},
    {u'”', u'\"'},
    {u'！', u'!'},
    {u'？', u'?'},
    {u'【', u'['},
    {u'】', u']'}
};

void appendAString(icu::UnicodeString& toBeAppended, icu::UnicodeString ExtPart)
{
    auto additionalWS = (!ExtPart.isEmpty() ? u" " : u"");
    toBeAppended += additionalWS + ExtPart;
}

icu::UnicodeString Translator::translateALine(icu::UnicodeString& s)
{
    icu::UnicodeString viet;
    int i = 0;
    int32_t s_length = s.countChar32();
    while (i < s_length)
    {
        int step = 1;
        icu::UnicodeString temp;
        int tmp_end = (vp_dic->getMaxLength() < (s_length - i)) ? (i + vp_dic->getMaxLength()) : s_length;
        //printUnicodeString(outFile, icu::UnicodeString(u" ") + cn.char32At(i) + icu::UnicodeString(u"\n"));
        icu::UnicodeString sub_cn;
        int j;
        for (j = tmp_end; j > 1; --j)
        {
            s.extract(i, j, sub_cn);
            //int sub_cn_length = j - i;
            // Translating using Names
            temp = names_dic->getTranslated(sub_cn);
            if (temp != sub_cn)
            {
                appendAString(viet, temp);
                step = sub_cn.length();
                break;
            }
            // Translating using VP
            temp = vp_dic->getTranslated(sub_cn);
            if (temp != sub_cn)
            {
                //printUnicodeString(outFile, sub_cn + icu::UnicodeString(u" ") + temp + icu::UnicodeString(u"\n"));
                appendAString(viet, temp);
                step = sub_cn.length();
                break;
            }
        }
        /* No substring matches? */
        if (j == 1)
        {
            UChar c = s.char32At(i);
            if (u_isxdigit(c))
            {
                UChar last_c = s.char32At(i - 1);
                //printUnicodeString(outFile, icu::UnicodeString(u" ") + cn.char32At(i) + icu::UnicodeString(u"\n"));
                icu::UnicodeString ws;
                if (   (i - 1) >= 0
                    && (u_isspace(last_c) || !u_isxdigit(last_c) || u_isspace(last_c))) ws = u" ";
                viet += ws + c;
            }
            else if (u_isalnum(c))
            {
                //printUnicodeString(outFile, icu::UnicodeString(u" ") + cn.char32At(i) + icu::UnicodeString(u"\n"));
                sub_cn = icu::UnicodeString(c);
                temp = vp_dic->getTranslated(sub_cn);
                if (temp == sub_cn) temp = hanviets_dic->getTranslated(c);
                appendAString(viet, temp);
            }
            else
            {
                auto new_punc = punctationTable.find(c);
                if (new_punc != punctationTable.end())
                {
                    viet += (*new_punc).second;
                }
                else
                {
                    viet += c;
                    //printUnicodeString(outFile, icu::UnicodeString(u" ") + cn.char32At(i) + icu::UnicodeString(u"\n"));
                }
            }
        }
        i = i + (step <= 0 ? 1 : step);
    }
    return viet;
}

std::vector<std::pair<icu::UnicodeString, icu::UnicodeString>> Translator::TranslateALine(icu::UnicodeString& s)
{
    std::vector<std::pair<icu::UnicodeString, icu::UnicodeString>> result;
    int i = 0;
    int32_t s_length = s.countChar32();
    while (i < s_length)
    {
        int step = 1;
        icu::UnicodeString temp;
        int tmp_end = (vp_dic->getMaxLength() < (s.countChar32() - i)) ? (i + vp_dic->getMaxLength()) : s_length;
        //printUnicodeString(outFile, icu::UnicodeString(u" ") + cn.char32At(i) + icu::UnicodeString(u"\n"));
        icu::UnicodeString sub_cn;
        int j;
        for (j = tmp_end; j > 1; --j)
        {
            s.extract(i, j, sub_cn);
            //printUnicodeString(outfile, sub_s + icu::UnicodeString(u"\n"));
            // Translating using Names
            temp = names_dic->getTranslated(sub_cn);
            if (temp != sub_cn)
            {
                result.emplace_back(std::make_pair(temp, sub_cn));
                step = sub_cn.length();
                break;
            }
            // Translating using VP
            temp = vp_dic->getTranslated(sub_cn);
            if (temp != sub_cn)
            {
                //printUnicodeString(outFile, sub_cn + icu::UnicodeString(u" ") + temp + icu::UnicodeString(u"\n"));
                result.emplace_back(std::make_pair(temp, sub_cn));
                step = sub_cn.length();
                break;
            }
        }
        /* No substring matches? */
        if (j == 1)
        {
            UChar c = s.char32At(i);
            if (u_isxdigit(c))
            {
                //printUnicodeString(outFile, icu::UnicodeString(u" ") + cn.char32At(i) + icu::UnicodeString(u"\n"));
                result.emplace_back(std::make_pair(s.char32At(i), c));
            }
            else if (u_isalnum(c))
            {
                //printUnicodeString(outFile, icu::UnicodeString(u" ") + cn.char32At(i) + icu::UnicodeString(u"\n"));
                sub_cn = icu::UnicodeString(c);
                temp = vp_dic->getTranslated(sub_cn);
                if (temp == sub_cn) temp = hanviets_dic->getTranslated(c);
                result.emplace_back(std::make_pair(temp, c));
            }
            else
            {
                auto new_punc = punctationTable.find(c);
                if (new_punc != punctationTable.end())
                {
                    result.emplace_back(std::make_pair((*new_punc).second, c));
                }
                else
                {
                    result.emplace_back(std::make_pair(s.char32At(i), c));
                    //printUnicodeString(outFile, icu::UnicodeString(u" ") + cn.char32At(i) + icu::UnicodeString(u"\n"));
                }
            }
        }
        i = i + (step >= 0 ? step : 1);
    }
    return result;
}
