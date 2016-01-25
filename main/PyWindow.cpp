#include "global.h"
#include "strings.hpp"
#include "page.h"
#include "python34.h"
#include "accelerators.h"
#include "inifile.h"
#include "Resource.h"
#include "Thread.h"
#include "dialogs.h"
#include<functional>
#include<sstream>
using namespace std;

struct PyWindow { 
    PyObject_HEAD 
PyObject* dic;
};

extern tstring appPath, appDir, appName, configFileName;
extern IniFile config;
extern bool headless;
extern HWND win, status;
extern shared_ptr<Page> curPage;
extern vector<shared_ptr<Page>> pages;
extern vector<tstring> argv;

tstring msg (const char* name);
int AppAddEvent (const string&, const PySafeObject&);
int AppRemoveEvent (const string&, int id);
int AddUserCommand (std::function<void(void)> f, int cmd=0);
int SetTimeout (const std::function<void(void)>& f, int time, bool repeat);
void ClearTimeout (int id);
shared_ptr<Page> OpenFile (tstring filename, int flags);
shared_ptr<Page> PageAddEmpty (bool focus, const string& type);

PyObject* PyMenuItem_GetMenuBar (void);
int PyShowPopupMenu (PyObject*);
PyObject* PyTaskDialog (PyObject*, PyObject*, PyObject*);

static int PyAddAccelerator (const tstring& kn, PySafeObject cb, OPT, bool specific) {
int k=0, kf=0;
KeyNameToCode(kn, kf, k);
if (k<=0) return 0;
function<void()> f = cb.asFunction<void()>();
int cmd = AddUserCommand(f);
if (cmd<=0) return 0;
HACCEL& accel = curPage&&specific? curPage->hPageAccel : sp.hAccel;
if (AddAccelerator(accel, kf, k, cmd)) return cmd;
else return 0;
}

static bool PyRemoveAccelerator (int id) {
bool re = RemoveAccelerator(sp.hAccel,id);
if (!re && curPage) re = RemoveAccelerator(curPage->hPageAccel, id);
return re;
}

tstring PyFindAcceleratorByID (int cmd) {
int k=0, kf=0;
FindAccelerator(sp.hAccel, cmd, kf, k);
if (k<0) return TEXT("");
else return KeyCodeToName(kf,k,false);
}

int PyFindAcceleratorByKey (const tstring& kn) {
int cmd=0, k=0, kf=0;
KeyNameToCode(kn, kf, k);
FindAccelerator(sp.hAccel, cmd, kf, k);
return cmd;
}

static PyObject* PyOpenFile (const tstring& filename) {
shared_ptr<Page> p;
RunSync([&]()mutable{
if (filename.size()>0) p = OpenFile(filename, OF_CHECK_OTHER_WINDOWS);
else p = PageAddEmpty(true, "text");
});
if (!p) { Py_RETURN_NONE; }
return p->GetPyData();
}

static PyObject* PyNewPage (OPT, string type) {
if (type.size()<=0) type="text";
shared_ptr<Page> p = PageAddEmpty(true, type);
if (!p) { Py_RETURN_NONE; }
return p->GetPyData();
}

static void PyFocusWin (void) {
RunSync([&]()mutable{
if (GetWindowLong(win, GWL_STYLE)&WS_MINIMIZE) ShowWindow(win, SW_RESTORE);
SetForegroundWindow(win);
});//
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

static void PyAlert (const tstring& str, OPT, const tstring& title) {
Py_BEGIN_ALLOW_THREADS
RunSync([&]()mutable{
MessageBox(win, str.c_str(), (title.size()>0?title:msg("Info")).c_str(), MB_OK | MB_ICONASTERISK);
});//RunSync
Py_END_ALLOW_THREADS
}

static void PyWarn (const tstring& str, OPT, const tstring& title) {
Py_BEGIN_ALLOW_THREADS
RunSync([&]()mutable{
MessageBox(win, str.c_str(), (title.size()>0?title:msg("Warning!")).c_str(), MB_OK | MB_ICONERROR);
});//RunSync
Py_END_ALLOW_THREADS
}

static int PyConfirm (const tstring& str, OPT, const tstring& title) {
bool re = false;
Py_BEGIN_ALLOW_THREADS
RunSync([&]()mutable{
re = (IDYES==MessageBox(win, str.c_str(), (title.size()>0?title:msg("Question")).c_str(), MB_YESNO | MB_ICONEXCLAMATION));
});//RunSync
Py_END_ALLOW_THREADS
return re;
}

static PyObject* PyChoiceDlg (PyObject* unused, PyObject* args, PyObject* dic) {
int initialSelection = 0;
PyObject* pOptions = NULL;
const wchar_t *prompt=0, *title=0;
static const char* KWLST[] = {"prompt", "title", "options", "initialSelection", NULL};
if (!PyArg_ParseTupleAndKeywords(args, dic, "uuO|i", (char**)KWLST, &prompt, &title, &pOptions, &initialSelection) || !PySequence_Check(pOptions)) return NULL;
vector<tstring> options;
for (int i=0, n=PySequence_Size(pOptions); i<n; i++) {
PyObject* item = PySequence_GetItem(pOptions,i);
if (!item || !PyUnicode_Check(item)) return NULL;
const wchar_t* str = PyUnicode_AsUnicode(item);
if (!str) return NULL;
options.push_back(str);
}
int re = -1;
Py_BEGIN_ALLOW_THREADS
RunSync([&]()mutable{
re = ChoiceDialog(win, title, prompt, options, initialSelection);
});//RunSync
Py_END_ALLOW_THREADS
return Py_BuildValue("i",re);
}

static PyObject* PyInputDlg (PyObject* unused, PyObject* args, PyObject* dic) {
PyObject* pOptions = NULL;
const wchar_t *prompt=0, *title=0, *pText=0;
static const char* KWLST[] = {"prompt", "title", "text", "list", NULL};
if (!PyArg_ParseTupleAndKeywords(args, dic, "uu|uO", (char**)KWLST, &prompt, &title, &pText, &pOptions)) return NULL;
if (pOptions && pOptions!=Py_None && !PySequence_Check(pOptions)) return NULL;
vector<tstring> options;
if (pOptions&&pOptions!=Py_None) for (int i=0, n=PySequence_Size(pOptions); i<n; i++) {
PyObject* item = PySequence_GetItem(pOptions,i);
if (!item || !PyUnicode_Check(item)) return NULL;
const wchar_t* str = PyUnicode_AsUnicode(item);
if (!str) return NULL;
options.push_back(str);
}
tstring text = pText?pText:TEXT("");
Py_BEGIN_ALLOW_THREADS
RunSync([&]()mutable{
text = InputDialog(win, title, prompt, text, options);
});//RunSync
Py_END_ALLOW_THREADS
if (text.size()<=0) { Py_RETURN_NONE; }
else return Py_BuildValue("u",text.c_str() );
}

static PyObject* PyFileDlg (PyObject* args, PyObject* dic, int flags) {
bool multiple=false;
int initialFilter=0;
const wchar_t *cTitle=0, *cFile=0;
PyObject* pFilters=0;
static const char* KWLST[] = {"file", "title", "filters", "initialFilter", "multiple", NULL};
if (!PyArg_ParseTupleAndKeywords(args, dic, "|uuOip", (char**)KWLST, &cFile, &cTitle, &pFilters, &initialFilter, &multiple)) return NULL;
wostringstream oFilters;
if (pFilters && PySequence_Check(pFilters)) for (int i=0, n=PySequence_Size(pFilters); i<n; i++) {
PyObject* pItem = PySequence_GetItem(pFilters,i);
if (!pItem || !PySequence_Check(pItem) || PySequence_Size(pItem)!=2) return NULL;
PyObject *pfText = PySequence_GetItem(pItem,0), *pfGlob = PySequence_GetItem(pItem,1);
if (!pfGlob || !pfText || !PyUnicode_Check(pfText) || !PyUnicode_Check(pfGlob)) return NULL;
const wchar_t *cGlob = PyUnicode_AsUnicode(pfGlob), *cText = PyUnicode_AsUnicode(pfText);
if (!cText || !cGlob) return NULL;
if (i>0) oFilters << (wchar_t)'|';
oFilters << cText << (wchar_t)'|' << cGlob;
}
if (!(flags&FD_OPEN)) multiple=false;
if (multiple) flags |= FD_MULTI;
tstring title = toTString(cTitle?cTitle:TEXT("")), file = toTString(cFile?cFile:TEXT("")), filters = toTString(oFilters.str());

int z=0;
tstring selection = TEXT("");
Py_BEGIN_ALLOW_THREADS
RunSync([&]()mutable{
selection = FileDialog(win, flags, file, title, filters, &initialFilter);
});//RunSync
Py_END_ALLOW_THREADS
PyObject* re = NULL;
if (selection.size()<=0) { re = Py_None; Py_INCREF(Py_None); }
else if (!multiple) re = Py_BuildValue("u", toWString(selection).c_str());
else if ((z=selection.find('|'))<0 || z>=selection.size()) {
re = PyList_New(1);
PyList_SetItem(re, 0, Py_BuildValue("u", selection.c_str()));
}
else {
vector<tstring> files = split(selection, TEXT("|"));
re = PyList_New(files.size() -1);
for (int i=1, n=files.size(); i<n; i++) PyList_SetItem(re, i -1, Py_BuildValue("u", toWString(files[0] + TEXT("\\") + files[i]).c_str()));
}
if (filters.size()>0) re = Py_BuildValue("(Oi)", re, initialFilter -1);
return re;
}

static PyObject* PyOpenFileDlg (PyObject* unused, PyObject* args, PyObject* dic) {
return PyFileDlg(args, dic, FD_OPEN);
}

static PyObject* PySaveFileDlg (PyObject* unused, PyObject* args, PyObject* dic) {
return PyFileDlg(args, dic, FD_SAVE);
}

static PyObject* PyFolderDlg (PyObject* unused, PyObject* args, PyObject* dic) {
wchar_t *cFolder=0, *cTitle=0, *cRoot=0;
bool includeFiles=false;
static const char* KWLST[] = {"folder", "title", "root", "showFiles", NULL};
if (!PyArg_ParseTupleAndKeywords(args, dic, "|uuup", (char**)KWLST, &cFolder, &cTitle, &cRoot, &includeFiles)) return NULL;
tstring re, folder = cFolder?cFolder:TEXT(""), title = cTitle?cTitle:TEXT(""), root = cRoot?cRoot:TEXT("");
Py_BEGIN_ALLOW_THREADS
RunSync([&]()mutable{
re = FolderDialog(win, folder, title, root, includeFiles);
});//RunSync
Py_END_ALLOW_THREADS
if (re.size()>0) return Py_BuildValue("u", re.c_str());
else { Py_RETURN_NONE; }
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

static int PySetTimer1 (const PySafeObject& cb, int time) {
return SetTimeout(cb.asFunction<void()>(), time, false);
}

static int PySetTimer2 (const PySafeObject& cb, int time) {
return SetTimeout(cb.asFunction<void()>(), time, true);
}

static int PyPlaySound (const tstring& filename) {
PlaySound(filename.c_str(), NULL, SND_ASYNC | SND_FILENAME);
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
Py_XDECREF(self->dic);
self->dic=NULL;
Py_TYPE(pySelf)->tp_free(pySelf);
}

static PyWindow* PyWindowNew (PyTypeObject* type, PyObject* args, PyObject* kwds) {
PyWindow* self = (PyWindow*)(type->tp_alloc(type, 0));
self->dic=PyDict_New();
return self;
}

static int PyWindowInit (PyWindow* self, PyObject* args, PyObject* kwds) {
return 0;
}

static string test123 (int a, OPT, string b) {
return b+toString(a);
}

static PyMethodDef PyWindowMethods[] = {
// General 6pad++ functions
PyDecl("open", PyOpenFile),
PyDecl("new", PyNewPage),
PyDecl("playSound", PyPlaySound),

// Basic dialog boxes and related functions
PyDecl("focus", PyFocusWin),
PyDecl("beep", Beep),
PyDecl("messageBeep", MessageBeep),
PyDecl("messageBox", PyMsgBox),
PyDecl("alert", PyAlert),
PyDecl("warning", PyWarn),
PyDecl("confirm", PyConfirm),
{"choice", (PyCFunction)PyChoiceDlg, METH_VARARGS | METH_KEYWORDS, NULL},
{"prompt", (PyCFunction)PyInputDlg, METH_VARARGS | METH_KEYWORDS, NULL},
{"taskDialog", (PyCFunction)PyTaskDialog, METH_VARARGS | METH_KEYWORDS, NULL},
{"openDialog", (PyCFunction)PyOpenFileDlg, METH_VARARGS | METH_KEYWORDS, NULL},
{"saveDialog", (PyCFunction)PySaveFileDlg, METH_VARARGS | METH_KEYWORDS, NULL},
{"chooseFolder", (PyCFunction)PyFolderDlg, METH_VARARGS | METH_KEYWORDS, NULL},

// Menus and accelerators management
PyDecl("addAccelerator", PyAddAccelerator),
PyDecl("RemoveAccelerator", PyRemoveAccelerator),
PyDecl("findAcceleratorByID", PyFindAcceleratorByID),
PyDecl("findAcceleratorByKey", PyFindAcceleratorByKey),
PyDecl("showPopupMenu", &PyShowPopupMenu),

// Global events management
PyDecl("addEvent", AppAddEvent),
PyDecl("removeEvent", AppRemoveEvent),
PyDecl("setTimeout", PySetTimer1),
PyDecl("setInterval", PySetTimer2),
PyDecl("clearTimeout", ClearTimeout),
PyDecl("clearInterval", ClearTimeout),

PyDecl("test", test123),
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
    Py_TPFLAGS_DEFAULT,        /* tp_flags */ 
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
    offsetof(PyWindow,dic),                         /* tp_dictoffset */ 
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



