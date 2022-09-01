#include <Dictionary.h>
#include <gtest/gtest.h>

TEST(TestDictionary, EmptyString) {
    Dictionary my_dic;
    my_dic.loadFromFile("");
    EXPECT_EQ(my_dic.getMaxLength(), 0);
    EXPECT_EQ(my_dic.getNumberOfRecords(), 0);
    EXPECT_EQ(my_dic.getTranslated(u"LOL"), icu::UnicodeString(u"LOL"));
    EXPECT_EQ(my_dic.getTranslated(u""), icu::UnicodeString(u""));
}

TEST(TestDictionary, BasicTests) {
    Dictionary my_dic;
    ASSERT_TRUE(my_dic.loadFromFile("test_vp.txt"));
    ASSERT_EQ(my_dic.getNumberOfRecords(), 8);
    ASSERT_EQ(my_dic.getMaxLength(), 16);
    EXPECT_EQ(my_dic.getTranslated(u"LOL"), icu::UnicodeString(u"LOL"));
    EXPECT_EQ(my_dic.getTranslated(u"空 ── 间 ── 凝 ── 固"), icu::UnicodeString(u"không ── gian ── ngưng ── cố"));
    EXPECT_EQ(my_dic.getTranslated(u"[ 醉 ][ 露 ][ 网 ]"), icu::UnicodeString(u""));
    EXPECT_EQ(my_dic.getTranslated(u"MarionetteHeart"), icu::UnicodeString(u"Marionette Heart"));
    EXPECT_EQ(my_dic.getTranslated(u"在天願做比翼鳥在地願做連理枝"), icu::UnicodeString(u"trên trời nguyện làm chim liền cánh, dưới đất nguyện làm cây liền cành"));
    EXPECT_EQ(my_dic.getTranslated(u"abc"), icu::UnicodeString(u"abc"));
    EXPECT_EQ(my_dic.getTranslated(u"摸 usquetaires"), icu::UnicodeString(u"mousquetaires"));
    EXPECT_EQ(my_dic.getTranslated(u"wENxuEmI. cOM"), icu::UnicodeString(u""));
    EXPECT_EQ(my_dic.getTranslated(u"刚刚"), icu::UnicodeString(u"vừa mới"));
}

TEST(TestDictionary, AddingRecords) {
    Dictionary my_dic;
    ASSERT_TRUE(my_dic.loadFromFile("test_vp.txt"));
    ASSERT_EQ(my_dic.getNumberOfRecords(), 8);
    ASSERT_EQ(my_dic.getMaxLength(), 16);
    /* Add new one */
    my_dic.addNewRecord(u"YWA", u"Trần Thanh Tâm");
    EXPECT_EQ(my_dic.getNumberOfRecords(), 9);
    EXPECT_EQ(my_dic.getMaxLength(), 16);
    EXPECT_EQ(my_dic.getTranslated(u"YWA"), icu::UnicodeString(u"Trần Thanh Tâm"));
    /* Add new one with empty first part */
    my_dic.addNewRecord(u"", u"GODTO");
    EXPECT_EQ(my_dic.getNumberOfRecords(), 9);
    EXPECT_EQ(my_dic.getMaxLength(), 16);
    EXPECT_EQ(my_dic.getTranslated(u""), icu::UnicodeString(u""));
    /* Add new one with empty second part */
    my_dic.addNewRecord(u"YAHOO", u"");
    EXPECT_EQ(my_dic.getNumberOfRecords(), 10);
    EXPECT_EQ(my_dic.getMaxLength(), 16);
    EXPECT_EQ(my_dic.getTranslated(u"YAHOO"), icu::UnicodeString(u""));
    /* Increase max length */
    my_dic.addNewRecord(u"YAHOOYAHOOYAHOOYAHOO", u"KATOO");
    EXPECT_EQ(my_dic.getNumberOfRecords(), 11);
    EXPECT_EQ(my_dic.getMaxLength(), 20);
    EXPECT_EQ(my_dic.getTranslated(u"YAHOOYAHOOYAHOOYAHOO"), icu::UnicodeString(u"KATOO"));
    /* Reduce min length */
    my_dic.addNewRecord(u"Y", u"KWAOOH");
    EXPECT_EQ(my_dic.getNumberOfRecords(), 12);
    EXPECT_EQ(my_dic.getMaxLength(), 20);
    EXPECT_EQ(my_dic.getTranslated(u"Y"), icu::UnicodeString(u"KWAOOH"));
    /* Add new one */
    my_dic.addNewRecord(u"三亿两千万又九百七四万两千三百四十", u"329,742,340");
    EXPECT_EQ(my_dic.getNumberOfRecords(), 13);
    EXPECT_EQ(my_dic.getMaxLength(), 20);
    EXPECT_EQ(my_dic.getTranslated(u"三亿两千万又九百七四万两千三百四十"), icu::UnicodeString(u"329,742,340"));
}

TEST(TestDictionary, DeletingRecords) {
    Dictionary my_dic;
    ASSERT_TRUE(my_dic.loadFromFile("test_vp.txt"));
    ASSERT_EQ(my_dic.getNumberOfRecords(), 8);
    ASSERT_EQ(my_dic.getMaxLength(), 16);
    /* Delete new one */
    my_dic.delRecord(u"摸 usquetaires");
    EXPECT_EQ(my_dic.getNumberOfRecords(), 7);
    EXPECT_EQ(my_dic.getMaxLength(), 16);
    EXPECT_EQ(my_dic.getTranslated(u"摸 usquetaires"), icu::UnicodeString(u"摸 usquetaires"));
    /* Delete the longest one */
    my_dic.delRecord(u"空 ── 间 ── 凝 ── 固");
    EXPECT_EQ(my_dic.getNumberOfRecords(), 6);
    EXPECT_EQ(my_dic.getMaxLength(), 15);
    EXPECT_EQ(my_dic.getTranslated(u"空 ── 间 ── 凝 ── 固"), icu::UnicodeString(u"空 ── 间 ── 凝 ── 固"));
    /* Delete the smallest one */
    my_dic.delRecord(u"刚刚");
    EXPECT_EQ(my_dic.getNumberOfRecords(), 5);
    EXPECT_EQ(my_dic.getMaxLength(), 15);
    EXPECT_EQ(my_dic.getTranslated(u"刚刚"), icu::UnicodeString(u"刚刚"));
}

TEST(TestDictionary, TestLengthSet) {
    Dictionary my_dic;
    ASSERT_TRUE(my_dic.loadFromFile("test_vp.txt"));
    ASSERT_EQ(my_dic.getNumberOfRecords(), 8);
    ASSERT_EQ(my_dic.getMaxLength(), 16);
    /* Basic test */
    std::set<int> tmp{16, 15, 14, 13, 2};
    EXPECT_EQ(my_dic.getLengthSet(), tmp);
    /* Adding a new element with length that does not present in length_list */
    tmp.insert(3);
    my_dic.addNewRecord(u"YWA", u"Trần Thanh Tâm");
    EXPECT_EQ(my_dic.getNumberOfRecords(), 9);
    EXPECT_EQ(my_dic.getLengthSet(), tmp);
    /* Delete an element */
    tmp.erase(3);
    my_dic.delRecord(u"YWA");
    EXPECT_EQ(my_dic.getNumberOfRecords(), 8);
    EXPECT_EQ(my_dic.getLengthSet(), tmp);
    EXPECT_EQ(my_dic.getMaxLength(), 16);
    /* Delete longest one */
    tmp.erase(16);
    my_dic.delRecord(u"空 ── 间 ── 凝 ── 固");
    EXPECT_EQ(my_dic.getNumberOfRecords(), 7);
    EXPECT_EQ(my_dic.getLengthSet(), tmp);
    EXPECT_EQ(my_dic.getMaxLength(), 15);
    /* Delete shortest one */
    tmp.erase(2);
    my_dic.delRecord(u"刚刚");
    EXPECT_EQ(my_dic.getNumberOfRecords(), 6);
    EXPECT_EQ(my_dic.getLengthSet(), tmp);
    EXPECT_EQ(my_dic.getMaxLength(), 15);
    /* Add new longtest one */
    tmp.insert(20);
    my_dic.addNewRecord(u"YAHOOYAHOOYAHOOYAHOO", u"KATOO");
    EXPECT_EQ(my_dic.getNumberOfRecords(), 7);
    EXPECT_EQ(my_dic.getLengthSet(), tmp);
    EXPECT_EQ(my_dic.getMaxLength(), 20);
    /* Add multiple records with the same length */
    tmp.insert(2);
    my_dic.addNewRecord(u"刚刚", u"vừa mới");
    EXPECT_EQ(my_dic.getNumberOfRecords(), 8);
    EXPECT_EQ(my_dic.getLengthSet(), tmp);
    EXPECT_EQ(my_dic.getMaxLength(), 20);
    EXPECT_EQ(my_dic.getTranslated(u"刚刚"), icu::UnicodeString(u"vừa mới"));

    my_dic.addNewRecord(u"HH", u"KJSHD");
    EXPECT_EQ(my_dic.getNumberOfRecords(), 9);
    EXPECT_EQ(my_dic.getLengthSet(), tmp);
    EXPECT_EQ(my_dic.getMaxLength(), 20);
    EXPECT_EQ(my_dic.getTranslated(u"HH"), icu::UnicodeString(u"KJSHD"));
    /* Delete one of multiple records with the same length */
    my_dic.delRecord(u"HH");
    EXPECT_EQ(my_dic.getNumberOfRecords(), 8);
    EXPECT_EQ(my_dic.getLengthSet(), tmp);
    EXPECT_EQ(my_dic.getMaxLength(), 20);
    EXPECT_EQ(my_dic.getTranslated(u"HH"), icu::UnicodeString(u"HH"));
    /* Delete all records with the same length */
    tmp.erase(2);
    my_dic.delRecord(u"刚刚");
    EXPECT_EQ(my_dic.getNumberOfRecords(), 7);
    EXPECT_EQ(my_dic.getLengthSet(), tmp);
    EXPECT_EQ(my_dic.getMaxLength(), 20);
    EXPECT_EQ(my_dic.getTranslated(u"刚刚"), icu::UnicodeString(u"刚刚"));
    /* Delete an nonexistent record */
    my_dic.delRecord(u"刚刚");
    EXPECT_EQ(my_dic.getNumberOfRecords(), 7);
    EXPECT_EQ(my_dic.getLengthSet(), tmp);
    EXPECT_EQ(my_dic.getMaxLength(), 20);
    /* One more time */
    my_dic.delRecord(u"刚刚");
    EXPECT_EQ(my_dic.getNumberOfRecords(), 7);
    EXPECT_EQ(my_dic.getLengthSet(), tmp);
    EXPECT_EQ(my_dic.getMaxLength(), 20);
    /* Now add it back */
    tmp.insert(2);
    my_dic.addNewRecord(u"刚刚", u"vừa mới");
    EXPECT_EQ(my_dic.getNumberOfRecords(), 8);
    EXPECT_EQ(my_dic.getLengthSet(), tmp);
    EXPECT_EQ(my_dic.getMaxLength(), 20);
    /* Then remove it again */
    tmp.erase(2);
    my_dic.delRecord(u"刚刚");
    EXPECT_EQ(my_dic.getNumberOfRecords(), 7);
    EXPECT_EQ(my_dic.getLengthSet(), tmp);
    EXPECT_EQ(my_dic.getMaxLength(), 20);
}

TEST(TestDictionary, TestFirstCharAvailability) {
    /* Macros */
    auto validateData = [&](Dictionary& dic, std::map<icu::UnicodeString, bool>& cases_to_validate) {
        for(std::pair<const icu::UnicodeString, bool>& a_case : cases_to_validate)
        {
             EXPECT_EQ(dic.isThereARecordStartWith(a_case.first.char32At(0)), a_case.second);
        }
    };
    /* Method starts here */
    Dictionary my_dic;
    ASSERT_TRUE(my_dic.loadFromFile("test_vp.txt"));
    /* Basic test */
    std::map<icu::UnicodeString, bool> my_vec;
    my_vec[icu::UnicodeString("空")] = true;
    my_vec[icu::UnicodeString("[")] = true;
    my_vec[icu::UnicodeString("M")] = true;
    my_vec[icu::UnicodeString("在")] = true;
    my_vec[icu::UnicodeString("黑")] = true;
    my_vec[icu::UnicodeString("w")] = true;
    my_vec[icu::UnicodeString("摸")] = true;
    my_vec[icu::UnicodeString("刚")] = true;
    validateData(my_dic, my_vec);
    /* Add a new one */
    my_dic.addNewRecord("YWA", "Trần Thanh Tâm");
    my_vec[icu::UnicodeString("Y")] = true;
    validateData(my_dic, my_vec);
    /* Delete it */
    my_dic.delRecord("YWA");
    my_vec[icu::UnicodeString("Y")] = false;
    validateData(my_dic, my_vec);
    /* Delete another one */
    my_dic.delRecord("空 ── 间 ── 凝 ── 固");
    my_vec[icu::UnicodeString("空")] = false;
    validateData(my_dic, my_vec);
    /* Yet another one */
    my_dic.delRecord("刚刚");
    my_vec[icu::UnicodeString("刚")] = false;
    validateData(my_dic, my_vec);
    /* Add some duplicate values */
    my_dic.addNewRecord("在刚刚", u"WOWLUL");
    my_dic.addNewRecord("在在在在", u"WOW");
    my_vec[icu::UnicodeString("在")] = true;
    validateData(my_dic, my_vec);
    /* Remove a record start with "在" */
    my_dic.delRecord("在天願做比翼鳥在地願做連理枝");
    my_vec[icu::UnicodeString("在")] = true;
    /* Remove all records start with "在" */
    my_dic.delRecord("在刚刚");
    my_dic.delRecord("在在在在");
    my_vec[icu::UnicodeString("在")] = false;
    validateData(my_dic, my_vec);
    /* Remove all records start with "在" */
    my_dic.delRecord("在刚刚");
    my_dic.delRecord("在在在在");
    my_dic.delRecord("在天願做比翼鳥在地願做連理枝");
    my_vec[icu::UnicodeString("在")] = false;
    validateData(my_dic, my_vec);
    /* Remove non-existence record */
    my_dic.delRecord("在翼鳥在");
    my_vec[icu::UnicodeString("在")] = false;
    validateData(my_dic, my_vec);
}
