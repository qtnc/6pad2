#include<windows.h>
#include<stdio.h>

unsigned long long getFileTime (const char* fn) {
unsigned long long l = 0;
FILETIME ft;
HANDLE h = CreateFile(fn, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
if (h==INVALID_HANDLE_VALUE) return 0;
if (GetFileTime(h, NULL, NULL, &ft)) l = (((unsigned long long)ft.dwHighDateTime)<<32) | ft.dwLowDateTime;
CloseHandle(h);
return l;
}

int main (int argc, char** argv) {
if (argc!=3) {
printf("Usage: isnewer <file1> <file2>\r\n");
printf("This utility returns error code 0 if file1 is newer than file2, 1 otherwise.\r\n");
}
unsigned long long 
time1 = getFileTime(argv[1]),
time2 = getFileTime(argv[2]);
return time1 < time2;
}
