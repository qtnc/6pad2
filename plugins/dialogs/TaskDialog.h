#ifndef _____TASK_DIALOG_X9_H
#define _____TASK_DIALOG_X9_H
#include "../../core/global.h"
#include "commctrlmsvc6.h"

HRESULT WINAPI TaskDialogIndirect (TASKDIALOGCONFIG* tdc, int* pButton, int* pRadio, int* pCheckbox);

#endif