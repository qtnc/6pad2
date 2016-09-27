#include "PyListBoxDialog.h"
using namespace std;

INT_PTR ListBoxDlgProc (HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp); 

static void PyListBoxDialogDealloc (PyObject* pySelf) {
PyListBoxDialog* self = (PyListBoxDialog*)pySelf;
delete self->signals;
Py_TYPE(pySelf)->tp_free(pySelf);
}

static int PyListBoxDialogInit (PyListBoxDialog* self, PyObject* args, PyObject* kwds) {
return 0;
}

static int PyMapLen (PyObject* o) {
PyListBoxDialog& t = *(PyListBoxDialog*)o;
return t.getItemCount();
}

static PyObject* PyMapGet (PyObject* o, PyObject* k) {
PyListBoxDialog& t = *(PyListBoxDialog*)o;
if (PyLong_Check(k)) {
int i = PyLong_AsLong(k), len = t.getItemCount();
if (i<0) i+=len;
if (i>=len) { PyErr_SetString(PyExc_ValueError, "List index out of range"); return NULL; }
return Py_BuildValue(Py_TString_Decl, t.get(i).c_str() );
}
else if (PySlice_Check(k)) {
int start, end, step, slicelen, len = t.getItemCount();
if (PySlice_GetIndicesEx(k, len, &start, &end, &step, &slicelen)) return NULL;
PyObject* list = PyList_New(0);
for (int i=start; i<end; i+=step) PyList_Append(list, Py_BuildValue(Py_TString_Decl, t.get(i).c_str() ));
return list;
}
PyErr_SetString(PyExc_TypeError, "int or slice expected"); 
return NULL;
}

static int PyMapSet (PyObject* o, PyObject* k, PyObject* v) {
PyListBoxDialog& t = *(PyListBoxDialog*)o;
if (PyLong_Check(k)) {
int i = PyLong_AsLong(k), len = t.getItemCount();
if (i<0) i+=len;
if (i>=len) { PyErr_SetString(PyExc_ValueError, "List index out of range"); return NULL; }
if (!v) { t.remove(i); return 0; }
else if (!PyUnicode_Check(v)) { PyErr_SetString(PyExc_TypeError, "str expected");  return -1; }
tstring s = PyUnicode_AsUnicode(v);
t.set(i,s);
return 0;
}
else if (PySlice_Check(k)) {
int start, end, step, slicelen, len = t.getItemCount();
if (PySlice_GetIndicesEx(k, len, &start, &end, &step, &slicelen)) return -1;
if (!v) {
for (int i=slicelen -1; i>=0; i--) t.remove(start + step*i);
return 0;
}
else {
if (!PySequence_Check(v)) { PyErr_SetString(PyExc_TypeError, "sequence expected"); return -1; }
if (PySequence_Size(v)!=slicelen) { PyErr_Format(PyExc_ValueError, "ATtempt to assign sequence of size %d to extended slice of size %d", PySequence_Size(v), slicelen); return -1; }
for (int i=0, j=start; i<slicelen; i++, j+=step) {
PyObject* x = PySequence_GetItem(v,i);
if (!PyUnicode_Check(x)) { PyErr_SetString(PyExc_TypeError, "str expected");  return -1; }
tstring s = PyUnicode_AsUnicode(x);
t.set(j,s);
}
return 0;
}}
PyErr_SetString(PyExc_TypeError, "int or slice expected"); 
return -1;
}

static PyMappingMethods PyListBoxDialogMapping = {
PyMapLen, // length
PyMapGet, // Get
PyMapSet, // set
};

static PyMethodDef PyListBoxDialogMethods[] = {
PyDecl("addEvent", &PyListBoxDialog::addEvent),
PyDecl("removeEvent", &PyListBoxDialog::removeEvent),
PyDecl("close", &PyListBoxDialog::close),
PyDecl("focus", &PyListBoxDialog::focus),
PyDecl("select", &PyListBoxDialog::select),
PyDecl("unselect", &PyListBoxDialog::unselect),
PyDecl("selectAll", &PyListBoxDialog::selectAll),
PyDecl("unselectAll", &PyListBoxDialog::unselectAll),
PyDecl("append", PyListBoxDialog::append),
PyDecl("insert", PyListBoxDialog::insert),
PyDecl("remove", PyListBoxDialog::remove),
PyDecl("clear", &PyListBoxDialog::clear),
{"open", (PyCFunction)&PyListBoxDialog::open, METH_VARARGS | METH_KEYWORDS | METH_CLASS, NULL},
PyDeclEnd
};

#define Prop(x) PyAccessor(#x, &PyListBoxDialog::get_##x, &PyListBoxDialog::set_##x)
#define RProp(x) PyReadOnlyAccessor(#x, &PyListBoxDialog::get_##x)
static PyGetSetDef PyListBoxDialogAccessors[] = {
RProp(closed),
Prop(selectedIndex), Prop(selectedValue), Prop(selectedIndices), Prop(selectedValues),
Prop(title), Prop(text), Prop(search),
PyDeclEnd
};
#undef Prop
#undef RProp

static PyTypeObject PyListBoxDialogType = { 
    PyVarObject_HEAD_INIT(NULL, 0) 
    "qc6paddlgs.ListBoxDialog",             /* tp_name */ 
    sizeof(PyListBoxDialog), /* tp_basicsize */ 
    0,                         /* tp_itemsize */ 
    PyListBoxDialogDealloc,                         /* tp_dealloc */ 
    0,                         /* tp_print */ 
    0,                         /* tp_getattr */ 
    0,                         /* tp_setattr */ 
    0,                         /* tp_reserved */ 
    0,                         /* tp_repr */ 
    0,                         /* tp_as_number */ 
    0,                         /* tp_as_sequence */ 
    &PyListBoxDialogMapping,                         /* tp_as_mapping */ 
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
    PyListBoxDialogMethods,             /* tp_methods */ 
NULL,             /* tp_members */ 
    PyListBoxDialogAccessors,                         /* tp_getset */ 
    0,                         /* tp_base */ 
    0,                         /* tp_dict */ 
    0,                         /* tp_descr_get */ 
    0,                         /* tp_descr_set */ 
    0,                         /* tp_dictoffset */ 
    (initproc)PyListBoxDialogInit,      /* tp_init */ 
    0,                         /* tp_alloc */ 
}; 

PyListBoxDialog* PyListBoxDialog::New (HWND hd, HWND hl, HWND he) {
GIL_PROTECT
PyListBoxDialog* t = (PyListBoxDialog*) PyListBoxDialogType.tp_alloc(&PyListBoxDialogType,0);
t->hDlg = hd;
t->hLb = hl;
t->hEdit = he;
t->signals = new LBDSignals();
return t;
}

void PyListBoxDialog::Delete () {
GIL_PROTECT
Py_XDECREF( (PyObject*)this );
}

bool PyRegister_ListBoxDialog (PyObject* m) {
//PyListBoxDialogType.tp_new = (decltype(PyListBoxDialogType.tp_new))PyListBoxDialogNew;
if (PyType_Ready(&PyListBoxDialogType) < 0)          return false;
Py_INCREF(&PyListBoxDialogType); 
PyModule_AddObject(m, "ListBoxDialog", (PyObject*)&PyListBoxDialogType); 
return true;
}

int PyListBoxDialog::get_selectedIndex () {
return SendMessage(hLb, LB_GETCURSEL, 0, 0);
}

void PyListBoxDialog::set_selectedIndex (int x) {
SendMessage(hLb, LB_SETCURSEL, x, 0);
}

tstring PyListBoxDialog::get_selectedValue () {
int sel = get_selectedIndex();
if (sel>=0) return GetListBoxItemText(hLb, sel);
else return TEXT("");
}

void PyListBoxDialog::set_selectedValue (const tstring& s) {
SendMessage(hLb, LB_SELECTSTRING, -1, s.c_str());
}

vector<int> PyListBoxDialog::get_selectedIndices () {
int count = SendMessage(hLb, LB_GETSELCOUNT, 0, 0);
if (count<=0) return {};
vector<int> list(count);
SendMessage(hLb, LB_GETSELITEMS, count, &list[0]);
return list;
}

void PyListBoxDialog::set_selectedIndices (const vector<int>& v) {
SendMessage(hLb, LB_SETSEL, false, -1);
for (int i: v) SendMessage(hLb, LB_SETSEL, true, i);
}

vector<tstring> PyListBoxDialog::get_selectedValues () {
vector<tstring> v;
for (int i: get_selectedIndices()) v.push_back(GetListBoxItemText(hLb,i));
return v;
}

void PyListBoxDialog::set_selectedValues (const vector<tstring>& v) {
//todo
}

int PyListBoxDialog::selected (int index) {
return SendMessage(hLb, LB_GETSEL, index, 0);
}

void PyListBoxDialog::select (int index, OPT, int sel) {
SendMessage(hLb, LB_SETSEL, sel, index);
}

void PyListBoxDialog::append (const tstring& s) {
SendMessage(hLb, LB_ADDSTRING, 0, s.c_str());
}

void PyListBoxDialog::insert (int i, const tstring& s) {
SendMessage(hLb, LB_INSERTSTRING, i, s.c_str());
}

tstring PyListBoxDialog::get (int i) {
return GetListBoxItemText(hLb, i);
}

void PyListBoxDialog::set (int i, const tstring& s) {
remove(i);
insert(i,s);
}

void PyListBoxDialog::remove (int i) {
SendMessage(hLb, LB_DELETESTRING, i, 0);
}

void PyListBoxDialog::clear () {
SendMessage(hLb, LB_RESETCONTENT, 0, 0);
}

int PyListBoxDialog::getItemCount () {
return SendMessage(hLb, LB_GETCOUNT, 0, 0);
}

int PyListBoxDialog::get_closed () {
return closed;
}

void PyListBoxDialog::close () {
if (!closed) SendMessage(hDlg, WM_COMMAND, IDCANCEL, 0);
}

PyObject* PyListBoxDialog::open (PyObject* unused, PyObject* args, PyObject* kwds) {
const wchar_t *title=TEXT(""), *hint=TEXT(""), *okText=NULL, *cancelText=NULL;
BOOL modal = false, searchField=false, multiple=false;
PyObject* callback=NULL;
static const char* KWLST[] = { "title", "hint", "modal", "callback", "okButtonText", "cancelButtonText", "multiple", "searchField", NULL};
if (!PyArg_ParseTupleAndKeywords(args, kwds, "|uupOuupp", (char**)KWLST, &title, &hint, &modal, &callback, &okText, &cancelText, &multiple, &searchField)) return NULL;
ListBoxDialogInfo lbdi = { title, hint, okText?okText:msg("&OK"), cancelText?cancelText:msg("Ca&ncel"), modal, multiple, searchField, callback, NULL };
if (modal) {
bool cancelled;
Py_BEGIN_ALLOW_THREADS
RunSync([&]()mutable{ cancelled = IDYES!=DialogBoxParam(hinstance, (multiple? IDD_LISTBOX2 : IDD_LISTBOX), sp->win, ListBoxDlgProc, &lbdi); });
Py_END_ALLOW_THREADS
PyListBoxDialog& dlg = *lbdi.dlg;
PyObject* re = NULL;
if (cancelled) re = Py_None;
else re = *dlg.signals->finalValue;
dlg.Delete();
return re;
} else {
HWND hDlg = CreateDialogParam(hinstance, (multiple? IDD_LISTBOX2 : IDD_LISTBOX), sp->win, ListBoxDlgProc, &lbdi);
ShowWindow(hDlg, SW_SHOW);
return (PyObject*) lbdi.dlg;
}}

tstring PyListBoxDialog::get_title () {
return GetWindowText(hDlg);
}

tstring PyListBoxDialog::get_text  () {
return GetDlgItemText(hDlg, 1001);
}

tstring PyListBoxDialog::get_search  () {
return GetDlgItemText(hDlg, 1002);
}
void PyListBoxDialog::set_title (const tstring& title) {
SetWindowText(hDlg, title);
}

void PyListBoxDialog::set_text (const tstring& text) {
SendMessage(hDlg, WM_USER, 1001, text.c_str());
}

void PyListBoxDialog::set_search (const tstring& text) {
SendMessage(hDlg, WM_USER, 1002, text.c_str());
}

void PyListBoxDialog::focus () {
if (!closed) RunSync([&]()mutable{
ShowWindow(hDlg, SW_SHOW);
SetForegroundWindow(hDlg);
});//
}

int PyListBoxDialog::addEvent (const string& type, PyGenericFunc cb) {
connection con;
if(false){}
#define E(n) else if (type==#n) con = signals->on##n .connect(PyFunc<typename decltype(signals->on##n)::signature_type>(cb.o));
E(action) E(select) E(contextMenu) E(search)
E(close) E(focus) E(blur)
E(keyDown) E(keyUp)
#undef E
if (con.connected()) return AddSignalConnection(con);
else return 0;
}

int PyListBoxDialog::removeEvent (const string& type, int id) {
connection con = RemoveSignalConnection(id);
bool re = con.connected();
con.disconnect();
return re;
}

static LRESULT CALLBACK ListBoxSubclassProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR subclassId, PyListBoxDialog* dlg) {
static tstring input = TEXT("");
static DWORD lastInput = 0;
switch(msg){
case WM_KEYDOWN: case WM_SYSKEYDOWN: {
int kc = LOWORD(wp) | GetCurrentModifiers();
if (!dlg->signals->onkeyDown((PyObject*)dlg, kc)) return true;
}break;
case WM_KEYUP: case WM_SYSKEYUP: {
int kc = LOWORD(wp) | GetCurrentModifiers();
if (!dlg->signals->onkeyUp((PyObject*)dlg, kc)) return true;
}break;
case WM_CHAR: {
TCHAR ch = LOWORD(wp);
if (ch<=32) break;
DWORD time = GetTickCount();
int pos = SendMessage(hwnd, LB_GETCURSEL, 0, 0);
if (time-lastInput>500) input = TEXT("");
lastInput = time;
input += ch;
int npos = SendMessage(hwnd, LB_FINDSTRING, pos -1, input.c_str() );
if (npos<0 || npos==LB_ERR) { MessageBeep(MB_OK); return true; }
else if (npos==pos) return true;
else { SendMessage(hwnd, LB_SETCURSEL, npos, 0); return true; }
}break;
case WM_CONTEXTMENU:
int udw = GetWindowLong(dlg->hDlg, GWL_USERDATA);
bool modal = udw&1, multiple = udw&2, re = modal;
PyObject* indices = multiple? toPyObject(dlg->get_selectedIndices()) : toPyObject(dlg->get_selectedIndex());
PyObject* values = multiple? toPyObject(dlg->get_selectedValues()) : toPyObject(dlg->get_selectedValue() .c_str() );
dlg->signals->oncontextMenu((PyObject*)&dlg, (PyObject*)indices, (PyObject*)values );
break;
}
return DefSubclassProc(hwnd, msg, wp, lp);
}

static LRESULT CALLBACK SearchFieldSubclassProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR subclassId, PyListBoxDialog* dlg) {
static DWORD timerId = 0;
switch(msg){
case WM_KEYDOWN:
if (LOWORD(wp)==VK_DOWN) {
if (SendMessage(dlg->hLb, LB_GETCURSEL, 0, 0)<0) SendMessage(dlg->hLb, LB_SETCURSEL, 0, 0);
SetFocus(dlg->hLb);
return true;
}break;
case WM_KEYUP:
SetTimer(hwnd, timerId, 500, NULL);
break;
case WM_KILLFOCUS:
KillTimer(hwnd, timerId);
timerId=0;
break;
case WM_TIMER:
KillTimer(hwnd, timerId);
timerId=0;
dlg->signals->onsearch( (PyObject*)dlg, GetWindowText(hwnd) );
break;
}
return DefSubclassProc(hwnd, msg, wp, lp);
}

INT_PTR ListBoxDlgProc (HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp) {
switch (umsg) {
case WM_INITDIALOG : {
GIL_PROTECT
HWND hEdit = GetDlgItem(hwnd, 1002), hLb = GetDlgItem(hwnd, 1003);
ListBoxDialogInfo& lbdi = *(ListBoxDialogInfo*)(lp);
lbdi.dlg = PyListBoxDialog::New(hwnd, hLb, hEdit);
PyListBoxDialog& dlg = *lbdi.dlg;
dlg.hEdit = hEdit;
dlg.hLb = hLb;
Py_XINCREF(&dlg);
SetWindowLong(hwnd, DWL_USER, (LONG)lbdi.dlg);
SetWindowLong(hwnd, GWL_USERDATA, (lbdi.modal?1:0) | (lbdi.multiple?2:0) | (lbdi.searchField?4:0) );
SetWindowText(hwnd, lbdi.title);
SetDlgItemText(hwnd, 1001, lbdi.label);
if (lbdi.okText.size()>0) SetDlgItemText(hwnd, IDYES, lbdi.okText);
else ShowWindow(GetDlgItem(hwnd, IDYES), SW_HIDE);
if (lbdi.cancelText.size()>0) SetDlgItemText(hwnd, IDCANCEL, lbdi.cancelText);
else ShowWindow(GetDlgItem(hwnd, IDCANCEL), SW_HIDE);
if (!lbdi.searchField) ShowWindow(GetDlgItem(hwnd, 1002), SW_HIDE);
SetWindowSubclass(hLb, (SUBCLASSPROC)ListBoxSubclassProc, 0, (DWORD_PTR)&dlg);
SetWindowSubclass(hEdit, (SUBCLASSPROC)SearchFieldSubclassProc, 0, (DWORD_PTR)&dlg);
if (lbdi.callback) lbdi.callback((PyObject*)&dlg);
SetFocus(lbdi.searchField? hEdit : hLb);
if (!lbdi.modal) sp->AddModlessWindow(hwnd);
}return false;
case WM_COMMAND :
switch(LOWORD(wp)) {
case 1003:
if (HIWORD(wp)==LBN_SELCHANGE && !(GetWindowLong(hwnd, GWL_USERDATA)&2)) {
GIL_PROTECT
PyListBoxDialog& dlg = *(PyListBoxDialog*)GetWindowLong(hwnd, DWL_USER);
PyObject *index = Py_BuildValue("i", dlg.get_selectedIndex()), *value = Py_BuildValue("u", dlg.get_selectedValue().c_str());
dlg.signals->onselect( (PyObject*)&dlg, index, value);
}
if (HIWORD(wp)!=LBN_DBLCLK) break;
case IDYES: {
GIL_PROTECT
PyListBoxDialog& dlg = *(PyListBoxDialog*)GetWindowLong(hwnd, DWL_USER);
int udw = GetWindowLong(hwnd, GWL_USERDATA);
bool modal = udw&1, multiple = udw&2, re = modal;
PyObject* indices = multiple? toPyObject(dlg.get_selectedIndices()) : toPyObject(dlg.get_selectedIndex());
PyObject* values = multiple? toPyObject(dlg.get_selectedValues()) : toPyObject(dlg.get_selectedValue() .c_str() );
if (!dlg.signals->onaction.empty()) re = dlg.signals->onaction((PyObject*)&dlg, (PyObject*)indices, (PyObject*)values );
dlg.signals->finalValue.assign( Py_BuildValue("(OO)", indices, values)  ,true);
if (!re) break; 
}
case IDCANCEL: {
PyListBoxDialog& dlg = *(PyListBoxDialog*)GetWindowLong(hwnd, DWL_USER);
bool modal = GetWindowLong(hwnd, GWL_USERDATA)&1, re = true;
if (!dlg.signals->onclose.empty()) re = dlg.signals->onclose((PyObject*)&dlg);
if (!re) return true;
if (modal) EndDialog(hwnd,LOWORD(wp));
else {
sp->RemoveModlessWindow(hwnd);
DestroyWindow(hwnd);
sp->GoToNextModlessWindow(0);
}
dlg.closed=true; 
dlg.Delete();
}break;//IDCANCEL
}break;//WM_COMMAND
case WM_ACTIVATE: {
PyListBoxDialog& dlg = *(PyListBoxDialog*)GetWindowLong(hwnd, DWL_USER);
if (LOWORD(wp)) dlg.signals->onfocus((PyObject*)&dlg);
else dlg.signals->onblur((PyObject*)&dlg);
}break;
case WM_USER: SetDlgItemText(hwnd, LOWORD(wp), (LPCTSTR)lp); break;
}
return FALSE;
}

