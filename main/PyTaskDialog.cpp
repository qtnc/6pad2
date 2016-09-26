#include "../core/TaskDialog.h"
#include "../core/python34.h"
#include "../core/strings.hpp"
#include "../core/thread.h"
using namespace std;

struct PyTaskDialogData {
tstring heading, text, details, footer;
PyFunc<int(PyObject*,int)> callback = nullptr;
HWND hwnd;
};

struct PyTaskDialog { 
    PyObject_HEAD 
PyTaskDialogData* data;
double value;
int modified, nButton, nRadio;
bool closed, checkboxChecked;
#define GS(x,f) \
tstring get_##x () { return data->x; } \
void set_##x (const tstring& s) { \
data->x=s; \
if (IsUIThread()) SendMessage(data->hwnd, TDM_SET_ELEMENT_TEXT, f, s.c_str()); \
else modified|=(1<<f); \
}
GS(heading, TDE_MAIN_INSTRUCTION) 
GS(text, TDE_CONTENT) 
GS(details, TDE_EXPANDED_INFORMATION) 
GS(footer, TDE_FOOTER)
#undef GS
tstring get_title () { return GetWindowText(data->hwnd); }
void set_title (const tstring& s) { SetWindowText(data->hwnd, s); }
double get_value () { return value; }
void set_value (double d) { value=d; modified|=(1<<30); }
int get_buttonClicked () {
if (nButton>=1000) return nButton -1000;
else if (nButton==IDCANCEL) return -1;
else return nButton;
}
int get_radioChecked () { return nRadio>=2000? nRadio -2000 : nRadio; }
void set_radioChecked (int rb) { 
nRadio = 2000+rb; 
if (IsUIThread()) SendMessage(data->hwnd, TDM_CLICK_RADIO_BUTTON, nRadio, 0);
else modified|=(1<<28); 
}
bool get_checkboxChecked () { return checkboxChecked; }
void set_checkboxChecked (bool b) { 
checkboxChecked=b; 
if (IsUIThread()) SendMessage(data->hwnd, TDM_CLICK_VERIFICATION, checkboxChecked, 0);
else modified|=(1<<27); 
}
bool get_closed () { return closed; }
void close () { modified|=(1<<29); nButton=IDCANCEL; }
void enableButton (int btn, bool enable) { SendMessage(data->hwnd, TDM_ENABLE_BUTTON, 1000+btn, enable); }
void enableRadioButton (int btn, bool enable) { SendMessage(data->hwnd, TDM_ENABLE_RADIO_BUTTON, 2000+btn, enable); }
void timer (HWND hwnd);
};

static void PyTaskDialogDealloc (PyObject* pySelf) {
PyTaskDialog* self = (PyTaskDialog*)pySelf;
delete self->data;
Py_TYPE(pySelf)->tp_free(pySelf);
}

static PyTaskDialog* PyTaskDialogNew (PyTypeObject* type, PyObject* args, PyObject* kwds) {
PyTaskDialog* self = (PyTaskDialog*)(type->tp_alloc(type, 0));
self->data = new PyTaskDialogData();
self->modified=0;
self->value=0;
self->closed=false;
return self;
}

static int PyTaskDialogInit (PyTaskDialog* self, PyObject* args, PyObject* kwds) {
return 0;
}

static PyMethodDef PyTaskDialogMethods[] = {
PyDecl("close", &PyTaskDialog::close),
PyDecl("enableButton", &PyTaskDialog::enableButton),
PyDecl("enableRadioButton", &PyTaskDialog::enableRadioButton),
PyDeclEnd
};

#define Prop(x) PyAccessor(#x, &PyTaskDialog::get_##x, &PyTaskDialog::set_##x)
#define RProp(x) PyReadOnlyAccessor(#x, &PyTaskDialog::get_##x)
static PyGetSetDef PyTaskDialogAccessors[] = {
Prop(heading), Prop(text), Prop(footer), Prop(details),
Prop(title), Prop(value),
Prop(radioChecked), Prop(checkboxChecked), 
RProp(buttonClicked), RProp(closed),
PyDeclEnd
};
#undef Prop
#undef RProp

static PyTypeObject PyTaskDialogType = { 
    PyVarObject_HEAD_INIT(NULL, 0) 
    "sixpad.TaskDialog",             /* tp_name */ 
    sizeof(PyTaskDialog), /* tp_basicsize */ 
    0,                         /* tp_itemsize */ 
    PyTaskDialogDealloc,                         /* tp_dealloc */ 
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
    PyTaskDialogMethods,             /* tp_methods */ 
NULL,             /* tp_members */ 
    PyTaskDialogAccessors,                         /* tp_getset */ 
    0,                         /* tp_base */ 
    0,                         /* tp_dict */ 
    0,                         /* tp_descr_get */ 
    0,                         /* tp_descr_set */ 
    0,                         /* tp_dictoffset */ 
    (initproc)PyTaskDialogInit,      /* tp_init */ 
    0,                         /* tp_alloc */ 
}; 

bool PyRegister_TaskDialog (PyObject* m) {
//PyTaskDialogType.tp_new = (decltype(PyTaskDialogType.tp_new))PyTaskDialogNew;
if (PyType_Ready(&PyTaskDialogType) < 0)          return false;
Py_INCREF(&PyTaskDialogType); 
PyModule_AddObject(m, "TaskDialog", (PyObject*)&PyTaskDialogType); 
return true;
}

void PyTaskDialog::timer (HWND hwnd) {
#define GS(x,f) if (modified&(1<<f)) SendMessage(hwnd, TDM_SET_ELEMENT_TEXT, f, data->x.c_str());
GS(heading, TDE_MAIN_INSTRUCTION) 
GS(text, TDE_CONTENT) 
GS(details, TDE_EXPANDED_INFORMATION) 
GS(footer, TDE_FOOTER)
#undef GS
if (modified&(1<<30)) SendMessage(hwnd, TDM_SET_PROGRESS_BAR_POS, (int)round(1000*value), 0);
if (modified&(1<<29)) SendMessage(hwnd, TDM_CLICK_BUTTON, nButton, 0);
if (modified&(1<<28)) SendMessage(hwnd, TDM_CLICK_RADIO_BUTTON, nRadio, 0);
if (modified&(1<<27)) SendMessage(hwnd, TDM_CLICK_VERIFICATION, checkboxChecked, 0);
modified=0;
}

static HRESULT CALLBACK TaskDialogCallback (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, PyTaskDialog* ptd) {
switch(msg){
case TDN_CREATED:
SendMessage(hwnd, TDM_SET_PROGRESS_BAR_RANGE, 0, MAKELPARAM(0, 1000));
ptd->closed=false;
ptd->data->hwnd = hwnd;
break;
case TDN_TIMER:
ptd->timer(hwnd);
break;
case TDN_BUTTON_CLICKED:
ptd->nButton = wp;
if (ptd->data->callback) return !ptd->data->callback((PyObject*)ptd, (int)wp);
break;
case TDN_RADIO_BUTTON_CLICKED:
ptd->nRadio = wp;
if (ptd->data->callback) ptd->data->callback((PyObject*)ptd, (int)wp);
break;
case TDN_VERIFICATION_CLICKED:
ptd->checkboxChecked = wp;
if (ptd->data->callback) ptd->data->callback((PyObject*)ptd, 3000+(wp?1:0) );
break;
case TDN_EXPANDO_BUTTON_CLICKED:
if (ptd->data->callback) ptd->data->callback((PyObject*)ptd, 3002+(wp?1:0) );
break;
case TDN_DESTROYED:
ptd->closed=true;
break;
}
return S_OK;
}

static inline tstring protectNull (const TCHAR* sz) {
return sz? sz : TEXT("");
}

PyObject* PyShowTaskDialog (PyObject* unused, PyObject* args, PyObject* kwds) {
int button=0, radio=0, checkbox=0, expanded=0, commandLinks=0, commandLinksNoIcon=0, expandInFooter=0, progressBar=0;
const TCHAR* icon = TEXT("none");
PyObject *buttonList=nullptr, *radioList=nullptr, *callback=nullptr;
PyTaskDialog* ptd = NULL;
vector<TASKDIALOG_BUTTON> pButtons, pRadios;
TASKDIALOGCONFIG td;
ZeroMemory(&td,sizeof(td));
td.cbSize = sizeof(td);
td.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW;
static const char* KWLST[] = { "title", "heading", "text", "footer", "details", "checkbox", "checked", "collapseButtonText", "expandButtonText", "expanded", "icon", "buttons", "radioButtons", "defaultButton", "defaultRadioButton", "expandInFooter", "commandLinks", "commandLinksNoIcon", "progressBar", "callback", NULL};
if (!PyArg_ParseTupleAndKeywords(args, kwds, "|$uuuuuupuupuOOiippppO", (char**)KWLST, &td.pszWindowTitle, &td.pszMainInstruction, &td.pszContent, &td.pszFooter, &td.pszExpandedInformation, &td.pszVerificationText, &checkbox, &td.pszExpandedControlText, &td.pszCollapsedControlText, &expanded, &icon, &buttonList, &radioList, &button, &radio, &expandInFooter, &commandLinks, &commandLinksNoIcon, &progressBar, &callback      )) return NULL;
if (callback && callback==Py_None) callback=nullptr;
if (expanded) td.dwFlags |= TDF_EXPANDED_BY_DEFAULT;
if (checkbox) td.dwFlags |= TDF_VERIFICATION_FLAG_CHECKED;
if (commandLinks) td.dwFlags  |= TDF_USE_COMMAND_LINKS;
if (commandLinksNoIcon) td.dwFlags |= TDF_USE_COMMAND_LINKS_NO_ICON;
if (expandInFooter) td.dwFlags |= TDF_EXPAND_FOOTER_AREA;
if (icon) td.pszMainIcon = MAKEINTRESOURCEW(-elt<tstring>(icon, 2, { TEXT("none"), TEXT("warning"), TEXT("error"), TEXT("info"), TEXT("shield") }));
if (buttonList && PyLong_Check(buttonList)) {
td.dwCommonButtons = PyLong_AsLong(buttonList);
td.nDefaultButton = button;
}
else if (buttonList && PySequence_Check(buttonList)) {
auto b = fromPyObject<vector<tstring>>(buttonList);
for (int i=0, n=b.size(); i<n; i++) pButtons.push_back({1000+i, b[i].c_str() });
td.nDefaultButton = 1000 + button;
td.cButtons = pButtons.size();
td.pButtons = &(pButtons[0]);
}
if (radioList && PySequence_Check(radioList)) {
auto b = fromPyObject<vector<tstring>>(radioList);
for (int i=0, n=b.size(); i<n; i++) pRadios.push_back({2000+i, b[i].c_str() });
td.nDefaultRadioButton = 2000 + radio;
td.cRadioButtons = pRadios.size();
td.pRadioButtons = &(pRadios[0]);
}
if (callback || progressBar) {
ptd = PyTaskDialogNew(&PyTaskDialogType, NULL, NULL);
ptd->data->heading = protectNull(td.pszMainInstruction);
ptd->data->text = protectNull(td.pszContent);
ptd->data->details = protectNull(td.pszExpandedInformation);
ptd->data->footer = protectNull(td.pszFooter);
ptd->data->callback = callback;
ptd->nButton = 0;
ptd->checkboxChecked = checkbox;
ptd->nRadio = radio;
ptd->closed=true;
td.pfCallback = (PFTASKDIALOGCALLBACK)TaskDialogCallback;
td.lpCallbackData = (LONG_PTR)ptd;
}
if (progressBar) {
if (IsUIThread()) {  PyErr_SetString(PyExc_SystemError, "task dialogs with progress bars must be called from a working thread, not from the main UI thread."); return NULL;  }
td.dwFlags |= TDF_SHOW_PROGRESS_BAR  |  TDF_CALLBACK_TIMER;
Py_BEGIN_ALLOW_THREADS
RunAsync([=]()mutable{ 
HRESULT re = TaskDialogIndirect(&td, &button, &radio, &checkbox); 
});
while(ptd->closed) Sleep(10);
Py_END_ALLOW_THREADS
return (PyObject*)ptd;
}
else {
Py_BEGIN_ALLOW_THREADS
HRESULT hr = TaskDialogIndirect(&td, &button, &radio, &checkbox);
if (button==IDCANCEL) button=-1;
else if (button>=1000) button -= 1000;
if (radio>=2000) radio -= 2000;
Py_END_ALLOW_THREADS
if (ptd) Py_XDECREF((PyObject*)ptd);
return Py_BuildValue("(iii)", button, radio, checkbox);
}}

