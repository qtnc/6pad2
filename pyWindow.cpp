#include "global.h"
#include "strings.hpp"
#include "python34.h"
#include "Resource.h"
#include "inifile.h"
#include<functional>
using namespace std;

extern tstring appPath, appDir, appName, configFileName;
extern IniFile config;
extern bool headless;
extern HWND win;
extern vector<tstring> argv;

extern "C" FILE* msvcfopen (const char* name, const char* ax) ;
extern "C" void msvcfclose (FILE*);

bool PyRegister_MyObj (PyObject* m);

tstring msg (const char* name);
void ConsolePrint (const tstring& str);
void SetClipboardText (const tstring&);
tstring GetClipboardText (void);
int AddUserCommand (std::function<void(void)> f);
bool AddAccelerator (int flags, int key, int cmd);
bool KeyNameToCode (const tstring& kn, int& flags, int& key);

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
MessageBox(win, str.c_str(), title.c_str(), MB_OK | MB_ICONASTERISK);
}

static int PyConfirm (const tstring& str, const tstring& title) {
return IDYES==MessageBox(win, str.c_str(), title.c_str(), MB_YESNO | MB_ICONEXCLAMATION);
}

static PyMethodDef _6padMainDefs[] = {
PyDecl("print", ConsolePrint),
PyDecl("beep", Beep),
PyDecl("messageBeep", MessageBeep),
PyDecl("messageBox", PyMsgBox),
PyDecl("alert", PyAlert),
PyDecl("confirm", PyConfirm),
PyDecl("getTranslation", msg),
PyDecl("setClipboardText", SetClipboardText),
PyDecl("getClipboardText", GetClipboardText),
PyDecl("addAccelerator", PyAddAccelerator),
PyDeclEnd
};

static PyModuleDef _6padMainMod = {
PyModuleDef_HEAD_INIT,
"window",
"6pad++ window module",
-1,  _6padMainDefs 
};

PyMODINIT_FUNC PyInit_6padMain (void) {
PyObject* mod = PyModule_Create(&_6padMainMod);
PyRegister_MyObj(mod);
return mod;
}

void PyStart (void) {
Py_SetProgramName(const_cast<wchar_t*>(toWString(argv[0]).c_str()));
PyImport_AppendInittab("window", PyInit_6padMain);;
Py_Initialize();
GIL_PROTECT
{
Resource res(TEXT("init.py"),257);
char* code = (char*)res.copy();
PyRun_SimpleString(code);
delete[] code;
}
string pyfn = toString(appName + TEXT(".py"));
FILE* fp = msvcfopen(pyfn.c_str(), "r");
if (fp) {
PyRun_SimpleFile(fp, pyfn.c_str() );
msvcfclose(fp);
}
// Ohter initialization stuff
}
