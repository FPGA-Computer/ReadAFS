#define DIRLEVEL_UP		-1
#define DIRLEVEL_DOWN	1

extern ulong CurAFS_Root;

int AFS_NewPath(char *Path, char *Dir);
int AFS_CD(HWND hwnd, char *Path, char *Dir, ulong sector);
void DisplayPartInfo(struct Partition *Part);

