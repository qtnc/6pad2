#include "global.h"
#include "python34.h"

tstring decodeFromPythonEncoding (const string& str, int offset, const char* encoding) {
GIL_PROTECT
PyObject* obj = PyUnicode_Decode(str.data()+offset, str.size()-offset, encoding, NULL);
if (!obj) { PyErr_Clear(); return TEXT(""); }
tstring re = PyUnicode_AsUnicode(obj);
Py_DECREF(obj);
return re;
}

string encodeToPythonEncoding (const tstring& str, const string& prefix, const char* encoding) {
GIL_PROTECT
string re;
char* buf=0; size_t len=0;
PyObject* uStr = Py_BuildValue("(u)", str.data() );
if (PyArg_ParseTuple(uStr, "et#", encoding, &buf, &len)) {
if (buf&&len>0) re = string(buf, buf+len);
}
else PyErr_Print();
Py_DECREF(uStr);
return prefix+re;
}
