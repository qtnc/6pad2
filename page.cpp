#include "global.h"
#include "page.h"
#include "file.h"
#include "inifile.h"

LRESULT CALLBACK EditAreaWinProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR subclassId, DWORD_PTR dwUdata);

extern HINSTANCE hinstance;
extern IniFile config, msgs;

bool TextPage::IsEmpty ()  {
return file.size()<=0 && GetWindowTextLength(zone)<=0;
}

bool TextPage::IsModified () {
return !zone || !!SendMessage(zone, EM_GETMODIFY, 0, 0);
}

HWND TextPage::CreateEditArea (HWND parent) {
static int count = 0;
tstring text;
int ss=0, se=0;
if (!zone) text = LoadText();
else {
SendMessage(zone, EM_GETSEL, (WPARAM)&ss, (LPARAM)&se);
text = GetWindowText(zone);
DestroyWindow(zone);
}
HWND hEdit  = CreateWindowEx(0, TEXT("EDIT"), TEXT(""),
WS_CHILD | WS_TABSTOP | ES_MULTILINE | ES_NOHIDESEL | ES_AUTOVSCROLL | ((flags&PF_AUTOLINEBREAK)? 0:ES_AUTOHSCROLL),
10, 10, 400, 400,
parent, (HMENU)(IDC_EDITAREA + count++), hinstance, NULL);
SendMessage(hEdit, EM_SETSEL, ss, se);
SendMessage(hEdit, EM_SETLIMITTEXT, 1073741823, 0);
//SendMessage(hEdit, WM_SETFONT, font, TRUE);
//int x = curPage->tabSpaces==0? 16 : ABS(curPage->tabSpaces)*4;
//SendMessage(hEdit, EM_SETTABSTOPS, 1, &x);
SetWindowText(hEdit, text.c_str());
SetWindowSubclass(hEdit, EditAreaWinProc, 0, 0);
return zone=hEdit;
}

bool TextPage::SaveText (const tstring& newFile) {
if (flags&PF_NOSAVE) return false;
if ((flags&PF_READONLY) && newFile.size()<=0) return false;
if (newFile.size()>0) { file = newFile; flags&=~PF_READONLY; }
if (file.size()<=0) return false;
if (!zone) return false;
int len = GetWindowTextLength(zone);
tstring str = GetWindowText(zone);
if (lineEnding==LE_UNIX) replace(str, TEXT("\r\n"), TEXT("\n"));
else if (lineEnding==LE_MAC) replace(str, TEXT("\r\n"), TEXT("\r"));
string cstr = ConvertToEncoding(str, encoding);
File fd(file, true);
if (fd) fd.writeFully(cstr.data(), cstr.size());
return true; 
}

tstring TextPage::LoadText (const tstring& newFile, bool guessFormat) {
if (newFile.size()>0) file = newFile;
tstring text = TEXT("");
if (file.size()>=0) {
File fd(file);
if (fd) {
string str = fd.readFully();
if (guessFormat) { encoding=-1; lineEnding=-1; indentationMode=-1; }
if (encoding<0) encoding = guessEncoding( (const unsigned char*)(str.data()), config.get("defaultEncoding", CP_ACP));
text = ConvertFromEncoding(str, encoding);
if (lineEnding<0) lineEnding = guessLineEnding(text.c_str(), config.get("defaultLineEnding", LE_DOS));
if (indentationMode<0) indentationMode = guessIndentationMode(text.c_str(), text.size(), config.get("defaultIndentationMode", 0));
if (lineEnding==LE_UNIX) replace(text, TEXT("\n"), TEXT("\r\n"));
else if (lineEnding==LE_MAC) replace(text, TEXT("\r"), TEXT("\r\n"));
}}
if (zone) {
int ss, se;
SendMessage(zone, EM_GETSEL, &ss, &se);
SetWindowText(zone, text.c_str());
SendMessage(zone, EM_SETSEL, ss, se);
SendMessage(zone, EM_SETMODIFY, 0, 0);
if (IsWindowVisible(zone)) SendMessage(zone, EM_SCROLLCARET, 0, 0);
}
return text;
}
