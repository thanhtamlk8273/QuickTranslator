#include "Dictionary.h"
#include <string>
#include <iostream>
#include <fstream>
#include <fmt/core.h>

class PossibleLengthList;

namespace {
    std::vector<std::string> log_ids;
    enum class LogLevel {
        LOG_INFO = 0,
        LOG_ERROR
    };

    const char* expandLogLevel(LogLevel level)
    {
        switch (level)
        {
            case LogLevel::LOG_INFO: return "[INFO]";
            case LogLevel::LOG_ERROR: return "[ERROR]";
            default: return "[UNKNOWN]";
        }
    }

    template<class... T>
    void log(int id, LogLevel level, const std::string& fmt, T... args)
    {
        if (id > (static_cast<int>(log_ids.size() - 1))) return;
        std::string log_line = fmt::format(fmt + "\n", args...);
        std::cout << std::string(expandLogLevel(level)) << log_ids[id] << log_line;
    }

    int registerLogId(const std::string& log_id)
    {
        log_ids.emplace_back(std::string("[") + log_id + std::string("]"));
        log(log_ids.size() - 1, LogLevel::LOG_INFO, "Register log ID {}", log_id);
        return (log_ids.size() - 1);
    }

    std::string toStdString(const icu::UnicodeString& s)
    {
        std::string result;
        s.toUTF8String(result);
        return result;
    }
}

bool Dictionary::loadFromFile(const std::string& _file_name)
{
    file_name = _file_name;
    log_id = registerLogId(file_name);
    log(log_id, LogLevel::LOG_INFO, "Start to load records from {}", file_name);
    std::ifstream dic_file = std::ifstream(file_name, std::ifstream::binary);
    if (!dic_file.is_open())
    {
        log(log_id, LogLevel::LOG_ERROR, "Unable to open {}", file_name);
        return false;
    }
    std::string line;
    /* And the rest */
    while (std::getline(dic_file, line))
    {
        icu::UnicodeString unicode_line = icu::UnicodeString::fromUTF8(line.c_str());
        auto index = unicode_line.indexOf(u"=");
        auto end_index = (unicode_line.indexOf(u"/") != -1) ? unicode_line.indexOf(u"/") : unicode_line.countChar32();
        if (index == -1) continue;
        icu::UnicodeString first_part = unicode_line.tempSubString(0, index);
        icu::UnicodeString second_part = unicode_line.tempSubString(index+1, end_index - (index + 1));
        if (first_part.trim().isEmpty()) continue;
        /* Add new record */
        auto emplace_result = records.try_emplace(first_part.trim(), second_part.trim());
        if (emplace_result.second)
        {
            possible_lengths.add(first_part.length());
            possible_first_chars.add(first_part.charAt(0));
        }
    }
    log(log_id, LogLevel::LOG_INFO, "{} records loaded", records.size());
    dic_file.close();
    return true;
}

icu::UnicodeString Dictionary::getTranslated(const icu::UnicodeString& text)
{
    if (records.find(text) != records.end())
    {
        return (records.find(text))->second;
    }
    return text;
}

bool Dictionary::isThereARecordStartWith(const char* ch)
{
    return isThereARecordStartWith(icu::UnicodeString::fromUTF8(icu::StringPiece(ch)).char32At(0));
}

bool Dictionary::isThereARecordStartWith(const UChar &ch)
{
    return (possible_first_chars.find(ch) > 0);
}

int Dictionary::getMaxLength()
{
    return possible_lengths.max_len;
}

int Dictionary::getNumberOfRecords() {
    return records.size();
}

void Dictionary::addNewRecord(icu::UnicodeString cn, icu::UnicodeString vn)
{
    if(cn.isEmpty())
    {
        return;
    }
    icu::UnicodeString first_part = cn.trim();
    icu::UnicodeString second_part = vn.trim();
    /* Add new element */
    records[first_part] = second_part;
    possible_first_chars.add(first_part.charAt(0));
    possible_lengths.add(first_part.length());
    log(log_id, LogLevel::LOG_INFO, "new record {} -> {} added", toStdString(cn), toStdString(vn));
}

void Dictionary::delRecord(icu::UnicodeString cn)
{
    icu::UnicodeString first_part = cn.trim();
    if(records.erase(first_part) == 0)
    {
        return;
    }
    possible_lengths.remove(first_part.length());
    possible_first_chars.remove(first_part.charAt(0));
    log(log_id, LogLevel::LOG_INFO, "record {} deleted", toStdString(cn));
}

void Dictionary::update()
{
    std::ofstream dic_file = std::ofstream(file_name, std::ifstream::binary | std::ifstream::trunc);
    std::string data;
    unsigned long count = 0;
    for (auto& record : records)
    {
        data += toStdString(record.first) + "=" + toStdString(record.second) + "\n";
        ++count;
    }
    dic_file.write(data.c_str(), data.length());
    dic_file.close();
    log(log_id, LogLevel::LOG_INFO, "{} records was written to file", count);
}

std::set<int> &Dictionary::getLengthSet()
{
    return possible_lengths.length_set;
}

/******************************/
/* PossibleLengthList methods */
/******************************/
void Dictionary::PossibleLengthList::add(int len)
{
    if(len == 0) return;
    length_set.insert(len);
    if(len > (int) reference_count.size())
    {
        reference_count.resize(len, 0);
    }
    ++reference_count[len-1];
    if(!length_set.empty())
    {
        max_len = *(length_set.rbegin());
    }
}

void Dictionary::PossibleLengthList::remove(int len)
{
    if(len == 0 || len > (int) reference_count.size()) return;
    if(--reference_count[len-1] > 0) {
        return;
    }
    length_set.erase(len);
    max_len = length_set.empty() ? 0 : *(length_set.rbegin());
}

/*********************************/
/* PossibleStartCharList methods */
/*********************************/
void Dictionary::PossibleFirstCharList::add(UChar ch)
{
    auto it = start_chars_list.find(ch);
    if(it != start_chars_list.end())
    {
        ++(it->second).ref_count;
    }
    else
    {
        PossibleFirstChar my_struct {1, ch};
        start_chars_list.emplace(ch, my_struct);
    }
}

void Dictionary::PossibleFirstCharList::remove(UChar ch)
{
    auto it = start_chars_list.find(ch);
    if(it == start_chars_list.end())
    {
        return;
    }
    int& ref_count = (it->second).ref_count;
    --ref_count;
    if(ref_count == 0)
    {
        start_chars_list.erase(ch);
    }
}

bool Dictionary::PossibleFirstCharList::find(UChar ch)
{
    return start_chars_list.find(ch) != start_chars_list.end();
}
