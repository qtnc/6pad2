#include "global.h"

tstring export GetWindowText (HWND hwnd) {
int len = GetWindowTextLength(hwnd);
tstring text = tstring(len,(TCHAR)0);
GetWindowText(hwnd, (TCHAR*)text.data(), len+1);
return text;
}

tstring export GetDlgItemText (HWND hwnd, int id) {
int len = GetDlgItemTextLength(hwnd, id);
tstring text = tstring(len,(TCHAR)0);
GetDlgItemText(hwnd, id, (TCHAR*)text.data(), len+1);
return text;
}

tstring export GetListBoxItemText (HWND hwnd, int index) {
int len = SendMessage(hwnd, LB_GETTEXTLEN, index, 0);
tstring text = tstring(len,(TCHAR)0);
SendMessage(hwnd, LB_GETTEXT, index, (TCHAR*)text.data());
return text;
}

/*tstring export GetComboBoxItemText (HWND hwnd, int index) {
int len = SendMessage(hwnd, CB_GETTEXTLEN, index, 0);
tstring text = tstring(len,(TCHAR)0);
SendMessage(hwnd, CB_GETTEXT, index, (TCHAR*)text.data());
return text;
}*/

tstring export EditGetLine (HWND hEdit, int sLine, int sPos) {
if (sPos<0) sPos = SendMessage(hEdit, EM_LINEINDEX, sLine, 0);
int length = SendMessage(hEdit, EM_LINELENGTH, sPos, 0);
tstring line(length, (TCHAR)0);
*(WORD*)(line.data()) = length+1;
SendMessage(hEdit, EM_GETLINE, sLine, (LPARAM)line.data() );
*(((TCHAR*)line.data())+length) = 0;
return line;
}

tstring export EditGetLine (HWND hEdit) {
return EditGetLine(hEdit, SendMessage(hEdit, EM_LINEFROMCHAR, -1, 0));
}

tstring export EditGetSelectedText (HWND hEdit) {
int sStart=0, sEnd=0;
SendMessage(hEdit, EM_GETSEL, &sStart, &sEnd);
if (sStart==sEnd) return TEXT("");
else return EditGetSubstring(hEdit, sStart, sEnd);
}

tstring export EditGetSubstring (HWND hEdit, int start, int end) {
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

tstring export GetMenuString (HMENU menu, UINT id, UINT type) {
int len = GetMenuString(menu, id, NULL, 0, type);
tstring text = tstring(len,(TCHAR)0);
GetMenuString(menu, id, (TCHAR*)(text.data()), len+1, type);
return text;
}

tstring export GetMenuName (HMENU menu, UINT pos, BOOL bypos) {
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_DATA;
if (!GetMenuItemInfo(menu, pos, bypos, &mii)) return TEXT("");
const TCHAR* re = (const TCHAR*)(mii.dwItemData);
if (re) return tstring(re);
else return TEXT("");
}

void export SetMenuName (HMENU menu, UINT pos, BOOL bypos, LPCTSTR name) {
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_DATA;
if (!GetMenuItemInfo(menu, pos, bypos, &mii)) return;
if (mii.dwItemData) free((void*)(mii.dwItemData));
mii.dwItemData = name? (ULONG_PTR)tstrdup(name) :NULL;
SetMenuItemInfo(menu, pos, bypos, &mii);
}

unsigned long long export GetCurTime () {
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

tstring export GetErrorText (int errorCode) {
TCHAR buf[512]={0};
FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 511, NULL);
return buf;
}

