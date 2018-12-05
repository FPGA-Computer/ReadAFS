// File operatins
#define FileOP_Copy			0x0001
#define FileOP_Delete		0x0002
#define FileOP_Move			(FileOP_Copy|FileOP_Delete)
#define FileOP_Rename		0x0004
#define FileOP_MkDir			0x0008

// Dir operations
#define FileOP_List     	0x0100
#define FileOP_Parent		0x0200
#define FileOP_Recurse		0x0800

#define FileOP_Move			(FileOP_Copy|FileOP_Delete)

// Disk Operation
#define FileOP_Validate		0x4000
#define FileOP_ExportHF		0x5000

extern DWORD FileOPThreadID;
DWORD WINAPI FileOPTask( LPVOID WHandle );

int  FileOperations(HWND hwnd,int Operation);

