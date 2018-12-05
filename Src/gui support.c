#include "afs.h"

HeaderPrefs LB_Header[]=
{{"Name",FNAME_WIDTH,LVCFMT_LEFT},
 {"Size (Bytes)",FSIZE_WIDTH,LVCFMT_RIGHT},
 {"Modified",FDATE_WIDTH,LVCFMT_LEFT},
 {NULL}
 };

void SetHeaderSort(HWND hwnd,int Dlg,HeaderPrefs *LB_Header,int Sort)
{ int i;
  LV_COLUMN header;
  char Buffer[HEADER_WIDTH];

  header.mask = LVCF_TEXT;

  for(i=0;LB_Header[i].Label;i++)
  { if((SORT_MASK & Sort)==i)
    {
      wsprintf(Buffer,"%s %s",LB_Header[i].Label,(Sort & SORT_DES)?SORT_SYM_DES:SORT_SYM_ASC);
      header.pszText = Buffer;
     }
    else
    {
      wsprintf(Buffer,"%s",LB_Header[i].Label);
      header.pszText = Buffer;
     }

    SendDlgItemMessage(hwnd,Dlg,LVM_SETCOLUMN,i,(LPARAM)&header);
   }
 }

int SortType(HWND hwnd,int Dlg,int Item)
{
  LV_COLUMN header;
  char Buffer[HEADER_WIDTH];

  header.mask = LVCF_TEXT;
  header.pszText = Buffer;
  header.cchTextMax = HEADER_WIDTH;

  if(SendDlgItemMessage(hwnd,Dlg,LVM_GETCOLUMN,Item,(LPARAM) &header))
  { if(strstr(Buffer,SORT_SYM_ASC))
      return(SORT_ASC);
    else if(strstr(Buffer,SORT_SYM_DES))
      return(SORT_DES);
    }

  return(0);
 }

void InitHeaders(HWND hwnd,int Dlg,HeaderPrefs *LB_Header,int Sort)
{ int i;
  LV_COLUMN header;

  header.mask = LVCF_FMT|LVCF_SUBITEM|LVCF_WIDTH|LVCF_TEXT;

  for(i=0;LB_Header[i].Label;i++)
  { header.pszText = LB_Header[i].Label;
    header.cx = LB_Header[i].Width;
    header.iSubItem=i;
    header.fmt = LB_Header[i].Format;

    SendDlgItemMessage(hwnd,Dlg,LVM_INSERTCOLUMN,i,(LPARAM) &header);
   }

  if(Sort!=NOSORT)
    SetHeaderSort(hwnd,Dlg,LB_Header,Sort);
 }

char *OpenFileGUI(HWND hwnd,char *Drive, char *Title, char *LpstrFilter)
{ OPENFILENAME DirBox;
  static char Path[MAXPATH]={0}, PrevDrive[MAXPATH]={0};

  if(Drive && strcmpi(PrevDrive,Drive))
  {
    Path[0]=0;													// Reset path if drives changed
    lstrcpyn(PrevDrive,Drive,sizeof(PrevDrive));
   }

  ZeroMemory(&DirBox, sizeof(DirBox));
  DirBox.lStructSize = sizeof(DirBox);
  DirBox.hwndOwner = hwnd;
  DirBox.lpstrFile = Path;
  DirBox.lpstrInitialDir = Drive;
  DirBox.lpstrFilter = LpstrFilter;
  DirBox.nMaxFile = MAX_PATH;
  DirBox.lpstrTitle = Title;
  DirBox.Flags = OFN_EXPLORER | OFN_LONGNAMES;

  if (GetOpenFileName(&DirBox))
  {  return(Path);
   }
  else
    return(NULL);
 }

int ChangeDir(HWND hwnd,int Dlg)
{
   char Path[AROS_MAXPATH],Dir[AROS_MAXPATH];
   int n, index;

   n=SendDlgItemMessage(hwnd,Dlg,LVM_GETSELECTEDCOUNT,0,0);

   if(n==1)
   { index=SendDlgItemMessage(hwnd,Dlg,LVM_GETNEXTITEM,-1,LVNI_SELECTED);
     n=(Dlg==IDC_FileList1)?0:1;

     if(!(DirEntry[n].Dir[index].Attrib&DIR_ENTRIES))
       return(FALSE);

     lstrcpyn(Dir,DirEntry[n].Dir[index].Filename,sizeof(Dir));

     if(Dlg==IDC_FileList1)
     { GetDlgItemText(hwnd,IDC_Path1,Path,sizeof(Path));

       if(AFS_CD(hwnd,Path,Dir,DirEntry[0].Dir[index].Sector))
       {
         SetDlgItemText(hwnd,IDC_Path1,Path);
        }
       return(TRUE);
      }
     else if (Dlg==IDC_FileList2)
     { GetDlgItemText(hwnd,IDC_Path2,Path,sizeof(Path));

       if(NewPath(Path,Dir))
       { ScanPCDir(Path);
         SetDlgItemText(hwnd,IDC_Path2,Path);
        }

       return(TRUE);
      }
    }
   return(FALSE); 
 }

void RescanDir(HWND hwnd,int list)
{
  char Path[MAXPATH+1];

  if(list==ADFDIR_ENTRY)
    AFS_CD(hwnd,"/",NULL,0);
  else
  {
    GetDlgItemText(hwnd,IDC_Path2,Path,sizeof(Path));

    if(*Path)
      ScanPCDir(Path);
   }  
 }

int ContextMenu(HWND hwnd,LPARAM lParam)
{ RECT rc;
  POINT pt;
  HMENU hmenu,Popup;
  char Buffer1[2],Buffer2[2];
  int i,n;

  pt.x = (short)LOWORD(lParam);
  pt.y = (short)HIWORD(lParam);

  Context=-1;

//  for(i=0;i<LISTWINDOWS;i++)
i=0;
  { GetDlgItemText(hwnd,DlgPath[i],Buffer1,sizeof(Buffer1));
    GetWindowRect(GetDlgItem(hwnd,DlgList[i]),&rc);

    if(PtInRect(&rc, pt)&&Buffer1[0])
    { n=SendDlgItemMessage(hwnd,DlgList[i],LVM_GETSELECTEDCOUNT,0,0);

      // Check if path exists for other window (does not work for >2 ;)
      GetDlgItemText(hwnd,DlgPath[LISTWINDOWS-1-i],Buffer2,sizeof(Buffer2));

      if((n==0)||Buffer2[0])
      { hmenu = LoadMenu(hInst,MAKEINTRESOURCE(IDM_MENU1));

        if(hmenu)
        { Context=i;
          Popup = GetSubMenu(hmenu,n!=0);

          TrackPopupMenu(Popup,TPM_LEFTALIGN | TPM_RIGHTBUTTON,pt.x,pt.y,0,hwnd,NULL);
          DestroyMenu(hmenu);
          CloseHandle(Popup);
          return(TRUE);
         }
       }
     }
   }

  return(FALSE);
 }

