#ifndef ___THREAD_H9
#define ___THREAD_H9
#include "global.h"
#include "sixpad.h"
#include<functional>

typedef std::function<void(void)> Proc;

extern HWND win;

class export Thread {
private:
HANDLE handle;
public:
template<class F> static inline Thread start (const F& f) { return Thread(f); }
template<class F> inline Thread (const F& cf): handle(0) { init(new Proc(cf)); }
bool export join (DWORD ms = INFINITE);
private:
void export init (Proc*);
};

inline bool IsUIThread () {
return (DWORD)(GetCurrentThreadId()) == SPPTR uiThreadId;
}

template<class F> inline void RunSync (const F& cf, bool del = false) {
Proc f(cf);
SendMessage(SPPTR win, WM_RUNPROC, del, &f);
}

template<class F> inline void RunAsync (const F& cf, bool del = true) {
Proc* f = new Proc(cf);
PostMessage(SPPTR win, WM_RUNPROC, del, f);
}

struct RAII_CRITICAL_SECTION {
CRITICAL_SECTION& cs;
RAII_CRITICAL_SECTION (CRITICAL_SECTION& c): cs(c) { EnterCriticalSection(&cs); }
~RAII_CRITICAL_SECTION(){ LeaveCriticalSection(&cs); }
};

#define SCOPE_LOCK(l) RAII_CRITICAL_SECTION ___RAII_CRITICAL_SECTION_VAR##__LINE__ (l)

#endif
