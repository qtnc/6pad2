#include "main.h"
using namespace std;

void CustomDialog::push (const void* data, size_t len) {
if (!buffer || pos+len>length) {
length = max(pos+len, 4 + length * 4/3);
buffer = (char*)realloc(buffer, length);
}
memcpy(buffer+pos, data, len);
pos+=len;
}

void CustomDialog::push (const tstring& s, size_t alignment) {
push(s.data(), sizeof(TCHAR) * (1+s.size()));
align(alignment);
}

void CustomDialog::align (size_t n) {
if (n<=1) return;
int r = pos%n;
if (r&1) push<char>(0);
if (r&2) push<short>(0);
if (r&4) push<int>(0);
}

void CustomDialog::setTitle (const tstring& title) {
DLGTEMPLATE dlghdr = {
DS_MODALFRAME | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
0, 0, 10, 10, 0, 0
};
push(dlghdr);
push<WORD>(0); // Menu
push<WORD>(0); // Dialog window class
push(title);
align(4);
}

void export CustomDialog::setSize (int x, int y, int w, int h) {
DLGTEMPLATE& dlgtpl = *(DLGTEMPLATE*)buffer;
dlgtpl.x = x;
dlgtpl.y = y;
dlgtpl.cx = w;
dlgtpl.cy = h;
}

void export CustomDialog::addItem (const tstring& type, int id, const tstring& text, DWORD style, int x, int y, int w, int h, DWORD exstyle) {
style |= WS_CHILD | WS_VISIBLE;
DLGTEMPLATE& dlgtpl = *(DLGTEMPLATE*)buffer;
dlgtpl.cdit++;
dlgtpl.cx = std::max((int)dlgtpl.cx, x+w+5);
dlgtpl.cy = std::max((int)dlgtpl.cy, y+h+5);
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

INT_PTR export CustomDialog::show (HWND parent, DLGPROC proc, void* udata) {
DLGTEMPLATE& dlgtpl = *(DLGTEMPLATE*)buffer;
return DialogBoxIndirectParam(hinstance, (LPCDLGTEMPLATE)buffer, parent, proc, (LPARAM)udata);
}





