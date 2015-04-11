#include "global.h"
#include "strings.hpp"
#include "python34.h"
#include "Resource.h"
#include "Thread.h"
#include "page.h"
#include "inifile.h"
#include<functional>
using namespace std;

extern tstring appPath, appDir, appName, configFileName;
extern IniFile config;
extern bool headless;
extern HWND win;
extern shared_ptr<Page> curPage;
extern vector<shared_ptr<Page>> pages;
extern vector<tstring> argv;

extern "C" FILE* msvcfopen (const char* name, const char* ax) ;
extern "C" void msvcfclose (FILE*);

bool PyRegister_MyObj (PyObject* m);

tstring msg (const char* name);
void ConsolePrint (const tstring& str);
tstring ConsoleRead (void);
void AppAddEvent (const string&, const PyCallback&);
void SetClipboardText (const tstring&);
tstring GetClipboardText (void);
int AddUserCommand (std::function<void(void)> f, int cmd=0);
bool AddAccelerator (int flags, int key, int cmd);
bool KeyNameToCode (const tstring& kn, int& flags, int& key);

bool PyRegister_MenuItem (PyObject* m);
PyObject* PyMenuItem_GetMenuBar (void);
PyObject* PyMenuItem_CreatePopupMenu (void);

bool PyRegister_EditorTab(PyObject* m);

static int PyAddAccelerator (const tstring& kn, PyCallback cb) {
int k=0, kf=0;
KeyNameToCode(kn, kf, k);
if (k<=0) return 0;
function<void(void)> f = [=]()mutable{ cb(); };
int cmd = AddUserCommand(f);
if (cmd<=0) return 0;
if (AddAccelerator(kf, k, cmd)) return cmd;
else return 0;
}

static int PyMsgBox (const tstring& str, const tstring& title, DWORD flags) {
return MessageBox(win, str.c_str(), title.c_str(), flags);
}

static void PyAlert (const tstring& str, const tstring& title) {
Py_BEGIN_ALLOW_THREADS
RunSync([&]()mutable{
MessageBox(win, str.c_str(), title.c_str(), MB_OK | MB_ICONASTERISK);
});//RunSync
Py_END_ALLOW_THREADS
}

static int PyConfirm (const tstring& str, const tstring& title) {
bool re = false;
Py_BEGIN_ALLOW_THREADS
RunSync([&]()mutable{
re = (IDYES==MessageBox(win, str.c_str(), title.c_str(), MB_YESNO | MB_ICONEXCLAMATION));
});//RunSync
Py_END_ALLOW_THREADS
return re;
}

static int PyEditorTabs_getTabCount () {
return pages.size();
}

static PyObject* PyEditorTabs_getCurTab () {
if (curPage) return curPage->GetPyData();
else {Py_RETURN_NONE;}
}

static PyObject* PyEditorTabs_getTab (int i) {
if (i>=0 && i<pages.size()) return pages[i]->GetPyData();
else {Py_RETURN_NONE;}
}

static tstring ConsoleReadImpl (void) {
tstring s;
Py_BEGIN_ALLOW_THREADS
s = ConsoleRead();
Py_END_ALLOW_THREADS
return s;
}

static PyMethodDef _6padMainDefs[] = {
// Overload of print, to be able to print in python console GUI
PyDecl("print", ConsolePrint),

// Basic dialog boxes and related functions
PyDecl("beep", Beep),
PyDecl("messageBeep", MessageBeep),
PyDecl("messageBox", PyMsgBox),
PyDecl("alert", PyAlert),
PyDecl("confirm", PyConfirm),

// Translation management
PyDecl("getTranslation", msg),

// Menus and accelerators management
PyDecl("addAccelerator", PyAddAccelerator),
PyDecl("getMenuBar", PyMenuItem_GetMenuBar),
PyDecl("createPopupMenu", PyMenuItem_CreatePopupMenu),

// Tabs management
PyDecl("getTabCount", PyEditorTabs_getTabCount),
PyDecl("getTab", PyEditorTabs_getTab),
PyDecl("getCurrentTab", PyEditorTabs_getCurTab),

// Misc functions
PyDecl("addEvent", AppAddEvent),
PyDecl("setClipboardText", SetClipboardText),
PyDecl("getClipboardText", GetClipboardText),
PyDecl("ConsoleReadImpl", ConsoleReadImpl),
PyDeclEnd
};

static PyModuleDef _6padMainMod = {
PyModuleDef_HEAD_INIT,
"window",
NULL, -1,  _6padMainDefs 
};

PyMODINIT_FUNC PyInit_6padMain (void) {
PyObject* mod = PyModule_Create(&_6padMainMod);
PyRegister_MenuItem(mod);
PyRegister_EditorTab(mod);
PyRegister_MyObj(mod);
return mod;
}

void PyStart (void) {
Py_SetProgramName(const_cast<wchar_t*>(toWString(argv[0]).c_str()));
PyImport_AppendInittab("window", PyInit_6padMain);;
Py_Initialize();
//PySys_SetPath(appDir.c_str());
PyEval_InitThreads();
GIL_PROTECT
{
Resource res(TEXT("init.py"),257);
char* code = (char*)res.copy();
PyRun_SimpleString(code);
delete[] code;
}
RunSync([](){});//Barrier to wait for the main loop to start
string pyfn = toString(appName + TEXT(".py"));
FILE* fp = msvcfopen(pyfn.c_str(), "r");
if (fp) {
PyRun_SimpleFile(fp, pyfn.c_str() );
msvcfclose(fp);
}
// Ohter initialization stuff goes here

// From now on, make this thread sleep forever
// For a yet unknown reason, if we don't do this, the python console window hangs
// Any clue why this is the case is welcome
//PyRun_SimpleString("from time import sleep");
//PyRun_SimpleString("while(1): sleep(60000)");
PyRun_SimpleString("import code");
PyRun_SimpleString("code.interact(banner='')");
}
