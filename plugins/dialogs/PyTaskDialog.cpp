#include "main.h"
#include "TaskDialog.h"

PyObject* PyTaskDialog (PyObject* unused, PyObject* args, PyObject* kwds) {
int button=0, radio=0, checkbox=0, expanded=0;
const TCHAR* icon = TEXT("info");
PyObject *buttonList=nullptr, *radioList=nullptr;
vector<TASKDIALOG_BUTTON> pButtons, pRadios;
TASKDIALOGCONFIG td;
ZeroMemory(&td,sizeof(td));
td.cbSize = sizeof(td);
td.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW;
static const char* KWLST[] = { "title", "heading", "text", "footer", "details", "checkbox", "checked", "collapseButtonText", "expandButtonText", "expanded", "icon", "buttons", "radioButtons", "defaultButton", "defaultRadioButton", NULL};
if (!PyArg_ParseTupleAndKeywords(args, kwds, "|uuuuuupuupuOOii", (char**)KWLST, &td.pszWindowTitle, &td.pszMainInstruction, &td.pszContent, &td.pszFooter, &td.pszExpandedInformation, &td.pszVerificationText, &checkbox, &td.pszExpandedControlText, &td.pszCollapsedControlText, &expanded, &icon, &buttonList, &radioList, &button, &radio    )) return NULL;
if (expanded) td.dwFlags |= TDF_EXPANDED_BY_DEFAULT;
if (checkbox) td.dwFlags |= TDF_VERIFICATION_FLAG_CHECKED;
if (icon) td.pszMainIcon = MAKEINTRESOURCEW(-1 -elt<tstring>(icon, 2, { TEXT("warning"), TEXT("error"), TEXT("info"), TEXT("shield") }));
if (buttonList && PyLong_Check(buttonList)) {
td.dwCommonButtons = PyLong_AsLong(buttonList);
td.nDefaultButton = button;
}
else if (buttonList && PySequence_Check(buttonList)) {
for (int i=0, n=PySequence_Size(buttonList); i<n; i++) {
PyObject* item = PySequence_GetItem(buttonList,i);
if (!item || !PyUnicode_Check(item)) return NULL;
const wchar_t* str = PyUnicode_AsUnicode(item);
if (!str) return NULL;
pButtons.push_back({1000+i, str});
}
td.nDefaultButton = 1000 + button;
td.cButtons = pButtons.size();
td.pButtons = &(pButtons[0]);
}
if (radioList && PySequence_Check(radioList)) {
for (int i=0, n=PySequence_Size(radioList); i<n; i++) {
PyObject* item = PySequence_GetItem(radioList,i);
if (!item || !PyUnicode_Check(item)) return NULL;
const wchar_t* str = PyUnicode_AsUnicode(item);
if (!str) return NULL;
pRadios.push_back({2000+i, str});
}
td.nDefaultRadioButton = 2000 + radio;
td.cRadioButtons = pRadios.size();
td.pRadioButtons = &(pRadios[0]);
}
HRESULT hr = TaskDialogIndirect(&td, &button, &radio, &checkbox);
if (button==IDCANCEL) button=-1;
else if (button>=1000) button -= 1000;
if (radio>=2000) radio -= 2000;
return Py_BuildValue("(iii)", button, radio, checkbox);
}

