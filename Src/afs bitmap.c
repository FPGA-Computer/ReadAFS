#include "afs.h"

struct Bitmaps *AllocBitmapStruct(struct Partition *Part)
{ ulong bytes, blocks, extblocks;
  struct Bitmaps *ptr;
  ubyte *BytePtr;

  bytes = (ulong)(Part->Sectors >> 3);

  // ceil
  if(Part->Sectors & 0x07)
    bytes ++;
    
  blocks = bytes/(sizeof(long)* 127);

  // ceil
  if(blocks * sizeof(long)*127!=bytes)
    blocks ++;

  if(blocks < BM_SIZE)									// root block holds < 49.6MB
    extblocks = 0;
  else
  { extblocks = (blocks-BM_SIZE)/127;  			// 1 ext block per 252MB

    // ceil
    if(extblocks * 127 != (blocks-BM_SIZE))
      extblocks++;
   }

  BytePtr = heapcalloc(sizeof(struct Bitmaps)+ bytes + (blocks + extblocks)* sizeof(ulong)+1);
  ptr = (struct Bitmaps *) BytePtr;

  if(BytePtr)
  {
    ptr->Bitmap = &BytePtr[sizeof(struct Bitmaps)];
    ptr->BitmapBlocks = (ulong *)&ptr->Bitmap[bytes];
    ptr->BitmapExtBlocks = (ulong *)&ptr->BitmapBlocks[blocks];
    ptr->Bytes = bytes;
    ptr->Sectors = (ulong) Part->Sectors;
    ptr->Blocks = blocks;
    ptr->ExtBlocks = extblocks;
    ptr->Reserved = Part->Reserved;
    // set bitmap = free
    memset(ptr->Bitmap,0xff,bytes);
   }

  return(ptr);
}

// should also keep track of dirty bitmap pages to minimize writes
int SetBitmap(struct Bitmaps *Bitmap,ulong Sector)
{ register ubyte *byte, bit;

  if(Sector<Bitmap->Sectors)
  { Sector-=Bitmap->Reserved;
    byte=&Bitmap->Bitmap[Sector>>3];
    bit= (ubyte)(1<<(Sector & 0x07));

    if(*byte & bit)
    { *byte &= (ubyte) ~bit;
      return(TRUE);
     }
    CondDebug(RPT_ERR,"SetBitMap() sector:%d marked previously",Sector+Bitmap->Reserved);
   }
  else
    CondDebug(RPT_ERR,"SetBitMap() sector:%d > %d",Sector,Bitmap->Sectors);

  return(FALSE);
 }

int CopyBitMapBlock(HANDLE Media,long Bmpage,i64 offset,struct Bitmaps *Bitmap,int copyflag)
{ ubyte BitmapBuf[LOGICAL_BLOCK_SIZE];
  int result=TRUE,size;

  struct bBitmapBlock *BM  = (struct bBitmapBlock *) BitmapBuf;

  if(copyflag)
  { // load bitmap blocks from disk
    ReadSector(Media,BitmapBuf,LOGICAL_BLOCK_SIZE,Bmpage+offset);
    BlockSwapEndian(BM,FS_BitmapBlock);
    
    size=Bitmap->Bytes-Bitmap->Blocks*sizeof(BM->map);

    CopyMemory(&Bitmap->Bitmap[Bitmap->Blocks*sizeof(BM->map)],BM->map,MIN(size,sizeof(BM->map)));
   }
  else
    SetBitmap(Bitmap,Bmpage);

  Bitmap->BitmapBlocks[Bitmap->Blocks++]=Bmpage;
  return(result);
 }

 // populate the bitmap data structure in RAM
CopyBitmaps(HANDLE Media,long *bmPages,long bmExt,i64 offset,struct Bitmaps *Bitmap,int copyflag)
{ ubyte HeaderBuffer[LOGICAL_BLOCK_SIZE];
  int i,ok=TRUE;
  
  struct bBitmapExtBlock  *BMX = (struct bBitmapExtBlock *) HeaderBuffer;

  Bitmap->Blocks=0;
  Bitmap->ExtBlocks=0;

  for(i=0;i<BM_SIZE;i++)
    ok &= CopyBitMapBlock(Media,bmPages[i],offset,Bitmap,copyflag);

  while(bmExt)
  {
    if(!copyflag)
      SetBitmap(Bitmap,bmExt);

    Bitmap->BitmapExtBlocks[Bitmap->ExtBlocks++]=bmExt;

    ok &= ReadSector(Media,HeaderBuffer,LOGICAL_BLOCK_SIZE,bmExt+offset);	// No checksums
    BlockSwapEndian(BMX,FS_BitmapExtBlock);

    for(i=0;(i<127)&& BMX->bmPages[i];i++)
      ok &= CopyBitMapBlock(Media,BMX->bmPages[i],offset,Bitmap,copyflag);

    bmExt= BMX->nextBlock;
   }
  return(ok);
 }

ulong AllocBitMapFrom(struct Bitmaps *Bitmap,ulong From)
{ ulong Sector,byte;
  ubyte bit;

  for(Sector=From-Bitmap->Reserved;Sector<Bitmap->Sectors;Sector++)
  {
    byte = Sector>>3;
    bit = (ubyte) (1<<(Sector & 0x07));

    if(Bitmap->Bitmap[byte]& bit)
    { Bitmap->Bitmap[byte] &= (ubyte) ~bit;
      return(Sector);
     }
   }

  return(0);
 }

int BitmapValid(HANDLE Media,i64 Offset)
{ ubyte Buffer[LOGICAL_BLOCK_SIZE];
  struct bRootBlock *RB = (struct bRootBlock *) Buffer;

  // -1 is -1 regardless of endianness
  return(ReadSector(Media,Buffer,sizeof(Buffer),Offset) && (RB->bmFlag==-1));
 }

int WriteBitmapBlk(HANDLE Media,ubyte *BitmapPtr,int bytes,ulong Sector,i64 offset)
{ uint size;
  ubyte BmapBlocks[LOGICAL_BLOCK_SIZE];
  struct bBitmapBlock *BB = (struct bBitmapBlock *) BmapBlocks;

  size = MIN(sizeof(BB->map),bytes);
  memset(BB->map,BMP_FILL,sizeof(BB->map));
  memcpy(BB->map,BitmapPtr,size);

  BB->checkSum = 0;
  BlockSwapEndian(BB,FS_BitmapBlock);
  BB->checkSum = LittleEndian(-ChecksumLong(BB,LOGICAL_BLOCK_SIZE/sizeof(long)));
  WriteSector(Media,BmapBlocks,LOGICAL_BLOCK_SIZE,Sector+offset);

  return(size);
 }
 
int WriteAllBitmaps(HANDLE Media,struct Bitmaps *Bitmap,ulong RootBlock,i64 offset)
{ SYSTEMTIME CurTime;
  FILETIME FileTime;
  struct DateStamp *Aros_DateStamp;
  ubyte Buffer[LOGICAL_BLOCK_SIZE],BmapBlocks[LOGICAL_BLOCK_SIZE],*BitmapPtr;
  ulong BMExtBlock, *NextBitmapBlock, *NextExtBlock, blocks, bytes;
  uint i,size;

  struct bRootBlock      *RB = (struct bRootBlock *) Buffer;
  struct bBitmapExtBlock *EB = (struct bBitmapExtBlock *) BmapBlocks;

  ReadSector(Media,Buffer,sizeof(Buffer),RootBlock+offset);
  BlockSwapEndian(Buffer,FS_RootBlock);

  BitmapPtr = Bitmap->Bitmap;
  NextBitmapBlock = Bitmap->BitmapBlocks;
  NextExtBlock = Bitmap->BitmapExtBlocks;
  blocks = Bitmap->Blocks;
  bytes = Bitmap->Bytes;
  
  // store bitmap + pointer in bmPages
  memset(RB->bmPages,0,BM_SIZE*sizeof(long));

  for(i=0;bytes &&(i<BM_SIZE);i++)
  {
    RB->bmPages[i] = *NextBitmapBlock++;
    size=WriteBitmapBlk(Media,BitmapPtr,bytes,RB->bmPages[i],offset);

    BitmapPtr+= size;
    bytes-= size;
    blocks--;
   }

  while(blocks)
  {
    // Create Extension block
    BMExtBlock = *NextExtBlock++;

    size = MIN(blocks,BmpBlkPerExt);

    memset(EB->bmPages,0,sizeof(EB->bmPages));
    memcpy(EB->bmPages,NextBitmapBlock,size*sizeof(long));

    EB->nextBlock = (blocks>BmpBlkPerExt)?*NextExtBlock:0;

    // write Bitmap extension block
    BlockSwapEndian(EB,FS_BitmapExtBlock);
    WriteSector(Media,BmapBlocks,sizeof(BmapBlocks),BMExtBlock+offset);

    // copy bitmap to bitmap blocks
    for(i=0;bytes && (i<BmpBlkPerExt);i++)
    {
      size=WriteBitmapBlk(Media,BitmapPtr,bytes,(*NextBitmapBlock++),offset);
      blocks--;
      BitmapPtr+= size;
      bytes -= size;
     }
   }

  // Bitmap is now valid
  RB->bmFlag = -1;

  // Update last access date stamp

  GetLocalTime(&CurTime);
  SystemTimeToFileTime(&CurTime,&FileTime);
  Aros_DateStamp = FileTime_AFS(&FileTime);

  RB->days  =  Aros_DateStamp->ds_Days;
  RB->mins  =  Aros_DateStamp->ds_Minute;
  RB->ticks =  Aros_DateStamp->ds_Tick;

  RB->checkSum = 0;
  BlockSwapEndian(Buffer,FS_RootBlock);
  RB->checkSum = LittleEndian(-ChecksumLong(Buffer,LOGICAL_BLOCK_SIZE/sizeof(long)));

  WriteSector(Media,Buffer,sizeof(Buffer),RootBlock+offset);
  return(TRUE);
 }

