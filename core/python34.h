#ifndef ___PYTHON___HPP___9
#define ___PYTHON___HPP___9
#include<python/python.h>
#include "strings.hpp"
#include<tuple>
#include<functional>
#include<type_traits>
#include<string>
#include "any.h"

#define GIL_PROTECT RAII_GIL ___RAII_GIL_VAR_##__LINE__; 

typedef PyObject*(*PyCFunc)(PyObject*,PyObject*);

struct RAII_GIL {
PyGILState_STATE gil;
RAII_GIL():  gil(PyGILState_Ensure()) { }
~RAII_GIL(){  PyGILState_Release(gil); }
};

struct PyObjectBase {
    PyObject_HEAD 
};

struct PyObjectWithDic: PyObjectBase {
PyObject* dic;
};

void PyStart (void);

struct ___DOLLAR___ {};

template<int N, class S, class... T> struct NthType {
typedef typename NthType<N -1, T...>::type type;
};

template<class S, class... T> struct NthType<0,S,T...> {
typedef S type;
};

template<int... S> struct TemplateSequence {};
template<int N, int... S> struct TemplateSequenceGenerator: TemplateSequenceGenerator<N -1, N -1, S...> {};
template<int... S> struct TemplateSequenceGenerator<0, S...> { typedef TemplateSequence<S...> sequence; };

template<class _> struct TemplatedZero {
static constexpr PyObject* zero = 0;
};

template <char... c> static inline const char* TemplatedString () {
static const char cc[sizeof...(c) +1] = { c..., 0 };
return cc;
}

template<class T> struct PyTypeSpec {
static constexpr const bool ignore = false;
static constexpr const char inLetter = 'N';
static constexpr const char outLetter = 'O';
};

template<> struct PyTypeSpec<nullptr_t> {
static constexpr const bool ignore = true;
static constexpr const char outLetter = '|';
};

template<> struct PyTypeSpec<___DOLLAR___> {
static constexpr const bool ignore = true;
static constexpr const char outLetter = '$';
};

template<class T> struct PyConverter { };

#define PyCheck(F,N,O) if (!O) return type(); \
if (!F(O)) { PyErr_Format(PyExc_TypeError, "%s expected, got %s", #N, O?O->ob_type->tp_name:"NULL"); return type(); }

#define PySeqCheck(N,O) if (!O) return type(); \
if (!PySequence_Check(O) || PySequence_Size(O)!=N) { PyErr_Format(PyExc_TypeError, "sequence of size %d expected, got %s", N, O?O->ob_type->tp_name:"NULL"); return type(); }

#define C(T,N,P,V,K)  template<> struct PyConverter<T> { \
typedef T type; \
static inline T outCast (PyObject* o) { \
PyCheck(K, #N, o)\
return V(o); \
} \
static inline PyObject* inCast (const T& x) { return P(x); } \
};

C(int, int, PyLong_FromLong, PyLong_AsLong, PyLong_Check)
C(DWORD, int, PyLong_FromLongLong, PyLong_AsLongLong, PyLong_Check)
C(size_t, int, PyLong_FromLongLong, PyLong_AsLongLong, PyLong_Check)
C(bool, bool, PyBool_FromLong, PyObject_IsTrue, PyLong_Check)
C(double, float, PyFloat_FromDouble, PyFloat_AsDouble, PyFloat_Check)
#undef C

#define C(T) template<> struct PyConverter<T> { \
static inline PyObject* inCast (const T& x) { return 0; } \
static inline T outCast (PyObject* x) { return T(); } \
};

C(std::nullptr_t)
C(___DOLLAR___)
#undef C

template<> struct PyConverter<PyObject*> {
static inline PyObject* inCast (PyObject* o) { 
Py_XINCREF(o);
return o; 
}
static inline PyObject* outCast (PyObject* o) { return o; }
};

template<> struct PyConverter<std::wstring> {
static inline std::wstring outCast (PyObject* o) { 
if (!o) return L"";
else if (PyUnicode_Check(o)) return PyUnicode_AsUnicode(o);
o = PyObject_Str(o);
tstring re = PyUnicode_AsUnicode(o);
Py_XDECREF(o);
return re;
}
static inline PyObject* inCast (const std::wstring& x) { return PyUnicode_FromUnicode(x.c_str(), x.size()); }
};

template<> struct PyConverter<std::string> {
static inline std::string outCast (PyObject* o) { 
if (!o) return "";
else return toString(PyConverter<std::wstring>::outCast(o)); 
}
static inline PyObject* inCast (const string& s) { return PyUnicode_FromString(s.c_str()); }
};

template<> struct PyConverter<const wchar_t*> {
static inline const wchar_t* outCast (PyObject* o) { 
if (!o) return nullptr;
static std::wstring s;
s = PyConverter<std::wstring>::outCast(o);
return s.c_str();
}
static inline PyObject* inCast (const wchar_t* s) { return PyUnicode_FromUnicode(s, wcslen(s)); }
};

template<> struct PyConverter<const char*> {
static inline const char* outCast (PyObject* o) { 
if (!o) return nullptr;
static std::string s;
s = PyConverter<std::string>::outCast(o);
return s.c_str();
}
static inline PyObject* inCast (const char* s) { return PyUnicode_FromString(s); }
};

template<> struct PyConverter<const std::string&>: PyConverter<std::string> {};
template<> struct PyConverter<const std::wstring&>: PyConverter<std::wstring> {};

struct PyGenericFunc {
PyObject* o;
};

struct PySafeObject {
PyObject* o;
inline PySafeObject (): o(0) {}
inline PySafeObject (PyObject* x, bool b=false): o(0) { operator=(x); }
inline PySafeObject (const PySafeObject& x): PySafeObject(x.o) {}
inline PySafeObject (PySafeObject&& x): o(x.o) { x.o=0; }
inline PySafeObject& operator= (const PySafeObject& x) { return operator=(x.o); }
PySafeObject& operator= (PyObject* x) {
GIL_PROTECT
Py_XINCREF(x);
Py_XDECREF(o);
o=x;
return *this;
}
PySafeObject& operator= (PySafeObject&& x) {
if (o) {
if (o==x.o) return *this;
GIL_PROTECT;
Py_XDECREF(o);
}
o = x.o;
x.o=0;
return *this;
}
inline ~PySafeObject  () {
if (o) {
GIL_PROTECT
Py_XDECREF(o);
}}
inline PySafeObject& assign (PyObject* x, bool b = false) {
if (!b) return operator=(x);
if (o) {
GIL_PROTECT
Py_XDECREF(o);
}
o=x;
return *this;
}
inline void incref () {
if (!o) return;
GIL_PROTECT
Py_XINCREF(o);
}
inline void decref () {
if (!o) return;
GIL_PROTECT
Py_XDECREF(o);
}
inline bool operator== (PyObject* x) const {  return x==o;  }
inline bool operator== (const PySafeObject& x) const {  return x.o==o;  }
inline PyObject* operator* () const { return o; }
inline operator bool () const { return !!o && o!=Py_None && o!=Py_False; }
};

template<class S> struct PyFunc {};

template<class R, class... A> struct PyFunc <R(A...)> {
PySafeObject func;
static R call (PyObject* f, A... arg) {
GIL_PROTECT
PySafeObject argtuple( Py_BuildValue(TemplatedString<'(', PyTypeSpec<A>::inLetter..., ')'>(), PyConverter<A>::inCast(arg)...) ,true);
PySafeObject result( PyObject_CallObject(f, *argtuple), true);
if (!result) PyErr_Print();
return PyConverter<R>::outCast(*result);
}
static R callMethod (PyObject* obj, const string& name, A... arg) {
GIL_PROTECT
PySafeObject result( PyObject_CallMethod(obj, name.c_str(), TemplatedString<'(', PyTypeSpec<A>::inLetter..., ')'>(), PyConverter<A>::inCast(arg)...), true);
if (!result) PyErr_Print();
return PyConverter<R>::outCast(*result);
}
inline PyFunc<R(A...)> (): func() {}
inline PyFunc<R(A...)> (PyObject* o, bool b=false): func(o,b) {}
inline operator bool () const { return func; }
inline bool operator== (const PyFunc<R(A...)>& x) const {  return x.func==func;  }
inline bool operator!= (const PyFunc<R(A...)>& x) const { return !operator==(x); }
inline R operator() (A... args) { return call(*func, args...); }
};

template<class... A> struct PyFunc<void(A...)> {
PySafeObject func;
static void call (PyObject* f, A... arg) {
GIL_PROTECT
PySafeObject argtuple( Py_BuildValue(TemplatedString<'(', PyTypeSpec<A>::inLetter..., ')'>(), PyConverter<A>::inCast(arg)...) ,true);
PySafeObject result( PyObject_CallObject(f, *argtuple), true);
if (!result) PyErr_Print();
}
static void callMethod (PyObject* obj, const string& name, A... arg) {
GIL_PROTECT
PySafeObject result( PyObject_CallMethod(obj, name.c_str(), TemplatedString<'(', PyTypeSpec<A>::inLetter..., ')'>(), PyConverter<A>::inCast(arg)...), true);
if (!result) PyErr_Print();
}
inline PyFunc<void(A...)> (PyObject* o, bool b=false): func(o,b) {}
inline operator bool () const { return func; }
inline bool operator== (const PyFunc<void(A...)>& x) const {  return x.func==func;  }
inline bool operator!= (const PyFunc<void(A...)>& x) const { return !operator==(x); }
inline void operator() (A... args) { call(*func, args...); }
};

template<class S> inline PyFunc<S> AsPyFunc  (PyObject* o) {
return PyFunc<S>(o);
}

template<class R, class... A> inline R CallMethod (PyObject* obj, const string& name, A... args) {
return PyFunc<R(A...)>::callMethod(obj, name, args...);
}

template<> struct PyConverter<PySafeObject> {
static inline PySafeObject outCast (PyObject* o) { return o; }
static inline PyObject* inCast (const PySafeObject& o) { 
Py_XINCREF(*o);
return *o; 
}
};

template<class S> struct PyConverter<PyFunc<S>> {
static inline PyFunc<S>outCast (PyObject* o) { 
if (o && o!=Py_None && !PyCallable_Check(o)) PyErr_SetString(PyExc_TypeError, "object is not callable");
return o; 
}
static inline PyObject* inCast (const PyFunc<S>& O) { 
PyObject* o = O.func.o;
Py_XINCREF(o);
return o; 
}
};

template<> struct PyConverter<PyGenericFunc> {
static inline PyGenericFunc outCast (PyObject* o) { 
if (o && o!=Py_None && !PyCallable_Check(o)) PyErr_SetString(PyExc_TypeError, "object is not callable");
return {o}; 
}
static inline PyObject* inCast (const PyGenericFunc& O) { 
Py_XINCREF(O.o);
return O.o;
}
};

template<class E> struct PyConverter<std::vector<E>> {
typedef std::vector<E> type;
static PyObject* inCast (const std::vector<E>& v) { 
int len = v.size();
PyObject* list = PyList_New(len);
for (int i=0; i<len; i++) PyList_SetItem(list, i, PyConverter<E>::inCast(v[i]));
return list;
}
static std::vector<E> outCast (PyObject* o) {
PyCheck(PySequence_Check, sequence, o)
int len = PySequence_Size(o);
std::vector<E> v;
for (int i=0; i<len; i++) {
PyObject* x = PySequence_GetItem(o,i);
v.push_back( PyConverter<E>::outCast(x));
Py_XDECREF(x);
}
return v;
}
};

template<class E1, class E2> struct PyConverter<std::pair<E1,E2>> {
typedef std::pair<E1,E2> type;
static PyObject* inCast (const std::pair<E1, E2>& p) { 
return Py_BuildValue("(NN)", PyConverter<E1>::inCast(p.first), PyConverter<E2>::inCast(p.second) );
}
static std::pair<E1,E2> outCast (PyObject* o) {
PySeqCheck(2, o)
return { PyConverter<E1>::outCast( PySequence_GetItem(o,0) ), PyConverter<E2>::outCast( PySequence_GetItem(o,1) ) };
}
};

template<class E> struct PyConverter<optional<E>> {
static PyObject* inCast (const optional<E>& o) { 
if (o) return PyConverter<E>::inCast(*o);
else Py_RETURN_NONE;
}
static optional<E> outCast (PyObject* o) {
if (!o||o==Py_None) return none;
else return PyConverter<E>::outCast(o);
}
};

template<> struct PyConverter<any> {
static PyObject* inCast (const any& a) { 
if (a.empty() || isoftype(a, nullptr_t)) Py_RETURN_NONE;
#define T(...) else if (isoftype(a,##__VA_ARGS__)) return PyConverter<__VA_ARGS__>::inCast( any_cast<__VA_ARGS__>(a) );
T(int) T(std::wstring) T(std::string) T(bool)
T(vector<tstring>) T(vector<int>)
T(pair<tstring,int>) T(pair<vector<tstring>,int>)
#undef T
else Py_RETURN_NONE;
}
static any outCast (PyObject* o) {
if (!o||o==Py_None) return any();
else if (o==Py_True) return true;
else if (o==Py_False) return false;
else if (PyLong_Check(o)) return (int)(PyLong_AsLong(o));
else if (PyUnicode_Check(o)) return toTString(PyUnicode_AsUnicode(o));
else return o;
}
};

template <class E> struct PyConverter<const std::vector<E>&>: PyConverter<std::vector<E>> {};

template<class... E> struct PyConverter<std::tuple<E...>> {
typedef std::tuple<E...> type;
private:
typedef typename TemplateSequenceGenerator<sizeof...(E)>::sequence sequence;
template<int... S> static inline PyObject* inCast (TemplateSequence<S...> seq, const std::tuple<E...>& t) {
return Py_BuildValue(TemplatedString<'(', PyTypeSpec<E>::inLetter..., ')'>(), PyConverter<typename NthType<S, E...>::type>::inCast(std::get<S>(t))...);
}
template<int... S> static inline std::tuple<E...> outCast (PyObject* o) {
return { PyConverter<typename NthType<S, E...>::type>::outCast(PySequence_GetItem(o,S))... };
}
public:
static inline PyObject* inCast (const std::tuple<E...>& t) { 
return inCast(sequence(), t);
}
static std::tuple<E...> outCast (PyObject* o) {
PySeqCheck(sizeof...(E), o)
return outCast(sequence(), o);
}
};

template <class T> inline PyObject* toPyObject (const T& x) {
return PyConverter<T>::inCast(x);
}

template<class T> inline T fromPyObject (PyObject* o) {
return PyConverter<T>::outCast(o);
}

template<class... A> struct PyParseTupleSpec {
template<int... S> struct TemplateSequence2 {};
template<int N, int... S> struct TemplateSequenceGenerator2: TemplateSequenceGenerator2< 
PyTypeSpec<typename NthType<N-1,A...>::type>::ignore? N -2 : N -1,
PyTypeSpec<typename NthType<N-1,A...>::type>::ignore? N -2 : N -1, 
S...> {};
template<int... S> struct TemplateSequenceGenerator2<0, S...> { typedef TemplateSequence2<S...> sequence; };
template<int... S> struct TemplateSequenceGenerator2<-1, -1, S...> { typedef TemplateSequence2<S...> sequence; };
template<int... S> static int PyArg_ParseTuple (TemplateSequence2<S...> seq, PyObject* pyTuple, const char* pyArgSpec, PyObject** args) { return ::PyArg_ParseTuple(pyTuple, pyArgSpec, &args[S]...); }
template<int... S> static int PyArg_ParseTupleAndKeywords (TemplateSequence2<S...> seq, PyObject* pyTuple, PyObject* pyDict, const char* pyArgSpec, const char* const* keywords, PyObject** args) { return ::PyArg_ParseTupleAndKeywords(pyTuple, pyDict, pyArgSpec, (char**)keywords, &args[S]...); }
typedef typename TemplateSequenceGenerator2<sizeof...(A)>::sequence sequence;
};

template<class R, class... A> struct PyCTupleCallSpec {
template<int... S> static R call (TemplateSequence<S...> seq, R(*f)(A...), PyObject** args) {   return f(  PyConverter<typename NthType<S, A...>::type>::outCast(args[S])...);  }
template<int... S> static R call (TemplateSequence<S...> seq, R(__stdcall *f)(A...), PyObject** args) {   return f(  PyConverter<typename NthType<S, A...>::type>::outCast(args[S])...);  }
template<class C, int... S> static R callmeth (TemplateSequence<S...> seq, C& c, R(C::*f)(A...), PyObject** args) {   return (c.*f)( PyConverter<typename NthType<S, A...>::type>::outCast(args[S])...);  }
};

template<class... A> struct PyCTupleCallSpec<void, A...> {
template<int... S> static void call (TemplateSequence<S...> seq, void(*f)(A...), PyObject**  args) {  f( PyConverter<typename NthType<S, A...>::type>::outCast(args[S])...);  }
template<int... S> static void call (TemplateSequence<S...> seq, void(__stdcall *f)(A...), PyObject**  args) {  f( PyConverter<typename NthType<S, A...>::type>::outCast(args[S])...);  }
template<class C, int... S> static void callmeth (TemplateSequence<S...> seq, C& c, void(C::*f)(A...), PyObject** args) {   (c.*f)( PyConverter<typename NthType<S, A...>::type>::outCast(args[S])...);  }
};

template<class CFunc> struct PyFuncSpec {
template<class R, class... A> static inline PyObject* func2 (R(*cfunc)(A...), PyObject* pySelf, PyObject* pyArgs) {
typename PyParseTupleSpec<A...>::sequence seq1;
typename TemplateSequenceGenerator<sizeof...(A)>::sequence seq2;
PyObject* args[sizeof...(A)] ;//= { TemplatedZero<A>::zero... };
ZeroMemory(args, sizeof(args));
if (!PyParseTupleSpec<A...>::PyArg_ParseTuple(seq1, pyArgs, TemplatedString<PyTypeSpec<A>::outLetter...>(), args)) return NULL;
R result = PyCTupleCallSpec<R,A...>::call(seq2, cfunc, args);
return PyConverter<R>::inCast(result);
}
template<class... A> static inline PyObject* func2 (void(*cfunc)(A...), PyObject* pySelf, PyObject* pyArgs) {
typename PyParseTupleSpec<A...>::sequence seq1;
typename TemplateSequenceGenerator<sizeof...(A)>::sequence seq2;
PyObject* args[sizeof...(A)] ;//= { TemplatedZero<A>::zero... };
ZeroMemory(args, sizeof(args));
if (!PyParseTupleSpec<A...>::PyArg_ParseTuple(seq1, pyArgs, TemplatedString<PyTypeSpec<A>::outLetter...>(), args)) return NULL;
PyCTupleCallSpec<void,A...>::call(seq2, cfunc, args);
Py_RETURN_NONE;
}
template<class R, class... A> static inline PyObject* func2 (R(__stdcall *cfunc)(A...), PyObject* pySelf, PyObject* pyArgs) {
typename PyParseTupleSpec<A...>::sequence seq1;
typename TemplateSequenceGenerator<sizeof...(A)>::sequence seq2;
PyObject* args[sizeof...(A)] ;//= { TemplatedZero<A>::zero... };
ZeroMemory(args, sizeof(args));
if (!PyParseTupleSpec<A...>::PyArg_ParseTuple(seq1, pyArgs, TemplatedString<PyTypeSpec<A>::outLetter...>(), args)) return NULL;
R result = PyCTupleCallSpec<R,A...>::call(seq2, cfunc, args);
return PyConverter<R>::inCast(result);
}
template<class... A> static inline PyObject* func2 (void(*__stdcall cfunc)(A...), PyObject* pySelf, PyObject* pyArgs) {
typename PyParseTupleSpec<A...>::sequence seq1;
typename TemplateSequenceGenerator<sizeof...(A)>::sequence seq2;
PyObject* args[sizeof...(A)] ;//= { TemplatedZero<A>::zero... };
ZeroMemory(args, sizeof(args));
if (!PyParseTupleSpec<A...>::PyArg_ParseTuple(seq1, pyArgs, TemplatedString<PyTypeSpec<A>::outLetter...>(), args)) return NULL;
PyCTupleCallSpec<void,A...>::call(seq2, cfunc, args);
Py_RETURN_NONE;
}
template<class R, class O, class... A> static inline PyObject* func2 (R(O::*cfunc)(A...), PyObject* pySelf, PyObject* pyArgs) {
typename PyParseTupleSpec<A...>::sequence seq1;
typename TemplateSequenceGenerator<sizeof...(A)>::sequence seq2;
PyObject* args[sizeof...(A)] ;//= { TemplatedZero<A>::zero... };
ZeroMemory(args, sizeof(args));
if (!PyParseTupleSpec<A...>::PyArg_ParseTuple(seq1, pyArgs, TemplatedString<PyTypeSpec<A>::outLetter...>(), args)) return NULL;
R result = PyCTupleCallSpec<R,A...>::callmeth(seq2, *(O*)(pySelf), cfunc, args);
return PyConverter<R>::inCast(result);
}
template<class O, class... A> static inline PyObject* func2 (void(O::*cfunc)(A...), PyObject* pySelf, PyObject* pyArgs) {
typename PyParseTupleSpec<A...>::sequence seq1;
typename TemplateSequenceGenerator<sizeof...(A)>::sequence seq2;
PyObject* args[sizeof...(A)] ;//= { TemplatedZero<A>::zero... };
ZeroMemory(args, sizeof(args));
if (!PyParseTupleSpec<A...>::PyArg_ParseTuple(seq1, pyArgs, TemplatedString<PyTypeSpec<A>::outLetter...>(), args)) return NULL;
PyCTupleCallSpec<void,A...>::callmeth(seq2, *(O*)(pySelf), cfunc, args);
Py_RETURN_NONE;
}
template<CFunc cfunc> static PyObject* func (PyObject* pySelf, PyObject* pyArgs) { return func2(cfunc, pySelf, pyArgs); }
};

template<class CFunc, const char* const* kwds> struct PyFuncSpecKW {
template<class R, class... A> static inline PyObject* func2 (R(*cfunc)(A...), PyObject* pySelf, PyObject* pyArgs, PyObject* pyKwds) {
typename PyParseTupleSpec<A...>::sequence seq1;
typename TemplateSequenceGenerator<sizeof...(A)>::sequence seq2;
PyObject* args[sizeof...(A)] ;//= { TemplatedZero<A>::zero... };
ZeroMemory(args, sizeof(args));
if (!PyParseTupleSpec<A...>::PyArg_ParseTupleAndKeywords(seq1, pyArgs, pyKwds, TemplatedString<PyTypeSpec<A>::outLetter...>(), kwds, args)) return NULL;
R result = PyCTupleCallSpec<R,A...>::call(seq2, cfunc, args);
return PyConverter<R>::inCast(result);
}
template<class... A> static inline PyObject* func2 (void(*cfunc)(A...), PyObject* pySelf, PyObject* pyArgs, PyObject* pyKwds) {
typename PyParseTupleSpec<A...>::sequence seq1;
typename TemplateSequenceGenerator<sizeof...(A)>::sequence seq2;
PyObject* args[sizeof...(A)] ;//= { TemplatedZero<A>::zero... };
ZeroMemory(args, sizeof(args));
if (!PyParseTupleSpec<A...>::PyArg_ParseTupleAndKeywords(seq1, pyArgs, pyKwds, TemplatedString<PyTypeSpec<A>::outLetter...>(), kwds, args)) return NULL;
PyCTupleCallSpec<void,A...>::call(seq2, cfunc, args);
Py_RETURN_NONE;
}
template<class R, class O, class... A> static inline PyObject* func2 (R(O::*cfunc)(A...), PyObject* pySelf, PyObject* pyArgs, PyObject* pyKwds) {
typename PyParseTupleSpec<A...>::sequence seq1;
typename TemplateSequenceGenerator<sizeof...(A)>::sequence seq2;
PyObject* args[sizeof...(A)] ;//= { TemplatedZero<A>::zero... };
ZeroMemory(args, sizeof(args));
if (!PyParseTupleSpec<A...>::PyArg_ParseTupleAndKeywords(seq1, pyArgs, pyKwds, TemplatedString<PyTypeSpec<A>::outLetter...>(), kwds, args)) return NULL;
R result = PyCTupleCallSpec<R,A...>::callmeth(seq2, *(O*)(pySelf), cfunc, args);
return PyConverter<R>::inCast(result);
}
template<class O, class... A> static inline PyObject* func2 (void(O::*cfunc)(A...), PyObject* pySelf, PyObject* pyArgs, PyObject* pyKwds) {
typename PyParseTupleSpec<A...>::sequence seq1;
typename TemplateSequenceGenerator<sizeof...(A)>::sequence seq2;
PyObject* args[sizeof...(A)] ;//= { TemplatedZero<A>::zero... };
ZeroMemory(args, sizeof(args));
if (!PyParseTupleSpec<A...>::PyArg_ParseTupleAndKeywords(seq1, pyArgs, pyKwds, TemplatedString<PyTypeSpec<A>::outLetter...>(), kwds, args)) return NULL;
PyCTupleCallSpec<void,A...>::callmeth(seq2, *(O*)(pySelf), cfunc, args);
Py_RETURN_NONE;
}
template<CFunc cfunc> static PyObject* func (PyObject* pySelf, PyObject* pyArgs, PyObject* pyKwds) { return func2(cfunc, pySelf, pyArgs, pyKwds); }
};

template<class S> struct PySetterSpec {
template<class O, class A> static inline int set2 (void(O::*setf)(A), PyObject* self, PyObject* pyVal) {
A cVal = PyConverter<A>::outCast(pyVal);
if (PyErr_Occurred()) return -1;
((*(O*)self).*setf)(cVal);
return 0;
}
template<class A> static inline int set2 (void(*setf)(A), PyObject* self, PyObject* pyVal) {
A cVal = PyConverter<A>::outCast(pyVal);
if (PyErr_Occurred()) return -1;
setf(cVal);
return 0;
}
template<S setf> static inline int setter (PyObject* self, PyObject* val, void* unused) { return set2(setf, self, val); }
};

template<class G> struct PyGetterSpec {
template<class O, class A> static inline PyObject* get2 (A(O::*getf)(void), PyObject* self ) {
A result = ((*(O*)self).*getf)();
return PyConverter<A>::inCast(result);
}
template<class A> static inline PyObject* get2 (A(*getf)(void), PyObject* self ) {
A result = getf();
return PyConverter<A>::inCast(result);
}
template<G getf> static inline PyObject* getter (PyObject* self, void* unused) { return get2(getf, self); }
};

#define PyDecl(n,f) {(n), (PyFuncSpec<decltype(f)>::func<f>), METH_VARARGS, NULL}
#define PyDeclKW(n,f,k) {(n), (PyCFunction)(PyFuncSpecKW<decltype(f),k>::func<f>), METH_VARARGS | METH_KEYWORDS, NULL}
#define PyDeclStatic(n,f) {(n), (PyFuncSpec<decltype(f)>::func<f>), METH_VARARGS | METH_CLASS, NULL}
#define PyDeclStaticKW(n,f,k) {(n), (PyCFunction)(PyFuncSpecKW<decltype(f),k>::func<f>), METH_VARARGS | METH_KEYWORDS | METH_CLASS, NULL}
#define PyAccessor(n,g,s) {(n), (PyGetterSpec<decltype(g)>::getter<g>), (PySetterSpec<decltype(s)>::setter<s>), NULL, NULL}
#define PyWriteOnlyAccessor(n,s) {(n), NULL, (PySetterSpec<decltype(s)>::setter<s>), NULL, NULL}
#define PyReadOnlyAccessor(n,g) {(n), (PyGetterSpec<decltype(g)>::getter<g>), NULL, NULL, NULL}
#define PyDeclEnd {0, 0, 0, 0}
#define OPT nullptr_t ___OPTIONAL_MARKER___
#define DOLLAR ___DOLLAR___ ___DOLLAR_MARKER___

#endif
