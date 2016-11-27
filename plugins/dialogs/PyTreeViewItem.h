#ifndef ___PYTREEVIEWITEM_H1
#define ___PYTREEVIEWITEM_H1
#include "main.h"

int TVGetStateImage (HWND hTree, HTREEITEM item);
void TVSetStateImage (HWND hTree, HTREEITEM item, int index);

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
PyObject* appendChild (const tstring& text, PyObject* value, UINT state);
PyObject* insertBefore (PyObject* child, const tstring& text, PyObject* value, UINT state);
PyObject* appendChildProxy (const tstring& text, PyObject* value, OPT, bool checked, bool partiallyChecked, bool expanded, bool selected);
PyObject* insertBeforeProxy (PyObject* child, const tstring& text, PyObject* value, OPT, bool checked, bool partiallyChecked, bool expanded, bool selected);
PyObject* get_parentNode  () ;
PyObject* get_firstChild () ;
PyObject* get_lastChild () ;
PyObject* get_childNodes ();
PyObject* get_nextSibling () ;
PyObject* get_previousSibling () ;
int get_hasChildNodes ();
tstring get_text ();
void set_text (const tstring& s);
PyObject* get_value ();
void set_value (PyObject*);
UINT state (UINT mask);
int get_expanded ();
void set_expanded (bool expanded) ;
int get_selected ();
void select () ;
int getStateImage ();
void setStateImage (int index);
int get_checked ();
int get_partiallyChecked ();
void set_checked (bool b);
void set_partiallyChecked (bool b);
};


#endif
