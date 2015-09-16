#ifndef ___PYTREEVIEWDIALOG_H1
#define ___PYTREEVIEWDIALOG_H1
#include "main.h"
#include "../../core/signals.h"

struct TVDSignals {
signal<void(PyObject*)> onfocus, onblur;
signal<bool(PyObject*), BoolSignalCombiner> onclose;
signal<bool(PyObject*, int), BoolSignalCombiner> onkeyDown, onkeyUp;
signal<bool(PyObject*, PyObject*), BoolSignalCombiner> onaction, onselect, onexpand, onedit, oncontextMenu;
signal<var(PyObject*, PyObject*, const tstring&), VarSignalCombiner> onedited;
};

struct PyTreeViewDialog {
    PyObject_HEAD 
HWND hDlg, hTree;
TVDSignals* signals;
bool closed;

static PyTreeViewDialog* New (HWND hDlg, HWND hTree);
void Delete ();
void close ();
void focus ();
bool allowEdit (HTREEITEM item, tstring& text);
bool allowEdit (HTREEITEM item);
int addEvent (const std::string& type, const PySafeObject& cb) ;
int removeEvent (const std::string& type, int id);
int get_closed ();
PyObject* get_root();
PyObject* get_selection ();
tstring get_title ();
tstring get_text  ();
void set_title (const tstring& title);
void set_text (const tstring& text);
};

struct TreeViewDialogInfo {
tstring title, label;
PyTreeViewDialog* dlg;
};

#endif
