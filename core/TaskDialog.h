#ifndef _____TASK_DIALOG_X9_H
#define _____TASK_DIALOG_X9_H
#include "global.h"
#include "commctrlmsvc6.h"
//Using XTaskDialog from http://www.naughter.com/xtaskdialog.html

export HRESULT WINAPI TaskDialogIndirect (TASKDIALOGCONFIG* tdc, int* pButton, int* pRadio, int* pCheckbox);

#endif