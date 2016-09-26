#ifndef _____ACCELERATORS_H____2
#define _____ACCELERATORS_H____2
#include "global.h"
#include "python34.h"
#include<functional>

template<class S> struct UserFunction {
std::function<S> func;
PyFunc<S> pyFunc;
inline UserFunction (): func(), pyFunc(nullptr) {}
inline UserFunction (const std::function<S>& f): func(f), pyFunc(nullptr) {}
inline UserFunction (const PyFunc<S>& f): func(), pyFunc(f) {}
inline operator bool () { return func||pyFunc; }
template <class... A> void operator() (A... args) {
if (func) func(args...);
else if (pyFunc) pyFunc(args...);
}
};

int AddUserCommand (const UserFunction<void(void)>& f, int cmd=0) ;
bool RemoveUserCommand (int cmd);
const UserFunction<void(void)>& findUserCommand (int cmd);
bool AddAccelerator (HACCEL& hAccel, int flags, int key, int cmd);
BOOL RemoveAccelerator (HACCEL& hAccel, int cmd);
bool FindAccelerator (HACCEL& hAccel, int& cmd, int& flags, int& key);
tstring KeyCodeToName (int flags, int vk, bool i18n);
bool KeyNameToCode (const tstring& kn, int& flags, int& key);
int SetTimeout (const UserFunction<void(void)>& f, int time, bool repeat);
void ClearTimeout (int id);

#endif
