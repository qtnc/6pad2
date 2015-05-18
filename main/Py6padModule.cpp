#include "global.h"
#include "strings.hpp"
#include "IniFile.h"
#include "File.h"
#include "python34.h"
#include "Resource.h"
#include "Thread.h"
#include "UniversalSpeech.h"
using namespace std;

extern IniFile config, msgs;
extern tstring appPath, appDir, appName, configFileName, appLocale;
extern vector<tstring> argv;

//extern "C" FILE* msvcfopen (const char* name, const char* ax) ;
//extern "C" void msvcfclose (FILE*);
tstring ConsoleRead (void);
void ConsolePrint (const tstring& str, bool say);
void SetClipboardText (const tstring&);
tstring GetClipboardText (void);
tstring msg (const char* name);

bool PyRegister_Window (PyObject* m); 
bool PyRegister_EditorTab(PyObject* m);
bool PyRegister_MenuItem (PyObject* m);
PyObject* CreatePyWindowObject ();

static int PyInclude (const string& fn) {
bool result = false;
FILE* fp = fopen(fn.c_str(), "r");
if (fp) {
result = !PyRun_SimpleFile(fp, fn.c_str() );
fclose(fp);
}
return result;
}

static tstring ConsoleReadImpl (void) {
tstring s;
Py_BEGIN_ALLOW_THREADS
s = ConsoleRead();
Py_END_ALLOW_THREADS
return s;
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

static void PyLoadExtension (const string& name) {
if (ends_with(name, ".py")) PyInclude(name);
else if (ends_with(name, ".dll")) {}//C++ extension, not yet supported
else PyImport_ImportModule(name.c_str());
}

static int PyLoadLang (const tstring& langfile) {
msgs.load(langfile);
}

static void PyBraille (const tstring& str) {
brailleDisplay(str.c_str());
}

static PyObject* PySayStr (PyObject* unused, PyObject* args) {
const wchar_t* str = 0;
bool interrupt = false;
if (!PyArg_ParseTuple(args, "u|p", &str, &interrupt)) return NULL;
if (speechSay(str, interrupt)) Py_RETURN_TRUE;
else Py_RETURN_FALSE;
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

// Speech
PyDecl("stopSpeech", speechStop),
{"say", PySayStr, METH_VARARGS, NULL},
PyDecl("braille", PyBraille),


// Extension, includes and other general functions
PyDecl("include", PyInclude),
PyDecl("loadExtension", PyLoadExtension),
PyDecl("loadTranslation", PyLoadLang),
PyDecl("preg_replace", preg_replace),

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
//PyRegister_MyObj(mod);
PyModule_AddObject(mod, "window", CreatePyWindowObject() );
PyModule_AddObject(mod, "locale", Py_BuildValue("u", appLocale.c_str()));
PyModule_AddObject(mod, "appdir", Py_BuildValue("u", appDir.c_str()));
PyModule_AddObject(mod, "appfullpath", Py_BuildValue("u", appPath.c_str()));
PyModule_AddObject(mod, "appname", Py_BuildValue("u", appName.c_str()));
PyModule_AddObject(mod, "configfile", Py_BuildValue("u", configFileName.c_str()));
return mod;
}

void PyStart (void) {
wstring modulePath = toWString( appDir + TEXT("\\python34.zip;") + appDir + TEXT("\\lib;") + appDir + TEXT("\\plugins") );
Py_SetPath( modulePath.c_str() );
Py_SetProgramName(const_cast<wchar_t*>(toWString(argv[0]).c_str()));
PyImport_AppendInittab("sixpad", PyInit_6padMain);;
Py_Initialize();
PyEval_InitThreads();
GIL_PROTECT
{
Resource res(TEXT("init.py"),257);
auto code = res.copy();
bool failed = !!PyRun_SimpleString(&code[0]);
if (failed) exit(1);
}
RunSync([](){});//Barrier to wait for the main loop to start
{auto p=config.equal_range("extension"); for(auto it=p.first; it!=p.second; ++it) {
string name = it->second;
PyLoadExtension(name);
}}
string pyfn = toString(appDir + TEXT("\\") + appName + TEXT(".py") , CP_ACP);
PyInclude(pyfn);
// Ohter initialization stuff goes here

// From now on, make this thread sleep forever
// For a yet unknown reason, if we don't do this, the python console window hangs
// Any clue why this is the case is welcome
//PyRun_SimpleString("from time import sleep");
//PyRun_SimpleString("while(1): sleep(60000)");
if (PyRun_SimpleString("import code")) exit(1);
PyRun_SimpleString("code.interact(banner='')");
}
