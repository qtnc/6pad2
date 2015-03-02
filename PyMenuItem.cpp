#include "global.h"
#include "strings.hpp"
#include "page.h"
#include "python34.h"
#include<cstring>
using namespace std;

struct PyMenuItem { 
    PyObject_HEAD
HMENU menu, submenu;
int pos, cmd;

string getName (void);
void setName (const string&);
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
};

static void PyMenuItemDealloc (PyObject* pySelf) {
PyMenuItem* self = (PyMenuItem*)pySelf;
Py_TYPE(pySelf)->tp_free(pySelf);
}

static PyMenuItem* PyMenuItemNew (PyTypeObject* type, PyObject* args, PyObject* kwds) {
PyMenuItem* self = (PyMenuItem*)(type->tp_alloc(type, 0));
return self;
}

static int PyMenuItemInit (PyMenuItem* self, PyObject* args, PyObject* kwds) {
return 0;
}

static PyMethodDef PyMenuItemMethods[] = {
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
PyDeclEnd
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

bool FindAccelerator (int cmd, int& flags, int& key);
tstring KeyCodeToName (int flags, int vk, bool i18n); 

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

string PyMenuItem::getName (void) {
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_DATA;
if (!GetMenuItemInfo(menu, pos, TRUE, &mii)) return "";
const char* re = (const char*)(mii.dwItemData);
if (re) return string(re);
else return "";
}

void SetMenuName (HMENU menu, int pos, const char* name) {
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_DATA;
if (!GetMenuItemInfo(menu, pos, TRUE, &mii)) return;
if (mii.dwItemData) free((void*)(mii.dwItemData));
mii.dwItemData = (ULONG_PTR)strdup(name);
SetMenuItemInfo(menu, pos, TRUE, &mii);
}

void PyMenuItem::setName (const string& name) {
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

PyObject* PyMenuItem::getItem (int n) {
if (!submenu) return NULL;
int len = getItemCount();
if (n<0) n+=len;
if (n>=len) return NULL;
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_ID | MIIM_SUBMENU;
if (!GetMenuItemInfo(submenu, n, TRUE, &mii)) return NULL;
PyMenuItem* it = PyMenuItemNew(&PyMenuItemType, NULL, NULL);
//it->parent = menu;
it->menu = submenu;
it->pos = n;
it->submenu = mii.hSubMenu;
it->cmd = mii.wID;
return (PyObject*)it;
}



