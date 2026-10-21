/* Interface to the ncurses menu library. */

#include "Python.h"
#include "py_curses.h"

#include <menu.h>

static const char PyCursesVersion[] = "0.1";

typedef struct {
    PyObject *PyCursesError;
    PyTypeObject *PyCursesMenu_Type;
    PyTypeObject *PyCursesItem_Type;
} _curses_menu_state;

typedef struct {
    PyObject_HEAD
    MENU *menu;
    PyObject *items;
    PyObject *win;
    PyObject *sub;
    PyObject *userptr;
} PyCursesMenuObject;

typedef struct {
    PyObject_HEAD
    ITEM *item;
    PyObject *userptr;
} PyCursesItemObject;

static inline _curses_menu_state *
get_curses_menu_state(PyObject *module)
{
    return (_curses_menu_state *)PyModule_GetState(module);
}

static PyObject *
PyCursesMenuCheckERR(_curses_menu_state *state, int code, const char *name)
{
    if (code == E_OK) {
        Py_RETURN_NONE;
    }
    PyErr_Format(state->PyCursesError, "%s() returned error %d", name, code);
    return NULL;
}

static PyCursesItemObject *
find_item_object(PyCursesMenuObject *self, ITEM *item)
{
    Py_ssize_t count = PyTuple_GET_SIZE(self->items);
    for (Py_ssize_t i = 0; i < count; i++) {
        PyCursesItemObject *candidate =
            (PyCursesItemObject *)PyTuple_GET_ITEM(self->items, i);
        if (candidate->item == item) {
            return candidate;
        }
    }
    return NULL;
}

static void
PyCursesItem_Dealloc(PyCursesItemObject *self)
{
    Py_CLEAR(self->userptr);
    (void)free_item(self->item);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static void
PyCursesMenu_Dealloc(PyCursesMenuObject *self)
{
    Py_CLEAR(self->userptr);
    if (self->menu != NULL) {
        (void)unpost_menu(self->menu);
        (void)set_menu_items(self->menu, NULL);
        (void)free_menu(self->menu);
        self->menu = NULL;
    }
    Py_CLEAR(self->items);
    Py_CLEAR(self->win);
    Py_CLEAR(self->sub);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *
py_menu_driver(PyCursesMenuObject *self, PyObject *arg)
{
    int request;
    if (!PyArg_ParseTuple(arg, "i:driver", &request)) {
        return NULL;
    }
    int result = menu_driver(self->menu, request);
    if (result == E_OK || result == E_UNKNOWN_COMMAND ||
        result == E_REQUEST_DENIED || result == E_NO_MATCH) {
        Py_RETURN_NONE;
    }
    _curses_menu_state *state = PyType_GetModuleState(Py_TYPE(self));
    return PyCursesMenuCheckERR(state, result, "menu_driver");
}

static PyObject *
py_menu_show(PyCursesMenuObject *self, PyObject *Py_UNUSED(ignored))
{
    _curses_menu_state *state = PyType_GetModuleState(Py_TYPE(self));
    return PyCursesMenuCheckERR(state, post_menu(self->menu), "post_menu");
}

static PyObject *
py_menu_hide(PyCursesMenuObject *self, PyObject *Py_UNUSED(ignored))
{
    _curses_menu_state *state = PyType_GetModuleState(Py_TYPE(self));
    return PyCursesMenuCheckERR(state, unpost_menu(self->menu), "unpost_menu");
}

static PyObject *
py_menu_window(PyCursesMenuObject *self, PyObject *Py_UNUSED(ignored))
{
    if (self->win == NULL) {
        Py_RETURN_NONE;
    }
    return Py_NewRef(self->win);
}

static PyObject *
py_menu_replace(PyCursesMenuObject *self, PyObject *arg)
{
    PyCursesWindowObject *win;
    if (!PyArg_ParseTuple(arg, "O!:replace", &PyCursesWindow_Type, &win)) {
        return NULL;
    }
    _curses_menu_state *state = PyType_GetModuleState(Py_TYPE(self));
    int result = set_menu_win(self->menu, win->win);
    if (result != E_OK) {
        return PyCursesMenuCheckERR(state, result, "set_menu_win");
    }
    Py_XSETREF(self->win, Py_NewRef(win));
    Py_RETURN_NONE;
}

static PyObject *
py_menu_set_sub(PyCursesMenuObject *self, PyObject *arg)
{
    PyCursesWindowObject *sub;
    if (!PyArg_ParseTuple(arg, "O!:set_sub", &PyCursesWindow_Type, &sub)) {
        return NULL;
    }
    _curses_menu_state *state = PyType_GetModuleState(Py_TYPE(self));
    int result = set_menu_sub(self->menu, sub->win);
    if (result != E_OK) {
        return PyCursesMenuCheckERR(state, result, "set_menu_sub");
    }
    Py_XSETREF(self->sub, Py_NewRef(sub));
    Py_RETURN_NONE;
}

static PyObject *
py_menu_items(PyCursesMenuObject *self, PyObject *Py_UNUSED(ignored))
{
    return Py_NewRef(self->items);
}

static PyObject *
py_menu_current_item(PyCursesMenuObject *self, PyObject *Py_UNUSED(ignored))
{
    PyCursesItemObject *item = find_item_object(self, current_item(self->menu));
    if (item == NULL) {
        Py_RETURN_NONE;
    }
    return Py_NewRef(item);
}

static PyObject *
py_menu_set_current_item(PyCursesMenuObject *self, PyObject *arg)
{
    PyObject *object;
    if (!PyArg_ParseTuple(arg, "O:set_current_item", &object)) {
        return NULL;
    }
    _curses_menu_state *state = PyType_GetModuleState(Py_TYPE(self));
    if (!PyObject_TypeCheck(object, state->PyCursesItem_Type)) {
        PyErr_SetString(PyExc_TypeError, "item must be a menu item");
        return NULL;
    }
    PyCursesItemObject *item = (PyCursesItemObject *)object;
    if (find_item_object(self, item->item) == NULL) {
        PyErr_SetString(PyExc_ValueError, "item does not belong to this menu");
        return NULL;
    }
    return PyCursesMenuCheckERR(state, set_current_item(self->menu, item->item),
                                "set_current_item");
}

static PyObject *
py_menu_item_count(PyCursesMenuObject *self, PyObject *Py_UNUSED(ignored))
{
    return PyLong_FromLong(item_count(self->menu));
}

static PyObject *
py_menu_set_userptr(PyCursesMenuObject *self, PyObject *arg)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(arg, "O:set_userptr", &obj)) {
        return NULL;
    }
    Py_INCREF(obj);
    Py_XSETREF(self->userptr, obj);
    (void)set_menu_userptr(self->menu, obj);
    Py_RETURN_NONE;
}

static PyObject *
py_menu_userptr(PyCursesMenuObject *self, PyObject *Py_UNUSED(ignored))
{
    if (self->userptr == NULL) {
        Py_RETURN_NONE;
    }
    return Py_NewRef(self->userptr);
}

static PyObject *
py_menu_set_mark(PyCursesMenuObject *self, PyObject *arg)
{
    const char *mark;
    if (!PyArg_ParseTuple(arg, "s:set_mark", &mark)) {
        return NULL;
    }
    _curses_menu_state *state = PyType_GetModuleState(Py_TYPE(self));
    return PyCursesMenuCheckERR(state, set_menu_mark(self->menu, mark),
                                "set_menu_mark");
}

static PyObject *
py_menu_mark(PyCursesMenuObject *self, PyObject *Py_UNUSED(ignored))
{
    return PyUnicode_FromString(menu_mark(self->menu));
}

static PyObject *
py_menu_set_format(PyCursesMenuObject *self, PyObject *arg)
{
    int rows, cols;
    if (!PyArg_ParseTuple(arg, "ii:set_format", &rows, &cols)) {
        return NULL;
    }
    set_menu_format(self->menu, rows, cols);
    Py_RETURN_NONE;
}

static PyObject *
py_menu_format(PyCursesMenuObject *self, PyObject *Py_UNUSED(ignored))
{
    int rows, cols;
    menu_format(self->menu, &rows, &cols);
    return Py_BuildValue("ii", rows, cols);
}

static PyObject *
py_menu_options(PyCursesMenuObject *self, PyObject *Py_UNUSED(ignored))
{
    return PyLong_FromUnsignedLong((unsigned long)menu_opts(self->menu));
}

static PyObject *
py_menu_set_options(PyCursesMenuObject *self, PyObject *arg)
{
    unsigned long options;
    if (!PyArg_ParseTuple(arg, "k:set_options", &options)) {
        return NULL;
    }
    set_menu_opts(self->menu, (Menu_Options)options);
    Py_RETURN_NONE;
}

static PyObject *
py_menu_options_on(PyCursesMenuObject *self, PyObject *arg)
{
    unsigned long options;
    if (!PyArg_ParseTuple(arg, "k:options_on", &options)) {
        return NULL;
    }
    _curses_menu_state *state = PyType_GetModuleState(Py_TYPE(self));
    return PyCursesMenuCheckERR(state, menu_opts_on(self->menu,
                                                    (Menu_Options)options),
                                "menu_opts_on");
}

static PyObject *
py_menu_options_off(PyCursesMenuObject *self, PyObject *arg)
{
    unsigned long options;
    if (!PyArg_ParseTuple(arg, "k:options_off", &options)) {
        return NULL;
    }
    _curses_menu_state *state = PyType_GetModuleState(Py_TYPE(self));
    return PyCursesMenuCheckERR(state, menu_opts_off(self->menu,
                                                     (Menu_Options)options),
                                "menu_opts_off");
}

static PyObject *
py_item_name(PyCursesItemObject *self, PyObject *Py_UNUSED(ignored))
{
    return PyUnicode_FromString(item_name(self->item));
}

static PyObject *
py_item_description(PyCursesItemObject *self, PyObject *Py_UNUSED(ignored))
{
    return PyUnicode_FromString(item_description(self->item));
}

static PyObject *
py_item_index(PyCursesItemObject *self, PyObject *Py_UNUSED(ignored))
{
    return PyLong_FromLong(item_index(self->item));
}

static PyObject *
py_item_options(PyCursesItemObject *self, PyObject *Py_UNUSED(ignored))
{
    return PyLong_FromUnsignedLong((unsigned long)item_opts(self->item));
}

static PyObject *
py_item_set_options(PyCursesItemObject *self, PyObject *arg)
{
    unsigned long options;
    if (!PyArg_ParseTuple(arg, "k:set_options", &options)) {
        return NULL;
    }
    set_item_opts(self->item, (Item_Options)options);
    Py_RETURN_NONE;
}

static PyObject *
py_item_options_on(PyCursesItemObject *self, PyObject *arg)
{
    unsigned long options;
    if (!PyArg_ParseTuple(arg, "k:options_on", &options)) {
        return NULL;
    }
    _curses_menu_state *state = PyType_GetModuleState(Py_TYPE(self));
    return PyCursesMenuCheckERR(state, item_opts_on(self->item,
                                                    (Item_Options)options),
                                "item_opts_on");
}

static PyObject *
py_item_options_off(PyCursesItemObject *self, PyObject *arg)
{
    unsigned long options;
    if (!PyArg_ParseTuple(arg, "k:options_off", &options)) {
        return NULL;
    }
    _curses_menu_state *state = PyType_GetModuleState(Py_TYPE(self));
    return PyCursesMenuCheckERR(state, item_opts_off(self->item,
                                                     (Item_Options)options),
                                "item_opts_off");
}

static PyObject *
py_item_set_userptr(PyCursesItemObject *self, PyObject *arg)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(arg, "O:set_userptr", &obj)) {
        return NULL;
    }
    Py_INCREF(obj);
    Py_XSETREF(self->userptr, obj);
    (void)set_item_userptr(self->item, obj);
    Py_RETURN_NONE;
}

static PyObject *
py_item_userptr(PyCursesItemObject *self, PyObject *Py_UNUSED(ignored))
{
    if (self->userptr == NULL) {
        Py_RETURN_NONE;
    }
    return Py_NewRef(self->userptr);
}

static PyMethodDef PyCursesMenu_Methods[] = {
    {"driver", (PyCFunction)py_menu_driver, METH_VARARGS, "Process a menu request."},
    {"show", (PyCFunction)py_menu_show, METH_NOARGS, "Post the menu."},
    {"hide", (PyCFunction)py_menu_hide, METH_NOARGS, "Unpost the menu."},
    {"window", (PyCFunction)py_menu_window, METH_NOARGS, "Return the menu window."},
    {"replace", (PyCFunction)py_menu_replace, METH_VARARGS, "Set the menu window."},
    {"set_sub", (PyCFunction)py_menu_set_sub, METH_VARARGS, "Set the menu subwindow."},
    {"items", (PyCFunction)py_menu_items, METH_NOARGS, "Return the menu items."},
    {"current_item", (PyCFunction)py_menu_current_item, METH_NOARGS, "Return the current item."},
    {"set_current_item", (PyCFunction)py_menu_set_current_item, METH_VARARGS, "Set the current item."},
    {"item_count", (PyCFunction)py_menu_item_count, METH_NOARGS, "Return the item count."},
    {"set_userptr", (PyCFunction)py_menu_set_userptr, METH_VARARGS, "Set the user pointer."},
    {"userptr", (PyCFunction)py_menu_userptr, METH_NOARGS, "Return the user pointer."},
    {"set_mark", (PyCFunction)py_menu_set_mark, METH_VARARGS, "Set the menu mark."},
    {"mark", (PyCFunction)py_menu_mark, METH_NOARGS, "Return the menu mark."},
    {"set_format", (PyCFunction)py_menu_set_format, METH_VARARGS, "Set the menu format."},
    {"format", (PyCFunction)py_menu_format, METH_NOARGS, "Return the menu format."},
    {"options", (PyCFunction)py_menu_options, METH_NOARGS, "Return menu options."},
    {"set_options", (PyCFunction)py_menu_set_options, METH_VARARGS, "Set menu options."},
    {"options_on", (PyCFunction)py_menu_options_on, METH_VARARGS, "Enable menu options."},
    {"options_off", (PyCFunction)py_menu_options_off, METH_VARARGS, "Disable menu options."},
    {NULL, NULL}
};

static PyMethodDef PyCursesItem_Methods[] = {
    {"name", (PyCFunction)py_item_name, METH_NOARGS, "Return the item name."},
    {"description", (PyCFunction)py_item_description, METH_NOARGS, "Return the item description."},
    {"index", (PyCFunction)py_item_index, METH_NOARGS, "Return the item index."},
    {"options", (PyCFunction)py_item_options, METH_NOARGS, "Return item options."},
    {"set_options", (PyCFunction)py_item_set_options, METH_VARARGS, "Set item options."},
    {"options_on", (PyCFunction)py_item_options_on, METH_VARARGS, "Enable item options."},
    {"options_off", (PyCFunction)py_item_options_off, METH_VARARGS, "Disable item options."},
    {"set_userptr", (PyCFunction)py_item_set_userptr, METH_VARARGS, "Set the user pointer."},
    {"userptr", (PyCFunction)py_item_userptr, METH_NOARGS, "Return the user pointer."},
    {NULL, NULL}
};

static PyType_Slot PyCursesMenu_Type_slots[] = {
    {Py_tp_dealloc, PyCursesMenu_Dealloc},
    {Py_tp_methods, PyCursesMenu_Methods},
    {0, 0}
};

static PyType_Spec PyCursesMenu_Type_spec = {
    .name = "_curses_menu.menu",
    .basicsize = sizeof(PyCursesMenuObject),
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_DISALLOW_INSTANTIATION,
    .slots = PyCursesMenu_Type_slots
};

static PyType_Slot PyCursesItem_Type_slots[] = {
    {Py_tp_dealloc, PyCursesItem_Dealloc},
    {Py_tp_methods, PyCursesItem_Methods},
    {0, 0}
};

static PyType_Spec PyCursesItem_Type_spec = {
    .name = "_curses_menu.item",
    .basicsize = sizeof(PyCursesItemObject),
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_DISALLOW_INSTANTIATION,
    .slots = PyCursesItem_Type_slots
};

static PyObject *
new_item_func(PyObject *module, PyObject *args)
{
    const char *name, *description;
    if (!PyArg_ParseTuple(args, "ss:new_item", &name, &description)) {
        return NULL;
    }
    _curses_menu_state *state = get_curses_menu_state(module);
    ITEM *item = new_item(name, description);
    if (item == NULL) {
        PyErr_SetString(state->PyCursesError, "new_item() failed");
        return NULL;
    }
    PyCursesItemObject *result = PyObject_New(PyCursesItemObject,
                                               state->PyCursesItem_Type);
    if (result == NULL) {
        free_item(item);
        return NULL;
    }
    result->item = item;
    result->userptr = NULL;
    return (PyObject *)result;
}

static PyObject *
new_menu_func(PyObject *module, PyObject *args)
{
    PyObject *items, *win_obj;
    if (!PyArg_ParseTuple(args, "OO:new_menu", &items, &win_obj)) {
        return NULL;
    }
    _curses_menu_state *state = get_curses_menu_state(module);
    if (!PyObject_TypeCheck(win_obj, &PyCursesWindow_Type)) {
        PyErr_SetString(PyExc_TypeError, "win must be a curses window");
        return NULL;
    }
    PyObject *sequence = PySequence_Tuple(items);
    if (sequence == NULL) {
        return NULL;
    }
    Py_ssize_t count = PyTuple_GET_SIZE(sequence);
    ITEM **native_items = PyMem_Malloc((size_t)(count + 1) * sizeof(*native_items));
    if (native_items == NULL) {
        Py_DECREF(sequence);
        return PyErr_NoMemory();
    }
    for (Py_ssize_t i = 0; i < count; i++) {
        PyObject *object = PyTuple_GET_ITEM(sequence, i);
        if (!PyObject_TypeCheck(object, state->PyCursesItem_Type)) {
            PyMem_Free(native_items);
            Py_DECREF(sequence);
            PyErr_SetString(PyExc_TypeError,
                            "items must contain _curses_menu.item objects");
            return NULL;
        }
        native_items[i] = ((PyCursesItemObject *)object)->item;
    }
    native_items[count] = NULL;
    MENU *menu = new_menu(native_items);
    PyMem_Free(native_items);
    if (menu == NULL) {
        Py_DECREF(sequence);
        PyErr_SetString(state->PyCursesError, "new_menu() failed");
        return NULL;
    }
    PyCursesWindowObject *win = (PyCursesWindowObject *)win_obj;
    if (set_menu_win(menu, win->win) != E_OK) {
        free_menu(menu);
        Py_DECREF(sequence);
        PyErr_SetString(state->PyCursesError, "set_menu_win() failed");
        return NULL;
    }
    PyCursesMenuObject *result = PyObject_New(PyCursesMenuObject,
                                               state->PyCursesMenu_Type);
    if (result == NULL) {
        free_menu(menu);
        Py_DECREF(sequence);
        return NULL;
    }
    result->menu = menu;
    result->items = sequence;
    result->win = Py_NewRef(win_obj);
    result->sub = NULL;
    result->userptr = NULL;
    return (PyObject *)result;
}

static PyMethodDef PyCurses_methods[] = {
    {"new_item", new_item_func, METH_VARARGS, "Create a menu item."},
    {"new_menu", new_menu_func, METH_VARARGS, "Create a menu."},
    {NULL, NULL}
};

static const struct { const char *name; int value; } menu_constants[] = {
    {"REQ_LEFT_ITEM", REQ_LEFT_ITEM}, {"REQ_RIGHT_ITEM", REQ_RIGHT_ITEM},
    {"REQ_UP_ITEM", REQ_UP_ITEM}, {"REQ_DOWN_ITEM", REQ_DOWN_ITEM},
    {"REQ_SCR_ULINE", REQ_SCR_ULINE}, {"REQ_SCR_DLINE", REQ_SCR_DLINE},
    {"REQ_SCR_DPAGE", REQ_SCR_DPAGE}, {"REQ_SCR_UPAGE", REQ_SCR_UPAGE},
    {"REQ_FIRST_ITEM", REQ_FIRST_ITEM}, {"REQ_LAST_ITEM", REQ_LAST_ITEM},
    {"REQ_NEXT_ITEM", REQ_NEXT_ITEM}, {"REQ_PREV_ITEM", REQ_PREV_ITEM},
    {"REQ_TOGGLE_ITEM", REQ_TOGGLE_ITEM}, {"REQ_CLEAR_PATTERN", REQ_CLEAR_PATTERN},
    {"REQ_BACK_PATTERN", REQ_BACK_PATTERN}, {"REQ_NEXT_MATCH", REQ_NEXT_MATCH},
    {"REQ_PREV_MATCH", REQ_PREV_MATCH}, {"O_ONEVALUE", O_ONEVALUE},
    {"O_SHOWDESC", O_SHOWDESC}, {"O_SHOWMATCH", O_SHOWMATCH},
    {"O_NONCYCLIC", O_NONCYCLIC}
};

static int
_curses_menu_exec(PyObject *module)
{
    _curses_menu_state *state = get_curses_menu_state(module);

    import_curses();
    if (PyErr_Occurred()) {
        return -1;
    }

    state->PyCursesMenu_Type = (PyTypeObject *)PyType_FromModuleAndSpec(
        module, &PyCursesMenu_Type_spec, NULL);
    if (state->PyCursesMenu_Type == NULL ||
        PyModule_AddType(module, state->PyCursesMenu_Type) < 0) {
        return -1;
    }
    state->PyCursesItem_Type = (PyTypeObject *)PyType_FromModuleAndSpec(
        module, &PyCursesItem_Type_spec, NULL);
    if (state->PyCursesItem_Type == NULL ||
        PyModule_AddType(module, state->PyCursesItem_Type) < 0) {
        return -1;
    }
    state->PyCursesError = PyErr_NewException("_curses_menu.error", NULL, NULL);
    if (state->PyCursesError == NULL ||
        PyModule_AddObjectRef(module, "error", state->PyCursesError) < 0) {
        return -1;
    }
    for (size_t i = 0; i < Py_ARRAY_LENGTH(menu_constants); i++) {
        if (PyModule_AddIntConstant(module, menu_constants[i].name,
                                    menu_constants[i].value) < 0) {
            return -1;
        }
    }
    return PyModule_AddStringConstant(module, "version", PyCursesVersion);
}

static int
_curses_menu_traverse(PyObject *module, visitproc visit, void *arg)
{
    _curses_menu_state *state = get_curses_menu_state(module);
    Py_VISIT(state->PyCursesError);
    Py_VISIT(state->PyCursesMenu_Type);
    Py_VISIT(state->PyCursesItem_Type);
    return 0;
}

static int
_curses_menu_clear(PyObject *module)
{
    _curses_menu_state *state = get_curses_menu_state(module);
    Py_CLEAR(state->PyCursesError);
    Py_CLEAR(state->PyCursesMenu_Type);
    Py_CLEAR(state->PyCursesItem_Type);
    return 0;
}

static PyModuleDef_Slot _curses_menu_slots[] = {
    {Py_mod_exec, _curses_menu_exec},
    {Py_mod_multiple_interpreters, Py_MOD_MULTIPLE_INTERPRETERS_NOT_SUPPORTED},
    {0, NULL}
};

static struct PyModuleDef _curses_menumodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "_curses_menu",
    .m_size = sizeof(_curses_menu_state),
    .m_methods = PyCurses_methods,
    .m_slots = _curses_menu_slots,
    .m_traverse = _curses_menu_traverse,
    .m_clear = _curses_menu_clear
};

PyMODINIT_FUNC
PyInit__curses_menu(void)
{
    return PyModuleDef_Init(&_curses_menumodule);
}
