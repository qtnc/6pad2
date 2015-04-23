#include "global.h"
#include "strings.hpp"
#include "page.h"
#include "python34.h"
#include "page.h"
#include "inifile.h"
#include "Resource.h"
#include "Thread.h"
#include<functional>
using namespace std;

struct PyWindow { 
    PyObject_HEAD 
};

extern tstring appPath, appDir, appName, configFileName;
extern IniFile config;
extern bool headless;
extern HWND win, status;
extern shared_ptr<Page> curPage;
extern vector<shared_ptr<Page>> pages;
extern vector<tstring> argv;

tstring msg (const char* name);
void AppAddEvent (const string&, const PyCallback&);
void AppRemoveEvent (const string&, const PyCallback&);
int AddUserCommand (std::function<void(void)> f, int cmd=0);
bool AddAccelerator (int flags, int key, int cmd);
bool KeyNameToCode (const tstring& kn, int& flags, int& key);
shared_ptr<Page> OpenFile (const tstring& filename, int flags);
shared_ptr<Page> PageAddEmpty (bool focus, const string& type);

PyObject* PyMenuItem_GetMenuBar (void);
PyObject* PyMenuItem_CreatePopupMenu (void);

static int PyAddAccelerator (const tstring& kn, PyCallback cb) {
int k=0, kf=0;
KeyNameToCode(kn, kf, k);
if (k<=0) return 0;
function<void(void)> f = [=]()mutable{  cb(); };
int cmd = AddUserCommand(f);
if (cmd<=0) return 0;
if (AddAccelerator(kf, k, cmd)) return cmd;
else return 0;
}

static PyObject* PyOpenFile (const tstring& filename) {
shared_ptr<Page> p = OpenFile(filename, 1);
if (!p) { Py_RETURN_NONE; }
return p->GetPyData();
}

static PyObject* PyNewPage (const string& type) {
shared_ptr<Page> p = PageAddEmpty(true, type);
if (!p) { Py_RETURN_NONE; }
return p->GetPyData();
}

static int PyMsgBox (const tstring& str, const tstring& title, DWORD flags) {
int re;
Py_BEGIN_ALLOW_THREADS
RunSync([&]()mutable{
re = MessageBox(win, str.c_str(), title.c_str(), flags);
});//RunSync
Py_END_ALLOW_THREADS
return re;
}

static void PyAlert (const tstring& str, const tstring& title) {
Py_BEGIN_ALLOW_THREADS
RunSync([&]()mutable{
MessageBox(win, str.c_str(), title.c_str(), MB_OK | MB_ICONASTERISK);
});//RunSync
Py_END_ALLOW_THREADS
}

static void PyWarn (const tstring& str, const tstring& title) {
Py_BEGIN_ALLOW_THREADS
RunSync([&]()mutable{
MessageBox(win, str.c_str(), title.c_str(), MB_OK | MB_ICONERROR);
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

static tstring PyGetStatusText () {
return GetWindowText(status);
}

static void PySetStatusText (const tstring& text) {
SetWindowText(status, text);
}

static tstring PyGetWinTitle () {
return GetWindowText(win);
}

static void PySetWinTitle (const tstring& title) {
SetWindowText(win, title);
}

static int PyEditorTabs_getTabCount () {
return pages.size();
}

static PyObject* PyEditorTabs_getCurTab () {
if (curPage) return curPage->GetPyData();
else {Py_RETURN_NONE;}
}

static PyObject* PyEditorTabs_getTabList () {
PyObject* list = PyList_New(pages.size());
for (int i=0, n=pages.size(); i<n; i++) {
PyObject* obj = pages[i]->GetPyData();
Py_XINCREF(obj);
PyList_SetItem(list, i, obj);
}
return list;
}

static void PyWindowDealloc (PyObject* pySelf) {
PyWindow* self = (PyWindow*)pySelf;
Py_TYPE(pySelf)->tp_free(pySelf);
}

static PyWindow* PyWindowNew (PyTypeObject* type, PyObject* args, PyObject* kwds) {
PyWindow* self = (PyWindow*)(type->tp_alloc(type, 0));
return self;
}

static int PyWindowInit (PyWindow* self, PyObject* args, PyObject* kwds) {
return 0;
}

static PyMethodDef PyWindowMethods[] = {
// General 6pad++ functions
PyDecl("open", PyOpenFile),
PyDecl("new", PyNewPage),

// Basic dialog boxes and related functions
PyDecl("beep", Beep),
PyDecl("messageBeep", MessageBeep),
PyDecl("messageBox", PyMsgBox),
PyDecl("alert", PyAlert),
PyDecl("warning", PyWarn),
PyDecl("confirm", PyConfirm),

// Menus and accelerators management
PyDecl("addAccelerator", PyAddAccelerator),
PyDecl("createPopupMenu", PyMenuItem_CreatePopupMenu),

// Global events management
PyDecl("addEvent", AppAddEvent),
PyDecl("removeEvent", AppRemoveEvent),

PyDeclEnd
};

static PyGetSetDef PyWindowAccessors[] = {

// Global attributes
PyAccessor("status", PyGetStatusText, PySetStatusText),
PyAccessor("title", PyGetWinTitle, PySetWinTitle),

// Tabs management
PyReadOnlyAccessor("curPage", PyEditorTabs_getCurTab),
PyReadOnlyAccessor("pages", PyEditorTabs_getTabList),
PyReadOnlyAccessor("pageCount", PyEditorTabs_getTabCount),

// Menus and accelerators management
PyReadOnlyAccessor("menus", PyMenuItem_GetMenuBar),

PyDeclEnd
};

static PyTypeObject PyWindowType = { 
    PyVarObject_HEAD_INIT(NULL, 0) 
    "window.Window",             /* tp_name */ 
    sizeof(PyWindow), /* tp_basicsize */ 
    0,                         /* tp_itemsize */ 
    PyWindowDealloc,                         /* tp_dealloc */ 
    0,                         /* tp_print */ 
    0,                         /* tp_getattr */ 
    0,                         /* tp_setattr */ 
    0,                         /* tp_reserved */ 
    0,                         /* tp_repr */ 
    0,                         /* tp_as_number */ 
    0,                         /* tp_as_sequence */ 
    0,                         /* tp_as_mapping */ 
    0,                         /* tp_hash  */ 
    0,                         /* tp_call */ 
    0,                         /* tp_str */ 
    0,                         /* tp_getattro */ 
    0,                         /* tp_setattro */ 
    0,                         /* tp_as_buffer */ 
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /* tp_flags */ 
    NULL,           /* tp_doc */
    0,                         /* tp_traverse */ 
    0,                         /* tp_clear */ 
    0,                         /* tp_richcompare */ 
    0,                         /* tp_weaklistoffset */ 
    0,                         /* tp_iter */ 
    0,                         /* tp_iternext */ 
    PyWindowMethods,             /* tp_methods */ 
NULL,             /* tp_members */ 
    PyWindowAccessors,                         /* tp_getset */ 
    0,                         /* tp_base */ 
    0,                         /* tp_dict */ 
    0,                         /* tp_descr_get */ 
    0,                         /* tp_descr_set */ 
    0,                         /* tp_dictoffset */ 
    (initproc)PyWindowInit,      /* tp_init */ 
    0,                         /* tp_alloc */ 
}; 

bool PyRegister_Window (PyObject* m) {
//PyWindowType.tp_new = (decltype(PyWindowType.tp_new))PyWindowNew;
if (PyType_Ready(&PyWindowType) < 0)          return false;
Py_INCREF(&PyWindowType); 
PyModule_AddObject(m, "Window", (PyObject*)&PyWindowType); 
return true;
}

PyObject* CreatePyWindowObject () {
return (PyObject*)PyWindowNew(&PyWindowType, NULL, NULL);
}
