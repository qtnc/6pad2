#ifndef ___PAGE_H9
#define ___PAGE_H9
#include "global.h"

#define PF_READONLY 1
#define PF_NOSAVE 2
#define PF_AUTOLINEBREAK 4

#define PF_NOFIND 0x80000000
#define PF_NOREPLACE 0x40000000
#define PF_NOGOTO 0x20000000
#define PF_NOENCODING 0x10000000
#define PF_NOINDENTATION 0x8000000
#define PF_NOLINEENDING 0x4000000
#define PF_NOAUTOLINEBREAK 0x2000000
#define PF_NOCOPY 0x1000000
#define PF_NOPASTE 0x800000
#define PF_NOUNDO 0x400000
#define PF_NOSELECTALL 0x200000

struct Page {
tstring name=TEXT(""), file=TEXT("");
int encoding=-1, indentationMode=-1, lineEnding=-1;
DWORD flags = 0;
HWND zone=0;

virtual HWND CreateEditArea (HWND parent)=0;
virtual bool IsEmpty () =0;
virtual bool IsModified () = 0;
virtual tstring LoadText (const tstring& fn = TEXT(""), bool guessFormat = true)  =0;
virtual bool SaveText (const tstring& fn = TEXT("")) =0;

virtual void UpdateStatusBar () {}
virtual void Copy ()  { SendMessage(zone, WM_COPY, 0, 0); }
virtual void Cut ()  { SendMessage(zone, WM_CUT, 0, 0); }
virtual void Paste ()  { SendMessage(zone, WM_PASTE, 0, 0); }
virtual void SelectAll () {}
virtual void GoTo (int pos) {}
virtual void GoToDialog () {}
virtual void FindDialog ()  {}
virtual void FindReplaceDialog ()  {}
virtual void FindNext () {}
virtual void FindPrev ()  {}
virtual void FindReplace (const tstring& search, const tstring& replace, bool caseSensitive, bool isRegex) {}
};

struct TextPage: Page {
virtual bool IsEmpty () ;
virtual bool IsModified () ;
virtual HWND CreateEditArea (HWND parent);
virtual tstring LoadText (const tstring& fn = TEXT(""), bool guessFormat=true ) ;
virtual bool SaveText (const tstring& fn = TEXT(""));

virtual void UpdateStatusBar () ;
virtual void SelectAll () ;
virtual void GoTo (int);
virtual void GoToDialog ();
virtual void FindDialog () ;
virtual void FindReplaceDialog () ;
virtual void FindNext ();
virtual void FindPrev () ;
virtual void FindReplace (const tstring& search, const tstring& replace, bool caseSensitive, bool isRegex);
};

#endif
