#include "afs.h"

ulong CurAFS_Root;

int AFS_NewPath(char *Path, char *Dir)
{
  char *ptr=strchr(Path,0);

  if((Dir[0]=='/') && (Dir[1]==0))			// Go up 1 level
  {
    for(;ptr>Path;ptr--)
    { if(*ptr=='/')
      { *ptr=0;
        return(DIRLEVEL_UP);
       }
      if(*ptr==':')
      { ptr[1]=0;
        return(DIRLEVEL_UP);
       } 
     }
    return(FALSE);
    }
   else                                   // Go down 1 level
   { wsprintf(ptr,ptr[-1]!=':'?"/%s":"%s",Dir);
     return(DIRLEVEL_DOWN);
    }
 }

int AFS_CD(HWND hwnd, char *Path, char *Dir, ulong sector)
{ int index;
  HANDLE AFS_Media;
  char PathName[MAXNAMELEN+2];
  ulong NewAFS_Root,RootBlock;

  index=SendDlgItemMessage(hwnd,IDC_RDB_Partition,CB_GETCURSEL,0,0);

  if(index!=CB_ERR)
  {
    AFS_Media=OpenDrive(RDB_Partitions[index].DeviceName);
    RootBlock = (ulong) RDB_Partitions[index].RootBlockOffset;

    if(AFS_Media)
    {
      FreeDir(ADFDIR_ENTRY);

      if(!strcmp(Path,"/"))          					// Magic string for current dir
        NewAFS_Root = CurAFS_Root;
      else if(!strcmp(Path,":"))                   // Magic string for current root
      {
        DisplayPartInfo(&RDB_Partitions[index]);
        wsprintf(PathName,"%s:",RDB_Partitions[index].VolumeName);
        SetDlgItemText(MainHwnd,IDC_Path1,PathName);
        NewAFS_Root = RootBlock;
       }
      else
      { AFS_NewPath(Path,Dir);
        NewAFS_Root = sector;
       }

      SetTextf(MainHwnd,IDC_StatusText,"Scanning for directory entries...");

      AFS_DirOper(AFS_Media,(NewAFS_Root!=RootBlock)?FileOP_List|FileOP_Parent:FileOP_List,
                  NewAFS_Root,RDB_Partitions[index].Offset,RDB_Partitions[index].FSType,NULL);

      SetTextf(MainHwnd,IDC_StatusText,"");
      SortItems(MainHwnd,ADFDIR_ENTRY);
      
      CurAFS_Root = NewAFS_Root;

      CloseDrive(AFS_Media);
      return(NewAFS_Root!=0);
     }  
   }

  return(FALSE);
 }

void DisplayPartInfo(struct Partition *Part)
{
  SetTextf(MenuHwnd,IDC_PartStatus,"%s: %d MiB, %d Sectors, Offset:%s. Bitmap: %s",
           (Part->Media==DRIVE_REMOVABLE)?"Floppy":(Part->Media==DRIVE_HARDFILE)?"Hardfile":"Hard Disk",
           (ulong)(Part->Sectors/2048),(ulong)Part->Sectors,i64toa(Part->Offset),
           (Part->Valid)?"Valid":"Invalid");
 }


