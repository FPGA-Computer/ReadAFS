// ---------------------------------------------------------
// InitCommonControlsEx()
typedef struct tagINITCOMMONCONTROLSEX
{ DWORD dwSize;
  DWORD dwICC;
} INITCOMMONCONTROLSEX, *LPINITCOMMONCONTROLSEX;

#define ICC_WIN95_CLASSES 				0x00ff
#define ICC_DATE_CLASSES 				0x0100
#define ICC_USEREX_CLASSES 			0x0200
#define ICC_COOL_CLASSES 				0x0400

BOOL WINAPI InitCommonControlsEx(LPINITCOMMONCONTROLSEX);

typedef struct tagNMSELCHANGE {
    NMHDR nmhdr;
    SYSTEMTIME stSelStart;
    SYSTEMTIME stSelEnd;
} NMSELCHANGE, *LPNMSELCHANGE;

// Virtual Listview - make a copy in the *.rc
#ifndef LVS_OWNERDATA
#define LVS_OWNERDATA 0x1000
#endif

#ifndef LVS_EX_FULLROWSELECT
#define LVS_EX_GRIDLINES            0x00000001
#define LVS_EX_SUBITEMIMAGES        0x00000002
#define LVS_EX_CHECKBOXES       		0x00000004
#define LVS_EX_TRACKSELECT      		0x00000008
#define LVS_EX_HEADERDRAGDROP   		0x00000010
#define LVS_EX_FULLROWSELECT    		0x00000020
#define LVS_EX_ONECLICKACTIVATE 		0x00000040
#define LVS_EX_TWOCLICKACTIVATE		0x00000080
#define LVS_EX_FLATSB               0x00000100
#define LVS_EX_REGIONAL					0x00000200
#define LVS_EX_INFOTIP              0x00000400
#define LVS_EX_UNDERLINEHOT			0x00000800
#define LVS_EX_UNDERLINECOLD        0x00001000
#define LVS_EX_MULTIWORKAREAS			0x00002000
#define LVS_EX_LABELTIP 				0x00004000
#define LVS_EX_NOHSCROLL				0x10000000

#define LVM_FIRST							0x1000
#define LVM_GETEXTENDEDLISTVIEWSTYLE (LVM_FIRST + 0x37)
#define LVM_SETEXTENDEDLISTVIEWSTYLE (LVM_FIRST + 0x36)
#endif
