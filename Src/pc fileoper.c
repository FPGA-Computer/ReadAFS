#include "afs.h"

void AddPCDir(WIN32_FIND_DATA *NextFile,uword Type)
{ ULARGE_INTEGER FileSize;
  FILETIME   LocalTime;

  FileSize.u.LowPart=NextFile->nFileSizeLow;
  FileSize.u.HighPart=NextFile->nFileSizeHigh;
  FileTimeToLocalFileTime(&NextFile->ftLastWriteTime,&LocalTime);

  AddEntry(PCDIR_ENTRY,NextFile->cFileName,(i64)FileSize.QuadPart,&LocalTime,Type,0);
 }

void ScanPCDir(char *Path)
{ HANDLE NextFileHandle;
  WIN32_FIND_DATA NextFile;
  char *ptr;

  ptr=strchr(Path,0);
  ptr[0]='*';
  ptr[1]=0;

  SendDlgItemMessage(MainHwnd,IDC_FileList2,LVM_SETITEMCOUNT,(WPARAM)0,0);
  FreeDir(PCDIR_ENTRY);

  // Add Directory Entries
  NextFileHandle=FindFirstFile(Path,&NextFile);

  if(NextFileHandle!=INVALID_HANDLE_VALUE)
  { do
    { if (NextFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        if(!((NextFile.cFileName[0]=='.')&&(NextFile.cFileName[1]==0)))
          AddPCDir(&NextFile,DIR_ENTRIES);
       }
      else
        AddPCDir(&NextFile,FILE_ENTRIES);
       
     } while(FindNextFile(NextFileHandle,&NextFile));
    FindClose(NextFileHandle);
   }

  SortItems(MainHwnd,PCDIR_ENTRY);
  *ptr=0;
 }

int BrowsePCDir(char *Title)
{ BROWSEINFO browse;
  LPITEMIDLIST pidl;
  int result;
  char *ptr, Path[MAXPATH];

  ZeroMemory(&browse, sizeof(browse));
  browse.hwndOwner = MainHwnd;
  browse.lpszTitle = Title;
  //  browse.ulFlags = BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;
  pidl= SHBrowseForFolder(&browse);

  if(pidl && SHGetPathFromIDList(pidl,Path) &&Path[0])
  { ptr=strchr(Path,0);

    // append backslash if there isn't one
    if(ptr[-1]!=SLASH)
    { *ptr++=SLASH;
      *ptr=0;
     }

    SetDlgItemText(MainHwnd,IDC_Path2,Path);
    ScanPCDir(Path);
    result=TRUE;
   }
  else
  { SetDlgItemText(MainHwnd,IDC_Path2,"");
    result=FALSE;
   }

  // Free the PIDL returned by SHBrowseForFolder.
  g_pMalloc->lpVtbl->Free(g_pMalloc, pidl);

  return(result);
 }

int NewPath(char *Path, char *Dir)
{ char *ptr;

  ptr=strchr(Path,0);

  if((Dir[0]=='.')&&(Dir[1]=='.')&&(Dir[2]==0))
  {
    ptr-=2;

    while(ptr>Path)
    { if(ptr[-1]=='\\')
      { *ptr=0;
        return(TRUE);
       }
      ptr--;
     }

    return(FALSE);
   }
  else
    return(wsprintf(ptr,"%s\\",Dir)!=0);
 }

int PC_DirOper(char *Path, int Oper)
{
  HANDLE NextFileHandle;
  WIN32_FIND_DATA NextFile;
  uint len, ok;

  len=lstrlen(Path);

  Path[len]='*';
  Path[len+1]=0;

  NextFileHandle=FindFirstFile(Path,&NextFile);

  Path[len]=0;

  if(NextFileHandle!=INVALID_HANDLE_VALUE)
  { do
    { if (NextFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        // Recurse directories ignoring /. & /.. special directories
        if((NextFile.cFileName[0]!='.') ||
           ((NextFile.cFileName[1]!='.') && (NextFile.cFileName[1]!=0)))
        { wsprintf(&Path[len],"%s\\",NextFile.cFileName);
          CondDebug(RPT_DEBUG,"AFS_Makedir %s",NextFile.cFileName);
          CondDebug(RPT_DEBUG,"AFS_CD %s",NextFile.cFileName);
          PC_DirOper(Path,Oper);
          Path[len]=0;
         }
       }
      else
      { CondDebug(RPT_DEBUG,"Operate on %s%s",Path,NextFile.cFileName);
        ok=TRUE;

        if(!ok)
          break;
       }

     } while(FindNextFile(NextFileHandle,&NextFile));

    CondDebug(RPT_DEBUG,"AFS_cd /");
    FindClose(NextFileHandle);
   }

  return(ok);
 }

