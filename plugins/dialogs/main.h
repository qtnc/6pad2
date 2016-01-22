#ifndef ___QC6PADDLGS_MAIN_DLL___
#define ___QC6PADDLGS_MAIN_DLL___
#include "../../core/global.h"

#define IDD_OUTPUT 899
#define IDD_TREEVIEW 898
#define IDD_TREEVIEW2 897
#define IDD_PROGRESS 896

#ifndef __WINDRES
#include "../../core/sixpad.h"
#include "../../core/python34.h"
#include "../../core/thread.h"

extern HINSTANCE hinstance;

struct export ProgressDialog {
tstring title, text;
HWND hwnd;
bool cancelled, paused, closed;
int lastUpdate;
ProgressDialog (const tstring& t, const tstring& l): title(t), text(l), hwnd(0), cancelled(false), closed(false), paused(false), lastUpdate(0) {}
void export show (HWND parent);
void export update (int pos);
void export update (const tstring& text);
void export close();
};

struct export CustomDialog {
private:
char* buffer;
size_t pos, length;

void align (size_t n);
void push (const tstring& s, size_t alignment=sizeof(WORD));
void push (const void* data, size_t len);
template<class T> inline void push (const T& val) { push(&val, sizeof(val)); }

public :
CustomDialog(const tstring& title): buffer(0), pos(0), length(0) { setTitle(title); }
~CustomDialog () { if (buffer) delete[] buffer; }
void export setSize (int x, int y, int w, int h);
void export addItem (const tstring& type, int id, const tstring& text, DWORD style, int x, int y, int w, int h, DWORD exstyle=0);
INT_PTR export show (HWND hwndParent, DLGPROC proc, void* udata=0);
private: void setTitle (const tstring& title);
};



#endif //__WINDRES
#endif
