#define UNICODE
#include<windows.h>
#include<python/python.h>
#include<stdio.h>
#include<string.h>

typedef struct { 
    PyObject_HEAD
HANDLE inRd, inWr, outRd, outWr, hProc, hThread;
const char* encoding;
BOOL binary;
} PyProcess;

static void Process32Wait (PyProcess* p) {
if (p->inWr) {
FlushFileBuffers(p->inWr);
Sleep(1);
CloseHandle(p->inWr);
p->inWr=0;
}
if (p->hProc) WaitForSingleObject(p->hProc, INFINITE);
}

static int Process32End (PyProcess* p, int code) {
if (!p || !p->hProc)  return -1;
int e = -1;
if (!code) Process32Wait(p);
else TerminateProcess(p->hProc, code);
GetExitCodeProcess(p->hProc, &e);
CloseHandle(p->hProc);
CloseHandle(p->hThread);
p->hThread = p->hProc = 0;
HANDLE* h = &p->inRd;
for (int i=0; i<6; i++, h++) {
if (!*h) continue;
CloseHandle(*h);
*h=NULL;
}
return e;
}

static DWORD Process32Open (PyProcess* p, LPCTSTR cmd) {
PROCESS_INFORMATION pi;
STARTUPINFO si;
SECURITY_ATTRIBUTES sa;
sa.nLength = sizeof(sa);
sa.bInheritHandle = TRUE;
sa.lpSecurityDescriptor = NULL;
CreatePipe(&p->inRd, &p->inWr, &sa, 0);
CreatePipe(&p->outRd, &p->outWr, &sa, 0);
if (!p->inRd || !p->inWr || !p->outRd || !p->outWr) return GetLastError();
SetHandleInformation(p->inWr, HANDLE_FLAG_INHERIT, FALSE);
SetHandleInformation(p->outRd, HANDLE_FLAG_INHERIT, FALSE);
ZeroMemory(&si, sizeof(si));
ZeroMemory(&pi, sizeof(pi));
si.cb = sizeof(si);
si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
si.wShowWindow = SW_HIDE;
si.hStdInput = p->inRd;
si.hStdOutput = si.hStdError = p->outWr;
if (!CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
DWORD e = GetLastError();
CloseHandle(p->inRd); CloseHandle(p->inWr);
CloseHandle(p->outRd); CloseHandle(p->outWr);
return e;
}
p->hProc = pi.hProcess;
p->hThread = pi.hThread;
return 0;
}

static void Process32Flush (PyProcess* p) {
if (!p || !p->inWr || !p->hProc) return;
FlushFileBuffers(p->inWr);
Sleep(1);
}

static int Process32Write (PyProcess* p, const void* buf, int len) {
if (!p || !p->inWr || !p->hProc) return -1;
int nWritten=0;
WriteFile(p->inWr, buf, len, &nWritten, NULL);
return nWritten;
}

static int Process32Read (PyProcess* p, void* buf, int len) {
if (!p || !p->outRd || !p->hProc) return -1;
int nRead=0;
ReadFile(p->outRd, buf, len, &nRead, NULL);
return nRead;
}

static int Process32ReadLine (PyProcess* p, const void** buf, int* len) {
if (!p || !p->outRd || !p->hProc) return -1;
int n=0, cap=32, nRead = 0;
char c=0, *s = malloc(cap);
while (GetFileSize(p->outRd,NULL)>0 && ReadFile(p->outRd, &c, 1, &nRead, NULL) && nRead>0) {
if (c!='\r') s[n++] = c;
if (c=='\n') break;
if (n>=cap) s = realloc(s, cap = cap*3/2 +1);
}
s[n]=0;
if (nRead<=0) { free(s); s=0; }
if (buf) *buf = s;
if (len) *len = n;
return n;
}

static int Process32ReadAll (PyProcess* p, const void** buf, int* len) {
if (!p || !p->outRd || !p->hProc) return -1;
int n=0, cap=1375, nRead;
char* s = malloc(cap);
while (GetFileSize(p->outRd,NULL)>0 && ReadFile(p->outRd, s+n, cap-n, &nRead, NULL) && nRead>0) {
n+=nRead;
if (n>=cap) s = realloc(s, cap = cap*3/2 +1);
else break;
}
s[n]=0;
if (buf) *buf = s;
if (len) *len=n;
return n;
}

static PyProcess* PyProcessNew (PyTypeObject* type, PyObject* args, PyObject* kwds) {
PyProcess* self = (PyProcess*)(type->tp_alloc(type, 0));
self->binary = FALSE;
self->encoding = NULL;
self->inRd = self->inWr = self->outRd = self->outWr = self->hProc = self->hThread = NULL;
LPCTSTR cmd = NULL;
DWORD result=0;
static const char* KWDS[] = {"command", "encoding", "binary", NULL};
if (!PyArg_ParseTupleAndKeywords(args, kwds, "u|$sp", KWDS, &cmd, &self->encoding, &self->binary)) return NULL;
if (self->encoding) self->encoding = strdup(self->encoding);
Py_BEGIN_ALLOW_THREADS
result = Process32Open(self, cmd);
Py_END_ALLOW_THREADS
if (result) {
char emsg[100]={0};
snprintf(emsg, 99, "Couldn't start process: OS error %d", result);
PyErr_SetString(PyExc_ValueError, emsg);
return NULL;
}
return self;
}

static int PyProcessInit (PyProcess* self, PyObject* args, PyObject* kwds) {
return 0;
}

static void PyProcessDealloc (PyObject* pySelf) {
PyProcess* self = (PyProcess*)pySelf;
if (self->encoding) free(self->encoding);
Py_TYPE(pySelf)->tp_free(pySelf);
}

static PyObject* PyProcWrite (PyProcess* p, PyObject* args) {
const char* buf = NULL;
int len = 0, re=0;
if (!PyArg_ParseTuple(args, "et#", p->encoding, &buf, &len)) return NULL;
Py_BEGIN_ALLOW_THREADS
re = Process32Write(p, buf, len);
Py_END_ALLOW_THREADS
PyMem_Free(buf);
return Py_BuildValue("i", re);
}

static PyObject* PyProcFlush  (PyProcess* p, PyObject* unused) {
Py_BEGIN_ALLOW_THREADS
Process32Flush(p);
Py_END_ALLOW_THREADS
Py_RETURN_NONE;
}

static PyObject* PyProcRead (PyProcess* p, PyObject* args) {
const char* buf = NULL;
int len = 0, re=0, requestedLen=-1;
if (!PyArg_ParseTuple(args, "|i", &requestedLen)) return NULL;
Py_BEGIN_ALLOW_THREADS
if (requestedLen<0) re = Process32ReadAll(p, &buf, &len);
else {
buf = malloc(requestedLen);
re = len = Process32Read(p, buf, requestedLen);
}
Py_END_ALLOW_THREADS
PyObject* val = NULL;
if (!buf || re<0 || len<0) { Py_INCREF(Py_None); val = Py_None; }
if (p->binary) val = Py_BuildValue("y#", buf, len);
else if (p->encoding) val = PyUnicode_Decode(buf, len, p->encoding, NULL);
else val = Py_BuildValue("s#", buf, len);
if (buf) free(buf);
return val;
}

static PyObject* PyProcReadLine (PyProcess* p, PyObject* unused) {
const char* buf = NULL;
int len = 0, re=0;
Py_BEGIN_ALLOW_THREADS
re = Process32ReadLine(p, &buf, &len);
Py_END_ALLOW_THREADS
PyObject* val = NULL;
if (!buf || re<0 || len<0) { Py_INCREF(Py_None); val = Py_None; }
if (p->binary) val = Py_BuildValue("y#", buf, len);
else if (p->encoding) val = PyUnicode_Decode(buf, len, p->encoding, NULL);
else val = Py_BuildValue("s#", buf, len);
if (buf) free(buf);
return val;
}

static PyObject* PyProcWait (PyProcess* p, PyObject* unused) {
Py_BEGIN_ALLOW_THREADS
Process32Wait(p);
Py_END_ALLOW_THREADS
Py_INCREF(p);
return p;
}

static PyObject* PyProcTerm (PyProcess* p, PyObject* args) {
int re = -1, code=0;
if (!PyArg_ParseTuple("|i", &code)) return NULL;
Py_BEGIN_ALLOW_THREADS
re = Process32End(p,code);
printf("Exit value = %d\r\n", re);
Py_END_ALLOW_THREADS
return Py_BuildValue("i",re);
}

static PyObject* PyNoOp (PyObject* o, PyObject* args) {
Py_RETURN_NONE;
}

static PyMethodDef PyProcessMethods[] = {
{"write", PyProcWrite, METH_VARARGS, NULL},
{"read", PyProcRead, METH_VARARGS, NULL},
{"readline", PyProcReadLine, METH_VARARGS, NULL},
{"wait", PyProcWait, METH_VARARGS, NULL},
{"flush", PyProcFlush, METH_VARARGS, NULL},
{"close", PyProcTerm, METH_VARARGS, NULL},
{"__enter__", PyNoOp, METH_VARARGS, NULL},
{"__exit__", PyProcTerm, METH_VARARGS, NULL},
{0,0,0,0}
};

static PyGetSetDef PyProcessAccessors[] = {
{0,0,0,0}
};

static PyTypeObject PyProcessType = { 
    PyVarObject_HEAD_INIT(NULL, 0) 
    "process32.Process",             /* tp_name */ 
    sizeof(PyProcess), /* tp_basicsize */ 
    0,                         /* tp_itemsize */ 
    PyProcessDealloc,                         /* tp_dealloc */ 
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
    PyProcessMethods,             /* tp_methods */ 
NULL,             /* tp_members */ 
    PyProcessAccessors,                         /* tp_getset */ 
    0,                         /* tp_base */ 
    0,                         /* tp_dict */ 
    0,                         /* tp_descr_get */ 
    0,                         /* tp_descr_set */ 
    0,                         /* tp_dictoffset */ 
    (initproc)PyProcessInit,      /* tp_init */ 
    0,                         /* tp_alloc */ 
};

static PyMethodDef SubprocessMainDefs [] = {
{0, 0, 0, 0}
}; 

static PyModuleDef SubprocessMod = {
PyModuleDef_HEAD_INIT,
"process32",
NULL, -1, SubprocessMainDefs  
};

PyMODINIT_FUNC PyInit_process32 (void) {
PyObject* mod = PyModule_Create(&SubprocessMod);
PyProcessType.tp_new = (typeof(PyProcessType.tp_new))PyProcessNew;
if (PyType_Ready(&PyProcessType) < 0)          return FALSE;
Py_INCREF(&PyProcessType); 
PyModule_AddObject(mod, "Process", (PyObject*)&PyProcessType); 
return mod;
}

