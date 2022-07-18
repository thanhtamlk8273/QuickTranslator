#include <memory>

#include <stdio.h>
#include "unicode/utypes.h"
#include "unicode/uchar.h"
#include "unicode/locid.h"
#include "unicode/ustring.h"
#include "unicode/ucnv.h"
#include "unicode/unistr.h"
#include "unicode/ustdio.h"
#include "unicode/ucsdet.h"

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <chrono>

#include "Dictionary.h"
#include "Translator.h"
std::ifstream inFile;
std::ofstream outFile;

void printUnicodeString(std::ofstream& out, const icu::UnicodeString& s) {
    std::string u8string;
    s.toUTF8String(u8string);
    out.write(u8string.c_str(), u8string.length());
}

void exit_function(std::string message, int error_code)
{
    std::cout << "[ERROR] " << message << "\n";
    exit(error_code);
}

int main()
{
    auto vp_dic = std::make_unique<Dictionary>();
    auto Names = std::make_unique<Dictionary>();
    auto HanViets = std::make_unique<Dictionary>();
    auto testDic = std::make_unique<Dictionary>();
    vp_dic->loadFromFile("VietPhrase.txt");
    Names->loadFromFile("Names.txt");
    HanViets->loadFromFile("ChinesePhienAmWords.txt");
    Translator my_translator(vp_dic.get(), Names.get(), HanViets.get());

    inFile = std::ifstream("target.txt", std::ifstream::binary);
    outFile = std::ofstream("output.txt", std::ofstream::binary);
    if (!inFile.is_open())
    {
        exit_function("Failed to open target.txt to read", 1);
    }
    if (!outFile.is_open())
    {
        exit_function("Failed to open output.txt to write to", 1);
    }
    icu::UnicodeString cn;
    icu::UnicodeString viet;
    icu::UnicodeString s(u"【提取群】983209522");
    std::cout << "start translating" << std::endl;
    auto startT = std::chrono::high_resolution_clock::now();
    std::string line;
    long line_count = 0;
    while (std::getline(inFile, line))
    {
        auto unicode_line = icu::UnicodeString::fromUTF8(line.c_str()).trim();
        auto result = my_translator.translateALine(unicode_line);
        viet += u"    " + result + u"\r\n";
        ++line_count;
    }
    printUnicodeString(outFile, viet);
    std::cout << "Done loading target.txt. Start translating\n";
    auto endT = std::chrono::high_resolution_clock::now();
    std::cout << "Writing translation result to file\n";
    std::chrono::duration<double> diff = (endT - startT);
    std::cout << diff.count() << "\n";
    return 0;
}