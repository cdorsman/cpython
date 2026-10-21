/*
 *   Interface to the ncurses form library
 *
 * Original version by Thomas Gellekum
 */

/* Release Number */

static const char PyCursesVersion[] = "0.1";

/* Includes */

#include "Python.h"

#include "py_curses.h"

#include <form.h>

typedef struct {
    PyObject *PyCursesError;
    PyTypeObject *PyCursesForm_Type;
    PyTypeObject *PyCursesField_Type;
} _curses_form_state;

static inline _curses_form_state *
get_curses_form_state(PyObject *module)
{
    void *state = PyModule_GetState(module);
    assert(state != NULL);
    return (_curses_form_state *)state;
}

static int
_curses_form_clear(PyObject *mod)
{
    _curses_form_state *state = get_curses_form_state(mod);
    Py_CLEAR(state->PyCursesError);
    Py_CLEAR(state->PyCursesForm_Type);
    Py_CLEAR(state->PyCursesField_Type);
    return 0;
}

static int
_curses_form_traverse(PyObject *mod, visitproc visit, void *arg)
{
    Py_VISIT(Py_TYPE(mod));
    _curses_form_state *state = get_curses_form_state(mod);
    Py_VISIT(state->PyCursesError);
    Py_VISIT(state->PyCursesForm_Type);
    Py_VISIT(state->PyCursesField_Type);
    return 0;
}

static void
_curses_form_free(void *mod)
{
    _curses_form_clear((PyObject *) mod);
}

/* Utility Functions */

/*
 * Check the return code from a curses function and return None
 * or raise an exception as appropriate.
 */

static PyObject *
PyCursesCheckERR(_curses_form_state *state, int code, const char *fname)
{
    if (code != ERR) {
        Py_RETURN_NONE;
    }
    else {
        if (fname == NULL) {
            PyErr_SetString(state->PyCursesError, catchall_ERR);
        }
        else {
            PyErr_Format(state->PyCursesError, "%s() returned ERR", fname);
        }
        return NULL;
    }
}

/*****************************************************************************
 The form Object
******************************************************************************/

/* Definition of the form object and form type */

typedef struct {
    PyObject_HEAD
    FORM *form;
    PyCursesWindowObject *wo;   /* for reference counts */
    PyObject *fields;            /* references to the form's field objects */
} PyCursesFormObject;

typedef struct {
    PyObject_HEAD
    FIELD *field;
} PyCursesFieldObject;

/* Some helper functions. The problem is that there's always a window
   associated with a form. To ensure that Python's GC doesn't pull
   this window from under our feet we need to keep track of references
   to the corresponding window object within Python. We can't use
   dupwin(oldwin) to keep a copy of the curses WINDOW because the
   contents of oldwin is copied only once; code like

   win = newwin(...)
   pan = new_form(win)
   win.addstr(some_string)
   pan.window().addstr(other_string)

   will fail. */

/* We keep a linked list of PyCursesFormObjects, lop. A list should
   suffice, I don't expect more than a handful or at most a few
   dozens of form objects within a typical program. */
typedef struct _list_of_forms {
    PyCursesFormObject *fo;
    struct _list_of_forms *next;
} list_of_forms;

/* list anchor */
static list_of_forms *lof;

/* Insert a new form object into lop */
static int
insert_lof(PyCursesFormObject *fo)
{
    list_of_forms *new;

    if ((new = (list_of_forms *)PyMem_Malloc(sizeof(list_of_forms))) == NULL) {
        PyErr_NoMemory();
        return -1;
    }
    new->fo = fo;
    new->next = lof;
    lof = new;
    return 0;
}

/* Remove the form object from lop */
static void
remove_lof(PyCursesFormObject *fo)
{
    list_of_forms *temp, *n;

    temp = lof;
    if (temp->fo == fo) {
        lof = temp->next;
        PyMem_Free(temp);
        return;
    }
    while (temp->next == NULL || temp->next->fo != fo) {
        if (temp->next == NULL) {
            PyErr_SetString(PyExc_RuntimeError,
                            "remove_lof: can't find form Object");
            return;
        }
        temp = temp->next;
    }
    n = temp->next->next;
    PyMem_Free(temp->next);
    temp->next = n;
    return;
}

/*[clinic input]
module _curses_form
class _curses_form.form "PyCursesFormObject *" "&PyCursesForm_Type"
class _curses_form.field "PyCursesFieldObject *" "&PyCursesField_Type"
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=3944205d14b113e8]*/

#include "clinic/_curses_form.c.h"

/* ------------- form routines --------------- */

/*[clinic input]
_curses_form.form.driver

    request: int
    /

Process one form request.
[clinic start generated code]*/

static PyObject *
_curses_form_form_driver_impl(PyCursesFormObject *self, int request)
/*[clinic end generated code: output=59a4e3f25455a481 input=e5e6bbc7be12289e]*/
{
    int result = form_driver(self->form, request);

    if (result != E_OK) {
        PyErr_SetString(PyExc_RuntimeError, "form_driver() failed");
        return NULL;
    }

    Py_RETURN_NONE;
}

/*[clinic input]
_curses_form.form.hide

    cls: defining_class

Hide the form.

This does not delete the object, it just makes the window on screen invisible.
[clinic start generated code]*/

static PyObject *
_curses_form_form_hide_impl(PyCursesFormObject *self, PyTypeObject *cls)
/*[clinic end generated code: output=f6b0cc5d6532fe9e input=3b625f61fa9435b1]*/
{
    _curses_form_state *state = PyType_GetModuleState(cls);
    return PyCursesCheckERR(state, unpost_form(self->form), "unpost_form");
}

/*[clinic input]
_curses_form.form.show

    cls: defining_class

Display the form (which might have been hidden).
[clinic start generated code]*/

static PyObject *
_curses_form_form_show_impl(PyCursesFormObject *self, PyTypeObject *cls)
/*[clinic end generated code: output=0a12482994b4691f input=422c5b32b671a402]*/
{
    _curses_form_state *state = PyType_GetModuleState(cls);
    return PyCursesCheckERR(state, post_form(self->form), "post_form");
}

/* Allocation and deallocation of form Objects */

static PyObject *
PyCursesForm_New(_curses_form_state *state, FORM *form,
                  PyCursesWindowObject *wo, PyObject *fields)
{
    PyCursesFormObject *fo = PyObject_New(PyCursesFormObject,
                                           state->PyCursesForm_Type);
    if (fo == NULL) {
        return NULL;
    }

    fo->form = form;
    fo->wo = NULL;
    fo->fields = Py_NewRef(fields);
    if (insert_lof(fo) < 0) {
        Py_DECREF(fo);
        return NULL;
    }
    fo->wo = (PyCursesWindowObject*)Py_NewRef(wo);
    return (PyObject *)fo;
}

static void
PyCursesForm_Dealloc(PyCursesFormObject *fo)
{
    PyObject *tp, *obj;

    tp = (PyObject *) Py_TYPE(fo);
    obj = (PyObject *) form_userptr(fo->form);
    if (obj) {
        (void)set_form_userptr(fo->form, NULL);
        Py_DECREF(obj);
    }
    (void)free_form(fo->form);
    if (fo->wo != NULL) {
        Py_DECREF(fo->wo);
        remove_lof(fo);
    }
    Py_DECREF(fo->fields);
    PyObject_Free(fo);
    Py_DECREF(tp);
}

/*[clinic input]
_curses_form.form.window

Return the window object associated with the form.
[clinic start generated code]*/

static PyObject *
_curses_form_form_window_impl(PyCursesFormObject *self)
/*[clinic end generated code: output=69f6d03a0d266a98 input=06e1c6da497f81bb]*/
{
    return Py_NewRef(self->wo);
}

/*[clinic input]
_curses_form.form.replace

    cls: defining_class
    win: object(type="PyCursesWindowObject *", subclass_of="&PyCursesWindow_Type")
    /

Change the window associated with the form to the window win.
[clinic start generated code]*/

static PyObject *
_curses_form_form_replace_impl(PyCursesFormObject *self, PyTypeObject *cls,
                               PyCursesWindowObject *win)
/*[clinic end generated code: output=7c1c1240cdd2300c input=222017a31c704536]*/
{
    _curses_form_state *state = PyType_GetModuleState(cls);

    int rtn = set_form_win(self->form, win->win);
    if (rtn != E_OK) {
        PyErr_SetString(state->PyCursesError, "set_form_win() failed");
        return NULL;
    }
    Py_SETREF(self->wo, (PyCursesWindowObject*)Py_NewRef(win));
    Py_RETURN_NONE;
}

/*[clinic input]
_curses_form.form.set_userptr

    cls: defining_class
    obj: object
    /

Set the form's user pointer to obj.
[clinic start generated code]*/

static PyObject *
_curses_form_form_set_userptr_impl(PyCursesFormObject *self,
                                   PyTypeObject *cls, PyObject *obj)
/*[clinic end generated code: output=c5331e4d470224ce input=44451e974a71bb14]*/
{
    PyCursesInitialised;
    Py_INCREF(obj);
    PyObject *oldobj = (PyObject *) form_userptr(self->form);
    int rc = set_form_userptr(self->form, (void*)obj);
    if (rc == ERR) {
        /* In case of an ncurses error, decref the new object again */
        Py_DECREF(obj);
    }
    else {
        Py_XDECREF(oldobj);
    }

    _curses_form_state *state = PyType_GetModuleState(cls);
    return PyCursesCheckERR(state, rc, "set_form_userptr");
}

/*[clinic input]
_curses_form.form.userptr

    cls: defining_class

Return the user pointer for the form.
[clinic start generated code]*/

static PyObject *
_curses_form_form_userptr_impl(PyCursesFormObject *self, PyTypeObject *cls)
/*[clinic end generated code: output=68045d893830da6d input=15cf6e5f55dc19f0]*/
{
    _curses_form_state *state = PyType_GetModuleState(cls);

    PyCursesInitialised;
    PyObject *obj = (PyObject *) form_userptr(self->form);
    if (obj == NULL) {
        PyErr_SetString(state->PyCursesError, "no userptr set");
        return NULL;
    }

    return Py_NewRef(obj);
}


/* Module interface */

static PyMethodDef PyCursesForm_Methods[] = {
    _CURSES_FORM_FORM_DRIVER_METHODDEF
    _CURSES_FORM_FORM_HIDE_METHODDEF
    _CURSES_FORM_FORM_REPLACE_METHODDEF
    _CURSES_FORM_FORM_SET_USERPTR_METHODDEF
    _CURSES_FORM_FORM_SHOW_METHODDEF
    _CURSES_FORM_FORM_USERPTR_METHODDEF
    _CURSES_FORM_FORM_WINDOW_METHODDEF
    {NULL,              NULL}   /* sentinel */
};

/* -------------------------------------------------------*/

static PyType_Slot PyCursesForm_Type_slots[] = {
    {Py_tp_dealloc, PyCursesForm_Dealloc},
    {Py_tp_methods, PyCursesForm_Methods},
    {0, 0},
};

static PyType_Spec PyCursesForm_Type_spec = {
    .name = "_curses_form.form",
    .basicsize = sizeof(PyCursesFormObject),
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_DISALLOW_INSTANTIATION,
    .slots = PyCursesForm_Type_slots
};

static void
PyCursesField_Dealloc(PyCursesFieldObject *self)
{
    (void)free_field(self->field);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyMethodDef PyCursesField_Methods[];

static PyType_Slot PyCursesField_Type_slots[] = {
    {Py_tp_dealloc, PyCursesField_Dealloc},
    {Py_tp_methods, PyCursesField_Methods},
    {0, 0},
};

static PyType_Spec PyCursesField_Type_spec = {
    .name = "_curses_form.field",
    .basicsize = sizeof(PyCursesFieldObject),
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_DISALLOW_INSTANTIATION,
    .slots = PyCursesField_Type_slots
};

/*[clinic input]
_curses_form.field.set_new_page

    cls: defining_class
    new_page: bool
    /

Mark this field as the start of a new form page.
[clinic start generated code]*/

static PyObject *
_curses_form_field_set_new_page_impl(PyCursesFieldObject *self,
                                     PyTypeObject *cls, int new_page)
/*[clinic end generated code: output=47156adb500296f6 input=b7b591df4a0423da]*/
{
    _curses_form_state *state = PyType_GetModuleState(cls);
    return PyCursesCheckERR(state, set_new_page(self->field, new_page),
                            "set_new_page");
}

/*[clinic input]
_curses_form.field.new_page

Return True if this field starts a new form page.
[clinic start generated code]*/

static PyObject *
_curses_form_field_new_page_impl(PyCursesFieldObject *self)
/*[clinic end generated code: output=a0a8aa52215c9bc4 input=e9fca7e56c8ddc74]*/
{
    if (new_page(self->field)) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyMethodDef PyCursesField_Methods[] = {
    _CURSES_FORM_FIELD_NEW_PAGE_METHODDEF
    _CURSES_FORM_FIELD_SET_NEW_PAGE_METHODDEF
    {NULL, NULL}
};

/*[clinic input]
_curses_form.new_field

    rows: int
    cols: int
    frow: int
    fcol: int
    offrow: int
    offcol: int
    /

Create a form field.
[clinic start generated code]*/

static PyObject *
_curses_form_new_field_impl(PyObject *module, int rows, int cols, int frow,
                            int fcol, int offrow, int offcol)
/*[clinic end generated code: output=42b75de2c4f6dc16 input=5888ca53fc073536]*/
{
    _curses_form_state *state = get_curses_form_state(module);
    FIELD *field = new_field(rows, cols, frow, fcol, offrow, offcol);
    if (field == NULL) {
        PyErr_SetString(state->PyCursesError, catchall_NULL);
        return NULL;
    }

    PyCursesFieldObject *result = PyObject_New(
        PyCursesFieldObject, state->PyCursesField_Type);
    if (result == NULL) {
        free_field(field);
        return NULL;
    }
    result->field = field;
    return (PyObject *)result;
}


/*[clinic input]
_curses_form.new_form

    fields: object
    win: object(type="PyCursesWindowObject *", subclass_of="&PyCursesWindow_Type")
    /

Create a form from field objects and associate it with window win.
[clinic start generated code]*/

static PyObject *
_curses_form_new_form_impl(PyObject *module, PyObject *fields,
                           PyCursesWindowObject *win)
/*[clinic end generated code: output=9e65e89a2d484429 input=39c31b74ffdbf6db]*/
{
    _curses_form_state *state = get_curses_form_state(module);

    PyObject *field_sequence = PySequence_Fast(fields, "fields must be a sequence");
    if (field_sequence == NULL) {
        return NULL;
    }

    Py_ssize_t field_count = PySequence_Fast_GET_SIZE(field_sequence);
    if (field_count > INT_MAX - 1) {
        Py_DECREF(field_sequence);
        PyErr_SetString(PyExc_OverflowError, "too many fields");
        return NULL;
    }

    FIELD **native_fields = PyMem_Malloc(
        ((size_t)field_count + 1) * sizeof(*native_fields));
    if (native_fields == NULL) {
        Py_DECREF(field_sequence);
        return PyErr_NoMemory();
    }

    PyObject **items = PySequence_Fast_ITEMS(field_sequence);
    for (Py_ssize_t i = 0; i < field_count; i++) {
        if (!PyObject_TypeCheck(items[i], state->PyCursesField_Type)) {
            PyMem_Free(native_fields);
            Py_DECREF(field_sequence);
            PyErr_SetString(PyExc_TypeError,
                            "fields must contain _curses_form.field objects");
            return NULL;
        }
        native_fields[i] = ((PyCursesFieldObject *)items[i])->field;
    }
    native_fields[field_count] = NULL;

    FORM *form = new_form(native_fields);
    PyMem_Free(native_fields);
    if (form == NULL) {
        Py_DECREF(field_sequence);
        PyErr_SetString(state->PyCursesError, catchall_NULL);
        return NULL;
    }
    PyObject *result = PyCursesForm_New(state, form, win, field_sequence);
    Py_DECREF(field_sequence);
    return result;
}

/* List of functions defined in the module */
static PyMethodDef PyCurses_methods[] = {
    _CURSES_FORM_NEW_FIELD_METHODDEF
    _CURSES_FORM_NEW_FORM_METHODDEF
    {NULL,              NULL}           /* sentinel */
};

static const struct {
    const char *name;
    int value;
} form_requests[] = {
    {"REQ_NEXT_PAGE", REQ_NEXT_PAGE},
    {"REQ_PREV_PAGE", REQ_PREV_PAGE},
    {"REQ_FIRST_PAGE", REQ_FIRST_PAGE},
    {"REQ_LAST_PAGE", REQ_LAST_PAGE},
    {"REQ_NEXT_FIELD", REQ_NEXT_FIELD},
    {"REQ_PREV_FIELD", REQ_PREV_FIELD},
    {"REQ_FIRST_FIELD", REQ_FIRST_FIELD},
    {"REQ_LAST_FIELD", REQ_LAST_FIELD},
    {"REQ_SNEXT_FIELD", REQ_SNEXT_FIELD},
    {"REQ_SPREV_FIELD", REQ_SPREV_FIELD},
    {"REQ_SFIRST_FIELD", REQ_SFIRST_FIELD},
    {"REQ_SLAST_FIELD", REQ_SLAST_FIELD},
    {"REQ_LEFT_FIELD", REQ_LEFT_FIELD},
    {"REQ_RIGHT_FIELD", REQ_RIGHT_FIELD},
    {"REQ_UP_FIELD", REQ_UP_FIELD},
    {"REQ_DOWN_FIELD", REQ_DOWN_FIELD},
    {"REQ_NEXT_CHAR", REQ_NEXT_CHAR},
    {"REQ_PREV_CHAR", REQ_PREV_CHAR},
    {"REQ_NEXT_LINE", REQ_NEXT_LINE},
    {"REQ_PREV_LINE", REQ_PREV_LINE},
    {"REQ_NEXT_WORD", REQ_NEXT_WORD},
    {"REQ_PREV_WORD", REQ_PREV_WORD},
    {"REQ_BEG_FIELD", REQ_BEG_FIELD},
    {"REQ_END_FIELD", REQ_END_FIELD},
    {"REQ_BEG_LINE", REQ_BEG_LINE},
    {"REQ_END_LINE", REQ_END_LINE},
    {"REQ_LEFT_CHAR", REQ_LEFT_CHAR},
    {"REQ_RIGHT_CHAR", REQ_RIGHT_CHAR},
    {"REQ_UP_CHAR", REQ_UP_CHAR},
    {"REQ_DOWN_CHAR", REQ_DOWN_CHAR},
    {"REQ_NEW_LINE", REQ_NEW_LINE},
    {"REQ_INS_CHAR", REQ_INS_CHAR},
    {"REQ_INS_LINE", REQ_INS_LINE},
    {"REQ_DEL_CHAR", REQ_DEL_CHAR},
    {"REQ_DEL_PREV", REQ_DEL_PREV},
    {"REQ_DEL_LINE", REQ_DEL_LINE},
    {"REQ_DEL_WORD", REQ_DEL_WORD},
    {"REQ_CLR_EOL", REQ_CLR_EOL},
    {"REQ_CLR_EOF", REQ_CLR_EOF},
    {"REQ_CLR_FIELD", REQ_CLR_FIELD},
    {"REQ_OVL_MODE", REQ_OVL_MODE},
    {"REQ_INS_MODE", REQ_INS_MODE},
    {"REQ_SCR_FLINE", REQ_SCR_FLINE},
    {"REQ_SCR_BLINE", REQ_SCR_BLINE},
    {"REQ_SCR_FPAGE", REQ_SCR_FPAGE},
    {"REQ_SCR_BPAGE", REQ_SCR_BPAGE},
    {"REQ_SCR_FHPAGE", REQ_SCR_FHPAGE},
    {"REQ_SCR_BHPAGE", REQ_SCR_BHPAGE},
    {"REQ_SCR_FCHAR", REQ_SCR_FCHAR},
    {"REQ_SCR_BCHAR", REQ_SCR_BCHAR},
    {"REQ_SCR_HFLINE", REQ_SCR_HFLINE},
    {"REQ_SCR_HBLINE", REQ_SCR_HBLINE},
    {"REQ_SCR_HFHALF", REQ_SCR_HFHALF},
    {"REQ_SCR_HBHALF", REQ_SCR_HBHALF},
    {"REQ_VALIDATION", REQ_VALIDATION},
    {"REQ_NEXT_CHOICE", REQ_NEXT_CHOICE},
    {"REQ_PREV_CHOICE", REQ_PREV_CHOICE},
};

/* Initialization function for the module */
static int
_curses_form_exec(PyObject *mod)
{
    _curses_form_state *state = get_curses_form_state(mod);
    /* Initialize object type */
    state->PyCursesForm_Type = (PyTypeObject *)PyType_FromModuleAndSpec(
        mod, &PyCursesForm_Type_spec, NULL);
    if (state->PyCursesForm_Type == NULL) {
        return -1;
    }

    if (PyModule_AddType(mod, state->PyCursesForm_Type) < 0) {
        return -1;
    }

    state->PyCursesField_Type = (PyTypeObject *)PyType_FromModuleAndSpec(
        mod, &PyCursesField_Type_spec, NULL);
    if (state->PyCursesField_Type == NULL) {
        return -1;
    }

    if (PyModule_AddType(mod, state->PyCursesField_Type) < 0) {
        return -1;
    }

    for (size_t i = 0; i < Py_ARRAY_LENGTH(form_requests); i++) {
        if (PyModule_AddIntConstant(mod, form_requests[i].name,
                                    form_requests[i].value) < 0) {
            return -1;
        }
    }

    import_curses();
    if (PyErr_Occurred()) {
        return -1;
    }

    /* For exception _curses_form.error */
    state->PyCursesError = PyErr_NewException(
        "_curses_form.error", NULL, NULL);

    if (PyModule_AddObjectRef(mod, "error", state->PyCursesError) < 0) {
        return -1;
    }

    /* Make the version available */
    PyObject *v = PyUnicode_FromString(PyCursesVersion);
    if (v == NULL) {
        return -1;
    }

    PyObject *d = PyModule_GetDict(mod);
    if (PyDict_SetItemString(d, "version", v) < 0) {
        Py_DECREF(v);
        return -1;
    }
    if (PyDict_SetItemString(d, "__version__", v) < 0) {
        Py_DECREF(v);
        return -1;
    }

    Py_DECREF(v);

    return 0;
}

static PyModuleDef_Slot _curses_slots[] = {
    {Py_mod_exec, _curses_form_exec},
    // XXX gh-103092: fix isolation.
    {Py_mod_multiple_interpreters, Py_MOD_MULTIPLE_INTERPRETERS_NOT_SUPPORTED},
    //{Py_mod_multiple_interpreters, Py_MOD_PER_INTERPRETER_GIL_SUPPORTED},
    {0, NULL}
};

/* Module definition */
static struct PyModuleDef _curses_formmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "_curses_form",
    .m_size = sizeof(_curses_form_state),
    .m_methods = PyCurses_methods,
    .m_slots = _curses_slots,
    .m_traverse = _curses_form_traverse,
    .m_clear = _curses_form_clear,
    .m_free = _curses_form_free
};

/* Initiate module */
PyMODINIT_FUNC
PyInit__curses_form(void)
{
    return PyModuleDef_Init(&_curses_formmodule);
}
