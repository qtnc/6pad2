#ifndef _____DLLMAIN_6PAD_H_____9
#define _____DLLMAIN_6PAD_H_____9
#include "global.h"
#include "IniFile.h"

struct SixpadData {
HWND win=0, status=0;
HINSTANCE hinstance = 0;
HFONT font=0;
tstring(*msg)(const char*) = 0;
int(*configGetInt)(const std::string&, int) = 0;
};
extern SixpadData sp;
extern HINSTANCE dllHinstance;

BOOL export SixpadDLLInit (const SixpadData*);

#endif
