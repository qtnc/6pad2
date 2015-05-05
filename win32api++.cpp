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

tstring EditGetSelectedText (HWND hEdit) {
int sStart=0, sEnd=0;
SendMessage(hEdit, EM_GETSEL, &sStart, &sEnd);
if (sStart==sEnd) return TEXT("");
else return EditGetSubstring(hEdit, sStart, sEnd);
}

tstring EditGetSubstring (HWND hEdit, int start, int end) {
int len = GetWindowTextLength(hEdit);
if (end<0) end+=len;
if (start<0) start+=len;
if (end>len) end=len;
if (start>len) start=len;
if (start>end) { int i=start; start=end; end=i; }
HLOCAL hLoc = (HLOCAL)SendMessage(hEdit, EM_GETHANDLE, 0, 0);
LPCTSTR text = (LPCTSTR)LocalLock(hLoc);
tstring str(text + start, text + end);
LocalUnlock(hLoc);
return str;
}

unsigned long long GetCurTime () {
unsigned long long l=0;
FILETIME ft;
GetSystemTimeAsFileTime(&ft);
l = (((unsigned long long)ft.dwHighDateTime)<<32ULL) | ft.dwLowDateTime;
return l;
}

unsigned long long GetFileTime (LPCTSTR fn, int type) {
unsigned long long l = 0;
FILETIME ft;
HANDLE h = CreateFile(fn, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
if (h==INVALID_HANDLE_VALUE) return 0;
if (GetFileTime(h, type==0?&ft:0, type==1?&ft:0, type==2?&ft:0)) l = (((unsigned long long)ft.dwHighDateTime)<<32) | ft.dwLowDateTime;
CloseHandle(h);
return l;
}
