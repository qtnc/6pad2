#ifndef _____DLLMAIN_6PAD_H_____9
#define _____DLLMAIN_6PAD_H_____9
#include "global.h"
#include "IniFile.h"

struct Page;
struct IniFile;

struct SixpadData {
// Global info & members
DWORD version;
HWND win, status, tabctl;
HINSTANCE hinstance, dllHinstance;
LPCTSTR className;

// Methods/functions
tstring(*msg)(const char*);
void(*RegisterPageFactory)(const string& name, const function<Page*()>& f);
int(*AddUserCommand)(std::function<void(void)> f, int cmd) ;
bool(*RemoveUserCommand)(int cmd);
bool(*AddAccelerator)(HACCEL& hAccel, int flags, int key, int cmd);
BOOL(*RemoveAccelerator)(HACCEL& hAccel, int cmd);
bool(*FindAccelerator)(HACCEL& hAccel, int& cmd, int& flags, int& key);
tstring(*KeyCodeToName)(int flags, int vk, bool i18n);
bool(*KeyNameToCode)(const tstring& kn, int& flags, int& key);
int(*SetTimeout)(const std::function<void(void)>& f, int time, bool repeat);
void(*ClearTimeout)(int id);
void(*AddModlessWindow)(HWND win);
void(*RemoveModlessWindow)(HWND win);
void(*GoToNextModlessWindow)(int distance);

// Data tables config & language
IniFile *msgs, *config;

// Additional data
HACCEL& hAccel;
LPVOID reserved;
HFONT font;
bool headless;
};

extern "C" BOOL export SixpadDLLInit (SixpadData*);
typedef BOOL(*SixpadDLLInitFunc)(SixpadData*);
extern "C" BOOL export CallSixPadDLLInit (SixpadDLLInitFunc func);

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
