#include "afs.h"

uint intl_toupper(uint c)
{
  return (c>='a' && c<='z') || (c>=224 && c<=254 && c!=247) ? c - ('a'-'A') : c ;
 }

int AFS_Hash(char *name)
{
  ulong hash=strlen(name);

  for(;*name;name++)
    hash = (hash*13 + intl_toupper((ubyte)*name))& 0x7ff;			/* not case sensitive */

  return(hash%((LOGICAL_BLOCK_SIZE/sizeof(long))-56));  	/* 0 < hash < 71 for 512 byte blocks */
 }

/*
ulong ChecksumLong( void *Block, int Size)
{
  ulong Checksum=0, *lptr = (ulong *) Block;

  for(;Size;Size--)
    Checksum+= *lptr++;

  return(Checksum);
 }
*/

long LittleEndian(long n)
{ long Dst;
  ubyte *src = (ubyte *)&n, *dst= (byte *)&Dst;

  dst[0]=src[3];
  dst[1]=src[2];
  dst[2]=src[1];
  dst[3]=src[0];

  return(Dst);
 }

long ChecksumLong( void *Block, int Size)
{
  ulong Checksum=0, *lptr = (ulong *) Block;

  for(;Size;Size--)
    Checksum+= LittleEndian(*lptr++);

  return(Checksum);
 }

void SwapLongs(void *Dst, int size)
{ register ubyte *ptr= (ubyte *)Dst;
  register ubyte t;

  for(;size;size--,ptr+=4)
  { t=ptr[3];
    ptr[3]=ptr[0];
    ptr[0]=t;
    t=ptr[2];
    ptr[2]=ptr[1];
    ptr[1]=t;
   }
 }

void BlockSwapEndian(void *Dst, int Type)
{ uint start;

  switch(Type)
  { case FS_BootBlock:
      SwapLongs(Dst,sizeof(struct BootBlock)/sizeof(long));
      break;
    case FS_PartBlock:
      SwapLongs(Dst,offsetof(struct myPartitionBlock,pb_DriveName)/sizeof(long));
      start=offsetof(struct myPartitionBlock,pb_Reserved2);
      SwapLongs(&((ubyte*)Dst)[start],(sizeof(struct myPartitionBlock)-start)/sizeof(long));
      break;
    case FS_RootBlock:
      SwapLongs(Dst,offsetof(struct bRootBlock,cTicks)/sizeof(long));
      start=offsetof(struct bRootBlock,days);
      SwapLongs(&((ubyte*)Dst)[start],(sizeof(struct bRootBlock)-start)/sizeof(long));
      break;
    case FS_RDBBlock:
      SwapLongs(Dst,offsetof(struct RigidDiskBlock,rdb_DiskVendor)/sizeof(long));
      break;
    case FS_FileHeader:
      SwapLongs(Dst,offsetof(struct bFileHeaderBlock,commLen)/sizeof(long));
      start=offsetof(struct bFileHeaderBlock,days);
      SwapLongs(&((ubyte*)Dst)[start],(offsetof(struct bFileHeaderBlock,nameLen)-start)/sizeof(long));
      start=offsetof(struct bFileHeaderBlock,r4);
      SwapLongs(&((ubyte*)Dst)[start],(sizeof(struct bFileHeaderBlock)-start)/sizeof(long));
      break;
    case FS_DirBlock:
      SwapLongs(Dst,offsetof(struct bDirBlock,commLen)/sizeof(long));
      start=offsetof(struct bDirBlock,days);
      SwapLongs(&((ubyte*)Dst)[start],(offsetof(struct bDirBlock,nameLen)-start)/sizeof(long));
      start=offsetof(struct bDirBlock,r6);
      SwapLongs(&((ubyte*)Dst)[start],(sizeof(struct bFileHeaderBlock)-start)/sizeof(long));
      break;
    case FS_LinkBlock:
      SwapLongs(Dst,offsetof(struct bLinkBlock,realName)/sizeof(long));
      start=offsetof(struct bLinkBlock,r2);
      SwapLongs(&((ubyte*)Dst)[start],(offsetof(struct bLinkBlock,nameLen)-start)/sizeof(long));
      start=offsetof(struct bLinkBlock,r3);
      SwapLongs(&((ubyte*)Dst)[start],(sizeof(struct bLinkBlock)-start)/sizeof(long));
      break;
    case FS_BitmapBlock:
    case FS_BitmapExtBlock:
      SwapLongs(Dst,sizeof(struct bBitmapBlock)/sizeof(long));
      break;
   }
 }
