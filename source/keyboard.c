#include "keyboard.h"

char * keyboard_3ds_get(int maxTextLength, const char* hintText)
{
	static SwkbdState swkbd;
	
	char * str = malloc(maxTextLength);
	memset(str, 0, maxTextLength);
	
	swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, maxTextLength);
	
    swkbdSetHintText(&swkbd, hintText);
	
	swkbdSetButton(&swkbd, SWKBD_BUTTON_LEFT, "Cancel", false);
	swkbdSetButton(&swkbd, SWKBD_BUTTON_RIGHT, "Confirm", true);
	
	swkbdSetFeatures(&swkbd, SWKBD_ALLOW_HOME);
	swkbdSetFeatures(&swkbd, SWKBD_ALLOW_RESET);
	swkbdSetFeatures(&swkbd, SWKBD_ALLOW_POWER);
	
	swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
	
	SwkbdDictWord words[7];
	swkbdSetDictWord(&words[0], ".3dsx", ".3dsx");
	swkbdSetDictWord(&words[1], ".cia", ".cia");
	swkbdSetDictWord(&words[2], "github", "github");
	swkbdSetDictWord(&words[3], "http://", "http://");
	swkbdSetDictWord(&words[4], "https://", "https://");
	swkbdSetDictWord(&words[5], "releases", "releases");
	swkbdSetDictWord(&words[6], "/3ds/", "/3ds/");
	
	swkbdSetDictionary(&swkbd, words, sizeof(words)/sizeof(SwkbdDictWord));
	
	swkbdInputText(&swkbd, str, maxTextLength);
	
	return str;
}