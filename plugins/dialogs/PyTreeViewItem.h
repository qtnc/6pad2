#ifndef ___PYTREEVIEWITEM_H1
#define ___PYTREEVIEWITEM_H1
#include "main.h"

struct PyTreeViewItem { 
    PyObject_HEAD 
HWND hTree;
HTREEITEM item;

PyTreeViewItem (HWND ht, HTREEITEM it): hTree(ht), item(it) {}
static PyTreeViewItem* New (HWND hTree, HTREEITEM item);
void Delete ();

PyObject* pyobj (HTREEITEM);
void remove () ;
void removeChild (PyObject*);
PyObject* addChild (const tstring& text, HTREEITEM hAfter = TVI_LAST, PyObject* udata=0, UINT state = 0);
PyObject* appendChild (const tstring& text, OPT, PyObject* value);
PyObject* insertBefore (const tstring& text, PyObject* child, OPT, PyObject* value);
PyObject* get_parent  () ;
PyObject* get_firstChild () ;
PyObject* get_children ();
PyObject* get_nextSibling () ;
PyObject* get_previousSibling () ;
tstring get_text ();
void set_text (const tstring& s);
PyObject* get_value ();
void set_value (PyObject*);
UINT state (UINT mask);
int get_expanded ();
void set_expanded (bool expanded) ;
int get_selected ();
void select () ;
};


#endif
