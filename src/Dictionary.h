#ifndef HEADER_H_DICTIONARY
#define HEADER_H_DICTIONARY

#include <unordered_map>
#include <unicode/unistr.h>
#include <string>
#include <set>
#include <vector>

template <>
struct std::hash<icu::UnicodeString> {
    std::size_t operator()(icu::UnicodeString const& s) const noexcept
    {
        return s.hashCode();
    }
};

class Dictionary
{
private:
    /* PossibleLengthList definition */
    struct PossibleLengthList {
        /* Data */
        int max_len;
        std::set<int> length_set;
        std::vector<int> reference_count;
        /* Methods */
        PossibleLengthList(): max_len(0) {};
        void add(int len);
        void remove(int len);
    };

    /* FirstCharList definition */
    struct PossibleFirstChar {
        int ref_count;
        UChar ch;
    };

    class PossibleFirstCharList {
        /* Data */
    private:
        std::unordered_map<UChar, PossibleFirstChar> start_chars_list;
    public:
        /* Methods */
        void add(UChar ch);
        void remove(UChar ch);
        bool find(UChar ch);
    };

    /* Variables */
	int log_id;
	std::string file_name;
	std::unordered_map<icu::UnicodeString, icu::UnicodeString> records;
    PossibleFirstCharList possible_first_chars;
    PossibleLengthList possible_lengths;
public:
    Dictionary(): log_id(-1) {};
    bool loadFromFile(const std::string& file_name);
    icu::UnicodeString getTranslated(const icu::UnicodeString& text);
    bool isThereARecordStartWith(const char* ch);
    bool isThereARecordStartWith(const UChar& ch);
    int getMaxLength();
	int getMinLength();
	int getNumberOfRecords();
	void addNewRecord(icu::UnicodeString cn, icu::UnicodeString vn);
	void delRecord(icu::UnicodeString cn);
	void update();
    std::set<int>& getLengthSet();
};
#endif
