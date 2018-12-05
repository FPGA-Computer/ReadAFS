#include "afs.h"

struct Partition RDB_Partitions[MAX_RDBPART];
int RDB_Parts=0;


struct Drives DriveDB[MAXDRIVES];
int Drives=0;

char *DriveTypes[]=
{ "Unknown",           	// 0
  NULL,                 // 1
  "Removable",          // DRIVE_REMOVABLE
  "Hard Disk",          // DRIVE_FIXED
  "Net Share",          // DRIVE_REMOTE
  "CD/DVD",             // DRIVE_CDROM
  "Ram Drive"           // DRIVE_RAMDISK
 };

uint mystrncpy(char *dst,char *src,uint len)
{ uint l;

  len--;
  for(l=0;len && src && *src;len--,l++)      // copy to end of src or len
    *dst++=*src++;

  *dst=0;                    						// terminate the string
  return(l);
 }


// This is a quick and dirty hack - for hard file compatibility

int PartitionInfo(char *Device,uint Drv,ubyte media, struct Drives *DB)
{ HANDLE DeviceHandle;
  uint aros_parts,i,Entry;
  ubyte Buffer[LOGICAL_BLOCK_SIZE];

  DeviceHandle = OpenDrive(Device);

  if(!DeviceHandle)
    return(-1);

  if(ReadSector(DeviceHandle,Buffer,LOGICAL_BLOCK_SIZE,0)&&
    (Buffer[MBR_Sig_55]==0x55)&&(Buffer[MBR_Sig_AA]==0xAA))
  {
    for(i=0,aros_parts=0;i<PRIMARY_PART;i++)
    {
      Entry = MBR_Partition + MBR_EntrySize*i;

      if(((Buffer[Entry+MBR_Status]==0x00)||(Buffer[Entry+MBR_Status]==0x80)) &&
         (Buffer[Entry+MBR_Type]==AROS_PARTITION))
      {
        wsprintf(DB->DriveName,"HD%d:P%d [AROS]",Drv,i);
        lstrcpyn(DB->DeviceName,Device,MAXDEVICENAME);
        DB->MediaType=media;

        DB->PartitionType=Buffer[Entry+MBR_Type];
        DB->PartitonNumber=(ubyte)i;

        DB->Offset =  Buffer[Entry+MBR_LBA_Start0]     |(Buffer[Entry+MBR_LBA_Start1]<<8) |
                     (Buffer[Entry+MBR_LBA_Start2]<<16)|(Buffer[Entry+MBR_LBA_Start3]<<24);

        DB->Size   =  Buffer[Entry+MBR_LBA_Len0]       |(Buffer[Entry+MBR_LBA_Len1]<<8) |
                     (Buffer[Entry+MBR_LBA_Len2]<<16)  |(Buffer[Entry+MBR_LBA_Len3]<<24);;
        DB++;
        aros_parts++;
       }
     }
   }

  CloseDrive(DeviceHandle);
  return(aros_parts);
 }

void EnumDevice(HWND hwnd)
{ int len,drvlen,i,partition;
  char Buffer[256];

  ZeroMemory(DriveDB,sizeof(DriveDB));
  SetTextf(MainHwnd,IDC_StatusText,"Scanning for local drives");
  len=GetLogicalDriveStrings(sizeof(Buffer),Buffer);

  SendDlgItemMessage(hwnd,IDC_AROS_Device,CB_RESETCONTENT,0,0);
  Drives=0;
  
  // Enum D=Logical Drives
  for(i=0;i<len;i+=drvlen+1)
  { drvlen=mystrncpy(DriveDB[Drives].DriveName,&Buffer[i],MAXDRIVENAME);

    // Logical drive: \\.\x:
    Buffer[i+drvlen-1]=0;
    wsprintf(DriveDB[Drives].DeviceName,"\\\\.\\%s",&Buffer[i]);

    DriveDB[Drives].MediaType = (ubyte)GetDriveType(DriveDB[Drives].DriveName);

    if(DriveTypes[DriveDB[Drives].MediaType])
    { wsprintf(&DriveDB[Drives].DriveName[3]," [%s]",DriveTypes[DriveDB[Drives].MediaType]);
      Drives++;
     }
   }

  // Enum Physical Drives
  for(i=0;Drives<MAXDRIVES;i++)
  {   wsprintf(DriveDB[Drives].DeviceName,"\\\\.\\PhysicalDrive%d",i);
      partition=PartitionInfo(DriveDB[Drives].DeviceName,i,0,&DriveDB[Drives]);

      if(partition<0)
        break;
      Drives+=partition;
   }

  for(i=0;i<Drives;i++)
  { SendDlgItemMessage(hwnd,IDC_AROS_Device,CB_INSERTSTRING,i,(LPARAM)DriveDB[i].DriveName);
   }

  SetTextf(MainHwnd,IDC_StatusText,"");
 }

void AddPartMenu(HWND hwnd,int DeviceIndex, char *DeviceName, BPCLSTR VolumeName,ulong FSType,
                 i64 Offset,ulong RootBlockOffset,i64 Sectors,ulong Reserved,ubyte Media,ubyte Valid)
{
  char MenuEntry[16+MAXNAMELEN], VolName[MAXNAMELEN+1];

  lstrcpyn(VolName,(char*)VolumeName+1,VolumeName[0]+1);

  lstrcpyn(RDB_Partitions[RDB_Parts].VolumeName,VolName,
           sizeof(RDB_Partitions[RDB_Parts].VolumeName));

  lstrcpyn(RDB_Partitions[RDB_Parts].DeviceName,DeviceName,
           sizeof(RDB_Partitions[RDB_Parts].DeviceName));

  RDB_Partitions[RDB_Parts].FSType = FSType;
  RDB_Partitions[RDB_Parts].DeviceIndex = DeviceIndex;		// needed??
  RDB_Partitions[RDB_Parts].Offset = Offset;
  RDB_Partitions[RDB_Parts].RootBlockOffset = RootBlockOffset;
  RDB_Partitions[RDB_Parts].Sectors = Sectors;
  RDB_Partitions[RDB_Parts].Media = Media;
  RDB_Partitions[RDB_Parts].Valid = Valid;
  RDB_Partitions[RDB_Parts].Reserved = Reserved;
  
  wsprintf(MenuEntry,"%s: (%c%c%c\\x%02x)",RDB_Partitions[RDB_Parts].VolumeName,
           FSType>>24,(FSType>>16)&0xff,(FSType>>8)&0xff,FSType&0xff);

  SendDlgItemMessage(hwnd,IDC_RDB_Partition,CB_ADDSTRING,0,(LPARAM)MenuEntry);
  RDB_Parts++;
 }

int SelectPartition(HWND hwnd,HANDLE Media,char *DeviceName,int Index)
{
  ubyte Buffer[LOGICAL_BLOCK_SIZE];
  ulong Checksum, NextLink, BlocksPerCyl, FileSystem, RootBlockOffset;
  uword Valid;
  i64 Offset,Sectors;

  struct BootBlock        *BB   = (struct BootBlock *) Buffer;
  struct bRootBlock       *RB   = (struct bRootBlock *) Buffer;
  struct RigidDiskBlock   *RDB  = (struct RigidDiskBlock *) Buffer;
  struct myPartitionBlock *Part = (struct myPartitionBlock *) Buffer;

  RDB_Parts=0;

  if(DriveDB[Index].MediaType==DRIVE_REMOVABLE)
  {
    ReadSector(Media,Buffer,LOGICAL_BLOCK_SIZE,BOOTBLOCK);
    BlockSwapEndian(Buffer,FS_BootBlock);
    FileSystem = *((long*)BB->bb_id);
    RootBlockOffset = BB->bb_dosblock;
    Sectors=RootBlockOffset*2;

    if(((FileSystem & 0xffffff00)==BBNAME_DOS)&&
         ReadSector(Media,Buffer,LOGICAL_BLOCK_SIZE,RootBlockOffset))
    { BlockSwapEndian(Buffer,FS_RootBlock);
      Valid = (uword) BitmapValid(Media,RootBlockOffset);

      if(!Valid)
        CondDebug(RPT_ERR,"Need to validate disk.");

      AddPartMenu(hwnd,Index,DeviceName,(BPCLSTR)&RB->nameLen,FileSystem,0,
                  RootBlockOffset,Sectors,2,DRIVE_REMOVABLE,Valid);
                  
      SendDlgItemMessage(hwnd,IDC_RDB_Partition,CB_SETCURSEL,0,0);
      return(TRUE);
     }
    return(FALSE);
   }

  ReadSector(Media,Buffer,LOGICAL_BLOCK_SIZE,DriveDB[Index].Offset+RDB_OFFSET);
  Checksum=ChecksumLong(Buffer,LittleEndian(RDB->rdb_SummedLongs));
  BlockSwapEndian(Buffer,FS_RDBBlock);

    // Visit "RDSK" Regid Disk Block
  if(RDB->rdb_ID == IDNAME_RIGIDDISK)
  {
    if(Checksum)
      CondDebug(RPT_ERR,"RDB Checksum:0x%08lx != 0",Checksum);

    CondDebug(RPT_INFO,"%s Geometry: %d Cyl %d Heads %d Sec",
              (DriveDB[Index].MediaType==DRIVE_HARDFILE)?DriveDB[Index].DeviceName:&DriveDB[Index].DeviceName[4],
              RDB->rdb_Cylinders,RDB->rdb_Heads,RDB->rdb_Sectors);

    CondDebug(RPT_INFO,"MBR Partition %d Offset:%s RDB Partition Cyl:[%d-%d] (%d Sec/Cyl)",
              DriveDB[Index].PartitonNumber,i64toa(DriveDB[Index].Offset+RDB_OFFSET),
              RDB->rdb_LoCylinder,RDB->rdb_HiCylinder,RDB->rdb_CylBlocks);

    //      Debug("BadBlocks block at Start + %d",RDB->rdb_BadBlockList);
    //      Debug("FSBlock block at Start + %d",RDB->rdb_FileSysHeaderList);

    NextLink=RDB->rdb_PartitionList;

    do  // Visit "PART" block
    { ReadSector(Media,Buffer,LOGICAL_BLOCK_SIZE,DriveDB[Index].Offset+NextLink);
      Checksum=ChecksumLong(Buffer,LittleEndian(RDB->rdb_SummedLongs));
      BlockSwapEndian(Buffer,FS_PartBlock);

    if(Checksum)
      CondDebug(RPT_ERR,"RDB Checksum:0x%08lx != 0",Checksum);

      FileSystem = *((long *)Part->pb_DosType);

      if((FileSystem & 0xffffff00)==BBNAME_DOS)
      {
        BlocksPerCyl = Part->pb_Surfaces*Part->pb_BlocksPerTrack;
        Offset = Part->pb_LowCyl*BlocksPerCyl+DriveDB[Index].Offset;
        Sectors = (Part->pb_HighCyl-Part->pb_LowCyl+1)*BlocksPerCyl;

        RootBlockOffset = (ulong) (Sectors-Part->pb_DOS_Reserved)/2 +1;

        Valid = (uword) BitmapValid(Media,RootBlockOffset+Offset);

        if(!Valid)
          CondDebug(RPT_ERR,"Need to validate partition.");

        AddPartMenu(hwnd,Index,DeviceName,(BPCLSTR)Part->pb_DriveName,FileSystem,
                    Offset,RootBlockOffset,Sectors,Part->pb_DOS_Reserved,
                    DriveDB[Index].MediaType,Valid);
       }
      else
      {
        CondDebug(RPT_INFO,"File system %08x not supported.",FileSystem);
       }
      //  Debug("Partition block at Start + %d",NextLink);
      //  HexDump(Buffer,LOGICAL_BLOCK_SIZE);

      NextLink = Part->pb_Next;
      } while(NextLink!=0xffffffff);
   }

  if(RDB_Parts>=1)
   SendDlgItemMessage(hwnd,IDC_RDB_Partition,CB_SETCURSEL,0,0);

  return(TRUE);
 }

void SelectAROS(HWND hwnd)
{ int n, result,partition;
  HANDLE Media;
  char *Hardfile;

  n=SendDlgItemMessage(hwnd,IDC_AROS_Device,CB_GETCURSEL,0,0);
  SendDlgItemMessage(hwnd,IDC_RDB_Partition,CB_RESETCONTENT,0,0);

  if(n!=CB_ERR)
  {
    if ((DriveDB[n].MediaType==DRIVE_REMOVABLE)||
       ((DriveDB[n].MediaType==0)&&(DriveDB[n].PartitionType==AROS_PARTITION)))
    {
      Media=OpenDrive(DriveDB[n].DeviceName);
      result=SelectPartition(hwnd,Media,DriveDB[n].DeviceName,n);
      CloseDrive(Media);

      if(result)
        return;
     }

    Hardfile = OpenFileGUI(hwnd,&DriveDB[n].DeviceName[4],"Open Hard File",NULL);

    if(Hardfile)
    {
      partition=PartitionInfo(Hardfile,0,DRIVE_HARDFILE,&DriveDB[Drives]);

      if(partition)
      { Media=OpenDrive(Hardfile);
        SelectPartition(hwnd,Media,DriveDB[Drives].DeviceName,Drives);
        CloseDrive(Media);
       } 
     }
   }
 }
