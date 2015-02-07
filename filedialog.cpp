#include "global.h"
#include<boost/shared_array.hpp>
using namespace boost;
using namespace std;

tstring FileDialog (HWND parent, int flags, const tstring& file, const tstring& title, const tstring& filters, int* nFilterSelected) {
int pathlen = ((flags&3)==3? 16384: 512);
shared_array<TCHAR> path( new TCHAR[pathlen] );
OPENFILENAME ofn;
ZeroMemory(&path[0], pathlen*sizeof(TCHAR));
printf("file=%d<%ls>\r\n", file.size(), file.c_str());
if (file.size()>0) memcpy(&path[0], file.c_str(), sizeof(TCHAR) * (file.size()+1));
ZeroMemory(&ofn, sizeof(OPENFILENAME));
                ofn.lStructSize = sizeof(OPENFILENAME);
ofn.hwndOwner = parent;
ofn.lpstrFile = &path[0];
ofn.nMaxFile = pathlen -1;
ofn.lpstrFilter = filters.data();
ofn.nFilterIndex = (nFilterSelected? *nFilterSelected : 1);
ofn.lpstrTitle = (title.size()>0? title.c_str() : NULL);
if (flags&1) ofn.Flags =                        OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER | ((flags&3)==3? OFN_ALLOWMULTISELECT : 0);
else ofn.Flags =                        OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_NOREADONLYRETURN;
if (
(flags&1 && GetOpenFileName(&ofn))
|| (!(flags&1) && GetSaveFileName(&ofn)) 
) {
if (nFilterSelected) *nFilterSelected = ofn.nFilterIndex;
return &path[0];
} 
else return TEXT("");
}
