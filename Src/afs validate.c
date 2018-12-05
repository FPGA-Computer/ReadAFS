#include "afs.h"

int ValidateDirCache(HANDLE Media,ulong Sector,i64 offset,struct Bitmaps *Bitmap)
{ ubyte Buffer[LOGICAL_BLOCK_SIZE];
  int ok=TRUE;
  ulong checksum;

  struct bDirCacheBlock *DC = (struct bDirCacheBlock *) Buffer;

  while(Sector)
  { ok &= ReadSector(Media,Buffer,LOGICAL_BLOCK_SIZE,Sector+offset)!=0;
    checksum=ChecksumLong(Buffer,LOGICAL_BLOCK_SIZE/sizeof(long));

    if(checksum)
    { CondDebug(RPT_ERR,"Bad checksum %08x!= 0 at Dir Cache Block Sector:%d",checksum,Sector);
      ok=FALSE;
     }

    ok &= SetBitmap(Bitmap,Sector);
    Sector = LittleEndian(DC->nextDirC);
   }
  return(ok);
 }

int ValidateData(HANDLE Media,ulong sector,i64 offset,struct Bitmaps *Bitmap)
{
  ulong next=sector, *block, checksum;
  int i, ok=TRUE;

  ubyte HeaderBuffer[LOGICAL_BLOCK_SIZE];
  struct bFileHeaderBlock *FH = (struct bFileHeaderBlock *) HeaderBuffer;
// struct bOFSDataBlock   *OFD = (struct bOFSDataBlock *) Buffer;

  while(TRUE)
  {
    // reload fileheader
    if(!next||!(ok &= ReadSector(Media,HeaderBuffer,LOGICAL_BLOCK_SIZE,next+offset)!=0))
      break;

    ok &= SetBitmap(Bitmap,next);
    
    checksum=ChecksumLong(HeaderBuffer,LOGICAL_BLOCK_SIZE/sizeof(long));

    if(checksum)
    { CondDebug(RPT_ERR,"Bad checksum %08x!= 0 at fileheader Sector:%d",checksum,next);
      ok=FALSE;
     }

    BlockSwapEndian(HeaderBuffer,FS_FileHeader);
    next = FH->extension;

    // Copy data blocks
    for(i=FH->highSeq,block=(ulong *)&FH->dataBlocks[MAX_DATABLK-1];i;i--,block--)
      ok &= SetBitmap(Bitmap,*block);
   }
  return(ok);
 }

int ValidateFS(HANDLE Media,struct Partition *Part,ulong Root,i64 offset,long FS,struct Bitmaps *Bitmap)
{ long hashTable[HT_SIZE],next,checksum;
  ubyte Buffer[LOGICAL_BLOCK_SIZE];
  char Name[MAXNAMELEN+2];
  int i, ok;

  struct bRootBlock       *RB  = (struct bRootBlock *) Buffer;
  struct bFileHeaderBlock *FH  = (struct bFileHeaderBlock *) Buffer;
  struct bDirBlock        *DB  = (struct bDirBlock *) Buffer;
  struct bLinkBlock       *LB  = (struct bLinkBlock *) Buffer;

  ok = ReadSector(Media,Buffer,LOGICAL_BLOCK_SIZE,Root+offset)!=0;
  checksum=ChecksumLong(Buffer,LOGICAL_BLOCK_SIZE/sizeof(long));

  if(checksum)
  { lstrcpyn(Name,RB->diskName,MIN(RB->nameLen+1,sizeof(Name)));
    CondDebug(RPT_ERR,"Bad checksum %08x!= 0 at %s '%s' Sector:%d",
              checksum,(RB->type==L_ENDIAN(ST_ROOT))?"root":"dir",Name,Root);
    ok=FALSE;
   }

  // root block
  if((RB->type==L_ENDIAN(T_HEADER)) && (RB->secType==L_ENDIAN(ST_ROOT)))
  {
    BlockSwapEndian(Buffer,FS_RootBlock);
    CopyBitmaps(Media,RB->bmPages,RB->bmExt,offset,Bitmap,FALSE);
   }
  else
    BlockSwapEndian(Buffer,FS_DirBlock);

  ok &= SetBitmap(Bitmap,Root);
  ok &= ValidateDirCache(Media,RB->extension,offset,Bitmap);

  // cache hash table
  CopyMemory(hashTable,RB->hashTable,sizeof(hashTable));

  for(i=0;!Abort&&(i<HT_SIZE);i++)
  { next=hashTable[i];

    while(next)
    { ok &= ReadSector(Media,Buffer,LOGICAL_BLOCK_SIZE,next+offset)!=0;

      if(DB->type==L_ENDIAN(T_HEADER))
      {
        checksum=ChecksumLong(Buffer,LOGICAL_BLOCK_SIZE/sizeof(long));

        if(checksum)
        { lstrcpyn(Name,RB->diskName,MIN(RB->nameLen+1,sizeof(Name)));
          CondDebug(RPT_ERR,"Bad checksum %08x!= 0 Type:%d '%s' Sector:%d",
                    checksum,LittleEndian(RB->type),Name,Root);
          ok=FALSE;
         }

        if (DB->secType==L_ENDIAN(ST_DIR))										// Dir block
        {
          /* Bitmap will be set by recursion */
          BlockSwapEndian(Buffer,FS_DirBlock);
          ok &= ValidateFS(Media,Part,next,offset,FS,Bitmap);		 	   // <-- Recursion
          next=DB->nextSameHash;
         }
        else if(FH->secType==L_ENDIAN(ST_FILE))									// File Header 2/-3
        {
          /* Bitmap will be set by recursion */
          BlockSwapEndian(Buffer,FS_FileHeader);
          ok &= ValidateData(Media,next,offset,Bitmap);
          next=FH->nextSameHash;
         }
        else if((LB->secType==L_ENDIAN(ST_LFILE)) ||                    // Links
                (LB->secType==L_ENDIAN(ST_LDIR))  ||
                (LB->secType==L_ENDIAN(ST_LSOFT)))
        {
          ok &= SetBitmap(Bitmap,next);
          next=LB->nextSameHash;
         }
       }
      else
      { CondDebug(RPT_ERR,"Bad header type %08x != 0x02 at %s",LittleEndian(DB->type),i64toa(next+offset));
        return(FALSE);
       }
     }
   }

  return(Abort?FALSE:ok);
 }

int CheckBitMap(HANDLE Media,struct Partition *Part)
{ ulong RootBlock = Part->RootBlockOffset;
  i64 offset = Part->Offset;
  int result;
  
  struct Bitmaps *Bitmap;

  Bitmap=AllocBitmapStruct(Part);		// Partition entry

  result=FALSE;

  if(Bitmap)
  { SetTextf(MainHwnd,IDC_StatusText,"Reconstructing bitmap from disk structure.");
    result=ValidateFS(Media,Part,RootBlock,offset,Part->FSType,Bitmap);

    if(result)
    { SetTextf(MainHwnd,IDC_StatusText,"Writing bitmap back to disk.");
      WriteAllBitmaps(Media,Bitmap,RootBlock,offset);
     }
    heapfree(Bitmap);
   }
  else
    CondDebug(RPT_ERR,"Unable to allocate memory.");

  if(result)
  { CondDebug(RPT_INFO,"Validation Successful.");
    Part->Valid = TRUE;
    DisplayPartInfo(Part);
   }
  else
    CondDebug(RPT_ERR,"Validation failed.");

  return(result);
 }

int AFS_Validate(struct Partition *Part)
{ int result;
  HANDLE AFS_Media;

  AFS_Media=OpenDrive(Part->DeviceName);

  if(AFS_Media)
  {
    result=CheckBitMap(AFS_Media,Part);
   }
  CloseDrive(AFS_Media);

  return(result);
 }

