#include "afs.h"

int AFS_DirOper(HANDLE Media,int Operation,ulong Root,i64 offset,long FS,char *PCPath)
{ long hashTable[HT_SIZE],next;
  ubyte Buffer[LOGICAL_BLOCK_SIZE];
  char Name[MAXNAMELEN*2+8];
  int i, len, copyok=TRUE, ok=TRUE;

  struct bRootBlock       *RB = (struct bRootBlock *) Buffer;
  struct bFileHeaderBlock *FH = (struct bFileHeaderBlock *) Buffer;
  struct bDirBlock        *DB = (struct bDirBlock *) Buffer;

  // Root block or Dir block - close enough
  ReadSector(Media,Buffer,LOGICAL_BLOCK_SIZE,Root+offset);
  BlockSwapEndian(Buffer,FS_DirBlock);

  // Fake a parent directory
  if(Operation & FileOP_Parent)
    AddEntry(ADFDIR_ENTRY,"/",0,AFS_FileTime(RB->cDays,RB->cMins,RB->cTicks),DIR_ENTRIES,RB->parent);

  if(Operation & FileOP_Copy)
  {
    if(Abort)
      return(FALSE);

    mkdir(PCPath);
    SetTextf(MainHwnd,IDC_StatusText,"mkdir %s",PCPath);
   }

  // cache hash table
  CopyMemory(hashTable,RB->hashTable,sizeof(hashTable));
  len=lstrlen(PCPath);

  for(i=0;!Abort&&(i<HT_SIZE);i++)
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

          if(Operation & FileOP_List)
            AddEntry(ADFDIR_ENTRY,Name,0,AFS_FileTime(DB->days,DB->mins,DB->ticks),DIR_ENTRIES,next);

          if(Operation & FileOP_Recurse)
          {
            wsprintf(&PCPath[len],"\\%s",Name);
            ok &= AFS_DirOper(Media,Operation,next,offset,FS,PCPath); // <-- Recursion

            PCPath[len]=0;
           }

          next=DB->nextSameHash;
         }
        else if(FH->secType==L_ENDIAN(ST_FILE))						// File Header 2/-3
        {
          BlockSwapEndian(Buffer,FS_FileHeader);
          lstrcpyn(Name,FH->fileName,MIN(FH->nameLen+1,sizeof(Name)));

          if(Operation & FileOP_List)
            AddEntry(ADFDIR_ENTRY,Name,FH->byteSize,AFS_FileTime(FH->days,FH->mins,FH->ticks),FILE_ENTRIES,next);

          if(!Abort && (Operation & FileOP_Copy))
          {
            wsprintf(&PCPath[len],"\\%s",Name);
            SetTextf(MainHwnd,IDC_StatusText,"copy %s to %s",Name,PCPath);

            ok &= copyok = AFS_CopyFileXfer(Media,PCPath,FH->byteSize,next,FS,offset);
           }
          if(Operation & FileOP_Delete & copyok)
          {
            SetTextf(MainHwnd,IDC_StatusText,"delete %s",Name);
           }
          next=FH->nextSameHash;
         }
        else
          next=DB->nextSameHash;
       }
      else
      { CondDebug(RPT_ERR,"Bad header type %08x != 0x02 at %s",LittleEndian(DB->type),i64toa(next+offset));
        return(FALSE);
       }
     }
   }

  if(!Abort && ok && (Operation & FileOP_Delete))
  {
    CondDebug(RPT_DEBUG,"AFS DeleteDir %s",Name);
   }

  return(ok);
 }

/*
void AFS_ExportHF(struct Partition *Part)
{
 }
*/


