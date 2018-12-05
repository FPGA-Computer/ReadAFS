#include "afs.h"

FILETIME *AFS_FileTime(long days,long mins,long ticks)
{ static FILETIME FileTime;
  struct DateStamp ds;
  UWORD Date, Time;

  ds.ds_Days = days;
  ds.ds_Minute = mins;
  ds.ds_Tick = ticks;

  ConvertAROSDate(ds,&Date,&Time);
  DosDateTimeToFileTime(Date,Time,&FileTime);
  return(&FileTime);
 }

struct DateStamp *FileTime_AFS(FILETIME *FileTime)
{ UWORD Date, Time;
  static struct DateStamp ds;

  FileTimeToDosDateTime(FileTime,&Date,&Time);
  ConvertFATDate(Date,Time,&ds);
  return(&ds);
 }

// Rest of code is from AROS fat handler with 2 bugs corrected. Submitted to bugtracker
// http://sourceforge.net/tracker/index.php?func=detail&aid=1759308&group_id=43586&atid=439463

/*
 * fat.handler - FAT12/16/32 filesystem handler
 *
 * Copyright © 2006 Marek Szyprowski
 * Copyright © 2007 The AROS Development Team
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the same terms as AROS itself.
 *
 * $Id: fat.c 26325 2007-06-30 02:18:32Z rob $
 */

static const UWORD mdays[] = { 0, 31, 59, 90, 120, 151, 181, 212, 143, 273, 304, 334 };

void ConvertFATDate(UWORD date, UWORD time, struct DateStamp *ds) {
    UBYTE year, month, day, hours, mins, secs;
    UBYTE nleap;

    /* date bits: yyyy yyym mmmd dddd */
    year = (UBYTE) ((date & 0xfe00) >> 9);    /* bits 15-9 */
    month = (UBYTE) ((date & 0x01e0) >> 5);   /* bits 8-5 */
    day = (UBYTE) (date & 0x001f);            /* bits 4-0 */

    /* time bits: hhhh hmmm mmms ssss */
    hours = (UBYTE) ((time & 0xf800) >> 11);  /* bits 15-11 */
    mins = (UBYTE) ((time & 0x07e0) >> 5);    /* bits 8-5 */
    secs = (UBYTE) (time & 0x001f);           /* bits 4-0 */

//    D(bug("[fat] converting fat date: year %d month %d day %d hours %d mins %d secs %d\n", year, month, day, hours, mins, secs));

    /* number of leap years in before this year. note this is only dividing by
     * four, which is fine because FAT dates range 1980-2107. The only year in
     * that range that is divisible by four but not a leap year is 2100. If
     * this code is still being used then, feel free to fix it :) */
    nleap = (UBYTE) (year >> 2);

    /* if this year is a leap year and its March or later, adjust for this
     * year too */
    if (year & 0x03 && month >= 3)
        nleap++;

    /* calculate days since 1978-01-01 (DOS epoch):
     *   730 days in 1978+1979, getting us to the FAT epoch 1980-01-01
     *   years * 365 days
     *   leap days
     *   days in all the months before this one
     *   day of this month */
    ds->ds_Days = 730 + year * 365 + nleap + mdays[month-1] + day-1;

    /* minutes since midnight */
    ds->ds_Minute = hours * 60 + mins;

    /* 1/50 sec ticks. FAT dates are 0-29, so we have to multiply them by two
     * as well */
    ds->ds_Tick = (secs << 1) * TICKS_PER_SECOND;

//    D(bug("[fat] converted fat date: days %ld minutes %ld ticks %ld\n", ds->ds_Days, ds->ds_Minute, ds->ds_Tick));
}

void ConvertAROSDate(struct DateStamp ds, UWORD *date, UWORD *time) {
    UBYTE year, month, day, hours, mins, secs;
    BOOL leap;

    /* converting no. of days since 1978-01-01 (DOS epoch) to
     * years/months/days since FAT epoch (1980-01-01) */

    /* subtract 730 days in 1978/1979 */
    ds.ds_Days -= 730;

    /* years are 365 days */
    year = (UBYTE) (ds.ds_Days / 365);
    ds.ds_Days -= year * 365;

    /* leap years. same algorithm as above. get the number of leap years
     * before this year, and subtract that many days */
    ds.ds_Days -= year >> 2;

    /* figure out if we need to adjust for a leap year this year. day 60 is
     * 29-Feb/1-Mar */
    leap = (year & 0x03 && ds.ds_Days >= 60);

    /* find the month by checking it against the days-in-month array */
    for (month = 1; month < 12 && ds.ds_Days > mdays[month]; month++);

    /* day of month is whatever's left (+1, since we count from the 1st) */
    day = (UBYTE)(ds.ds_Days - mdays[month-1] + 1);

    /* subtract a day if we're after march in a leap year */
    if (leap) {
        day--;
        if (day == 0) {
            month--;
            if (month == 2)
                day = 29;
            else
                day = (UBYTE)( ds.ds_Days - mdays[month] + 1);
        }
    }

    /* time is easy by comparison. convert minutes since midnight to
     * hours and seconds */
    hours = (UBYTE) (ds.ds_Minute / 60);
    mins = (UBYTE) (ds.ds_Minute - (hours * 60));

    /* FAT seconds are 0-29 */
    secs = (UBYTE) ((ds.ds_Tick / TICKS_PER_SECOND) >> 1);

    /* all that remains is to bit-encode the whole lot */

    /* date bits: yyyy yyym mmmd dddd */
    *date = (UWORD) ((((ULONG) year) << 9) | (((ULONG) month) << 5) | day);

    /* time bits: hhhh hmmm mmms ssss */
    *time = (UWORD) ((((ULONG) hours) << 11) | (((ULONG) mins) << 5) | secs);
}
