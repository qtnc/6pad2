#ifndef ___THREAD_H9
#define ___THREAD_H9
#include "global.h"
#include<functional>

typedef std::function<void(void)> Proc;

extern HWND win;

class Thread {
private:
HANDLE handle;
public:
template<class F> static inline Thread start (const F& f) { return Thread(f); }
template<class F> inline Thread (const F& cf): handle(0) { init(new Proc(cf)); }
bool join (DWORD ms = INFINITE);
private:
void init (Proc*);
};

template<class F> inline void RunInEDT (const F& cf) {
Proc f(cf);
SendMessage(win, WM_RUNPROC, 0, &f);
}

struct RAII_CRITICAL_SECTION {
CRITICAL_SECTION& cs;
RAII_CRITICAL_SECTION (CRITICAL_SECTION& c): cs(c) { EnterCriticalSection(&cs); }
~RAII_CRITICAL_SECTION(){ LeaveCriticalSection(&cs); }
};

#define SCOPE_LOCK(l) RAII_CRITICAL_SECTION ___RAII_CRITICAL_SECTION_VAR##__LINE__ (l)

#endif
