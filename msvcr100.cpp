#include<windows.h>
// This fixes a bug with MinGW and the msvcr100 variant of the MSVCRT
// _findfirst and _findnext are absent from msvcr100.dll and libmsvcr100.a
// and should be redirected to _findfirst32 and _findnext32

typedef long intptr_t;

extern "C" intptr_t __declspec(dllimport) _findfirst32 (const char*, struct _finddata_t *);
extern "C" int __declspec(dllimport) _findnext32 (intptr_t, struct _finddata_t *);

extern "C" intptr_t __declspec(dllexport) _findfirst (const char* a, struct _finddata_t *b) {
return _findfirst32(a,b);
}

extern "C" int __declspec(dllexport) _findnext (intptr_t h, struct _finddata_t *b) {
return _findnext32(h,b);
}
