#include "global.h"
#include "sixpad.h"
#include<boost/shared_array.hpp>
using namespace std;
using boost::shared_array;

inline tstring msg (const char* name) { 
return sp.msg(name); 
}

static void TranslateNulls (LPTSTR path) {
for (TCHAR* p=path; *p||p[1]; p++) if(!*p&&p[1]) *p='|';
}

static tstring& UntranslateNulls (tstring& s) {
TCHAR *orig = (TCHAR*)s.data();
int len = s.size();
for (TCHAR* p = orig; p-orig<=len; p++) if (*p=='|') *p=0;
s.append(4,0);
return s;
}

tstring export FileDialog (HWND parent, int flags, const tstring& file, const tstring& title, tstring filters, int* nFilterSelected) {
int pathlen = ((flags&3)==3? 32768: 512);
shared_array<TCHAR> path( new TCHAR[pathlen] );
OPENFILENAME ofn;
ZeroMemory(&path[0], pathlen*sizeof(TCHAR));
ZeroMemory(&ofn, sizeof(OPENFILENAME));
if (file.size()>0) memcpy(&path[0], file.c_str(), sizeof(TCHAR) * (file.size()+1));
                ofn.lStructSize = sizeof(OPENFILENAME);
ofn.hwndOwner = parent;
ofn.lpstrFile = &path[0];
ofn.nMaxFile = pathlen -1;
ofn.lpstrFilter = UntranslateNulls(filters).data();
ofn.nFilterIndex = (nFilterSelected? *nFilterSelected : 1);
ofn.lpstrTitle = (title.size()>0? title.c_str() : NULL);
if (flags&1) ofn.Flags =                        OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER | ((flags&3)==3? OFN_ALLOWMULTISELECT : 0);
else ofn.Flags =                        OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_NOREADONLYRETURN;
if (
(flags&1 && GetOpenFileName(&ofn))
|| (!(flags&1) && GetSaveFileName(&ofn)) 
) {
if (nFilterSelected) *nFilterSelected = ofn.nFilterIndex;
if (3==(flags&3)) TranslateNulls(&path[0]);
return &path[0];
} 
else return TEXT("");
}

bool export FontDialog (HWND parent, LOGFONT& lf) {
CHOOSEFONT cf;
ZeroMemory(&cf, sizeof(cf));
cf.lStructSize = sizeof(cf);
cf.hwndOwner = parent;
cf.lpLogFont = &lf;
cf.Flags = CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT;
return ChooseFont(&cf);
}

COLORREF export ColorDialog (HWND parent, COLORREF initial) {
COLORREF custColors[16] = {0};
CHOOSECOLOR cc;
ZeroMemory(&cc, sizeof(cc));
cc.lStructSize = sizeof(cc);
cc.hwndOwner = parent;
cc.rgbResult = initial;
cc.lpCustColors = custColors;
cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
if (!ChooseColor(&cc)) return initial;
else return cc.rgbResult;
}

struct ChoiceDlgData {
const vector<tstring>& choices;
const tstring &prompt, &title;
int selection;
ChoiceDlgData (const vector<tstring>&  c, const tstring& t, const tstring& p, int s= -1): choices(c), title(t), prompt(p), selection(s) {}
};

static LRESULT CALLBACK ChoiceListDlgSubclassProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR subclassId, DWORD_PTR unused) {
static tstring input = TEXT("");
static DWORD lastInput = 0;
switch(msg){
case WM_CHAR: {
TCHAR ch = LOWORD(wp);
if (ch<32) break;
DWORD time = GetTickCount();
int pos = SendMessage(hwnd, LB_GETCURSEL, 0, 0);
if (time-lastInput>500) input = TEXT("");
lastInput = time;
input += ch;
int npos = SendMessage(hwnd, LB_FINDSTRING, pos -1, input.c_str() );
if (npos<0 || npos==LB_ERR) { MessageBeep(MB_OK); return true; }
else if (npos==pos) return true;
else { SendMessage(hwnd, LB_SETCURSEL, npos, 0); return true; }
}break;
}
return DefSubclassProc(hwnd, msg, wp, lp);
}

static INT_PTR CALLBACK ChoiceDlgProc (HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp) {
static ChoiceDlgData* data = NULL;
switch (umsg) {
case WM_INITDIALOG : {
HWND hList = GetDlgItem(hwnd, 1002);
data = (ChoiceDlgData*)lp;
SetDlgItemText(hwnd, IDOK, msg("&OK"));
SetDlgItemText(hwnd, IDCANCEL, msg("Ca&ncel"));
SetDlgItemText(hwnd, 1001, data->prompt);
SetWindowText(hwnd, data->title);
SendMessage(hList, LB_RESETCONTENT, 0, 0);
for (int l, i=0, n=data->choices.size(); i<n; i++) {
l = SendMessage(hList, LB_ADDSTRING, 0, data->choices[i].c_str() );
SendMessage(hList, LB_SETITEMDATA, l, i);
}
SendMessage(hList, LB_SETCURSEL, data->selection, 0);
SetWindowSubclass(hList, ChoiceListDlgSubclassProc, 0, 0);
SetFocus(hList);
}return TRUE;
case WM_COMMAND :
switch (LOWORD(wp)) {
case IDOK : {
HWND hList = GetDlgItem(hwnd, 1002);
int sel = SendMessage(hList, LB_GETCURSEL, 0, 0);
data->selection = sel<0? -1 : SendMessage(hList, LB_GETITEMDATA, sel, 0);
EndDialog(hwnd, wp); 
}return true;
case IDCANCEL : 
data->selection = -1; 
EndDialog(hwnd, wp); 
return true;
}}
return FALSE;
}

int export ChoiceDialog (HWND parent, const tstring& title, const tstring& prompt, const vector<tstring>& choices, int defaultSelection) {
ChoiceDlgData cdd(choices, title, prompt, defaultSelection);
DialogBoxParam(dllHinstance, IDD_CHOICE, parent, ChoiceDlgProc, &cdd);
return cdd.selection;
}
