#include "global.h"
#include "page.h"
#include "file.h"
#include "inifile.h"
#include<boost/regex.hpp>

#define FF_CASE 1
#define FF_REGEX 2
#define FF_UPWARDS 4

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
static list<FindData> finds;

extern HINSTANCE hinstance;
extern HWND win, status;
extern IniFile config, msgs;
extern tstring configFileName;

tstring msg (const char* name) ;
void SetClipboardText (const tstring&);
tstring GetClipboardText (void);
void PrepareSmartPaste (tstring& text, const tstring& indent);
bool PageGoToNext (int);
void PageSetName (shared_ptr<Page> p, const tstring& name);

typedef LRESULT(*PositionFinder)(HWND,int);

void Page::SetName (const tstring& name) { PageSetName(shared_from_this(),name); }

void TextPage::SetCurrentPosition (int pos) {
if (!zone) return;
SendMessage(zone, EM_SETSEL, pos, pos);
if (IsWindowVisible(zone)) SendMessage(zone, EM_SCROLLCARET, 0, 0);
}

int TextPage::GetCurrentPosition () {
int pos=0;
SendMessage(zone, EM_GETSEL, 0, &pos);
return pos;
}

bool TextPage::IsEmpty ()  {
return file.size()<=0 && GetWindowTextLength(zone)<=0;
}

bool TextPage::IsModified () {
return !zone || !!SendMessage(zone, EM_GETMODIFY, 0, 0);
}

void TextPage::SetModified (bool b) {
SendMessage(zone, EM_SETMODIFY, b, 0);
}

void TextPage::Copy ()  { SendMessage(zone, WM_COPY, 0, 0); }
void TextPage::Cut ()  { SendMessage(zone, WM_CUT, 0, 0); }
void TextPage::Paste ()  { SendMessage(zone, WM_PASTE, 0, 0); }

void TextPage::SelectAll () {
SendMessage(zone, EM_SETSEL, 0, -1);
}

void TextPage::GetSelection (int& start, int& end) {
SendMessage(zone, EM_GETSEL, &start, &end);
}

tstring TextPage::GetSelectedText ()  {
return EditGetSelectedText(zone);
}

int TextPage::GetTextLength () {
return GetWindowTextLength(zone);
}

tstring TextPage::GetText ()  {
return GetWindowText(zone);
}

tstring TextPage::GetLine (int line) {
return EditGetLine(zone, line);
}

int TextPage::GetLineCount ()  {
return SendMessage(zone, EM_GETLINECOUNT, 0, 0);
}

int TextPage::GetLineLength (int line) {
return SendMessage(zone, EM_LINELENGTH, GetLineStartIndex(line), 0);
}

int TextPage::GetLineStartIndex (int line) {
return SendMessage(zone, EM_LINEINDEX, line, 0);
}

int TextPage::GetLineOfPos (int pos) {
return SendMessage(zone, EM_LINEFROMCHAR, pos, 0);
}

void TextPage::SetSelection (int start, int end) {
SendMessage(zone, EM_SETSEL, start, end);
if (IsWindowVisible(zone)) SendMessage(zone, EM_SCROLLCARET, 0, 0);
}

void TextPage::SetText (const tstring& str) {
int start, end;
SendMessage(zone, EM_GETSEL, &start, &end);
SetWindowText(zone, str);
SendMessage(zone, EM_SETSEL, start, end);
if (IsWindowVisible(zone)) SendMessage(zone, EM_SCROLLCARET, 0, 0);
}

void TextPage::ReplaceTextRange (int start, int end, const tstring& str) {
int oldStart, oldEnd;
SendMessage(zone, EM_GETSEL, &oldStart, &oldEnd);
if (start>=0||end>=0) SendMessage(zone, EM_SETSEL, start, end);
SendMessage(zone, EM_REPLACESEL, 0, str.c_str());
SendMessage(zone, EM_SETSEL, oldStart, oldEnd);
if (IsWindowVisible(zone)) SendMessage(zone, EM_SCROLLCARET, 0, 0);
}

void TextPage::SetSelectedText (const tstring& str) {
int start;
SendMessage(zone, EM_GETSEL, &start, 0);
SendMessage(zone, EM_REPLACESEL, 0, str.c_str());
SendMessage(zone, EM_SETSEL, start, start+str.size());
if (IsWindowVisible(zone)) SendMessage(zone, EM_SCROLLCARET, 0, 0);
}

PyObject* CreatePyEditorTabObject (shared_ptr<Page>);
PyObject* TextPage::GetPyData () {
if (!pyData) pyData = CreatePyEditorTabObject(shared_from_this());
return *pyData;
}

tregex FindData::newRegex (bool findOnly) {
using namespace boost;
int options =
(flags&FF_REGEX? regex_constants::perl | regex_constants::mod_s | regex_constants::collate | (findOnly?regex_constants::nosubs:0)  : regex_constants::literal)
| (flags&FF_CASE? 0 : regex_constants::icase);
return tregex(findText, options);
}

void TextPage::FindNext () {
using namespace boost;
HWND& edit = zone;
if (finds.size()<=0) { FindDialog(); return; }
FindData& fd = finds.front();
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

void TextPage::FindPrev () {
using namespace boost;
HWND& edit = zone;
if (finds.size()<=0) { FindDialog(); return; }
FindData& fd = finds.front();
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

void FindNew(TextPage& tp, const tstring& searchText, bool scase, bool regex, bool up) {
FindData fd(searchText, TEXT(""), (scase?FF_CASE:0) | (regex?FF_REGEX:0) | (up?FF_UPWARDS:0) );
auto it = find(finds.begin(), finds.end(), fd);
if (it!=finds.end()) finds.erase(it);
finds.push_front(fd);
if (up) tp.FindPrev();
else tp.FindNext();
}

void TextPage::FindReplace (const tstring& searchText, const tstring& replaceText, bool scase, bool regex) {
using namespace boost;
FindData fd(searchText, replaceText, (scase?FF_CASE:0) | (regex?FF_REGEX:0) );
auto it = find(finds.begin(), finds.end(), fd);
if (it!=finds.end()) finds.erase(it);
finds.push_front(fd);
int start, end;
HWND& edit = zone;
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

bool TextPage::SaveText (const tstring& newFile) {
if (flags&PF_NOSAVE) return false;
if ((flags&PF_READONLY) && newFile.size()<=0) return false;
if (newFile.size()>0) { file = newFile; flags&=~PF_READONLY; }
if (file.size()<=0) return false;
tstring str = GetText();
var re = dispatchEvent("save", var(), str);
if (re.getType()==T_STR) str = re.toTString();
if (lineEnding==LE_UNIX) str = str_replace(str, TEXT("\r\n"), TEXT("\n"));
else if (lineEnding==LE_MAC) str = str_replace(str, TEXT("\r\n"), TEXT("\r"));
string cstr = ConvertToEncoding(str, encoding);
File fd(file, true);
if (fd) fd.writeFully(cstr.data(), cstr.size());
if (file==configFileName) config.load(configFileName);
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
normalizeLineEndings(text);
for (int i=0, n=text.size(); i<n; i++) if (text[i]==0) text[i]=127;
var re = dispatchEvent("load", var(), text);
if (re.getType()==T_STR) text = re.toTString();
}}
SetText(text);
return text;
}

static void StatusBarUpdate (HWND hEdit, HWND status) {
int spos=0, epos=0;
SendMessage(hEdit, EM_GETSEL, &spos, &epos);
int sline = SendMessage(hEdit, EM_LINEFROMCHAR, spos, 0);
int scolumn = spos - SendMessage(hEdit, EM_LINEINDEX, sline, 0);
if (spos!=epos) {
int eline = SendMessage(hEdit, EM_LINEFROMCHAR, epos, 0);
int ecolumn = epos - SendMessage(hEdit, EM_LINEINDEX, eline, 0);
SetWindowText(status, tsnprintf(512, msg("Li %d, Col %d to Li %d, Col %d"), 1+sline, 1+scolumn, 1+eline, 1+ecolumn));
} else {
int nlines = SendMessage(hEdit, EM_GETLINECOUNT, 0, 0);
int max = GetWindowTextLength(hEdit);
int prc = max? 100 * spos / max :0;
SetWindowText(status, tsnprintf(512, msg("Li %d, Col %d.\t%d%%, %d lines"), 1+sline, 1+scolumn, prc, nlines));
}}

void TextPage::UpdateStatusBar (HWND hStatus) {
if (zone) StatusBarUpdate(zone, hStatus);
}

static INT_PTR CALLBACK GoToLineDlgProc (HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp) {
static TextPage* page = 0;
switch (umsg) {
case WM_INITDIALOG : {
page = (TextPage*)(lp);
HWND edit = page->zone;
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
HWND edit = page->zone;
tstring tmp = GetDlgItemText(hwnd, 1002);
int num = toInt(tmp);
if (tmp[0]=='+' || tmp[0]=='-') num += SendMessage(edit, EM_LINEFROMCHAR, -1, 0);
else --num;
int max = SendMessage(edit, EM_GETLINECOUNT, 0, 0);
if (num<0) num=0;
else if (num>=max) num=max-1;
int pos = SendMessage(edit, EM_LINEINDEX, num, 0);
page->SetCurrentPosition(pos);
}
case IDCANCEL : EndDialog(hwnd, wp); return TRUE;
}}
return FALSE;
}

void TextPage::GoToDialog () {
DialogBoxParam(IDD_GOTOLINE, win, GoToLineDlgProc, this);
}

static INT_PTR CALLBACK FindReplaceDlgProc (HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp) {
static TextPage* page = 0;
switch (umsg) {
case WM_INITDIALOG : {
bool findOnly = (lp&1)==0;
page = (TextPage*)(lp&0xFFFFFFFCL);
FindData fd = finds.size()>0? finds.front() : FindData(TEXT(""), TEXT(""), 0);
SetWindowText(hwnd, msg(findOnly? "Find" : "Search and replace"));
SetDlgItemText(hwnd, IDOK, msg(!findOnly? "Replace &all" : "&OK") );
SetDlgItemText(hwnd, IDCANCEL, msg("Ca&ncel"));
SetDlgItemText(hwnd, 2000, msg("&Search for") + TEXT(":") );
SetDlgItemText(hwnd, 2001, msg("&Replace with") + TEXT(":") );
SetDlgItemText(hwnd, 2002, msg("Direction")+TEXT(":") );
SetDlgItemText(hwnd, 1003, msg("&Case sensitive"));
SetDlgItemText(hwnd, 1004, msg("Regular e&xpression"));
SetDlgItemText(hwnd, 1005, msg("&Up"));
SetDlgItemText(hwnd, 1006, msg("&Down"));
EnableDlgItem(hwnd, 1002, !findOnly);
EnableDlgItem(hwnd, 1005, findOnly);
EnableDlgItem(hwnd, 1006, findOnly);
SetDlgItemText(hwnd, 1001, fd.findText.c_str() );
SetDlgItemText(hwnd, 1002, fd.replaceText.c_str() );
SendMessage(GetDlgItem(hwnd, 1003), BM_SETCHECK, (fd.flags&FF_CASE)?BST_CHECKED:BST_UNCHECKED, 0);
SendMessage(GetDlgItem(hwnd, 1004), BM_SETCHECK, (fd.flags&FF_REGEX)?BST_CHECKED:BST_UNCHECKED, 0);
SendMessage(GetDlgItem(hwnd, (fd.flags&FF_UPWARDS)?1005:1006), BM_SETCHECK, BST_CHECKED, 0);
HWND hFindCb = GetDlgItem(hwnd,1001), hReplCb = GetDlgItem(hwnd,1002);
for (FindData& f: finds) {
SendMessage(hFindCb, CB_ADDSTRING, 0, f.findText.c_str() );
SendMessage(hReplCb, CB_ADDSTRING, 0, f.replaceText.c_str() );
}
return TRUE;
}//WM_INITDIALOG
case WM_COMMAND :
switch (LOWORD(wp)) {
case IDOK : {
BOOL sr = IsDlgItemEnabled(hwnd, 1002);
BOOL searchCase = IsDlgButtonChecked(hwnd, 1003);
BOOL searchRegex = IsDlgButtonChecked(hwnd, 1004);
BOOL searchUp = IsDlgButtonChecked(hwnd, 1005);
tstring searchText = GetDlgItemText(hwnd, 1001);
tstring replaceText = GetDlgItemText(hwnd, 1002);
if (sr) page->FindReplace(searchText, replaceText, searchCase, searchRegex);
else FindNew(*page, searchText, searchCase, searchRegex, searchUp);
}
case IDCANCEL : EndDialog(hwnd, wp); return TRUE;
}}
return FALSE;
}

static inline void FindReplaceDlg2 (TextPage& tp, bool replace) {
DWORD val = (DWORD)&tp;
if (replace) val++;
DialogBoxParam(IDD_SEARCHREPLACE, win, FindReplaceDlgProc, val);
}

void TextPage::FindDialog () {
FindReplaceDlg2(*this,false);
}

void TextPage::FindReplaceDialog () {
FindReplaceDlg2(*this,true);
}

static int EZGetNextParagPos (HWND hEdit, int pos) {
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

static int EZGetPrevParagPos (HWND hEdit, int pos) {
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

static int EZGetNextBracketPos (HWND hEdit, int pos) {
int len = GetWindowTextLength(hEdit);
HLOCAL hLoc = (HLOCAL)SendMessage(hEdit, EM_GETHANDLE, 0, 0);
LPCTSTR text = (LPCTSTR)LocalLock(hLoc);
pos++;
while(pos<len && text[pos++]!='}');
LocalUnlock(hLoc);
return pos;
}

static int EZGetPrevBracketPos (HWND hEdit, int pos) {
int len = GetWindowTextLength(hEdit);
HLOCAL hLoc = (HLOCAL)SendMessage(hEdit, EM_GETHANDLE, 0, 0);
LPCTSTR text = (LPCTSTR)LocalLock(hLoc);
while(pos>0 && text[--pos]!='{');
LocalUnlock(hLoc);
return pos;
}

static int EZGetEndIndentedBlockPos (HWND hEdit, int pos) {
int ln = SendMessage(hEdit, EM_LINEFROMCHAR, pos, 0), maxline = SendMessage(hEdit, EM_GETLINECOUNT, 0, 0), maxpos = GetWindowTextLength(hEdit);
tstring indent = EditGetLine(hEdit, ln, pos);
int n = indent.find_first_not_of(TEXT("\t \xA0"));
if (n<0 || n>=indent.size()) n=indent.size();
indent = indent.substr(0,n);
while(++ln<maxline) {
tstring line = EditGetLine(hEdit, ln);
if (!startsWith(line, indent)) break;
}
n = SendMessage(hEdit, EM_LINEINDEX, ln -1, 0);
n +=  SendMessage(hEdit, EM_LINELENGTH, n, 0);
if (pos==n && n<maxpos -2) return EZGetEndIndentedBlockPos(hEdit, pos+2);
return n;
}

static int EZGetStartIndentedBlockPos (HWND hEdit, int pos) {
int ln = SendMessage(hEdit, EM_LINEFROMCHAR, pos, 0);
tstring indent = EditGetLine(hEdit, ln, pos);
int n = indent.find_first_not_of(TEXT("\t \xA0"));
if (n<0 || n>=indent.size()) n=indent.size();
indent = indent.substr(0,n);
while(--ln>=0) {
tstring line = EditGetLine(hEdit, ln);
if (!startsWith(line, indent)) break;
}
n = SendMessage(hEdit, EM_LINEINDEX, ln+1, 0);
if (pos>=n && pos<=n+indent.size() && n>2) return EZGetStartIndentedBlockPos(hEdit, n -2);
return n;
}

static LRESULT __fastcall EZHandleEnter (TextPage* page, HWND hEdit) {
int pos=0, nLine=0, addIndent=0;
SendMessage(hEdit, EM_GETSEL, &pos, 0);
nLine = SendMessage(hEdit, EM_LINEFROMCHAR, pos, 0);
tstring addString, line = EditGetLine(hEdit, nLine, pos);
var re = page->dispatchEvent("enter", var(), line);
switch(re.getType()) {
case T_NULL: break;
case T_BOOL: if (!re) return false; break;
case T_INT: addIndent = re.toInt(); break;
case T_STR: addString = re.toTString(); break;
}
pos = line.find_first_not_of(TEXT("\t \xA0"));
if (pos<0 || pos>=line.size()) pos=line.size();
if (addIndent<0) pos = max(0, pos + addIndent * max(1, page->indentationMode));
line = line.substr(0,pos);
if (addIndent>0) for (int i=0, n=min(addIndent,100); i<n; i++) {
if (page->indentationMode<=0) line += TEXT("\t");
else line += tstring(page->indentationMode, ' ');
}
tstring repl = TEXT("\r\n") + line + addString;
SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)repl.c_str() );
SendMessage(hEdit, EM_SCROLLCARET, 0, 0);
return true;
}

static LRESULT EZHandleHome (HWND hEdit, bool normal) {
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

template<class F> static LRESULT EZHandleMoveDown (HWND hEdit, const F& f, bool moveHome=true) {
int pos=0;
SendMessage(hEdit, EM_GETSEL, 0, &pos);
pos = f(hEdit, pos);
SendMessage(hEdit, EM_SETSEL, pos, pos);
if (moveHome) return EZHandleHome(hEdit, false);
else return true;
}

template <class F> static LRESULT EZHandleMoveUp (HWND hEdit, const F& f) {
int pos=0;
SendMessage(hEdit, EM_GETSEL, 0, &pos);
pos = f(hEdit, pos);
SendMessage(hEdit, EM_SETSEL, pos, pos);
return EZHandleHome(hEdit, false);
}

template <class F> static LRESULT EZHandleSelectDown (HWND hEdit, const F& f) {
int spos=0, pos=0;
SendMessage(hEdit, EM_GETSEL, &spos, &pos);
pos = f(hEdit, pos);
SendMessage(hEdit, EM_SETSEL, spos, pos);
return true;
}

template <class F> static LRESULT EZHandleSelectUp (HWND hEdit, const F& f) {
int spos=0, pos=0;
SendMessage(hEdit, EM_GETSEL, &spos, &pos);
pos = f(hEdit, pos);
SendMessage(hEdit, EM_SETSEL, spos, pos);
return true;
}

static LRESULT EZHandleTab (TextPage* curPage, HWND hEdit) {
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
SendMessage(hEdit, EM_SETSEL, sOffset, sOffset+str.size());
}}
else { // There is no selection
tstring line = EditGetLine(hEdit, sLine, sPos);
int pos = line.find_first_not_of(TEXT("\t \xA0"));
if (pos<0) pos = line.size();
if (sPos > sOffset+pos) { Beep(1000,200); return true; }
if (IsShiftDown()) {
for (int i=0; i<1 || i<curPage->indentationMode; i++) SendMessage(hEdit, WM_CHAR, VK_BACK, 0);
return true;
}
else { // shift not down
if (curPage->indentationMode>0) for (int i=0; i<curPage->indentationMode; i++) SendMessage(hEdit, WM_CHAR, 32, 0);
return curPage->indentationMode>0;
}}}

static LRESULT CALLBACK EditAreaWinProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR subclassId, TextPage* curPage) {
if (msg==WM_CHAR) {
if (!curPage->dispatchEvent<bool, true>("keyPressed", (int)LOWORD(wp) )) return true;
switch(LOWORD(wp)) {
case VK_RETURN: return EZHandleEnter(curPage, hwnd);
case VK_TAB: if (EZHandleTab(curPage, hwnd)) return true; break;
}}
else if (msg==WM_KEYDOWN) {
if (!curPage->dispatchEvent<bool, true>("keyDown", (int)LOWORD(wp) )) return true;
switch(LOWORD(wp)) {
case VK_DOWN:
if (IsCtrlDown()) {
if (IsShiftDown()) return EZHandleSelectDown(hwnd, EZGetNextParagPos);
else return EZHandleMoveDown(hwnd, EZGetNextParagPos);
}
break;
case VK_UP:
if (IsCtrlDown()) {
if (IsShiftDown()) return EZHandleSelectUp(hwnd, EZGetPrevParagPos);
else return EZHandleMoveUp(hwnd, EZGetPrevParagPos);
}
break;
case VK_TAB:
if (IsCtrlDown()) return PageGoToNext(IsShiftDown()? -1 : 1);
break;
case VK_HOME:
if (!IsCtrlDown() && !IsShiftDown()) return EZHandleHome(hwnd, IsAltDown());
break;
}}
else if (msg==WM_KEYUP) {
if (!curPage->dispatchEvent<bool, true>("keyUp", (int)LOWORD(wp) )) return true;
StatusBarUpdate(hwnd, status);
}
else if (msg==WM_SYSKEYDOWN) {
switch(LOWORD(wp)) {
case VK_UP: 
if (IsShiftDown()) return EZHandleSelectUp(hwnd, EZGetPrevBracketPos);
else return EZHandleMoveUp(hwnd, EZGetPrevBracketPos);
break;
case VK_DOWN: 
if (IsShiftDown()) return EZHandleSelectDown(hwnd, EZGetNextBracketPos);
else return EZHandleMoveDown(hwnd, EZGetNextBracketPos, false);
break;
case VK_LEFT:  
if (IsShiftDown()) return EZHandleSelectUp(hwnd, EZGetStartIndentedBlockPos);
else return EZHandleMoveUp(hwnd, EZGetStartIndentedBlockPos);
break;
case VK_RIGHT: 
if (IsShiftDown()) return EZHandleSelectDown(hwnd, EZGetEndIndentedBlockPos);
else return EZHandleMoveDown(hwnd, EZGetEndIndentedBlockPos, false);
break;
}}
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

void TextPage::CreateZone (HWND parent) {
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
WS_CHILD | WS_TABSTOP | WS_VSCROLL | WS_BORDER | ES_MULTILINE | ES_NOHIDESEL | ES_AUTOVSCROLL | ((flags&PF_AUTOLINEBREAK)? 0:ES_AUTOHSCROLL|WS_HSCROLL),
10, 10, 400, 400,
parent, (HMENU)(IDC_EDITAREA + count++), hinstance, NULL);
SendMessage(hEdit, EM_SETLIMITTEXT, 1073741823, 0);
//SendMessage(hEdit, WM_SETFONT, font, TRUE);
{ int x=16; SendMessage(hEdit, EM_SETTABSTOPS, 1, &x); }
SetWindowText(hEdit, text.c_str());
SendMessage(hEdit, EM_SETSEL, ss, se);
SendMessage(hEdit, EM_SCROLLCARET, 0, 0);
SetWindowSubclass(hEdit, (SUBCLASSPROC)EditAreaWinProc, 0, (DWORD_PTR)this);
zone=hEdit;
}

void TextPage::HideZone () {
ShowWindow(zone, SW_HIDE);
EnableWindow(zone, FALSE);
}

void TextPage::ShowZone (const RECT& r) {
EnableWindow(zone, TRUE);
SetWindowPos(zone, NULL,
r.left+3, r.top+3, r.right - r.left -6, r.bottom - r.top -6,
SWP_NOZORDER | SWP_SHOWWINDOW);
SendMessage(zone, EM_SCROLLCARET, 0, 0);
}

void TextPage::FocusZone () {
SetFocus(zone);
}

void TextPage::ResizeZone (const RECT& r) {
MoveWindow(zone, r.left+3, r.top+3, r.right-r.left -6, r.bottom-r.top -6, TRUE);
}



