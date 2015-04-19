#include "global.h"
#include "strings.hpp"
#include "page.h"
#include "python34.h"
#include<boost/weak_ptr.hpp>
using namespace std;

void PageSetLineEnding (shared_ptr<Page> p, int le);
void PageSetEncoding (shared_ptr<Page> p, int enc);
void PageSetIndentationMode (shared_ptr<Page> p, int im);
void PageSetAutoLineBreak (shared_ptr<Page> p, bool alb);

extern vector<shared_ptr<Page>> pages;

struct PyEditorTab { 
    PyObject_HEAD
weak_ptr<Page> wpPage;

shared_ptr<Page> page () {
shared_ptr<Page> p = wpPage.lock();
if (p) return p;
else {
PyErr_SetString(PyExc_ValueError, "Page is closed");
return shared_ptr<Page>(Page::create());
}}

int isClosed () { return wpPage.expired(); }
int isModified () { return page()->IsModified(); }
tstring getName () { return page()->name; }
tstring getFile () { return page()->file; }
void setName (const tstring& s) { page()->SetName(s); }
void setFile (const tstring& s) { page()->file=s; }
int getLineEnding () { return page()->lineEnding; }
int getEncoding () { return page()->encoding; }
int getIndentationMode () { return page()->indentationMode; }
int getAutoLineBreak () { return 0!=(page()->flags&PF_AUTOLINEBREAK); }
void setLineEnding (int le) { PageSetLineEnding(page(),le); }
void setEncoding (int e) { PageSetEncoding(page(),e); }
void setIndentationMode (int i) { PageSetIndentationMode(page(),i); }
void setAutoLineBreak (int b) { PageSetAutoLineBreak(page(),b); }
void addEvent (const string& type, const PyCallback& cb) {  page()->addEvent(type,cb); }
void removeEvent (const string& type, const PyCallback& cb) { page()->removeEvent(type,cb); }
void focus () { page()->EnsureFocus(); }
void close () { page()->EnsureFocus(); page()->Close(); }
int getTextLength () { return page()->GetTextLength(); }
tstring getSelectedText () { return page()->GetSelectedText(); }
void setSelectedText (const tstring& s) { page()->SetSelectedText(s); }
tstring getText () { return page()->GetText(); }
void setText (const tstring& s) { page()->SetText(s); }
int getSelectionStart () { return page()->GetSelectionStart(); }
int getSelectionEnd () { return page()->GetSelectionEnd(); }
void setSelectionStart (int s) { page()->SetSelectionStart(s); }
void setSelectionEnd (int s) { page()->SetSelectionEnd(s); }
void setSelection (int s, int e) { page()->SetSelection(s,e); }
int getLineLength (int l) { return page()->GetLineLength(l); }
tstring getLine (int l) { return page()->GetLine(l); }
int getLineStartIndex (int l) { return page()->GetLineStartIndex(l); }
int getLineEndIndex (int l) { return page()->GetLineStartIndex(l) + page()->GetLineLength(l); }
int getLineOfPos (int pos) { return page()->GetLineOfPos(pos); }
void replaceTextRange (int start, int end, const tstring& str) { page()->ReplaceTextRange(start, end, str); }
void deleteTextRange (int start, int end) { replaceTextRange(start, end, TEXT("")); }
void insertTextAt (int pos, const tstring& str) { replaceTextRange(pos, pos, str); }
};

static void PyEditorTabDealloc (PyObject* pySelf) {
PyEditorTab* self = (PyEditorTab*)pySelf;
self->wpPage = weak_ptr<Page>();
Py_TYPE(pySelf)->tp_free(pySelf);
}

static PyEditorTab* PyEditorTabNew (PyTypeObject* type, PyObject* args, PyObject* kwds) {
PyEditorTab* self = (PyEditorTab*)(type->tp_alloc(type, 0));
self->wpPage = weak_ptr<Page>();
return self;
}

static int PyEditorTabInit (PyEditorTab* self, PyObject* args, PyObject* kwds) {
return 0;
}

static PyMethodDef PyEditorTabMethods[] = {
PyDecl("addEvent", &PyEditorTab::addEvent),
PyDecl("removeEvent", &PyEditorTab::removeEvent),
PyDecl("select", &PyEditorTab::setSelection),
PyDecl("line", &PyEditorTab::getLine),
PyDecl("lineLength", &PyEditorTab::getLineLength),
PyDecl("lineOfOffset", &PyEditorTab::getLineOfPos),
PyDecl("lineStartOffset", &PyEditorTab::getLineStartIndex),
PyDecl("lineEndOffset", &PyEditorTab::getLineEndIndex),
PyDecl("replace", &PyEditorTab::replaceTextRange),
PyDecl("insert", &PyEditorTab::insertTextAt),
PyDecl("delete", &PyEditorTab::deleteTextRange),
PyDecl("focus", &PyEditorTab::focus),
PyDecl("close", &PyEditorTab::close),
PyDeclEnd
};

static PyGetSetDef PyEditorTabAccessors[] = {
PyReadOnlyAccessor("closed", &PyEditorTab::isClosed),
PyAccessor("name", &PyEditorTab::getName, &PyEditorTab::setName),
PyAccessor("file", &PyEditorTab::getFile, &PyEditorTab::setFile),
PyReadOnlyAccessor("modified", &PyEditorTab::isModified),
PyAccessor("lineEnding", &PyEditorTab::getLineEnding, &PyEditorTab::setLineEnding),
PyAccessor("encoding", &PyEditorTab::getEncoding, &PyEditorTab::setEncoding),
PyAccessor("indentation", &PyEditorTab::getIndentationMode, &PyEditorTab::setIndentationMode),
PyAccessor("autoLineBreak", &PyEditorTab::getAutoLineBreak, &PyEditorTab::setAutoLineBreak),
PyAccessor("selectionStart", &PyEditorTab::getSelectionStart, &PyEditorTab::setSelectionStart),
PyAccessor("selectionEnd", &PyEditorTab::getSelectionEnd, &PyEditorTab::setSelectionEnd),
PyAccessor("selectedText", &PyEditorTab::getSelectedText, &PyEditorTab::setSelectedText),
PyAccessor("text", &PyEditorTab::getText, &PyEditorTab::setText),
PyReadOnlyAccessor("textLength", &PyEditorTab::getTextLength),
PyDeclEnd
};

static PyTypeObject PyEditorTabType = { 
    PyVarObject_HEAD_INIT(NULL, 0) 
    "sixpad.EditorTab",             /* tp_name */ 
    sizeof(PyEditorTab), /* tp_basicsize */ 
    0,                         /* tp_itemsize */ 
    PyEditorTabDealloc,                         /* tp_dealloc */ 
    0,                         /* tp_print */ 
    0,                         /* tp_getattr */ 
    0,                         /* tp_setattr */ 
    0,                         /* tp_reserved */ 
    0,                         /* tp_repr */ 
    0,                         /* tp_as_number */ 
    0,                         /* tp_as_sequence */ 
    0,                         /* tp_as_mapping */ 
    0,                         /* tp_hash  */ 
    0,                         /* tp_call */ 
    0,                         /* tp_str */ 
    0,                         /* tp_getattro */ 
    0,                         /* tp_setattro */ 
    0,                         /* tp_as_buffer */ 
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /* tp_flags */ 
    NULL,           /* tp_doc */
    0,                         /* tp_traverse */ 
    0,                         /* tp_clear */ 
    0,                         /* tp_richcompare */ 
    0,                         /* tp_weaklistoffset */ 
    0,                         /* tp_iter */ 
    0,                         /* tp_iternext */ 
    PyEditorTabMethods,             /* tp_methods */ 
NULL,             /* tp_members */ 
    PyEditorTabAccessors,                         /* tp_getset */ 
    0,                         /* tp_base */ 
    0,                         /* tp_dict */ 
    0,                         /* tp_descr_get */ 
    0,                         /* tp_descr_set */ 
    0,                         /* tp_dictoffset */ 
    (initproc)PyEditorTabInit,      /* tp_init */ 
    0,                         /* tp_alloc */ 
}; 

PyObject* CreatePyEditorTabObject (shared_ptr<Page> p) {
GIL_PROTECT
PyEditorTab* it = PyEditorTabNew(&PyEditorTabType, NULL, NULL);
it->wpPage = p;
return (PyObject*)it;
}

bool PyRegister_EditorTab (PyObject* m) {
//PyEditorTabType.tp_new = (decltype(PyEditorTabType.tp_new))PyEditorTabNew;
if (PyType_Ready(&PyEditorTabType) < 0)          return false;
Py_INCREF(&PyEditorTabType); 
PyModule_AddObject(m, "EditorTab", (PyObject*)&PyEditorTabType); 
return true;
}
