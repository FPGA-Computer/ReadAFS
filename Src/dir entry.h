#define BLOCK_SIZE 				(8*1024L-64)
#define DIRSIZE_INC 				(BLOCK_SIZE/sizeof(DirList))

#define PCDIR_ENTRY				1
#define ADFDIR_ENTRY				0

// Basic directory attributes
#define DIR_ENTRIES			0x01
#define FILE_ENTRIES			0x02
#define LINK_ENTRIES			0x04

#define DIR_LABEL				"< Directory >"
#define LINK_LABEL			"< Link >"

typedef struct StrAlloc
{ uint			free;
  struct		   StrAlloc *next;
  ubyte			storage[BLOCK_SIZE];
 } StringAlloc;

typedef struct
{ char			*Filename;
  i64				FileSize;
  FILETIME     LocalTime;
  uword			Attrib;
  ulong			Sector;
 } DirList;

typedef struct
{ DirList		*Dir;
  StringAlloc	*Pool;
  ulong			Size;
  ulong			NextSize;
 } DirBlock;

extern DirBlock DirEntry[];
extern HANDLE   DirMutex;
extern char     *DirMutexName;

void *heaprealloc(void *block, size_t size);
void *heapcalloc(size_t size);
void  heapfree(void *block);
void *myalloc(int Entry,uint size);
int AddEntry(int Entry, char *Filename,i64 FileSize,FILETIME *LocalTime,uword	Attrib, ulong Sector);
void  FreeDir(int Entry);
int   Display_Dir(char *Dst, int size, int Entry, ulong item, ulong iSubItem);
void  RefreshList(HWND hwnd,int Entry);
char *i64toa(i64 n);
void  SortItems(HWND hwnd,int Entry);
void  Sort(int Entry,int Asc, int Column);

void InitHeaders(HWND hwnd,int Dlg,HeaderPrefs *LB_Header,int Sort);
int  SortType(HWND hwnd,int Dlg,int Item);
void SetHeaderSort(HWND hwnd,int Dlg,HeaderPrefs *LB_Header,int Sort);

