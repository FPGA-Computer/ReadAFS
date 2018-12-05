#include "afs.h" 

DWORD FileOPThreadID;

void EnableWindows(HWND hwnd,int flag)
{
  Input=flag;
  EnableWindow(GetDlgItem(hwnd,IDC_Abort),!flag);
 }

DWORD WINAPI FileOPTask( LPVOID WHandle )
{ HWND hwnd = (HWND) WHandle;
  MSG msg;
  int Result;

  while(TRUE)
  { WaitMessage();
    Result=GetMessage(&msg,NULL,0,WM_USER);

    if(!Result)
      return(0);

    EnableWindows(hwnd,FALSE);

    Abort=FALSE;
    switch(msg.wParam)
    {
      case FileOP_Validate:
        AFS_Validate(&RDB_Partitions[msg.lParam]);
        break;
/*
      case FileOP_ExportHF:
        AFS_ExportHF(&RDB_Partitions[msg.lParam]);
        break;
*/

      default:
        FileOperations(hwnd,msg.wParam);
     }

    EnableWindows(hwnd,TRUE);

    Abort=FALSE;
    SetTextf(MainHwnd,IDC_StatusText,"");
   }
 }

int FileOperations(HWND hwnd,int Operation)
{
  int  Src=Context, Dst=1-Src, index=-1, len, len2, ok, partition;
  char PCPath[MAXPATH+1];

  if(Src<0)
    return(FALSE);

  len=GetDlgItemText(hwnd,DlgPath[PCDIR_ENTRY],PCPath,sizeof(PCPath));
  partition = SendDlgItemMessage(hwnd,IDC_RDB_Partition,CB_GETCURSEL,0,0);

  // mark all selected
  while((index=SendDlgItemMessage(hwnd,DlgList[Src],LVM_GETNEXTITEM,index,LVNI_SELECTED))!=-1)
  {
    len2= len + wsprintf(&PCPath[len],"%s",DirEntry[Src].Dir[index].Filename);

    if(strcmp(DirEntry[Src].Dir[index].Filename,"/") &&
       strcmp(DirEntry[Src].Dir[index].Filename,".."))
    {
      switch(Operation)
      {
        case FileOP_Move:
        case FileOP_Copy:

          if(Src==ADFDIR_ENTRY)
          {
            if(DirEntry[Src].Dir[index].Attrib&FILE_ENTRIES)
            {
              SetTextf(MainHwnd,IDC_StatusText,"copy %s to %s",DirEntry[Src].Dir[index].Filename,PCPath);

              ok=AFS_CopyFileTo(PCPath,(ulong)DirEntry[Src].Dir[index].FileSize,DirEntry[Src].Dir[index].Sector,
                                RDB_Partitions[partition].FSType,partition,RDB_Partitions[partition].Offset);
             }
            else
            { ok=AFS_FileOPDirTo(PCPath,DirEntry[Src].Dir[index].Sector,
                                (Operation==FileOP_Move)?FileOP_Move|FileOP_Recurse:FileOP_Copy|FileOP_Recurse,
                                 RDB_Partitions[partition].FSType,partition,RDB_Partitions[partition].Offset);
             }
           }
          else
          {
            PCPath[len2]='\\';
            PCPath[len2+1]=0;

            PC_DirOper(PCPath,0);
           }

          if(!ok||(Operation==FileOP_Copy))
            break;

        case FileOP_Delete:
          SetTextf(MainHwnd,IDC_StatusText,"delete %s %s",DirEntry[Src].Dir[index].Attrib&DIR_ENTRIES?"Dir":"file",
                   DirEntry[Src].Dir[index].Filename);
          break;

        case FileOP_MkDir:
          SetTextf(MainHwnd,IDC_StatusText,"mkdir %s",DirEntry[Src].Dir[index].Filename);
          break;

       }

      if(Abort)
      {
        CondDebug(RPT_WARN,"Pending Operation(s) Aborted.");
        break;
       }
      if(!ok)
        CondDebug(RPT_ERR,"Operation Failed.");
     }
   }

  if(ok)
  {
    RescanDir(hwnd,Dst);

    if(Operation & (FileOP_Delete))
      RescanDir(hwnd,Src);
   }
  return(TRUE);
 }

