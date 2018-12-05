/*
    Copyright © 1995-2001, The AROS Development Team. All rights reserved.
    $Id: dos.h 20404 2003-12-24 15:27:23Z falemagn $

    Desc: Basic DOS structures and constants
    Lang: english
*/

/* Number of so-called "ticks" in a second. */
#define TICKS_PER_SECOND 50

/* DateStamp structure as used in different library-functions. This
   structure sufficiently describes a system-date and -time (i.e. any
   date since 1.1.1978). */
struct DateStamp
{
   LONG ds_Days;   /* Number of days since 1.1.1978 */
   LONG ds_Minute; /* Number of minutes since midnight */
   LONG ds_Tick;   /* Number of ticks (1/50 second) in the current minute
                      Note that this may not be exact */
};

void ConvertFATDate(UWORD date, UWORD time, struct DateStamp *ds);
void ConvertAROSDate(struct DateStamp ds, UWORD *date, UWORD *time);

// AROS to/from Windows FILETIME
FILETIME *AFS_FileTime(long days,long mins,long ticks);
struct DateStamp *FileTime_AFS(FILETIME *FileTime);

