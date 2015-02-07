#ifndef ___WINDOWS2_H___
#define ___WINDOWS2_H___
#ifdef __cplusplus
extern "C" {
#endif
#include<windows.h>

typedef LRESULT (CALLBACK *SUBCLASSPROC)(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
BOOL WINAPI SetWindowSubclass(HWND,SUBCLASSPROC,UINT_PTR,DWORD_PTR);
BOOL WINAPI GetWindowSubclass(HWND,SUBCLASSPROC,UINT_PTR,DWORD_PTR*);
BOOL WINAPI RemoveWindowSubclass(HWND,SUBCLASSPROC,UINT_PTR);
LRESULT WINAPI DefSubclassProc(HWND,UINT,WPARAM,LPARAM);

#ifdef __cplusplus
} // extern "C"
#endif
#endif
