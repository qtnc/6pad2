#ifndef ___GLOBAL_H9
#define ___GLOBAL_H9
#define UNICODE
#define _WIN32_IE 0x0600
#define _WIN32_WINNT 0x600
#include<windows.h>
#include<commctrl.h>

#ifndef __WINDRES
#define export __declspec(dllexport)
#include<cstdio>
#include<string>
#include<vector>

#ifdef UNICODE
#define Py_TString_Decl "u"
typedef std::wstring tstring;
#define toTString toWString
#define tstrlen wcslen
#define tstrdup wcsdup
#define tstrrchr wcsrchr
#define tsnprintf snwprintf
inline bool IsUnicode () { return true; }
#else
#define Py_TString_Decl "s"
typedef std::string tstring;
#define toTString toString
#define tstrlen strlen
#define tstrdup strdup
#define tstrrchr strrchr
#define tsnprintf snprintf
inline bool IsUnicode () { return false; }
#endif
#define TSTR(s) tstring(TEXT(s))
#define wsnprintf snwprintf
#include "strings.hpp"
#include "win32api++.hpp"
#endif//windres

#ifndef RELEASE
#define RELEASE 0
#endif

#ifndef DEBUG
#define DEBUG 0
#endif

#define SIXPAD_VERSION "Alpha 10.2"
#define SIXPAD_VERSION_ID 0x0000000A

#define OF_CHECK_OTHER_WINDOWS 1
#define OF_NEW_INSTANCE 2

#define LE_DOS 0
#define LE_UNIX 1
#define LE_MAC 2
#define LE_RS 3
#define LE_LS 4

#define CP_UTF16 1200
#define CP_UTF16_LE 1200
#define CP_UTF16_BE 1201
#define CP_UTF16_LE_BOM 1202
#define CP_UTF16_BE_BOM 1203
#define CP_UTF32 12000
#define CP_UTF32_LE 12000
#define CP_UTF32_BE 12001
#define CP_UTF32_LE_BOM 12002
#define CP_UTF32_BE_BOM 12003
#define CP_UTF8_BOM 65002
#define CP_ISO_8859_15 28605
//#define CP_MSDOS 850

#define WM_RUNPROC WM_USER + 1563

#define IDC_TABCTL 1
#define IDC_STATUSBAR 2
#define IDC_EDITAREA 5

#define IDD_CONSOLE 900
#define IDD_GOTOLINE 901
#define IDD_SEARCHREPLACE 902
#define IDD_CHOICE 903
#define IDD_INPUT 904

#define IDM_NEW 1000
#define IDM_OPEN 1001
#define IDM_SAVE 1002
#define IDM_SAVE_AS 1003
#define IDM_CLOSE 1004
#define IDM_EXIT 1005
#define IDM_OPEN_NI 1006
#define IDM_REOPEN 1007
#define IDM_NEXT_MODLESS 1896
#define IDM_PREV_MODLESS 1897
#define IDM_ABOUT 1898
#define IDM_CRASH 1899
#define IDM_RECENT_FILE 1900
#define IDM_COPY 2000
#define IDM_CUT 2001
#define IDM_PASTE 2002
#define IDM_UNDO 2003
#define IDM_REDO 2004
#define IDM_SELECTALL 2005
#define IDM_FIND 2006
#define IDM_FINDNEXT 2007
#define IDM_FINDPREV 2008
#define IDM_REPLACE 2009
#define IDM_GOTOLINE 2010
#define IDM_ENCODING 2100
#define IDM_LE_DOS 2200
#define IDM_LE_UNIX 2201
#define IDM_LE_MAC 2202
#define IDM_LE_RS 2203
#define IDM_LE_LS 2204
#define IDM_INDENTATION_TABS 2209
#define IDM_INDENTATION_SPACES 2210
#define IDM_TAB_WIDTH 2220
#define IDM_AUTOLINEBREAK 2230
#define IDM_NEXTPAGE 2231
#define IDM_PREVPAGE 2232
#define IDM_MARKSEL 2233
#define IDM_SELTOMARK 2234
#define IDM_GOTOMARK 2235
#define IDM_SELECTFONT 2236
#define IDM_OTHER_ENCODINGS 2237
#define IDM_QUICKJUMP 2238
#define IDM_OPEN_CONSOLE 2300
#define IDM_USER_COMMAND 3000

#ifndef __WINDRES
using namespace std;
#endif
#endif