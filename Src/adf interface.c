#include "afs.h"
#include "adf interface.h"

RETCODE Win32ReadSector(struct Device *dev, long n, int size, unsigned char* buf)
{

  return  ReadSector(HANDLE Handle, ubyte *Buffer, long Size,DWORD Offset);
 }

RETCODE Win32WriteSector(struct Device *dev, long n, int size, unsigned char* buf)
{

  return  WriteSector(HANDLE Handle, ubyte *Buffer, long Size, DWORD Offset);
 }

RETCODE Win32InitDevice(struct Device *dev, char* name, BOOL ro)
{
  return OpenDrive(char *DeviceName);
 }

RETCODE Win32ReleaseDevice(struct Device *dev)
{
  return CloseDrive(HANDLE DevHandle);
 }

BOOL Win32IsDevNative(char*)
{ ;
 }

void adfInitNativeFct()
{
	struct nativeFunctions *nFct = (struct nativeFunctions*)adfEnv.nativeFct;

	nFct->adfInitDevice = Win32InitDevice;
	nFct->adfNativeReadSector = Win32ReadSector;
	nFct->adfNativeWriteSector = Win32WriteSector;
	nFct->adfReleaseDevice = Win32ReleaseDevice;
	nFct->adfIsDevNative = Win32IsDevNative;
 }


// Following code replace adf_env.c

int WarnLevel;

void ReportError(char* msg)
{
  if(WarnLevel>=REPORT_WARNING)
    Debug("Error: <%s>",msg);
 }

void ReportWarning(char* msg)
{
  if(WarnLevel>=REPORT_ERROR)
    Debug("Warning: <%s>",msg);
 }

void ReportVerbose(char* msg)
{
  if(WarnLevel>=REPORT_VERBOSE)
    Debug("Verbose: <%s>",msg);
 }

void ReportChanged(SECTNUM nSect, int changedType)
{
 }

void rwHeadAccess(SECTNUM physical, SECTNUM logical, BOOL write)
{
  if(WarnLevel>=REPORT_NAGGING)
    Debug("phy %ld / log %ld : %c",physical,logical,write?'W':'R');
 }

void ReportProgress(int Percent)
{
  SendDlgItemMessage(MainHwnd,IDC_ProgressBar,PBM_SETPOS,PerCent,0);
 }

void adfEnvInitDefault()
{
  adfEnv.wFct           = ReportWarning;
  adfEnv.eFct           = ReportError;
  adfEnv.vFct           = ReportVerbose;
  adfEnv.notifyFct      = ReportChanged;
  adfEnv.rwhAccess      = rwHeadAccess;
  adfEnv.progressBar    = ReportProgress;

  adfEnv.useDirCache    = FALSE;
  adfEnv.useRWAccess    = FALSE;
  adfEnv.useNotify      = FALSE;
  adfEnv.useProgressBar = FALSE;

  WarnLevel = REPORT_ERROR;

  ShowWindow(GetDlgItem(MainHwnd,IDC_ProgressBar),adfEnv.useProgressBar?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(MainHwnd,IDC_ProgressLbl),adfEnv.useProgressBar?SW_SHOW:SW_HIDE);

  adfEnv.nativeFct=(struct nativeFunctions*)malloc(sizeof(struct nativeFunctions));

  if (!adfEnv.nativeFct)
    (*adfEnv.wFct)("adfInitDefaultEnv : malloc");

  adfInitNativeFct();
 }

/*
 * adfEnvCleanUp
 *
 */
void adfEnvCleanUp()
{
    free(adfEnv.nativeFct);
} 

/*
 * adfGetVersionNumber
 *
 */
char* adfGetVersionNumber()
{
	return(ADFLIB_VERSION);
}

/*
 * adfGetVersionDate
 *
 */
char* adfGetVersionDate()
{
	return(ADFLIB_DATE);
}
