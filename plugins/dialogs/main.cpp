#include "main.h"

HINSTANCE hinstance = 0;
SixpadData* sp = 0;

extern void test123 (void);

static PyMethodDef MainDefs [] = {
PyDecl("test", test123),
{0, 0, 0, 0}
}; 

static PyModuleDef MainMod = {
PyModuleDef_HEAD_INIT,
"qc6paddlgs",
NULL, -1, MainDefs  
};

static BOOL SixpadDLLInit2 (SixpadData* d) { sp=d; }

PyMODINIT_FUNC PyInit_qc6paddlgs  (void) {
CallSixPadDLLInit(SixpadDLLInit2);
PyObject* mod = PyModule_Create(&MainMod);
//PyProcessType.tp_new = (typeof(PyProcessType.tp_new))PyProcessNew;
//if (PyType_Ready(&PyProcessType) < 0)          return FALSE;
//Py_INCREF(&PyProcessType); 
//PyModule_AddObject(mod, "Process", (PyObject*)&PyProcessType); 
return mod;
}

extern "C" BOOL WINAPI __declspec(dllexport) DllMain (HINSTANCE hDll, DWORD reason, LPVOID unused) {
if (reason==DLL_PROCESS_ATTACH) {
hinstance = hDll;
//DisableThreadLibraryCalls(hDll);
}
return true;
}


