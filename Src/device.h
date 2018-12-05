// Device dependent calls
HANDLE OpenDrive(char *DeviceName);
void CloseDrive(HANDLE DevHandle);
long WriteSector(HANDLE Handle, ubyte *Buffer, long Size,i64 Offset);
long ReadSector(HANDLE Handle, ubyte *Buffer, long Size,i64 Offset);

