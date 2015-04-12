#ifndef ___PAGE_H9
#define ___PAGE_H9
#include "global.h"
#include "python34.h"
#include "eventlist.h"
#include<boost/enable_shared_from_this.hpp>

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

struct Page: std::enable_shared_from_this<Page>  {
tstring name=TEXT(""), file=TEXT("");
int encoding=-1, indentationMode=-1, lineEnding=-1;
DWORD flags = 0;
HWND zone=0;
PySafeObject pyData;
eventlist listeners;

virtual HWND CreateEditArea (HWND parent)=0;
virtual PyObject* GetPyData () { return *pyData; }
virtual bool IsEmpty () =0;
virtual bool IsModified () = 0;
virtual tstring LoadText (const tstring& fn = TEXT(""), bool guessFormat = true)  =0;
virtual bool SaveText (const tstring& fn = TEXT("")) =0;

virtual void UpdateStatusBar () {}
virtual void GetSelection (int& start, int& end) = 0;
virtual tstring GetSelectedText ()  =0;
virtual int GetAllTextLength ()  = 0;
virtual tstring GetAllText ()  =0;
virtual void SetSelection (int start, int end) =0;
virtual void SetSelectedText (const tstring& str) =0;
virtual void SetAllText (const tstring& str) = 0;
virtual void ReplaceTextRange (int start, int end, const tstring& str) =0;
virtual tstring GetLine (int line)  =0;
virtual int GetLineCount ()  =0;
virtual int GetLineLength (int line) =0;
virtual int GetLineStartIndex (int line) =0;
virtual int GetLineOfPos (int pos) =0;

inline int GetSelectionStart () { int s,e; GetSelection(s,e); return s; }
inline int GetSelectionEnd () { int s,e; GetSelection(s,e); return e; }
inline void SetSelectionStart (int x) { SetSelection(x, GetSelectionEnd()); }
inline void SetSelectionEnd (int x) { SetSelection(GetSelectionStart(), x); }

virtual void SetName (const tstring& name) ;
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

template<class R, R initial, class... A> inline R dispatchEvent (const string& type, A... args) { return listeners.dispatch<R,initial>(type, *pyData, args...); }
template<class... A> inline void dispatchEvent (const string& type, A... args) { listeners.dispatch(type, *pyData, args...); }
inline void addEvent (const std::string& type, const PyCallback& cb) { listeners.add(type,cb); }
inline void removeEvent (const std::string& type, const PyCallback& cb) { listeners.remove(type,cb); }
};

struct TextPage: Page {
virtual bool IsEmpty () ;
virtual bool IsModified () ;
virtual HWND CreateEditArea (HWND parent);
virtual tstring LoadText (const tstring& fn = TEXT(""), bool guessFormat=true ) ;
virtual bool SaveText (const tstring& fn = TEXT(""));

virtual PyObject* GetPyData ();
virtual void UpdateStatusBar () ;
virtual void GetSelection (int& start, int& end);
virtual tstring GetSelectedText () ;
virtual tstring GetAllText () ;
virtual int GetAllTextLength () ;
virtual void ReplaceTextRange (int start, int end, const tstring& str);
virtual tstring GetLine (int line) ;
virtual int GetLineCount () ;
virtual int GetLineLength (int line);
virtual int GetLineStartIndex (int line);
virtual int GetLineOfPos (int pos);
virtual void SetSelection (int start, int end);
virtual void SetSelectedText (const tstring& str);
virtual void SetAllText (const tstring& str);

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
