/*[clinic input]
preserve
[clinic start generated code]*/

PyDoc_STRVAR(_curses_form_form_driver__doc__,
"driver($self, request, /)\n"
"--\n"
"\n"
"Process one form request.");

#define _CURSES_FORM_FORM_DRIVER_METHODDEF    \
    {"driver", (PyCFunction)_curses_form_form_driver, METH_O, _curses_form_form_driver__doc__},

static PyObject *
_curses_form_form_driver_impl(PyCursesFormObject *self, int request);

static PyObject *
_curses_form_form_driver(PyCursesFormObject *self, PyObject *arg)
{
    PyObject *return_value = NULL;
    int request;

    request = PyLong_AsInt(arg);
    if (request == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = _curses_form_form_driver_impl(self, request);

exit:
    return return_value;
}

PyDoc_STRVAR(_curses_form_form_hide__doc__,
"hide($self, /)\n"
"--\n"
"\n"
"Hide the form.\n"
"\n"
"This does not delete the object, it just makes the window on screen invisible.");

#define _CURSES_FORM_FORM_HIDE_METHODDEF    \
    {"hide", _PyCFunction_CAST(_curses_form_form_hide), METH_METHOD|METH_FASTCALL|METH_KEYWORDS, _curses_form_form_hide__doc__},

static PyObject *
_curses_form_form_hide_impl(PyCursesFormObject *self, PyTypeObject *cls);

static PyObject *
_curses_form_form_hide(PyCursesFormObject *self, PyTypeObject *cls, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    if (nargs) {
        PyErr_SetString(PyExc_TypeError, "hide() takes no arguments");
        return NULL;
    }
    return _curses_form_form_hide_impl(self, cls);
}

PyDoc_STRVAR(_curses_form_form_show__doc__,
"show($self, /)\n"
"--\n"
"\n"
"Display the form (which might have been hidden).");

#define _CURSES_FORM_FORM_SHOW_METHODDEF    \
    {"show", _PyCFunction_CAST(_curses_form_form_show), METH_METHOD|METH_FASTCALL|METH_KEYWORDS, _curses_form_form_show__doc__},

static PyObject *
_curses_form_form_show_impl(PyCursesFormObject *self, PyTypeObject *cls);

static PyObject *
_curses_form_form_show(PyCursesFormObject *self, PyTypeObject *cls, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    if (nargs) {
        PyErr_SetString(PyExc_TypeError, "show() takes no arguments");
        return NULL;
    }
    return _curses_form_form_show_impl(self, cls);
}

PyDoc_STRVAR(_curses_form_form_window__doc__,
"window($self, /)\n"
"--\n"
"\n"
"Return the window object associated with the form.");

#define _CURSES_FORM_FORM_WINDOW_METHODDEF    \
    {"window", (PyCFunction)_curses_form_form_window, METH_NOARGS, _curses_form_form_window__doc__},

static PyObject *
_curses_form_form_window_impl(PyCursesFormObject *self);

static PyObject *
_curses_form_form_window(PyCursesFormObject *self, PyObject *Py_UNUSED(ignored))
{
    return _curses_form_form_window_impl(self);
}

PyDoc_STRVAR(_curses_form_form_replace__doc__,
"replace($self, win, /)\n"
"--\n"
"\n"
"Change the window associated with the form to the window win.");

#define _CURSES_FORM_FORM_REPLACE_METHODDEF    \
    {"replace", _PyCFunction_CAST(_curses_form_form_replace), METH_METHOD|METH_FASTCALL|METH_KEYWORDS, _curses_form_form_replace__doc__},

static PyObject *
_curses_form_form_replace_impl(PyCursesFormObject *self, PyTypeObject *cls,
                               PyCursesWindowObject *win);

static PyObject *
_curses_form_form_replace(PyCursesFormObject *self, PyTypeObject *cls, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    #if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)
    #  define KWTUPLE (PyObject *)&_Py_SINGLETON(tuple_empty)
    #else
    #  define KWTUPLE NULL
    #endif

    static const char * const _keywords[] = {"", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "replace",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[1];
    PyCursesWindowObject *win;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser, 1, 1, 0, argsbuf);
    if (!args) {
        goto exit;
    }
    if (!PyObject_TypeCheck(args[0], &PyCursesWindow_Type)) {
        _PyArg_BadArgument("replace", "argument 1", (&PyCursesWindow_Type)->tp_name, args[0]);
        goto exit;
    }
    win = (PyCursesWindowObject *)args[0];
    return_value = _curses_form_form_replace_impl(self, cls, win);

exit:
    return return_value;
}

PyDoc_STRVAR(_curses_form_form_set_userptr__doc__,
"set_userptr($self, obj, /)\n"
"--\n"
"\n"
"Set the form\'s user pointer to obj.");

#define _CURSES_FORM_FORM_SET_USERPTR_METHODDEF    \
    {"set_userptr", _PyCFunction_CAST(_curses_form_form_set_userptr), METH_METHOD|METH_FASTCALL|METH_KEYWORDS, _curses_form_form_set_userptr__doc__},

static PyObject *
_curses_form_form_set_userptr_impl(PyCursesFormObject *self,
                                   PyTypeObject *cls, PyObject *obj);

static PyObject *
_curses_form_form_set_userptr(PyCursesFormObject *self, PyTypeObject *cls, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    #if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)
    #  define KWTUPLE (PyObject *)&_Py_SINGLETON(tuple_empty)
    #else
    #  define KWTUPLE NULL
    #endif

    static const char * const _keywords[] = {"", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "set_userptr",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[1];
    PyObject *obj;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser, 1, 1, 0, argsbuf);
    if (!args) {
        goto exit;
    }
    obj = args[0];
    return_value = _curses_form_form_set_userptr_impl(self, cls, obj);

exit:
    return return_value;
}

PyDoc_STRVAR(_curses_form_form_userptr__doc__,
"userptr($self, /)\n"
"--\n"
"\n"
"Return the user pointer for the form.");

#define _CURSES_FORM_FORM_USERPTR_METHODDEF    \
    {"userptr", _PyCFunction_CAST(_curses_form_form_userptr), METH_METHOD|METH_FASTCALL|METH_KEYWORDS, _curses_form_form_userptr__doc__},

static PyObject *
_curses_form_form_userptr_impl(PyCursesFormObject *self, PyTypeObject *cls);

static PyObject *
_curses_form_form_userptr(PyCursesFormObject *self, PyTypeObject *cls, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    if (nargs) {
        PyErr_SetString(PyExc_TypeError, "userptr() takes no arguments");
        return NULL;
    }
    return _curses_form_form_userptr_impl(self, cls);
}

PyDoc_STRVAR(_curses_form_field_set_new_page__doc__,
"set_new_page($self, new_page, /)\n"
"--\n"
"\n"
"Mark this field as the start of a new form page.");

#define _CURSES_FORM_FIELD_SET_NEW_PAGE_METHODDEF    \
    {"set_new_page", _PyCFunction_CAST(_curses_form_field_set_new_page), METH_METHOD|METH_FASTCALL|METH_KEYWORDS, _curses_form_field_set_new_page__doc__},

static PyObject *
_curses_form_field_set_new_page_impl(PyCursesFieldObject *self,
                                     PyTypeObject *cls, int new_page);

static PyObject *
_curses_form_field_set_new_page(PyCursesFieldObject *self, PyTypeObject *cls, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    #if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)
    #  define KWTUPLE (PyObject *)&_Py_SINGLETON(tuple_empty)
    #else
    #  define KWTUPLE NULL
    #endif

    static const char * const _keywords[] = {"", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "set_new_page",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[1];
    int new_page;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser, 1, 1, 0, argsbuf);
    if (!args) {
        goto exit;
    }
    new_page = PyObject_IsTrue(args[0]);
    if (new_page < 0) {
        goto exit;
    }
    return_value = _curses_form_field_set_new_page_impl(self, cls, new_page);

exit:
    return return_value;
}

PyDoc_STRVAR(_curses_form_field_new_page__doc__,
"new_page($self, /)\n"
"--\n"
"\n"
"Return True if this field starts a new form page.");

#define _CURSES_FORM_FIELD_NEW_PAGE_METHODDEF    \
    {"new_page", (PyCFunction)_curses_form_field_new_page, METH_NOARGS, _curses_form_field_new_page__doc__},

static PyObject *
_curses_form_field_new_page_impl(PyCursesFieldObject *self);

static PyObject *
_curses_form_field_new_page(PyCursesFieldObject *self, PyObject *Py_UNUSED(ignored))
{
    return _curses_form_field_new_page_impl(self);
}

PyDoc_STRVAR(_curses_form_new_field__doc__,
"new_field($module, rows, cols, frow, fcol, offrow, offcol, /)\n"
"--\n"
"\n"
"Create a form field.");

#define _CURSES_FORM_NEW_FIELD_METHODDEF    \
    {"new_field", _PyCFunction_CAST(_curses_form_new_field), METH_FASTCALL, _curses_form_new_field__doc__},

static PyObject *
_curses_form_new_field_impl(PyObject *module, int rows, int cols, int frow,
                            int fcol, int offrow, int offcol);

static PyObject *
_curses_form_new_field(PyObject *module, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    int rows;
    int cols;
    int frow;
    int fcol;
    int offrow;
    int offcol;

    if (!_PyArg_CheckPositional("new_field", nargs, 6, 6)) {
        goto exit;
    }
    rows = PyLong_AsInt(args[0]);
    if (rows == -1 && PyErr_Occurred()) {
        goto exit;
    }
    cols = PyLong_AsInt(args[1]);
    if (cols == -1 && PyErr_Occurred()) {
        goto exit;
    }
    frow = PyLong_AsInt(args[2]);
    if (frow == -1 && PyErr_Occurred()) {
        goto exit;
    }
    fcol = PyLong_AsInt(args[3]);
    if (fcol == -1 && PyErr_Occurred()) {
        goto exit;
    }
    offrow = PyLong_AsInt(args[4]);
    if (offrow == -1 && PyErr_Occurred()) {
        goto exit;
    }
    offcol = PyLong_AsInt(args[5]);
    if (offcol == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = _curses_form_new_field_impl(module, rows, cols, frow, fcol, offrow, offcol);

exit:
    return return_value;
}

PyDoc_STRVAR(_curses_form_new_form__doc__,
"new_form($module, fields, win, /)\n"
"--\n"
"\n"
"Create a form from field objects and associate it with window win.");

#define _CURSES_FORM_NEW_FORM_METHODDEF    \
    {"new_form", _PyCFunction_CAST(_curses_form_new_form), METH_FASTCALL, _curses_form_new_form__doc__},

static PyObject *
_curses_form_new_form_impl(PyObject *module, PyObject *fields,
                           PyCursesWindowObject *win);

static PyObject *
_curses_form_new_form(PyObject *module, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    PyObject *fields;
    PyCursesWindowObject *win;

    if (!_PyArg_CheckPositional("new_form", nargs, 2, 2)) {
        goto exit;
    }
    fields = args[0];
    if (!PyObject_TypeCheck(args[1], &PyCursesWindow_Type)) {
        _PyArg_BadArgument("new_form", "argument 2", (&PyCursesWindow_Type)->tp_name, args[1]);
        goto exit;
    }
    win = (PyCursesWindowObject *)args[1];
    return_value = _curses_form_new_form_impl(module, fields, win);

exit:
    return return_value;
}
/*[clinic end generated code: output=1510bc3a04b28fbb input=a9049054013a1b77]*/
