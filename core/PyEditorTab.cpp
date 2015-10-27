#include "global.h"
#include "strings.hpp"
#include "page.h"
#include "thread.h"
#include "python34.h"
//#include<boost/weak_ptr.hpp>
using namespace std;

extern vector<shared_ptr<Page>> pages;

struct PyProxyUndoState: UndoState  {
PySafeObject obj;
PyProxyUndoState (const PySafeObject& o): obj(o) {}
void Undo (Page&);
void Redo (Page&);
};

struct PyEditorTab { 
    PyObject_HEAD
PyObject* dic;
weak_ptr<Page> wpPage;
bool seqAsLines;

shared_ptr<Page> page () {
shared_ptr<Page> p = wpPage.lock();
if (p) return p;
else {
PyErr_SetString(PyExc_ValueError, "Page is closed");
return shared_ptr<Page>(new Page());
}}

int isClosed () { return wpPage.expired(); }
int isModified () { return page()->IsModified(); }
void setModified (bool b) { page()->SetModified(b); }
int isReadOnly () { return page()->IsReadOnly(); }
void setReadOnly (bool b) { page()->SetReadOnly(b); }
tstring getName () { return page()->name; }
tstring getFile () { return page()->file; }
void setName (const tstring& s) { page()->SetName(s); }
void setFile (const tstring& s) { page()->file=s; }
int getSeqLineFlag () { return seqAsLines; }
void setSeqLineFlag (int x) { seqAsLines=!!x; }
int getLineEnding () { return page()->lineEnding; }
int getEncoding () { return page()->encoding; }
int getIndentationMode () { return page()->indentationMode; }
int getTabWidth () { return page()->tabWidth; }
tstring getIndentString () { shared_ptr<Page> p = page(); return tstring(max(p->indentationMode,1), p->indentationMode>0?' ':'\t'); }
int getAutoLineBreak () { return 0!=(page()->flags&PF_AUTOLINEBREAK); }
void setLineEnding (int le) { RunSync([&]()mutable{ page()->SetLineEnding(le); }); }
void setEncoding (int e) { RunSync([&]()mutable{ page()->SetEncoding(e); }); }
void setIndentationMode (int i) { RunSync([&]()mutable{ page()->SetIndentationMode(i); }); }
void setTabWidth (int i) { RunSync([&]()mutable{ page()->SetTabWidth(i); }); }
void setAutoLineBreak (int b) { RunSync([&]()mutable{ page()->SetAutoLineBreak(b); }); }
int addEvent (const string& type, const PySafeObject& cb) {  return page()->AddEvent(type,cb); }
int removeEvent (const string& type, int id) { return page()->RemoveEvent(type, id); }
void focus () { page()->Focus(); }
void close () { page()->Focus(); page()->Close(); }
bool find (const tstring& term, bool scase, bool regex, bool up, bool stealthty) {  RunSync([&]()mutable{  page()->Find(term, scase, regex, up, stealthty); });  }
void searchReplace (const tstring& sText, const tstring& rText, bool scase, bool regex, bool stealthty) {  RunSync([&]()mutable{ page()->FindReplace(sText, rText, scase, regex, stealthty); });  }
int findNext () { bool re; RunSync([&]()mutable{ re = page()->FindNext(); });  return re; }
int findPrev () { bool re; RunSync([&]()mutable{ re = page()->FindPrev(); }); return re; }
void undo () { RunSync([&]()mutable{ page()->Undo(); }); }
void redo () { RunSync([&]()mutable{ page()->Redo(); }); }
void save () { RunSync([&]()mutable{ page()->SaveFile(); }); }
void reload () { RunSync([&]()mutable{ page()->LoadFile(TEXT(""),false); }); }
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
void setPosition (int p) { setSelection(p,p); }
int getLineLength (int l) { return page()->GetLineLength(l); }
tstring getLine (int l) { return page()->GetLine(l); }
int getCurLine () { return getLineOfPos(getSelectionEnd()); }
int getLineStartIndex (int l) { return page()->GetLineStartIndex(l); }
int getLineEndIndex (int l) { return page()->GetLineStartIndex(l) + page()->GetLineLength(l); }
int getLineSafeStartIndex (int l) { return page()->GetLineSafeStartIndex(l); }
int getLineIndentLevel (int l) { return page()->GetLineIndentLevel(l); }
int getLineOfPos (int pos) { return page()->GetLineOfPos(pos); }
int getLineCount () { return page()->GetLineCount(); }
int getColOfPos (int p) { int l = getLineOfPos(p); int x = getLineStartIndex(l); return p-x; }
int getCurCol () { return getColOfPos(getSelectionEnd()); }
tstring getCurLineText () { return getLine(getCurLine()); }
void setCurLine (int i) { setPosition(getLineSafeStartIndex(i)); }
void setCurLineText (const tstring&);
void replaceTextRange (int start, int end, const tstring& str) { page()->ReplaceTextRange(start, end, str, true); }
void deleteTextRange (int start, int end) { replaceTextRange(start, end, TEXT("")); }
void insertTextAt (int pos, const tstring& str) { replaceTextRange(pos, pos, str); }
tstring getTextSubstring (int start, int end) { return page()->GetTextSubstring(start,end); }
void pushUndoState (PyObject* o) { page()->PushUndoState(shared_ptr<UndoState>(new PyProxyUndoState(o)), false); }
};//PyEditorTab class

static void PyEditorTabDealloc (PyObject* pySelf) {
PyEditorTab* self = (PyEditorTab*)pySelf;
Py_XDECREF(self->dic);
self->dic = NULL;
self->wpPage = weak_ptr<Page>();
Py_TYPE(pySelf)->tp_free(pySelf);
}

static PyEditorTab* PyEditorTabNew (PyTypeObject* type, PyObject* args, PyObject* kwds) {
PyEditorTab* self = (PyEditorTab*)(type->tp_alloc(type, 0));
self->wpPage = weak_ptr<Page>();
self->seqAsLines = false;
self->dic = PyDict_New();
return self;
}

static int PyEditorTabInit (PyEditorTab* self, PyObject* args, PyObject* kwds) {
return 0;
}

static PyObject* PyPageFind (PyEditorTab* self, PyObject* args, PyObject* dic) {
const wchar_t *findText=0;
bool scase=false, regex=false, up=false, stealthty=false;
static const char* KWLST[] = {"term", "scase", "regex", "up", "stealthty", NULL};
if (!PyArg_ParseTupleAndKeywords(args, dic, "u|pppp", (char**)KWLST, &findText, &scase, &regex, &up, &stealthty)) return NULL;
bool re=false;
if(self) re = self->find(findText, scase, regex, up, stealthty);
PyObject* re2 = re? Py_True : Py_False;
Py_INCREF(re2);
return re2;
}

static PyObject* PyPageFindReplace (PyEditorTab* self, PyObject* args, PyObject* dic) {
const wchar_t *findText=0, *replaceText=0;
bool scase=false, regex=false, stealthty=false;
static const char* KWLST[] = {"search", "replacement", "scase", "regex", "stealthty", NULL};
if (!PyArg_ParseTupleAndKeywords(args, dic, "uu|ppp", (char**)KWLST, &findText, &replaceText, &scase, &regex, &stealthty)) return NULL;
if(self) self->searchReplace(findText, replaceText, scase, regex, stealthty);
Py_RETURN_NONE;
}


static int PyMapLen (PyObject* o) {
PyEditorTab& t = *(PyEditorTab*)o;
if (t.seqAsLines) return t.getLineCount();
else return t.getTextLength();
}

static PyObject* PyMapGet (PyObject* o, PyObject* k) {
PyEditorTab& t = *(PyEditorTab*)o;
if (PyLong_Check(k)) {
int i = PyLong_AsLong(k);
if (!t.seqAsLines) {
if (i<0) i += t.getTextLength();
return Py_BuildValue(Py_TString_Decl, t.getTextSubstring(i, i+1).c_str() );
}
else {
if (i<0) i+=t.getLineCount();
int s = t.getLineStartIndex(i), e = t.getLineLength(i);
return Py_BuildValue(Py_TString_Decl, t.getTextSubstring(s, s+e).c_str() );
}}
else if (PySlice_Check(k)) {
int start, end, step, slicelen, len = (t.seqAsLines? t.getLineCount() : t.getTextLength() );
if (PySlice_GetIndicesEx(k, len, &start, &end, &step, &slicelen)) return NULL;
if (step!=1) { PyErr_SetString(PyExc_ValueError, "step!=1 isn't supported."); return NULL; }
if (start>end) { int i=start; start=end; end=i; }
if (t.seqAsLines) { start = t.getLineStartIndex(start); end = t.getLineEndIndex(end -1); }
return Py_BuildValue(Py_TString_Decl, t.getTextSubstring(start, end).c_str() );
}
PyErr_SetString(PyExc_TypeError, "int or slice expected"); 
return NULL;
}

static int PyMapSet (PyObject* o, PyObject* k, PyObject* v) {
if (v&&!PyUnicode_Check(v)) { PyErr_SetString(PyExc_TypeError, "str expected");  return -1; }
tstring str = v? PyUnicode_AsUnicode(v) : TEXT("");
PyEditorTab& t = *(PyEditorTab*)o;
if (PyLong_Check(k)) {
int i = PyLong_AsLong(k);
if (!t.seqAsLines) {
if (i<0) i += t.getTextLength();
t.replaceTextRange(i, i+1, str);
return 0;
}
else {
if (i<0) i+=t.getLineCount();
int s = t.getLineStartIndex(i);
int e = s + t.getLineLength(i);
if (!v) e+=2;
t.replaceTextRange(s, e, str);
return 0;
}}
else if (PySlice_Check(k)) {
int start, end, step, slicelen, len = (t.seqAsLines? t.getLineCount() : t.getTextLength() );
if (PySlice_GetIndicesEx(k, len, &start, &end, &step, &slicelen)) return -1;
if (step!=1) { PyErr_SetString(PyExc_ValueError, "step!=1 isn't supported."); return -1; }
if (start>end) { int i=start; start=end; end=i; }
if (t.seqAsLines) { start = t.getLineStartIndex(start); end = t.getLineEndIndex(end -1); }
if (!v&&t.seqAsLines) end+=2;
t.replaceTextRange(start, end, str);
return 0;
}
PyErr_SetString(PyExc_TypeError, "int or slice expected"); 
return -1;
}

static PyMappingMethods PyEditorTabMapping = {
PyMapLen, // length
PyMapGet, // Get
PyMapSet, // set
};

static PyMethodDef PyEditorTabMethods[] = {
PyDecl("addEvent", &PyEditorTab::addEvent),
PyDecl("removeEvent", &PyEditorTab::removeEvent),
PyDecl("select", &PyEditorTab::setSelection),
PyDecl("line", &PyEditorTab::getLine),
PyDecl("lineLength", &PyEditorTab::getLineLength),
PyDecl("lineOfOffset", &PyEditorTab::getLineOfPos),
PyDecl("lineStartOffset", &PyEditorTab::getLineStartIndex),
PyDecl("lineEndOffset", &PyEditorTab::getLineEndIndex),
PyDecl("lineSafeStartOffset", &PyEditorTab::getLineSafeStartIndex),
PyDecl("lineIndentLevel", &PyEditorTab::getLineIndentLevel),
PyDecl("columnOfOffset", &PyEditorTab::getColOfPos),
PyDecl("substring", &PyEditorTab::getTextSubstring),
PyDecl("replace", &PyEditorTab::replaceTextRange),
PyDecl("insert", &PyEditorTab::insertTextAt),
PyDecl("delete", &PyEditorTab::deleteTextRange),
PyDecl("focus", &PyEditorTab::focus),
PyDecl("close", &PyEditorTab::close),
PyDecl("pushUndoState", &PyEditorTab::pushUndoState),
PyDecl("undo", &PyEditorTab::undo),
PyDecl("redo", &PyEditorTab::redo),
PyDecl("save", &PyEditorTab::save),
PyDecl("reload", &PyEditorTab::reload),
//PyDecl("find", &PyEditorTab::find),
{"find", (PyCFunction)PyPageFind, METH_VARARGS | METH_KEYWORDS, NULL},
PyDecl("findNext", &PyEditorTab::findNext),
PyDecl("findPrevious", &PyEditorTab::findPrev),
{"searchReplace", (PyCFunction)PyPageFindReplace, METH_VARARGS | METH_KEYWORDS, NULL},
PyDeclEnd
};

static PyGetSetDef PyEditorTabAccessors[] = {
PyReadOnlyAccessor("closed", &PyEditorTab::isClosed),
PyAccessor("name", &PyEditorTab::getName, &PyEditorTab::setName),
PyAccessor("file", &PyEditorTab::getFile, &PyEditorTab::setFile),
PyAccessor("modified", &PyEditorTab::isModified, &PyEditorTab::setModified),
PyAccessor("readOnly", &PyEditorTab::isReadOnly, &PyEditorTab::setReadOnly),
PyAccessor("rangesInLines", &PyEditorTab::getSeqLineFlag, &PyEditorTab::setSeqLineFlag),
PyAccessor("lineEnding", &PyEditorTab::getLineEnding, &PyEditorTab::setLineEnding),
PyAccessor("encoding", &PyEditorTab::getEncoding, &PyEditorTab::setEncoding),
PyAccessor("indentation", &PyEditorTab::getIndentationMode, &PyEditorTab::setIndentationMode),
PyAccessor("tabWidth", &PyEditorTab::getTabWidth, &PyEditorTab::setTabWidth),
PyAccessor("autoLineBreak", &PyEditorTab::getAutoLineBreak, &PyEditorTab::setAutoLineBreak),
PyAccessor("selectionStart", &PyEditorTab::getSelectionStart, &PyEditorTab::setSelectionStart),
PyAccessor("selectionEnd", &PyEditorTab::getSelectionEnd, &PyEditorTab::setSelectionEnd),
PyAccessor("position", &PyEditorTab::getSelectionEnd, &PyEditorTab::setPosition),
PyAccessor("selectedText", &PyEditorTab::getSelectedText, &PyEditorTab::setSelectedText),
PyAccessor("text", &PyEditorTab::getText, &PyEditorTab::setText),
PyAccessor("curLine", &PyEditorTab::getCurLine, &PyEditorTab::setCurLine),
PyAccessor("curLineText", &PyEditorTab::getCurLineText, &PyEditorTab::setCurLineText),
PyReadOnlyAccessor("curColumn", &PyEditorTab::getCurCol),
PyReadOnlyAccessor("textLength", &PyEditorTab::getTextLength),
PyReadOnlyAccessor("lineCount", &PyEditorTab::getLineCount),
PyReadOnlyAccessor("indentString", &PyEditorTab::getIndentString),
PyDeclEnd
};

static PyTypeObject PyEditorTabType = { 
    PyVarObject_HEAD_INIT(NULL, 0) 
    "sixpad.Page",             /* tp_name */ 
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
    &PyEditorTabMapping,                         /* tp_as_mapping */ 
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
    offsetof(PyEditorTab,dic),                         /* tp_dictoffset */ 
    (initproc)PyEditorTabInit,      /* tp_init */ 
    0,                         /* tp_alloc */ 
}; 

void PyEditorTab::setCurLineText (const tstring& str) { 
int l = getCurLine();
int s = getLineStartIndex(l);
int e = s + getLineLength(l);
replaceTextRange(s, e, str);
}

void PyProxyUndoState::Undo (Page& p) {
GIL_PROTECT
PyObject* arg = p.GetPyData();
CallMethod<void>(*obj, "undo", arg);
}

void PyProxyUndoState::Redo (Page& p) {
GIL_PROTECT
PyObject* arg = p.GetPyData();
CallMethod<void>(*obj, "redo", arg);
}

PyObject* export CreatePyEditorTabObject (shared_ptr<Page> p) {
GIL_PROTECT
PyEditorTab* it = PyEditorTabNew(&PyEditorTabType, NULL, NULL);
it->wpPage = p;
return (PyObject*)it;
}

bool export PyRegister_EditorTab (PyObject* m) {
//PyEditorTabType.tp_new = (decltype(PyEditorTabType.tp_new))PyEditorTabNew;
if (PyType_Ready(&PyEditorTabType) < 0)          return false;
Py_INCREF(&PyEditorTabType); 
PyModule_AddObject(m, "EditorTab", (PyObject*)&PyEditorTabType); 
return true;
}
