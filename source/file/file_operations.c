#include "common.h"
#include "dirlist.h"
#include "file_operations.h"
#include "fs.h"
#include "keyboard.h"
#include "main.h"
#include "utils.h"

/*
*	Copy Mode
*	-1 : Nothing
*	0  : Copy
*	1  : Move
*/
int copymode = NOTHING_TO_COPY;

void newFolder()
{
	char tempFolder[256];
	strcpy(tempFolder, keyboard_3ds_get(256, "", "Enter name"));
	
	if (strncmp(tempFolder, "", 1) == 0)
		mainMenu(KEEP);
	
	char path[500];
	strcpy(path, cwd);
	
	strcat(path, tempFolder);
	
	makeDir(fsArchive, path);
	
	mainMenu(CLEAR);	
}

int renameFile()
{
	File * file = findindex(position);

	if(file == NULL) 
		return -1;

	if(strncmp(file->name, "..", 2) == 0) 
		return -2;

	char oldPath[500], newPath[500], name[255];

	strcpy(oldPath, cwd);
	strcpy(newPath, cwd);
	strcat(oldPath, file->name);
	
	strcpy(name, keyboard_3ds_get(255, file->name, "Enter name"));
	strcat(newPath, name);
	
	fsRename(fsArchive, oldPath, newPath);
	
	mainMenu(CLEAR);
	
	return 0;
}

int delete_folder_recursive(char * path)
{
	// Internal file list
	File * filelist = NULL;

	// Open working directory
	Handle dirHandle;
	Result directory = FSUSER_OpenDirectory(&dirHandle, fsArchive, fsMakePath(PATH_ASCII, path));
	
	u32 entriesRead;
	static char dname[1024];

	// Opened directory
	if(!(directory))
	{
		do
		{
			static FS_DirectoryEntry info;
			memset(&info, 0, sizeof(FS_DirectoryEntry));
			
			entriesRead = 0;
			FSDIR_Read(dirHandle, &entriesRead, 1, &info);
			
			if(entriesRead)
			{
				utf2ascii(&dname[0], info.name);
				
				// Valid filename
				if(strlen(dname) > 0)
				{
					if((strncmp(dname, ".", 1) == 0) || (strncmp(dname, "..", 2) == 0))
						continue;

					// Allocate memory
					File * item = (File *)malloc(sizeof(File));
					
					// Clear memory
					memset(item, 0, sizeof(File));

					// Copy file name
					strcpy(item->name, dname);

					// Set folder flag
					item->isDir = info.attributes & FS_ATTRIBUTE_DIRECTORY;

					// New list
					if(filelist == NULL) 
						filelist = item;

					// Existing list
					else
					{
						// Iterator variable
						File * list = filelist;

						// Append to list
						while(list->next != NULL) 
							list = list->next;

						// Link item
						list->next = item;
					}
				}
			}
		} 
		while(entriesRead);
	}

	// Close directory
	FSDIR_Close(dirHandle);
	
	// List node
	File * node = filelist;

	// Iterate files
	for(; node != NULL; node = node->next)
	{
		// Directory
		if(node->isDir)
		{
			// Required buffer size
			int size = strlen(path) + strlen(node->name) + 2;

			// Allocate buffer
			char * buffer = (char *)malloc(size);

			// Combine path
			strcpy(buffer, path);
			strcpy(buffer + strlen(buffer), node->name);
			buffer[strlen(buffer) + 1] = 0;
			buffer[strlen(buffer)] = '/';

			// Recursion delete
			delete_folder_recursive(buffer);
			
			// Free memory
			free(buffer);
		}

		// File
		else
		{
			// Required buffer size
			int size = strlen(path) + strlen(node->name) + 1;

			// Allocate buffer
			char * buffer = (char *)malloc(size);

			// Combine path
			strcpy(buffer, path);
			strcpy(buffer + strlen(buffer), node->name);

			// Delete file
			fsRemove(fsArchive, buffer);

			// Free memory
			free(buffer);
		}
	}

	// Free temporary list
	recursiveFree(filelist);

	// Delete now empty folder
	return fsRmdir(fsArchive, path);
}

int delete(void)
{
	// Find file
	File * file = findindex(position);

	// Not found
	if(file == NULL) 
		return -1;

	if (sysProtection)
	{
		if((strncmp(file->name, "..", 2) == 0) || (SYS_FILES)) 
			return -2;
	}
	else
	{
		if(strncmp(file->name, "..", 2) == 0) 
			return -2;
	}

	// File path
	char path[1024];

	// Puzzle path
	strcpy(path, cwd);
	strcpy(path + strlen(path), file->name);

	// Delete folder
	if (file->isDir)
	{
		// Add trailing slash
		path[strlen(path) + 1] = 0;
		path[strlen(path)] = '/';

		// Delete folder
		return delete_folder_recursive(path);
	}

	// Delete file
	else 
		return fsRemove(fsArchive, path);
}

// Copy file or folder
void copy(int flag)
{
	// Find file
	File * file = findindex(position);

	// Not found
	if(file == NULL) 
		return;

	// Copy file source
	strcpy(copysource, cwd);
	strcpy(copysource + strlen(copysource), file->name);

	// Add recursive folder flag
	if ((file->isDir) && (strncmp(file->name, "..", 2) != 0))
		flag |= COPY_FOLDER_RECURSIVE;

	// Set copy flags
	copymode = flag;
}

// Copy file from A to B
int copy_file(char * a, char * b)
{
	// Chunk size
	int chunksize = 512 * 1024;

	// Reading buffer
	char * buffer = (char *)malloc(chunksize);

	// Accumulated writing
	int totalwrite = 0;

	// Accumulated reading
	int totalread = 0;

	// Result
	int result = 0;

	// Open file for reading
	int in = open(a, O_RDONLY, 0777);

	// Opened file for reading
	if(in >= 0)
	{
		// Delete output file (if existing)
		fsRemove(fsArchive, b);

		// Open file for writing
		int out = open(b, O_WRONLY | O_CREAT | O_TRUNC, 0777);

		// Opened file for writing
		if(out >= 0)
		{
			// Read byte count
			int b_read = 0;

			// Copy loop (512KB at a time)
			while((b_read = read(in, buffer, chunksize)) > 0)
			{
				// Accumulate read data
				totalread += b_read;

				// Write data
				totalwrite += write(out, buffer, b_read);
			}

			// Close output file
			close(out);

			// Insufficient copy
			if(totalread != totalwrite) 
				result = -3;
		}

		// Output open error
		else 
			result = -2;

		// Close input file
		close(in);
	}

	// Input open error
	else 
		result = -1;

	// Free memory
	free(buffer);

	// Return result
	return result;
}

// Copy folder from A to B
int copy_folder_recursive(char * a, char * b)
{
	// Open working Directory
	Handle dirHandle;
	Result directory = FSUSER_OpenDirectory(&dirHandle, fsArchive, fsMakePath(PATH_ASCII, a));
	
	u32 entriesRead;
	static char dname[1024];

	// Opened directory
	if(!(directory))
	{
		// Create output directory (is allowed to fail, we can merge folders after all)
		makeDir(fsArchive, b);

		// Iterate files
		do
		{
			static FS_DirectoryEntry info;
			memset(&info, 0, sizeof(FS_DirectoryEntry));
			
			entriesRead = 0;
			FSDIR_Read(dirHandle, &entriesRead, 1, &info);
			
			if(entriesRead)
			{
				utf2ascii(&dname[0], info.name);
				
				// Valid filename
				if(strlen(dname) > 0)
				{
					// Calculate buffer size
					int insize = strlen(a) + strlen(dname) + 2;
					int outsize = strlen(b) + strlen(dname) + 2;

					// Allocate buffer
					char * inbuffer = (char *)malloc(insize);
					char * outbuffer = (char *)malloc(outsize);

					// Puzzle input path
					strcpy(inbuffer, a);
					inbuffer[strlen(inbuffer) + 1] = 0;
					inbuffer[strlen(inbuffer)] = '/';
					strcpy(inbuffer + strlen(inbuffer), dname);

					// Puzzle output path
					strcpy(outbuffer, b);
					outbuffer[strlen(outbuffer) + 1] = 0;
					outbuffer[strlen(outbuffer)] = '/';
					strcpy(outbuffer + strlen(outbuffer), dname);

					// Another folder
					if(info.attributes & FS_ATTRIBUTE_DIRECTORY)
						copy_folder_recursive(inbuffer, outbuffer); // Copy folder (via recursion)

					// Simple file
					else
						copy_file(inbuffer, outbuffer); // Copy file

					// Free buffer
					free(inbuffer);
					free(outbuffer);
				}
			}
		} 
		while(entriesRead);

		// Close directory
		FSDIR_Close(dirHandle);

		// Return success
		return 0;
	}
	
	// Open error
	else 
		return -1;
}

// Paste file or folder
int paste(void)
{
	// No copy source
	if(copymode == NOTHING_TO_COPY) 
		return -1;

	// Source and target folder are identical
	char * lastslash = NULL; 
	int i = 0; 
	
	for(; i < strlen(copysource); i++) 
		if(copysource[i] == '/') 
			lastslash = copysource + i;
		
	char backup = lastslash[1];
	lastslash[1] = 0;
	int identical = strcmp(copysource, cwd) == 0;
	lastslash[1] = backup;
	
	if(identical) 
		return -2;

	// Source filename
	char * filename = lastslash + 1;

	// Required target path buffer size
	int requiredlength = strlen(cwd) + strlen(filename) + 1;

	// Allocate target path buffer
	char * copytarget = (char *)malloc(requiredlength);

	// Puzzle target path
	strcpy(copytarget, cwd);
	strcpy(copytarget + strlen(copytarget), filename);

	// Return result
	int result = -3;

	// Recursive folder copy
	if((copymode & COPY_FOLDER_RECURSIVE) == COPY_FOLDER_RECURSIVE)
	{
		// Check files in current folder
		File * node = files; for(; node != NULL; node = node->next)
		{
			// Found a file matching the name (folder = ok, file = not)
			if(strcmp(filename, node->name) == 0 && !node->isDir)
				return -4; // Error out
		}

		// Copy folder recursively
		result = copy_folder_recursive(copysource, copytarget);

		// Source delete
		if(result == 0 && (copymode & COPY_DELETE_ON_FINISH) == COPY_DELETE_ON_FINISH)
		{
			// Append trailing slash (for recursion to work)
			copysource[strlen(copysource) + 1] = 0;
			copysource[strlen(copysource)] = '/';

			// Delete source
			delete_folder_recursive(copysource);
		}
	}

	// Simple file copy
	else
	{
		// Copy file
		result = copy_file(copysource, copytarget);

		// Source delete
		if(result == 0 && (copymode & COPY_DELETE_ON_FINISH) == COPY_DELETE_ON_FINISH)
			fsRemove(fsArchive, copysource); // Delete File
	}

	// Paste success
	if(result == 0)
	{
		// Erase cache data
		memset(copysource, 0, sizeof(copysource));
		copymode = NOTHING_TO_COPY;
	}

	// Free target path buffer
	free(copytarget);

	// Return result
	return result;
}

void saveLastDirectory()
{
	writeFile("/3ds/3DShell/lastdir.txt", cwd);
}