#include "global.h"
#include "strings.hpp"
#include "Thread.h"
#include "python34.h"
#include<cstring>
using namespace std;

extern HMENU menu;
extern HWND win;

int AddUserCommand (std::function<void(void)> f);
bool AddAccelerator (int flags, int key, int cmd);
bool FindAccelerator (int cmd, int& flags, int& key);
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
int getItemCount (void);
PyObject* getItem (int n);
PyObject* getItemByName (const tstring&);
PyObject* getParent (void) { return parent; }
PyObject* addItem (tstring label, PyCallback action, const tstring& accelerator, const tstring& name, int pos);
};

static void PyMenuItemDealloc (PyObject* pySelf) {
PyMenuItem* self = (PyMenuItem*)pySelf;
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
else { Py_RETURN_NONE; }
}

static PyObject* PyMenuItem_AddItem (PyObject* o, PyObject* args, PyObject* dic) {
static const char* KWLST[] = {"action", "label", "index", "accelerator", "name", NULL};
PyMenuItem& self = *(PyMenuItem*)o;
const wchar_t *label=0, *accelerator=0, *name=0;
int index=-1, length = self.getItemCount();
PyObject* action = NULL;
if (!self.submenu) { PyErr_SetString(PyExc_ValueError, "not a submenu"); return NULL; }
if (!PyArg_ParseTupleAndKeywords(args, dic, "Ou|iuu", (char**)KWLST, &action, &label, &index, &accelerator, &name)) return NULL;
if (!PyCallable_Check(action)) { PyErr_SetString(PyExc_ValueError, "action must be callable"); return NULL; }
if (index<0) index+=length+1;
if (index>length) { PyErr_SetString(PyExc_ValueError, "index out of range"); return NULL; }
return self.addItem(label, action, accelerator?accelerator:TEXT(""), name?name:TEXT(""), index);
}

static PyMethodDef PyMenuItemMethods[] = {
{"add", (PyCFunction)PyMenuItem_AddItem, METH_VARARGS | METH_KEYWORDS, NULL},
PyDeclEnd
};

static PyGetSetDef PyMenuItemAccessors[] = {
PyAccessor("name", &PyMenuItem::getName, &PyMenuItem::setName),
PyAccessor("label", &PyMenuItem::getLabel, &PyMenuItem::setLabel),
PyAccessor("enabled", &PyMenuItem::isEnabled, &PyMenuItem::setEnabled),
PyAccessor("checked", &PyMenuItem::isChecked, &PyMenuItem::setChecked),
PyAccessor("radio", &PyMenuItem::isRadio, &PyMenuItem::setRadio),
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
    "window.MenuItem",             /* tp_name */ 
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
if (!GetMenuItemInfo(menu, pos, TRUE, &mii)) return 0;
return (0!=(mii.fState&flag));
}

void PyMenuItem::setChecked (int checked) {
if (submenu) return;
checked = checked? MF_CHECKED : MF_UNCHECKED;
CheckMenuItem(menu, pos, MF_BYPOSITION | checked);
}

void PyMenuItem::setEnabled (int enabled) {
enabled = enabled? MF_ENABLED : MF_DISABLED;
EnableMenuItem(menu, pos, MF_BYPOSITION | enabled);
}

int PyMenuItem::isRadio (void) {
if (submenu) return 0;
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_FTYPE;
if (!GetMenuItemInfo(menu, cmd, FALSE, &mii)) return 0;
return 0!=(mii.fType&MFT_RADIOCHECK);
}

void PyMenuItem::setRadio (int radio) {
if (submenu) return;
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_FTYPE;
if (!GetMenuItemInfo(menu, cmd, FALSE, &mii)) return;
if (radio) mii.fType|=MFT_RADIOCHECK;
else mii.fType&=~MFT_RADIOCHECK;
SetMenuItemInfo(menu, cmd, FALSE, &mii);
}

tstring GetMenuName (HMENU menu, int pos) {
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_DATA;
if (!GetMenuItemInfo(menu, pos, TRUE, &mii)) return TEXT("");
const TCHAR* re = (const TCHAR*)(mii.dwItemData);
if (re) return tstring(re);
else return TEXT("");
}

void SetMenuName (HMENU menu, int pos, const TCHAR* name) {
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_DATA;
if (!GetMenuItemInfo(menu, pos, TRUE, &mii)) return;
if (mii.dwItemData) free((void*)(mii.dwItemData));
mii.dwItemData = (ULONG_PTR)tstrdup(name);
SetMenuItemInfo(menu, pos, TRUE, &mii);
}

tstring PyMenuItem::getName (void) {
return GetMenuName(menu, pos);
}

void PyMenuItem::setName (const tstring& name) {
SetMenuName(menu, pos, name.c_str() );
}

tstring PyMenuItem::getLabel (void) {
int len = GetMenuString(menu, pos, NULL, 0, MF_BYPOSITION);
tstring text = tstring(len,(TCHAR)0);
GetMenuString(menu, pos, (TCHAR*)(text.data()), len+1, MF_BYPOSITION);
int x = text.find_first_of(TEXT("\t\a"));
if (x>=0 && x<text.size()) text = text.substr(0,x);
return text;
}

void PyMenuItem::setLabel (tstring label) {
int kflags=0, key=0;
FindAccelerator(cmd, kflags, key);
if (key) label += TEXT("\t\t") + KeyCodeToName(kflags, key, true);
if (submenu) ModifyMenu(menu, (UINT)submenu, MF_BYCOMMAND | MF_POPUP, (UINT)submenu, label.c_str() );
else ModifyMenu(menu, cmd, MF_BYCOMMAND | MF_STRING, cmd, label.c_str() );
}

int PyMenuItem::getItemCount (void) {
if (submenu) return  GetMenuItemCount(submenu);
else return -1;
}

PyObject* PyMenuItem::getItemByName (const tstring& name) {
if (submenu) for (int i=0, l=getItemCount(); i<l; i++) {
if (GetMenuName(submenu, i)==name) return getItem(i);
}
Py_RETURN_NONE;
}

PyObject* PyMenuItem::getItem (int n) {
if (!submenu) { Py_RETURN_NONE; }
int len = getItemCount();
if (n<0) n+=len;
if (n>=len) { Py_RETURN_NONE; }
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_ID | MIIM_SUBMENU;
if (!GetMenuItemInfo(submenu, n, TRUE, &mii)) return NULL;
PyMenuItem* it = PyMenuItemNew(&PyMenuItemType, NULL, NULL);
Py_XINCREF((PyObject*)this);
it->parent = (PyObject*)this;
it->menu = submenu;
it->pos = n;
it->submenu = mii.hSubMenu;
it->cmd = mii.wID;
it->popup = false;
return (PyObject*)it;
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

PyObject* PyMenuItem::addItem (tstring  label, PyCallback action, const tstring& accelerator, const tstring& name, int pos) {
if (!submenu) { Py_RETURN_NONE; }
int cmd = AddUserCommand([=]()mutable{ action(); }), kf=0, key=0;
if (accelerator.size()>0 && KeyNameToCode(accelerator, kf, key)) AddAccelerator(kf, key, cmd);
if (key) label += TEXT("\t\t") + KeyCodeToName(kf, key, true);
InsertMenu(submenu, pos, MF_STRING | MF_BYPOSITION, cmd, label.c_str() );
if (name.size()>0) SetMenuName(submenu, pos, name.c_str());
DrawMenuBar(win);
return getItem(pos);
}

