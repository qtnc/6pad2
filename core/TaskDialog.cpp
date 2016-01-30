#include "TaskDialog.h"

#define MB2_FLAGMASK (MB2_LINKS)

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

export int MessageBox2 (HWND hwnd, const tstring& title, const tstring& heading, const tstring& text, const vector<tstring>& buttons, int flags, int defaultButton) {
int result = -1;
vector<TASKDIALOG_BUTTON> tdb;
for (int i=0, n=buttons.size(); i<n; i++) {
tdb.push_back({ 1000+i, buttons[i].c_str() });
}
TASKDIALOGCONFIG td;
ZeroMemory(&td,sizeof(td));
td.cbSize = sizeof(td);
td.hwndParent = hwnd;
td.dwFlags = TDF_POSITION_RELATIVE_TO_WINDOW | TDF_ALLOW_DIALOG_CANCELLATION | (flags&MB2_FLAGMASK);
td.pszWindowTitle = title.size()>0? title.c_str() : NULL;
td.pszMainInstruction = heading.size()>0? heading.c_str() : NULL;
td.pszContent = text.size()>0? text.c_str() : NULL;
td.cButtons = buttons.size();
td.pButtons = &(tdb[0]);
td.nDefaultButton = 1000+defaultButton;
td.pszMainIcon = MAKEINTRESOURCEW(-(flags&0x0F));
TaskDialogIndirect(&td, &result, nullptr, nullptr);
if (result==2) return -1;
else return result -1000;
}

