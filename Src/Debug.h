enum { RPT_NONE, RPT_ERR, RPT_WARN, RPT_INFO, RPT_DEBUG };

#define RPT_ERR_Prefix		"Error: "
#define RPT_WARN_Prefix		"Warn: "
#define RPT_INFO_Prefix		"Info: "
#define RPT_DEBUG_Prefix	"Debug: "

// Debug
void Puts(char *Msg);
/*void Debug(const char *Format,...); */
void SetTextf(HWND Hwnd,int idc, const char *Format,...);
void CondDebug(int level,const char *Format,...);
void HexDump(ubyte *Block, int Size);
void InitDbg(HWND Hwnd);
void SaveLogFile(HWND Hwnd);
