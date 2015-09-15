#include "PyTreeViewItem.h"
using namespace std;

static void PyTreeViewItemDealloc (PyObject* pySelf) {
PyTreeViewItem* self = (PyTreeViewItem*)pySelf;
Py_TYPE(pySelf)->tp_free(pySelf);
}

static int PyTreeViewItemInit (PyTreeViewItem* self, PyObject* args, PyObject* kwds) {
return 0;
}

#define M(x) PyDecl(#x, &PyTreeViewItem::x)
static PyMethodDef PyTreeViewItemMethods[] = {
M(appendChild), M(insertBefore), M(removeChild),
PyDecl("delete", &PyTreeViewItem::remove),
PyDeclEnd
};
#undef M

#define Prop(x) PyAccessor(#x, &PyTreeViewItem::get_##x, &PyTreeViewItem::set_##x)
#define RProp(x) PyReadOnlyAccessor(#x, &PyTreeViewItem::get_##x)
static PyGetSetDef PyTreeViewItemAccessors[] = {
Prop(text), Prop(value),
Prop(expanded),
RProp(parent), RProp(nextSibling), RProp(previousSibling), RProp(firstChild), RProp(children),
PyDeclEnd
};
#undef Prop
#undef RProp

static PyTypeObject PyTreeViewItemType = { 
    PyVarObject_HEAD_INIT(NULL, 0) 
    "qc6paddlgs.TreeViewItem",             /* tp_name */ 
    sizeof(PyTreeViewItem), /* tp_basicsize */ 
    0,                         /* tp_itemsize */ 
    PyTreeViewItemDealloc,                         /* tp_dealloc */ 
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
    PyTreeViewItemMethods,             /* tp_methods */ 
NULL,             /* tp_members */ 
    PyTreeViewItemAccessors,                         /* tp_getset */ 
    0,                         /* tp_base */ 
    0,                         /* tp_dict */ 
    0,                         /* tp_descr_get */ 
    0,                         /* tp_descr_set */ 
    0,                         /* tp_dictoffset */ 
    (initproc)PyTreeViewItemInit,      /* tp_init */ 
    0,                         /* tp_alloc */ 
}; 

PyTreeViewItem* PyTreeViewItem::New (HWND ht, HTREEITEM hItem) {
GIL_PROTECT
PyTreeViewItem* it = (PyTreeViewItem*) PyTreeViewItemType.tp_alloc(&PyTreeViewItemType,0);
it->hTree = ht;
it->item = hItem;
return it;
}

void PyTreeViewItem::Delete () {
GIL_PROTECT
Py_XDECREF( (PyObject*)this );
}

bool PyRegister_TreeViewItem (PyObject* m) {
//PyTreeViewItemType.tp_new = (decltype(PyTreeViewItemType.tp_new))PyTreeViewItemNew;
if (PyType_Ready(&PyTreeViewItemType) < 0)          return false;
Py_INCREF(&PyTreeViewItemType); 
PyModule_AddObject(m, "TreeViewItem", (PyObject*)&PyTreeViewItemType); 
return true;
}

PyObject* PyTreeViewItem::addChild (const tstring& text, HTREEITEM hAfter, PyObject* udata, UINT state) {
TVINSERTSTRUCT ins;
ins.hParent = item;
ins.hInsertAfter = hAfter;
ins.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
ins.item.hItem = NULL;
ins.item.state =  state;
ins.item.stateMask = TVIS_EXPANDED | TVIS_SELECTED;
ins.item.pszText = (LPTSTR)text.c_str();
ins.item.cchTextMax = text.size();
ins.item.lParam = (LPARAM)udata;
HTREEITEM newItem = (HTREEITEM)SendMessage(hTree, TVM_INSERTITEM, 0, &ins);
Py_XINCREF(udata);
return (PyObject*) PyTreeViewItem::New(hTree, newItem);
}

PyObject* PyTreeViewItem::appendChild (const tstring& text, OPT, PyObject* value) {
return addChild(text, TVI_LAST, value);
}

PyObject* PyTreeViewItem::insertBefore (const tstring& text, PyObject* child, OPT, PyObject* value) {
if (!child || child==Py_None) return appendChild(text, 0, value);
HTREEITEM beforeItem = ((PyTreeViewItem*)child) ->item;
HTREEITEM afterItem = (HTREEITEM)SendMessage(hTree, TVM_GETNEXTITEM, TVGN_PREVIOUS, beforeItem);
if (!afterItem) afterItem = TVI_FIRST;
return addChild(text, afterItem, value);
}

void PyTreeViewItem::remove () {
SendMessage(hTree, TVM_DELETEITEM, 0, item);
}

void PyTreeViewItem::removeChild (PyObject* o) {
((PyTreeViewItem*)o)->remove();
}

PyObject* PyTreeViewItem::pyobj (HTREEITEM it) {
if (it) return (PyObject*) PyTreeViewItem::New(hTree, it);
else { Py_RETURN_NONE; }
}

PyObject* PyTreeViewItem::get_parent () {
return pyobj((HTREEITEM)SendMessage(hTree, TVM_GETNEXTITEM, TVGN_PARENT, item));
}

PyObject* PyTreeViewItem::get_firstChild () {
return pyobj((HTREEITEM)SendMessage(hTree, TVM_GETNEXTITEM, TVGN_CHILD, item));
}

PyObject* PyTreeViewItem::get_nextSibling () {
return pyobj((HTREEITEM)SendMessage(hTree, TVM_GETNEXTITEM, TVGN_NEXT, item));
}

PyObject* PyTreeViewItem::get_previousSibling  () {
return pyobj((HTREEITEM)SendMessage(hTree, TVM_GETNEXTITEM, TVGN_PREVIOUS, item));
}

PyObject* PyTreeViewItem::get_children () {
PyObject* list = PyList_New(0);
HTREEITEM cur = (HTREEITEM)SendMessage(hTree, TVM_GETNEXTITEM, TVGN_CHILD, item);
while(cur){
PyList_Append(list, pyobj(cur));
cur = (HTREEITEM)SendMessage(hTree, TVM_GETNEXTITEM, TVGN_NEXT, cur);
}
return list;
}

void PyTreeViewItem::set_text (const tstring& s) {
TVITEM it;
it.hItem = item;
it.mask = TVIF_TEXT;
it.pszText = (LPTSTR)s.c_str();
it.cchTextMax = s.size();
SendMessage(hTree, TVM_SETITEM, 0, &it);
}

tstring PyTreeViewItem::get_text () {
TCHAR buf[512] = {0};
TVITEM it;
it.hItem = item;
it.mask = TVIF_TEXT;
it.pszText = buf;
it.cchTextMax = 511;
SendMessage(hTree, TVM_GETITEM, 0, &it);
return it.pszText;
}

void PyTreeViewItem::set_value (PyObject* newVal) {
PyObject* oldVal = get_value();
TVITEM it;
it.hItem = item;
it.mask = TVIF_PARAM;
it.lParam = (LPARAM)newVal;
Py_XINCREF(newVal);
Py_XDECREF(oldVal);
SendMessage(hTree, TVM_SETITEM, 0, &it);
}

PyObject* PyTreeViewItem::get_value () {
TVITEM it;
it.hItem = item;
it.mask = TVIF_PARAM;
SendMessage(hTree, TVM_GETITEM, 0, &it);
PyObject* val = (PyObject*)it.lParam;
Py_XINCREF(val);
return val;
}


UINT PyTreeViewItem::state (UINT mask) {
return SendMessage(hTree, TV_FIRST+39, item, mask);
}

int PyTreeViewItem::get_expanded () {
return !!state(TVIS_EXPANDED);
}

int PyTreeViewItem::get_selected () {
return !!state(TVIS_SELECTED);
}

void PyTreeViewItem::select () {
SendMessage(hTree, TVM_SELECTITEM, TVGN_CARET, item);
}

void PyTreeViewItem::set_expanded (bool expand) {
SendMessage(hTree, TVM_EXPAND, expand?TVE_EXPAND:TVE_COLLAPSE, item);
}
