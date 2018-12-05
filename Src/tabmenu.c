#include "afs.h"

int  TabTop=1;

TabItems Tabs[]=
{ { "Main Menu",DeviceProc,MAKEINTRESOURCE(IDD_DeviceMenu)},
  { "Logging",ConsoleProc,MAKEINTRESOURCE(IDD_Console)},
  { NULL,NULL,NULL }
 };

uint AddTabs(HWND hwnd,int dlg, TabItems tab[])
{ TC_ITEM item;
  RECT TabRect;

  int i, cyMargin;
  long dwDlgBase;

  ZeroMemory(&item,sizeof(item));

  item.mask= TCIF_TEXT|TCIF_PARAM;

  for(i=0;tab[i].Menu;i++)
  { item.pszText = tab[i].Menu;
    item.cchTextMax = strlen(tab[i].Menu)+1;
    item.lParam=i;

    SendDlgItemMessage(hwnd,dlg,TCM_INSERTITEM,(WPARAM)i,(LPARAM)&item);
   }

  SendDlgItemMessage(hwnd,dlg,TCM_ADJUSTRECT,(WPARAM)FALSE,(LPARAM)&TabRect);

  dwDlgBase = GetDialogBaseUnits();
//  cxMargin = LOWORD(dwDlgBase) / 4;
  cyMargin = HIWORD(dwDlgBase) / 8;
  OffsetRect(&TabRect,0,cyMargin*9);

  for(i=0;tab[i].Menu;i++)
  { tab[i].Hwnd=CreateDialog(hInst,tab[i].Resource,hwnd,tab[i].Proc);
   }
  return(i-1);
 }

void OpenTab(HWND hwnd,int dlg,int *Top,TabItems tab[])
{ int sel;

  SetWindowPos(tab[*Top].Hwnd,HWND_BOTTOM,0,0,0,0,
                 SWP_NOSIZE|SWP_NOMOVE|SWP_HIDEWINDOW);

  sel=SendDlgItemMessage(hwnd,dlg,TCM_GETCURSEL,0,0);
  *Top=(sel==-1)?0:sel;      // Default to first tab

  SetWindowPos(tab[*Top].Hwnd,HWND_TOP,0,0,0,0,
               SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);
 }
