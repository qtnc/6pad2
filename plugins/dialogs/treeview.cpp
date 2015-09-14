#include "main.h"

struct TreeViewDialog {
HWND hTree;
HTREEITEM addItem (const tstring& text, HTREEITEM parent=TVI_ROOT, HTREEITEM after=TVI_LAST);
void removeItem (HTREEITEM item);
void select (HTREEITEM item);
bool allowEdit (HTREEITEM item, tstring& text);
bool allowEdit (HTREEITEM item);
};

struct TreeViewDialogInfo {
tstring title, label;
TreeViewDialog* dlg;
};

void TreeViewDialog::removeItem (HTREEITEM item) {
SendMessage(hTree, TVM_DELETEITEM, 0, item);
}

HTREEITEM TreeViewDialog::addItem (const tstring& text, HTREEITEM parent, HTREEITEM after) {
TVINSERTSTRUCT ins;
ins.hParent = parent;
ins.hInsertAfter = after;
ins.item.mask = TVIF_TEXT;
ins.item.hItem = NULL;
ins.item.state = 0;
ins.item.stateMask = 0;
ins.item.pszText = (LPTSTR)text.c_str();
ins.item.cchTextMax = text.size();
return (HTREEITEM)SendMessage(hTree, TVM_INSERTITEM, 0, &ins);
}

void TreeViewDialog::select (HTREEITEM item) {
SendMessage(hTree, TVM_SELECTITEM, TVGN_CARET, item);
}

bool TreeViewDialog::allowEdit (HTREEITEM item) {
return true;
}

bool TreeViewDialog::allowEdit (HTREEITEM item, tstring& text) {
return true;
}


static LRESULT CALLBACK EditingTreeLabelProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR subclassId, HWND hTree) {
if (msg==WM_KEYDOWN) switch(LOWORD(wp)){
case VK_RETURN: SendMessage(hTree, TVM_ENDEDITLABELNOW, FALSE, 0); return true;
case VK_ESCAPE: SendMessage(hTree, TVM_ENDEDITLABELNOW, TRUE, 0); return true;
}
else if (msg==WM_GETDLGCODE) return DLGC_WANTALLKEYS;
return DefSubclassProc(hwnd, msg, wp, lp);
}

INT_PTR TreeViewDlgProc (HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp) {
switch (umsg) {
case WM_INITDIALOG : {
TreeViewDialogInfo& tvdi = *(TreeViewDialogInfo*)(lp);
tvdi.dlg = new TreeViewDialog();
TreeViewDialog& dlg = *tvdi.dlg;
HWND hTree = GetDlgItem(hwnd, 1002);
dlg.hTree = hTree;
SetWindowLong(hwnd, DWL_USER, (LONG)tvdi.dlg);
SetWindowText(hwnd, tvdi.title);
SetDlgItemText(hwnd, 1001, tvdi.label);
SetDlgItemText(hwnd, IDOK, msg("&OK"));
SetDlgItemText(hwnd, IDCANCEL, msg("&Close"));
for (int i=0; i<5; i++) {
HTREEITEM item = dlg.addItem(TEXT("Item ") + toTString(i+1));
for (int j=0; j<5; j++) {
dlg.addItem(TEXT("Item ") + toTString(i+1) + TEXT(".") + toTString(j+1), item);
}}
SetFocus(hTree);
//SetWindowSubclass(GetDlgItem(hwnd,1002), (SUBCLASSPROC)ConsoleDlgInputSubclassProc, 0, 0);
sp->AddModlessWindow(hwnd);
}return false;
case WM_COMMAND :
switch(LOWORD(wp)) {
case IDOK:
Beep(800,200);
break;
case IDCANCEL:
sp->RemoveModlessWindow(hwnd);
DestroyWindow(hwnd);
sp->GoToNextModlessWindow(0);
delete (TreeViewDialog*)( GetWindowLong(hwnd, DWL_USER));
break;
}break;
case WM_NOTIFY : {
LPNMHDR nmh = (LPNMHDR)(lp);
if (nmh->idFrom==1002) {
TreeViewDialog& dlg = *(TreeViewDialog*)GetWindowLong(hwnd, DWL_USER);
switch(nmh->code){
case NM_DBLCLK:
case NM_RETURN:
SendMessage(hwnd, WM_COMMAND, IDOK, 0);
return true;
case NM_RCLICK:
Beep(1200,200);
return true;
case NM_RDBLCLK:
break;
case TVN_DELETEITEM : {
TVITEM& item = ((LPNMTREEVIEW)lp)->itemOld;
}break;
case TVN_BEGINLABELEDIT: {
TVITEM& item = ((LPNMTVDISPINFO)lp)->item;
if (!dlg.allowEdit(item.hItem)) return true;
HWND hEdit = (HWND)SendMessage(nmh->hwndFrom, TVM_GETEDITCONTROL, 0, 0);
if (hEdit) SetWindowSubclass(hEdit, (SUBCLASSPROC)EditingTreeLabelProc, 0, (DWORD_PTR)nmh->hwndFrom);
return false;
}break;
case TVN_ENDLABELEDIT: {
TVITEM& item = ((LPNMTVDISPINFO)lp)->item;
if (item.pszText) {
tstring text = item.pszText;
if (!dlg.allowEdit(item.hItem, text)) return false;
item.mask = TVIF_TEXT;
item.pszText = (LPTSTR)text.c_str();
item.cchTextMax = text.size();
SendMessage(nmh->hwndFrom, TVM_SETITEM, 0, &item);
}
return true;
}break;
case TVN_KEYDOWN: {
WORD key = *(WORD*)(nmh+1);
switch(key){
case 0x5D: { 
NMHDR z = *(LPNMHDR)lp; z.code = NM_RCLICK;
SendMessage(hwnd, WM_NOTIFY, 0, &z);
}break;
case VK_F2: SendMessage(nmh->hwndFrom, TVM_EDITLABEL, 0, SendMessage(nmh->hwndFrom, TVM_GETNEXTITEM, TVGN_CARET, NULL)); break;
}return false;}
//other notifications
}}}break;
case WM_ACTIVATE:
//SetDlgItemFocus(hwnd, 1002);
break;
}
return FALSE;
}

void test123 (void) {
TreeViewDialogInfo tvdi = { TEXT("Tree view dialog"), TEXT("Tree view control"), NULL };
HWND hDlg = CreateDialogParam(hinstance, IDD_TREEVIEW, sp->win, TreeViewDlgProc, &tvdi);
ShowWindow(hDlg, SW_SHOW);
}
