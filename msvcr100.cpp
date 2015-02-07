// MinGW is stupid ! it links obligatorily to msvcrt.dll
// But python34.dll is linked to msvcr100.dll
// This causes problems when accessing files with fopen functions
#include<cstdio>
#include<windows.h>

static HINSTANCE msvcr = NULL;

extern "C" FILE* msvcfopen (const char* name, const char* ax) {
typedef FILE*(*FOPEN)(const char*, const char*);
static FOPEN fopenfunc = NULL;
if (!fopenfunc) {
if (!msvcr) msvcr = LoadLibraryA("msvcr100.dll");
fopenfunc = (FOPEN)GetProcAddress(msvcr, "fopen");
}
if (fopenfunc) return fopenfunc(name, ax);
else return NULL;
}

extern "C" void msvcfclose (FILE* fp) {
typedef void(*FCLOSE)(FILE*);
static FCLOSE fclosefunc = NULL;
if (!fclosefunc) {
if (!msvcr) msvcr = LoadLibraryA("msr100.dll");
fclosefunc = (FCLOSE)GetProcAddress(msvcr, "fclose");
}
if (fclosefunc) fclosefunc(fp);
}
