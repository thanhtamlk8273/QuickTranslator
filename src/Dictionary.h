#ifndef HEADER_H_DICTIONARY
#define HEADER_H_DICTIONARY

#include <unicode/uconfig.h>
#include <unordered_map>
#include <unicode/unistr.h>
#include <chrono>
#include <string>
#include <unordered_set>
#include <set>
#include <fstream>

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
	int log_id;
	std::string file_name;
	std::unordered_map<icu::UnicodeString, icu::UnicodeString> records;
	int max_len;
	int min_len;
    std::set<int> length_list;
    std::unordered_map<int, int> length_availability;
    std::unordered_set<UChar> start_char_list;
public:
	Dictionary(): log_id(-1), max_len(0), min_len(100) {};
    bool loadFromFile(const std::string& file_name);
    icu::UnicodeString getTranslated(const icu::UnicodeString& text);
    bool isThereARecordStartWith(const UChar& c);
    int getMaxLength();
	int getMinLength();
	int getNumberOfRecords();
	void addNewRecord(icu::UnicodeString cn, icu::UnicodeString vn);
	void delRecord(icu::UnicodeString cn);
	void update();
	/* For testing purpose */
    std::set<int>& getLengthList() { return length_list; }
	std::unordered_map<int, int>& getLengthAvailability() { return length_availability; }
};

#endif
