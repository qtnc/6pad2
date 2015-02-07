#ifndef ___PAGE_H9
#define ___PAGE_H9
#include "global.h"

#define PF_READONLY 1
#define PF_NOSAVE 2
#define PF_AUTOLINEBREAK 4

struct Page {
tstring name=TEXT(""), file=TEXT("");
int encoding=-1, indentationMode=-1, lineEnding=-1, flags = 0;
HWND zone=0;

virtual HWND CreateEditArea (HWND parent)=0;
virtual bool IsEmpty () =0;
virtual bool IsModified () = 0;
virtual tstring LoadText (const tstring& fn = TEXT(""), bool guessFormat = true)  =0;
virtual bool SaveText (const tstring& fn = TEXT("")) =0;
};

struct TextPage: Page {
virtual bool IsEmpty () ;
virtual bool IsModified () ;
virtual HWND CreateEditArea (HWND parent);
virtual tstring LoadText (const tstring& fn = TEXT(""), bool guessFormat=true ) ;
virtual bool SaveText (const tstring& fn = TEXT(""));
};

#endif
