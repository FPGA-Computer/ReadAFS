#include "afs.h"

HWND DBGHwnd;
int WarnLevel = RPT_INFO;
char *LogMsg[] = { "None","Errors","Warnings","Verbose","Everything" };

void InitDbg(HWND Hwnd)
{ HFONT hFont;
  int index;
  
  DBGHwnd=Hwnd;

  for(index=0;index<sizeof(LogMsg)/sizeof(char *);index++)
    SendDlgItemMessage(Hwnd,IDC_ReportLevel,CB_ADDSTRING,0,(LPARAM)LogMsg[index]);

  SendDlgItemMessage(Hwnd,IDC_ReportLevel,CB_SETCURSEL,WarnLevel,0);

  // Use fixed size font for hex dump
  hFont = CreateFont(-12,8,0,0,FW_LIGHT,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
                     CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,
                     "Courier New");
  SendDlgItemMessage(Hwnd,IDC_DBG,WM_SETFONT,(WPARAM)hFont,1);
 }

void SaveLogFile(HWND Hwnd)
{ char *logfile, Buffer[256];
  FILE *outfile;
  int i,n;

  logfile=OpenFileGUI(Hwnd,NULL,"Logfile save as",NULL);

  if(logfile)
  {
    outfile=fopen(logfile,"w");

    if(outfile)
    {
      n=SendDlgItemMessage(Hwnd,IDC_DBG,LB_GETCOUNT,0,0);
      for(i=0;i<n;i++)
      { SendDlgItemMessage(Hwnd,IDC_DBG,LB_GETTEXT,i,(LPARAM)Buffer);
        fprintf(outfile,"%s\n",Buffer);
       }

      fclose(outfile);
     }
   }
 }


void CondDebug(int Level, const char *Format,...)
{ char Buffer[256];
  char *Prefix[]= {"",RPT_ERR_Prefix,RPT_WARN_Prefix,RPT_INFO_Prefix,RPT_DEBUG_Prefix};
  int n;

  va_list args;
  va_start (args,Format);

  if(WarnLevel>=Level)
  {
    n=wsprintf(Buffer,"%s",Prefix[Level]);
    wvsprintf(&Buffer[n],Format,args);
    SendDlgItemMessage(DBGHwnd,IDC_DBG,LB_ADDSTRING,0,(LPARAM)Buffer);
   }

  va_end (args);
 }

void SetTextf(HWND Hwnd,int idc, const char *Format,...)
{ char Buffer[256];
  int n;

  va_list args;

  va_start (args,Format);
  n=wvsprintf(&Buffer[2],Format,args);
  SetDlgItemText(Hwnd,idc,&Buffer[2]);

  if(n && (WarnLevel>=RPT_INFO))
  {
    Buffer[0]='>';
    Buffer[1]=' ';
    SendDlgItemMessage(DBGHwnd,IDC_DBG,LB_ADDSTRING,0,(LPARAM)Buffer);
   }
  va_end (args);
 }


