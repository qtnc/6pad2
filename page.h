#ifndef ___PAGE_H9
#define ___PAGE_H9
#include "global.h"
#include "python34.h"
#include "eventlist.h"
#include<boost/enable_shared_from_this.hpp>

#define PF_CLOSED 1
#define PF_READONLY 2
#define PF_MUSTSAVEAS 4
#define PF_AUTOLINEBREAK 8

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
#define PF_NOSAVE 0x100000

struct Page;

struct UndoState {
virtual void Undo (Page&) = 0;
virtual void Redo (Page&) = 0;
virtual bool Join (UndoState& s) { return false; }
virtual int GetTypeId () { return 0; }
virtual ~UndoState(){}
};

struct Page: std::enable_shared_from_this<Page>  {
tstring name=TEXT(""), file=TEXT("");
int encoding=-1, indentationMode=-1, lineEnding=-1, markedPosition=0, curUndoState=0;
DWORD flags = 0;
HWND zone=0;
PySafeObject pyData;
eventlist listeners;
std::vector<shared_ptr<UndoState>> undoStates;

virtual ~Page() {}
virtual void SetName (const tstring& name) ;
virtual bool IsEmpty () ;
virtual bool IsModified () ;
virtual void SetModified (bool);
virtual bool IsReadOnly ();
virtual void SetReadOnly (bool);
virtual void CreateZone (HWND parent);
virtual void ResizeZone (const RECT&);
virtual void HideZone ();
virtual void ShowZone (const RECT&);
virtual void FocusZone ();
virtual void EnsureFocus ();
virtual void Close () ;
virtual bool LoadFile (const tstring& fn = TEXT(""), bool guessFormat=true ) ;
virtual bool LoadData (const string& data, bool guessFormat=true);
virtual bool SaveFile (const tstring& fn = TEXT(""));
virtual string SaveData ();
virtual void Copy () ;
virtual void Cut ();
virtual void Paste ();
virtual void Undo () ;
virtual void Redo () ;
virtual void PushUndoState (shared_ptr<UndoState> state, bool tryToJoin = true);

virtual PyObject* GetPyData ();
virtual void UpdateStatusBar (HWND) ;
virtual void GetSelection (int& start, int& end);
virtual tstring GetSelectedText () ;
virtual tstring GetText () ;
virtual tstring GetTextSubstring (int start, int end);
virtual int GetTextLength () ;
virtual void ReplaceTextRange (int start, int end, const tstring& str, bool keepOldSelection=true);
virtual tstring GetLine (int line) ;
virtual int GetLineCount () ;
virtual int GetLineLength (int line);
virtual int GetLineStartIndex (int line);
virtual int GetLineSafeStartIndex (int line);
virtual int GetLineIndentLevel (int line);
virtual int GetLineOfPos (int pos);
virtual void SetSelection (int start, int end);
virtual void SetSelectedText (const tstring& str);
virtual void SetText (const tstring& str);

virtual void SelectAll () ;
virtual int GetCurrentPosition ();
virtual void SetCurrentPosition  (int);
virtual void GoToDialog ();
virtual void FindDialog () ;
virtual void FindReplaceDialog () ;
virtual void Find(const tstring& searchText, bool scase, bool regex, bool up);
virtual void FindNext ();
virtual void FindPrev () ;
virtual void FindReplace (const tstring& search, const tstring& replace, bool caseSensitive, bool isRegex);

inline int GetSelectionStart () { int s,e; GetSelection(s,e); return s; }
inline int GetSelectionEnd () { int s,e; GetSelection(s,e); return e; }
inline void SetSelectionStart (int x) { SetSelection(x, GetSelectionEnd()); }
inline void SetSelectionEnd (int x) { SetSelection(GetSelectionStart(), x); }
inline void MarkCurrentPosition () { markedPosition = GetCurrentPosition(); }
inline void SelectToMark () { SetSelection(markedPosition, GetSelectionEnd()); }
inline void GoToMark () { SetCurrentPosition(markedPosition); }

template<class R, R initial, class... A> inline R dispatchEvent (const string& type, A... args) { return listeners.dispatch<R,initial>(type, *pyData, args...); }
template<class... A> inline var dispatchEvent (const string& type, const var& def, A... args) { return listeners.dispatch(type, def, *pyData, args...); }
template<class... A> inline void dispatchEvent (const string& type, A... args) { listeners.dispatch(type, *pyData, args...); }
inline void addEvent (const std::string& type, const PyCallback& cb) { listeners.add(type,cb); }
inline void removeEvent (const std::string& type, const PyCallback& cb) { listeners.remove(type,cb); }

static inline Page* create () { return new Page(); }
};

#endif
