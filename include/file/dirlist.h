#ifndef DIRLIST_H
#define DIRLIST_H

#include <3ds.h>

#define CLEAR 1
#define KEEP  0

#define FILES_PER_PAGE 5
#define MAX_FILES      1024

/*
*	File struct
*/
typedef struct File
{
	struct File * next; // Next item
	int isDir; // Folder flag
	int isReadOnly; // Read-only flag
	int isHidden; // Hidden file flag
	u8 name[256]; // File name
	char ext[4]; // File extension
	u64 size; // File size
} File;

/*
*	Menu position
*/
extern int position;

/*
*	Number of files
*/
extern int fileCount;

/*
*	File list
*/
extern File * files;

/*
*	Generic definitions
*/
int selectionX, selectionY, DEFAULT_STATE;

/*
*	Auto-hide scroll bar
*/
//u64 scroll_time;
//int scroll_x;

#define STATE_HOME     0
#define STATE_OPTIONS  1
#define STATE_SETTINGS 2
#define STATE_UPDATE   3
#define STATE_FTP      4
#define STATE_THEME    5
#define STATE_SORT     6

#define IF_OPTIONS	((DEFAULT_STATE != STATE_HOME) && (DEFAULT_STATE != STATE_UPDATE) && (DEFAULT_STATE != STATE_SETTINGS) && \
					(DEFAULT_STATE != STATE_SORT) && (DEFAULT_STATE != STATE_FTP) && (DEFAULT_STATE != STATE_THEME))
					
#define IF_SETTINGS ((DEFAULT_STATE != STATE_HOME) && (DEFAULT_STATE != STATE_UPDATE) && (DEFAULT_STATE != STATE_OPTIONS) && \
					(DEFAULT_STATE != STATE_FTP) && (DEFAULT_STATE != STATE_THEME) && (DEFAULT_STATE != STATE_SORT))

#define IF_SORT     ((DEFAULT_STATE != STATE_HOME) && (DEFAULT_STATE != STATE_UPDATE) && (DEFAULT_STATE != STATE_OPTIONS) && \
					(DEFAULT_STATE != STATE_FTP) && (DEFAULT_STATE != STATE_THEME) && (DEFAULT_STATE != STATE_SETTINGS))

#define IF_THEME 	(DEFAULT_STATE == STATE_THEME)
					
#define CAN_COPY 	(((copyF == false) && (cutF != true) && (deleteDialog == false)))
#define CAN_CUT 	(((cutF == false) && (copyF != true) && (deleteDialog == false)))

#define SYS_FILES 	((strcmp(file->name, "boot.firm") == 0) || (strcmp(file->name, "boot9strap") == 0) || \
					(strcmp(getLastNChars(file->name, 12), "Nintendo 3DS") == 0) || (strcmp(file->name, "arm9loaderhax.bin") == 0) || \
					(strncmp(cwd, "/Nintendo 3DS/", 14) == 0) || (strncmp(cwd, "/boot9strap/", 12) == 0))
					
#define resetSelection() selectionX = -1, selectionY = -1;

Result updateList(int clearindex);
void displayFiles(void);
void recursiveFree(File * node);
void openFile(void);
int navigate(int _case);
File * getFileIndex(int index);
int drawDeletionDialog(void);
int displayProperties(void);

#endif