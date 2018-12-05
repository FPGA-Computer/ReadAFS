#include "afs.h"

DirBlock DirEntry[2] = { {NULL,NULL,0,0},{NULL,NULL,0,0} };
HANDLE   DirMutex=NULL;
char     *DirMutexName = "Dir Mutex";

char *i64toa(i64 n)
{ static char Buffer[33];

  _ui64toa(n,Buffer,10);
  return Buffer;
 }

void *heaprealloc(void *block, size_t size)
{
  return(block?HeapReAlloc(GetProcessHeap(),0,block,size)
              :HeapAlloc(GetProcessHeap(),0,size));
 }

void *heapcalloc(size_t size)
{ return(HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size));
 }

void heapfree(void *block)
{ HeapFree(GetProcessHeap(),0,block);
 }

// localized block allocator - can only free all memory all at once
void *myalloc(int Entry,uint size)
{ StringAlloc *ptr;
  ubyte *block;

  if ((!DirEntry[Entry].Pool)||(DirEntry[Entry].Pool->free+size>BLOCK_SIZE))
  { ptr=heapcalloc(sizeof(StringAlloc));

    if(!ptr)
      return(NULL);

    ptr->next=DirEntry[Entry].Pool;   // prepend new block to front
    DirEntry[Entry].Pool=ptr;
   }

  block= &DirEntry[Entry].Pool->storage[DirEntry[Entry].Pool->free];
  DirEntry[Entry].Pool->free += size;

  return((void *)block);
 }

int AddEntry(int Entry, char *Filename,i64 FileSize,FILETIME *LocalTime,uword	Attrib, ulong Sector)
{ DirList *tempptr;
  ulong Size=DirEntry[Entry].Size,NewSize;
  int len;

  // auto-incr size of array
  if (Size>=DirEntry[Entry].NextSize)
  {
    NewSize=Size+DIRSIZE_INC;
    tempptr=heaprealloc(DirEntry[Entry].Dir,NewSize*sizeof(DirList));

    if(!tempptr)
      return(FALSE);

    DirEntry[Entry].Dir=tempptr;

    ZeroMemory(&DirEntry[Entry].Dir[Size],(NewSize-DirEntry[Entry].Size)*sizeof(DirList));
    DirEntry[Entry].NextSize=NewSize;
   }

  tempptr=&DirEntry[Entry].Dir[Size];
  len=strlen(Filename);

  tempptr->Filename = myalloc(Entry,len+1);
  lstrcpyn(tempptr->Filename,Filename,len+1);

  tempptr->FileSize = FileSize;
  tempptr->LocalTime.dwLowDateTime  =  LocalTime->dwLowDateTime;
  tempptr->LocalTime.dwHighDateTime =  LocalTime->dwHighDateTime;
  tempptr->Attrib = Attrib;
  tempptr->Sector = Sector;
  DirEntry[Entry].Size++;

  return(TRUE);
 }

void FreeDir(int Entry)
{ StringAlloc *ptr, *ptr1;

  // Free strings
  WaitForSingleObject(DirMutex,INFINITE);
  ptr= DirEntry[Entry].Pool;

  while(ptr)
  { ptr1=ptr->next;
    heapfree(ptr);
    ptr=ptr1;
   }

  if (DirEntry[Entry].Dir)
  { heapfree(DirEntry[Entry].Dir);
   }

  ZeroMemory(&DirEntry[Entry],sizeof(DirBlock));
  ReleaseMutex(DirMutex);
 }

int Display_Dir(char *Dst, int size, int Entry, ulong item, ulong iSubItem)
{ SYSTEMTIME LocalSystime;
  int result=FALSE,len;

  WaitForSingleObject(DirMutex,INFINITE);

  if(item<DirEntry[Entry].Size)
  { result=TRUE;

    if(!iSubItem)
      lstrcpyn(Dst,DirEntry[Entry].Dir[item].Filename,size);
    else if(iSubItem==1)
    {
      if(DirEntry[Entry].Dir[item].Attrib & DIR_ENTRIES)
        lstrcpyn(Dst,DIR_LABEL,size);
      else if(DirEntry[Entry].Dir[item].Attrib & LINK_ENTRIES)
        lstrcpyn(Dst,LINK_LABEL,size);
      else
        _ui64toa((i64)DirEntry[Entry].Dir[item].FileSize,Dst,10);
     }
    else if(iSubItem==2)
    { FileTimeToSystemTime(&DirEntry[Entry].Dir[item].LocalTime,&LocalSystime);
      len=GetDateFormat(0,DATE_SHORTDATE,&LocalSystime,NULL,Dst,size);
      Dst[len-1]=' ';

      GetTimeFormat(0,0,&LocalSystime,"hh:mmt",Dst+len,size-len);
     }
    else
      result=FALSE;
   }

  ReleaseMutex(DirMutex);
  return(result);
 }

// If sizes or time is the same, sort by name
//-------------------------------------------------------------------------------
int listcmp_Name(const void *ptr1, const void *ptr2)
{
  DirList *File1=(DirList *)ptr1, *File2=(DirList *)ptr2;

  if(File1->Attrib < File2->Attrib)
    return(-1);
  else if (File1->Attrib == File2->Attrib)
    return(lstrcmpi(File1->Filename,File2->Filename));
  return(1);
 }

int listcmp_Size(const void *ptr1, const void *ptr2)
{ 
  DirList *File1=(DirList *)ptr1, *File2=(DirList *)ptr2;

  if(File1->FileSize < File2->FileSize)
    return(-1);
  else if(File1->FileSize == File2->FileSize)
    return(listcmp_Name(ptr1,ptr2));
  return(1);
 }

int listcmp_Time(const void *ptr1, const void *ptr2)
{
  DirList *File1=(DirList *)ptr1, *File2=(DirList *)ptr2;
  int result =  CompareFileTime(&File1->LocalTime,&File2->LocalTime);

  return(result?result:listcmp_Name(ptr1,ptr2));
 }

//-------------------------------------------------------------------------------
int listcmpdes_Name(const void *ptr1, const void *ptr2)
{ return(-listcmp_Name(ptr1,ptr2)); }

int listcmpdes_Size(const void *ptr1, const void *ptr2)
{ return(-listcmp_Size(ptr1,ptr2)); }

int listcmpdes_Time(const void *ptr1, const void *ptr2)
{ return(-listcmp_Time(ptr1,ptr2)); }

//-------------------------------------------------------------------------------
int (_USERENTRY *listcmpasc[])(const void *, const void *)
= { listcmp_Name,listcmp_Size,listcmp_Time };

int (_USERENTRY *listcmpdes[])(const void *, const void *)
= { listcmpdes_Name,listcmpdes_Size,listcmpdes_Time };

//-------------------------------------------------------------------------------

void Sort(int Entry,int Asc, int Column)
{ qsort(DirEntry[Entry].Dir,DirEntry[Entry].Size,sizeof(DirList),Asc?listcmpasc[Column]:listcmpdes[Column]);
 }

void SortItems(HWND hwnd,int Entry)
{ int n, index;

  for(index=0;LB_Header[index].Label;index++)
  { n=SortType(hwnd,DlgList[Entry],index);

    if(n& (SORT_DES|SORT_ASC))
    { index=(index & SORT_MASK);
      Sort(Entry,!(n&SORT_DES),index);
      SendDlgItemMessage(hwnd,DlgList[Entry],LVM_SETITEMCOUNT,DirEntry[Entry].Size,0);
      break;
     }
   }
 }


