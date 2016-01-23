#include "PyTreeViewDialog.h"
#include "PyTreeViewItem.h"
//#include "commctrlmsvc6.h"
using namespace std;

#include "PyTreeViewDialog.h"
using namespace std;

INT_PTR TreeViewDlgProc (HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp); 

static void PyTreeViewDialogDealloc (PyObject* pySelf) {
PyTreeViewDialog* self = (PyTreeViewDialog*)pySelf;
delete self->signals;
Py_TYPE(pySelf)->tp_free(pySelf);
}

static int PyTreeViewDialogInit (PyTreeViewDialog* self, PyObject* args, PyObject* kwds) {
return 0;
}

static PyMethodDef PyTreeViewDialogMethods[] = {
PyDecl("addEvent", &PyTreeViewDialog::addEvent),
PyDecl("removeEvent", &PyTreeViewDialog::removeEvent),
PyDecl("close", &PyTreeViewDialog::close),
PyDecl("focus", &PyTreeViewDialog::focus),
{"open", (PyCFunction)&PyTreeViewDialog::open, METH_VARARGS | METH_KEYWORDS | METH_CLASS, NULL},
PyDeclEnd
};

#define Prop(x) PyAccessor(#x, &PyTreeViewDialog::get_##x, &PyTreeViewDialog::set_##x)
#define RProp(x) PyReadOnlyAccessor(#x, &PyTreeViewDialog::get_##x)
static PyGetSetDef PyTreeViewDialogAccessors[] = {
RProp(root), RProp(closed),
RProp(selectedItem), RProp(selectedValue), RProp(selectedItems), RProp(selectedValues),
Prop(title), Prop(text),
PyDeclEnd
};
#undef Prop
#undef RProp

static PyTypeObject PyTreeViewDialogType = { 
    PyVarObject_HEAD_INIT(NULL, 0) 
    "qc6paddlgs.TreeViewDialog",             /* tp_name */ 
    sizeof(PyTreeViewDialog), /* tp_basicsize */ 
    0,                         /* tp_itemsize */ 
    PyTreeViewDialogDealloc,                         /* tp_dealloc */ 
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
    PyTreeViewDialogMethods,             /* tp_methods */ 
NULL,             /* tp_members */ 
    PyTreeViewDialogAccessors,                         /* tp_getset */ 
    0,                         /* tp_base */ 
    0,                         /* tp_dict */ 
    0,                         /* tp_descr_get */ 
    0,                         /* tp_descr_set */ 
    0,                         /* tp_dictoffset */ 
    (initproc)PyTreeViewDialogInit,      /* tp_init */ 
    0,                         /* tp_alloc */ 
}; 

PyTreeViewDialog* PyTreeViewDialog::New (HWND hd, HWND ht) {
GIL_PROTECT
PyTreeViewDialog* t = (PyTreeViewDialog*) PyTreeViewDialogType.tp_alloc(&PyTreeViewDialogType,0);
t->hDlg = hd;
t->hTree = ht;
t->signals = new TVDSignals();
return t;
}

void PyTreeViewDialog::Delete () {
GIL_PROTECT
Py_XDECREF( (PyObject*)this );
}

bool PyRegister_TreeViewDialog (PyObject* m) {
//PyTreeViewDialogType.tp_new = (decltype(PyTreeViewDialogType.tp_new))PyTreeViewDialogNew;
if (PyType_Ready(&PyTreeViewDialogType) < 0)          return false;
Py_INCREF(&PyTreeViewDialogType); 
PyModule_AddObject(m, "TreeViewDialog", (PyObject*)&PyTreeViewDialogType); 
return true;
}

bool PyTreeViewDialog::allowEdit (HTREEITEM item) {
if (!(GetWindowLong(hDlg, GWL_USERDATA)&4)) return false;
bool re = true;
if (!signals->onedit.empty()) re = signals->onedit((PyObject*)this, (PyObject*)PyTreeViewItem::New(hTree, item));
return re;
}

bool PyTreeViewDialog::allowEdit (HTREEITEM item, tstring& text) {
if (!signals->onedited.empty()) {
var re = signals->onedited((PyObject*)this, (PyObject*)PyTreeViewItem::New(hTree, item), text);
if (re.getType()==T_BOOL && !re) return false;
else if (re.getType()==T_STR) text = re.toTString();
}
return text.size()>0;
}

PyObject* PyTreeViewDialog::get_root () {
return (PyObject*) PyTreeViewItem::New(hTree, TVI_ROOT);
}

PyObject* PyTreeViewDialog::get_selectedItem () {
HTREEITEM selection = (HTREEITEM)SendMessage(hTree, TVM_GETNEXTITEM, TVGN_CARET, 0);
if (selection) return (PyObject*) PyTreeViewItem::New(hTree, selection);
else { Py_RETURN_NONE; }
}

PyObject* PyTreeViewDialog::get_selectedValue () {
HTREEITEM selection = (HTREEITEM)SendMessage(hTree, TVM_GETNEXTITEM, TVGN_CARET, 0);
if (selection) {
TVITEM it;
it.hItem = selection;
it.mask = TVIF_PARAM;
SendMessage(hTree, TVM_GETITEM, 0, &it);
PyObject* val = (PyObject*)it.lParam;
Py_XINCREF(val);
return val;
}
else { Py_RETURN_NONE; }
}

template<class F> static void TVWalk (HWND hTree, HTREEITEM item, F func) {
HTREEITEM cur = (HTREEITEM)SendMessage(hTree, TVM_GETNEXTITEM, TVGN_CHILD, item);
while(cur){
if (func(cur)) TVWalk(hTree, cur, func);
cur = (HTREEITEM)SendMessage(hTree, TVM_GETNEXTITEM, TVGN_NEXT, cur);
}}

PyObject* PyTreeViewDialog::get_selectedItems () {
PyObject* list = PyList_New(0);
TVWalk(hTree, TVI_ROOT, [&](HTREEITEM item)mutable{
int state = TVGetStateImage(hTree, item);
if (state==2) PyList_Append(list, (PyObject*)PyTreeViewItem::New(hTree, item));
return state!=2;
});
return list;
}

PyObject* PyTreeViewDialog::get_selectedValues () {
PyObject* list = PyList_New(0);
TVWalk(hTree, TVI_ROOT, [&](HTREEITEM item)mutable{
int state = TVGetStateImage(hTree, item);
if (state==2) {
TVITEM it;
it.hItem = item;
it.mask = TVIF_PARAM;
SendMessage(hTree, TVM_GETITEM, 0, &it);
PyObject* val = (PyObject*)it.lParam;
Py_XINCREF(val);
PyList_Append(list, val);
}
return state!=2;
});
return list;
}

int PyTreeViewDialog::get_closed () {
return closed;
}

void PyTreeViewDialog::close () {
if (!closed) SendMessage(hDlg, WM_COMMAND, IDCANCEL, 0);
}

PyObject* PyTreeViewDialog::open (PyObject* unused, PyObject* args, PyObject* kwds) {
const wchar_t *title=TEXT(""), *hint=TEXT(""), *okText=NULL, *cancelText=NULL;
BOOL modal = false, checkboxes=false, editable=false;
PyObject* callback=NULL;
static const char* KWLST[] = { "title", "hint", "modal", "callback", "okButtonText", "cancelButtonText", "multiple", "editable", NULL};
if (!PyArg_ParseTupleAndKeywords(args, kwds, "|uupOuupp", (char**)KWLST, &title, &hint, &modal, &callback, &okText, &cancelText, &checkboxes, &editable)) return NULL;
TreeViewDialogInfo tvdi = { title, hint, okText?okText:msg("&OK"), cancelText?cancelText:msg("Ca&ncel"), modal, checkboxes, editable, callback, NULL };
if (modal) {
bool cancelled;
Py_BEGIN_ALLOW_THREADS
RunSync([&]()mutable{ cancelled = IDYES!=DialogBoxParam(hinstance, (checkboxes? IDD_TREEVIEW2 : IDD_TREEVIEW), sp->win, TreeViewDlgProc, &tvdi); });
Py_END_ALLOW_THREADS
PyTreeViewDialog& dlg = *tvdi.dlg;
PyObject* re = NULL;
if (cancelled) re = Py_None;
else re = *dlg.signals->finalValue;
dlg.Delete();
return re;
} else {
HWND hDlg = CreateDialogParam(hinstance, (checkboxes? IDD_TREEVIEW2 : IDD_TREEVIEW), sp->win, TreeViewDlgProc, &tvdi);
ShowWindow(hDlg, SW_SHOW);
return (PyObject*) tvdi.dlg;
}}

tstring PyTreeViewDialog::get_title () {
return GetWindowText(hDlg);
}

tstring PyTreeViewDialog::get_text  () {
return GetDlgItemText(hDlg, 1001);
}

void PyTreeViewDialog::set_title (const tstring& title) {
SetWindowText(hDlg, title);
}

void PyTreeViewDialog::set_text (const tstring& text) {
SendMessage(hDlg, WM_USER, 1001, text.c_str());
}

void PyTreeViewDialog::focus () {
if (!closed) RunSync([&]()mutable{
ShowWindow(hDlg, SW_SHOW);
SetForegroundWindow(hDlg);
});//
}

int PyTreeViewDialog::addEvent (const string& type, const PySafeObject& cb) {
connection con;
if(false){}
#define E(n) else if (type==#n) con = signals->on##n .connect(cb.asFunction<typename decltype(signals->on##n)::signature_type>());
E(action) E(select) E(expand) E(contextMenu) E(check)
E(edit) E(edited) E(close) E(focus) E(blur)
E(keyDown) E(keyUp)
#undef E
if (con.connected()) return AddSignalConnection(con);
else return 0;
}

int PyTreeViewDialog::removeEvent (const string& type, int id) {
connection con = RemoveSignalConnection(id);
bool re = con.connected();
con.disconnect();
return re;
}

static void TVHandleCheckItem (HWND hTree, HTREEITEM item, bool checked, int flags=3) {
if (flags&1) {
// Check/uncheck recursively all subitems
HTREEITEM cur = (HTREEITEM)SendMessage(hTree, TVM_GETNEXTITEM, TVGN_CHILD, item);
while(cur){
TVSetStateImage(hTree, cur, checked? 2 : 1);
TVHandleCheckItem(hTree, cur, checked, 1);
cur = (HTREEITEM)SendMessage(hTree, TVM_GETNEXTITEM, TVGN_NEXT, cur);
}}
if (flags&2) {
HTREEITEM parent = (HTREEITEM)SendMessage(hTree, TVM_GETNEXTITEM, TVGN_PARENT, item);
if (parent){
int count=0, countChecked=0;
HTREEITEM cur = (HTREEITEM)SendMessage(hTree, TVM_GETNEXTITEM, TVGN_CHILD, parent);
while(cur){
count++;
if (TVGetStateImage(hTree, cur)>1) countChecked++;
cur = (HTREEITEM)SendMessage(hTree, TVM_GETNEXTITEM, TVGN_NEXT, cur);
}
int state = 0;
if (count==countChecked) state=2; // everything is checked
else if (countChecked==0) state=1; // Nothing is checked
else state=3; // Some items are checked
TVSetStateImage(hTree, parent, state);
TVHandleCheckItem(hTree, parent, checked, 2);
}}}

static LRESULT CALLBACK TreeViewSubclassProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR subclassId, PyTreeViewDialog* dlg) {
switch(msg){
case WM_KEYDOWN: case WM_SYSKEYDOWN: {
int kc = LOWORD(wp) | GetCurrentModifiers();
if (!dlg->signals->onkeyDown((PyObject*)dlg, kc)) return true;
}break;
case WM_KEYUP: case WM_SYSKEYUP: {
int kc = LOWORD(wp) | GetCurrentModifiers();
if (!dlg->signals->onkeyUp((PyObject*)dlg, kc)) return true;
if (kc==VK_SPACE) SendMessage(hwnd, WM_USER, 0, SendMessage(hwnd, TVM_GETNEXTITEM, TVGN_CARET, 0));
}break;
case WM_LBUTTONUP: {
TVHITTESTINFO info;
info.pt.x = HIWORD(lp);
info.pt.y = LOWORD(lp);
info.flags = 0;
info.hItem = NULL;
SendMessage(hwnd, TVM_HITTEST, 0, &info);
if (info.hItem && (info.flags&TVHT_ONITEMSTATEICON)) SendMessage(hwnd, WM_USER, 0, info.hItem);
}break;
case WM_USER: {
HTREEITEM item = (HTREEITEM)lp;
int state = TVGetStateImage(hwnd, item), re=true;
if (!dlg->signals->oncheck.empty()) re = dlg->signals->oncheck((PyObject*)dlg, (PyObject*)PyTreeViewItem::New(hwnd, item));
if (re) TVHandleCheckItem(hwnd, item, state>1);
}break;
}
return DefSubclassProc(hwnd, msg, wp, lp);
}

static LRESULT CALLBACK EditingTreeLabelProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR subclassId, HWND hTree) {
if (msg==WM_KEYDOWN) switch(LOWORD(wp)){
case VK_RETURN: SendMessage(hTree, TVM_ENDEDITLABELNOW, FALSE, 0); return true;
case VK_ESCAPE: SendMessage(hTree, TVM_ENDEDITLABELNOW, TRUE, 0); return true;
}
else if (msg==WM_GETDLGCODE) return DLGC_WANTALLKEYS;
return DefSubclassProc(hwnd, msg, wp, lp);
}

INT_PTR TreeViewDlgProc (HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp) {
switch (umsg) {
case WM_INITDIALOG : {
GIL_PROTECT
HWND hTree = GetDlgItem(hwnd, 1002);
TreeViewDialogInfo& tvdi = *(TreeViewDialogInfo*)(lp);
tvdi.dlg = PyTreeViewDialog::New(hwnd, hTree);
PyTreeViewDialog& dlg = *tvdi.dlg;
dlg.hTree = hTree;
Py_XINCREF(&dlg);
SetWindowLong(hwnd, DWL_USER, (LONG)tvdi.dlg);
SetWindowLong(hwnd, GWL_USERDATA, (tvdi.modal?1:0) | (tvdi.checkboxes?2:0) | (tvdi.editable?4:0) );
SetWindowText(hwnd, tvdi.title);
SetDlgItemText(hwnd, 1001, tvdi.label);
if (tvdi.okText.size()>0) SetDlgItemText(hwnd, IDYES, tvdi.okText);
else ShowWindow(GetDlgItem(hwnd, IDYES), SW_HIDE);
if (tvdi.cancelText.size()>0) SetDlgItemText(hwnd, IDCANCEL, tvdi.cancelText);
else ShowWindow(GetDlgItem(hwnd, IDCANCEL), SW_HIDE);
if (tvdi.checkboxes) SendMessage(hTree, 0x1100+44, 0x80, 0x80); // Partial checkboxes
SetWindowSubclass(hTree, (SUBCLASSPROC)TreeViewSubclassProc, 0, (DWORD_PTR)&dlg);
if (tvdi.callback) tvdi.callback.asFunction<void(PyObject*)>()((PyObject*)&dlg);
SetFocus(hTree);
sp->AddModlessWindow(hwnd);
}return false;
case WM_COMMAND :
switch(LOWORD(wp)) {
case IDYES: {
GIL_PROTECT
PyTreeViewDialog& dlg = *(PyTreeViewDialog*)GetWindowLong(hwnd, DWL_USER);
int udw = GetWindowLong(hwnd, GWL_USERDATA);
bool modal = udw&1, checkboxes = udw&2, re = modal;
PyObject* selection = checkboxes? dlg.get_selectedValues() : dlg.get_selectedValue();
if (!dlg.signals->onaction.empty()) re = dlg.signals->onaction((PyObject*)&dlg, (PyObject*)selection);
dlg.signals->finalValue = selection;
if (!re) break; 
}
case IDCANCEL: {
PyTreeViewDialog& dlg = *(PyTreeViewDialog*)GetWindowLong(hwnd, DWL_USER);
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
case WM_NOTIFY : {
LPNMHDR nmh = (LPNMHDR)(lp);
if (nmh->idFrom==1002) {
PyTreeViewDialog& dlg = *(PyTreeViewDialog*)GetWindowLong(hwnd, DWL_USER);
switch(nmh->code){
case NM_DBLCLK:
case NM_RETURN:
SendMessage(hwnd, WM_COMMAND, IDOK, 0);
return true;
case NM_RCLICK:
if (!dlg.signals->oncontextMenu.empty()) dlg.signals->oncontextMenu((PyObject*)&dlg, dlg.get_selectedItem());
return true;
case NM_RDBLCLK:
break;
case TVN_SELCHANGED: {
TVITEM& item = ((LPNMTREEVIEW)lp)->itemNew;
if (!dlg.signals->onselect.empty()) dlg.signals->onselect((PyObject*)&dlg, (PyObject*)PyTreeViewItem::New(nmh->hwndFrom, item.hItem));
}break;
case TVN_ITEMEXPANDING: {
if (!(((LPNMTREEVIEW)lp)->action & TVE_EXPAND)) return false;
TVITEM& item = ((LPNMTREEVIEW)lp)->itemNew;
bool re = true;
if (!dlg.signals->onexpand.empty()) re = dlg.signals->onexpand((PyObject*)&dlg, (PyObject*)PyTreeViewItem::New(nmh->hwndFrom, item.hItem));
return !re;
}break;
case TVN_DELETEITEM : {
TVITEM& item = ((LPNMTREEVIEW)lp)->itemOld;
{ GIL_PROTECT Py_XDECREF((PyObject*)item.lParam); }
}break;
case TVN_BEGINLABELEDIT: {
TVITEM& item = ((LPNMTVDISPINFO)lp)->item;
if (!dlg.allowEdit(item.hItem)) {
MessageBeep(MB_OK);
return true;
}
HWND hEdit = (HWND)SendMessage(nmh->hwndFrom, TVM_GETEDITCONTROL, 0, 0);
if (hEdit) SetWindowSubclass(hEdit, (SUBCLASSPROC)EditingTreeLabelProc, 0, (DWORD_PTR)nmh->hwndFrom);
return false;
}break;
case TVN_ENDLABELEDIT: {
TVITEM& item = ((LPNMTVDISPINFO)lp)->item;
if (item.pszText) {
tstring text = item.pszText;
if (!dlg.allowEdit(item.hItem, text)) return false;
item.mask = TVIF_TEXT;
item.pszText = (LPTSTR)text.c_str();
item.cchTextMax = text.size();
SendMessage(nmh->hwndFrom, TVM_SETITEM, 0, &item);
}
return true;
}break;
case TVN_KEYDOWN: {
WORD key = *(WORD*)(nmh+1);
switch(key){
case 0x5D: { 
NMHDR z = *(LPNMHDR)lp; z.code = NM_RCLICK;
SendMessage(hwnd, WM_NOTIFY, 0, &z);
}break;
case VK_F2: {
HTREEITEM item = (HTREEITEM)SendMessage(nmh->hwndFrom, TVM_GETNEXTITEM, TVGN_CARET, NULL);
if (dlg.allowEdit(item)) SendMessage(nmh->hwndFrom, TVM_EDITLABEL, 0, item); 
else MessageBeep(MB_OK);
}break;
}return false;}
//other notifications
}}}break;
case WM_ACTIVATE: {
PyTreeViewDialog& dlg = *(PyTreeViewDialog*)GetWindowLong(hwnd, DWL_USER);
if (LOWORD(wp)) dlg.signals->onfocus((PyObject*)&dlg);
else dlg.signals->onblur((PyObject*)&dlg);
}break;
case WM_USER: SetDlgItemText(hwnd, LOWORD(wp), (LPCTSTR)lp); break;
}
return FALSE;
}

