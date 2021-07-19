/* GENERATED SOURCE. DO NOT MODIFY. */
// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License
/*
 *******************************************************************************
 * Copyright (C) 1996-2010, International Business Machines Corporation and    *
 * others. All Rights Reserved.                                                *
 *******************************************************************************
 */

package ohos.global.icu.impl.data;

import java.util.Calendar;
import java.util.ListResourceBundle;

import ohos.global.icu.util.EasterHoliday;
import ohos.global.icu.util.Holiday;
import ohos.global.icu.util.SimpleHoliday;

/**
 * @hide exposed on OHOS
 */
public class HolidayBundle_da_DK extends ListResourceBundle {
    static private final Holiday[] fHolidays = { SimpleHoliday.NEW_YEARS_DAY,
            new SimpleHoliday(Calendar.APRIL, 30, -Calendar.FRIDAY, "General Prayer Day"),
            new SimpleHoliday(Calendar.JUNE, 5, "Constitution Day"), SimpleHoliday.CHRISTMAS_EVE,
            SimpleHoliday.CHRISTMAS, SimpleHoliday.BOXING_DAY, SimpleHoliday.NEW_YEARS_EVE,

            // Easter and related holidays
            EasterHoliday.MAUNDY_THURSDAY, EasterHoliday.GOOD_FRIDAY, EasterHoliday.EASTER_SUNDAY,
            EasterHoliday.EASTER_MONDAY, EasterHoliday.ASCENSION, EasterHoliday.WHIT_MONDAY, };

    static private final Object[][] fContents = { { "holidays", fHolidays }, };

    @Override
    public synchronized Object[][] getContents() {
        return fContents;
    }
}
