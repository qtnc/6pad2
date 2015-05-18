#ifndef ___DIALOGS_H9
#define ___DIALOGS_H9
#include "global.h"

#define FD_SAVE 0
#define FD_OPEN 1
#define FD_MULTI 2

tstring export FileDialog (HWND parent, int flags, const tstring& file = TEXT(""), const tstring& title = TEXT(""), tstring  filters = TEXT(""), int* nFilterSelected = 0) ;
bool export FontDialog (HWND parent, LOGFONT&);
COLORREF export ColorDialog (HWND parent, COLORREF clr = RGB(0,0,0) );
int export ChoiceDialog (HWND parent, const tstring& title, const tstring& prompt, const std::vector<tstring>& choices, int defaultSelection = -1);

#endif
