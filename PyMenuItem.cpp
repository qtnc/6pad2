#include "global.h"
#include "strings.hpp"
#include "Thread.h"
#include "python34.h"
#include<cstring>
using namespace std;

extern HMENU menu;
extern HWND win;

int AddUserCommand (std::function<void(void)> f, int cmd = 0);
bool RemoveUserCommand (int cmd);
bool AddAccelerator (int flags, int key, int cmd);
bool FindAccelerator (int cmd, int& flags, int& key);
BOOL RemoveAccelerator (int cmd);
tstring KeyCodeToName (int flags, int vk, bool i18n); 
bool KeyNameToCode (const tstring& kn, int& flags, int& key);
bool ActionCommand (HWND hwnd, int cmd);

struct PyMenuItem { 
    PyObject_HEAD
PyObject* parent;
HMENU menu, submenu;
int pos, cmd;
bool popup;

tstring getName (void);
void setName (const tstring&);
tstring getLabel (void);
void setLabel (tstring);
int isSubMenu (void) { return !!submenu; }
int hasFlag (int flag);
int isChecked (void) { return hasFlag(MFS_CHECKED); }
void setChecked (int);
int isEnabled (void) { return !hasFlag(MFS_DISABLED); }
void setEnabled (int);
int isRadio (void);
void setRadio (int);
tstring getAccelerator (void);
void setAccelerator (const tstring&);
int getCmd (void);
int getItemCount (void);
PyObject* getItem (int n);
PyObject* getItemByName (const tstring&);
PyObject* getParent (void) { return parent; }
PyObject* addItem (tstring label, PyCallback action, const tstring& accelerator, const tstring& name, int pos, int isSubmenu, int isSeparator);
void remove (void);
int show (void);
};

static void PyMenuItemDealloc (PyObject* pySelf) {
PyMenuItem* self = (PyMenuItem*)pySelf;
if (self->popup && self->submenu) {
RunSync([&]()mutable{ DestroyMenu(self->submenu); });
printf("Destroyed menu @%p\r\n", self);
}
Py_XDECREF(self->parent);
Py_TYPE(pySelf)->tp_free(pySelf);
}

static PyMenuItem* PyMenuItemNew (PyTypeObject* type, PyObject* args, PyObject* kwds) {
PyMenuItem* self = (PyMenuItem*)(type->tp_alloc(type, 0));
return self;
}

static int PyMenuItemInit (PyMenuItem* self, PyObject* args, PyObject* kwds) {
return 0;
}

static int PyMenuItem_MapLen (PyObject * o) {
return ((PyMenuItem*)o) ->getItemCount();
}

static PyObject* PyMenuItem_CallAction (PyObject* o, PyObject* args, PyObject* kwds) {
PyMenuItem* self = (PyMenuItem*)o;
ActionCommand(win, self->cmd);
Py_RETURN_NONE;
}

static PyObject* PyMenuItem_MapGetItem (PyObject* o, PyObject* key) {
PyObject* re = PyObject_GenericGetAttr(o,key);
if (re) return re;
PyErr_Clear();
PyMenuItem* self = (PyMenuItem*)o;
if (PyLong_Check(key)) {
return self->getItem(PyLong_AsLong(key));
}
else if (PyUnicode_Check(key)) {
tstring name = toTString(PyUnicode_AsUnicode(key));
return self->getItemByName(name);
}
else {
PyErr_SetString(PyExc_TypeError, "key must be int or str");
return NULL;
}}

static PyObject* PyMenuItem_AddItem (PyObject* o, PyObject* args, PyObject* dic) {
static const char* KWLST[] = {"label", "action", "index", "accelerator", "name", "submenu", "separator", NULL};
PyMenuItem& self = *(PyMenuItem*)o;
const wchar_t *label=0, *accelerator=0, *name=0;
int setsub=0, setsep=0, index=-1, length = self.getItemCount();
PyObject* action = NULL;
if (!self.submenu) { PyErr_SetString(PyExc_ValueError, "not a submenu"); return NULL; }
if (!PyArg_ParseTupleAndKeywords(args, dic, "|uOiuuii", (char**)KWLST, &label, &action, &index, &accelerator, &name, &setsub, &setsep)) return NULL;
if (action && action!=Py_None && !setsub && !PyCallable_Check(action)) { PyErr_SetString(PyExc_ValueError, "action must be callable"); return NULL; }
if (index<0) index+=length+1;
if (index>length) { PyErr_SetString(PyExc_ValueError, "index out of range"); return NULL; }
return self.addItem(toTString(label), action, toTString(accelerator), toTString(name), index, setsub, setsep);
}

static PyObject* PyMenuItem_RemItem (PyObject* o, PyObject* args, PyObject* dic) {
static const char* KWLST[] = { "name", "index", NULL};
PyMenuItem& self = *(PyMenuItem*)o;
PyObject* arg = NULL, *arg2=NULL;
if (!PyArg_ParseTupleAndKeywords(args, dic, "|OO", (char**)KWLST, &arg, &arg2)) return NULL;
if (arg&&arg2) { PyErr_SetString(PyExc_ValueError, "only whether name or index may be specified"); return NULL; }
if (!arg) arg=arg2;
if (arg && !self.submenu) { PyErr_SetString(PyExc_ValueError, "not a submenu"); return NULL; }
if (arg) {
PyMenuItem* item = (PyMenuItem*)PyMenuItem_MapGetItem(o,arg);
if (!item) return NULL;
item->remove();
}
else self.remove();
Py_RETURN_NONE;
}

static PyMethodDef PyMenuItemMethods[] = {
{"add", (PyCFunction)PyMenuItem_AddItem, METH_VARARGS | METH_KEYWORDS, NULL},
{"remove", (PyCFunction)PyMenuItem_RemItem, METH_VARARGS | METH_KEYWORDS, NULL},
PyDecl("show", &PyMenuItem::show),
PyDeclEnd
};

static PyGetSetDef PyMenuItemAccessors[] = {
PyAccessor("name", &PyMenuItem::getName, &PyMenuItem::setName),
PyAccessor("label", &PyMenuItem::getLabel, &PyMenuItem::setLabel),
PyAccessor("accelerator", &PyMenuItem::getAccelerator, &PyMenuItem::setAccelerator),
PyAccessor("enabled", &PyMenuItem::isEnabled, &PyMenuItem::setEnabled),
PyAccessor("checked", &PyMenuItem::isChecked, &PyMenuItem::setChecked),
PyAccessor("radio", &PyMenuItem::isRadio, &PyMenuItem::setRadio),
PyReadOnlyAccessor("id", &PyMenuItem::getCmd),
PyReadOnlyAccessor("length", &PyMenuItem::getItemCount),
PyReadOnlyAccessor("submenu", &PyMenuItem::isSubMenu),
PyReadOnlyAccessor("parent", &PyMenuItem::getParent),
PyDeclEnd
};

static PyMappingMethods PyMenuItemMapping {
PyMenuItem_MapLen,
PyMenuItem_MapGetItem,
NULL
};

static PyTypeObject PyMenuItemType = { 
    PyVarObject_HEAD_INIT(NULL, 0) 
    "sixpad.MenuItem",             /* tp_name */ 
    sizeof(PyMenuItem), /* tp_basicsize */ 
    0,                         /* tp_itemsize */ 
    PyMenuItemDealloc,                         /* tp_dealloc */ 
    0,                         /* tp_print */ 
    0,                         /* tp_getattr */ 
    0,                         /* tp_setattr */ 
    0,                         /* tp_reserved */ 
    0,                         /* tp_repr */ 
    0,                         /* tp_as_number */ 
    0,                         /* tp_as_sequence */ 
  &PyMenuItemMapping,                         /* tp_as_mapping */ 
    0,                         /* tp_hash  */ 
    PyMenuItem_CallAction,                         /* tp_call */ 
    0,                         /* tp_str */ 
    PyMenuItem_MapGetItem,                         /* tp_getattro */ 
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
    PyMenuItemMethods,             /* tp_methods */ 
NULL,             /* tp_members */ 
    PyMenuItemAccessors,                         /* tp_getset */ 
    0,                         /* tp_base */ 
    0,                         /* tp_dict */ 
    0,                         /* tp_descr_get */ 
    0,                         /* tp_descr_set */ 
    0,                         /* tp_dictoffset */ 
    (initproc)PyMenuItemInit,      /* tp_init */ 
    0,                         /* tp_alloc */ 
};

bool PyRegister_MenuItem (PyObject* m) {
//PyMenuItemType.tp_new = (decltype(PyMenuItemType.tp_new))PyMenuItemNew;
if (PyType_Ready(&PyMenuItemType) < 0)          return false;
Py_INCREF(&PyMenuItemType); 
PyModule_AddObject(m, "MenuItem", (PyObject*)&PyMenuItemType); 
return true;
}

int PyMenuItem::hasFlag (int flag) {
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_STATE | MIIM_FTYPE;
RunSync([&]()mutable{
if (!GetMenuItemInfo(menu, pos, TRUE, &mii)) mii.fState=0;
});//RunSync
return (0!=(mii.fState&flag));
}

void PyMenuItem::setChecked (int checked) {
if (submenu) return;
checked = checked? MF_CHECKED : MF_UNCHECKED;
RunSync([&]()mutable{
CheckMenuItem(menu, pos, MF_BYPOSITION | checked);
});//RunSync
}

void PyMenuItem::setEnabled (int enabled) {
enabled = enabled? MF_ENABLED | MF_GRAYED  : MF_DISABLED;
RunSync([&]()mutable{
EnableMenuItem(menu, pos, MF_BYPOSITION | enabled);
});//RunSync
}

int PyMenuItem::isRadio (void) {
if (submenu) return 0;
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_FTYPE;
RunSync([&]()mutable{
if (!GetMenuItemInfo(menu, cmd, FALSE, &mii)) mii.fType=0;
});//RunSync
return 0!=(mii.fType&MFT_RADIOCHECK);
}

void PyMenuItem::setRadio (int radio) {
if (submenu) return;
RunSync([&]()mutable{
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_FTYPE;
if (!GetMenuItemInfo(menu, cmd, FALSE, &mii)) return;
if (radio) mii.fType|=MFT_RADIOCHECK;
else mii.fType&=~MFT_RADIOCHECK;
SetMenuItemInfo(menu, cmd, FALSE, &mii);
});//RunSync
}

int PyMenuItem::getCmd (void) { return cmd; }

tstring GetMenuName (HMENU menu, int pos) {
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_DATA;
if (!GetMenuItemInfo(menu, pos, TRUE, &mii)) return TEXT("");
const TCHAR* re = (const TCHAR*)(mii.dwItemData);
if (re) return tstring(re);
else return TEXT("");
}

void SetMenuName (HMENU menu, int pos, BOOL bypos, LPCTSTR name) {
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_DATA;
if (!GetMenuItemInfo(menu, pos, bypos, &mii)) return;
if (mii.dwItemData) free((void*)(mii.dwItemData));
mii.dwItemData = name? (ULONG_PTR)tstrdup(name) :NULL;
SetMenuItemInfo(menu, pos, bypos, &mii);
}

tstring PyMenuItem::getName (void) {
tstring re = TEXT("");
RunSync([&]()mutable{
re = GetMenuName(menu, pos);
});//RunSync
return re;
}

void PyMenuItem::setName (const tstring& name) {
RunSync([&]()mutable{
SetMenuName(menu, pos, TRUE, name.c_str() );
});//RunSync
}

tstring PyMenuItem::getLabel (void) {
tstring re = TEXT("");
RunSync([&]()mutable{
int len = GetMenuString(menu, pos, NULL, 0, MF_BYPOSITION);
tstring text = tstring(len,(TCHAR)0);
GetMenuString(menu, pos, (TCHAR*)(text.data()), len+1, MF_BYPOSITION);
int x = text.find_first_of(TEXT("\t\a"));
if (x>=0 && x<text.size()) text = text.substr(0,x);
re = text;
});//RunSync
return re;
}

void PyMenuItem::setLabel (tstring label) {
int kflags=0, key=0;
FindAccelerator(cmd, kflags, key);
if (key) label += TEXT("\t\t") + KeyCodeToName(kflags, key, true);
RunSync([&]()mutable{
if (submenu) ModifyMenu(menu, (UINT)submenu, MF_BYCOMMAND | MF_POPUP, (UINT)submenu, label.c_str() );
else ModifyMenu(menu, cmd, MF_BYCOMMAND | MF_STRING, cmd, label.c_str() );
});//RunSync
}

tstring PyMenuItem::getAccelerator (void) {
int kf=0, key=0;
if (!submenu && FindAccelerator(cmd, kf, key))return KeyCodeToName(kf,key,false);
else return TEXT("");
}

void PyMenuItem::setAccelerator (const tstring& s) {
int kf=0, key=0;
if (submenu) return;
if (!KeyNameToCode(s, kf, key)) return;
RemoveAccelerator(cmd);
AddAccelerator(kf, key, cmd);
setLabel(getLabel());
}

int PyMenuItem::getItemCount (void) {
if (submenu) return  GetMenuItemCount(submenu);
else return -1;
}

PyObject* PyMenuItem::getItemByName (const tstring& name) {
PyObject* re = NULL;
RunSync([&]()mutable{
if (submenu) for (int i=0, l=getItemCount(); i<l; i++) {
if (GetMenuName(submenu, i)==name) { re = getItem(i); return; }
}
});//RunSync
if (re) return re;
else Py_RETURN_NONE;
}

PyObject* PyMenuItem::getItem (int n) {
if (!submenu) { Py_RETURN_NONE; }
PyObject* re = NULL;
RunSync([&]()mutable{
int len = getItemCount();
if (n<0) n+=len;
if (n>=len) return;
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_ID | MIIM_SUBMENU;
if (!GetMenuItemInfo(submenu, n, TRUE, &mii)) return;
PyMenuItem* it = PyMenuItemNew(&PyMenuItemType, NULL, NULL);
it->parent = (PyObject*)this;
it->menu = submenu;
it->pos = n;
it->submenu = mii.hSubMenu;
it->cmd = mii.wID;
it->popup = false;
re = (PyObject*)it;
});//RunSync
if (re) Py_XINCREF((PyObject*)this);
return re;
}

PyObject* PyMenuItem_GetMenuBar (void) {
PyMenuItem* it = PyMenuItemNew(&PyMenuItemType, NULL, NULL);
it->parent = NULL;
it->menu = NULL;
it->pos = 0;
it->submenu = menu;
it->cmd = 0;
it->popup = false;
return (PyObject*)it;
}

PyObject* PyMenuItem_CreatePopupMenu (void) {
HMENU menu = NULL;
RunSync([&]()mutable{
menu = CreatePopupMenu();
});//RunSync
if (!menu) return NULL;
PyMenuItem* it = PyMenuItemNew(&PyMenuItemType, NULL, NULL);
it->parent = NULL;
it->menu = NULL;
it->pos = 0;
it->submenu = menu;
it->cmd = 0;
it->popup = true;
return (PyObject*)it;
}

void PyMenuItem::remove (void) {
RunSync([&]()mutable{
SetMenuName(menu, pos, true, NULL);
DeleteMenu(menu, pos, MF_BYPOSITION);
DrawMenuBar(win);
if (cmd>=IDM_USER_COMMAND && cmd<0xF000 && !submenu) {
RemoveUserCommand(cmd);
RemoveAccelerator(cmd);
}
});//RunSync
}

int PyMenuItem::show (void) {
if (!submenu || !popup) return 0;
int re = -1;
Py_BEGIN_ALLOW_THREADS
RunSync([&]()mutable{
POINT p;
GetCursorPos(&p);
re = TrackPopupMenu(submenu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, p.x, p.y, 0, GetForegroundWindow(), NULL);
});//RunSync
Py_END_ALLOW_THREADS
return re;
}

PyObject* PyMenuItem::addItem (tstring  label, PyCallback action, const tstring& accelerator, const tstring& name, int pos, int isSubmenu, int isSeparator) {
if (!submenu) { Py_RETURN_NONE; }
int cmd = pos+1, kf=0, key= 0;
if (action && !isSeparator && !isSubmenu) cmd = AddUserCommand([=]()mutable{  action(); });
if (cmd && accelerator.size()>0 && KeyNameToCode(accelerator, kf, key)) AddAccelerator(kf, key, cmd);
if (key) label += TEXT("\t\t") + KeyCodeToName(kf, key, true);
PyObject* re = NULL;
RunSync([&]()mutable{
HMENU hSub = isSubmenu? CreateMenu() : NULL;
if (isSubmenu) InsertMenu(submenu, pos, MF_STRING | MF_BYPOSITION | MF_POPUP, (UINT)hSub, label.c_str() );
else if (isSeparator) InsertMenu(submenu, pos, MF_SEPARATOR | MF_BYPOSITION, 0xEFFF, TEXT(""));
else InsertMenu(submenu, pos, MF_STRING | MF_BYPOSITION, cmd, label.c_str() );
if (name.size()>0) SetMenuName(submenu, pos, TRUE, name.c_str());
DrawMenuBar(win);
re = getItem(pos);
});//RunSync
return re;
}

