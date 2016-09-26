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

struct PyPage: PyObjectWithDic { 
weak_ptr<Page> wpPage;
bool seqAsLines;

shared_ptr<Page> page () {
shared_ptr<Page> p = wpPage.lock();
if (p) return p;
else {
PyErr_SetString(PyExc_ValueError, "Page is closed");
return shared_ptr<Page>(new Page());
}}

bool isClosed () { return wpPage.expired(); }
bool isModified () { return page()->IsModified(); }
void setModified (bool b) { page()->SetModified(b); }
bool isReadOnly () { return page()->IsReadOnly(); }
void setReadOnly (bool b) { page()->SetReadOnly(b); }
tstring getName () { return page()->name; }
tstring getFile () { return page()->file; }
void setName (const tstring& s) { page()->SetName(s); }
void setFile (const tstring& s) { page()->file=s; }
bool getSeqLineFlag () { return seqAsLines; }
void setSeqLineFlag (bool x) { seqAsLines=x; }
int getLineEnding () { return page()->lineEnding; }
int getEncoding () { return page()->encoding; }
int getIndentationMode () { return page()->indentationMode; }
int getTabWidth () { return page()->tabWidth; }
tstring getIndentString () { shared_ptr<Page> p = page(); return tstring(max(p->indentationMode,1), p->indentationMode>0?' ':'\t'); }
bool getAutoLineBreak () { return 0!=(page()->flags&PF_AUTOLINEBREAK); }
void setLineEnding (int le) { RunSync([&]()mutable{ page()->SetLineEnding(le); }); }
void setEncoding (int e) { RunSync([&]()mutable{ page()->SetEncoding(e); }); }
void setIndentationMode (int i) { RunSync([&]()mutable{ page()->SetIndentationMode(i); }); }
void setTabWidth (int i) { RunSync([&]()mutable{ page()->SetTabWidth(i); }); }
void setAutoLineBreak (bool b) { RunSync([&]()mutable{ page()->SetAutoLineBreak(b); }); }
string getDotEditorConfigValue (const string& key, OPT, const string& def) {
IniFile& ini = page()->dotEditorConfig;
auto it = ini.find(key);
return it!=ini.end()? it->second : def;
}
int addEvent (const string& type, PyGenericFunc cb) {  return page()->AddEvent(type,cb); }
int removeEvent (const string& type, int id) { return page()->RemoveEvent(type, id); }
void focus () { page()->Focus(); }
void close () { page()->Focus(); page()->Close(); }
bool find (const tstring& term, OPT, bool scase, bool regex, bool up, bool stealthty) {  bool re=false; RunSync([&]()mutable{  re = page()->Find(term, scase, regex, up, stealthty); });  return re; }
void searchReplace (const tstring& sText, const tstring& rText, OPT, bool scase, bool regex, bool stealthty) {  RunSync([&]()mutable{ page()->FindReplace(sText, rText, scase, regex, stealthty); });  }
bool findNext () { bool re; RunSync([&]()mutable{ re = page()->FindNext(); });  return re; }
bool findPrev () { bool re; RunSync([&]()mutable{ re = page()->FindPrev(); }); return re; }
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
int getLineEndIndex (int l) { return page()->GetLineEndIndex(l); }
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
};//PyPage class

static void PyPageDealloc (PyObject* pySelf) {
PyPage* self = (PyPage*)pySelf;
Py_XDECREF(self->dic);
self->dic = NULL;
self->wpPage = weak_ptr<Page>();
Py_TYPE(pySelf)->tp_free(pySelf);
}

static PyPage* PyPageNew (PyTypeObject* type, PyObject* args, PyObject* kwds) {
PyPage* self = (PyPage*)(type->tp_alloc(type, 0));
self->wpPage = weak_ptr<Page>();
self->seqAsLines = false;
self->dic = PyDict_New();
return self;
}

static int PyPageInit (PyPage* self, PyObject* args, PyObject* kwds) {
return 0;
}

static int PyMapLen (PyObject* o) {
PyPage& t = *(PyPage*)o;
if (t.seqAsLines) return t.getLineCount();
else return t.getTextLength();
}

static PyObject* PyMapGet (PyObject* o, PyObject* k) {
PyPage& t = *(PyPage*)o;
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
PyPage& t = *(PyPage*)o;
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

static constexpr const char* PyPage_find_KWLST[] = {"term", "scase", "regex", "up", "stealthty", NULL};
static constexpr const char* PyPage_searchReplace_KWLST[] = {"search", "replacement", "scase", "regex", "stealthty", NULL};

static PyMappingMethods PyPageMapping = {
PyMapLen, // length
PyMapGet, // Get
PyMapSet, // set
};

static PyMethodDef PyPageMethods[] = {
PyDecl("addEvent", &PyPage::addEvent),
PyDecl("removeEvent", &PyPage::removeEvent),
PyDecl("select", &PyPage::setSelection),
PyDecl("line", &PyPage::getLine),
PyDecl("lineLength", &PyPage::getLineLength),
PyDecl("lineOfOffset", &PyPage::getLineOfPos),
PyDecl("lineStartOffset", &PyPage::getLineStartIndex),
PyDecl("lineEndOffset", &PyPage::getLineEndIndex),
PyDecl("lineSafeStartOffset", &PyPage::getLineSafeStartIndex),
PyDecl("lineIndentLevel", &PyPage::getLineIndentLevel),
PyDecl("columnOfOffset", &PyPage::getColOfPos),
PyDecl("substring", &PyPage::getTextSubstring),
PyDecl("replace", &PyPage::replaceTextRange),
PyDecl("insert", &PyPage::insertTextAt),
PyDecl("delete", &PyPage::deleteTextRange),
PyDecl("focus", &PyPage::focus),
PyDecl("close", &PyPage::close),
PyDecl("pushUndoState", &PyPage::pushUndoState),
PyDecl("undo", &PyPage::undo),
PyDecl("redo", &PyPage::redo),
PyDecl("save", &PyPage::save),
PyDecl("reload", &PyPage::reload),
PyDeclKW("find", &PyPage::find, PyPage_find_KWLST),
PyDecl("findNext", &PyPage::findNext),
PyDecl("findPrevious", &PyPage::findPrev),
PyDeclKW("searchReplace", &PyPage::searchReplace, PyPage_searchReplace_KWLST),
PyDecl("doteditorconfig", &PyPage::getDotEditorConfigValue),
PyDeclEnd
};

static PyGetSetDef PyPageAccessors[] = {
PyReadOnlyAccessor("closed", &PyPage::isClosed),
PyAccessor("name", &PyPage::getName, &PyPage::setName),
PyAccessor("file", &PyPage::getFile, &PyPage::setFile),
PyAccessor("modified", &PyPage::isModified, &PyPage::setModified),
PyAccessor("readOnly", &PyPage::isReadOnly, &PyPage::setReadOnly),
PyAccessor("rangesInLines", &PyPage::getSeqLineFlag, &PyPage::setSeqLineFlag),
PyAccessor("lineEnding", &PyPage::getLineEnding, &PyPage::setLineEnding),
PyAccessor("encoding", &PyPage::getEncoding, &PyPage::setEncoding),
PyAccessor("indentation", &PyPage::getIndentationMode, &PyPage::setIndentationMode),
PyAccessor("tabWidth", &PyPage::getTabWidth, &PyPage::setTabWidth),
PyAccessor("autoLineBreak", &PyPage::getAutoLineBreak, &PyPage::setAutoLineBreak),
PyAccessor("selectionStart", &PyPage::getSelectionStart, &PyPage::setSelectionStart),
PyAccessor("selectionEnd", &PyPage::getSelectionEnd, &PyPage::setSelectionEnd),
PyAccessor("position", &PyPage::getSelectionEnd, &PyPage::setPosition),
PyAccessor("selectedText", &PyPage::getSelectedText, &PyPage::setSelectedText),
PyAccessor("text", &PyPage::getText, &PyPage::setText),
PyAccessor("curLine", &PyPage::getCurLine, &PyPage::setCurLine),
PyAccessor("curLineText", &PyPage::getCurLineText, &PyPage::setCurLineText),
PyReadOnlyAccessor("curColumn", &PyPage::getCurCol),
PyReadOnlyAccessor("textLength", &PyPage::getTextLength),
PyReadOnlyAccessor("lineCount", &PyPage::getLineCount),
PyReadOnlyAccessor("indentString", &PyPage::getIndentString),
PyDeclEnd
};

static PyTypeObject PyPageType = { 
    PyVarObject_HEAD_INIT(NULL, 0) 
    "sixpad.Page",             /* tp_name */ 
    sizeof(PyPage), /* tp_basicsize */ 
    0,                         /* tp_itemsize */ 
    PyPageDealloc,                         /* tp_dealloc */ 
    0,                         /* tp_print */ 
    0,                         /* tp_getattr */ 
    0,                         /* tp_setattr */ 
    0,                         /* tp_reserved */ 
    0,                         /* tp_repr */ 
    0,                         /* tp_as_number */ 
    0,                         /* tp_as_sequence */ 
    &PyPageMapping,                         /* tp_as_mapping */ 
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
    PyPageMethods,             /* tp_methods */ 
NULL,             /* tp_members */ 
    PyPageAccessors,                         /* tp_getset */ 
    0,                         /* tp_base */ 
    0,                         /* tp_dict */ 
    0,                         /* tp_descr_get */ 
    0,                         /* tp_descr_set */ 
    offsetof(PyPage,dic),                         /* tp_dictoffset */ 
    (initproc)PyPageInit,      /* tp_init */ 
    0,                         /* tp_alloc */ 
}; 

void PyPage::setCurLineText (const tstring& str) { 
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

PyObject* export CreatePyPageObject (shared_ptr<Page> p) {
GIL_PROTECT
PyPage* it = PyPageNew(&PyPageType, NULL, NULL);
it->wpPage = p;
return (PyObject*)it;
}

shared_ptr<Page> Page::FromPyData (PyObject* o) {
PyPage* e = (PyPage*)o;
return e->page();
}

bool export PyRegister_Page (PyObject* m) {
//PyPageType.tp_new = (decltype(PyPageType.tp_new))PyPageNew;
if (PyType_Ready(&PyPageType) < 0)          return false;
Py_INCREF(&PyPageType); 
PyModule_AddObject(m, "Page", (PyObject*)&PyPageType); 
return true;
}
