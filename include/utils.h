#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>

#include <png.h>

#define SDK(a,b,c,d) ((a<<24)|(b<<16)|(c<<8)|d)

#define touchInRect(x1, x2, y1, y2) ((touchGetX() >= (x1) && touchGetX() <= (x2)) && (touchGetY() >= (y1) && touchGetY() <= (y2)))

#define wait(nanoSec) svcSleepThread(nanoSec);

typedef struct 
{
	u32 magic;
	u8* pixels;
	int width;
	int height;
	u16 bitperpixel;
} Bitmap;

void installDirectories(void);
void getSizeString(char * string, uint64_t size);
int touchGetX(void);
int touchGetY(void);
void setConfig(const char * path, bool set);
const char * getLastNChars(char * str, int n);
u8 getRegion(void);
u8 getLanguage(void);
const char * getUsername(void);
bool isN3DS(void);
void u16_to_u8(char* buf, const u16* input, size_t bufsize);

#endif