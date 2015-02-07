#ifndef ___DIALOGS_H9
#define ___DIALOGS_H9
#include "global.h"

#define FD_SAVE 0
#define FD_OPEN 1
#define FD_MULTI 2

tstring FileDialog (HWND parent, int flags, const tstring& file = TEXT(""), const tstring& title = TEXT(""), const tstring& filters = TEXT(""), int* nFilterSelected = 0) ;

#endif
