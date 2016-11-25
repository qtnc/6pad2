#include "global.h"
#include "Page.h"
#include "strings.hpp"
#include "Thread.h"
#include "python34.h"
#include "accelerators.h"
#include<cstring>
using namespace std;

extern HMENU menu;
extern HWND win;
extern shared_ptr<Page> curPage;

bool ActionCommand (HWND hwnd, int cmd);

struct PyMenuItem { 
    PyObject_HEAD
PyObject* parent;
HMENU menu, submenu;
int cmd;
std::weak_ptr<PageGroup> wpGroup;

optional<tstring> get_group (void) { auto g = wpGroup.lock(); if (g) return g->name; else return none; }
tstring get_name (void);
void set_name (const tstring&);
tstring get_label (void);
void set_label (tstring);
bool get_submenu (void) { return !!submenu; }
bool hasFlag (int flag);
bool get_checked (void) { return hasFlag(MFS_CHECKED); }
void set_checked (bool);
bool get_enabled (void) { return !hasFlag(MFS_DISABLED); }
void set_enabled (bool);
bool get_radio (void);
void set_radio (bool);
tstring get_accelerator (void);
void set_accelerator (const tstring&);
PyFunc<void()> get_action (void);
void set_action (PyFunc<void()> action);
int get_id (void);
UINT getID (void);
int get_length (void);
PyObject* getItem (int n);
PyObject* getItemByName (const tstring&);
PyObject* get_parent (void) { return parent; }
PyObject* addItem (tstring label, OPT, PyFunc<void()> action, optional<int> pos, const tstring& accelerator, const tstring& name, bool isSubmenu, bool isSeparator, bool isSpecific, tstring group);
PyObject* removeItem (OPT, PyObject*, PyObject*);
void remove (void);
};

static constexpr const char* addItem_KWLST[] = {"label", "action", "index", "accelerator", "name", "submenu", "separator", "specific", "group", NULL};
static constexpr const char* removeItem_KWLST[] = { "name", "index", NULL};


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
return ((PyMenuItem*)o) ->get_length();
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

PyObject* PyMenuItem::removeItem (OPT, PyObject* arg, PyObject* arg2) {
if (arg&&arg2) { PyErr_SetString(PyExc_ValueError, "only whether name or index may be specified"); return NULL; }
if (!arg) arg=arg2;
if (arg && !submenu) { PyErr_SetString(PyExc_ValueError, "not a submenu"); return NULL; }
if (arg) {
PyMenuItem* item = (PyMenuItem*)PyMenuItem_MapGetItem((PyObject*)this,arg);
if (!item) return NULL;
item->remove();
return (PyObject*)this;
}
else remove();
Py_RETURN_NONE;
}

static PyMethodDef PyMenuItemMethods[] = {
PyDeclKW("add", PyMenuItem::addItem, addItem_KWLST),
PyDeclKW("remove", &PyMenuItem::removeItem, removeItem_KWLST),
PyDeclEnd
};

#define Prop(x) PyAccessor(#x, &PyMenuItem::get_##x, &PyMenuItem::set_##x)
#define RProp(x) PyReadOnlyAccessor(#x, &PyMenuItem::get_##x)
static PyGetSetDef PyMenuItemAccessors[] = {
Prop(name), Prop(label),
Prop(accelerator), Prop(action), 
Prop(enabled), Prop(checked), Prop(radio),
RProp(submenu), RProp(length),
RProp(id), RProp(parent), RProp(group), 
PyDeclEnd
};
#undef Prop
#undef RProp

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

UINT PyMenuItem::getID () {
if (submenu) return (UINT)submenu;
else return cmd;
}

bool PyMenuItem::hasFlag (int flag) {
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_STATE | MIIM_FTYPE;
RunSync([&]()mutable{
if (!GetMenuItemInfo(menu, getID(), FALSE, &mii)) mii.fState=0;
});//RunSync
return (0!=(mii.fState&flag));
}

void PyMenuItem::set_checked (bool checked) {
if (submenu) return;
checked = checked? MF_CHECKED : MF_UNCHECKED;
RunSync([&]()mutable{
CheckMenuItem(menu, cmd, MF_BYCOMMAND | checked);
});//RunSync
}

void PyMenuItem::set_enabled (bool enabled) {
enabled = enabled? MF_ENABLED :  MF_GRAYED  | MF_DISABLED;
RunSync([&]()mutable{
EnableMenuItem(menu, getID(), MF_BYCOMMAND | enabled);
});//RunSync
}

bool PyMenuItem::get_radio (void) {
if (submenu) return 0;
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_FTYPE;
RunSync([&]()mutable{
if (!GetMenuItemInfo(menu, cmd, FALSE, &mii)) mii.fType=0;
});//RunSync
return 0!=(mii.fType&MFT_RADIOCHECK);
}

void PyMenuItem::set_radio (bool radio) {
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

int PyMenuItem::get_id  (void) { return cmd; }

tstring PyMenuItem::get_name (void) {
tstring re = TEXT("");
RunSync([&]()mutable{
re = GetMenuName(menu, getID(), FALSE);
});//RunSync
return re;
}

void PyMenuItem::set_name (const tstring& name) {
RunSync([&]()mutable{
SetMenuName(menu, getID(), FALSE, name.c_str() );
});//RunSync
}

tstring PyMenuItem::get_label (void) {
tstring re = TEXT("");
RunSync([&]()mutable{
tstring text = GetMenuString(menu, getID(), MF_BYCOMMAND);
int x = text.find_first_of(TEXT("\t\a"));
if (x>=0 && x<text.size()) text = text.substr(0,x);
re = text;
});//RunSync
return re;
}

void PyMenuItem::set_label (tstring label) {
int kflags=0, key=0;
HACCEL& hAccel = sp.hAccel;
FindAccelerator(hAccel, cmd, kflags, key);
if (key) label += TEXT("\t\t") + KeyCodeToName(kflags, key, true);
RunSync([&]()mutable{
if (submenu) ModifyMenu(menu, (UINT)submenu, MF_BYCOMMAND | MF_POPUP, (UINT)submenu, label.c_str() );
else ModifyMenu(menu, cmd, MF_BYCOMMAND | MF_STRING, cmd, label.c_str() );
});//RunSync
}

tstring PyMenuItem::get_accelerator (void) {
int kf=0, key=0;
HACCEL& hAccel = sp.hAccel;
if (!submenu && FindAccelerator(hAccel, cmd, kf, key))return KeyCodeToName(kf,key,false);
else return TEXT("");
}

void PyMenuItem::set_accelerator (const tstring& s) {
int kf=0, key=0;
if (submenu) return;
if (!KeyNameToCode(s, kf, key)) return;
HACCEL& hAccel = sp.hAccel;
RemoveAccelerator(hAccel, cmd);
AddAccelerator(hAccel, kf, key, cmd);
set_label(get_label());
}

PyFunc<void()> PyMenuItem::get_action (void) {
if (submenu) return Py_None;
auto uf = findUserCommand(cmd);
if (uf.pyFunc) return uf.pyFunc;
else return Py_None;
}

void PyMenuItem::set_action (PyFunc<void()> action) {
if (submenu) return;
if (!action) RemoveUserCommand(cmd);
else AddUserCommand( action, cmd);
}

int PyMenuItem::get_length (void) {
if (submenu) return  GetMenuItemCount(submenu);
else return -1;
}

PyObject* PyMenuItem::getItemByName (const tstring& name) {
PyObject* re = NULL;
RunSync([&]()mutable{
if (submenu) for (int i=0, l=get_length(); i<l; i++) {
if (GetMenuName(submenu, i, TRUE)==name) { re = getItem(i); return; }
}
});//RunSync
if (re) return re;
else Py_RETURN_NONE;
}

PyObject* PyMenuItem::getItem (int n) {
if (!submenu) { Py_RETURN_NONE; }
PyObject* re = NULL;
RunSync([&]()mutable{
auto group = wpGroup.lock();
int len = get_length();
if (n<0) n+=len;
if (n>=len) return;
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_ID | MIIM_SUBMENU | MIIM_DATA;
if (!GetMenuItemInfo(submenu, n, TRUE, &mii)) return;
PyMenuItem* it = PyMenuItemNew(&PyMenuItemType, NULL, NULL);
it->parent = (PyObject*)this;
it->menu = submenu;
it->submenu = mii.hSubMenu;
it->cmd = mii.wID;
if (group) it->wpGroup = group;
else it->wpGroup = PageGroup::FindGroupContainingMenu(submenu, mii.wID);
re = (PyObject*)it;
});//RunSync
if (re) Py_XINCREF((PyObject*)this);
return re;
}

PyObject* PyMenuItem_GetMenuBar (void) {
PyMenuItem* it = PyMenuItemNew(&PyMenuItemType, NULL, NULL);
it->parent = NULL;
it->menu = NULL;
it->submenu = menu;
it->cmd = 0;
return (PyObject*)it;
}

void PyMenuItem::remove (void) {
RunSync([&]()mutable{
auto group = wpGroup.lock();
HACCEL& hAccel = group? group->accel : sp.hAccel;
SetMenuName(menu, getID(), FALSE, NULL);
DeleteMenu(menu, getID(), MF_BYCOMMAND);
if (group) group->RemoveMenu(menu, getID());
if (cmd>=IDM_USER_COMMAND && cmd<0xF000 && !submenu) {
RemoveUserCommand(cmd);
RemoveAccelerator(hAccel, cmd);
}
DrawMenuBar(win);
});//RunSync
}

PyObject* PyMenuItem::addItem (tstring  label, OPT, PyFunc<void()> action, optional<int> optPos, const tstring& accelerator, const tstring& name, bool isSubmenu, bool isSeparator, bool isSpecific, tstring groupName) {
if (!submenu) { Py_RETURN_NONE; }
auto group = wpGroup.lock();
if (isSpecific && curPage) groupName = tsnprintf(32, TEXT("Page@%p"), curPage.get());
if (!groupName.empty()) group = PageGroup::getGroup(groupName);
if (group && !name.empty()) {
auto item = group->ContainsMenu(name);
printf("group=%ls/%ls, name=%ls, re=%p\n", groupName.c_str(), group->name.c_str(), name.c_str(), item);
if (item) {
PyObject* re = NULL;
RunSync([&]()mutable{ 
if (curPage) curPage->AddPageGroup(group);
re = getItem(item->pos);
});//RunSync
return re;
}}
HACCEL& hAccel = group? group->accel : sp.hAccel;
int pos = optPos? *optPos : -1;
int cmd = pos+1, kf=0, key= 0;
if (action && !isSeparator && !isSubmenu) cmd = AddUserCommand(action);
if (cmd && accelerator.size()>0 && KeyNameToCode(accelerator, kf, key)) AddAccelerator(hAccel, kf, key, cmd);
if (key) label += TEXT("\t\t") + KeyCodeToName(kf, key, true);
PyObject* re = NULL;
RunSync([&]()mutable{
if (pos<0) pos += 1 + GetMenuItemCount(submenu);
HMENU hSub = isSubmenu? CreateMenu() : NULL;
if (group && curPage) curPage->AddPageGroup(group);
if (isSubmenu) InsertMenu(submenu, pos, MF_STRING | MF_BYPOSITION | MF_POPUP, (UINT)hSub, label.c_str() );
else if (isSeparator) InsertMenu(submenu, pos, MF_SEPARATOR | MF_BYPOSITION, 0xEFFF, TEXT(""));
else InsertMenu(submenu, pos, MF_STRING | MF_BYPOSITION, cmd, label.c_str() );
if (name.size()>0) SetMenuName(submenu, pos, TRUE, name.c_str());
if (group) group->AddMenu(name, submenu, hSub?(UINT)hSub:cmd, pos, hSub?MF_POPUP:MF_STRING);
DrawMenuBar(win);
re = getItem(pos);
});//RunSync
return re;
}

int ShowContextMenu (const vector<tstring>& items) {
POINT p;
HMENU menu = CreatePopupMenu();
int itemID = 0;
for (const tstring& item: items) AppendMenu(menu, MF_STRING, ++itemID, item.c_str() );
GetCursorPos(&p);
itemID = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, p.x, p.y, 0, GetForegroundWindow(), NULL) -1;
DestroyMenu(menu);
return itemID;
}

int PyShowPopupMenu (const vector<tstring>& items) {
int result = -1;
Py_BEGIN_ALLOW_THREADS
RunSync([&]()mutable{
result = ShowContextMenu(items);
});//RunSync
Py_END_ALLOW_THREADS
return result;
}
