#ifndef HEADER_H_DICTIONARY
#define HEADER_H_DICTIONARY

#include <unordered_map>
#include <unicode/unistr.h>
#include <string>
#include <set>
#include <vector>
#include <log.h>

template <>
struct std::hash<icu::UnicodeString> {
    const int32_t hash_length = 5;

    std::size_t operator()(icu::UnicodeString const& s) const noexcept
    {
        return s.tempSubString(0, hash_length).hashCode();
    }
};

class Dictionary
{
public:
    /* Member types */
    using size_type = int32_t;
private:
    /* PossibleLengthList definition */
    struct PossibleLengthList {
        /* Data */
        size_type max_len;
        std::set<size_type> length_set;
        std::vector<int> reference_count;
        /* Methods */
        PossibleLengthList(): max_len(0) {};
        void add(size_type len);
        void remove(size_type len);
    };

    class PossibleFirstCharList {
    private:
        std::unordered_map<UChar, int> start_chars_list;
    public:
        /* Methods */
        void add(UChar ch);
        void remove(UChar ch);
        bool find(UChar ch);
    };

    /* Variables */
    Log logger;
	std::string file_name;
    std::unordered_map<icu::UnicodeString, icu::UnicodeString> records;
    PossibleFirstCharList possible_first_chars;
    PossibleLengthList possible_lengths;
public:
    /* Methods */
    Dictionary();
    bool loadFromFile(const std::string& file_name);
    icu::UnicodeString getTranslated(const icu::UnicodeString& text);
    bool isThereARecordStartWith(const char* ch);
    bool isThereARecordStartWith(const UChar& ch);
    size_type getMaxLength();
    size_type getNumberOfRecords();
	void addNewRecord(icu::UnicodeString cn, icu::UnicodeString vn);
	void delRecord(icu::UnicodeString cn);
	void update();
    std::set<size_type>& getLengthSet();
};
#endif
