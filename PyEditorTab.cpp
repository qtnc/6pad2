#include "global.h"
#include "strings.hpp"
#include "page.h"
#include "python34.h"
using namespace std;

void PageSetLineEnding (shared_ptr<Page> p, int le);
void PageSetEncoding (shared_ptr<Page> p, int enc);
void PageSetIndentationMode (shared_ptr<Page> p, int im);
void PageSetAutoLineBreak (shared_ptr<Page> p, bool alb);

struct PyEditorTab { 
    PyObject_HEAD
shared_ptr<Page> page;
bool isClosed () { return !!page; }
tstring getName () { return page->name; }
tstring getFile () { return page->file; }
void setName (const tstring& s) { page->name=s; }
void setFile (const tstring& s) { page->file=s; }
int getLineEnding () { return page->lineEnding; }
int getEncoding () { return page->encoding; }
int getIndentationMode () { return page->indentationMode; }
int getAutoLineBreak () { return 0!=(page->flags&PF_AUTOLINEBREAK); }
void setLineEnding (int le) { PageSetLineEnding(page,le); }
void setEncoding (int e) { PageSetEncoding(page,e); }
void setIndentationMode (int i) { PageSetIndentationMode(page,i); }
void setAutoLineBreak (int b) { PageSetAutoLineBreak(page,b); }
};

static void PyEditorTabDealloc (PyObject* pySelf) {
PyEditorTab* self = (PyEditorTab*)pySelf;
Py_TYPE(pySelf)->tp_free(pySelf);
}

static PyEditorTab* PyEditorTabNew (PyTypeObject* type, PyObject* args, PyObject* kwds) {
PyEditorTab* self = (PyEditorTab*)(type->tp_alloc(type, 0));
return self;
}

static int PyEditorTabInit (PyEditorTab* self, PyObject* args, PyObject* kwds) {
return 0;
}

static PyMethodDef PyEditorTabMethods[] = {
PyDeclEnd
};

static PyGetSetDef PyEditorTabAccessors[] = {
PyAccessor("name", &PyEditorTab::getName, &PyEditorTab::setName),
PyAccessor("file", &PyEditorTab::getFile, &PyEditorTab::setFile),
PyReadOnlyAccessor("closed", &PyEditorTab::isClosed),
PyAccessor("lineEnding", &PyEditorTab::getLineEnding, &PyEditorTab::setLineEnding),
PyAccessor("encoding", &PyEditorTab::getEncoding, &PyEditorTab::setEncoding),
PyAccessor("indentation", &PyEditorTab::getIndentationMode, &PyEditorTab::setIndentationMode),
PyAccessor("autoLineBreak", &PyEditorTab::getAutoLineBreak, &PyEditorTab::setAutoLineBreak),
PyDeclEnd
};

static PyTypeObject PyEditorTabType = { 
    PyVarObject_HEAD_INIT(NULL, 0) 
    "window.EditorTab",             /* tp_name */ 
    sizeof(PyEditorTab), /* tp_basicsize */ 
    0,                         /* tp_itemsize */ 
    PyEditorTabDealloc,                         /* tp_dealloc */ 
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
    PyEditorTabMethods,             /* tp_methods */ 
NULL,             /* tp_members */ 
    PyEditorTabAccessors,                         /* tp_getset */ 
    0,                         /* tp_base */ 
    0,                         /* tp_dict */ 
    0,                         /* tp_descr_get */ 
    0,                         /* tp_descr_set */ 
    0,                         /* tp_dictoffset */ 
    (initproc)PyEditorTabInit,      /* tp_init */ 
    0,                         /* tp_alloc */ 
}; 

bool PyRegister_EditorTab (PyObject* m) {
PyEditorTabType.tp_new = (decltype(PyEditorTabType.tp_new))PyEditorTabNew;
if (PyType_Ready(&PyEditorTabType) < 0)          return false;
Py_INCREF(&PyEditorTabType); 
PyModule_AddObject(m, "EditorTab", (PyObject*)&PyEditorTabType); 
return true;
}
