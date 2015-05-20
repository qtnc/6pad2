#ifndef _____DLLMAIN_6PAD_H_____9
#define _____DLLMAIN_6PAD_H_____9
#include "global.h"
#include "IniFile.h"

struct Page;
struct IniFile;

struct SixpadData {
tstring(*msg)(const char*);
void(*RegisterPageFactory)(const string& name, const function<Page*()>& f);
int(*AddUserCommand)(std::function<void(void)> f, int cmd) ;
bool(*RemoveUserCommand)(int cmd);
bool(*AddAccelerator)(int flags, int key, int cmd);
BOOL(*RemoveAccelerator)(int cmd);
tstring(*KeyCodeToName)(int flags, int vk, bool i18n);
bool(*KeyNameToCode)(const tstring& kn, int& flags, int& key);
int(*SetTimeout)(const std::function<void(void)>& f, int time, bool repeat);
void(*ClearTimeout)(int id);
IniFile *msgs, *config;
LPCTSTR className;
DWORD version;
LPVOID reserved;
HWND win, status, tabctl;
HINSTANCE hinstance, dllHinstance;
HFONT font;
};

extern "C" BOOL export SixpadDLLInit (SixpadData*);

#ifdef SPDLL
#define SPPTR sp->
extern HINSTANCE dllHinstance;
extern SixpadData* sp;
inline tstring msg (const char* name) {  return sp->msg(name);  }
#else
#define SPPTR sp.
extern SixpadData sp;
tstring msg (const char* name);
#endif
#endif
