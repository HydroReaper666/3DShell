#ifndef FS_H
#define FS_H

#include <3ds.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

FS_Archive fsArchive;

int BROWSE_STATE;

#define STATE_SD 0
#define STATE_NAND 1

typedef struct 
{
    Handle handle;
    u64 offset;
	u64 size;
	unsigned int error;
} FSFILE;

void openArchive(FS_ArchiveID id);
void closeArchive(void);
Result makeDir(FS_Archive archive, const char * path);
bool fileExists(FS_Archive archive, const char * path);
bool dirExists(FS_Archive archive, const char * path);
char* getFileModifiedTime(char * path);
u64 getFileSize(FS_Archive archive, const char * path);
Result fsRemove(FS_Archive archive, const char * filename);
Result fsRmdir(FS_Archive archive, const char * path);
Result fsRmdirRecursive(FS_Archive archive, const char * path);
Result fsRename(FS_Archive archive, const char * old_filename, const char * new_filename);
Result fsOpen(Handle * handle, const char * path, u32 flags);
Result fsClose(Handle filehandle);
Result writeFile(const char * path, const char * buf);

#endif