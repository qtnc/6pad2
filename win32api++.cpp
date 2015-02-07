#include "global.h"

tstring GetWindowText (HWND hwnd) {
int len = GetWindowTextLength(hwnd);
tstring text = tstring(len,(TCHAR)0);
GetWindowText(hwnd, (TCHAR*)text.data(), len+1);
return text;
}

tstring GetDlgItemText (HWND hwnd, int id) {
int len = GetDlgItemTextLength(hwnd, id);
tstring text = tstring(len,(TCHAR)0);
GetDlgItemText(hwnd, id, (TCHAR*)text.data(), len+1);
return text;
}

tstring EditGetLine (HWND hEdit, int sLine, int sPos) {
if (sPos<0) sPos = SendMessage(hEdit, EM_LINEINDEX, sLine, 0);
int length = SendMessage(hEdit, EM_LINELENGTH, sPos, 0);
tstring line(length, (TCHAR)0);
*(WORD*)(line.data()) = length+1;
SendMessage(hEdit, EM_GETLINE, sLine, (LPARAM)line.data() );
*(((TCHAR*)line.data())+length) = 0;
return line;
}

tstring EditGetLine (HWND hEdit) {
return EditGetLine(hEdit, SendMessage(hEdit, EM_LINEFROMCHAR, -1, 0));
}

static unsigned long long filetimeTo1970 (unsigned long long l) {
static unsigned long long rep = 0;
if (!rep) {
SYSTEMTIME st1 = { 1970, 1, 0, 1, 0, 0, 0, 0 };
FILETIME ft1;
SystemTimeToFileTime(&st1, &ft1);
rep = (((unsigned long long)ft1.dwHighDateTime)<<32) | ft1.dwLowDateTime;
}
return ((l-rep)/10000000LL);
}

unsigned long long GetFileTime (LPCTSTR fn, int type) {
unsigned long long l = 0;
FILETIME ft;
HANDLE h = CreateFile(fn, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
if (h==INVALID_HANDLE_VALUE) return 0;
if (GetFileTime(h, type==0?&ft:0, type==1?&ft:0, type==2?&ft:0)) l = (((unsigned long long)ft.dwHighDateTime)<<32) | ft.dwLowDateTime;
CloseHandle(h);
return filetimeTo1970(l);
}
