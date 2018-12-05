#include <windows.h>
#include <winbase.h>
#include <winnt.h>
#include <commctrl.h>
#include <winioctl.h>

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <alloc.h>
#include <dir.h>
#include <string.h>
#include <shlobj.h>

#define VERSION				"AFS Util Version 0.1 - KCL - Copyright(C) 2007"

#define MAXDRIVES				32
#define MAXDRIVENAME   		32

//  \\.\PhysicalDrivenn
#define MAXDEVICENAME		(MAXPATH+1)
#define MAX_RDBPART			16

#define LINEWIDTH				16
#define BOOTBLOCK				0
#define SLASH 					'\\'

#define LISTWINDOWS			2

// Listing window width
#define FNAME_LEN			   42
#define FSIZE_LEN				14
#define FDATE_LEN			   18

#define FNAME_WIDTH        (FNAME_LEN*6)
#define FSIZE_WIDTH        (FSIZE_LEN*6)
#define FDATE_WIDTH        (FDATE_LEN*6)

#define HEADER_WIDTH			32

// Header sort
#define NOSORT 				0x8000
#define SORT_DES				0x0200
#define SORT_ASC				0x0100
#define SORT_MASK				0x00ff

// Fake up/down arrows
//#define SORT_SYM_DES       "\\/"
//#define SORT_SYM_ASC       "/\\"

#define SORT_SYM_DES       "v"
#define SORT_SYM_ASC       "^"

#define AROS_PARTITION		0x30

// There is no artifical limits on path names, but we have to give it something
#define AROS_MAXPATH			4096

typedef unsigned char 	ubyte;
typedef unsigned char 	UBYTE;
typedef long          	LONG;
typedef short           word;
typedef unsigned short	uword;
typedef unsigned short	UWORD;
typedef unsigned long 	ulong;
typedef unsigned int		uint;
typedef __int64  			i64;
typedef unsigned char	*BPCLSTR;

typedef struct
{ char  *Label;
  uword  Width;
  uword  Format;
 } HeaderPrefs;

// Skip loading some of the AROS headers
#define EXEC_TYPES_H
#define AROS_SYSTEM_H

// AROS include files
#include "aros/macros.h"
#include "devices/bootblock.h"
#include "devices/hardblocks.h"
#include "adf_blk.h"

#include "debug.h"
#include "afs rc.h"
#include "new win.h"
#include "afs part.h"
#include "afs path.h"
#include "date.h"
#include "device.h"
#include "dir entry.h"
#include "gui support.h"
#include "afs fileop.h"
#include "pc fileoper.h"
#include "file oper.h"
#include "afs support.h"
#include "afs validate.h"
#include "afs bitmap.h"
#include "tabmenu.h"

#define MIN(X,Y) ((X)<(Y)?(X):(Y))


extern char *FilePrefix[];
extern HWND MainHwnd, MenuHwnd, DBGHwnd;
extern HINSTANCE hInst;
extern LPMALLOC g_pMalloc;
extern struct Drives DriveDB[];
extern int Context, Abort, Input;
extern int DlgList[], DlgPath[];
extern int WarnLevel;

extern i64 SectorOffset;
extern int RawOpened;

//MainHwnd,IDC_StatusText

int  CALLBACK dircmp(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
BOOL CALLBACK ConsoleProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DeviceProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK MainDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nCmdShow);

