#ifndef _____DLLMAIN_6PAD_H_____9
#define _____DLLMAIN_6PAD_H_____9
#include "global.h"
#include "IniFile.h"

struct SixpadData {
HWND win=0, status=0, tabctl=0;
HINSTANCE hinstance = 0, dllHinstance=0;
HFONT font=0;
tstring appDir = TEXT("");
tstring(*msg)(const char*) = 0;
int(*configGetInt)(const std::string&, int) = 0;
};
extern SixpadData* sp;

BOOL export SixpadDLLInit (SixpadData*);

#ifdef SPDLL
extern HINSTANCE dllHinstance;
inline tstring msg (const char* name) {  return sp->msg(name);  }
#else
tstring msg (const char* name);
#endif
#endif
