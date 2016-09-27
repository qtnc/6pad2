#include "main.h"
using namespace std;

void export ProgressDialog::update (int pos) {
if (pos==lastUpdate) return;
lastUpdate=pos;
SendMessage(hwnd, WM_USER, pos, 0);
}

void export ProgressDialog::update (const tstring& text) {
SendMessage(hwnd, WM_USER, 0, text.c_str());
}

void export ProgressDialog::close () {
if (!closed) SendMessage(hwnd, WM_COMMAND, IDCANCEL, 0);
}

INT_PTR ProgressDlgProc (HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp) {
static ProgressDialog* info = nullptr;
switch(umsg){
case WM_INITDIALOG:{
info = (ProgressDialog*)lp;
SetWindowText(hwnd, info->title);
SetDlgItemText(hwnd, 1001, info->text);
SetDlgItemText(hwnd, IDCANCEL, msg("Ca&ncel"));
SetDlgItemText(hwnd, 1003, msg("&Pause"));
SendDlgItemMessage(hwnd, 1002, PBM_SETRANGE32, 0, 100);
SendDlgItemMessage(hwnd, 1002, PBM_SETPOS, 0, 0);
info->hwnd=hwnd;
info->cancelled=false;
info->paused=false;
info->closed=false;
}return TRUE; //WM_INITDIALOG
case WM_COMMAND: switch(LOWORD(wp)){
case IDCANCEL:
info->closed=true;
info->cancelled=true;
EndDialog(hwnd,1);
break;
case 1003:
info->paused = !info->paused;
SetDlgItemText(hwnd, 1003, msg(info->paused?"&Continue":"&Pause"));
break;
}break;//WM_COMMAND
case WM_USER:
if(lp) SetDlgItemText(hwnd, 1001, (LPCTSTR)lp);
else SendDlgItemMessage(hwnd, 1002, PBM_SETPOS, wp, 0);
break;
}
return FALSE;
}

void export ProgressDialog::show (HWND parent) {
DialogBoxParam(hinstance, IDD_PROGRESS, parent, ProgressDlgProc, this);
}

