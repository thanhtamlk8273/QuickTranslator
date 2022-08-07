#include "Translator.h"

#include "unicode/utypes.h"
#include "unicode/uchar.h"
#include "unicode/locid.h"
#include "unicode/ustring.h"
#include "unicode/ucnv.h"
#include "unicode/ustdio.h"
#include "unicode/ucsdet.h"
#include "unicode/schriter.h"

#include "Dictionary.h"
#include <map>
#include <iostream>
#include <algorithm>
#include <array>

namespace {
    static const std::map<UChar32, UChar32> punctationTable = {
        {u'，', u','},
        {u'。', u'.'},
        {u'“', u'\"'},
        {u'”', u'\"'},
        {u'！', u'!'},
        {u'？', u'?'},
        {u'【', u'['},
        {u'】', u']'},
        {u'、', u','}
    };

    UChar32 getPunc(const UChar32 punc)
    {
        UChar32 replacement = punc;
        auto result = punctationTable.find(punc);
        if (result != punctationTable.end())
        {
            replacement = (*result).second;
        }
        return replacement;
    };

    void appendAString(icu::UnicodeString& toBeAppended, icu::UnicodeString ExtPart)
    {
        auto additionalWS = (!ExtPart.isEmpty() ? u" " : u"");
        toBeAppended += additionalWS + ExtPart;
    }
}

icu::UnicodeString Translator::getLatinString(icu::UnicodeString&& s)
{
    icu::UnicodeString result;
    icu::StringCharacterIterator it(s);
    for(it.setToStart(); it.hasNext();)
    {
        UChar32 c=it.next32PostInc();
        if(ublock_getCode(c) != UBLOCK_BASIC_LATIN) break;
        result += c;
    }
    return result;
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
        icu::UnicodeString sub_cn;
        int j;
        for (j = tmp_end; j > 1; --j)
        {
            sub_cn = s.tempSubString(i, j);
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
                appendAString(viet, temp);
                step = sub_cn.length();
                break;
            }
        }
        /* No substring matches? */
        if (j == 1)
        {
            UChar c = s.charAt(i);
            if (u_isxdigit(c))
            {
                UChar last_c = s.charAt(i - 1);
                icu::UnicodeString ws;
                if (   (i - 1) >= 0
                    && (u_isspace(last_c) || !u_isxdigit(last_c) || u_isspace(last_c))) ws = u" ";
                viet += ws + c;
            }
            else if (u_isalnum(c))
            {
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
                }
            }
        }
        i = i + (step <= 0 ? 1 : step);
    }
    return viet;
}

void updatePossibleLengthList(std::set<int>& my_list, int value)
{
    std::set<int>::iterator it;
    for(it = my_list.begin(); it != my_list.end();)
    {
        if(*it > value)
        {
            my_list.erase(it++);
        }
        else
        {
            ++it;
        }
    }
}

std::vector<std::pair<icu::UnicodeString, icu::UnicodeString>> Translator::TranslateALine(icu::UnicodeString& s)
{
    std::set<int> names_possible_lengths = names_dic->getLengthSet();
    std::set<int> vps_possible_lengths = vp_dic->getLengthSet();
    std::vector<std::pair<icu::UnicodeString, icu::UnicodeString>> result;
    int i = 0;
    int32_t s_length = s.length();
    while (i < s_length)
    {
        int max_length = (vp_dic->getMaxLength() < (s_length - i)) ? (i + vp_dic->getMaxLength()) : s_length;
        /* Update possible lengths lists */
        updatePossibleLengthList(names_possible_lengths, max_length);
        updatePossibleLengthList(vps_possible_lengths, max_length);
        /* prepare possible substrings */
        std::vector<icu::UnicodeString> subcn_vec(max_length);
        for(int j = 1; j < max_length + 1; ++j)
        {
            subcn_vec[j-1] = s.tempSubString(i, j);
        }
        /* Initialize important variables */
        int notTranslated = true;
        int step = 1;
        /* Names have higher priority */
        bool matchInNames = (names_dic->isThereARecordStartWith(s.charAt(i)));
        if(matchInNames)
        {
            std::set<int>::reverse_iterator it;
            for(it = names_possible_lengths.rbegin(); it != names_possible_lengths.rend(); ++it)
            {

                int len = *it;
                icu::UnicodeString cn = names_dic->getTranslated(subcn_vec[len-1]);
                if(cn != subcn_vec[len-1])
                {
                    result.emplace_back(cn, subcn_vec[len-1]);
                    step = subcn_vec[len-1].length();
                    notTranslated = false;
                    break;
                }
            }
        }
        /* Search in VP */
        bool matchInVP = (vp_dic->isThereARecordStartWith(s.charAt(i)));
        if(notTranslated && matchInVP)
        {
            std::set<int>::reverse_iterator it;
            for(it = vps_possible_lengths.rbegin(); it != vps_possible_lengths.rend(); ++it)
            {
                int len = *it;
                icu::UnicodeString cn = vp_dic->getTranslated(subcn_vec[len-1]);
                if(cn != subcn_vec[len-1])
                {
                    result.emplace_back(cn, subcn_vec[len-1]);
                    step = subcn_vec[len-1].length();
                    notTranslated = false;
                    break;
                }
            }
        }
        /* No substring matches? */
        if (notTranslated)
        {
            UChar c = s.char32At(i);
            if(u_ispunct(c))
            {
                /* With punctation, we will find its equivalent in punctationTable
                 * Then append it to the last transaltion unit if there is one
                 * Otherwise, make it a new unit */
                if(result.empty() == false)
                {
                    result.back().first.append(getPunc(c));
                    result.back().second.append(getPunc(c));
                }
                else
                {
                    result.emplace_back(getPunc(c), c);
                }
            }
            else if (ublock_getCode(c) == UBLOCK_BASIC_LATIN)
            {
                icu::UnicodeString latinString = getLatinString(s.tempSubString(i, max_length));
                result.emplace_back(latinString, latinString);
                step = latinString.length();
            }
            else
            {
                result.emplace_back(hanviets_dic->getTranslated(c), c);
            }
        }
        i = i + (step > 0 ? step : 1);
    }
    return result;
}
