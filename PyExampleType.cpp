#include "global.h"
#if DEBUG
#include "strings.hpp"
#include "page.h"
#include "python34.h"
using namespace std;

struct PyMyObj { 
    PyObject_HEAD 
int val;
int getInt () { printf("Val get: %d\r\n", val); return val; }
void setInt (int x) { val=x; printf("Val set: %d\r\n", val); }
};

static void PyMyObjDealloc (PyObject* pySelf) {
PyMyObj* self = (PyMyObj*)pySelf;
Py_TYPE(pySelf)->tp_free(pySelf);
}

static PyMyObj* PyMyObjNew (PyTypeObject* type, PyObject* args, PyObject* kwds) {
PyMyObj* self = (PyMyObj*)(type->tp_alloc(type, 0));
self->val = 4;
return self;
}

static int PyMyObjInit (PyMyObj* self, PyObject* args, PyObject* kwds) {
return 0;
}

static PyMethodDef PyMyObjMethods[] = {
PyDeclEnd
};

static PyGetSetDef PyMyObjAccessors[] = {
PyAccessor("val", &PyMyObj::getInt, &PyMyObj::setInt),
PyDeclEnd
};

static PyTypeObject PyMyObjType = { 
    PyVarObject_HEAD_INIT(NULL, 0) 
    "sixpad.MyObj",             /* tp_name */ 
    sizeof(PyMyObj), /* tp_basicsize */ 
    0,                         /* tp_itemsize */ 
    PyMyObjDealloc,                         /* tp_dealloc */ 
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
    PyMyObjMethods,             /* tp_methods */ 
NULL,             /* tp_members */ 
    PyMyObjAccessors,                         /* tp_getset */ 
    0,                         /* tp_base */ 
    0,                         /* tp_dict */ 
    0,                         /* tp_descr_get */ 
    0,                         /* tp_descr_set */ 
    0,                         /* tp_dictoffset */ 
    (initproc)PyMyObjInit,      /* tp_init */ 
    0,                         /* tp_alloc */ 
}; 

bool PyRegister_MyObj (PyObject* m) {
PyMyObjType.tp_new = (decltype(PyMyObjType.tp_new))PyMyObjNew;
if (PyType_Ready(&PyMyObjType) < 0)          return false;
Py_INCREF(&PyMyObjType); 
PyModule_AddObject(m, "MyObj", (PyObject*)&PyMyObjType); 
return true;
}

#endif
