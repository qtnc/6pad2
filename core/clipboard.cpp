#include "global.h"
#include "strings.hpp"
#include "sixpad.h"
#include<shlobj.h>


bool export SetClipboardText (const tstring&  text2) {
wstring text = toWString(text2);
int len = text.size();
HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, sizeof(wchar_t)*(1+len));
if (!hMem) return false;
wchar_t* mem = (wchar_t*)GlobalLock(hMem);
memcpy(mem, text.data(), sizeof(wchar_t) * len);
mem[len]=0;
GlobalUnlock(hMem);
if (!OpenClipboard(sp->win)) return false;
EmptyClipboard();
SetClipboardData(CF_UNICODETEXT, hMem);
CloseClipboard();
return true;
}

tstring export GetClipboardText (void) {
if (!IsClipboardFormatAvailable(CF_UNICODETEXT) || !OpenClipboard(sp->win)) return TEXT("");
HGLOBAL hMem = GetClipboardData(CF_UNICODETEXT);
const wchar_t* hMemData = (wchar_t*)GlobalLock(hMem);
tstring text= toTString(hMemData);
GlobalUnlock(hMem);
CloseClipboard();
return text;
}

bool export SetClipboardFiles (const vector<tstring>& files) {
int pos=sizeof(DROPFILES), size = sizeof(DROPFILES) + 4;
for (const tstring& s: files) size += (s.size()+1) * sizeof(TCHAR);
HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size);
DROPFILES *df = (DROPFILES*) GlobalLock(hGlobal);
ZeroMemory(df, size);
df->pFiles = sizeof(DROPFILES); 
df->fWide = IsUnicode()?TRUE:FALSE;
LPTSTR ptr = (LPTSTR) (df + 1);
for (const tstring& s: files) {
memcpy(ptr, s.data(), s.size() * sizeof(TCHAR));
ptr += s.size() +1;
}
GlobalUnlock(hGlobal);
if (!OpenClipboard(sp->win)) return false;
EmptyClipboard();
SetClipboardData(CF_HDROP, hGlobal);
CloseClipboard();
return true;
}

vector<tstring> export GetHDROPFiles (HDROP hDrop) {
vector<tstring> files;
int nFiles = DragQueryFile(hDrop, -1, NULL, NULL);
if (nFiles>0) for (int i=0; i<nFiles; i++) {
TCHAR buf[300] = {0};
if (!DragQueryFile(hDrop, i, buf, 299)) break;
files.push_back(buf);
}
return files;
}

vector<tstring> export GetClipboardFiles (void) {
if (!IsClipboardFormatAvailable(CF_HDROP) || !OpenClipboard(sp->win)) return {};
HGLOBAL hMem = GetClipboardData(CF_HDROP);
HDROP hDrop = (HDROP)GlobalLock(hMem);
vector<tstring> files = GetHDROPFiles(hDrop);
GlobalUnlock(hMem);
CloseClipboard();
return files;
}
