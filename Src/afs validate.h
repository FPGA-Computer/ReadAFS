
int ValidateDirCache(HANDLE Media,ulong Sector,i64 offset,struct Bitmaps *Bitmap);
int ValidateData(HANDLE Media,ulong sector,i64 offset,struct Bitmaps *Bitmap);
int ValidateFS(HANDLE Media,struct Partition *Part,ulong Root,i64 offset,long FS,struct Bitmaps *Bitmap);
int CheckBitMap(HANDLE Media,struct Partition *Part);
int AFS_Validate(struct Partition *Part);

