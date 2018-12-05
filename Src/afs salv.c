/* Partition: 4GB 8388608 blocks

   1:1 mapping using an array of unsiged long
   8388608 x 4 bytes = 32MB

   22 bits as next index
   10 bits as control
*/

#define Block_Mask		0x003fffff

#define Block_ErrMask	0x00c00000
#define Block_CkSum		0x00400000
#define Block_HWErr		0x00800000

#define Block_TypeMask  0x0f000000

enum BlockTypes
{ Block_Unknown   = 0x00000000,
  Block_Data	   = 0x01000000,
  Block_Root	   = 0x02000000,
  Block_FileHead  = 0x03000000,
  Block_FileExt	= 0x04000000,
  Block_Dir		   = 0x05000000,
  Block_OFSData	= 0x06000000,
  Block_BitMap    = 0x07000000,
  Block_BitMapExt = 0x08000000,
  Block_Link		= 0x09000000,
  Block_DirCach	= 0x0a000000
  // Future use
 };

 ulong *Sector;
 int	Stage=1, HardErrors=0, CRCErrors=0, LinkError=0;

 Sector = (ulong *) myAlloc(Size*sizeof(ulong));
 ZeroMemory(Sector,Size*sizeof(ulong));

 printf("Stage %d scanning\n",Stage);

 depth first(root block)
 {
  ReadSector( )

  if error
    Sector[] |= Block_HWErr;
    errors ++;

  figure out types?
 }

 if (HardErrors==0, CRCErrors==0, LinkError==0)
    & stage = 1 -> exit

 fixes
  root?
  dir links?
  data/data extension?
  bit map?


