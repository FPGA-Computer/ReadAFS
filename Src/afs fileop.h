/*
#define AFSFF_HASHMASK	0x00ff
#define AFSFF_TYPEMASK	0xff00
*/

#define AFSFF_DIR			0x0100
#define AFSFF_FILE   	0x0200
#define AFSFF_LINK    	0x0400
#define AFSFF_PARENT		0x1000

// Control attributes
#define AFSFF_HASH	   0x8000

// Simplified header block that's common to Root/fileheader/extension/link

struct GenericBlock
{
  /*000*/  long type;
  /*004*/  long r1[5];
  /*018*/  long hashTable[HT_SIZE];
  /*138*/  long r2[30];
  /*1b0*/  char nameLen;
  /*1b1*/  char Name[MAXNAMELEN+1];
  /*1d0*/  long r3[8];
  /*1f0*/  long nextSameHash;
  /*1f4*/  long parent;
  /*1f8*/  long extension;
  /*1fc*/  long secType;
 };


int AFS_CopyFileXfer(HANDLE AFS_Media,char *Dst,ulong FileSize,ulong sector,long FS,i64 offset);
int AFS_CopyFileTo(char *Dst,ulong FileSize,ulong sector,long FS,int Part,i64 offset);
int AFS_DirOper(HANDLE Media,int Operation,ulong Root,i64 offset,long FS,char *PCPath);
int AFS_FileOPDirTo(char *Dst,ulong sector,int Operation,long FS,int Part,i64 offset);

void AFS_ExportHF(struct Partition *Part);


