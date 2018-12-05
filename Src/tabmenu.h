// tab control
typedef struct
{ char       *Menu;
  BOOL        CALLBACK (*Proc)(HWND,UINT,WPARAM,LPARAM);
  LPCSTR      Resource;
  HWND        Hwnd;
 } TabItems;

uint  AddTabs(HWND hwnd,int dlg, TabItems tab[]);
void  OpenTab(HWND hwnd,int dlg,int *Top,TabItems tab[]);

extern TabItems Tabs[];
extern RECT TabRect;
extern int  TabTop;
