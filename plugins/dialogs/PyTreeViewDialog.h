#ifndef ___PYTREEVIEWDIALOG_H1
#define ___PYTREEVIEWDIALOG_H1
#include "main.h"
#include "../../core/signals.h"

struct TVDSignals {
signal<void(PyObject*)> onfocus, onblur;
signal<bool(PyObject*), BoolSignalCombiner> onclose;
signal<bool(PyObject*, PyObject*), BoolSignalCombiner> onaction, onselect, onexpand, onedit, oncontextMenu, oncopy, oncut, onpaste, ondelete;
signal<var(PyObject*, PyObject*, const tstring&), VarSignalCombiner> onedited;
};

struct PyTreeViewDialog {
    PyObject_HEAD 
HWND hDlg, hTree;
TVDSignals* signals;

static PyTreeViewDialog* New (HWND hDlg, HWND hTree);
void Delete ();
bool allowEdit (HTREEITEM item, tstring& text);
bool allowEdit (HTREEITEM item);
int addEvent (const std::string& type, const PySafeObject& cb) ;
int removeEvent (const std::string& type, int id);
PyObject* get_root();
PyObject* get_selection ();
};

struct TreeViewDialogInfo {
tstring title, label;
PyTreeViewDialog* dlg;
};

#endif
