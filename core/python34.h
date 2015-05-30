#ifndef ___PYTHON___HPP___9
#define ___PYTHON___HPP___9
#include<python/python.h>
#include "strings.hpp"
#include "variant.h"
#include<tuple>
#include<functional>
#include<type_traits>
#include<string>

#define GIL_PROTECT RAII_GIL ___RAII_GIL_VAR_##__LINE__; 

typedef PyObject*(*PyCFunc)(PyObject*,PyObject*);

struct RAII_GIL {
PyGILState_STATE gil;
RAII_GIL():  gil(PyGILState_Ensure()) { }
~RAII_GIL(){  PyGILState_Release(gil); }
};

struct PyObjectWithDic {
    PyObject_HEAD 
PyObject* dic;
};

void PyStart (void);

///Automatic wrappers!

template<class T> struct PyTypeSpec {  };

template<> struct PyTypeSpec<int> { 
typedef int type;
static constexpr const char c = 'i'; 
static inline int convert (int i) { return i; }
static inline int convert2 (int i) { return i; }
static inline int convert3 (PyObject* o) { 
if (!PyLong_Check(o)) { PyErr_SetString(PyExc_TypeError, "int expected"); return 0; }
return PyLong_AsLong(o); 
}
};

template<> struct PyTypeSpec<DWORD> { 
typedef DWORD type;
static constexpr const char c = 'I'; 
static inline DWORD convert (DWORD i) { return i; }
static inline DWORD convert2 (DWORD i) { return i; }
static inline DWORD convert3 (PyObject* o) { 
if (!PyLong_Check(o)) { PyErr_SetString(PyExc_TypeError, "int expected"); return 0; }
return PyLong_AsLongLong(o); 
}
};

template<> struct PyTypeSpec<size_t> { 
typedef size_t type;
static constexpr const char c = 'n'; 
static inline size_t convert (size_t i) { return i; }
static inline size_t convert2 (size_t i) { return i; }
static inline int convert3 (PyObject* o) { 
if (!PyLong_Check(o)) { PyErr_SetString(PyExc_TypeError, "int expected"); return 0; }
return PyLong_AsLongLong(o); 
}
};

template<> struct PyTypeSpec<double> { 
typedef double type;
static constexpr const char c = 'd'; 
static inline double convert (double i) { return i; }
static inline double convert2 (double i) { return i; }
static inline double convert3 (PyObject* o) { 
if (!PyFloat_Check(o)) { PyErr_SetString(PyExc_TypeError, "float expected"); return 0; }
return PyFloat_AsDouble(o); 
}
};

template<> struct PyTypeSpec<bool> { 
typedef bool type;
static constexpr const char c = 'p'; 
static inline bool convert (bool i) { return i; }
static inline bool convert2 (bool i) { return i; }
static inline bool convert3 (PyObject* o) {
if (o==Py_True) return true;
else if (o==Py_False) return false; 
else PyErr_SetString(PyExc_TypeError, "bool expected");
return false;
}
};

template<> struct PyTypeSpec<std::string> { 
typedef char* type;
static constexpr const char c = 's'; 
static inline std::string convert (const char* s) { return s; }
static inline const char* convert2 (const std::string& s) { return s.c_str(); }
static inline string convert3 (PyObject* o) { 
if (!PyUnicode_Check(o)) { PyErr_SetString(PyExc_TypeError, "str expected"); return ""; }
return toString(PyUnicode_AsUnicode(o)); 
}
};

template<> struct PyTypeSpec<const std::string&> { 
typedef char* type;
static constexpr const char c = 's'; 
static inline std::string convert (const char* s) { return s; }
static inline const char* convert2 (const std::string& s) { return s.c_str(); }
static inline string convert3 (PyObject* o) { 
if (!PyUnicode_Check(o)) { PyErr_SetString(PyExc_TypeError, "str expected"); return ""; }
return toString(PyUnicode_AsUnicode(o)); 
}
};

template<> struct PyTypeSpec<std::wstring> { 
typedef wchar_t* type;
static constexpr const char c = 'u'; 
static inline std::wstring convert (const wchar_t* s) { return s; }
static inline const wchar_t* convert2 (const std::wstring& s) { return s.c_str(); }
static inline wstring convert3 (PyObject* o) { 
if (!PyUnicode_Check(o)) { PyErr_SetString(PyExc_TypeError, "str expected"); return L""; }
return toWString(PyUnicode_AsUnicode(o)); 
}
};

template<> struct PyTypeSpec<const std::wstring&> { 
typedef wchar_t* type;
static constexpr const char c = 'u'; 
static inline std::wstring convert (const wchar_t* s) { return s; 
}
static inline const wchar_t* convert2 (const std::wstring& s) { return s.c_str(); }
static inline wstring convert3 (PyObject* o) { 
if (!PyUnicode_Check(o)) { PyErr_SetString(PyExc_TypeError, "str expected"); return L""; }
return toWString(PyUnicode_AsUnicode(o)); 
}
};

template<> struct PyTypeSpec<const char*> { 
typedef const char* type;
static constexpr const char c = 's'; 
static inline const char* convert (const char* s) { return s; }
static inline const char* convert2 (const char*  s) { return s; }
};

template<> struct PyTypeSpec<const wchar_t*> { 
typedef const wchar_t* type;
static constexpr const char c = 'u'; 
static inline const wchar_t* convert (const wchar_t* s) { return s; }
static inline const wchar_t* convert2 (const wchar_t*  s) { return s; }
};

template<> struct PyTypeSpec<var> {
typedef PyObject* type;
static constexpr const char c = 'O'; 
static PyObject* convert2 (var v) { 
switch(v.getType()){
case T_INT: return Py_BuildValue("i", v.toInt());
case T_STR: return Py_BuildValue(Py_TString_Decl, v.toTString().c_str() );
case T_BOOL:
if (v) Py_RETURN_TRUE;
else Py_RETURN_FALSE;
default: Py_RETURN_NONE;
}}
static var convert3 (PyObject* o) {
if (o==Py_None) return var();
else if (o==Py_True) return true;
else if (o==Py_False) return false;
else if (PyLong_Check(o)) return (int)(PyLong_AsLong(o));
else if (PyUnicode_Check(o)) return toTString(PyUnicode_AsUnicode(o));
else PyErr_SetString(PyExc_TypeError, "none, bool, int or str  expected"); 
return var();
}};

template<> struct PyTypeSpec<PyObject*> { 
typedef PyObject* type;
static constexpr const char c = 'O'; 
static inline PyObject* convert (PyObject* i) { return i; }
static inline PyObject* convert2 (PyObject* i) { return i; }
static inline PyObject* convert3 (PyObject* o) { return o; }
};

template<class... Args> inline const char* PyTypeSpecs (void) {
static constexpr const int n = sizeof...(Args);
static constexpr const char cc[n+1] = { PyTypeSpec<Args>::c... ,0};
return cc;
};

template<class... Args> inline const char* PyTypeSpecsTuple (void) {
static constexpr const int n = sizeof...(Args);
static constexpr const char cc[n+3] = { '(', PyTypeSpec<Args>::c... , ')', 0};
return cc;
};

template<class S> struct PyCallback {
};

struct PySafeObject {
PyObject* o;
inline PySafeObject (): o(0) {}
inline PySafeObject (PyObject* x): o(0) { operator=(x); }
inline PySafeObject (const PySafeObject& x): PySafeObject(x.o) {}
inline PySafeObject& operator= (const PySafeObject& x) { return operator=(x.o); }
PySafeObject& operator= (PyObject* x) {
GIL_PROTECT
Py_XINCREF(x);
Py_XDECREF(o);
o=x;
return *this;
}
PySafeObject& operator= (PySafeObject&& x) {
if (this==&x) return *this;
GIL_PROTECT;
Py_XDECREF(o);
o = x.o;
x.o = 0;
return *this;
}
inline PySafeObject (PySafeObject&& x): o(x.o) { x.o=0; }
inline ~PySafeObject  () { operator=(NULL); }
inline bool operator== (PyObject* x) const {  return x==o;  }
inline bool operator== (const PySafeObject& x) const {  return x.o==o;  }
inline PyObject* operator* () const { return o; }
inline operator bool () const { return !!o && o!=Py_None && o!=Py_False; }
template<class S> inline PyCallback<S> asFunction () const;
};

template<class R, class... A> struct PyCallback<R(A...)> {
PySafeObject func;
static R call (PyObject* f, A... args) {
GIL_PROTECT
PyObject* argtuple = Py_BuildValue(PyTypeSpecsTuple<A...>(), PyTypeSpec<A>::convert2(args)...);
PyObject* pyResult = PyObject_CallObject(f, argtuple);
if (!pyResult) PyErr_Print();
Py_XDECREF(argtuple);
R cResult = PyTypeSpec<R>::convert3(pyResult);
Py_XDECREF(pyResult);
return cResult;
}
static R callMethod (PyObject* obj, const string& name, A... args) {
GIL_PROTECT
PyObject* pyResult = PyObject_CallMethod(obj, name.c_str(), PyTypeSpecsTuple<A...>(), PyTypeSpec<A>::convert2(args)...);
if (!pyResult) PyErr_Print();
R cResult = PyTypeSpec<R>::convert3(pyResult);
Py_XDECREF(pyResult);
return cResult;
}
inline PyCallback<R(A...)> (const PySafeObject& o): func(o) {}
inline operator bool () const { return func; }
inline bool operator== (const PyCallback<R(A...)>& x) const { 
printf("operator==: %p, %p\r\n", func.o, x.func.o);
return x.func==func; 
}
inline bool operator!= (const PyCallback<R(A...)>& x) const { return !operator==(x); }
inline R operator() (A... args) { return call(*func, args...); }
};

template<class... A> struct PyCallback<void(A...)> {
PySafeObject func;
static void call (PyObject* f, A... args) {
GIL_PROTECT
PyObject* argtuple = Py_BuildValue(PyTypeSpecsTuple<A...>(), PyTypeSpec<A>::convert2(args)...);
PyObject* pyResult = PyObject_CallObject(f, argtuple);
if (!pyResult) PyErr_Print();
Py_XDECREF(argtuple);
Py_XDECREF(pyResult);
}
static void callMethod (PyObject* obj, const string& name, A... args) {
GIL_PROTECT
PyObject* pyResult = PyObject_CallMethod(obj, name.c_str(), PyTypeSpecsTuple<A...>(), PyTypeSpec<A>::convert2(args)...);
if (!pyResult) PyErr_Print();
Py_XDECREF(pyResult);
}
inline PyCallback<void(A...)> (const PySafeObject& o): func(o) {}
inline operator bool () const { return func; }
inline bool operator== (const PyCallback<void(A...)>& x) const { 
printf("operator==: %p, %p\r\n", func.o, x.func.o);
return x.func==func; 
}
inline bool operator!= (const PyCallback<void(A...)>& x) const { return !operator==(x); }
inline void operator() (A... args) { call(*func, args...); }
};

template<> struct PyTypeSpec<PySafeObject> { 
typedef PyObject* type;
static constexpr const char c = 'O'; 
static inline PySafeObject convert (PyObject* i) { return i; }
static inline PyObject* convert2 (const PySafeObject& i) { return i.o; }
static inline PySafeObject convert3 (PyObject* o) {  return o;  }
};

template<> struct PyTypeSpec<const PySafeObject&> { 
typedef PyObject* type;
static constexpr const char c = 'O'; 
static inline PySafeObject convert (PyObject* i) { return i; }
static inline PyObject* convert2 (const PySafeObject& i) { return i.o; }
static inline PySafeObject convert3 (PyObject* o) {  return o;  }
};

template<class S> inline PyCallback<S> PySafeObject::asFunction () const {
return PyCallback<S>(*this);
}

template<class R, class... A> inline R CallMethod (PyObject* obj, const string& name, A... args) {
return PyCallback<R(A...)>::callMethod(obj, name, args...);
}

template<int... S> struct TemplateSequence {};
template<int N, int... S> struct TemplateSequenceGenerator: TemplateSequenceGenerator<N -1, N -1, S...> {};
template<int... S> struct TemplateSequenceGenerator<0, S...> { typedef TemplateSequence<S...> sequence; };

template<class... A> struct PyParseTupleSpec {
template<int... S> static int PyArg_ParseTuple (TemplateSequence<S...> seq, PyObject* pyTuple, const char* pyArgSpec, std::tuple<typename PyTypeSpec<A>::type...>& args) { return ::PyArg_ParseTuple(pyTuple, pyArgSpec, &std::get<S>(args)...); }
};

template<class R, class... A> struct PyCTupleCallSpec {
template<int... S> static R call (TemplateSequence<S...> seq, R(*f)(A...), std::tuple<typename PyTypeSpec<A>::type...>& args) {   return f(  PyTypeSpec<typename std::tuple_element<S, std::tuple<A...>>::type>::convert(std::get<S>(args))...);  }
template<int... S> static R call (TemplateSequence<S...> seq, R(__stdcall *f)(A...), std::tuple<typename PyTypeSpec<A>::type...>& args) {   return f(  PyTypeSpec<typename std::tuple_element<S, std::tuple<A...>>::type>::convert(std::get<S>(args))...);  }
template<class C, int... S> static R callmeth (TemplateSequence<S...> seq, C& c, R(C::*f)(A...), std::tuple<typename PyTypeSpec<A>::type...>& args) {   return (c.*f)(  PyTypeSpec<typename std::tuple_element<S, std::tuple<A...>>::type>::convert(std::get<S>(args))...);  }
};

template<class... A> struct PyCTupleCallSpec<void, A...> {
template<int... S> static void call (TemplateSequence<S...> seq, void(*f)(A...), std::tuple<typename PyTypeSpec<A>::type...>& args) {  f(  PyTypeSpec<typename std::tuple_element<S, std::tuple<A...>>::type>::convert(std::get<S>(args))...);  }
template<int... S> static void call (TemplateSequence<S...> seq, void(__stdcall *f)(A...), std::tuple<typename PyTypeSpec<A>::type...>& args) {  f(  PyTypeSpec<typename std::tuple_element<S, std::tuple<A...>>::type>::convert(std::get<S>(args))...);  }
template<class C, int... S> static void callmeth (TemplateSequence<S...> seq, C& c, void(C::*f)(A...), std::tuple<typename PyTypeSpec<A>::type...>& args) {   (c.*f)(  PyTypeSpec<typename std::tuple_element<S, std::tuple<A...>>::type>::convert(std::get<S>(args))...);  }
};

template<class CFunc> struct PyFuncSpec {
template<class R, class... A> static inline PyObject* func2 (R(*cfunc)(A...), PyObject* pySelf, PyObject* pyArgs) {
typename TemplateSequenceGenerator<sizeof...(A)>::sequence seq;
std::tuple<typename PyTypeSpec<A>::type...> argtuple;
if (!PyParseTupleSpec<A...>::PyArg_ParseTuple(seq, pyArgs, PyTypeSpecs<A...>(), argtuple)) return NULL;
R result = PyCTupleCallSpec<R,A...>::call(seq, cfunc, argtuple);
return Py_BuildValue(PyTypeSpecs<R>(), PyTypeSpec<R>::convert2(result) );
}
template<class... A> static inline PyObject* func2 (void(*cfunc)(A...), PyObject* pySelf, PyObject* pyArgs) {
typename TemplateSequenceGenerator<sizeof...(A)>::sequence seq;
std::tuple<typename PyTypeSpec<A>::type...> argtuple;
if (!PyParseTupleSpec<A...>::PyArg_ParseTuple(seq, pyArgs, PyTypeSpecs<A...>(), argtuple)) return NULL;
PyCTupleCallSpec<void,A...>::call(seq, cfunc, argtuple);
Py_RETURN_NONE;
}
template<class R, class... A> static inline PyObject* func2 (R(__stdcall *cfunc)(A...), PyObject* pySelf, PyObject* pyArgs) {
typename TemplateSequenceGenerator<sizeof...(A)>::sequence seq;
std::tuple<typename PyTypeSpec<A>::type...> argtuple;
if (!PyParseTupleSpec<A...>::PyArg_ParseTuple(seq, pyArgs, PyTypeSpecs<A...>(), argtuple)) return NULL;
R result = PyCTupleCallSpec<R,A...>::call(seq, cfunc, argtuple);
return Py_BuildValue(PyTypeSpecs<R>(), PyTypeSpec<R>::convert2(result) );
}
template<class... A> static inline PyObject* func2 (void(*__stdcall cfunc)(A...), PyObject* pySelf, PyObject* pyArgs) {
typename TemplateSequenceGenerator<sizeof...(A)>::sequence seq;
std::tuple<typename PyTypeSpec<A>::type...> argtuple;
if (!PyParseTupleSpec<A...>::PyArg_ParseTuple(seq, pyArgs, PyTypeSpecs<A...>(), argtuple)) return NULL;
PyCTupleCallSpec<void,A...>::call(seq, cfunc, argtuple);
Py_RETURN_NONE;
}
template<class R, class O, class... A> static inline PyObject* func2 (R(O::*cfunc)(A...), PyObject* pySelf, PyObject* pyArgs) {
typename TemplateSequenceGenerator<sizeof...(A)>::sequence seq;
std::tuple<typename PyTypeSpec<A>::type...> argtuple;
if (!PyParseTupleSpec<A...>::PyArg_ParseTuple(seq, pyArgs, PyTypeSpecs<A...>(), argtuple)) return NULL;
R result = PyCTupleCallSpec<R,A...>::callmeth(seq, *(O*)(pySelf), cfunc, argtuple);
return Py_BuildValue(PyTypeSpecs<R>(), PyTypeSpec<R>::convert2(result) );
}
template<class O, class... A> static inline PyObject* func2 (void(O::*cfunc)(A...), PyObject* pySelf, PyObject* pyArgs) {
typename TemplateSequenceGenerator<sizeof...(A)>::sequence seq;
std::tuple<typename PyTypeSpec<A>::type...> argtuple;
if (!PyParseTupleSpec<A...>::PyArg_ParseTuple(seq, pyArgs, PyTypeSpecs<A...>(), argtuple)) return NULL;
PyCTupleCallSpec<void,A...>::callmeth(seq, *(O*)(pySelf), cfunc, argtuple);
Py_RETURN_NONE;
}
template<CFunc cfunc> static PyObject* func (PyObject* pySelf, PyObject* pyArgs) { return func2(cfunc, pySelf, pyArgs); }
};

template<class S> struct PySetterSpec {
template<class O, class A> static inline int set2 (void(O::*setf)(A), PyObject* self, PyObject* pyVal) {
A cVal = PyTypeSpec<A>::convert3(pyVal);
if (PyErr_Occurred()) return -1;
((*(O*)self).*setf)(cVal);
return 0;
}
template<class A> static inline int set2 (void(*setf)(A), PyObject* self, PyObject* pyVal) {
A cVal = PyTypeSpec<A>::convert3(pyVal);
if (PyErr_Occurred()) return -1;
setf(cVal);
return 0;
}
template<S setf> static inline int setter (PyObject* self, PyObject* val, void* unused) { return set2(setf, self, val); }
};

template<class G> struct PyGetterSpec {
template<class O, class A> static inline PyObject* get2 (A(O::*getf)(void), PyObject* self ) {
A result = ((*(O*)self).*getf)();
return Py_BuildValue(PyTypeSpecs<A>(), PyTypeSpec<A>::convert2(result) );
}
template<class A> static inline PyObject* get2 (A(*getf)(void), PyObject* self ) {
A result = getf();
return Py_BuildValue(PyTypeSpecs<A>(), PyTypeSpec<A>::convert2(result) );
}
template<G getf> static inline PyObject* getter (PyObject* self, void* unused) { return get2(getf, self); }
};

#define PyToCType(t,x) (PyTypeSpec<t>::convert3(x))
#define PyToPyType(x) (Py_BuildValue(PyTypeSpecs<decltype(x)>(), PyTypeSpec<decltype(x)>::convert2(x)))
#define PyDecl(n,f) {(n), (PyFuncSpec<decltype(f)>::func<f>), METH_VARARGS, NULL}
#define PyAccessor(n,g,s) {(n), (PyGetterSpec<decltype(g)>::getter<g>), (PySetterSpec<decltype(s)>::setter<s>), NULL, NULL}
#define PyWriteOnlyAccessor(n,s) {(n), NULL, (PySetterSpec<decltype(s)>::setter<s>), NULL, NULL}
#define PyReadOnlyAccessor(n,g) {(n), (PyGetterSpec<decltype(g)>::getter<g>), NULL, NULL, NULL}
#define PyDeclEnd {0, 0, 0, 0}

#endif
