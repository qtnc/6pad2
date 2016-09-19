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
shared_ptr<Page> OpenFile (tstring filename, int flags);
shared_ptr<Page> PageAddEmpty (bool focus, const string& type);

PyObject* PyMenuItem_GetMenuBar (void);
int PyShowPopupMenu (const vector<tstring>&);

PyObject* PyShowTaskDialog (PyObject* unused, PyObject* args, PyObject* kwds); 

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
static constexpr const char* Accelerator_KWLST[] = { "key", "action", "specific", NULL };

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
MessageBox(GetForegroundWindow(), str.c_str(), (title.size()>0?title:msg("Info")).c_str(), MB_OK | MB_ICONASTERISK);
});//RunSync
Py_END_ALLOW_THREADS
}

static void PyWarn (const tstring& str, OPT, const tstring& title) {
Py_BEGIN_ALLOW_THREADS
RunSync([&]()mutable{
MessageBox(GetForegroundWindow(), str.c_str(), (title.size()>0?title:msg("Warning!")).c_str(), MB_OK | MB_ICONERROR);
});//RunSync
Py_END_ALLOW_THREADS
}

static bool PyConfirm (const tstring& str, OPT, const tstring& title) {
bool re = false;
Py_BEGIN_ALLOW_THREADS
RunSync([&]()mutable{
re = (IDYES==MessageBox(GetForegroundWindow(), str.c_str(), (title.size()>0?title:msg("Question")).c_str(), MB_YESNO | MB_ICONEXCLAMATION));
});//RunSync
Py_END_ALLOW_THREADS
return re;
}

static int PyChoiceDlg (const tstring& prompt, const tstring& title, const vector<tstring>& options, OPT, int initialSelection) {
int re = -1;
Py_BEGIN_ALLOW_THREADS
RunSync([&]()mutable{
re = ChoiceDialog(GetForegroundWindow(), title, prompt, options, initialSelection);
});//RunSync
Py_END_ALLOW_THREADS
return re;
}
static constexpr const char* PyChoiceDlgKWLST[] = {"prompt", "title", "options", "initialSelection", NULL};

static optional<tstring> PyInputDlg (const tstring& prompt, const tstring& title, OPT, const tstring& text, const vector<tstring>& options) {
tstring result;
Py_BEGIN_ALLOW_THREADS
RunSync([&]()mutable{
result = InputDialog(GetForegroundWindow(), title, prompt, text, options);
});//RunSync
Py_END_ALLOW_THREADS
if (result.empty()) return none;
else return result;
}
static constexpr const char* PyInputDlgKWLST[] = {"prompt", "title", "text", "list", NULL};

static any PyFileDlg (int flags, const tstring& file, const tstring& title, const vector<pair<tstring,tstring>>& pFilters, int initialFilter, bool multiple) {
wostringstream oFilters;
int i=0;
for (auto& p: pFilters) {
if (i++>0) oFilters << (wchar_t)'|';
oFilters << p.first << (wchar_t)'|' << p.second;
}
if (!(flags&FD_OPEN)) multiple=false;
if (multiple) flags |= FD_MULTI;
tstring selection, filters = toTString(oFilters.str());
Py_BEGIN_ALLOW_THREADS
RunSync([&]()mutable{
selection = FileDialog(GetForegroundWindow(), flags, file, title, filters, &initialFilter);
});//RunSync
Py_END_ALLOW_THREADS
if (selection.empty()) return nullptr;
else if (!multiple) {
if (filters.empty()) return selection;
else return pair<tstring,int>(selection, initialFilter -1);
}
else if ((i=selection.find('|'))<0 || i>=selection.size()) {
if (filters.empty()) return vector<tstring>{selection};
else return pair<vector<tstring>,int>({selection}, initialFilter -1);
}
else {
vector<tstring> files = split(selection, TEXT("|"));
tstring prepend = files[0];
files.erase(files.begin());
for (tstring& file: files) file = prepend + TEXT("\\") + file;
if (filters.empty()) return files;
else return pair<vector<tstring>,int>(files, initialFilter -1);
}}
static constexpr const char* PyFileDlg_KWLST[] = {"file", "title", "filters", "initialFilter", "multiple", NULL};

static any PyOpenFileDlg  (OPT, const tstring& file, const tstring& title, const vector<pair<tstring,tstring>>& pFilters, int initialFilter, bool multiple) {
return PyFileDlg(FD_OPEN, file, title, pFilters, initialFilter, multiple);
}

static any PySaveFileDlg  (OPT, const tstring& file, const tstring& title, const vector<pair<tstring,tstring>>& pFilters, int initialFilter, bool multiple) {
return PyFileDlg(FD_SAVE, file, title, pFilters, initialFilter, multiple);
}

static optional<tstring> PyFolderDlg (OPT, const tstring& folder, const tstring& title, const tstring& root, bool includeFiles) {
tstring re;
Py_BEGIN_ALLOW_THREADS
RunSync([&]()mutable{
re = FolderDialog(GetForegroundWindow(), folder, title, root, includeFiles);
});//RunSync
Py_END_ALLOW_THREADS
if (re.empty()) return none;
else return re;
}
static constexpr const char* PyFolderDlg_KWLST[] = {"folder", "title", "root", "showFiles", NULL};

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
return SetTimeout(PyCallback<void(void)>(cb), time, false);
}

static int PySetTimer2 (const PySafeObject& cb, int time) {
return SetTimeout(cb.asFunction<void()>(), time, true);
}

static void PyPlaySound (const tstring& filename) {
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

constexpr const char* test123kw[] = { "one", "two", NULL };
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
PyDeclKW("choice", PyChoiceDlg, PyChoiceDlgKWLST),
PyDeclKW("prompt", PyInputDlg, PyInputDlgKWLST),
{"taskDialog", (PyCFunction)PyShowTaskDialog, METH_VARARGS | METH_KEYWORDS, NULL },
PyDeclKW("openDialog", PyOpenFileDlg, PyFileDlg_KWLST),
PyDeclKW("saveDialog", PySaveFileDlg, PyFileDlg_KWLST),
PyDeclKW("chooseFolder", PyFolderDlg, PyFolderDlg_KWLST),

// Menus and accelerators management
PyDeclKW("addAccelerator", PyAddAccelerator, Accelerator_KWLST),
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

PyDeclKW("test", test123, test123kw),
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



