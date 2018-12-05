extern HeaderPrefs LB_Header[];

// gui-support
int  ChangeDir(HWND hwnd,int Dlg);
int  ContextMenu(HWND hwnd,LPARAM lParam);
char *OpenFileGUI(HWND hwnd,char *Drive, char *Title, char *LpstrFilter);
void SetHeaderSort(HWND hwnd,int Dlg,HeaderPrefs *LB_Header,int Sort);
int  SortType(HWND hwnd,int Dlg,int Item);
void InitHeaders(HWND hwnd,int Dlg,HeaderPrefs *LB_Header,int Sort);
char *OpenFileGUI(HWND hwnd,char *Drive, char *Title, char *LpstrFilter);
void RescanDir(HWND hwnd,int list);

