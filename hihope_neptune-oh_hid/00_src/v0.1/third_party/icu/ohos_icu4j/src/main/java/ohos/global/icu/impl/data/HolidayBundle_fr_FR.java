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
public class HolidayBundle_fr_FR extends ListResourceBundle {
    static private final Holiday[] fHolidays = {
        SimpleHoliday.NEW_YEARS_DAY,
        new SimpleHoliday(Calendar.MAY,        1,  0, "Labor Day"),
        new SimpleHoliday(Calendar.MAY,        8,  0, "Victory Day"),
        new SimpleHoliday(Calendar.JULY,      14,  0, "Bastille Day"),
        SimpleHoliday.ASSUMPTION,
        SimpleHoliday.ALL_SAINTS_DAY,
        new SimpleHoliday(Calendar.NOVEMBER,  11,  0, "Armistice Day"),
        SimpleHoliday.CHRISTMAS,

        // Easter and related holidays
        EasterHoliday.EASTER_SUNDAY,
        EasterHoliday.EASTER_MONDAY,
        EasterHoliday.ASCENSION,
        EasterHoliday.WHIT_SUNDAY,
        EasterHoliday.WHIT_MONDAY,
    };
    static private final Object[][] fContents = {
        { "holidays",   fHolidays },
    };
    @Override
    public synchronized Object[][] getContents() { return fContents; }
}
