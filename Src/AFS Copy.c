#include "afs.h"

int AFS_CopyFileXfer(HANDLE AFS_Media,char *Dst,ulong FileSize,ulong sector,long FS,i64 offset)
{ FILE *Outfile;
  ulong next=sector, *block, size;
  int i, ok;

  ubyte HeaderBuffer[LOGICAL_BLOCK_SIZE],CopyBuffer[LOGICAL_BLOCK_SIZE];
  struct bFileHeaderBlock *FH = (struct bFileHeaderBlock *) HeaderBuffer;
  struct bOFSDataBlock    *OD = (struct bOFSDataBlock *) CopyBuffer;

  Outfile=fopen(Dst,"wb");

  if(!Outfile)
    return(FALSE);

  while(FileSize)
  {
    // reload fileheader
    if(!next||!(ok=ReadSector(AFS_Media,HeaderBuffer,LOGICAL_BLOCK_SIZE,next+offset)))
      break;

    BlockSwapEndian(HeaderBuffer,FS_FileHeader);
    next = FH->extension;

    // Copy data blocks
    for(i=FH->highSeq,block=(ulong *)&FH->dataBlocks[MAX_DATABLK-1];i;i--,block--)
    {
      if(!(ok=ReadSector(AFS_Media,CopyBuffer,LOGICAL_BLOCK_SIZE,*block+offset)))
        break;

      if (isFFS(FS))
        size = MIN(LOGICAL_BLOCK_SIZE,FileSize);
      else
        size = MIN(sizeof(OD->data),FileSize);

      FileSize-=size;

      ok=(fwrite(isFFS(FS)?CopyBuffer:OD->data,1,size,Outfile)==size);

      if(!ok)
        break;
     }
   }
  fclose(Outfile);
  return(ok);
 }

int AFS_CopyFileTo(char *Dst,ulong FileSize,ulong sector,long FS,int Part,i64 offset)
{
  HANDLE AFS_Media;
  int ok;

  AFS_Media=OpenDrive(RDB_Partitions[Part].DeviceName);

  if(AFS_Media)
  {
    ok=AFS_CopyFileXfer(AFS_Media,Dst,FileSize,sector,FS,offset);
    CloseDrive(AFS_Media);
   }

  return(ok);
 }

int AFS_FileOPDirTo(char *Dst,ulong sector,int Operation,long FS,int Part,i64 offset)
{
  HANDLE AFS_Media;
  int ok;

  AFS_Media=OpenDrive(RDB_Partitions[Part].DeviceName);

  if(AFS_Media)
  {
    ok=AFS_DirOper(AFS_Media,Operation,sector,offset,FS,Dst);
    CloseDrive(AFS_Media);
   }

  return(ok);
 }

