#include "global.h"
#include "strings.hpp"
#include "page.h"
#include "inifile.h"
#include "file.h"
#include "dialogs.h"
#include "accelerators.h"
#include "Thread.h"
#include "Resource.h"
#include "UniversalSpeech.h"
#include "python34.h"
#include "sixpad.h"
#include<boost/signals2.hpp>
#include<list>
#include<unordered_map>
#include<map>
#include<functional>
#include<clocale>
#include<io.h>
#include<fcntl.h>
#include<shellapi.h>
using namespace std;
using boost::signals2::signal;

#define OF_REUSEOPENEDTABS 1
#define OF_NEWINSTANCE 2
#define OF_EXITONDOUBLEOPEN 4

IniFile msgs, config;
tstring appPath, appDir, appName, configFileName, appLocale;
shared_ptr<Page> curPage(0);
list<tstring> consoleInput = { TEXT("import sixpad"), TEXT("from sixpad import window") };
list<tstring> recentFiles;
vector<int> encodings = { GetACP(), GetOEMCP(), CP_UTF8, CP_UTF8_BOM, CP_UTF16_LE, CP_UTF16_LE_BOM, CP_UTF16_BE, CP_UTF16_BE_BOM };
vector<tstring> argv;
vector<shared_ptr<Page>> pages;
vector<HWND> modlessWindows;
unordered_map<int, function<void(void)>> userCommands, timers;
unordered_map<string,function<Page*()>> pageFactories = { {"text", [](){return new Page();}} };

signal<void()> onactivated, ondeactivated, onclosed, onresized;
signal<bool(), BoolSignalCombiner> onclose;
signal<bool(const tstring&), BoolSignalCombiner> onquickReach;
signal<bool(const tstring&, int, int), BoolSignalCombiner> onfileDropped;
signal<var(const tstring&)> onpageBeforeOpen, ontitle;
signal<var(const tstring&,int)> onquickReachAutocomplete;
signal<void(shared_ptr<Page>)> onpageOpened;

TCHAR CLASSNAME[32] = {0};
bool firstInstance = false, headless=false, isDebug = false;
HINSTANCE hinstance = 0;
HWND win=0, tabctl=0, status=0, consoleWin=0;
HFONT gfont = NULL;
HMENU menu = 0, menuFormat=0, menuEncoding=0, menuLineEnding=0, menuIndentation=0, menuRecentFiles = 0;
HACCEL hAccel = 0, hGlobAccel=0;
HANDLE consoleInputEvent=0;
CRITICAL_SECTION csConsoleInput;

shared_ptr<Page> OpenFile (const tstring& file, int flags=0);
void OpenConsoleWindow (void);
void UpdateRecentFilesMenu (void);
void ParseLineCol (tstring& file, int& line, int& col);
void ClearTimeout (int id);
void CSignal ( void(*)(int) );
LRESULT WINAPI AppWinProc (HWND, UINT, WPARAM, LPARAM);

tstring msg (const char* x) {
auto it = msgs.find(string(x));
if (it!=msgs.end()) return toTString(it->second);
else return toTString(string(x));
}

void UpdateWindowTitle () {
tstring title = curPage->name + TEXT(" - ") + appName;
var re = ontitle(title);
if (re.getType()==T_STR) title = re.toTString();
SetWindowText(win, title.c_str() );
}

bool PageDeactivated (shared_ptr<Page> p) {
if (!p) return true;
if (!p->ondeactivate(p)) return false;
p->HideZone();
p->ondeactivated(p);
return true;
}

void PageActivated (shared_ptr<Page> p) {
if (!p) return;
curPage = p;
RECT r; GetClientRect(win, &r);
r.left = 5; r.top = 5; r.right -= 10; r.bottom -= 49;
SendMessage(tabctl, TCM_ADJUSTRECT, FALSE, &r);
p->ShowZone(r);
p->FocusZone();
p->UpdateStatusBar(status);
UpdateWindowTitle();
int encidx = -1; for (int i=0; i<encodings.size(); i++) { if (p->encoding==encodings[i]) { encidx=i; break; }}
CheckMenuRadioItem(menuEncoding, 0, encodings.size(), encidx, MF_BYPOSITION);
CheckMenuRadioItem(menuLineEnding, 0, 2, p->lineEnding, MF_BYPOSITION);
CheckMenuRadioItem(menuIndentation, 0, 8, p->indentationMode, MF_BYPOSITION);
CheckMenuItem(menuFormat, IDM_AUTOLINEBREAK, MF_BYCOMMAND | (p->flags&PF_AUTOLINEBREAK? MF_CHECKED : MF_UNCHECKED));
EnableMenuItem2(menu, IDM_SAVE, MF_BYCOMMAND, !(p->flags&PF_NOSAVE));
EnableMenuItem2(menu, IDM_SAVE_AS, MF_BYCOMMAND, !(p->flags&PF_NOSAVE));
EnableMenuItem2(menu, IDM_REOPEN, MF_BYCOMMAND, !(p->flags&PF_NORELOAD));
EnableMenuItem2(menu, IDM_UNDO, MF_BYCOMMAND, !(p->flags&PF_NOUNDO));
EnableMenuItem2(menu, IDM_REDO, MF_BYCOMMAND, !(p->flags&PF_NOUNDO));
EnableMenuItem2(menu, IDM_COPY, MF_BYCOMMAND, !(p->flags&PF_NOCOPY));
EnableMenuItem2(menu, IDM_CUT, MF_BYCOMMAND, !(p->flags&PF_NOCOPY));
EnableMenuItem2(menu, IDM_PASTE, MF_BYCOMMAND, !(p->flags&PF_NOPASTE));
EnableMenuItem2(menu, IDM_SELECTALL, MF_BYCOMMAND, !(p->flags&PF_NOSELECTALL));
EnableMenuItem2(menu, IDM_GOTOLINE, MF_BYCOMMAND, !(p->flags&PF_NOGOTO));
EnableMenuItem2(menu, IDM_FIND, MF_BYCOMMAND, !(p->flags&PF_NOFIND));
EnableMenuItem2(menu, IDM_FINDNEXT, MF_BYCOMMAND, !(p->flags&PF_NOFIND));
EnableMenuItem2(menu, IDM_FINDPREV, MF_BYCOMMAND, !(p->flags&PF_NOFIND));
EnableMenuItem2(menu, IDM_REPLACE, MF_BYCOMMAND, !(p->flags&(PF_NOFIND|PF_NOREPLACE)));
EnableMenuItem2(menuFormat, 0, MF_BYPOSITION, !(p->flags&PF_NOENCODING));
EnableMenuItem2(menuFormat, 1, MF_BYPOSITION, !(p->flags&PF_NOLINEENDING));
EnableMenuItem2(menuFormat, 2, MF_BYPOSITION, !(p->flags&PF_NOINDENTATION));
EnableMenuItem2(menuFormat, IDM_AUTOLINEBREAK, MF_BYCOMMAND, !(p->flags&PF_NOAUTOLINEBREAK));
curPage->onactivated(curPage);
}
static tstring GetDefaultWindowTitle (void) {
return appName  + TEXT(" ") + tstring(TEXT(SIXPAD_VERSION));
}

static int EncodingAdd (int enc) {
//if (!IsValidCodePage(enc)) return -1;
auto it = std::find(encodings.begin(), encodings.end(), enc);
if (it!=encodings.end()) return it - encodings.begin();
int i = encodings.size();
encodings.push_back(enc);
InsertMenu(menuEncoding, i, MF_STRING | MF_BYPOSITION, IDM_ENCODING+i, msg(("Encoding" + toString(enc)).c_str()).c_str() );
return i;
}

void PageNoneActive (void) {
EnableMenuItem2(menu, IDM_SAVE, MF_BYCOMMAND, false);
EnableMenuItem2(menu, IDM_SAVE_AS, MF_BYCOMMAND, false);
EnableMenuItem2(menu, IDM_REOPEN, MF_BYCOMMAND, false);
EnableMenuItem2(menu, IDM_UNDO, MF_BYCOMMAND, false);
EnableMenuItem2(menu, IDM_REDO, MF_BYCOMMAND, false);
EnableMenuItem2(menu, IDM_COPY, MF_BYCOMMAND, false);
EnableMenuItem2(menu, IDM_CUT, MF_BYCOMMAND, false);
EnableMenuItem2(menu, IDM_PASTE, MF_BYCOMMAND, false);
EnableMenuItem2(menu, IDM_SELECTALL, MF_BYCOMMAND, false);
EnableMenuItem2(menu, IDM_GOTOLINE, MF_BYCOMMAND, false);
EnableMenuItem2(menu, IDM_FIND, MF_BYCOMMAND, false);
EnableMenuItem2(menu, IDM_FINDNEXT, MF_BYCOMMAND, false);
EnableMenuItem2(menu, IDM_FINDPREV, MF_BYCOMMAND, false);
EnableMenuItem2(menu, IDM_REPLACE, MF_BYCOMMAND, false);
EnableMenuItem2(menuFormat, 0, MF_BYPOSITION, false);
EnableMenuItem2(menuFormat, 1, MF_BYPOSITION, false);
EnableMenuItem2(menuFormat, 2, MF_BYPOSITION, false);
EnableMenuItem2(menuFormat, IDM_AUTOLINEBREAK, MF_BYCOMMAND, false);
SetWindowText(win, GetDefaultWindowTitle() );
}

inline void PageSetLineEnding (shared_ptr<Page> p, int le) {
if (p==curPage) CheckMenuRadioItem(menuLineEnding, 0, 2, p->lineEnding, MF_BYPOSITION);
}

inline void PageSetEncoding (shared_ptr<Page> p, int enc) {
int idx = std::find(encodings.begin(), encodings.end(), enc) - encodings.begin();
if (idx>=0&&idx<encodings.size()) enc=idx;
else enc = EncodingAdd(enc);
if (enc<0 || enc>=encodings.size()) return;
if (p==curPage) CheckMenuRadioItem(menuEncoding, 0, encodings.size(), enc, MF_BYPOSITION);
}

inline void PageSetIndentationMode (shared_ptr<Page> p, int im) {
if (p==curPage) CheckMenuRadioItem(menuIndentation, 0, 8, p->indentationMode, MF_BYPOSITION);
}

inline void PageSetName (shared_ptr<Page> p, const tstring& name) {
int pos = std::find(pages.begin(), pages.end(), p) -pages.begin();
if (pos<0 || pos>=pages.size()) return;
TCITEM it;
it.mask = TCIF_TEXT;
it.pszText = (LPTSTR)(p->name.c_str());
SendMessage(tabctl, TCM_SETITEM, pos, &it);
if (curPage==p) UpdateWindowTitle();
}

inline void PageSetAutoLineBreak (shared_ptr<Page> p, bool alb) {
/*if (!p) return;
if (alb) p->flags |= PF_AUTOLINEBREAK;
else p->flags &=~PF_AUTOLINEBREAK;
if (p==curPage) CheckMenuItem(menuFormat, IDM_AUTOLINEBREAK, MF_BYCOMMAND | (p->flags&PF_AUTOLINEBREAK? MF_CHECKED : MF_UNCHECKED));
p->CreateZone(tabctl);
if (p==curPage) PageActivated(p);*/
}

void PageActivate (int i) {
SendMessage(tabctl, TCM_SETCURFOCUS, i, 0);
}

inline void PageEnsureFocus (shared_ptr<Page> p) {
if (GetForegroundWindow()!=win) SetForegroundWindow(win);
if (curPage!=p) {
int i = std::find(pages.begin(), pages.end(), p) -pages.begin();
if (i>=0&&i<pages.size()) PageActivate(i);
}
p->FocusZone();
}

void PageAttrChanged (shared_ptr<Page> p, int attr, var val) {
switch(attr){
case PA_NAME: PageSetName(p, val.toTString()); break;
case PA_ENCODING: PageSetEncoding(p, val.toInt()); break;
case PA_LINE_ENDING: PageSetLineEnding(p, val.toInt()); break;
case PA_INDENTATION_MODE: PageSetIndentationMode(p, val.toInt()); break;
case PA_AUTOLINEBREAK: PageSetAutoLineBreak(p, val.toBool()); break;
case PA_FOCUS: PageEnsureFocus(p); break;
}}

shared_ptr<Page> PageCreate (const string& type) {
function<Page*()> f = pageFactories[type];
return shared_ptr<Page>( f? f() : 0);
}

void PageSaved (shared_ptr<Page> p) {
if (curPage==p) UpdateWindowTitle();
if (p->file==configFileName) { config.clear(); config.load(configFileName); }
}

void PageClosed (shared_ptr<Page> p) {
if (curPage==p) { PageDeactivated(p); curPage=0; }
int idx = std::find(pages.begin(), pages.end(), p) -pages.begin();
pages.erase(pages.begin()+idx);
SendMessage(tabctl, TCM_DELETEITEM, idx, 0);
if (pages.size()>idx) PageActivate(idx);
else if (pages.size()>0) PageActivate(idx -1);
else if (pages.size()==0) PageNoneActive();
}

void PageOpened (shared_ptr<Page> p) { 
onpageOpened(p);
p->onsaved.connect(PageSaved);
p->onclosed.connect( 2147483647, PageClosed);
p->onattrChange.connect(PageAttrChanged);
}

void PageAdd (shared_ptr<Page> p, bool focus = true) {
p->CreateZone(tabctl);
p->LoadFile();
pages.push_back(p);
TCITEM it;
it.mask = TCIF_TEXT;
it.pszText = (LPTSTR)(p->name.c_str());
int pos = SendMessage(tabctl, TCM_GETITEMCOUNT, 0, 0);
SendMessage(tabctl, TCM_INSERTITEM, pos, &it);
int oldpos = SendMessage(tabctl, TCM_GETCURSEL, 0, 0);
PageOpened(p);
if (focus) PageActivate(pages.size() -1);
if (focus&&pages.size()==1) PageActivated(p);
}

shared_ptr<Page> PageAddEmpty (bool focus = true, const string& type = "text") {
static int count = 0;
shared_ptr<Page> p = PageCreate(type);
p->name = msg("Untitled") + TEXT(" ") + toTString(++count);
p->encoding = config.get("defaultEncoding", (int)GetACP() );
p->lineEnding = config.get("defaultLineEnding", LE_DOS);
p->indentationMode = config.get("defaultIndentationMode", 0);
p->flags = config.get("defaultAutoLineBreak", false)? PF_AUTOLINEBREAK : 0;
PageAdd(p, focus);
return p;
}

void PageReopen (shared_ptr<Page> p) {
if (!p) return;
p->LoadFile(TEXT(""), false);
}

bool PageGoToNext (int delta = 1) {
int cur = SendMessage(tabctl, TCM_GETCURSEL, 0, 0);
int next = (cur + delta + pages.size()) %pages.size();
if (next!=cur) PageActivate(next);
else return false;
return true;
}

void RegisterPageFactory (const string& name, const function<Page*()>& f) {
pageFactories[name] = f;
}

bool AppWindowClosing () {
if (!onclose()) return false;
for (int i=0, j=0; i<pages.size(); i++) {
shared_ptr<Page> p = pages[i];
if ((p->flags&PF_NOSAVE) || p->file.size()<=0) continue;
int pos = p->GetCurrentPosition();
config.set("lastFile" + toString(j), p->file);
config.set("lastFilePos" + toString(j), pos);
config.erase("lastFile" + toString(++j));
}
for (int i=pages.size() -1; i>=0; i--) if (!pages[i]->Close()) return false;
return true;
}

void AppWindowClosed () { 
onclosed();
}

void AppWindowActivated () {
static bool on = false;
if (on) return;
struct _tmp_{bool&b; _tmp_(bool&x):b(x){b=true;} ~_tmp_(){b=false;} } _tmp1_(on);
onactivated();
for (auto p: pages) {
if (p->CheckFileModification()) {
if (p->IsModified() && IDYES!=MessageBox(win, tsnprintf(512, msg("%s has been modified in another application. Do you want to reload it ?"), p->name.c_str()).c_str(), p->name.c_str(), MB_ICONEXCLAMATION | MB_YESNO) ) p->lastSave = GetCurTime();
else p->LoadFile(TEXT(""),false);
}}}

void AppWindowDeactivated () {
ondeactivated();
}

void AppWindowGainedFocus () {
if (curPage) curPage->FocusZone();
}

void AppWindowResized () {
RECT r; GetClientRect(win, &r);
MoveWindow(tabctl, 5, 5, r.right -10, r.bottom -49, TRUE);
MoveWindow(status, 5, r.bottom -32, r.right -10, 27, TRUE);
if (curPage) {
r.left = 5; r.top = 5; r.right -= 10; r.bottom -= 49;
SendMessage(tabctl, TCM_ADJUSTRECT, FALSE, &r);
curPage->ResizeZone(r);
}
onresized();
}

int AppAddEvent (const string& type, const PySafeObject& cb) {
connection con; 
if(false){}
#define E(n) else if (type==#n) con = on##n .connect(cb.asFunction<typename decltype(on##n)::signature_type>());
E(pageBeforeOpen) E(pageOpened)
E(close) E(resized) E(activated) E(deactivated)
E(title) E(fileDropped)
E(quickReach) E(quickReachAutocomplete)
#undef E
if (con.connected()) return AddSignalConnection(con);
else return 0;
}

int AppRemoveEvent (const string& type, int id) {
connection con = RemoveSignalConnection(id);
bool re = con.connected();
con.disconnect();
return re;
}

static void EncodingShowAll () {
map<tstring, int, nat_less<TCHAR>> encmap;
vector<tstring> encstrs;
vector<int> encvals;
int curenc = (curPage? curPage->encoding : -1);
for (int enc: getAllAvailableEncodings()) {
if (enc==20127 || enc==21027) continue;
tstring name = str_replace(msg(("Encoding" + toString(enc)).c_str()), TEXT("&"), TEXT(""));
encmap.insert(pair<tstring,int>( name, enc));
}
for (auto it: encmap) {
if (it.second==curenc) curenc = encvals.size();
encstrs.push_back(it.first);
encvals.push_back(it.second);
}
curenc = ChoiceDialog(win, msg("Choose encoding"), msg("Choose an encoding in the list below") + TEXT(":"), encstrs, curenc);
if (curPage && curenc>=0 && curenc<=encvals.size()) curPage->SetEncoding(encvals[curenc]);
}

static void DoPaste (void) {
if (IsClipboardFormatAvailable(CF_HDROP) && OpenClipboard(win)) {
HGLOBAL hMem = GetClipboardData(CF_HDROP);
HDROP hDrop = (HDROP)GlobalLock(hMem);
SendMessage(win, WM_DROPFILES, hDrop, 0);
GlobalUnlock(hMem);
CloseClipboard();
}
else if (curPage) curPage->Paste();
}

static void DoDropFiles (HDROP hDrop) {
POINT pt;
int nFiles = DragQueryFile(hDrop, -1, NULL, NULL);
DragQueryPoint(hDrop, &pt);
if (nFiles<=0) return;
for (int i=0; i<nFiles; i++) {
TCHAR buf[300] = {0};
if (!DragQueryFile(hDrop, i, buf, 299)) break;
tstring file = buf;
if (curPage && !curPage->onfileDropped(curPage, file, pt.x, pt.y)) continue;
else if (!onfileDropped(file, pt.x, pt.y)) continue;
if (2==config.get("instanceMode",0)) OpenFile(file, OF_NEWINSTANCE); 
else OpenFile(file, OF_REUSEOPENEDTABS);
}}

tstring ConsoleRead (void) {
while (consoleInput.size()<=0) WaitForSingleObject(consoleInputEvent, INFINITE);
SCOPE_LOCK(csConsoleInput);
tstring s = consoleInput.front() + TEXT("\n");
consoleInput.pop_front();
return s;
}

void ConsolePrint (const tstring& s2, bool say) {
tstring s=s2; s = preg_replace(s, TEXT("(?:\r\n|\n|\r)"), TEXT("\r\n"));
if (!RELEASE) {
fprintf(stderr, "%ls", s.c_str());
fflush(stderr);
}
if (s2!=TEXT(">>> ")) {
if (!consoleWin ) RunSync(OpenConsoleWindow);
if (say) speechSay(s2.c_str(), false);
}
if (consoleWin) SendMessage(consoleWin, WM_COMMAND, 999, &s);
}

bool OpenFile3 (const tstring& file) {
for (int i=0; i<pages.size(); i++) {
auto page = pages[i];
if (file==page->file) {
SendMessage(tabctl, TCM_SETCURFOCUS, i, 0);
return true;
}}
if (1==config.get("instanceMode", 0)) {
OpenFile(file);
return true;
}
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
//if (flags&OF_EXITONDOUBLEOPEN) exit(0);
return true;
}}}
return false;
}

shared_ptr<Page> OpenFile (const tstring& file1, int flags) {
tstring file=file1; int line=0, col=0;
ParseLineCol(file, line, col);
if (OpenFile2(file, flags)) return NULL;
string type = "text";
var vtype = onpageBeforeOpen(file);
if (vtype.getType()==T_STR) type = toString(vtype.toTString());
shared_ptr<Page> cp = curPage;
shared_ptr<Page> p = PageCreate(type);
if (!p) return p;
p->file = file;
PageAdd(p);
if (line>0&&col>0) p->SetCurrentPositionLC(line -1, col -1);
if (cp&&cp->IsEmpty()) cp->Close();
auto itrf = std::find(recentFiles.begin(), recentFiles.end(), file);
if (itrf!=recentFiles.end()) recentFiles.erase(itrf);
recentFiles.push_front(file);
if (recentFiles.size()>config.get("maxRecentFiles", 10)) recentFiles.pop_back();
UpdateRecentFilesMenu();
return p;
}

void OpenFileDialog (int flags) {
tstring file = (curPage? curPage->file : tstring(TEXT("")) );
file = FileDialog(win, FD_OPEN | FD_MULTI, file, msg("Open") );
if (file.size()<=0) return;
int nVbar = file.find('|');
if (nVbar<0 || nVbar>=file.size()) OpenFile(file, flags);
else {
vector<tstring> files = split(file, TEXT("|"));
for (int i=1, n=files.size(); i<n; i++) {
OpenFile(files[0] + TEXT("\\") + files[i], flags);
}}}

void UpdateRecentFilesMenu (void) {
for (int i=recentFiles.size() -1; i>=0; i--)  DeleteMenu(menuRecentFiles, i, MF_BYPOSITION);
int i=0;
for (const tstring& file: recentFiles) {
InsertMenu(menuRecentFiles, i, MF_STRING | MF_BYPOSITION, IDM_RECENT_FILE +i, tsnprintf(512, TEXT("&%d. %ls"), i+1, file.c_str() ).c_str() );
i++;
}
DeleteMenu(menuRecentFiles, recentFiles.size(), MF_BYPOSITION);
}

void I18NMenus (HMENU menu) {
int count = GetMenuItemCount(menu);
if (count<=0) return;
for (int i=0; i<count; i++) {
int len = GetMenuString(menu, i, NULL, 0, MF_BYPOSITION);
wchar_t buf[len+1];
GetMenuString(menu, i, buf, len+1, MF_BYPOSITION);
string oldLabel = toString(buf);
tstring newLabel = msg(oldLabel.c_str());
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_ID | MIIM_SUBMENU;
if (!GetMenuItemInfo(menu, i, TRUE, &mii)) continue;
int accFlags=0, accKey=0;
int flg2 = MF_BYPOSITION | (mii.hSubMenu? MF_POPUP : MF_STRING);
if (FindAccelerator((int&)(mii.wID), accFlags, accKey)) newLabel += TEXT("\t\t") + KeyCodeToName(accFlags, accKey, true);
ModifyMenu(menu, i, flg2, mii.wID, newLabel.c_str() );
if (mii.hSubMenu) I18NMenus(mii.hSubMenu);
}}

void SetMenuName (HMENU, int, BOOL, LPCTSTR);
void SetMenuNamesFromResource (HMENU menu) {
Resource res(TEXT("menuNames"),257);
const char *str, *data = (const char*)res.data();
WORD cmd=0;
int count=0;
while(cmd!=0xFFFF){
if (++count>24) break;
cmd = *(const WORD*)(data);
if (cmd==0xFFFF) break;
data += sizeof(WORD);
str = data;
data += 2+strlen(str);
SetMenuName(menu, cmd, cmd<1000, toTString(str).c_str() );
}}

static void termHandler (void) {
if (win) MessageBox(win, msg("A fatal error occurred and the application must be terminated. Please excuse for the inconvenience.").c_str(), msg("Fatal error").c_str(), MB_OK | MB_ICONERROR);
exit(3);
}

static void sigsegv (int exCode) { 
std::terminate(); 
}

extern "C" int WINAPI WinMain (HINSTANCE hThisInstance,                      HINSTANCE hPrevInstance,                      LPSTR lpszArgument,                      int nWindowStile) {
long long time = GetTickCount();
sp.hinstance = hinstance = hThisInstance;
if (!(isDebug = IsDebuggerPresent())) {
set_terminate(termHandler);
set_unexpected(termHandler);
CSignal(sigsegv);
}
SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
SixpadDLLInit(&sp);

{//Getting paths and such global parameters
TCHAR fn[512]={0};
GetModuleFileName(NULL, fn, 511);
appPath = fn;
TCHAR* fnBs = tstrrchr(fn, (TCHAR)'\\');
TCHAR* fnDot = tstrrchr(fn, (TCHAR)'.');
*fnBs=0;
*fnDot=0;
tsnprintf(CLASSNAME, sizeof(CLASSNAME)/sizeof(TCHAR), TEXT("QC6Pad01_%s"), fnBs+1);
appLocale = toTString(setlocale(LC_ALL,""));
int i = appLocale.find('_');
if (i>=0&&i<appLocale.size()) appLocale = appLocale.substr(0,i);
to_lower(appLocale);
appName = fnBs+1;
appDir = fn;
configFileName = appDir + TEXT("\\") + appName + TEXT(".ini");
config.load(configFileName);
if (!msgs.load(appDir + TEXT("\\") + appName + TEXT("-") + appLocale + TEXT(".lng") )) msgs.load(appDir + TEXT("\\") + appName + TEXT("-english.lng") );
}

{//Loading command line parameters
int argc=0;
wchar_t** args = CommandLineToArgvW(GetCommandLineW(), &argc);
if (args) {
for (int i=0; i<argc; i++) argv.push_back(toTString(args[i]));
LocalFree(args);
}}
for (int i=1; i<argv.size(); i++) {
tstring arg = argv[i];
if (arg.size()<=0) continue;
else if (arg[0]=='-' || arg[0]=='/') { // options
arg[0]='/';
if (arg==TEXT("/headless")) headless=true;
continue; 
} 
if (OpenFile2(arg, OF_REUSEOPENEDTABS)) return 0;
}
bool writeToStdout = (1&GetFileType(GetStdHandle(STD_OUTPUT_HANDLE))), 
readFromStdin = (1&GetFileType(GetStdHandle(STD_INPUT_HANDLE)));
string dataFromStdin;
if (readFromStdin) {
setmode(fileno(stdin),O_BINARY);
File f(TEXT("&in:"));
dataFromStdin = f.readFully();
}
firstInstance = !FindWindow(CLASSNAME,NULL);

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
if (!RegisterClassEx(&wincl)) return 1;
INITCOMMONCONTROLSEX ccex = { sizeof(INITCOMMONCONTROLSEX), ICC_BAR_CLASSES | ICC_PROGRESS_CLASS | ICC_TAB_CLASSES | ICC_COOL_CLASSES | ICC_TREEVIEW_CLASSES | ICC_LISTVIEW_CLASSES   };
if (!InitCommonControlsEx(&ccex)) return 1;
}

{//Create window block
sp.win = win = CreateWindowEx(
WS_EX_CONTROLPARENT | WS_EX_ACCEPTFILES,
CLASSNAME, GetDefaultWindowTitle().c_str(), 
WS_VISIBLE | WS_OVERLAPPEDWINDOW,
CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
HWND_DESKTOP, NULL, hinstance, NULL);
RECT r; GetClientRect(win, &r);
sp.tabctl = tabctl = CreateWindowEx(
0, WC_TABCONTROL, NULL, 
WS_VISIBLE | WS_CHILD | TCS_SINGLELINE | TCS_FOCUSNEVER | (config.get("tabsAtBottom", false)? TCS_BOTTOM:0),
5, 5, r.right -10, r.bottom -49, 
win, (HMENU)IDC_TABCTL, hinstance, NULL);
sp.status = status = CreateWindowEx(
0, L"STATIC", TEXT("Status"), 
WS_VISIBLE | WS_CHILD | WS_BORDER | SS_NOPREFIX | SS_LEFT, 
5, r.bottom -32, r.right -10, 27, 
win, (HMENU)IDC_STATUSBAR, hinstance, NULL);
hAccel = LoadAccelerators(hinstance, TEXT("accel"));
hGlobAccel = LoadAccelerators(hinstance, TEXT("globaccel"));
menu = GetMenu(win);
menuFormat = GetSubMenu(menu,2);
menuEncoding = GetSubMenu(menuFormat,0);
menuLineEnding = GetSubMenu(menuFormat,1);
menuIndentation = GetSubMenu(menuFormat,2);
menuRecentFiles = GetSubMenu(GetSubMenu(menu, 0), 6);
for (int i=0; i<encodings.size(); i++) InsertMenu(menuEncoding, i, MF_STRING | MF_BYPOSITION, IDM_ENCODING+i, (TEXT("Encoding")+toTString(encodings[i])).c_str() );
for (int i=1; i<=8; i++) InsertMenu(menuIndentation, i, MF_STRING | MF_BYPOSITION, IDM_INDENTATION_SPACES -1 +i, tsnprintf(32, msg("%d spaces"), i).c_str() );
I18NMenus(menu);
SetMenuNamesFromResource(menu);
}

{//TExt font
LOGFONT lf = { -13, 0, 0, 0, 400, 0, 0, 0, 0, 3, 2, 1, 0x31, TEXT("Lucida Console") };
File fdFont(appDir + TEXT("\\") + appName + TEXT(".fnt"));
if (fdFont)  fdFont.read(&lf, sizeof(lf));
sp.font = gfont = CreateFontIndirect(&lf);
}

{//Recent files
for (int i=0; config.contains("recentFile"+toString(i)); i++) {
recentFiles.push_back(config.get<tstring>("recentFile"+toString(i), TEXT("") ));
}
UpdateRecentFilesMenu();
}//END Recentfiles

consoleInputEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
InitializeCriticalSection(&csConsoleInput);
Thread::start(PyStart);

for (int i=1; i<argv.size(); i++) {
const tstring& arg = argv[i];
if (arg.size()<=0) continue;
else if (arg[0]=='-' || arg[0]=='/') { continue; } // options
OpenFile(arg);
}

if (firstInstance) {//Reload last opened files
int mode = config.get("reloadLastFilesMode",0);
if (mode==1 && pages.size()>0) mode=0;
for (int i=0; config.contains("lastFile" + toString(i)); i++) {
tstring fileName = config.get<tstring>("lastFile" + toString(i), TEXT(""));
int pos = config.get("lastFilePos" + toString(i), 0);
config.erase("lastFile" + toString(i));
config.erase("lastFilePos" + toString(i));
if (mode<=0) continue;
shared_ptr<Page> p = OpenFile(fileName);
p->SetCurrentPosition(pos);
}}
if (writeToStdout || readFromStdin) {
shared_ptr<Page> p = PageAddEmpty(false);
if (writeToStdout) p->flags |= PF_WRITETOSTDOUT | PF_NOSAVE | PF_NORELOAD;
p->LoadData(dataFromStdin);
p->SetName(msg("Standard input/output"));
}
if (pages.size()<=0) PageAddEmpty(false);

time = GetTickCount() -time;
if (!RELEASE) fprintf(stderr, "Init time = %d ms\r\n", time);

if (headless) PostQuitMessage(0);
else ShowWindow(win, nWindowStile);
DragAcceptFiles(win, true);
PageActivated(pages[0]);

MSG msg;
while (GetMessage(&msg,NULL,0,0)) {
if (TranslateAccelerator(win, hGlobAccel, &msg)) continue;
for (HWND hWin: modlessWindows) if (IsDialogMessage(hWin, &msg)) goto endmsgloop; // continue x2
if (TranslateAccelerator(win, hAccel, &msg)) continue;
TranslateMessage(&msg);	
DispatchMessage(&msg);
endmsgloop: ;
}

{int i=0; for(const tstring& file: recentFiles) {
config.set("recentFile" + toString(i++), file);
}}
if (config.size()>0) config.save(appDir + TEXT("\\") + appName + TEXT(".ini") );
return msg.wParam;
}

void GoToNextModlessWindow (int dist) {
int n = modlessWindows.size();
if (n<=0) {
SetForegroundWindow(win);
return;
}
HWND hWin = GetForegroundWindow();
int curPos = (win==hWin? n : (find(modlessWindows.begin(), modlessWindows.end(), hWin) -modlessWindows.begin() ));
int nextPos = (curPos + dist + n +1)%(n+1);
SetForegroundWindow(nextPos==n? win : modlessWindows[nextPos]);
}

static void AboutDlg () {
string pyver = Py_GetVersion();
pyver = pyver.substr(0, pyver.find(' '));
MessageBox(win, tsnprintf(512,
TEXT("6pad++ %s\r\nCopyright \xA9 2015, Quentin Cosendey\r\nhttp://quentinc.net/\r\n\r\n") 
+ msg("This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License.")
+ TEXT("\r\n\r\n") + msg("This program embeds python %s from") + TEXT(" Guido van Rossum (http://www.python.org/)"),
TEXT(SIXPAD_VERSION), toTString(pyver).c_str()
).c_str(), msg("About").c_str(), MB_OK | MB_ICONINFORMATION);
}

static void SelFontDlg () {
LOGFONT lf = { 0, 0, 0, 0, 400, false, false, false, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, TEXT("Courier")};
{//Loading font
File fdFont(appDir + TEXT("\\") + appName + TEXT(".fnt"));
if (fdFont) {
LOGFONT lf = {0};
fdFont.read(&lf, sizeof(lf));
}}
if (!FontDialog(win,lf)) return;
char* clf = (char*)&lf;
File fdFont(appDir + TEXT("\\") + appName + TEXT(".fnt"), true);
fdFont.write(&lf, sizeof(lf));
gfont = CreateFontIndirect(&lf);
for (auto p: pages) p->SetFont(gfont);
}

static LRESULT CALLBACK QuickSearchSubclassProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR subclassId, DWORD_PTR unused) {
switch(msg){
case WM_CHAR:
switch(LOWORD(wp)){
case VK_TAB: {
int ss, se; tstring text = GetWindowText(hwnd);
SendMessage(hwnd, EM_GETSEL, &ss, &se);
var re = onquickReachAutocomplete(text, min(ss,se));
if (re.getType()==T_STR) {
tstring newText = re.toTString();
SetWindowText(hwnd, newText);
ss = first_mismatch(text, newText);
se = newText.size();
SendMessage(hwnd, EM_SETSEL, ss, se);
}
else  MessageBeep(MB_OK);
}return true;
case VK_RETURN:
if (onquickReach( GetWindowText(hwnd) )) MessageBeep(MB_OK);
case VK_ESCAPE: 
if (curPage) curPage->FocusZone();
return true;
}break;
case WM_KILLFOCUS:
ShowWindow(hwnd, SW_HIDE);
break;
}
return DefSubclassProc(hwnd, msg, wp, lp);
}

static bool ShowQuickSearch () {
static HWND hLbl=0, hEdit = 0;
if (!hLbl) hLbl = CreateWindowEx(0, TEXT("STATIC"), (msg("Quick reach") + TEXT(":")).c_str(),
WS_CHILD,
1, 1, 1, 1,
sp.win, (HMENU)(0), sp.hinstance, NULL);
if (!hEdit) {
hEdit = CreateWindowEx(0, TEXT("EDIT"), TEXT(""),
WS_CHILD | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL,
1, 1, 1, 1, 
sp.win, (HMENU)(4), sp.hinstance, NULL);
SetWindowSubclass(hEdit, (SUBCLASSPROC)QuickSearchSubclassProc, 0, 0);
}
RECT r; GetClientRect(win,&r);
MoveWindow(hEdit, r.right -125, r.bottom -32, 120, 30, TRUE);
SetWindowText(hEdit, NULL);
ShowWindow(hEdit, SW_SHOW);
SetFocus(hEdit);
}

bool ActionCommand (HWND hwnd, int cmd) {
switch(cmd){
case IDM_OPEN: 
if (2==config.get("instanceMode",0)) OpenFileDialog(OF_NEWINSTANCE); 
else OpenFileDialog(OF_REUSEOPENEDTABS);
return true;
case IDM_OPEN_NI: 
if (1==config.get("instanceMode",0)) OpenFileDialog(OF_REUSEOPENEDTABS);
else OpenFileDialog(OF_NEWINSTANCE); 
return true;
case IDM_REOPEN: PageReopen(curPage); return true;
case IDM_SAVE: if (curPage) curPage->Save(); return true;
case IDM_SAVE_AS: if (curPage) curPage->Save(true);  return true;
case IDM_NEW: PageAddEmpty(true); return true;
case IDM_CLOSE: if (curPage) curPage->Close(); return true;
case IDM_SELECTALL: if (curPage) curPage->SelectAll(); return true;
case IDM_COPY: if (curPage) curPage->Copy();  return true;
case IDM_CUT: if (curPage) curPage->Cut();  return true;
case IDM_PASTE: DoPaste(); return true;
case IDM_UNDO: if (curPage) curPage->Undo(); break;
case IDM_REDO: if (curPage) curPage->Redo(); break;
case IDM_MARKSEL: if (curPage) curPage->MarkCurrentPosition(); break;
case IDM_SELTOMARK: if (curPage) curPage->SelectToMark(); break;
case IDM_GOTOMARK: if (curPage) curPage->GoToMark(); break;
case IDM_GOTOLINE: if (curPage) curPage->GoToDialog(); return true;
case IDM_FIND: if (curPage) curPage->FindDialog(); return true;
case IDM_REPLACE: if (curPage) curPage->FindReplaceDialog(); return true;
case IDM_FINDNEXT: if (curPage) curPage->FindNext(); return true;
case IDM_FINDPREV: if (curPage) curPage->FindPrev(); return true;
case IDM_LE_DOS: case IDM_LE_UNIX: case IDM_LE_MAC: if (curPage) curPage->SetLineEnding(cmd-IDM_LE_DOS); return true;
case IDM_AUTOLINEBREAK: if (curPage) curPage->SetAutoLineBreak(!(curPage->flags&PF_AUTOLINEBREAK)); return true;
case IDM_QUICKSEARCH: ShowQuickSearch(); return true;
case IDM_OPEN_CONSOLE: OpenConsoleWindow(); return true;
case IDM_OTHER_ENCODINGS: EncodingShowAll(); return true;
case IDM_NEXTPAGE: PageGoToNext(1); break;
case IDM_PREVPAGE: PageGoToNext(-1); break;
case IDM_NEXT_MODLESS: GoToNextModlessWindow(1); return true;
case IDM_PREV_MODLESS: GoToNextModlessWindow(-1); return true;
case IDM_ABOUT: AboutDlg(); break;
case IDM_SELECTFONT: SelFontDlg(); break;
case IDM_EXIT: SendMessage(win, WM_CLOSE, 0, 0); return true;
case IDM_CRASH:
break;
//{ int* p=0; *p=0; } return true; // Force crash
}
if (cmd>=IDM_ENCODING && cmd<IDM_ENCODING+encodings.size() && curPage) {
curPage->SetEncoding(encodings[cmd-IDM_ENCODING]);
return true;
}
if (cmd>=IDM_INDENTATION_TABS && cmd<=IDM_INDENTATION_SPACES+8 && curPage) {
curPage->SetIndentationMode(cmd-IDM_INDENTATION_TABS);
return true;
}
if (cmd>=IDM_RECENT_FILE && cmd<IDM_RECENT_FILE+100 && cmd<IDM_RECENT_FILE+recentFiles.size()) {
const tstring& file = *next(recentFiles.begin(), cmd - IDM_RECENT_FILE);
auto it = find_if(pages.begin(), pages.end(), [&](shared_ptr<Page>& p){ return p->file==file; });
if (it!=pages.end())PageActivate(it - pages.begin() );
else OpenFile(file);
return true;
}
if (cmd>=IDM_USER_COMMAND && cmd<0xF000) {
auto it = userCommands.find(cmd);
if (it!=userCommands.end()) (it->second)();
return true;
}
return false;
}

static LRESULT CALLBACK ConsoleDlgInputSubclassProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR subclassId, DWORD_PTR unused) {
static vector<tstring> history;
static int historyPtr=0;
switch(msg){
case WM_KEYDOWN:
if (LOWORD(wp)==VK_DOWN) {
historyPtr = min(historyPtr+1, (int)history.size());
if (historyPtr<history.size()) SetWindowText(hwnd, history[historyPtr]);
else SetWindowText(hwnd, TEXT(""));
SendMessage(hwnd, EM_SETSEL, 0, -1);
}
else if (LOWORD(wp)==VK_UP) {
historyPtr = max(min(historyPtr -1, (int)history.size()), -1);
if (historyPtr>=0 && historyPtr<history.size()) SetWindowText(hwnd, history[historyPtr]);
else SetWindowText(hwnd, TEXT(""));
SendMessage(hwnd, EM_SETSEL, 0, -1);
}
break;
case WM_USER: {
tstring str = GetWindowText(hwnd);
auto it = std::find(history.begin(), history.end(), str);
if (it!=history.end()) history.erase(it);
history.push_back(str);
if (history.size()>50) history.erase(history.begin());
historyPtr = history.size();
return true;
}}
return DefSubclassProc(hwnd, msg, wp, lp);
}

INT_PTR consoleDlgProc (HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp) {
switch (umsg) {
case WM_INITDIALOG : {
HWND hEdit = GetDlgItem(hwnd,1001);
SetWindowText(hwnd, msg("Python console"));
SetDlgItemText(hwnd, 1001, TEXT(">>> "));
SetDlgItemText(hwnd, 2000, msg("&Input")+TEXT(":"));
SetDlgItemText(hwnd, 2001, msg("&Output")+TEXT(":"));
SetDlgItemText(hwnd, 1003, msg("E&val"));
SetDlgItemText(hwnd, 1004, msg("Clea&r"));
SetDlgItemText(hwnd, IDCANCEL, msg("&Close"));
SendMessage(hEdit, EM_SETLIMITTEXT, 67108864, 0);
SendMessage(hEdit, WM_SETFONT, gfont, TRUE);
//int xx = curPage->tabSpaces==0? 16 : ABS(curPage->tabSpaces)*4;
//SendMessage(hEdit, EM_SETTABSTOPS, 1, &xx);
SetDlgItemFocus(hwnd, 1002);
SetWindowSubclass(GetDlgItem(hwnd,1002), (SUBCLASSPROC)ConsoleDlgInputSubclassProc, 0, 0);
modlessWindows.push_back(hwnd);
}return false;
case WM_COMMAND :
switch(LOWORD(wp)) {
case 1003 : {
tstring line = GetDlgItemText(hwnd, 1002);
SendDlgItemMessage(hwnd, 1002, WM_USER, 0, 0);
SetDlgItemText(hwnd, 1002, TEXT(""));
SetDlgItemFocus(hwnd, 1002);
ConsolePrint(line + TEXT("\r\n"), false);
SCOPE_LOCK(csConsoleInput);
consoleInput.push_back(line);
if (consoleInput.size()==1) SetEvent(consoleInputEvent);
}break;
case 1004 :
SetDlgItemText(hwnd, 1001, TEXT(">>>"));
break;
case IDCANCEL : {
consoleWin = NULL;
modlessWindows.erase(find(modlessWindows.begin(), modlessWindows.end(), hwnd));
DestroyWindow(hwnd);
GoToNextModlessWindow(0);
}break;
case 999: {
tstring& str = *(tstring*)(lp);
HWND hEdit = GetDlgItem(hwnd, 1001);
int n = GetWindowTextLength(hEdit), ss, se;
preg_replace(str, TEXT("(?:\r\n|\n|\r)"), TEXT("\r\n"));
SendMessage(hEdit, EM_GETSEL, &ss, &se);
SendMessage(hEdit, EM_SETSEL, n, n);
SendMessage(hEdit, EM_REPLACESEL, 0, str.c_str());
SendMessage(hEdit, EM_SETSEL, ss, se);
SendMessage(hEdit, EM_SCROLLCARET, 0, 0);
}break; }
if (HIWORD(wp)==EN_SETFOCUS && LOWORD(wp)==1002) SendDlgItemMessage(hwnd,1002,EM_SETSEL,0,-1);
break;
case WM_ACTIVATE:
//SetDlgItemFocus(hwnd, 1002);
break;
}
return FALSE;
}

void OpenConsoleWindow () {
if (!consoleWin) consoleWin = CreateDialogParam(hinstance, IDD_CONSOLE, win, consoleDlgProc, NULL);
if (consoleWin) {
ShowWindow(consoleWin, SW_SHOW);
SetForegroundWindow(consoleWin);
}}

LRESULT WINAPI AppWinProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
switch(msg){
case WM_COMMAND:
if (ActionCommand(hwnd, LOWORD(wp))) return TRUE;
break;
case WM_SYSCOMMAND :
if (LOWORD(wp)==SC_KEYMENU && lp==0) return ShowQuickSearch();
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
Proc* proc = (Proc*)lp;
(*proc)();
if (wp) delete proc;
return true;
}break;
case WM_TIMER: {
int id = LOWORD(wp);
auto it = timers.find(id);
if (it!=timers.end()) {
it->second();
if (id&0x8000) ClearTimeout(id);
}}break;
case WM_COPYDATA: {
COPYDATASTRUCT& cp = *(COPYDATASTRUCT*)(lp);
if (cp.dwData==76) return OpenFile3((LPCTSTR)(cp.lpData));
}break;
case WM_DROPFILES:
DoDropFiles((HDROP)wp);
DragFinish((HDROP)wp);
return true;
case WM_SETFOCUS :
AppWindowGainedFocus();
break;
case WM_ACTIVATE :
if(LOWORD(wp)) AppWindowActivated();
else AppWindowDeactivated();
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

SixpadData sp = {
&msg, &RegisterPageFactory,
&AddUserCommand, &RemoveUserCommand,
&AddAccelerator, &RemoveAccelerator,
&KeyCodeToName,  &KeyNameToCode,
&SetTimeout, &ClearTimeout,
&msgs, &config, 
CLASSNAME, SIXPAD_VERSION_ID, 0
};
