// very inefficient macro - for constants only
#define L_ENDIAN(X) (long)((((X)>>24)&0x000000ff)|(((X)>>8)&0x0000ff00)|(((X)<<8)&0x00ff0000)|(((X)<<24)&0xff000000))

enum FSBlocks { FS_BootBlock, FS_PartBlock, FS_RootBlock, FS_RDBBlock, FS_FileHeader,
                FS_DirBlock, FS_LinkBlock, FS_BitmapBlock, FS_BitmapExtBlock };

void SwapLongs(void *Dst, int size);
void BlockSwapEndian(void *Dst, int Type);
long LittleEndian(long n);
long ChecksumLong( void *Block, int Size);
uint intl_toupper(uint c);
int AFS_Hash(char *name);

