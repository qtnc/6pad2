#ifndef ___PYPROGRESSDIALOG_H1
#define ___PYPROGRESSDIALOG_H1
#include "main.h"

struct PyProgressDialog {
    PyObject_HEAD 
ProgressDialog* pd;

static PyProgressDialog* New (const tstring& title, const tstring& label);
static PyObject* open (PyObject* unused, PyObject* args, PyObject* kw); 
void close ();
int get_closed ();
int get_cancelled ();
int get_paused ();
tstring get_title ();
tstring get_text  ();
int get_value ();
void set_title (const tstring& title);
void set_text (const tstring& text);
void set_value (int i);
};

#endif
