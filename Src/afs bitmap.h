// fill value for partially filled bitmap block
#define BMP_FILL			0xff

// these values should have been defined in adf_blk.h
//#define BmpPerBlk			127
#define BmpPerBlk       ((sizeof(struct bBitmapBlock)-offsetof(struct bBitmapBlock,map))/sizeof(long))

//#define BmpBlkPerExt		127
#define BmpBlkPerExt		(offsetof(struct bBitmapExtBlock,nextBlock)/sizeof(long))

struct Bitmaps
{
  ubyte  *Bitmap;          	// Bitmaps
  ulong  *BitmapBlocks;	  		// disk blocks allocated for bitmap
  ulong  *BitmapExtBlocks;		// disk blocks allocated for bitmap extension
  ulong  Sectors;					// sectors
  ulong  Bytes;					// # of bytes needed
  ulong  Blocks;					// # of bBitmapBlock needed
  ulong  ExtBlocks;				// # of bBitmapExtBlock needed
  ulong  Reserved;				// # of blocks reserved
 };

struct Bitmaps *AllocBitmapStruct(struct Partition *Part);
int SetBitmap(struct Bitmaps *Bitmap,ulong Sector);
int CopyBitMapBlock(HANDLE Media,long Bmpage,i64 offset,struct Bitmaps *Bitmap,int copyflag);
CopyBitmaps(HANDLE Media,long *bmPages,long bmExt,i64 offset,struct Bitmaps *Bitmap,int copyflag);
ulong AllocBitMapFrom(struct Bitmaps *Bitmap,ulong From);
int BitmapValid(HANDLE Media,i64 Offset);
int WriteAllBitmaps(HANDLE Media,struct Bitmaps *Bitmap,ulong RootBlock,i64 offset);

