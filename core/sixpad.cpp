#include "global.h"
#include "sixpad.h"

HINSTANCE dllHinstance=0;
SixpadData sp;

BOOL extern SixpadDLLInit (const SixpadData* d) {
sp = *d;
return true;
}

extern "C" BOOL WINAPI __declspec(dllexport) DllMain (HINSTANCE hDll, DWORD reason, LPVOID unused) {
if (reason==DLL_PROCESS_ATTACH) {
dllHinstance = hDll;
//DisableThreadLibraryCalls(hDll);
}
return true;
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
if (!OpenClipboard(sp.win)) return false;
EmptyClipboard();
SetClipboardData(CF_UNICODETEXT, hMem);
CloseClipboard();
return true;
}

tstring export GetClipboardText (void) {
if (!IsClipboardFormatAvailable(CF_UNICODETEXT) || !OpenClipboard(sp.win)) return TEXT("");
HGLOBAL hMem = GetClipboardData(CF_UNICODETEXT);
const wchar_t* hMemData = (wchar_t*)GlobalLock(hMem);
tstring text= toTString(hMemData);
GlobalUnlock(hMem);
CloseClipboard();
return text;
}

