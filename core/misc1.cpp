#include "global.h"
#include "strings.hpp"
#include "sixpad.h"

static vector<int> enclist = { 1200, 1201, 1202, 1203, 65002 };

void EnumPythonBonusCP (vector<int>& v);

static BOOL CALLBACK EnumProc1 (LPTSTR cpstr) {
int enc = toInt(tstring(cpstr));
enclist.push_back(enc);
return true;
}

const vector<int>& getAllAvailableEncodings () {
if (enclist.size()<=5) {
EnumSystemCodePages(EnumProc1, CP_INSTALLED);
EnumPythonBonusCP(enclist);
}
return enclist;
}

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

