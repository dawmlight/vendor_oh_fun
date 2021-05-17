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

#include "date_time_format_impl.h"
#include "date_time_data.h"

using namespace OHOS::I18N;
using namespace std;

/**
 * construct a DateTimeFormat object with request pattern and locale.
 * now we only support patterns defined in AvailableDateTimeFormatPatterns.
 * locale, locale information to retrive datetime resource form icu data.
 */
DateTimeFormatImpl::DateTimeFormatImpl(AvailableDateTimeFormatPattern requestPattern, const LocaleInfo &locale)
{
    fLocale = locale;
    this->requestPattern = requestPattern;
}

std::string DateTimeFormatImpl::GetWeekName(const int32_t &index, DateTimeDataType type) const
{
    return (data == nullptr) ? "" : data->GetDayName(index, type);
}

std::string DateTimeFormatImpl::GetMonthName(const int32_t &index, DateTimeDataType type) const
{
    return (data == nullptr) ? "" : data->GetMonthName(index, type);
}

std::string DateTimeFormatImpl::GetAmPmMarker(const int32_t &index, DateTimeDataType type) const
{
    return (data == nullptr) ? "" : data->GetAmPmMarker(index, type);
}

string DateTimeFormatImpl::GetStringFromPattern(const AvailableDateTimeFormatPattern &requestPattern) const
{
    if (!data) {
        return "";
    }
    switch (requestPattern) {
        case HOUR12_MINUTE_SECOND: {
            string hourMinuteSecond = data->GetPattern(0, PatternType::HOUR_MINUTE_SECOND_PATTERN);
            if (hourMinuteSecond != "") {
                return hourMinuteSecond;
            }
            string hmPattern = data->GetPattern(HOUR12_MINUTE_INDEX, PatternType::REGULAR_PATTERN);
            return AddSeconds(hmPattern);
        }
        case HOUR24_MINUTE_SECOND: {
            string hourMinuteSecond = data->GetPattern(1, PatternType::HOUR_MINUTE_SECOND_PATTERN);
            if (hourMinuteSecond != "") {
                return hourMinuteSecond;
            }
            string hmPattern = data->GetPattern(HOUR24_MINUTE_INDEX, PatternType::REGULAR_PATTERN);
            return AddSeconds(hmPattern);
        }
        case HOUR_MINUTE_SECOND: {
            char defaultHour = data->GetDefaultHour();
            if (defaultHour == 'h') {
                string hourMinuteSecond = data->GetPattern(0, PatternType::HOUR_MINUTE_SECOND_PATTERN);
                if (hourMinuteSecond != "") {
                    return hourMinuteSecond;
                }
                string hmPattern = data->GetPattern(HOUR12_MINUTE_SECOND_INDEX, PatternType::REGULAR_PATTERN);
                return AddSeconds(hmPattern);
            }
            string hourMinuteSecond = data->GetPattern(1, PatternType::HOUR_MINUTE_SECOND_PATTERN);
            if (hourMinuteSecond != "") {
                return hourMinuteSecond;
            }
            string hmPattern = data->GetPattern(HOUR24_MINUTE_SECOND_INDEX, PatternType::REGULAR_PATTERN);
            return AddSeconds(hmPattern);
        }
        case ABBR_MONTH_DAY: {
            return data->GetPattern(ABBR_MONTH_DAY_INDEX, PatternType::REGULAR_PATTERN);
        }
        case HOUR12_MINUTE: {
            return data->GetPattern(HOUR12_MINUTE_INDEX, PatternType::REGULAR_PATTERN);
        }
        default: {
            return GetStringFromPattern2(requestPattern);
        }
    }
}

string DateTimeFormatImpl::GetStringFromPattern2(const AvailableDateTimeFormatPattern &requestPattern) const
{
    switch (requestPattern) {
        case HOUR24_MINUTE: {
            return data->GetPattern(HOUR24_MINUTE_INDEX, PatternType::REGULAR_PATTERN);
        }
        case HOUR_MINUTE: {
            char defaultHour = data->GetDefaultHour();
            if (defaultHour == 'h') {
                return data->GetPattern(HOUR12_MINUTE_INDEX, PatternType::REGULAR_PATTERN);
            }
            return data->GetPattern(HOUR24_MINUTE_INDEX, PatternType::REGULAR_PATTERN);
        }
        case ABBR_MONTH_WEEKDAY_DAY: {
            return data->GetPattern(ABBR_MONTH_WEEKDAY_DAY_INDEX, PatternType::REGULAR_PATTERN);
        }
        case FULL: {
            return data->GetPattern(FULL_DATE_INDEX, PatternType::FULL_MEDIUM_SHORT_PATTERN);
        }
        case MEDIUM: {
            return data->GetPattern(MEDIUM_DATE_INDEX, PatternType::FULL_MEDIUM_SHORT_PATTERN);
        }
        case SHORT: {
            return data->GetPattern(SHORT_DATE_INDEX, PatternType::FULL_MEDIUM_SHORT_PATTERN);
        }
        case YEAR_ABBR_MONTH_ABBR_WEEKDAY_DAY: {
            return data->GetPattern(YEAR_ABBR_MONTH_ABBR_WEEKDAY_DAY_INDEX, PatternType::REGULAR_PATTERN);
        }
        case YEAR_WIDE_MONTH_ABBR_WEEKDAY_DAY: {
            return data->GetPattern(YEAR_WIDE_MONTH_ABBR_WEEKDAY_DAY_INDEX, PatternType::REGULAR_PATTERN);
        }
        case YEAR_SHORT_MONTH_WIDE_WEEKDAY_DAY: {
            return data->GetPattern(YEAR_SHORT_MONTH_WIDE_WEEKDAY_DAY_INDEX, PatternType::REGULAR_PATTERN);
        }
        case YEAR_SHORT_MONTH_ABBR_WEEKDAY_DAY: {
            return data->GetPattern(YEAR_SHORT_MONTH_ABBR_WEEKDAY_DAY_INDEX, PatternType::REGULAR_PATTERN);
        }
        case YEAR_ABBR_MONTH_WIDE_WEEKDAY_DAY: {
            return data->GetPattern(YEAR_ABBR_MONTH_WIDE_WEEKDAY_DAY_INDEX, PatternType::REGULAR_PATTERN);
        }
        case YEAR_WIDE_MONTH_DAY: {
            return data->GetPattern(YEAR_WIDE_MONTH_DAY_INDEX, PatternType::REGULAR_PATTERN);
        }
        default: {
            return "";
        }
    }
}

string DateTimeFormatImpl::AddSeconds(const string &hmPattern) const
{
    uint32_t size = hmPattern.size();
    if (size == 0 || data == nullptr) {
        return "";
    }
    int32_t i = hmPattern.size() - 1;
    string out;
    out.reserve(DECIMAL_COUNT); // allocate ten more bytes
    while (i >= 0) {
        if (hmPattern[i] == 'm') {
            break;
        }
        --i;
    }
    out.append(hmPattern.substr(0, i + 1));
    out.append(1, data->GetTimeSeparator());
    out.append("ss");
    out.append(hmPattern.substr(i + 1, hmPattern.size() - i - 1));
    return out;
}

DateTimeFormatImpl::~DateTimeFormatImpl()
{
    FreeResource();
}

bool DateTimeFormatImpl::Init(const DataResource &resource)
{
    std::string formatAbbreviatedMonthNames = resource.GetString(DataResourceType::GREGORIAN_FORMAT_ABBR_MONTH);
    std::string formatWideMonthNames = resource.GetString(DataResourceType::GREGORIAN_FORMAT_WIDE_MONTH);
    std::string standaloneAbbreviatedMonthNames =
        resource.GetString(DataResourceType::GREGORIAN_STANDALONE_ABBR_MONTH);
    std::string standaloneWideMonthNames = resource.GetString(DataResourceType::GREGORIAN_STANDALONE_WIDE_MONTH);
    std::string formatAbbreviatedDayNames = resource.GetString(DataResourceType::GREGORIAN_FORMAT_ABBR_DAY);
    std::string formatWideDayNames = resource.GetString(DataResourceType::GREGORIAN_FORMAT_WIDE_DAY);
    std::string standaloneAbbreviatedDayNames = resource.GetString(DataResourceType::GREGORIAN_STANDALONE_ABBR_DAY);
    std::string standaloneWideDayNames = resource.GetString(DataResourceType::GREGORIAN_STANDALONE_WIDE_DAY);
    std::string amPmMarkers = resource.GetString(DataResourceType::GREGORIAN_AM_PMS);
    std::string patterns = resource.GetString(DataResourceType::GREGORIAN_TIME_PATTERNS) + "_" +
        resource.GetString(DataResourceType::GREGORIAN_DATE_PATTERNS);
    std::string timeSeparator = resource.GetString(DataResourceType::TIME_SEPARATOR);
    std::string defaultHour = resource.GetString(DataResourceType::DEFAULT_HOUR);
    std::string hourMinuteSecondPatterns = resource.GetString(DataResourceType::GREGORIAN_HOUR_MINUTE_SECOND_PATTERN);
    std::string fullMediumShortPatterns = resource.GetString(DataResourceType::GREGORIAN_FULL_MEDIUM_SHORT_PATTERN);
    char sepAndHour[SEP_HOUR_SIZE];
    if ((timeSeparator.size() < 1) || (defaultHour.size() < 1)) {
        return false;
    }
    sepAndHour[0] = timeSeparator[0];
    sepAndHour[1] = defaultHour[0];
    data = new(nothrow) DateTimeData(amPmMarkers, sepAndHour, 2); // 2 is length of sepAndHour
    if (data == nullptr) {
        return false;
    }
    data->SetMonthNamesData(formatAbbreviatedMonthNames, formatWideMonthNames,
        standaloneAbbreviatedMonthNames, standaloneWideMonthNames);
    data->SetDayNamesData(formatAbbreviatedDayNames, formatWideDayNames,
        standaloneAbbreviatedDayNames, standaloneWideDayNames);
    data->SetPatternsData(patterns, hourMinuteSecondPatterns, fullMediumShortPatterns);
    fPattern = GetStringFromPattern(requestPattern);
    return true;
}

void DateTimeFormatImpl::FreeResource()
{
    if (data != nullptr) {
        delete data;
        data = nullptr;
    }
    if (numberFormat != nullptr) {
        delete numberFormat;
        numberFormat = nullptr;
    }
}

void DateTimeFormatImpl::ApplyPattern(const AvailableDateTimeFormatPattern &requestPattern)
{
    fPattern = GetStringFromPattern(requestPattern);
}

LocaleInfo DateTimeFormatImpl::GetLocale()
{
    return fLocale;
}

/**
 * parse a time (represent by the seconds elapsed from UTC 1970, January 1 00:00:00) to its text format.
 * cal, seconds from from UTC 1970, January 1 00:00:00
 * zoneInfoOffest, string representation of offsett such as "+01:45"
 * appendTo, output of this method.
 */
void DateTimeFormatImpl::Format(const time_t &cal, const string &zoneInfo, string &appendTo, I18nStatus &status) const
{
    bool inQuote = false;
    char pre = '\0';
    uint32_t count = 0;
    const time_t adjust = cal + ParseZoneInfo(zoneInfo);
    struct tm tmStruct = {0};
    tm *tmPtr = &tmStruct;
    gmtime_r(&adjust, tmPtr);

    const tm time = *tmPtr;
    for (size_t i = 0; i < fPattern.size(); ++i) {
        char current = fPattern.at(i);
        if ((current != pre) && (count != 0)) {
            Process(time, appendTo, pre, count, status);
            count = 0;
        }
        if (current == QUOTE) {
            if ((i + 1 < fPattern.size()) && fPattern[i + 1] == QUOTE) {
                appendTo.append(1, QUOTE);
                ++i;
            } else {
                inQuote = !inQuote;
            }
        } else if (!inQuote && (((current >= 'a') && (current <= 'z')) || ((current >= 'A') && (current <= 'Z')))) {
            pre = current;
            ++count;
        } else {
            appendTo.append(1, current);
        }
    }

    if (count != 0) {
        Process(time, appendTo, pre, count, status);
    }
}

/**
 * parse zoneInfo string such as “+1:45” to 1 hour 45 minutes = 3600 * 1 + 45 * 60 seconds
 */
int32_t DateTimeFormatImpl::ParseZoneInfo(const string &zoneInfo) const
{
    int32_t ret = 0;
    uint32_t size = zoneInfo.size();
    if (size == 0) {
        return ret;
    }
    bool sign = true;
    uint32_t index = 0;
    if (zoneInfo[index] == '+') {
        ++index;
    } else if (zoneInfo[index] == '-') {
        ++index;
        sign = false;
    }

    uint32_t hour = 0;
    uint32_t minute = 0;
    bool isHour = true;
    uint32_t temp = 0;
    while (index < size) {
        char cur = zoneInfo[index];
        if (isdigit(cur)) {
            temp *= DECIMAL_COUNT; // convert string to its decimal format
            temp += (cur - '0');
            ++index;
        } else if (cur == ':' && isHour) {
            hour = temp;
            temp = 0;
            ++index;
            isHour = false;
        } else {
            return 0;
        }
    }
    if (!isHour && (temp != 0)) {
        minute = temp;
    }
    ret = SECONDS_IN_HOUR * hour + minute * SECONDS_IN_MINUTE;

    if (!sign) {
        return -ret;
    }
    return ret;
}

/**
 * convert a UTC seconds to its string format
 */
void DateTimeFormatImpl::Process(const tm &time, string &appendTo, char pre, uint32_t count, I18nStatus &status) const
{
    if ((status != I18nStatus::ISUCCESS) || !data) {
        return;
    }
    if (IsTimeChar(pre)) {
        ProcessTime(time, appendTo, pre, count, status);
        return;
    }
    switch (pre) {
        case 'L': {
            int32_t standaloneMonth = time.tm_mon;
            if (count == WIDE_COUNT) {
                appendTo.append(data->GetMonthName(standaloneMonth, DateTimeDataType::STANDALONE_WIDE));
            } else if (count == ABB_COUNT) {
                appendTo.append(data->GetMonthName(standaloneMonth, DateTimeDataType::STANDALONE_ABBR));
            } else {
                ZeroPadding(appendTo, count, MAX_COUNT, standaloneMonth + 1);
            }
            break;
        }
        case 'M': {
            int32_t month = time.tm_mon;
            if (count == WIDE_COUNT) {
                appendTo.append(data->GetMonthName(month, DateTimeDataType::FORMAT_WIDE));
            } else if (count == ABB_COUNT) {
                appendTo.append(data->GetMonthName(month, DateTimeDataType::FORMAT_ABBR));
            } else {
                ZeroPadding(appendTo, count, MAX_COUNT, month + 1);
            }
            break;
        }
        default: {
            ProcessWeekDayYear(time, appendTo, pre, count, status);
        }
    }
}

void DateTimeFormatImpl::ProcessWeekDayYear(const tm &time, string &appendTo, char pre,
    uint32_t count, I18nStatus &status) const
{
    switch (pre) {
        case 'c': {
            int32_t standaloneWeekDay = time.tm_wday;
            if (count == WIDE_COUNT) {
                appendTo.append(data->GetDayName(standaloneWeekDay, DateTimeDataType::STANDALONE_WIDE));
            } else if (count == ABB_COUNT) {
                appendTo.append(data->GetDayName(standaloneWeekDay, DateTimeDataType::STANDALONE_ABBR));
            } else {
                ZeroPadding(appendTo, count, MAX_COUNT, standaloneWeekDay);
            }
            break;
        }
        // case 'c':
        case 'e':
        case 'E': {
            int32_t weekDay = time.tm_wday;
            if (count == WIDE_COUNT) {
                appendTo.append(data->GetDayName(weekDay, DateTimeDataType::FORMAT_WIDE));
            } else if (count == ABB_COUNT) {
                appendTo.append(data->GetDayName(weekDay, DateTimeDataType::FORMAT_ABBR));
            } else {
                ZeroPadding(appendTo, count, MAX_COUNT, weekDay);
            }
            break;
        }
        case 'd': {
            int32_t day = time.tm_mday;
            ZeroPadding(appendTo, count, MAX_COUNT, day);
            break;
        }
        case 'y': {
            int32_t year = time.tm_year + YEAR_START;
            if (count == SHORT_YEAR_FORMAT_COUNT) {
                int adjustYear = year - (year / ONE_HUNDRED_YEAR) * ONE_HUNDRED_YEAR;
                ZeroPadding(appendTo, count, MAX_COUNT, adjustYear);
            } else {
                appendTo.append(FormatYear(year));
            }
            break;
        }
        default: {
            return;
        }
    }
}

bool DateTimeFormatImpl::IsTimeChar(char ch) const
{
    string timeCharacters = "ahHkKms:";
    return (timeCharacters.find(ch) != string::npos) ? true : false;
}

void DateTimeFormatImpl::ProcessTime(const tm &time, string &appendTo, char pre,
    uint32_t count, I18nStatus &status) const
{
    switch (pre) {
        case 'a': {
            int32_t index = (time.tm_hour < LENGTH_HOUR) ? 0 : 1;
            string amText = data->GetAmPmMarker(index, DateTimeDataType::FORMAT_ABBR);
            appendTo.append(amText);
            break;
        }
        // deal with hour.
        // input is in the range of 0, 23. And final representation is in the range of 1, 12
        case 'h': {
            int32_t hour = (time.tm_hour == 0) ? LENGTH_HOUR : time.tm_hour;
            int32_t index = (hour > LENGTH_HOUR) ? (hour - LENGTH_HOUR) : hour;
            ZeroPadding(appendTo, count, MAX_COUNT, index);
            break;
        }
        case 'H': {
            int32_t hour = time.tm_hour;
            ZeroPadding(appendTo, count, MAX_COUNT, hour);
            break;
        }
        case 'K': {
            int32_t hour = time.tm_hour;
            int32_t index = (hour >= LENGTH_HOUR) ? (hour - LENGTH_HOUR) : hour;
            ZeroPadding(appendTo, count, MAX_COUNT, index);
            break;
        }
        case 'k': {
            int32_t hour = time.tm_hour + 1;
            ZeroPadding(appendTo, count, MAX_COUNT, hour);
            break;
        }
        case 'm': {
            int32_t minute = time.tm_min;
            ZeroPadding(appendTo, count, MAX_COUNT, minute);
            break;
        }
        case 's': {
            int32_t second = time.tm_sec;
            ZeroPadding(appendTo, count, MAX_COUNT, second);
            break;
        }
        case ':': {
            appendTo.append(1, data->GetTimeSeparator());
        }
        default:
            return;
    }
}

/**
 * Padding numbers with 0, minValue is the requested minimal length of the output number
 */
void DateTimeFormatImpl::ZeroPadding(string &appendTo, uint32_t minValue, uint32_t maxValue, int32_t value) const
{
    // value should >= 0
    if (value < 0) {
        return;
    }
    uint32_t adjustValue = (minValue < maxValue) ? minValue : maxValue;
    uint32_t count = GetLength(value);
    string temp = "";
    while (count < adjustValue) {
        temp += GetZero();
        ++count;
    }
    string tempOut = FormatNumber(value);
    temp += FormatNumber(value);
    appendTo.append(temp);
}

string DateTimeFormatImpl::FormatNumber(int32_t value) const
{
    int status = 0;
    return numberFormat ? numberFormat->Format(value, status) : to_string(value);
}

string DateTimeFormatImpl::GetZero() const
{
    int status = 0;
    return numberFormat ? numberFormat->Format(0, status) : "0";
}

uint32_t DateTimeFormatImpl::GetLength(int32_t value) const
{
    if (value < DECIMAL_COUNT) {
        return 1;
    }
    uint32_t count = 0;
    uint32_t temp = value;
    while (temp) {
        ++count;
        temp = temp / DECIMAL_COUNT;
    }
    return count;
}
string DateTimeFormatImpl::FormatYear(int32_t value) const
{
    int status = 0;
    return numberFormat ? numberFormat->FormatNoGroup(value, status) : to_string(value);
}
