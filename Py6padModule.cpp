#include "global.h"
#include "strings.hpp"
#include "IniFile.h"
#include "python34.h"
#include "Resource.h"
#include "Thread.h"
using namespace std;

extern IniFile config;
extern tstring appPath, appDir, appName, configFileName;
extern vector<tstring> argv;

extern "C" FILE* msvcfopen (const char* name, const char* ax) ;
extern "C" void msvcfclose (FILE*);
tstring ConsoleRead (void);
void ConsolePrint (const tstring& str);
void SetClipboardText (const tstring&);
tstring GetClipboardText (void);
tstring msg (const char* name);

bool PyRegister_MyObj (PyObject* m);
bool PyRegister_Window (PyObject* m); 
bool PyRegister_EditorTab(PyObject* m);
bool PyRegister_MenuItem (PyObject* m);
PyObject* CreatePyWindowObject ();

static bool PyInclude (const string& fn) {
FILE* fp = msvcfopen(fn.c_str(), "r");
if (fp) {
PyRun_SimpleFile(fp, fn.c_str() );
msvcfclose(fp);
}
return !!fp;
}

static tstring ConsoleReadImpl (void) {
tstring s;
Py_BEGIN_ALLOW_THREADS
s = ConsoleRead();
Py_END_ALLOW_THREADS
return s;
}

static tstring GetCurrentDirectory2 (void) {
TCHAR buf[300] = {0};
GetCurrentDirectory(300, buf);
return buf;
}

static string PyGetConfig (const string& key, const string& def) {
auto it = config.find(key);
if (it!=config.end()) return it->second;
else return def;
}

static PyObject* PySetConfig (PyObject* unused, PyObject* args, PyObject* dic) {
const char *key=NULL, *value=NULL;
bool allowMulti = false;
static const char* KWLST[] = {"key", "value", "multiple", NULL};
if (!PyArg_ParseTupleAndKeywords(args, dic, "ss|p", (char**)KWLST, &key, &value, &allowMulti)) return NULL;
if (key&&value) config.set(string(key), string(value), allowMulti);
Py_RETURN_NONE;
}

static PyObject* PyGetConfigMulti (const string& key) {
PyObject* list = PyList_New( config.count(key) );
int i=0;
for (auto it=config.find(key); it!=config.end(); ++it) {
PyList_SetItem(list, i++, Py_BuildValue("s", it->second.c_str()) );
}
return list;
}

static PyMethodDef _6padMainDefs[] = {
// Translation management
PyDecl("msg", msg),

// Configuration management
PyDecl("getConfig", PyGetConfig),
{"setConfig", (PyCFunction)PySetConfig, METH_VARARGS | METH_KEYWORDS, NULL},
PyDecl("getConfigAsList", PyGetConfigMulti),

// Clipboard management
PyDecl("setClipboardText", SetClipboardText),
PyDecl("getClipboardText", GetClipboardText),

// FIle and directory management
PyDecl("include", PyInclude),
PyDecl("getCurrentDirectory", GetCurrentDirectory2),
PyDecl("setCurrentDirectory", SetCurrentDirectory),

// Overload of print, to be able to print in python console GUI
PyDecl("sysPrint", ConsolePrint),
// Console read for interactive interpreter ithin python console GUI
PyDecl("sysRead", ConsoleReadImpl),
PyDeclEnd
};

static PyModuleDef _6padMainMod = {
PyModuleDef_HEAD_INIT,
"sixpad",
NULL, -1,  _6padMainDefs 
};

PyMODINIT_FUNC PyInit_6padMain (void) {
PyObject* mod = PyModule_Create(&_6padMainMod);
PyRegister_Window(mod);
PyRegister_MenuItem(mod);
PyRegister_EditorTab(mod);
PyRegister_MyObj(mod);
PyModule_AddObject(mod, "window", CreatePyWindowObject() );
return mod;
}

void PyStart (void) {
Py_SetProgramName(const_cast<wchar_t*>(toWString(argv[0]).c_str()));
PyImport_AppendInittab("sixpad", PyInit_6padMain);;
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
string pyfn = toString(appDir + TEXT("\\") + appName + TEXT(".py"));
PyInclude(pyfn);
for (auto it = config.find("extension"); it!=config.end(); ++it) PyInclude(it->second);

// Ohter initialization stuff goes here

// From now on, make this thread sleep forever
// For a yet unknown reason, if we don't do this, the python console window hangs
// Any clue why this is the case is welcome
//PyRun_SimpleString("from time import sleep");
//PyRun_SimpleString("while(1): sleep(60000)");
PyRun_SimpleString("import code");
PyRun_SimpleString("code.interact(banner='')");
}
