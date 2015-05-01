#include "global.h"
#include<boost/shared_array.hpp>
using namespace std;
using boost::shared_array;

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

tstring FileDialog (HWND parent, int flags, const tstring& file, const tstring& title, tstring filters, int* nFilterSelected) {
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

bool FontDialog (HWND parent, LOGFONT& lf) {
CHOOSEFONT cf;
ZeroMemory(&cf, sizeof(cf));
cf.lStructSize = sizeof(cf);
cf.hwndOwner = parent;
cf.lpLogFont = &lf;
cf.Flags = CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT;
return ChooseFont(&cf);
}

COLORREF ColorDialog (HWND parent, COLORREF initial) {
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
