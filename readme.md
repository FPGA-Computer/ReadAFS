I found this sitting on my HDD. I haven't worked on this since.
This code provided is as is and I am not responsible for any HDD or file system damages.

This works on the OFS/FFS and can mount raw disk or disk image file for copying files and validation.
The program does not support extended partition which is used by later version of AROS.

Licenses:

Ones that have explicited copyright notice by their owner. They are from 
AROS source tree. The rest are copyrighted by K.C. Lee (c) 2007, but are free 
redistributable under GPLV3 license.

-------------------------------------------------------------------------

The original code was compiled into afs_util version 0.1 using Borland C 
compiler on Windows using Win32API. (x86 little endian)

There are error detection modification added since and have not been fully tested.

The code entry routine as are the other validation routines are in "afs validate.c"
int AFS_Validate(struct Partition *Part);

Main validation code transverse the ofs/ffs data structure recursively marking the
bitmap as it goes. When there are no errors encountered, it overwrites the bitmap
and modify the file date and the bitmap valid field.

Device dependent calls in device.h Should be easy to rewrite for AROS. 

i64 is 64-bit integer.

HANDLE OpenDrive(char *DeviceName);

void CloseDrive(HANDLE DevHandle);

long WriteSector(HANDLE Handle, ubyte *Buffer, long Size,i64 Offset);

long ReadSector(HANDLE Handle, ubyte *Buffer, long Size,i64 Offset);

debug.h:

void SetTextf(HWND Hwnd,int idc, const char *Format,...);   <-- Set Text Format

first 2 parameters are for windows, rest is for c printf()

void CondDebug(int level,const char *Format,...);  <- much better than D(ebug())

Have fun hacking away!
