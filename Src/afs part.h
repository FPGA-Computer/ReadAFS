// MBR   http://en.wikipedia.org/wiki/Master_boot_record

#define PRIMARY_PART	4

enum Pri_Partition
{ MBR_Status     = 0x00,
  MBR_Start_CHS  = 0x01,
  MBR_Type       = 0x04,
  MBR_End_CHS    = 0x05,
  MBR_LBA_Start0 = 0x08,
  MBR_LBA_Start1 = 0x09,
  MBR_LBA_Start2 = 0x0a,
  MBR_LBA_Start3 = 0x0b,
  MBR_LBA_Len0   = 0x0c,
  MBR_LBA_Len1   = 0x0d,
  MBR_LBA_Len2   = 0x0e,
  MBR_LBA_Len3   = 0x0f,
  MBR_EntrySize  = 0x10
 };

enum MBR_Partition
{
  MBR_BootCode   = 0x000,
  MBR_Sig_Opt    = 0x1B8,
  MBR_Pads       = 0x1BC,
  MBR_Partition  = 0x1BE,
  MBR_Sig_55     = 0x1FE,
  MBR_Sig_AA     = 0x1FF
 };

// Partition block definition - Incomplete one in compiler header

struct myPartitionBlock
{
    ULONG   pb_ID;
    ULONG   pb_SummedLongs;
    LONG    pb_ChkSum;
    ULONG   pb_HostID;
    ULONG   pb_Next;
    ULONG   pb_Flags;
    ULONG   pb_Reserved1[2];
    ULONG   pb_DevFlags;
    UBYTE   pb_DriveName[32];
    ULONG   pb_Reserved2[15];
    ULONG   pb_VectorSize;				// 128/80 	ulong 	1 	size of vector 	== 16 (longs), 11 is the minimal value
    ULONG   pb_SizeBlock;        	// 132/84 	ulong 	1 	SizeBlock	size of the blocks in longs == 128 for BSIZE = 512
    ULONG   pb_SecOrg;					// 136/88 	ulong 	1 	SecOrg 		== 0
    ULONG   pb_Surfaces;         	// 140/8c 	ulong 	1 	Surfaces 	number of heads (surfaces) of drive
    ULONG   pb_SectorsPerBlock;  	// 144/90 	ulong 	1 	sectors/block 	sectors per block == 1
    ULONG   pb_BlocksPerTrack;   	// 148/94 	ulong 	1 	blocks/track 	blocks per track
    ULONG   pb_DOS_Reserved;     	// 152/98 	ulong 	1 	Reserved 	DOS reserved blocks at start of partition usually = 2 (minimum 1)
    ULONG   pb_PreAlloc;				// 156/9c 	ulong 	1 	PreAlloc 	DOS reserved blocks at end of partition
    ULONG   pb_Interleave;				// 160/a0 	ulong 	1 	Interleave 	== 0
    ULONG   pb_LowCyl;					// 164/a4 	ulong 	1 	LowCyl		first cylinder of a partition (inclusive)
    ULONG   pb_HighCyl;					// 168/a8 	ulong 	1 	HighCyl		last cylinder of a partition (inclusive)
    ULONG   pb_NumBuffer;				// 172/ac 	ulong 	1 	NumBuffer 	often 30 (used for buffering)
    ULONG   pb_BufMemType;				// 176/b0 	ulong 	1 	BufMemType 	type of mem to allocate for buffers ==0
    ULONG   pb_MaxTransfer;			// 180/b4 	ulong 	1 	MaxTransfer 	max number of type to transfer at a type
    ULONG   pb_Mask;						// 184/b8 	ulong 	1 	Mask 		Address mask to block out certain memory
    ULONG   pb_BootPri;					// 188/bc 	ulong		1 	BootPri 	boot priority for autoboot
    char    pb_DosType[4];				// 192/c0 	char		4	DosType 	'DOS' and the FFS/OFS flag only
    ULONG   pb_Baud;						// 196/c4   ulong		1	Baud 		Define default baud rate for Commodore's
    ULONG   pb_Control;					// 200/c8   ulong		1	Control		used by Commodore's AUX handler
    ULONG   pb_Bootblocks;				// 204/cc   ulong		1	Bootblocks	Kickstart 2.0: number of blocks
    ULONG   pb_EReserved[12]; 		// 208/d0	ulong		12 RESERVED
 };

#define RDB_OFFSET	1			// RDB located at Sector Offset 1

#define DRIVE_HARDFILE	0xff

struct Drives
{ char   DriveName[MAXDRIVENAME];
  char   DeviceName[MAXPATH+1];
  byte   MediaType;
  ubyte  PartitionType;
  ubyte  PartitonNumber;
  i64		Offset;
  i64		Size;
 };

struct Partition
{ int   DeviceIndex;                 // Not used for Win32, but probably can be used as Unit# for AROS
  char  DeviceName[MAXPATH+1];
  char  VolumeName[MAXNAMELEN+1];
  long  FSType;
  ulong RootBlockOffset;
  i64   Offset;                      // offset into bootblock
  i64   Sectors;                     // total number of sectors
  ulong Reserved;                    // AROS dos reserved
  ubyte Media;                       // Media type
  ubyte Valid;                       // Bitmap Valid
 };

extern struct Partition RDB_Partitions[];
extern int RDB_Parts;

void AddPartMenu(HWND hwnd,int DeviceIndex, char *DeviceName, BPCLSTR VolumeName,ulong FSType,
                 i64 Offset,ulong RootBlockOffset,i64 Sectors,ulong Reserved,ubyte Media,ubyte Valid);
int SelectPartition(HWND hwnd,HANDLE Media,char *DeviceName,int Index);
void SelectAROS(HWND hwnd);

// Enum
uint mystrncpy(char *dst,char *src,uint len);
int PartitionInfo(char *Device,uint Drv,byte media, struct Drives *DB);
void EnumDevice(HWND hwnd);

