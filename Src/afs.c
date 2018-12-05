#include "afs.h"

HWND MainHwnd, MenuHwnd;
HINSTANCE hInst;
LPMALLOC g_pMalloc;

int Context, Abort=FALSE, Input=TRUE /*, MBR=TRUE */;
int DlgList[]   = {IDC_FileList1,IDC_FileList2};
int DlgPath[]   = {IDC_Path1,IDC_Path2};

//-----------------------------------------------------------------------
BOOL CALLBACK ConsoleProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{ int index;

  switch(msg)
  { case WM_INITDIALOG:
      InitDbg(hwnd);
      break;

    case WM_COMMAND:        													// Buttons
      switch(LOWORD(wParam))
      {
        case IDC_SaveLog:
          SaveLogFile(hwnd);
          break;

        case IDC_ClearLog:
          SendDlgItemMessage(hwnd,IDC_DBG,LB_RESETCONTENT,0,0);
          break;

        case IDC_ReportLevel:
          if((HIWORD(wParam)==CBN_CLOSEUP)&&((index=SendDlgItemMessage(hwnd,IDC_ReportLevel,CB_GETCURSEL,0,0))!=CB_ERR))
            WarnLevel=index;
          break;
       }
    default:
      return FALSE;
   }
  return TRUE;
 }

BOOL CALLBACK DeviceProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{ int index;

  switch(msg)
  { case WM_INITDIALOG:
      MenuHwnd = hwnd;
//      SendDlgItemMessage(hwnd,IDC_MBROption,BM_SETCHECK,MBR?BST_CHECKED:BST_UNCHECKED,0);
      break;
    case WM_COMMAND:        													// Buttons
      if(Input) switch(LOWORD(wParam))
      {
/*
        case IDC_MBROption:
          if(HIWORD(wParam)==BN_CLICKED)
            MBR=SendDlgItemMessage(hwnd,IDC_MBROption,BM_GETCHECK,0,0);
          break;
*/
        case IDC_AROS_Device:
          if(HIWORD(wParam)==CBN_DROPDOWN)
          { EnumDevice(hwnd);
            break;
           }
          else if(HIWORD(wParam)==CBN_CLOSEUP)
            SelectAROS(hwnd);

        case IDC_RDB_Partition:
          if(HIWORD(wParam)!=CBN_CLOSEUP)
            break;
          // Otherwise fall through

        case IDC_Mount_ADF:		 		// Fake command sent from Main window
          index = SendDlgItemMessage(hwnd,IDC_RDB_Partition,CB_GETCURSEL,0,0);

          if(index!=CB_ERR)
          {
            EnableWindow(GetDlgItem(hwnd,IDC_Validate),!RDB_Partitions[index].Valid);

            if (RDB_Partitions[index].Valid)
              AFS_CD(hwnd,":",NULL,0);

//          EnableWindow(GetDlgItem(hwnd,IDC_WriteHF),(RDB_Partitions[index].Media!=DRIVE_HARDFILE));
           }
          EnableWindow(GetDlgItem(MainHwnd,IDC_Mount_ADF),index!=CB_ERR);
          break;
/*
        case IDC_WriteHF:
          index=SendDlgItemMessage(hwnd,IDC_RDB_Partition,CB_GETCURSEL,0,0);

          if(index!=CB_ERR)
            PostThreadMessage(FileOPThreadID,WM_USER,FileOP_ExportHF,index);
          break;
*/
        case IDC_Validate:
          index=SendDlgItemMessage(hwnd,IDC_RDB_Partition,CB_GETCURSEL,0,0);

          if(index!=CB_ERR)
            PostThreadMessage(FileOPThreadID,WM_USER,FileOP_Validate,index);
          break;
       }
    default:
      return FALSE;
   }
  return TRUE;
 }

BOOL CALLBACK MainDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{ int code,n,index;

  switch(msg)
  { case WM_INITDIALOG:
    { INITCOMMONCONTROLSEX InitCtrlEx;
      RECT Rc,Rc1;
      InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
      InitCtrlEx.dwICC  = ICC_DATE_CLASSES;

      InitCommonControlsEx(&InitCtrlEx);

      n=SendDlgItemMessage(hwnd,IDC_FileList1,LVM_GETEXTENDEDLISTVIEWSTYLE,0,0);
      SendDlgItemMessage(hwnd,IDC_FileList1,LVM_SETEXTENDEDLISTVIEWSTYLE,0,
                         n|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP);

      n=SendDlgItemMessage(hwnd,IDC_FileList2,LVM_GETEXTENDEDLISTVIEWSTYLE,0,0);
      SendDlgItemMessage(hwnd,IDC_FileList2,LVM_SETEXTENDEDLISTVIEWSTYLE,0,
                         n|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP);

      CoInitialize(NULL);       // Init for SHBrowseForFolder Function
      SHGetMalloc(&g_pMalloc);

      MainHwnd=hwnd;

      SetWindowText(hwnd,VERSION);
      InitHeaders(hwnd,IDC_FileList1,LB_Header,SORT_ASC);
      InitHeaders(hwnd,IDC_FileList2,LB_Header,SORT_ASC);

      DirMutex=CreateMutex(NULL,FALSE,DirMutexName);
      CreateThread(NULL,0,FileOPTask,hwnd,0,&FileOPThreadID);

      // Tab menu
      n=AddTabs(hwnd,IDC_TabMenu,Tabs);
      OpenTab(hwnd,IDC_TabMenu,&TabTop,Tabs);

      // Adjust status window
      SendDlgItemMessage(hwnd,IDC_TabMenu,TCM_GETITEMRECT,n,(LPARAM)&Rc);
      GetWindowRect(GetDlgItem(hwnd,IDC_StatusText),&Rc1);
      SetWindowPos(GetDlgItem(hwnd,IDC_StatusText),HWND_BOTTOM,Rc1.left+Rc.right,Rc1.top-22,
                   Rc1.right-Rc1.left-Rc.right,Rc1.bottom-Rc1.top,SWP_NOZORDER|SWP_SHOWWINDOW);

      SendDlgItemMessage(hwnd,IDC_ProgressBar,PBM_SETSTEP,2,0);
     }
     break;

    case WM_CLOSE:
      CoUninitialize();
      SendDlgItemMessage(hwnd,IDC_FileList1,LVM_SETITEMCOUNT,(WPARAM)0,0);
      SendDlgItemMessage(hwnd,IDC_FileList2,LVM_SETITEMCOUNT,(WPARAM)0,0);
      FreeDir(ADFDIR_ENTRY);
      FreeDir(PCDIR_ENTRY);

      WaitForSingleObject(DirMutex,INFINITE);
      PostQuitMessage(WM_QUIT);

      CloseHandle(DirMutex);
      DestroyWindow(hwnd);
      break;

    case WM_NOTIFY:
      code = ((LPNMHDR)lParam)->code;

      switch(code)
      { case TCN_SELCHANGE:
          if(LOWORD(wParam)==IDC_TabMenu)
            OpenTab(hwnd,IDC_TabMenu,&TabTop,Tabs);
          break;

        case LVN_ENDLABELEDIT :
        { LV_DISPINFO *lpdi = (LV_DISPINFO *)lParam;

          if(Input && (lpdi->item.pszText))
            if (LOWORD(wParam)==IDC_FileList1)
            {
              CondDebug(RPT_DEBUG,"AROS Rename (%d) %s --> %s",lpdi->item.iItem,DirEntry[0].Dir[lpdi->item.iItem].Filename,lpdi->item.pszText);
             }
            else if (LOWORD(wParam)==IDC_FileList2)
            {
              CondDebug(RPT_DEBUG,"PC Rename (%d) %s --> %s",lpdi->item.iItem,DirEntry[1].Dir[lpdi->item.iItem].Filename,lpdi->item.pszText);
             }
         }
        break;

        case NM_DBLCLK:
          return( Input?ChangeDir(hwnd,LOWORD(wParam)):FALSE );

        case LVN_COLUMNCLICK:
          if((LOWORD(wParam)==IDC_FileList1)||(LOWORD(wParam)==IDC_FileList2))
          { index=((NM_LISTVIEW *)lParam)->iSubItem;
            n=(SortType(hwnd,LOWORD(wParam),index)==SORT_ASC)?SORT_DES:SORT_ASC;
            index=(index & SORT_MASK)|n;
            SetHeaderSort(hwnd,LOWORD(wParam),LB_Header,index);
            SortItems(hwnd,(LOWORD(wParam)==IDC_FileList2));
           }
          break;

        case LVN_GETDISPINFO:
        { LV_DISPINFO *lpdi = (LV_DISPINFO *)lParam;

          if(lpdi->item.mask & LVIF_TEXT)
            return( Display_Dir(lpdi->item.pszText,lpdi->item.cchTextMax,(LOWORD(wParam)==IDC_FileList2),
                               (ulong)lpdi->item.iItem,lpdi->item.iSubItem));
          return(FALSE);
         }

        default:
          return(FALSE);
       }
      break;

    case WM_CONTEXTMENU:
      return(Input?ContextMenu(hwnd,lParam):FALSE);

    case WM_COMMAND:        													// Buttons

      if((LOWORD(wParam)==IDC_Abort)&& !Abort)
      { SetTextf(MainHwnd,IDC_StatusText,"Aborting...");
        Abort= TRUE;
       }
      else if (Input)
        switch(LOWORD(wParam))
        { case IDC_PC_Dir:
            BrowsePCDir("Please select PC Folder");
            break;

          case IDC_Refresh:
            RescanDir(hwnd,PCDIR_ENTRY);
            break;

          case IDC_Mount_ADF:
            SendMessage(MenuHwnd,WM_COMMAND,wParam,lParam);
            break;

          case CM_MakDir:
            PostThreadMessage(FileOPThreadID,WM_USER,FileOP_MkDir,lParam);
            break;
          case CM_Copy:
            PostThreadMessage(FileOPThreadID,WM_USER,FileOP_Copy,lParam);
            break;
          case CM_Move:
            PostThreadMessage(FileOPThreadID,WM_USER,FileOP_Move,lParam);
            break;
         }
      break;

    default:
      return FALSE;
   }

  return TRUE;
 }

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
  hInst=hInstance;
//  return(DialogBox(hInstance,MAKEINTRESOURCE(IDD_MainDialog),NULL,MainDlgProc));
return(DialogBox(hInstance,MAKEINTRESOURCE(IDD_DeviceMenu),NULL,DeviceProc));

}
