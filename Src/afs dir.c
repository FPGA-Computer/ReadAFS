#include "afs.h"

void AFS_Dir(HWND hwnd,HANDLE Media,ulong Root,i64 offset,int Parent)
{ long hashTable[HT_SIZE],next;
  ubyte Buffer[LOGICAL_BLOCK_SIZE];
  char Name[MAXNAMELEN*2+8];
  int i,len;

  struct bRootBlock       *RB = (struct bRootBlock *) Buffer;
  struct bFileHeaderBlock *FH = (struct bFileHeaderBlock *) Buffer;
  struct bDirBlock        *DB = (struct bDirBlock *) Buffer;
  struct bLinkBlock		  *LB = (struct bLinkBlock *) Buffer;

  // Root block or Dir block - close enough
  ReadSector(Media,Buffer,LOGICAL_BLOCK_SIZE,Root+offset);
  BlockSwapEndian(Buffer,FS_DirBlock);

  // Fake a parent directory
  if(Parent)
    AddEntry(ADFDIR_ENTRY,"/",0,AFS_FileTime(RB->cDays,RB->cMins,RB->cTicks),DIR_ENTRIES,RB->parent);

  // cache hash table
  CopyMemory(hashTable,RB->hashTable,sizeof(hashTable));

  for(i=0;i<HT_SIZE;i++)
  { next=hashTable[i];

    while(next)
    { ReadSector(Media,Buffer,LOGICAL_BLOCK_SIZE,next+offset);

      if(DB->type==L_ENDIAN(T_HEADER))
      {
        // list dir
        if (DB->secType==L_ENDIAN(ST_DIR))							// Dir block
        {
          BlockSwapEndian(Buffer,FS_DirBlock);
          lstrcpyn(Name,DB->dirName,MIN(DB->nameLen+1,sizeof(Name)));
          AddEntry(ADFDIR_ENTRY,Name,0,AFS_FileTime(DB->days,DB->mins,DB->ticks),DIR_ENTRIES,next);
          next=DB->nextSameHash;
         }
        else if((LB->secType==L_ENDIAN(ST_LFILE)) || 				// Links -4/4/3
                (LB->secType==L_ENDIAN(ST_LDIR))  ||
                (LB->secType==L_ENDIAN(ST_LSOFT)))
        {
          BlockSwapEndian(Buffer,FS_LinkBlock);
          len=LB->nameLen;
          lstrcpyn(Name,LB->name,len);
          len+=wsprintf(&Name[len]," -> ");
          lstrcpyn(&Name[len],&LB->realName[1],LB->realName[0]+1);				// Assuming BPCL string

          AddEntry(ADFDIR_ENTRY,Name,0,AFS_FileTime(LB->days,LB->mins,LB->ticks),LINK_ENTRIES,LB->realEntry);
          next=LB->nextSameHash;
         }
        else if(FH->secType==L_ENDIAN(ST_FILE))						// File Header 2/-3
        {
          BlockSwapEndian(Buffer,FS_FileHeader);
          lstrcpyn(Name,FH->fileName,MIN(FH->nameLen+1,sizeof(Name)));
          AddEntry(ADFDIR_ENTRY,Name,FH->byteSize,AFS_FileTime(FH->days,FH->mins,FH->ticks),FILE_ENTRIES,next);
          next=FH->nextSameHash;
         }
       }
      else
      { CondDebug(RPT_ERR,"Error: Bad header type %08x != 0x02 at %s",LittleEndian(DB->type),i64toa(next+offset));
        return;
       }
     }
   }
  SortItems(hwnd,ADFDIR_ENTRY);
 }

// This function returns the sector # of the file/directory
// returns 0 if not found.

ulong AFS_FindFile(HANDLE Media,char *Name,long Type,ulong start,i64 offset)
{
  ubyte Buffer[LOGICAL_BLOCK_SIZE];

  struct GenericBlock *GB = (struct GenericBlock *) Buffer;
  long   next=start;

  ReadSector(Media,Buffer,LOGICAL_BLOCK_SIZE,next+offset);

  if(Type & AFSFF_PARENT)
    return(LittleEndian(GB->parent));
    
  next = LittleEndian(GB->hashTable[AFS_Hash(Name)]);

  do
  { ReadSector(Media,Buffer,LOGICAL_BLOCK_SIZE,next+offset);

    if (GB->type==L_ENDIAN(T_HEADER))
    {
      if (GB->secType==L_ENDIAN(ST_DIR))
      {
        if (!(Type & AFSFF_DIR) || strncmpi(Name,GB->Name,GB->nameLen))
          next=LittleEndian(GB->nextSameHash);
        else
          return((ulong)next);
       }
      else if (GB->secType==L_ENDIAN(ST_FILE))
      {
        if (!(Type & AFSFF_FILE) || strncmpi(Name,GB->Name,GB->nameLen))
          next=LittleEndian(GB->nextSameHash);
        else
          return((ulong)next);
       }
      else if ((GB->secType==L_ENDIAN(ST_LFILE))    ||
               (GB->secType==L_ENDIAN(ST_LDIR))     ||
               (GB->secType==L_ENDIAN(ST_LSOFT)))
      {
        if (!(Type & AFSFF_LINK) || strncmpi(Name,GB->Name,GB->nameLen))
          next=LittleEndian(GB->nextSameHash);
        else
          return((ulong)next);
       }
     }
    else
    { CondDebug(RPT_ERR,"Error: Bad header type %08x != 0x02 at %s",LittleEndian(GB->type),i64toa(next+offset));
      break;
     }
   } while(next);

  return(0);
 }

