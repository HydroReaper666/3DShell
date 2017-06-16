#include "unzip/archive.h"

#include "main.h"
#include "utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void setBilinearFilter(sf2d_texture *texture)
{
	sf2d_texture_set_params(texture, GPU_TEXTURE_MAG_FILTER(GPU_LINEAR) | GPU_TEXTURE_MIN_FILTER(GPU_NEAREST));
}

void endDrawing()
{
	sf2d_end_frame();
	sf2d_swapbuffers();
}

void getSizeString(char *string, uint64_t size) //Thanks TheOfficialFloW
{
	double double_size = (double)size;

	int i = 0;
	static char *units[] = { "B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };
	while (double_size >= 1024.0f) {
		double_size /= 1024.0f;
		i++;
	}

	sprintf(string, "%.*f %s", (i == 0) ? 0 : 2, double_size, units[i]);
}

int touchGetX()
{
	touchPosition pos;	
	hidTouchRead(&pos);
	return pos.px;
}

int touchGetY()
{
	touchPosition pos;	
	hidTouchRead(&pos);
	return pos.py;
}

sf2d_texture * sfil_load_IMG_file(const char * filename, sf2d_place place)
{
	int w, h;
	unsigned char *data = stbi_load(filename, &w, &h, NULL, 4);
	
	if (data == NULL) 
		return NULL;
	
	sf2d_texture * texture = NULL;
	texture = sf2d_create_texture_mem_RGBA8(data, w, h, TEXFMT_RGBA8, place);
	stbi_image_free(data);
	
	return texture;
}

int extractZip(const char * zipFile, const char * path) 
{
	char tmpFile2[1024];
	char tmpPath2[1024];
	FS_Archive sdmcArchive = 0;
	
	FSUSER_OpenArchive(&sdmcArchive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
	FS_Path tempPath = fsMakePath(PATH_ASCII, path);
	FSUSER_CreateDirectory(sdmcArchive, tempPath, FS_ATTRIBUTE_DIRECTORY);
	FSUSER_CloseArchive(sdmcArchive);
	
	strcpy(tmpPath2, "sdmc:");
	strcat(tmpPath2, (char *)path);
	chdir(tmpPath2);
	
	if (strncmp("romfs:/", zipFile, 7) == 0) 
		strcpy(tmpFile2, zipFile);
	else
	{
		strcpy(tmpFile2, "sdmc:");
		strcat(tmpFile2, (char*)zipFile);
	}
	
	Zip *handle = ZipOpen(tmpFile2);
	
	if (handle == NULL) 
		return 0;
	
	int result = ZipExtract(handle, NULL);
	ZipClose(handle);
	return result;
}

void setConfig(char * path, bool set) //using individual txt files for configs for now (plan to change this later when there's more options)
{
	if (set == true)
	{
		int config = 1;
		FILE * write = fopen(path, "w");
		fprintf(write, "%d", config);
		fclose(write);
	}
	else
	{
		int config = 0;
		FILE * write = fopen(path, "w");
		fprintf(write, "%d", config);
		fclose(write);
	}
}

const char * getLastNChars(char * str, int n)
{
	int len = strlen(str);
	const char *last_n = &str[len - n];
	return last_n;
}

u8 getRegion()
{
	u8 region;
	CFGU_SecureInfoGetRegion(&region);
	
	return region;
}

u8 getLanguage()
{
	u8 language;
	CFGU_GetSystemLanguage(&language);
	
	return language;
}

const char * getUsername() 
{
	int i;
	size_t size = 0x16;
	u8 * temp = (u8*)malloc(size);
	char * username = (char*)malloc(size / 2);
	
	for(i = 0; i < (size / 2); i++)
		username[i] = 0;
	
	CFGU_GetConfigInfoBlk2(0x1C, 0xA0000, temp);
	
	for(i = 0; i < (size / 2); i++)
		username[i] = (char)((u16*)temp)[i];
	
	return username;
}

bool isN3DS()
{
	bool isNew3DS = 0;
	APT_CheckNew3DS(&isNew3DS);
    
	if (isNew3DS)
		return true;
	else
		return false;
}

int fastStrcmp(const char *ptr0, const char *ptr1, int len)
{
	int fast = len/sizeof(size_t) + 1;
	int offset = (fast-1)*sizeof(size_t);
	int current_block = 0;

	if(len <= sizeof(size_t))
		fast = 0;

	size_t *lptr0 = (size_t*)ptr0;
	size_t *lptr1 = (size_t*)ptr1;

	while(current_block < fast)
	{
		if((lptr0[current_block] ^ lptr1[current_block]))
		{
			int pos;
			
			for(pos = current_block*sizeof(size_t); pos < len ; ++pos )
			{
				if((ptr0[pos] ^ ptr1[pos]) || (ptr0[pos] == 0) || (ptr1[pos] == 0))
					return (int)((unsigned char)ptr0[pos] - (unsigned char)ptr1[pos]);
			}	
		}

		++current_block;
    }

	while(len > offset)
	{
		if((ptr0[offset] ^ ptr1[offset]))
			return (int)((unsigned char)ptr0[offset] - (unsigned char)ptr1[offset]); 
		
		++offset;
	}	
	
	return 0;
}