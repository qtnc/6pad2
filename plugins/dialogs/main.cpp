#include "main.h"

HINSTANCE hinstance = 0;
SixpadData* sp = 0;

bool PyRegister_TreeViewItem (PyObject* m);
bool PyRegister_TreeViewDialog (PyObject* m);
bool PyRegister_ProgressDialog (PyObject* m);
void test123 (void) { Beep(1047,250); }

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
PyRegister_TreeViewItem (mod);
PyRegister_TreeViewDialog(mod);
PyRegister_ProgressDialog(mod);
return mod;
}

extern "C" BOOL WINAPI __declspec(dllexport) DllMain (HINSTANCE hDll, DWORD reason, LPVOID unused) {
if (reason==DLL_PROCESS_ATTACH) {
hinstance = hDll;
//DisableThreadLibraryCalls(hDll);
}
return true;
}


