#include "afs.h"

HANDLE OpenDrive(char *DeviceName)
{ HANDLE DevHandle;

  DevHandle = CreateFile(DeviceName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,
                           NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_RANDOM_ACCESS,NULL);

  if(DevHandle!=INVALID_HANDLE_VALUE)
  {
    SendDlgItemMessage(MainHwnd,IDC_ProgressBar,PBM_SETPOS,0,0);
    return(DevHandle);
   }

  return(NULL);
 }

void CloseDrive(HANDLE DevHandle)
{
  SendDlgItemMessage(MainHwnd,IDC_ProgressBar,PBM_SETPOS,0,0);

  if(DevHandle)
    CloseHandle(DevHandle);
 }

long ReadSector(HANDLE Handle, ubyte *Buffer, long Size,i64 Offset)
{ DWORD Read;
  LARGE_INTEGER ByteOffset;
  ByteOffset.QuadPart = ( /*SectorOffset+ */ (i64)Offset)*(i64)LOGICAL_BLOCK_SIZE;

  if((SetFilePointer(Handle,ByteOffset.u.LowPart,&ByteOffset.u.HighPart,FILE_BEGIN)!=0xFFFFFFFF)&&
      ReadFile(Handle,Buffer,Size,&Read,NULL))
  { SendDlgItemMessage(MainHwnd,IDC_ProgressBar,PBM_STEPIT,0,0);
    return(Read);
   }
  else
    return 0;
 }

long WriteSector(HANDLE Handle, ubyte *Buffer, long Size,i64 Offset)
{ DWORD Written;
  LARGE_INTEGER ByteOffset;

  ByteOffset.QuadPart = ( /* SectorOffset + */ (i64)Offset)*(i64)LOGICAL_BLOCK_SIZE;

  if((SetFilePointer(Handle,ByteOffset.u.LowPart,&ByteOffset.u.HighPart,FILE_BEGIN)!=0xFFFFFFFF)&&
     WriteFile(Handle,Buffer,Size,&Written,NULL))
  { SendDlgItemMessage(MainHwnd,IDC_ProgressBar,PBM_STEPIT,0,0);
    return(Written);
   }
  else
    return 0;
 }

