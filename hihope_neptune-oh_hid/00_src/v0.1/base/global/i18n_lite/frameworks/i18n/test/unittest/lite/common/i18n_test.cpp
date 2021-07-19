/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "i18n_test.h"
#include <gtest/gtest.h>
#include "date_time_format.h"
#include "iostream"
#include "locale_info.h"
#include "number_format.h"
#include "plural_format.h"
#include "types.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS::I18N;

namespace {
class I18NTest : public testing::Test {
public:
    void SetUp() const;
    void TearDown() const;
};

void I18NTest::SetUp(void) const
{
}

void I18NTest::TearDown(void) const
{
}

/**
 * @tc.name: I18nFuncTest001
 * @tc.desc: Test LocaleInfo constructor1
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest001, TestSize.Level1)
{
    LocaleInfo *locale = new LocaleInfo("zh", "Hans", "CN");
    EXPECT_TRUE(locale != nullptr);
    delete locale;
}

/**
 * @tc.name: I18nFuncTest002
 * @tc.desc: Test LocaleInfo constructor2
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest002, TestSize.Level1)
{
    LocaleInfo *locale = new LocaleInfo("zh", "CN");
    EXPECT_TRUE(locale != nullptr);
    delete locale;
}

/**
 * @tc.name: I18nFuncTest003
 * @tc.desc: Test LocaleInfo default Constructor
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest003, TestSize.Level1)
{
    LocaleInfo *locale = new LocaleInfo();
    EXPECT_TRUE(locale != nullptr);
    delete locale;
}

/**
 * @tc.name: I18nFuncTest004
 * @tc.desc: Test LocaleInfo copy constructor
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest004, TestSize.Level1)
{
    LocaleInfo locale("zh", "CN");
    LocaleInfo locale2(locale);
    EXPECT_TRUE(strcmp(locale2.GetLanguage(), "zh") == 0);
    EXPECT_TRUE(strcmp(locale2.GetRegion(), "CN") == 0);
}

/**
 * @tc.name: I18nFuncTest005
 * @tc.desc: Test LocaleInfo equals function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest005, TestSize.Level1)
{
    LocaleInfo locale("zh", "CN");
    LocaleInfo locale2("zh", "CN");
    EXPECT_TRUE(locale == locale2);
}

/**
 * @tc.name: I18nFuncTest006
 * @tc.desc: Test LocaleInfo GetLanguage function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest006, TestSize.Level1)
{
    LocaleInfo locale("ar", "AE");
    EXPECT_TRUE(strcmp(locale.GetLanguage(), "ar") == 0);
}

/**
 * @tc.name: I18nFuncTest007
 * @tc.desc: Test LocaleInfo GetRegion function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest007, TestSize.Level1)
{
    LocaleInfo locale("ar", "AE");
    EXPECT_TRUE(strcmp(locale.GetRegion(), "AE") == 0);
}

/**
 * @tc.name: I18nFuncTest008
 * @tc.desc: Test LocaleInfo GetScript function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest008, TestSize.Level1)
{
    LocaleInfo locale("zh", "Hans", "CN");
    EXPECT_TRUE(strcmp(locale.GetScript(), "Hans") == 0);
}

/**
 * @tc.name: I18nFuncTest009
 * @tc.desc: Test LocaleInfo GetId function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest009, TestSize.Level1)
{
    LocaleInfo locale("zh", "Hans", "CN");
    EXPECT_TRUE(strcmp(locale.GetId(), "zh-Hans-CN") == 0);
}

/**
 * @tc.name: I18nFuncTest010
 * @tc.desc: Test LocaleInfo IsDefaultLocale function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest010, TestSize.Level1)
{
    LocaleInfo locale("zh", "Hans", "CN");
    EXPECT_TRUE(!locale.IsDefaultLocale());
    LocaleInfo locale2("en", "US");
    EXPECT_TRUE(locale2.IsDefaultLocale());
}

/**
 * @tc.name: I18nFuncTest011
 * @tc.desc: Test DateTimeFormat Constructor
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest011, TestSize.Level1)
{
    LocaleInfo locale("zh", "Hans", "CN");
    AvailableDateTimeFormatPattern pattern = AvailableDateTimeFormatPattern::HOUR_MINUTE;
    DateTimeFormat *formatter = new DateTimeFormat(pattern, locale);
    EXPECT_TRUE(formatter != nullptr);
    delete formatter;
}

/**
 * @tc.name: I18nFuncTest012
 * @tc.desc: Test DateTimeFormat Init function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest012, TestSize.Level1)
{
    LocaleInfo locale("zh", "Hans", "CN");
    AvailableDateTimeFormatPattern pattern = AvailableDateTimeFormatPattern::HOUR_MINUTE;
    DateTimeFormat formatter(pattern, locale);
    EXPECT_TRUE(formatter.Init());
}

/**
 * @tc.name: I18nFuncTest013
 * @tc.desc: Test DateTimeFormat Format function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest013, TestSize.Level1)
{
    LocaleInfo locale("zh", "", "");
    std::string out = "";
    time_t now = 3600 * 3 + 3600 * 24 * 6;
    I18nStatus status =  I18nStatus::ISUCCESS;
    AvailableDateTimeFormatPattern pattern = AvailableDateTimeFormatPattern::FULL;
    DateTimeFormat formatter(pattern, locale);
    formatter.Format(now, "-1:45", out, status);
    std::string expect = "1970年1月7日星期三";
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest014
 * @tc.desc: Test DateTimeFormat Format function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest014, TestSize.Level1)
{
    LocaleInfo locale("ar", "", "");
    std::string out = "";
    time_t now = 3600 * 3 + 3600 * 24 * 6;
    I18nStatus status =  I18nStatus::ISUCCESS;
    AvailableDateTimeFormatPattern pattern = AvailableDateTimeFormatPattern::FULL;
    DateTimeFormat formatter(pattern, locale);
    formatter.Format(now, "-1:45", out, status);
    std::string expect = "الأربعاء، ٧ يناير، ١٩٧٠";
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest015
 * @tc.desc: Test DateTimeFormat Format function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest015, TestSize.Level1)
{
    LocaleInfo locale("en", "", "US");
    std::string out = "";
    time_t now = 3600 * 3 + 3600 * 24 * 6;
    I18nStatus status =  I18nStatus::ISUCCESS;
    AvailableDateTimeFormatPattern pattern = AvailableDateTimeFormatPattern::FULL;
    DateTimeFormat formatter(pattern, locale);
    formatter.Format(now, "-1:45", out, status);
    std::string expect = "Wednesday, January 7, 1970";
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest016
 * @tc.desc: Test DateTimeFormat ApplyPattern function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest016, TestSize.Level1)
{
    LocaleInfo locale("zh", "", "");
    std::string out = "";
    time_t now = 3600 * 3 + 3600 * 24 * 6;
    I18nStatus status =  I18nStatus::ISUCCESS;
    AvailableDateTimeFormatPattern pattern = AvailableDateTimeFormatPattern::SHORT;
    DateTimeFormat formatter(pattern, locale);
    formatter.ApplyPattern(AvailableDateTimeFormatPattern::FULL);
    formatter.Format(now, "-1:45", out, status);
    std::string expect = "1970年1月7日星期三";
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest017
 * @tc.desc: Test DateTimeFormat ApplyPattern function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest017, TestSize.Level1)
{
    LocaleInfo locale("ar", "", "");
    std::string out = "";
    time_t now = 3600 * 3 + 3600 * 24 * 6;
    I18nStatus status =  I18nStatus::ISUCCESS;
    AvailableDateTimeFormatPattern pattern = AvailableDateTimeFormatPattern::SHORT;
    DateTimeFormat formatter(pattern, locale);
    formatter.ApplyPattern(AvailableDateTimeFormatPattern::FULL);
    formatter.Format(now, "-1:45", out, status);
    std::string expect = "الأربعاء، ٧ يناير، ١٩٧٠";
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest018
 * @tc.desc: Test DateTimeFormat ApplyPattern function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest018, TestSize.Level1)
{
    LocaleInfo locale("en", "", "US");
    std::string out = "";
    time_t now = 3600 * 3 + 3600 * 24 * 6;
    I18nStatus status =  I18nStatus::ISUCCESS;
    AvailableDateTimeFormatPattern pattern = AvailableDateTimeFormatPattern::SHORT;
    DateTimeFormat formatter(pattern, locale);
    formatter.ApplyPattern(AvailableDateTimeFormatPattern::FULL);
    formatter.Format(now, "-1:45", out, status);
    std::string expect = "Wednesday, January 7, 1970";
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest019
 * @tc.desc: Test DateTimeFormat GetWeekName
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest019, TestSize.Level1)
{
    LocaleInfo locale("zh", "", "CN");
    std::string weekDays[] = { "星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六" };
    AvailableDateTimeFormatPattern pattern = AvailableDateTimeFormatPattern::SHORT;
    DateTimeFormat formatter(pattern, locale);
    for (int i = 0; i < 7; ++i) {
        std::string out = formatter.GetWeekName(i, DateTimeDataType::FORMAT_WIDE);
        EXPECT_TRUE(weekDays[i] == out);
    }
}

/**
 * @tc.name: I18nFuncTest020
 * @tc.desc: Test DateTimeFormat GetMonthName
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest020, TestSize.Level1)
{
    LocaleInfo locale("en", "", "US");
    std::string months[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September",
        "October", "November", "December" };
    AvailableDateTimeFormatPattern pattern = AvailableDateTimeFormatPattern::SHORT;
    DateTimeFormat formatter(pattern, locale);
    for (int i = 0; i < 12; ++i) {
        std::string out = formatter.GetMonthName(i, DateTimeDataType::FORMAT_WIDE);
        EXPECT_TRUE(months[i] == out);
    }
}

/**
 * @tc.name: I18nFuncTest021
 * @tc.desc: Test numberFormat format integer
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest021, TestSize.Level1)
{
    LocaleInfo locale("am", "", "");
    int status =  0;
    NumberFormat formatter(locale, status);
    EXPECT_TRUE(status == 0);
    int intNum1 = 1234567;
    int intNum2 = 123456789;
    int intNum3 = 1234;
    std::string out = formatter.Format(intNum1, status);
    std::string out2 = formatter.Format(intNum2, status);
    std::string out3 = formatter.Format(intNum3, status);
    EXPECT_TRUE(out == "1,234,567");
    EXPECT_TRUE(out2 == "123,456,789");
    EXPECT_TRUE(out3 == "1,234");
}

/**
 * @tc.name: I18nFuncTest022
 * @tc.desc: Test numberFormat format no group integer
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest022, TestSize.Level1)
{
    LocaleInfo locale("am", "", "");
    int  status =  0;
    NumberFormat formatter(locale, status);
    EXPECT_TRUE(status == 0);
    int intNum1 = 1234567;
    int intNum2 = 123456789;
    int intNum3 = 1234;
    std::string out = formatter.FormatNoGroup(intNum1, status);
    std::string out2 = formatter.FormatNoGroup(intNum2, status);
    std::string out3 = formatter.FormatNoGroup(intNum3, status);
    EXPECT_TRUE(out == "1234567");
    EXPECT_TRUE(out2 == "123456789");
    EXPECT_TRUE(out3 == "1234");
}

/**
 * @tc.name: I18nFuncTest023
 * @tc.desc: Test numberFormat format double
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest023, TestSize.Level1)
{
    LocaleInfo locale("am", "", "");
    int status =  0;
    NumberFormat formatter(locale, status);
    EXPECT_TRUE(status == 0);
    double doubleNum1 = 1234567.567;
    double doubleNum2 = 1.23456789567E8;
    double doubleNum3 = 1234.0;
    std::string out = formatter.Format(doubleNum1, NumberFormatType::DECIMAL, status);
    std::string out2 = formatter.Format(doubleNum2, NumberFormatType::DECIMAL, status);
    std::string out3 = formatter.Format(doubleNum3, NumberFormatType::DECIMAL, status);
    EXPECT_TRUE(out == "1,234,567.567");
    EXPECT_TRUE(out2 == "123,456,789.567");
    EXPECT_TRUE(out3 == "1,234");
}

/**
 * @tc.name: I18nFuncTest024
 * @tc.desc: Test numberFormat format no group double
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest024, TestSize.Level1)
{
    LocaleInfo locale("am", "", "");
    int status =  0;
    NumberFormat formatter(locale, status);
    EXPECT_TRUE(status == 0);
    double doubleNum1 = 1234567.567;
    double doubleNum2 = 1.23456789567E8;
    double doubleNum3 = 1234.0;
    std::string out = formatter.FormatNoGroup(doubleNum1, NumberFormatType::DECIMAL, status);
    std::string out2 = formatter.FormatNoGroup(doubleNum2, NumberFormatType::DECIMAL, status);
    std::string out3 = formatter.FormatNoGroup(doubleNum3, NumberFormatType::DECIMAL, status);
    EXPECT_TRUE(out == "1234567.567");
    EXPECT_TRUE(out2 == "123456789.567");
    EXPECT_TRUE(out3 == "1234");
}

/**
 * @tc.name: I18nFuncTest025
 * @tc.desc: Test numberFormat format SetMaxDecimalLength;
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest025, TestSize.Level1)
{
    LocaleInfo locale("am", "", "");
    int status = 0;
    NumberFormat formatter(locale, status);
    EXPECT_TRUE(status == 0);
    double doubleNum1 = 1234567.567;
    double doubleNum2 = 1.23456789567E8;
    double doubleNum3 = 1234.0;
    formatter.SetMaxDecimalLength(1);
    std::string out = formatter.FormatNoGroup(doubleNum1, NumberFormatType::DECIMAL, status);
    std::string out2 = formatter.FormatNoGroup(doubleNum2, NumberFormatType::DECIMAL, status);
    std::string out3 = formatter.FormatNoGroup(doubleNum3, NumberFormatType::DECIMAL, status);
    EXPECT_TRUE(out == "1234567.6");
    EXPECT_TRUE(out2 == "123456789.6");
    EXPECT_TRUE(out3 == "1234");
}

/**
 * @tc.name: I18nFuncTest026
 * @tc.desc: Test numberFormat format SetMinDecimalLength;
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest026, TestSize.Level1)
{
    LocaleInfo locale("am", "", "");
    int status =  0;
    NumberFormat formatter(locale, status);
    EXPECT_TRUE(status == 0);
    double doubleNum1 = 1234567.56;
    double doubleNum2 = 1.2;
    double doubleNum3 = 1234.0;
    formatter.SetMinDecimalLength(3);
    std::string out = formatter.FormatNoGroup(doubleNum1, NumberFormatType::DECIMAL, status);
    std::string out2 = formatter.FormatNoGroup(doubleNum2, NumberFormatType::DECIMAL, status);
    std::string out3 = formatter.FormatNoGroup(doubleNum3, NumberFormatType::DECIMAL, status);
    EXPECT_TRUE(out == "1234567.560");
    EXPECT_TRUE(out2 == "1.200");
    EXPECT_TRUE(out3 == "1234.000");
}

/**
 * @tc.name: I18nFuncTest027
 * @tc.desc: Test numberFormat format percent;
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest027, TestSize.Level1)
{
    LocaleInfo locale("am", "", "");
    int status = 0;
    NumberFormat formatter(locale, status);
    EXPECT_TRUE(status == 0);
    double percentNum1 = 0.1234;
    double percentNum2 = 0.123456;
    std::string out = formatter.Format(percentNum1, NumberFormatType::PERCENT, status);
    std::string out2 = formatter.Format(percentNum2, NumberFormatType::PERCENT, status);
    EXPECT_TRUE(out == "12%");
    EXPECT_TRUE(out2 == "12%");
}
/**
 * @tc.name: I18nFuncTest028
 * @tc.desc: Test NumberFormat Format function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18NFuncTest0028, TestSize.Level1)
{
    LocaleInfo locale("zh", "", "");
    int status = 0;
    NumberFormat formatter(locale, status);
    std::string out = formatter.Format(1234567, status);
    std::string expect = "1,234,567";
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest029
 * @tc.desc: Test PluralFormat GetPluralRuleIndex function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18nFuncTest029, TestSize.Level1)
{
    LocaleInfo locale("am", "", "");
    I18nStatus status = I18nStatus::ISUCCESS;
    PluralFormat formatter(locale, status);
    int out = formatter.GetPluralRuleIndex(0, status);
    int expect = PluralRuleType::ONE;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(1, status);
    expect = PluralRuleType::ONE;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(2, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(17, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(100, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(1000, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(10000, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(100000, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(1000000, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest030
 * @tc.desc: Test PluralFormat GetPluralRuleIndex function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18nFuncTest030, TestSize.Level1)
{
    LocaleInfo locale("ar", "", "");
    I18nStatus status = I18nStatus::ISUCCESS;
    PluralFormat formatter(locale, status);
    int out = formatter.GetPluralRuleIndex(3, status);
    int expect = PluralRuleType::FEW;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(10, status);
    expect = PluralRuleType::FEW;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(11, status);
    expect = PluralRuleType::MANY;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(26, status);
    expect = PluralRuleType::MANY;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(1, status);
    expect = PluralRuleType::ONE;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(100, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(2, status);
    expect = PluralRuleType::TWO;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(0, status);
    expect = PluralRuleType::ZERO;
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest031
 * @tc.desc: Test PluralFormat GetPluralRuleIndex function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18nFuncTest031, TestSize.Level1)
{
    LocaleInfo locale("as", "", "");
    I18nStatus status = I18nStatus::ISUCCESS;
    PluralFormat formatter(locale, status);
    int out = formatter.GetPluralRuleIndex(0, status);
    int expect = PluralRuleType::ONE;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(1, status);
    expect = PluralRuleType::ONE;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(2, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest032
 * @tc.desc: Test PluralFormat GetPluralRuleIndex function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18nFuncTest032, TestSize.Level1)
{
    LocaleInfo locale("az", "", "");
    I18nStatus status = I18nStatus::ISUCCESS;
    PluralFormat formatter(locale, status);
    int out = formatter.GetPluralRuleIndex(1, status);
    int expect = PluralRuleType::ONE;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(0, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(2, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(16, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(100, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest033
 * @tc.desc: Test PluralFormat GetPluralRuleIndex function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18nFuncTest033, TestSize.Level1)
{
    LocaleInfo locale("be", "", "");
    I18nStatus status = I18nStatus::ISUCCESS;
    PluralFormat formatter(locale, status);
    int out = formatter.GetPluralRuleIndex(2, status);
    int expect = PluralRuleType::FEW;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(4, status);
    expect = PluralRuleType::FEW;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(0, status);
    expect = PluralRuleType::MANY;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(5, status);
    expect = PluralRuleType::MANY;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(19, status);
    expect = PluralRuleType::MANY;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(1, status);
    expect = PluralRuleType::ONE;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(21, status);
    expect = PluralRuleType::ONE;
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest034
 * @tc.desc: Test PluralFormat GetPluralRuleIndex function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18nFuncTest034, TestSize.Level1)
{
    LocaleInfo locale("bg", "", "");
    I18nStatus status = I18nStatus::ISUCCESS;
    PluralFormat formatter(locale, status);
    int out = formatter.GetPluralRuleIndex(1, status);
    int expect = PluralRuleType::ONE;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(0, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(2, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(16, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(100, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(1000, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(10000, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(100000, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(1000000, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest035
 * @tc.desc: Test PluralFormat GetPluralRuleIndex function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18nFuncTest035, TestSize.Level1)
{
    LocaleInfo locale("bn", "", "");
    I18nStatus status = I18nStatus::ISUCCESS;
    PluralFormat formatter(locale, status);
    int out = formatter.GetPluralRuleIndex(0, status);
    int expect = PluralRuleType::ONE;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(1, status);
    expect = PluralRuleType::ONE;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(2, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(17, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(100, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(1000, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(10000, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(100000, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(1000000, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest036
 * @tc.desc: Test PluralFormat GetPluralRuleIndex function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18nFuncTest036, TestSize.Level1)
{
    LocaleInfo locale("bo", "", "");
    I18nStatus status = I18nStatus::ISUCCESS;
    PluralFormat formatter(locale, status);
    int out = formatter.GetPluralRuleIndex(0, status);
    int expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(15, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(100, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(1000, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(10000, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(100000, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(1000000, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest037
 * @tc.desc: Test PluralFormat GetPluralRuleIndex function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18nFuncTest037, TestSize.Level1)
{
    LocaleInfo locale("bs", "", "");
    I18nStatus status = I18nStatus::ISUCCESS;
    PluralFormat formatter(locale, status);
    int out = formatter.GetPluralRuleIndex(2, status);
    int expect = PluralRuleType::FEW;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(4, status);
    expect = PluralRuleType::FEW;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(22, status);
    expect = PluralRuleType::FEW;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(1, status);
    expect = PluralRuleType::ONE;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(21, status);
    expect = PluralRuleType::ONE;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(0, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest038
 * @tc.desc: Test PluralFormat GetPluralRuleIndex function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18nFuncTest038, TestSize.Level1)
{
    LocaleInfo locale("ca", "", "");
    I18nStatus status = I18nStatus::ISUCCESS;
    PluralFormat formatter(locale, status);
    int out = formatter.GetPluralRuleIndex(1, status);
    int expect = PluralRuleType::ONE;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(0, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest039
 * @tc.desc: Test PluralFormat GetPluralRuleIndex function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18nFuncTest039, TestSize.Level1)
{
    LocaleInfo locale("cs", "", "");
    I18nStatus status = I18nStatus::ISUCCESS;
    PluralFormat formatter(locale, status);
    int out = formatter.GetPluralRuleIndex(2, status);
    int expect = PluralRuleType::FEW;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(4, status);
    expect = PluralRuleType::FEW;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(1, status);
    expect = PluralRuleType::ONE;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(0, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest040
 * @tc.desc: Test PluralFormat GetPluralRuleIndex function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18nFuncTest040, TestSize.Level1)
{
    LocaleInfo locale("da", "", "");
    I18nStatus status = I18nStatus::ISUCCESS;
    PluralFormat formatter(locale, status);
    int out = formatter.GetPluralRuleIndex(1, status);
    int expect = PluralRuleType::ONE;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(0, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest041
 * @tc.desc: Test PluralFormat GetPluralRuleIndex function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18nFuncTest041, TestSize.Level1)
{
    LocaleInfo locale("de", "", "");
    I18nStatus status = I18nStatus::ISUCCESS;
    PluralFormat formatter(locale, status);
    int out = formatter.GetPluralRuleIndex(1, status);
    int expect = PluralRuleType::ONE;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(0, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(1000000, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest042
 * @tc.desc: Test PluralFormat GetPluralRuleIndex function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18nFuncTest042, TestSize.Level1)
{
    LocaleInfo locale("el", "", "");
    I18nStatus status = I18nStatus::ISUCCESS;
    PluralFormat formatter(locale, status);
    int out = formatter.GetPluralRuleIndex(1, status);
    int expect = PluralRuleType::ONE;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(100, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest043
 * @tc.desc: Test PluralFormat GetPluralRuleIndex function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18nFuncTest043, TestSize.Level1)
{
    LocaleInfo locale("en", "", "");
    I18nStatus status = I18nStatus::ISUCCESS;
    PluralFormat formatter(locale, status);
    int out = formatter.GetPluralRuleIndex(1, status);
    int expect = PluralRuleType::ONE;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(16, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest044
 * @tc.desc: Test PluralFormat GetPluralRuleIndex function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18nFuncTest044, TestSize.Level1)
{
    LocaleInfo locale("es", "", "");
    I18nStatus status = I18nStatus::ISUCCESS;
    PluralFormat formatter(locale, status);
    int out = formatter.GetPluralRuleIndex(1, status);
    int expect = PluralRuleType::ONE;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(0, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(2, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest045
 * @tc.desc: Test PluralFormat GetPluralRuleIndex function
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18nFuncTest045, TestSize.Level1)
{
    LocaleInfo locale("et", "", "");
    I18nStatus status = I18nStatus::ISUCCESS;
    PluralFormat formatter(locale, status);
    int out = formatter.GetPluralRuleIndex(1, status);
    int expect = PluralRuleType::ONE;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(2, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
    out = formatter.GetPluralRuleIndex(100000, status);
    expect = PluralRuleType::OTHER;
    EXPECT_TRUE(expect == out);
}

/**
 * @tc.name: I18nFuncTest046
 * @tc.desc: Test NumberFormat Format percent
 * @tc.type: FUNC
 */
HWTEST_F(I18NTest, I18nFuncTest046, TestSize.Level1)
{
    LocaleInfo locale("de", "DE");
    I18nStatus status = I18nStatus::ISUCCESS;
    NumberFormat format(locale, status);
    std::string out = format.Format(0.12, NumberFormatType::PERCENT, status);
    std::string expect = "12 %";
    cout << "out is " << out << endl;
    EXPECT_TRUE(expect == out);
}
} // anonymous namespace

