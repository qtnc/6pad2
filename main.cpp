#include "global.h"
#include "strings.hpp"
#include "page.h"
#include "inifile.h"
#include "file.h"
#include "dialogs.h"
#include "python34.h"
#include<boost/regex.hpp>

#define FF_CASE 1
#define FF_REGEX 2
#define FF_UPWARDS 4
#define OF_REUSEOPENEDTABS 1
#define OF_NEWINSTANCE 2
#define OF_EXITONDOUBLEOPEN 4

#ifdef UNICODE
typedef boost::wregex tregex;
typedef boost::wcregex_iterator tcregex_iterator;
typedef boost::wcmatch tcmatch;
#else
typedef boost::regex tregex;
typedef boost::cregex_iterator tcregex_iterator;
#endif

struct FindData {
tstring findText, replaceText;
int flags;
inline FindData (const tstring& s=TEXT(""), const tstring& r=TEXT(""), int f=0) : findText(s), replaceText(r), flags(f) {}
inline bool operator== (const FindData& f) { return f.findText==findText && f.replaceText==replaceText && f.flags==flags; }
inline bool operator!= (const FindData& f) { return !(*this==f); }
tregex newRegex (bool);
};

tstring appPath, appDir, appName;
IniFile msgs, config;
vector<int> encodings = { CP_ACP, CP_UTF8, CP_UTF8_BOM, CP_UTF16_LE, CP_UTF16_LE_BOM, CP_UTF16_BE, CP_UTF16_BE_BOM, CP_ISO_8859_15, CP_MSDOS };
vector<tstring> argv;
vector<shared_ptr<Page>> pages;
shared_ptr<Page> curPage(0);
list<FindData> finds;

TCHAR CLASSNAME[32] = {0};
HINSTANCE hinstance = 0;
HWND win=0, tabctl=0, status=0;
HMENU menu = 0, menuFormat=0, menuEncoding=0, menuLineEnding=0, menuIndentation=0;
HACCEL hAccel = 0;

void SaveCurFile (bool saveas = false);
void GoToLineDialog () ;
void SearchReplaceDialog (bool);
BOOL WINAPI PredispatchMessage (MSG&);
LRESULT WINAPI AppWinProc (HWND, UINT, WPARAM, LPARAM);
void PrepareSmartPaste (tstring& text, const tstring& indent);

tstring msg (const char* name) {
return toTString(msgs.get<string>(name, name));
}

HWND GetCurEditArea () {
if (curPage && curPage->zone) return curPage->zone;
else return NULL;
}

bool PageDeactivated (shared_ptr<Page> p) {
if (!p) return true;
ShowWindow(p->zone, SW_HIDE);
EnableWindow(p->zone, FALSE);
return true;
}

void PageActivated (shared_ptr<Page> p) {
RECT r; GetClientRect(win, &r);
r.left = 5; r.top = 5; r.right -= 10; r.bottom -= 49;
SendMessage(tabctl, TCM_ADJUSTRECT, FALSE, &r);
EnableWindow(p->zone, TRUE);
SetWindowPos(p->zone, NULL,
r.left+3, r.top+3, r.right - r.left -6, r.bottom - r.top -6,
SWP_NOZORDER | SWP_SHOWWINDOW);
SetFocus(p->zone);
SetWindowText(win, (p->name + TEXT(" - ") + appName).c_str() );
int encidx = -1; for (int i=0; i<encodings.size(); i++) { if (p->encoding==encodings[i]) { encidx=i; break; }}
CheckMenuRadioItem(menuEncoding, 0, encodings.size(), encidx, MF_BYPOSITION);
CheckMenuRadioItem(menuLineEnding, 0, 2, p->lineEnding, MF_BYPOSITION);
CheckMenuRadioItem(menuIndentation, 0, 8, p->indentationMode, MF_BYPOSITION);
CheckMenuItem(menuFormat, IDM_AUTOLINEBREAK, MF_BYCOMMAND | (p->flags&PF_AUTOLINEBREAK? MF_CHECKED : MF_UNCHECKED));
curPage = p;
}

bool PageClosing (shared_ptr<Page> p) {
if (!p->IsModified()) return true;
int re = MessageBox(win, tsnprintf(512, msg("Save changes to %s?"), p->name.c_str()).c_str(), p->name.c_str(), MB_ICONEXCLAMATION  | MB_YESNOCANCEL);
if (re==IDYES) { 
shared_ptr<Page> cp = curPage;
curPage=p;
SaveCurFile();
curPage = cp;
return true;
}
else return re==IDNO;
}

void PageClosed (shared_ptr<Page> p) { }

void PageOpened (shared_ptr<Page> p) { }

void PageSetLineEnding (shared_ptr<Page> p, int le) {
if (!p) return;
p->lineEnding = le;
CheckMenuRadioItem(menuLineEnding, 0, 2, p->lineEnding, MF_BYPOSITION);
}

void PageSetEncoding (shared_ptr<Page> p, int enc) {
if (!p) return;
p->encoding = encodings[enc];
CheckMenuRadioItem(menuEncoding, 0, encodings.size(), enc, MF_BYPOSITION);
}

void PageSetIndentationMode (shared_ptr<Page> p, int im) {
p->indentationMode = im;
CheckMenuRadioItem(menuIndentation, 0, 8, p->indentationMode, MF_BYPOSITION);
}

void PageSetAutoLineBreak (shared_ptr<Page> p, bool alb) {
if (!p) return;
if (alb) p->flags |= PF_AUTOLINEBREAK;
else p->flags &=~PF_AUTOLINEBREAK;
CheckMenuItem(menuFormat, IDM_AUTOLINEBREAK, MF_BYCOMMAND | (p->flags&PF_AUTOLINEBREAK? MF_CHECKED : MF_UNCHECKED));
p->CreateEditArea(tabctl);
if (p==curPage) PageActivated(p);
}

void PageActivate (int i) {
SendMessage(tabctl, TCM_SETCURFOCUS, i, 0);
}

void PageAdd (shared_ptr<Page> p, bool focus = true) {
p->CreateEditArea(tabctl);
pages.push_back(p);
TCITEM it;
it.mask = TCIF_TEXT;
it.pszText = (LPTSTR)(p->name.c_str());
int pos = SendMessage(tabctl, TCM_GETITEMCOUNT, 0, 0);
SendMessage(tabctl, TCM_INSERTITEM, pos, &it);
int oldpos = SendMessage(tabctl, TCM_GETCURSEL, 0, 0);
PageOpened(p);
if (focus) PageActivate(pages.size() -1);
}

shared_ptr<Page> PageAddEmpty (bool focus = true) {
static int count = 0;
shared_ptr<Page> p = shared_ptr<Page>(new TextPage());
p->name = msg("Untitled") + TEXT(" ") + toTString(++count);
p->encoding = config.get("defaultEncoding", CP_ACP);
p->lineEnding = config.get("defaultLineEnding", LE_DOS);
p->indentationMode = config.get("defaultIndentationMode", 0);
p->flags = config.get("defaultAutoLineBreak", false)? PF_AUTOLINEBREAK : 0;
PageAdd(p, focus);
return p;
}

bool PageDelete (shared_ptr<Page> p, int idx = -1) {
if (!p) return false;
if (!PageClosing(p)) return false;
if (curPage==p) { PageDeactivated(p); curPage=0; }
PageClosed(p);
if (idx<0) for (int i=0; i<pages.size(); i++) { if (pages[i]==p) { idx=i; break; }}
pages.erase(pages.begin()+idx);
SendMessage(tabctl, TCM_DELETEITEM, idx, 0);
if (pages.size()>idx) PageActivate(idx);
else if (pages.size()>0) PageActivate(idx -1);
return true;
}

void PageReopen (shared_ptr<Page> p) {
if (!p) return;
p->LoadText(TEXT(""),false);
}

bool PageGoToNext (int delta = 1) {
int cur = SendMessage(tabctl, TCM_GETCURSEL, 0, 0);
int next = (cur + delta + pages.size()) %pages.size();
if (next!=cur) PageActivate(next);
else return false;
return true;
}

void StatusBarUpdate (HWND hEdit) {
int spos=0, epos=0;
SendMessage(hEdit, EM_GETSEL, &spos, &epos);
int sline = SendMessage(hEdit, EM_LINEFROMCHAR, spos, 0);
int scolumn = spos - SendMessage(hEdit, EM_LINEINDEX, sline, 0);
if (spos!=epos) {
int eline = SendMessage(hEdit, EM_LINEFROMCHAR, epos, 0);
int ecolumn = epos - SendMessage(hEdit, EM_LINEINDEX, eline, 0);
SetWindowText(status, tsnprintf(512, msg("Li %d, Col %d to Li %d, Col %d"), sline, scolumn, eline, ecolumn));
} else {
int nlines = SendMessage(hEdit, EM_GETLINECOUNT, 0, 0);
int max = GetWindowTextLength(hEdit);
int prc = max? 100 * spos / max :0;
SetWindowText(status, tsnprintf(512, msg("Li %d, Col %d.\t%d%%, %d lines"), sline, scolumn, prc, nlines));
}}

bool AppWindowClosing () {
for (int i=pages.size() -1; i>=0; i--) if (!PageDelete(pages[i],i)) return false;
return true;
}

void AppWindowClosed () { }

void AppWindowOpened () { }

void AppWindowActivated () { }

void AppWindowGainedFocus () {
HWND hEdit = GetCurEditArea();
if (hEdit) SetFocus(hEdit);
}

void AppWindowResized () {
RECT r; GetClientRect(win, &r);
MoveWindow(tabctl, 5, 5, r.right -10, r.bottom -49, TRUE);
MoveWindow(status, 5, r.bottom -32, r.right -10, 27, TRUE);
HWND edit = curPage? curPage->zone : NULL;
if (edit) {
r.left = 5; r.top = 5; r.right -= 10; r.bottom -= 49;
SendMessage(tabctl, TCM_ADJUSTRECT, FALSE, &r);
MoveWindow(edit, r.left+3, r.top+3, r.right-r.left -6, r.bottom-r.top -6, TRUE);
}}

bool SetClipboardText (const tstring&  text2) {
wstring text = toWString(text2);
int len = text.size();
HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, sizeof(wchar_t)*(1+len));
if (!hMem) return false;
wchar_t* mem = (wchar_t*)GlobalLock(hMem);
memcpy(mem, text.data(), sizeof(wchar_t) * len);
mem[len]=0;
GlobalUnlock(hMem);
if (!OpenClipboard(win)) return false;
EmptyClipboard();
SetClipboardData(CF_UNICODETEXT, hMem);
CloseClipboard();
return true;
}

tstring GetClipboardText (void) {
if (!IsClipboardFormatAvailable(CF_UNICODETEXT) || !OpenClipboard(win)) return TEXT("");
HGLOBAL hMem = GetClipboardData(CF_UNICODETEXT);
const wchar_t* hMemData = (wchar_t*)GlobalLock(hMem);
tstring text= toTString(hMemData);
GlobalUnlock(hMem);
CloseClipboard();
return text;
}

bool AddAccelerator (int flags, int key, int cmd) {
int n = CopyAcceleratorTable(hAccel, NULL, 0);
ACCEL* accels = (ACCEL*)malloc(sizeof(ACCEL) * (n+1));
CopyAcceleratorTable(hAccel, accels, n);
accels[n].fVirt = flags;
accels[n].key = key;
accels[n].cmd = cmd;
HACCEL hNew = CreateAcceleratorTable(accels, n+1);
if (hNew) {
DestroyAcceleratorTable(hAccel);
hAccel = hNew;
}
free(accels);
}

BOOL RemoveAccelerator (int cmd) {
int n = CopyAcceleratorTable(hAccel, NULL, 0);
ACCEL* accels = (ACCEL*)malloc(sizeof(ACCEL) * (n+1));
CopyAcceleratorTable(hAccel, accels, n);
BOOL found = FALSE;
int i; for(i=0; i<n; i++) {
if (accels[i].cmd==cmd) {
accels[i] = accels[n -1];
found = TRUE;
break;
}}
if (found) {
HACCEL hNew = CreateAcceleratorTable(accels, n -1);
if (hNew) {
DestroyAcceleratorTable(hAccel);
hAccel = hNew;
}}
free(accels);
return found;
}

BOOL FindAccelerator (int cmd, int* flags, int* key) {
int n = CopyAcceleratorTable(hAccel, NULL, 0);
ACCEL* accels = (ACCEL*)malloc(sizeof(ACCEL) * (n+1));
CopyAcceleratorTable(hAccel, accels, n);
BOOL found = FALSE; int i; 
for (i=0; i<n; i++) {
if (accels[i].cmd==cmd) {
found = TRUE;
if (flags) *flags = accels[i].fVirt;
if (key) *key = accels[i].key;
break;
}}
free(accels);
return found;
}

void GoToLine (int ln) {
HWND edit = GetCurEditArea();
int pos = SendMessage(edit, EM_LINEINDEX, ln, 0);
SendMessage(edit, EM_SETSEL, pos, pos);
SendMessage(edit, EM_SCROLLCARET, 0, 0);
}

tregex FindData::newRegex (bool findOnly) {
using namespace boost;
int options =
(flags&FF_REGEX? regex_constants::perl | regex_constants::mod_s | regex_constants::collate | (findOnly?regex_constants::nosubs:0)  : regex_constants::literal)
| (flags&FF_CASE? 0 : regex_constants::icase);
return tregex(findText, options);
}

void FindNext () {
using namespace boost;
if (finds.size()<=0) { SearchReplaceDialog(false); return; }
FindData& fd = finds.front();
HWND edit = GetCurEditArea();
int pos;
SendMessage(edit, EM_GETSEL, 0, &pos);
tstring text = GetWindowText(edit);
tregex reg = fd.newRegex(false);
tcmatch m;
match_flag_type mtype = match_flag_type::match_default;
if (pos>0) mtype |= match_flag_type::match_prev_avail;
if (regex_search(text.data()+pos, text.data()+text.size(), m, reg, mtype)) {
int start = m[0].first - text.data();
int end = m[0].second - text.data();
SendMessage(edit, EM_SETSEL, start, end);
SendMessage(edit, EM_SCROLLCARET, 0, 0);
}
else MessageBeep(MB_ICONASTERISK);
}

void FindPrev () {
using namespace boost;
if (finds.size()<=0) { SearchReplaceDialog(false); return; }
FindData& fd = finds.front();
HWND edit = GetCurEditArea();
int pos, lastStart=-1, lastEnd=-1;
SendMessage(edit, EM_GETSEL, &pos, 0);
tstring text = GetWindowText(edit);
tregex reg = fd.newRegex(false);
match_flag_type mtype = match_flag_type::match_default;
if (pos>0) mtype |= match_flag_type::match_prev_avail;
for (tcregex_iterator _end, it(text.data(), text.data()+text.size(), reg, mtype); it!=_end; ++it) {
auto m = *it;
int start = m[0].first - text.data();
int end = m[0].second - text.data();
if (start>pos) break;
lastStart=start;
lastEnd=end;
}
if (lastStart>=0 && lastEnd>=0 && lastStart<=pos && lastEnd<=pos) {
SendMessage(edit, EM_SETSEL, lastStart, lastEnd);
SendMessage(edit, EM_SCROLLCARET, 0, 0);
}
else MessageBeep(MB_ICONASTERISK);
}

void FindNew(const tstring& searchText, bool scase, bool regex, bool up) {
FindData fd(searchText, TEXT(""), (scase?FF_CASE:0) | (regex?FF_REGEX:0) | (up?FF_UPWARDS:0) );
auto it = find(finds.begin(), finds.end(), fd);
if (it!=finds.end()) finds.erase(it);
finds.push_front(fd);
if (up) FindPrev();
else FindNext();
}

void SearchReplace (const tstring& searchText, const tstring& replaceText, bool scase, bool regex) {
using namespace boost;
FindData fd(searchText, replaceText, (scase?FF_CASE:0) | (regex?FF_REGEX:0) );
auto it = find(finds.begin(), finds.end(), fd);
if (it!=finds.end()) finds.erase(it);
finds.push_front(fd);
int start, end;
HWND edit = GetCurEditArea();
SendMessage(edit, EM_GETSEL, &start, &end);
tstring text = GetWindowText(edit);
if (start!=end) text = tstring(text.begin()+start, text.begin()+end);
tregex reg = fd.newRegex(true);
match_flag_type flags = (fd.flags&FF_REGEX? match_flag_type::match_default | match_flag_type::format_perl : match_flag_type::format_literal);
text = regex_replace(text, reg, fd.replaceText, flags);
if (start!=end) SendMessage(edit, EM_REPLACESEL, 0, text.c_str() );
else SetWindowText(edit, text);
SendMessage(edit, EM_SCROLLCARET, 0, 0);
}

void SaveCurFile (bool saveas) {
if (!curPage) return;
if (saveas || curPage->file.size()<=0) {
tstring file = FileDialog(win, FD_SAVE, curPage->file, msg("Save as") );
if (file.size()<=0) return;
curPage->file = file;
curPage->name = file.substr(1+file.rfind((TCHAR)'\\'));
SetWindowText(win, (curPage->name + TEXT(" - ") + appName).c_str() );
}
curPage->SaveText();
}

bool OpenFile3 (const tstring& file) {
for (int i=0; i<pages.size(); i++) {
auto page = pages[i];
if (file==page->file) {
SendMessage(tabctl, TCM_SETCURFOCUS, i, 0);
return true;
}}
return false;
}

bool OpenFile2 (const tstring& file, int flags) {
if (flags&OF_NEWINSTANCE) {
ShellExecute(win, TEXT("open"), appPath.c_str(), file.c_str(), NULL, SW_SHOW);
return true;
}
if (flags&OF_REUSEOPENEDTABS) {
COPYDATASTRUCT cp;
cp.dwData = 76;
cp.cbData = sizeof(TCHAR)*(file.size()+1);
cp.lpData = (LPVOID)file.c_str();
HWND hWin = NULL;
while (hWin=FindWindowEx(NULL, hWin, CLASSNAME, NULL)) {
if (SendMessage(hWin, WM_COPYDATA, win, &cp)) {
SetForegroundWindow(hWin);
if (flags&OF_EXITONDOUBLEOPEN) exit(0);
return true;
}}}
return false;
}

void OpenFile (const tstring& file, int flags=0) {
if (OpenFile2(file, flags)) return;
shared_ptr<Page> cp = curPage;
shared_ptr<Page> p(new TextPage());
p->LoadText(file);
p->name = file.substr(1+file.rfind((TCHAR)'\\'));
PageAdd(p);
if (cp&&cp->IsEmpty()) PageDelete(cp);
}

void OpenFileDialog (int flags) {
tstring file = (curPage? curPage->file : tstring(TEXT("")) );
file = FileDialog(win, FD_OPEN, file, msg("Open file") );
if (file.size()<=0) return;
OpenFile(file, flags);
}

void I18NMenus (HMENU menu) {
int count = GetMenuItemCount(menu);
if (count<=0) return;
for (int i=0; i<count; i++) {
int len = GetMenuString(menu, i, NULL, 0, MF_BYPOSITION);
wchar_t buf[len+1];
GetMenuString(menu, i, buf, len+1, MF_BYPOSITION);
string oldLabel = toString(buf);
string newLabel = msgs.get(oldLabel, oldLabel);
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_ID | MIIM_SUBMENU;
if (!GetMenuItemInfo(menu, i, TRUE, &mii)) continue;
int flg2 = MF_BYPOSITION | (mii.hSubMenu? MF_POPUP : MF_STRING);
ModifyMenu(menu, i, flg2, mii.wID, toTString(newLabel).c_str() );
if (mii.hSubMenu) I18NMenus(mii.hSubMenu);
}}

extern "C" int WINAPI WinMain (HINSTANCE hThisInstance,                      HINSTANCE hPrevInstance,                      LPSTR lpszArgument,                      int nWindowStile) {
hinstance = hThisInstance;
long long time = GetTickCount();

//signal(SIGSEGV, sigsegv);
//signal(SIGFPE, sigsegv);
//signal(SIGILL, sigsegv);
SetErrorMode(SEM_FAILCRITICALERRORS);

{//Getting paths and such global parameters
TCHAR fn[512]={0};
GetModuleFileName(NULL, fn, 511);
appPath = fn;
TCHAR* fnBs = tstrrchr(fn, (TCHAR)'\\');
TCHAR* fnDot = tstrrchr(fn, (TCHAR)'.');
*fnBs=0;
*fnDot=0;
tsnprintf(CLASSNAME, sizeof(CLASSNAME)/sizeof(TCHAR), TEXT("QC6Pad01_%s"), fnBs+1);
appName = fnBs+1;
appDir = fn;
config.load(appDir + TEXT("\\") + appName + TEXT(".ini") );
msgs.load(appDir + TEXT("\\") + appName + TEXT(".lng") );
}

{//Loading command line parameters
int argc=0;
wchar_t** args = CommandLineToArgvW(GetCommandLineW(),&argc);
for (wchar_t** arg = args; *arg; ++arg) argv.push_back(toTString(*arg));
LocalFree(args);
}
for (int i=1; i<argv.size(); i++) {
const tstring& arg = argv[i];
if (arg.size()<=0) continue;
else if (arg[0]=='-' || arg[0]=='/') { continue; } // options
if (OpenFile2(arg, OF_REUSEOPENEDTABS)) return 0;
}

{//Register class and controls block
    WNDCLASSEX wincl;
wincl.hInstance = hinstance;
wincl.lpszClassName = CLASSNAME;
wincl.lpfnWndProc = AppWinProc;
wincl.style = CS_DBLCLKS;
wincl.cbSize = sizeof (WNDCLASSEX);
wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
wincl.lpszMenuName = TEXT("menu");
wincl.cbClsExtra = 0;
wincl.cbWndExtra = 0;
wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;
if (!RegisterClassEx(&wincl)) {
MessageBox(NULL, TEXT("Couldn't register window class"), TEXT("Fatal error"), MB_OK|MB_ICONERROR);
return 1;
} 
INITCOMMONCONTROLSEX ccex = { sizeof(INITCOMMONCONTROLSEX), ICC_BAR_CLASSES |  ICC_HOTKEY_CLASS | ICC_PROGRESS_CLASS | ICC_UPDOWN_CLASS | ICC_TAB_CLASSES  };
if (!InitCommonControlsEx(&ccex)) {
MessageBox(NULL, TEXT("Couldn't initialize common controls"), TEXT("Fatal error"), MB_OK|MB_ICONERROR);
return 1;
}}

{//Create window block
win = CreateWindowEx(
WS_EX_CONTROLPARENT | WS_EX_ACCEPTFILES,
CLASSNAME, TEXT("6Pad++"), 
WS_VISIBLE | WS_OVERLAPPEDWINDOW,
CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
HWND_DESKTOP, NULL, hinstance, NULL);
RECT r; GetClientRect(win, &r);
tabctl = CreateWindowEx(
0, WC_TABCONTROL, NULL, 
WS_TABSTOP | WS_VISIBLE | WS_CHILD | TCS_SINGLELINE | TCS_FOCUSNEVER,
5, 5, r.right -10, r.bottom -49, 
win, (HMENU)IDC_TABCTL, hinstance, NULL);
status = CreateWindowEx(
0, L"STATIC", TEXT("Status"), 
WS_VISIBLE | WS_CHILD | WS_BORDER | SS_NOPREFIX | SS_LEFT, 
5, r.bottom -32, r.right -10, 27, 
win, (HMENU)IDC_STATUSBAR, hinstance, NULL);
hAccel = LoadAccelerators(hinstance, TEXT("accel"));
menu = GetMenu(win);
menuFormat = GetSubMenu(menu,2);
menuEncoding = GetSubMenu(menuFormat,0);
menuLineEnding = GetSubMenu(menuFormat,1);
menuIndentation = GetSubMenu(menuFormat,2);
for (int i=0; i<encodings.size(); i++) InsertMenu(menuEncoding, i, MF_STRING | MF_BYPOSITION, IDM_ENCODING+i, (TEXT("Encoding")+toTString(encodings[i])).c_str() );
for (int i=1; i<=8; i++) InsertMenu(menuIndentation, i, MF_STRING | MF_BYPOSITION, IDM_INDENTATION_SPACES -1 +i, tsnprintf(32, msg("%d spaces"), i).c_str() );
DeleteMenu(menuEncoding, encodings.size(), MF_BYPOSITION);
I18NMenus(menu);
}

for (int i=1; i<argv.size(); i++) {
const tstring& arg = argv[i];
if (arg.size()<=0) continue;
else if (arg[0]=='-' || arg[0]=='/') { continue; } // options
OpenFile(arg);
}
if (pages.size()<=0) PageAddEmpty(false);

PyStart();
time = GetTickCount() -time;
printf("Init time = %d ms\r\n", time);

ShowWindow(win, nWindowStile);
AppWindowOpened();
PageActivated(pages[0]);

MSG msg;
while (GetMessage(&msg,NULL,0,0)) {
if (PredispatchMessage(msg)) continue;
//if (IsDialogMessage(win, &msg)) continue;
if (TranslateAccelerator(win, hAccel, &msg)) continue;
TranslateMessage(&msg);	
DispatchMessage(&msg);
}
return msg.wParam;
}

bool ActionCommand (int cmd) {
switch(cmd){
case IDM_OPEN: OpenFileDialog(OF_REUSEOPENEDTABS); return true;
case IDM_OPEN_NI: OpenFileDialog(OF_NEWINSTANCE); return true;
case IDM_REOPEN: PageReopen(curPage); return true;
case IDM_SAVE: SaveCurFile(); return true;
case IDM_SAVE_AS: SaveCurFile(true); return true;
case IDM_NEW: PageAddEmpty(); return true;
case IDM_CLOSE: PageDelete(curPage); return true;
case IDM_SELECTALL: SendMessage(GetCurEditArea(), EM_SETSEL, 0, -1); return true;
case IDM_COPY: SendMessage(GetCurEditArea(), WM_COPY, 0, 0); return true;
case IDM_CUT: SendMessage(GetCurEditArea(), WM_CUT, 0, 0); return true;
case IDM_PASTE: SendMessage(GetCurEditArea(), WM_PASTE, 0, 0); return true;
case IDM_GOTOLINE: GoToLineDialog(); return true;
case IDM_FIND: SearchReplaceDialog(false); return true;
case IDM_REPLACE: SearchReplaceDialog(true); return true;
case IDM_FINDNEXT: FindNext(); return true;
case IDM_FINDPREV: FindPrev(); return true;
case IDM_NEXTPAGE: PageGoToNext(1); break;
case IDM_PREVPAGE: PageGoToNext(-1); break;
case IDM_EXIT: SendMessage(win, WM_CLOSE, 0, 0); return true;
case IDM_LE_DOS: case IDM_LE_UNIX: case IDM_LE_MAC: PageSetLineEnding(curPage, cmd-IDM_LE_DOS); return true;
case IDM_AUTOLINEBREAK: PageSetAutoLineBreak(curPage, curPage&&!(curPage->flags&PF_AUTOLINEBREAK)); return true;
}
if (cmd>=IDM_ENCODING && cmd<IDM_ENCODING+encodings.size() ) {
PageSetEncoding(curPage, cmd-IDM_ENCODING);
return true;
}
if (cmd>=IDM_INDENTATION_TABS && cmd<=IDM_INDENTATION_SPACES+8) {
PageSetIndentationMode(curPage, cmd-IDM_INDENTATION_TABS);
return true;
}
return false;
}

inline bool IsCtrlDown () { return GetKeyState(VK_CONTROL)<0; }
inline bool IsShiftDown () { return GetKeyState(VK_SHIFT)<0; }
inline bool IsAltDown () { return GetKeyState(VK_MENU)<0; }

BOOL WINAPI PredispatchMessage (MSG& msg) {
return false;
}

int EZGetNextParagPos (HWND hEdit, int pos) {
int nl=0, len = GetWindowTextLength(hEdit);
HLOCAL hLoc = (HLOCAL)SendMessage(hEdit, EM_GETHANDLE, 0, 0);
LPCTSTR text = (LPCTSTR)LocalLock(hLoc);
while(pos<len) {
TCHAR c = text[pos++];
if (c=='\n' && ++nl>=2) break;
else if (c>32) nl=0;
}
for (int i=pos; i<len && text[i]<=32; i++) if (text[i]=='\n') pos=i+1;
LocalUnlock(hLoc);
return pos;
}

int EZGetPrevParagPos (HWND hEdit, int pos) {
int nl=0, realBegPos=pos;
HLOCAL hLoc = (HLOCAL)SendMessage(hEdit, EM_GETHANDLE, 0, 0);
LPCTSTR text = (LPCTSTR)LocalLock(hLoc);
while (pos>0 && text[--pos]<=32);
while(pos>0) {
TCHAR c = text[--pos];
if (c=='\n' && ++nl>=2) break;
else if (c>32) { nl=0; realBegPos=pos; }
}
LocalUnlock(hLoc);
return realBegPos;
}

LRESULT EZHandleEnter (HWND hEdit) {
int pos=0, nLine=0;
SendMessage(hEdit, EM_GETSEL, &pos, 0);
nLine = SendMessage(hEdit, EM_LINEFROMCHAR, pos, 0);
tstring line = EditGetLine(hEdit, nLine, pos);
pos = line.find_first_not_of(TEXT("\t \xA0"));
if (pos<0 || pos>=line.size()) pos=0;
tstring repl = TEXT("\r\n") + line.substr(0, pos);
SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)repl.c_str() );
SendMessage(hEdit, EM_SCROLLCARET, 0, 0);
return true;
}

LRESULT EZHandleHome (HWND hEdit, bool normal) {
int pos=0, nLine=0, offset=0;
SendMessage(hEdit, EM_GETSEL, &pos, 0);
nLine = SendMessage(hEdit, EM_LINEFROMCHAR, pos, 0);
offset = SendMessage(hEdit, EM_LINEINDEX, nLine, 0);
if (normal) {
SendMessage(hEdit, EM_SETSEL, offset, offset);
return true;
}
tstring line = EditGetLine(hEdit, nLine, pos);
pos = line.find_first_not_of(TEXT("\t \xA0"));
if (pos<0 || pos>=line.size()) pos=0;
SendMessage(hEdit, EM_SETSEL, offset+pos, offset+pos);
return true;
}

LRESULT EZHandleCtrlDown (HWND hEdit) {
int pos=0;
SendMessage(hEdit, EM_GETSEL, 0, &pos);
pos = EZGetNextParagPos(hEdit, pos);
SendMessage(hEdit, EM_SETSEL, pos, pos);
return EZHandleHome(hEdit, false);
}

LRESULT EZHandleCtrlUp (HWND hEdit) {
int pos=0;
SendMessage(hEdit, EM_GETSEL, 0, &pos);
pos = EZGetPrevParagPos(hEdit, pos);
SendMessage(hEdit, EM_SETSEL, pos, pos);
return EZHandleHome(hEdit, false);
}

LRESULT EZHandleCtrlShiftDown (HWND hEdit) {
int spos=0, pos=0;
SendMessage(hEdit, EM_GETSEL, &spos, &pos);
pos = EZGetNextParagPos(hEdit, pos);
SendMessage(hEdit, EM_SETSEL, spos, pos);
return true;
}

LRESULT EZHandleCtrlShiftUp (HWND hEdit) {
int spos=0, pos=0;
SendMessage(hEdit, EM_GETSEL, &spos, &pos);
pos = EZGetPrevParagPos(hEdit, pos);
SendMessage(hEdit, EM_SETSEL, spos, pos);
return true;
}

LRESULT EZHandleF8 (HWND hEdit) {
if (IsShiftDown()) {
int curPos, markedPos = (int)GetProp(hEdit, TEXT("F8"));
SendMessage(hEdit, EM_GETSEL, &curPos, 0);
SendMessage(hEdit, EM_SETSEL, markedPos, curPos);
SendMessage(hEdit, EM_SCROLLCARET, 0, 0);
} else {
int curPos, curPos2;
SendMessage(hEdit, EM_GETSEL, &curPos, &curPos2);
if (curPos!=curPos2) {
SendMessage(hEdit, EM_SETSEL, curPos2, curPos);
SendMessage(hEdit, EM_SCROLLCARET, 0, 0);
}
else SetProp(hEdit, TEXT("F8"), (HANDLE)curPos);
}}

LRESULT EZHandleTab (HWND hEdit) {
int sPos=0, ePos=0;
SendMessage(hEdit, EM_GETSEL, &sPos, &ePos);
int sLine = SendMessage(hEdit, EM_LINEFROMCHAR, sPos, 0);
int sOffset = SendMessage(hEdit, EM_LINEINDEX, sLine, 0);
if (sPos!=ePos) { // There is a selection, indent/deindent
int eLine = SendMessage(hEdit, EM_LINEFROMCHAR, ePos, 0);
int eOffset = SendMessage(hEdit, EM_LINEINDEX, eLine, 0);
int eLineLen = SendMessage(hEdit, EM_LINELENGTH, ePos, 0);
if (sLine==eLine) Beep(800,150);
else {
tstring str = GetWindowText(hEdit).substr(sOffset, eOffset + eLineLen -sOffset);
tstring indent = curPage->indentationMode==0? TEXT("\t") : tstring(curPage->indentationMode, ' ');
if (IsShiftDown()) str = preg_replace(str, TEXT("^")+indent, TEXT(""));
else str = preg_replace(str, TEXT("^"), indent);
SendMessage(hEdit, EM_SETSEL, sOffset, eOffset+eLineLen);
SendMessage(hEdit, EM_REPLACESEL, 0, str.c_str());
}}
else { // There is no selection
tstring line = EditGetLine(hEdit, sLine, sPos);
int pos = line.find_first_not_of(TEXT("\t \xA0"));
if (pos<0 || pos>=line.size()) pos=0;
if (sPos >= sOffset+pos+1) { Beep(1000,200); return true; }
if (IsShiftDown()) {
for (int i=0; i<1 || i<curPage->indentationMode; i++) SendMessage(hEdit, WM_CHAR, VK_BACK, 0);
return true;
}
else { // shift not down
if (curPage->indentationMode>0) for (int i=0; i<curPage->indentationMode; i++) SendMessage(hEdit, WM_CHAR, 32, 0);
return curPage->indentationMode>0;
}}}

LRESULT CALLBACK EditAreaWinProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR subclassId, DWORD_PTR dwUdata) {
if (msg==WM_CHAR) {
switch(LOWORD(wp)) {
case VK_RETURN: return EZHandleEnter(hwnd);
case VK_TAB: if (EZHandleTab(hwnd)) return true; break;
}}
else if (msg==WM_KEYDOWN) {
switch(LOWORD(wp)) {
case VK_DOWN:
if (IsCtrlDown()) {
if (IsShiftDown()) return EZHandleCtrlShiftDown(hwnd);
else return EZHandleCtrlDown(hwnd);
}
break;
case VK_UP: 
if (IsCtrlDown()) {
if (IsShiftDown()) return EZHandleCtrlShiftUp(hwnd);
else return EZHandleCtrlUp(hwnd);
}
break;
case VK_TAB:
if (IsCtrlDown()) return PageGoToNext(IsShiftDown()? -1 : 1);
break;
case VK_HOME:
if (!IsCtrlDown() && !IsShiftDown()) return EZHandleHome(hwnd, IsAltDown());
break;
case VK_F8:
if (!IsCtrlDown() && !IsAltDown()) return EZHandleF8(hwnd);
break;
}}
else if (msg==WM_KEYUP) {
StatusBarUpdate(hwnd);
}
else if (msg==WM_PASTE) {
tstring line = EditGetLine(hwnd);
tstring str = GetClipboardText();
int pos = line.find_first_not_of(TEXT(" \t"));
if (pos>=line.size()) pos=line.size();
tstring indent = line.substr(0,pos);
PrepareSmartPaste(str, indent);
SendMessage(hwnd, EM_REPLACESEL, 0, str.c_str());
return true;
}
else if (msg==WM_COPY) {
int spos=0, epos=0;
SendMessage(hwnd, EM_GETSEL, &spos, &epos);
if (spos==epos) {
tstring str = EditGetLine(hwnd);
SetClipboardText(str);
return true;
}}
else if (msg==WM_CUT) {
int spos=0, epos=0;
SendMessage(hwnd, EM_GETSEL, &spos, &epos);
if (spos==epos) {
int lnum = SendMessage(hwnd, EM_LINEFROMCHAR, spos, 0);
int lindex = SendMessage(hwnd, EM_LINEINDEX, lnum, 0);
int llen = SendMessage(hwnd, EM_LINELENGTH, spos, 0);
SendMessage(hwnd, EM_SETSEL, lindex+llen, lindex+llen+2);
SendMessage(hwnd, EM_REPLACESEL, 0, 0);
SendMessage(hwnd, EM_SETSEL, lindex, lindex+llen);
}}
return DefSubclassProc(hwnd, msg, wp, lp);
}

INT_PTR CALLBACK GoToLineDlgProc (HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp) {
switch (umsg) {
case WM_INITDIALOG : {
HWND edit = GetCurEditArea();
int num = SendMessage(edit, EM_GETLINECOUNT, 0, 0);
SetWindowText(hwnd, msg("Go to line") );
SetDlgItemText(hwnd, 1001, tsnprintf(128, msg("Enter a line number between 1 and %d"), num)+TEXT(":") );
SetDlgItemText(hwnd, IDOK, msg("&OK") );
SetDlgItemText(hwnd, IDCANCEL, msg("Ca&ncel") ); 
num = SendMessage(edit, EM_LINEFROMCHAR, -1, 0);
SetDlgItemInt(hwnd, 1002, num+1, FALSE);
}return TRUE;
case WM_COMMAND :
switch (LOWORD(wp)) {
case IDOK : {
HWND edit = GetCurEditArea();
tstring tmp = GetDlgItemText(hwnd, 1002);
int num = toInt(tmp);
if (tmp[0]=='+' || tmp[0]=='-') num += SendMessage(edit, EM_LINEFROMCHAR, -1, 0);
else --num;
int max = SendMessage(edit, EM_GETLINECOUNT, 0, 0);
if (num<0) num=0;
else if (num>=max) num=max-1;
GoToLine(num);
}
case IDCANCEL : EndDialog(hwnd, wp); return TRUE;
}}
return FALSE;
}

void GoToLineDialog () {
DialogBoxParam(IDD_GOTOLINE, win, GoToLineDlgProc, NULL);
}

INT_PTR CALLBACK SearchReplaceDlgProc (HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp) {
switch (umsg) {
case WM_INITDIALOG :
SetWindowText(hwnd, msg(!lp? "Find" : "Search and replace"));
SetDlgItemText(hwnd, IDOK, msg(lp? "Replace &all" : "&OK") );
SetDlgItemText(hwnd, IDCANCEL, msg("Ca&ncel"));
SetDlgItemText(hwnd, 2000, msg("&Search for") + TEXT(":") );
SetDlgItemText(hwnd, 2001, msg("&Replace with") + TEXT(":") );
SetDlgItemText(hwnd, 2002, msg("Direction")+TEXT(":") );
SetDlgItemText(hwnd, 1003, msg("&Case sensitive"));
SetDlgItemText(hwnd, 1004, msg("Regular e&xpression"));
SetDlgItemText(hwnd, 1005, msg("&Up"));
SetDlgItemText(hwnd, 1006, msg("&Down"));
EnableDlgItem(hwnd, 1002, lp);
EnableDlgItem(hwnd, 1005, !lp);
EnableDlgItem(hwnd, 1006, !lp);
//SetDlgItemText(hwnd, 1001, curSearch);
//SetDlgItemText(hwnd, 1002, curReplace);
//SendMessage(GetDlgItem(hwnd, 1003), BM_SETCHECK, (searchFlags&1)?BST_CHECKED:BST_UNCHECKED, 0);
//SendMessage(GetDlgItem(hwnd, 1007), BM_SETCHECK, (searchFlags&2)?BST_CHECKED:BST_UNCHECKED, 0);
//SendMessage(GetDlgItem(hwnd, 1004), BM_SETCHECK, searchRegex?BST_CHECKED:BST_UNCHECKED, 0);
//SendMessage(GetDlgItem(hwnd, searchUpward?1005:1006), BM_SETCHECK, BST_CHECKED, 0);
//if (searchList) {
//HWND hCb = GetDlgItem(hwnd,1001);
//int i,n; for (i=0, n=l_len(searchList); i<n; i++) SendMessage(hCb, CB_ADDSTRING, 0, l_item(searchList,i));
//}
//if (replaceList) {
//HWND hCb = GetDlgItem(hwnd,1002);
//int i,n; for (i=0, n=l_len(replaceList); i<n; i++) SendMessage(hCb, CB_ADDSTRING, 0, l_item(replaceList,i));
//}
return TRUE;
case WM_COMMAND :
switch (LOWORD(wp)) {
case IDOK : {
BOOL sr = IsDlgItemEnabled(hwnd, 1002);
BOOL searchCase = IsDlgButtonChecked(hwnd, 1003);
BOOL searchRegex = IsDlgButtonChecked(hwnd, 1004);
BOOL searchUp = IsDlgButtonChecked(hwnd, 1005);
tstring searchText = GetDlgItemText(hwnd, 1001);
tstring replaceText = GetDlgItemText(hwnd, 1002);
if (sr) SearchReplace(searchText, replaceText, searchCase, searchRegex);
else FindNew(searchText, searchCase, searchRegex, searchUp);
}
case IDCANCEL : EndDialog(hwnd, wp); return TRUE;
}}
return FALSE;
}

void SearchReplaceDialog (bool replace) {
DialogBoxParam(IDD_SEARCHREPLACE, win, SearchReplaceDlgProc, replace);
}

LRESULT WINAPI AppWinProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
switch(msg){
case WM_COMMAND:
if (ActionCommand(LOWORD(wp))) return TRUE;
break;
case WM_NOTIFY : switch (((LPNMHDR)lp)->code) {
case TCN_SELCHANGING : 
if (!PageDeactivated(curPage)) return TRUE;
SendMessage(tabctl, TCM_HIGHLIGHTITEM, SendMessage(tabctl, TCM_GETCURSEL, 0, 0), FALSE);
break;
case TCN_SELCHANGE : {
int i = SendMessage(tabctl, TCM_GETCURSEL, 0, 0);
PageActivated(pages[i]);
SendMessage(tabctl, TCM_HIGHLIGHTITEM, i, TRUE);
}break;//TCN_CHANGE
}break;//notifications
case WM_RUNPROC : {
typedef std::function<void()> Proc;
Proc* proc = (Proc*)lp;
(*proc)();
delete proc;
return true;
}break;
case WM_COPYDATA: {
COPYDATASTRUCT& cp = *(COPYDATASTRUCT*)(lp);
if (cp.dwData==76) return OpenFile3((LPCTSTR)(cp.lpData));
}break;
case WM_SETFOCUS :
AppWindowGainedFocus();
break;
case WM_ACTIVATE :
if(wp) AppWindowActivated();
break;
case WM_SIZE :
AppWindowResized();
break;
case WM_CLOSE:
if (!AppWindowClosing()) return true;
AppWindowClosed();
break;
case WM_DESTROY:
PostQuitMessage(0);
break;
}
return DefWindowProc(hwnd, msg, wp, lp);
}

