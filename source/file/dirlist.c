#include "cia.h"
#include "clock.h"
#include "common.h"
#include "compile_date.h"
#include "dirlist.h"
#include "file_operations.h"
#include "fs.h"
#include "ftp.h"
#include "gallery.h"
#include "language.h"
#include "main.h"
#include "mcu.h"
#include "net.h"
#include "power.h"
#include "screenshot.h"
#include "sound.h"
#include "text.h"
#include "theme.h"
#include "updater.h"
#include "utils.h"
#include "wifi.h"

struct colour Storage_colour;
struct colour TopScreen_colour;
struct colour TopScreen_min_colour;
struct colour TopScreen_bar_colour;
struct colour BottomScreen_colour;
struct colour BottomScreen_bar_colour;
struct colour BottomScreen_text_colour;
struct colour Options_select_colour;
struct colour Options_text_colour;
struct colour Options_title_text_colour;
struct colour Settings_colour;
struct colour Settings_title_text_colour;
struct colour Settings_text_colour;
struct colour Settings_text_min_colour;

/*
*	Menu position
*/
int position = 0;

/*
*	Number of files
*/
int fileCount = 0;

/*
*	File list
*/
File * files = NULL;

void updateList(int clearindex)
{
	recursiveFree(files);
	files = NULL;
	fileCount = 0;
	
	Handle dirHandle;
	Result directory = FSUSER_OpenDirectory(&dirHandle, sdmcArchive, fsMakePath(PATH_ASCII, cwd));
	
	u32 entriesRead;
	static char dname[1024];

	if(!(directory))
	{
		/* Add fake ".." entry except on root */
		if (strcmp(cwd, ROOT_PATH)) 
		{
			// New List
			files = (File *)malloc(sizeof(File));

			// Clear Memory
			memset(files, 0, sizeof(File));

			// Copy File Name
			strcpy(files->name, "..");

			// Set Folder Flag
			files->isFolder = 1;

			fileCount++;
		}

		do
		{
			static FS_DirectoryEntry info;
			memset(&info, 0, sizeof(FS_DirectoryEntry));
			
			entriesRead = 0;
			FSDIR_Read(dirHandle, &entriesRead, 1, &info);
			
			if(entriesRead)
			{
				utf2ascii(&dname[0], info.name);
				
				// Ingore null filename
				if(dname[0] == '\0') 
					continue;

				// Ignore "." in all Directories
				if(strcmp(dname, ".") == 0) 
					continue;

				// Ignore ".." in Root Directory
				if(strcmp(cwd, ROOT_PATH) == 0 && strcmp(dname, "..") == 0) 
					continue;

				// Allocate Memory
				File * item = (File *)malloc(sizeof(File));

				// Clear Memory
				memset(item, 0, sizeof(File));

				// Copy File Name
				strcpy(item->name, dname);

				// Set Folder Flag
				item->isFolder = info.attributes & FS_ATTRIBUTE_DIRECTORY;
				
				// Set Read-Only Flag
				item->isRDONLY = info.attributes & FS_ATTRIBUTE_READ_ONLY; 
				
				// Copy File Name
				strcpy(item->ext, info.shortExt);

				// New List
				if(files == NULL) 
					files = item;

				// Existing List
				else
				{
					// Iterator Variable
					File * list = files;

					// Append to List
					while(list->next != NULL) list = list->next;

					// Link Item
					list->next = item;
				}

				// Increase File Count
				fileCount++;
			}
		} 
		while(entriesRead);

		// Close Directory
		FSDIR_Close(dirHandle);
	}

	// Attempt to keep Index
	if(!clearindex)
	{
		// Fix Position
		if(position >= fileCount) position = fileCount - 1;
	}

	// Reset Position
	else position = 0;
}

void recursiveFree(File * node)
{
	// End of List
	if(node == NULL)
		return;

	// Nest Further
	recursiveFree(node->next);

	// Free Memory
	free(node);
}

void displayFiles()
{
	// Bottom screen options
	sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
		
	sf2d_draw_rectangle(0, 0, 320, 240, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255));
	sf2d_draw_rectangle(0, 0, 320, 20, RGBA8(BottomScreen_bar_colour.r, BottomScreen_bar_colour.g, BottomScreen_bar_colour.b, 255));
	
	if (DEFAULT_STATE == STATE_HOME)
	{
		sf2d_draw_texture(s_HomeIcon, -2, -2);
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, welcomeMsg)) / 2), 40, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, welcomeMsg);
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, currDate)) / 2), 60, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, currDate);
		sftd_draw_textf(font, 2, 225, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, "3DShell v%i.%i Beta - %d%02d%02d", VERSION_MAJOR, VERSION_MINOR, YEAR, MONTH, DAY);
	}
	else
		sf2d_draw_texture(homeIcon, -2, -2);
	
	if (DEFAULT_STATE == STATE_SETTINGS)
	{
		sf2d_draw_texture(s_SettingsIcon, 50, 1);
		sf2d_draw_rectangle(0, 20, 320, 220, RGBA8(Settings_colour.r, Settings_colour.g, Settings_colour.b, 255));
		
		sftd_draw_text(font, 10, 30, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), 11, lang_settings[language][0]);
		
		sftd_draw_text(font, 10, 50, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), 11, lang_settings[language][5]); // Grey'd out - cannot be accessed yet.
		sftd_draw_textf(font2, 10, 62, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), 10, "%s (/3ds/3DShell/bgm.ogg)", lang_settings[language][6]);
		
		sftd_draw_text(font, 10, 90, RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), 11, lang_settings[language][1]);
		sftd_draw_text(font2, 10, 102, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), 10, lang_settings[language][2]);
		
		sftd_draw_text(font, 10, 130, RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), 11, lang_settings[language][3]);
		sftd_draw_textf(font2, 10, 142, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), 10, "%s %s", lang_settings[language][4], theme_dir);
		
		if (bgmEnable)
			sf2d_draw_texture(toggleOn, 280, 50);
		else 
			sf2d_draw_texture(toggleOff, 280, 50);
		
		if (sysProtection)
			sf2d_draw_texture(toggleOn, 280, 90);
		else 
			sf2d_draw_texture(toggleOff, 280, 90);
		
		sf2d_draw_texture(themeIcon, 283, 125);
	}
	else
		sf2d_draw_texture(settingsIcon, 50, 1);
	
	if (DEFAULT_STATE == STATE_UPDATE)
	{
		sf2d_draw_texture(s_UpdateIcon, 75, 0);
		sftd_draw_text(font, ((320 - (sftd_get_text_width(font, 11, lang_update[language][0]))) / 2), 40, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, lang_update[language][0]);
		
		Result ret = installCIA("/3ds/3DShell/3DShell.cia", MEDIATYPE_SD, true);
		if (ret != 0)
			sftd_draw_text(font, ((320 - (sftd_get_text_width(font, 11, lang_update[language][1]))) / 2), 60, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, lang_update[language][1]);
		else
			sftd_draw_text(font, ((320 - (sftd_get_text_width(font, 11, lang_update[language][2]))) / 2), 60, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, lang_update[language][2]);
		
		sftd_draw_text(font, ((320 - (sftd_get_text_width(font, 11, lang_update[language][3]))) / 2), 80, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, lang_update[language][3]);
		
		wait(300000000);
		longjmp(exitJmp, 1);
	}
	else
		sf2d_draw_texture(updateIcon, 75, 0);
	
	sf2d_draw_texture(ftpIcon, 100, 0);
	
	if (DEFAULT_STATE == STATE_THEME)
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, lang_themes[language][0])) / 2), 40, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, lang_themes[language][0]);
	
	if (DEFAULT_STATE == STATE_DOWNLOAD)
	{
		sf2d_draw_texture(s_dlIcon, 125, 0);
		
		sf2d_draw_rectangle(0, 20, 320, 220, RGBA8(Settings_colour.r, Settings_colour.g, Settings_colour.b, 255));
		
		sftd_draw_textf(font, 10, 40, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), 11, "Enter URL: %s", dl_url);	
	
		sftd_draw_textf(font, 10, 60, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), 11, "%lu", dl_size);	
	}
	else
		sf2d_draw_texture(dlIcon, 125, 0);
	
	if (BROWSE_STATE == STATE_SD)
		sf2d_draw_texture(s_SdIcon, 150, 0);
	else
		sf2d_draw_texture(sdIcon, 150, 0);
	
	if (BROWSE_STATE == STATE_NAND)
		sf2d_draw_texture(s_NandIcon, 175, 0);
	else
		sf2d_draw_texture(nandIcon, 175, 0);
	
	sf2d_draw_texture(searchIcon, (320 - searchIcon->width), -2);
	
	if (DEFAULT_STATE == STATE_OPTIONS)
	{
		sf2d_draw_texture(s_OptionsIcon, 25, 0);
		
		sf2d_draw_texture(options, 37, 20);
	
		sf2d_draw_rectangle(37 + (selectionX * 123), 56 + (selectionY * 37), 123, 37, RGBA8(Options_select_colour.r, Options_select_colour.g, Options_select_colour.b, 255));
			
		sftd_draw_text(font, 42, 36, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), 11, lang_options[language][0]);
		sftd_draw_text(font2, 232, 196, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), 10, lang_options[language][8]);
		
		sftd_draw_text(font, 47, 72, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 11, lang_options[language][1]);
		sftd_draw_text(font, 47, 109,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 11, lang_options[language][3]);
		sftd_draw_text(font, 47, 146,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 11, lang_options[language][5]);
		
		sftd_draw_text(font, 170, 72,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 11, lang_options[language][2]);
	
		if(copyF == false)
			sftd_draw_text(font, 170, 109,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 11, lang_options[language][4]);
		else
			sftd_draw_text(font, 170, 109,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 11, lang_options[language][7]);
	
		if(cutF == false)
			sftd_draw_text(font, 170, 146,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 11, lang_options[language][6]);
		else
			sftd_draw_text(font, 170, 146,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 11, lang_options[language][7]);
	}
	else
		sf2d_draw_texture(optionsIcon, 25, 0);
		
	sf2d_end_frame();
	// Ends here
	
	sf2d_start_frame(GFX_TOP, GFX_LEFT);
        		
	sf2d_draw_texture(background, 0, 0);
	
	sftd_draw_textf(font, 84, 28, RGBA8(TopScreen_bar_colour.r, TopScreen_bar_colour.g, TopScreen_bar_colour.b, 255), 11, "%.35s", cwd); // Display current path
	
	drawWifiStatus(270, 2);
	drawBatteryStatus(295, 2);
	digitalTime(346, 1);
	
	FS_ArchiveResource	resource = {0};
	
	if (BROWSE_STATE == STATE_SD)
		FSUSER_GetArchiveResource(&resource, SYSTEM_MEDIATYPE_SD);
	else 
		FSUSER_GetArchiveResource(&resource, SYSTEM_MEDIATYPE_CTR_NAND);
	
	double totalStorage = (((u64) resource.totalClusters * (u64) resource.clusterSize) / 1024.0 / 1024.0);
	double usedStorage = (totalStorage - (((u64) resource.freeClusters * (u64) resource.clusterSize) / 1024.0 / 1024.0));
	double fill = ((usedStorage / totalStorage) * 208.0);
	
	sf2d_draw_rectangle(83, 47, fill, 2, RGBA8(Storage_colour.r, Storage_colour.g, Storage_colour.b, 255)); // Draw storage bar

	// File Iterator Variable
	int i = 0;

	// Print Counter
	int printed = 0;

	// Draw File List
	File * file = files;
	
	for(; file != NULL; file = file->next)
	{
		// Limit the files per page
		if(printed == FILES_PER_PAGE) 
			break;

		if(position < FILES_PER_PAGE || i > (position - FILES_PER_PAGE))
		{
			// Draw selector
			if(i == position)
				sf2d_draw_texture(selector, 0, 53 + (38 * printed));
			
			char path[500];
			strcpy(path, cwd);
			strcpy(path + strlen(path), file->name);

			sf2d_draw_texture(uncheck, 8, 66 + (38 * printed));

			if (file->isFolder)
				sf2d_draw_texture(folderIcon, 30, 58 + (38 * printed));
			else if ((strncmp(file->ext, "mp3", 3) == 0) || (strncmp(file->ext, "MP3", 3) == 0))
				sf2d_draw_texture(audioIcon, 30, 58 + (38 * printed));
			else if ((strncmp(file->ext, "txt", 3) == 0) || (strncmp(file->ext, "TXT", 3) == 0))
				sf2d_draw_texture(txtIcon, 30, 58 + (38 * printed));
			else if ((strncmp(file->ext, "bin", 3) == 0) || (strncmp(file->ext, "BIN", 3) == 0) || 
					(strncmp(file->ext, "fir", 3) == 0) || (strncmp(file->ext, "FIR", 3) == 0))
				sf2d_draw_texture(systemIcon, 30, 58 + (38 * printed));
			else if ((strncmp(file->ext, "rar", 3) == 0) || (strncmp(file->ext, "RAR", 3) == 0) || 
					(strncmp(file->ext, "zip", 3) == 0) || (strncmp(file->ext, "ZIP", 3) == 0))
				sf2d_draw_texture(zipIcon, 30, 58 + (38 * printed));
			else if ((strncmp(file->ext, "jpg", 3) == 0) || (strncmp(file->ext, "JPG", 3) == 0) || 
					(strncmp(file->ext, "png", 3) == 0) || (strncmp(file->ext, "PNG", 3) == 0) || 
					(strncmp(file->ext, "gif", 3) == 0) || (strncmp(file->ext, "GIF", 3) == 0))
				sf2d_draw_texture(imgIcon, 30, 58 + (38 * printed));
			else if ((strncmp(file->ext, "3ds", 3) == 0) || (strncmp(file->ext, "3DS", 3) == 0) || 
					(strncmp(file->ext, "cia", 3) == 0) || (strncmp(file->ext, "CIA", 3) == 0))
				sf2d_draw_texture(appIcon, 30, 58 + (38 * printed));
			else
				sf2d_draw_texture(fileIcon, 30, 58 + (38 * printed));
			
			char buf[64], size[16];

			strncpy(buf, file->name, sizeof(buf));
			buf[sizeof(buf) - 1] = '\0';
			int len = strlen(buf);
			len = 40 - len;
			
			while(len -- > 0)
				strcat(buf, " ");
			
			sftd_draw_textf(font, 70, 60 + (38 * printed), RGBA8(TopScreen_colour.r ,TopScreen_colour.g, TopScreen_colour.b, 255), 11, "%.52s", buf); // Display file name
			
			if ((file->isFolder) && (strncmp(file->name, "..", 2) != 0))
				sftd_draw_textf(font2, 70, 75 + (38 * printed), RGBA8(TopScreen_min_colour.r, TopScreen_min_colour.g, TopScreen_min_colour.b, 255), 10, "%s drwxr-x---", getFileModifiedTime(path));
			else if (strncmp(file->name, "..", 2) == 0)
				sftd_draw_text(font2, 70, 75 + (38 * printed), RGBA8(TopScreen_min_colour.r, TopScreen_min_colour.g, TopScreen_min_colour.b, 255), 10, lang_files[language][0]);
			else
			{
				getSizeString(size, getFileSize(sdmcArchive, path));
				sftd_draw_textf(font2, 70, 75 + (38 * printed), RGBA8(TopScreen_min_colour.r, TopScreen_min_colour.g, TopScreen_min_colour.b, 255), 10, "%s -rw-rw----", getFileModifiedTime(path));
				sftd_draw_textf(font2, 395 - sftd_get_text_width(font2, 10, size), 75 + (38 * printed), RGBA8(TopScreen_colour.r, TopScreen_colour.g, TopScreen_colour.b, 255), 10, "%s", size);
			}
			
			printed++; // Increase printed counter
		}

		i++; // Increase Counter
	}
	
	endDrawing();
}

void openFile(void)
{
	File * file = findindex(position);

	if(file == NULL) 
		return;
	
	char path[1024];
	
	strcpy(fileName, file->name);
	
	strcpy(path, cwd);
	strcpy(path + strlen(path), file->name);
	
	if (file->isFolder)
	{
		// Attempt to navigate to Target
		if(navigate(0) == 0)
		{	
			saveLastDirectory();
			updateList(CLEAR);
			displayFiles(CLEAR);
		}
	}
	
	else if ((strncmp(file->ext, "png", 3) == 0) || (strncmp(file->ext, "PNG", 3) == 0))
		displayImage(path, 0);
	else if ((strncmp(file->ext, "jpg", 3) == 0) || (strncmp(file->ext, "JPG", 3) == 0))
		displayImage(path, 1);
	else if ((strncmp(file->ext, "gif", 3) == 0) || (strncmp(file->ext, "GIF", 3) == 0))
		displayImage(path, 2);
	else if ((strncmp(file->ext, "txt", 3) == 0) || (strncmp(file->ext, "TXT", 3) == 0))
		displayText(path);
	else if ((strncmp(file->ext, "cia", 3) == 0) || (strncmp(file->ext, "CIA", 3) == 0))
		displayCIA(path);
	else if ((strncmp(file->ext, "zip", 3) == 0) || (strncmp(file->ext, "ZIP", 3) == 0))
	{
		extractZip(path, cwd);
		updateList(CLEAR);
		displayFiles(CLEAR);
	}
}

// Navigate to Folder
int navigate(int _case)
{
	// Find File
	File * file = findindex(position);

	// Not a Folder
	if(file == NULL || !file->isFolder) 
		return -1;

	// Special Case ".."
	if((_case == -1) || (strncmp(file->name, "..", 2) == 0))
	{
		// Slash Pointer
		char * slash = NULL;

		// Find Last '/' in Working Directory
		int i = strlen(cwd) - 2; for(; i >= 0; i--)
		{
			// Slash discovered
			if(cwd[i] == '/')
			{
				// Save Pointer
				slash = cwd + i + 1;

				// Stop Search
				break;
			}
		}

		// Terminate Working Directory
		slash[0] = 0;
		saveLastDirectory();
	}

	// Normal Folder
	else
	{
		// Append Folder to Working Directory
		strcpy(cwd + strlen(cwd), file->name);
		cwd[strlen(cwd) + 1] = 0;
		cwd[strlen(cwd)] = '/';
		
		saveLastDirectory();
	}

	// Return Success
	return 0;
}

// Find File Information by Index
File * findindex(int index)
{
	// File Iterator Variable
	int i = 0;

	// Find File Item
	File * file = files; 
	
	for(; file != NULL && i != index; file = file->next) 
		i++;

	// Return File
	return file;
}

int drawDeletionDialog()
{	
	while(deleteDialog == true)
	{
		hidScanInput();
		hidTouchRead(&touch);
		
		sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
		
		sf2d_draw_rectangle(0, 0, 320, 240, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255));

		sf2d_draw_texture(deletion, 20, 55);
		
		sftd_draw_text(font, 27, 72, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), 11, lang_deletion[language][0]);
		
		sftd_draw_text(font2, 206, 159, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), 10, lang_deletion[language][3]);
		sftd_draw_text(font2, 255, 159, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), 10, lang_deletion[language][4]);
	
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, lang_deletion[language][1])) / 2), 100, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), 11, lang_deletion[language][1]);
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, lang_deletion[language][2])) / 2), 115, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), 11, lang_deletion[language][2]);
			
		endDrawing();
		
		if ((kHeld & KEY_L) && (kHeld & KEY_R))
			captureScreenshot();
			
		if ((kPressed & KEY_A) || (touchInRect(240, 320, 142, 185)))
		{
			if(delete() == 0)
			{
				updateList(CLEAR);
				displayFiles(CLEAR);
			}
			
			break;
		}
		else if ((kPressed & KEY_B) || (touchInRect(136, 239, 142, 185)))
			break;
	}
	
	deleteDialog = false;
	return 0;
}

int displayProperties()
{
	File * file = findindex(position);

	if(file == NULL) 
		return -1;
	
	char path[255], fullPath[500];
	
	strcpy(path, cwd);
	strcpy(fullPath, cwd);
	strcpy(fullPath + strlen(fullPath), file->name);
	
	char size[16];
	getSizeString(size, getFileSize(sdmcArchive, fullPath));
	
	while(properties == true)
	{
		hidScanInput();
		hidTouchRead(&touch);
		
		sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
		
		sf2d_draw_rectangle(0, 0, 320, 240, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255));
		
		sf2d_draw_texture(_properties, 36, 20);
		
		sftd_draw_text(font, 41, 33, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), 11, lang_properties[language][0]);
		
		sftd_draw_text(font2, 247, 201, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), 10, lang_properties[language][6]);
	
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, lang_properties[language][1])) / 2), 50, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), 11, lang_properties[language][1]);
		
		sftd_draw_text(font2, 42, 74, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), 10, lang_properties[language][2]);
			sftd_draw_textf(font2, 100, 74,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, "%.36s", fileName);
		sftd_draw_text(font2, 42, 94, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), 10, lang_properties[language][3]);
			sftd_draw_textf(font2, 100, 94, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, "%s", path);
			
		sftd_draw_text(font2, 42, 114, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), 10, lang_properties[language][4]);	
		
		if (file->isFolder)
			sftd_draw_text(font2, 100, 114,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, lang_files[language][1]);
		else if ((strncmp(file->ext, "CIA", 3) == 0) || (strncmp(file->ext, "cia", 3) == 0) || (strncmp(file->ext, "3DS", 3) == 0) || (strncmp(file->ext, "3ds", 3) == 0))
			sftd_draw_text(font2, 100, 114,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, lang_files[language][2]);
		else if ((strncmp(file->ext, "bin", 3) == 0) || (strncmp(file->ext, "BIN", 3) == 0) || (strncmp(file->ext, "fir", 3) == 0) || (strncmp(file->ext, "FIR", 3) == 0))
			sftd_draw_text(font2, 100, 114,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, lang_files[language][3]);
		else if ((strncmp(file->ext, "zip", 3) == 0) || (strncmp(file->ext, "ZIP", 3) == 0))
			sftd_draw_text(font2, 100, 114,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, lang_files[language][4]);
		else if ((strncmp(file->ext, "rar", 3) == 0) || (strncmp(file->ext, "RAR", 3) == 0))
			sftd_draw_text(font2, 100, 114,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, lang_files[language][5]);
		else if ((strncmp(file->ext, "PNG", 3) == 0) || (strncmp(file->ext, "png", 3) == 0))
			sftd_draw_text(font2, 100, 114,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, lang_files[language][6]);
		else if ((strncmp(file->ext, "JPG", 3) == 0) || (strncmp(file->ext, "jpg", 3) == 0))
			sftd_draw_text(font2, 100, 114,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, lang_files[language][7]);
		else if ((strncmp(file->ext, "MP3", 3) == 0) || (strncmp(file->ext, "mp3", 3) == 0))
			sftd_draw_text(font2, 100, 114,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, lang_files[language][8]);
		else if ((strncmp(file->ext, "txt", 3) == 0) || (strncmp(file->ext, "TXT", 3) == 0) || (strncmp(file->ext, "XML", 3) == 0) || (strncmp(file->ext, "xml", 3) == 0))
			sftd_draw_text(font2, 100, 114,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, lang_files[language][9]);
		else
			sftd_draw_text(font2, 100, 114,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, lang_files[language][10]);
			
		if (!(file->isFolder))
		{
			sftd_draw_text(font2, 42, 134,RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), 10, lang_properties[language][5]);
				sftd_draw_textf(font2, 100, 134,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, "%s", size);
		}
		
		endDrawing();
		
		if ((kHeld & KEY_L) && (kHeld & KEY_R))
			captureScreenshot();
		
		if ((kPressed & KEY_B) || (kPressed & KEY_A) || (touchInRect(36, 284, 192, 220)))
			properties = false;
	}
	
	return 0;
}