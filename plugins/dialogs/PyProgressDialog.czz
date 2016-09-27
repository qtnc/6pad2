#include "PyProgressDialog.h"
using namespace std;

static void PyProgressDialogDealloc (PyObject* pySelf) {
PyProgressDialog* self = (PyProgressDialog*)pySelf;
delete self->pd;
Py_TYPE(pySelf)->tp_free(pySelf);
}

static int PyProgressDialogInit (PyProgressDialog* pySelf, PyObject* args, PyObject* kwds) {
PyProgressDialog* self = (PyProgressDialog*)pySelf;
self->pd=0;
return 0;
}

static PyMethodDef PyProgressDialogMethods[] = {
PyDecl("close", &PyProgressDialog::close),
{"open", (PyCFunction)&PyProgressDialog::open, METH_VARARGS | METH_KEYWORDS | METH_CLASS, NULL},
PyDeclEnd
};

#define Prop(x) PyAccessor(#x, &PyProgressDialog::get_##x, &PyProgressDialog::set_##x)
#define RProp(x) PyReadOnlyAccessor(#x, &PyProgressDialog::get_##x)
static PyGetSetDef PyProgressDialogAccessors[] = {
 RProp(closed), RProp(cancelled), RProp(paused),
Prop(title), Prop(text), Prop(value),
PyDeclEnd
};
#undef Prop
#undef RProp

static PyTypeObject PyProgressDialogType = { 
    PyVarObject_HEAD_INIT(NULL, 0) 
    "qc6paddlgs.ProgressDialog",             /* tp_name */ 
    sizeof(PyProgressDialog), /* tp_basicsize */ 
    0,                         /* tp_itemsize */ 
    PyProgressDialogDealloc,                         /* tp_dealloc */ 
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
    PyProgressDialogMethods,             /* tp_methods */ 
NULL,             /* tp_members */ 
    PyProgressDialogAccessors,                         /* tp_getset */ 
    0,                         /* tp_base */ 
    0,                         /* tp_dict */ 
    0,                         /* tp_descr_get */ 
    0,                         /* tp_descr_set */ 
    0,                         /* tp_dictoffset */ 
    (initproc)PyProgressDialogInit,      /* tp_init */ 
    0,                         /* tp_alloc */ 
}; 

PyProgressDialog* PyProgressDialog::New (const tstring& title, const tstring& label) {
GIL_PROTECT
PyProgressDialog* ppd  = (PyProgressDialog*) PyProgressDialogType.tp_alloc(&PyProgressDialogType,0);
ppd->pd = new ProgressDialog(title, label);
return ppd;
}

bool PyRegister_ProgressDialog (PyObject* m) {
//PyProgressDialogType.tp_new = (decltype(PyProgressDialogType.tp_new))PyProgressDialogNew;
if (PyType_Ready(&PyProgressDialogType) < 0)          return false;
Py_INCREF(&PyProgressDialogType); 
PyModule_AddObject(m, "ProgressDialog", (PyObject*)&PyProgressDialogType); 
return true;
}

int PyProgressDialog::get_closed () {
return pd && pd->closed;
}

int PyProgressDialog::get_cancelled  () {
return pd && pd->cancelled;
}

int PyProgressDialog::get_paused () {
return pd && pd->paused;
}

int PyProgressDialog::get_value () {
return pd? pd->lastUpdate : -1;
}

void PyProgressDialog::close () {
if (pd&&!pd->closed) pd->close();
}

PyObject* PyProgressDialog::open (PyObject* unused, PyObject* args, PyObject* kwds) {
if (IsUIThread()) { 
PyErr_SetString(PyExc_SystemError, "ProgressDialog.open must be called in a working thread, not in the main UI thread.");
return NULL; 
}
const wchar_t *title=TEXT(""), *text=TEXT("");
static const char* KWLST[] = { "title", "text", NULL};
if (!PyArg_ParseTupleAndKeywords(args, kwds, "|uu", (char**)KWLST, &title, &text)) return NULL;
PyProgressDialog* ppd = PyProgressDialog::New(title, text);
Py_BEGIN_ALLOW_THREADS
RunAsync([=]()mutable{  if (ppd&&ppd->pd) ppd->pd->show(SPPTR win);   });
Py_END_ALLOW_THREADS
return (PyObject*)ppd;
}

tstring PyProgressDialog::get_title () {
return pd? pd->title : TEXT("");
}

tstring PyProgressDialog::get_text  () {
return pd? pd->text : TEXT("");
}

void PyProgressDialog::set_title (const tstring& title) {
if (pd) { SetWindowText(pd->hwnd, title); pd->title=title; }
}

void PyProgressDialog::set_text (const tstring& text) {
if (pd) { pd->update(text); pd->text=text; }
}

void PyProgressDialog::set_value (int v) {
if (pd) pd->update(v);
}

