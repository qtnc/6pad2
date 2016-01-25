#include "TaskDialog.h"

export HRESULT WINAPI TaskDialogIndirect (TASKDIALOGCONFIG* tdc, int* pButton, int* pRadio, int* pCheckbox) {
typedef HRESULT(*WINAPI TaskDialogIndirectFunc)(TASKDIALOGCONFIG*,int*,int*,int*);
static TaskDialogIndirectFunc func = nullptr;
if (!func) {
HINSTANCE dll = LoadLibrary(TEXT("comctl32.dll"));
if (dll) func = (TaskDialogIndirectFunc)GetProcAddress(dll, "TaskDialogIndirect");
if (!func&&dll) func = (TaskDialogIndirectFunc)GetProcAddress(dll, "TaskDialogIndirectW");
if (!func) {
dll = LoadLibrary(TEXT("xtaskdlg.dll"));
if (dll) func = (TaskDialogIndirectFunc)GetProcAddress(dll, "TaskDialogIndirect");
}}
if (!func) return GetLastError();
return func(tdc, pButton, pRadio, pCheckbox);
}

