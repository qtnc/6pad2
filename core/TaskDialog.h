#ifndef _____TASK_DIALOG_X9_H
#define _____TASK_DIALOG_X9_H
#include "global.h"
#include "commctrlmsvc6.h"
//Using XTaskDialog from http://www.naughter.com/xtaskdialog.html

#define MB2_ICONEXCLAMATION 1
#define MB2_ICONWARNING 1
#define MB2_ICONERROR 2
#define MB2_ICONINFORMATION 3
#define MB2_ICONASTERISK 3
#define MB2_ICONSHIELD 4
#define MB2_LINKS (TDF_USE_COMMAND_LINKS| TDF_USE_COMMAND_LINKS_NO_ICON)


export HRESULT WINAPI TaskDialogIndirect (TASKDIALOGCONFIG* tdc, int* pButton, int* pRadio, int* pCheckbox);
export int MessageBox2 (HWND hwnd, const tstring& title, const tstring& heading, const tstring& text, const vector<tstring>& buttons={}, int flags = MB2_ICONINFORMATION, int defaultButton=0);

#endif