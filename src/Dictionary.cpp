#include "Dictionary.h"
#include <string>
#include <string_view>
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <fmt/core.h>

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

    std::string toStdString(icu::UnicodeString& s)
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
            length_list.insert(first_part.length());
            ++length_availability[first_part.length()];
            start_char_list.insert(first_part.charAt(0));
        }
    }
    if(length_list.empty() == false)
    {
        auto result = std::minmax_element(length_list.begin(), length_list.end());
        min_len = *(result.first);
        max_len = *(result.second);
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

bool Dictionary::isThereARecordStartWith(const UChar &c)
{
    return (start_char_list.count(c) > 0);
}

int32_t Dictionary::getMaxLength()
{
    return max_len;
}

int32_t Dictionary::getMinLength()
{
    return min_len;
}

int Dictionary::getNumberOfRecords() {
    return records.size();
}

void Dictionary::addNewRecord(icu::UnicodeString cn, icu::UnicodeString vn)
{
    icu::UnicodeString first_part = cn.trim();
    icu::UnicodeString second_part = vn.trim();
    if (first_part.isEmpty()) return;
    /* Add new element */
    auto emplace_result = records[first_part] = second_part;
    start_char_list.insert(first_part.charAt(0));
    length_list.insert(first_part.length());
    ++length_availability[first_part.length()];
    if (first_part.length() > max_len) max_len = first_part.length();
    if (first_part.length() < min_len) min_len = first_part.length();
    log(log_id, LogLevel::LOG_INFO, "new record {} -> {} added", toStdString(cn), toStdString(vn));
}

void Dictionary::delRecord(icu::UnicodeString cn)
{
    icu::UnicodeString first_part = cn.trim();
    if (first_part.isEmpty()) return;
    int erase_count = records.erase(first_part);
    if (erase_count == 0) return;
    if (length_availability[first_part.length()] == 0)
    {
        log(log_id, LogLevel::LOG_ERROR, "Abnormality. availability of the length has reached 0 but we can still delete from records");
        return;
    }
    if((--length_availability[first_part.length()]) == 0)
    {
        length_list.erase(first_part.length());
        /* A length has been deleted from length_list. Re-calculate min and max is necessary */
        if(length_list.empty() == false)
        {
            auto result = std::minmax_element(length_list.begin(), length_list.end());
            min_len = *(result.first);
            max_len = *(result.second);
        }
        start_char_list.erase(first_part.charAt(0));
    }
    log(log_id, LogLevel::LOG_INFO, "record {} deleted", toStdString(cn));
}

void Dictionary::update()
{
    std::ofstream dic_file = std::ofstream(file_name, std::ifstream::binary | std::ifstream::trunc);
    std::string data;
    unsigned long count = 0;
    for (auto& record : records)
    {
        std::string first_part;
        std::string second_part;
        record.first.toUTF8String(first_part);
        record.second.toUTF8String(second_part);
        data += first_part + "=" + second_part + "\n";
        ++count;
    }
    dic_file.write(data.c_str(), data.length());
    dic_file.close();
    log(log_id, LogLevel::LOG_INFO, "{} records was written to file", count);
}

