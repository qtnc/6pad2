#ifndef ___PYLISTBOXDIALOG_H1
#define ___PYLISTBOXDIALOG_H1
#include "main.h"
#include "../../core/signals.h"

struct LBDSignals {
signal<void(PyObject*)> onfocus, onblur;
signal<bool(PyObject*), BoolSignalCombiner> onclose;
signal<bool(PyObject*, int), BoolSignalCombiner> onkeyDown, onkeyUp;
signal<bool(PyObject*, PyObject*, PyObject*), BoolSignalCombiner> onaction, onselect, oncontextMenu;
signal<bool(PyObject*, const tstring&), BoolSignalCombiner> onsearch;
PySafeObject finalValue;
};

struct PyListBoxDialog {
    PyObject_HEAD 
HWND hDlg, hLb, hEdit;
LBDSignals* signals;
bool closed, modal;

static PyListBoxDialog* New (HWND hDlg, HWND hLb, HWND hEdit=0);
void Delete ();
static PyObject* open (PyObject* unused, PyObject* args, PyObject* kw); 
void close ();
void focus ();
int selected (int);
void select (int, OPT=nullptr, int=1);
inline void unselect (int x) { select(x, nullptr, 0); }
inline void selectAll () { select(-1); }
inline void unselectAll () { unselect(-1); }
void append (const tstring&);
void insert (int, const tstring&);
void remove (int);
void clear();
tstring get (int i);
void set (int i, const tstring& s);
int getItemCount ();
int addEvent (const std::string& type, const PySafeObject& cb) ;
int removeEvent (const std::string& type, int id);
int get_closed ();
int get_selectedIndex ();
void set_selectedIndex (int);
tstring get_selectedValue ();
void set_selectedValue (const tstring&);
std::vector<int> get_selectedIndices();
void set_selectedIndices (const std::vector<int>&);
std::vector<tstring> get_selectedValues ();
void set_selectedValues (const std::vector<tstring>&);
tstring get_title ();
tstring get_text  ();
tstring get_search ();
void set_title (const tstring& title);
void set_text (const tstring& text);
void set_search (const tstring& text);
};

struct ListBoxDialogInfo {
tstring title, label, okText, cancelText;
bool modal, multiple, searchField;
PySafeObject callback;
PyListBoxDialog* dlg;
};

#endif
