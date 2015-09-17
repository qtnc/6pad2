#include "main.h"
using namespace std;

struct DlgBuffer {
char* buffer;
size_t pos, length;
DlgBuffer(): buffer(0), pos(0), length(0) {}
void align (size_t n);
void push (const tstring& s, size_t alignment=sizeof(WORD));
void push (const void* data, size_t len);
template<class T> inline void push (const T& val) { push(&val, sizeof(val)); }

void pushHeader (const tstring& title, int nItems, int x, int y, int w, int h);
void pushItem (const tstring& type, int id, const tstring& text, DWORD style, int x, int y, int w, int h, DWORD exstyle=0);
};

void DlgBuffer::push (const void* data, size_t len) {
if (!buffer || pos+len>length) {
length = max(pos+len, 4 + length * 4/3);
buffer = (char*)realloc(buffer, length);
}
memcpy(buffer+pos, data, len);
pos+=len;
printf("push: len=%d, pos=%d, max=%d\n", len, pos, length);
}

void DlgBuffer::push (const tstring& s, size_t alignment) {
printf("push string: %ls, len=%d\n", s.c_str(), s.size());
push(s.data(), sizeof(TCHAR) * (1+s.size()));
align(alignment);
}

void DlgBuffer::align (size_t n) {
if (n<=1) return;
int r = pos%n;
if (r&1) push<char>(0);
if (r&2) push<short>(0);
if (r&4) push<int>(0);
}
void DlgBuffer::pushHeader (const tstring& title, int nItems, int x, int y, int w, int h) {
DLGTEMPLATE dlghdr = {
DS_MODALFRAME | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
0, nItems, x, y, w, h
};
push(dlghdr);
push<WORD>(0); // Menu
push<WORD>(0); // Dialog window class
push(title);
}

void DlgBuffer::pushItem (const tstring& type, int id, const tstring& text, DWORD style, int x, int y, int w, int h, DWORD exstyle) {
DLGITEMTEMPLATE itemtpl = {
style, exstyle,
x, y, w, h, id
};
push(itemtpl);
push(type); // control window class
push(text); // control text
push<WORD>(0); // additional data
align(sizeof(DWORD));	
}

INT_PTR DlgProc (HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp) {
switch(umsg) {
case WM_INITDIALOG: {
Beep(1600,150);
return true;
}
case WM_COMMAND: switch(LOWORD(wp)) {
case IDOK:
case IDCANCEL:
EndDialog(hwnd, LOWORD(wp));
return true;
}break;//WM_COMMAND
}
return false;
}

void test123 (void) {
printf("Test123\n");
DlgBuffer dlg;
dlg.pushHeader( TEXT("Custom dialog box!"), 4, 10, 10, 400, 70);
dlg.pushItem( TEXT("STATIC"), 1001, TEXT("Enter your name :"), WS_VISIBLE | SS_LEFT, 10, 10, 375, 15);
dlg.pushItem( TEXT("EDIT"), 1002, TEXT(""), WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL, 10, 30, 375, 15);
dlg.pushItem( TEXT("BUTTON"), IDOK, TEXT("&OK"), WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 250, 50, 60, 16);
dlg.pushItem( TEXT("BUTTON"), IDCANCEL, TEXT("Ca&ncel"), WS_TABSTOP | WS_VISIBLE | BS_PUSHBUTTON, 325, 50, 60, 15);
Beep(800,150);
int result = DialogBoxIndirectParam(hinstance, (LPCDLGTEMPLATE)dlg.buffer, sp->win, (DLGPROC)DlgProc, 0);
Beep(1200,150);
printf("re=%d, error=%d\n", result, GetLastError());
}
