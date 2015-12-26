#include <Python.h>
#include <structmember.h>

#include "cotoolz/coiter.h"
#include "cotoolz/emptycoroutine.h"

static PyObject *
inner_coiter_new(PyTypeObject *cls, PyObject *it)
{
    coiter *self;

    if (!(self = (coiter*) cls->tp_alloc(cls, 0))) {
        Py_DECREF(it);
        return NULL;
    }

    if (!(self->ci_it = PyObject_GetIter(it))) {
        Py_DECREF(self);
        return NULL;
    }

    /* These macros lookup the method on the object, and fallback to
     * self._{meth} if not found.
     */
    #define STR(a) #a
    #define SET_METH(meth)                                              \
        if (!(self->ci_ ## meth =                                       \
              PyObject_GetAttrString(it, STR(meth)))){                  \
            if (PyErr_ExceptionMatches(PyExc_AttributeError)) {         \
                PyErr_Clear();                                          \
                if (!(self->ci_ ## meth =                               \
                      PyObject_GetAttrString((PyObject*) self,          \
                                             "_" STR(meth)))) {         \
                    Py_DECREF(self);                                    \
                    return NULL;                                        \
                }                                                       \
            }                                                           \
            else {                                                      \
                Py_DECREF(self);                                        \
                return NULL;                                            \
            }                                                           \
        }                                                               \
        NULL  /* puts a semicolon at the end of the macro */

    SET_METH(send);
    SET_METH(throw);
    SET_METH(close);

    #undef STR
    #undef SET_METH
    return (PyObject*) self;
}

PyObject *
PyCoiter_New(PyObject *it)
{
    return inner_coiter_new(&PyCoiter_Type, it);
}

static PyObject *
coiter_new(PyTypeObject *cls, PyObject *args, PyObject *kwargs)
{
    if (cls == &PyCoiter_Type && !_PyArg_NoKeywords("coiter()", kwargs)) {
        return NULL;
    }
    if (PyTuple_Size(args) != 1) {
        PyErr_Format(PyExc_TypeError,
                     "coiter() takes exactly one argument (%zd given)",
                     PyTuple_Size(args));
        return NULL;
    }
    return inner_coiter_new(cls, PyTuple_GET_ITEM(args, 0));
}

PyObject *
PyCoiter_Throw(PyObject *ci, PyObject *excinfo)
{
    if (!PyCoiter_Check(ci)) {
        PyErr_BadInternalCall();
        return NULL;
    }
    return PyObject_Call(((coiter*) ci)->ci_throw, excinfo, NULL);
}

int
PyCoiter_Close(PyObject *ci)
{
    static PyObject *empty = NULL;

    if (!PyCoiter_Check(ci)) {
        PyErr_BadInternalCall();
        return 1;
    }
    if (!empty && !(empty = PyTuple_New(0))) {
        return 1;
    }
    Py_DECREF(PyObject_Call(((coiter*) ci)->ci_close, empty, NULL));
    return 0;
}

static int
coiter_traverse(coiter *self, visitproc visit, void *arg)
{
    Py_VISIT(self->ci_it);
    Py_VISIT(self->ci_send);
    Py_VISIT(self->ci_throw);
    Py_VISIT(self->ci_close);
    return 0;
}

static int
coiter_clear(coiter *self)
{
    Py_CLEAR(self->ci_it);
    Py_CLEAR(self->ci_send);
    Py_CLEAR(self->ci_throw);
    Py_CLEAR(self->ci_close);
    return 0;
}

static void
coiter_dealloc(coiter *self)
{
    PyObject_GC_UnTrack(self);
    Py_XDECREF(self->ci_it);
    Py_XDECREF(self->ci_send);
    Py_XDECREF(self->ci_throw);
    Py_XDECREF(self->ci_close);
    Py_TYPE(self)->tp_free(self);
}

static PyObject *
coiter_send(coiter *self, PyObject *value)
{
    static PyObject * volatile cached_args = NULL;
    PyObject *args;
    PyObject * ret;

    args = cached_args;
    if (!args || Py_REFCNT(args) != 1) {
        Py_CLEAR(cached_args);
        if (!(cached_args = args = PyTuple_New(1)))
            return NULL;
    }
    Py_INCREF(args);
    assert (Py_REFCNT(args) == 2);
    Py_INCREF(value);
    PyTuple_SET_ITEM(args, 0, value);
    ret = PyObject_Call(((coiter*) self)->ci_send, args, NULL);
    if (args == cached_args) {
        if (Py_REFCNT(args) == 2) {
            value = PyTuple_GET_ITEM(args, 0);
            PyTuple_SET_ITEM(args, 0, NULL);
            Py_XDECREF(value);
        }
        else {
            Py_CLEAR(cached_args);
        }
    }
    Py_DECREF(args);
    return ret;
}

PyObject *
PyCoiter_Send(PyObject *ci, PyObject *value)
{
    if (!PyCoiter_Check(ci)) {
        PyErr_BadInternalCall();
        return NULL;
    }
    return coiter_send((coiter*) ci, value);
}

static PyObject *
coiter_iternext(coiter *self)
{
    return coiter_send(self, Py_None);
}

PyDoc_STRVAR(coiter__send_doc,
             "Emulates sending a value into the iterator by calling next.\n"
             "\n"
             "If the inner iterator is a coroutine, this will forward the \n"
             "sent value into the coroutine.\n"
             "\n"
             "Paramaters\n"
             "----------\n"
             "value : any\n"
             "    The value to send in, if this is not wrapping a coroutine\n"
             "    then the value is ignored.\n"
             "\n"
             "Returns\n"
             "-------\n"
             "y : any\n"
             "    The next yielded value.\n"
             "\n"
             "Raises\n"
             "------\n"
             "StopIteration\n"
             "    When there are no more values in the inner iterator.\n");

static PyObject *
coiter__send(coiter *self, PyObject *_)
{
    PyObject *ret;

    if (!(ret = PyIter_Next(self->ci_it))) {
        if (!PyErr_Occurred()) {
            PyErr_SetString(PyExc_StopIteration, "");
        }
    }
    return ret;
}

PyDoc_STRVAR(coiter__throw_doc,
             "Emulates throwing a value into the iterator by raising the\n"
             "exception.\n"
             "\n"
             "If the inner iterator is a coroutine, this will forward the\n"
             "thrown exception into the coroutine.\n"
             "\n"
             "Paramaters\n"
             "----------\n"
             "exc : Exception\n"
             "    The exception to raise.\n"
             "-OR-\n"
             "type : Exception class\n"
             "    The type of exception to raise.\n"
             "arg : any\n"
             "    The argument to ``type``.\n"
             "tb : traceback\n"
             "    The traceback to raise the exception with.\n"
             "\n"
             "Raises\n"
             "------\n"
             "exc\n"
             "    The exception that was passed in.\n");

static PyObject *
coiter__throw(coiter *self, PyObject *args)
{
    _ctz_set_exc_from_tuple(args);
    return NULL;
}

PyDoc_STRVAR(coiter__close_doc,
             "Emulates closing the iterator.\n"
             "\n"
             "If the inner iterable is a coroutine then this will forward\n"
             "the call to ``close`` into the coroutine.\n"
             "\n"
             "Any further calls to ``next`` or ``.send`` will\n"
             "raise a ``StopIteration``\n");

static PyObject *
coiter__close(coiter *self, PyObject *_)
{
    Py_DECREF(self->ci_it);
    Py_INCREF(Py_EmptyCoroutine);
    self->ci_it = Py_EmptyCoroutine;
    Py_RETURN_NONE;
}

static PyObject *
coiter___reduce__(coiter *self, PyObject *args)
{
    PyObject *rargs;

    if (!(rargs = PyTuple_Pack(1, self->ci_it))) {
        return NULL;
    }
    return Py_BuildValue("ON", Py_TYPE(self), rargs);
}

static PyMethodDef coiter_methods[] = {
    {"_send", (PyCFunction) coiter__send, METH_O, coiter__send_doc},
    {"_throw", (PyCFunction) coiter__throw, METH_VARARGS, coiter__throw_doc},
    {"_close", (PyCFunction) coiter__close, METH_NOARGS, coiter__close_doc},
    {"__reduce__", (PyCFunction) coiter___reduce__, METH_NOARGS, ""},
    {NULL},
};

#define OFF(a) offsetof(coiter, a)

/* These are the public methods of coiter. */
static PyMemberDef coiter_members[] = {
    {"send", T_OBJECT_EX, OFF(ci_send), READONLY, coiter__send_doc},
    {"throw", T_OBJECT_EX, OFF(ci_throw), READONLY, coiter__throw_doc},
    {"close", T_OBJECT_EX, OFF(ci_close), READONLY, coiter__close_doc},
    {NULL},
};

#undef OFF

PyDoc_STRVAR(coiter_doc,
             "A wrapper around standard iterators that allows them to\n"
             "respond to the coroutine protocol.\n"
             "\n"
             "If the wrapped iterable is a coroutine, then the coroutine\n"
             "methods will delegate to the inner coroutine.\n"
             "\n"
             "Paramaters\n"
             "----------\n"
             "it : iterable\n"
             "    The iterable to wrap.\n"
             "\n"
             "Methods\n"
             "-------\n"
             "send(value)\n"
             "    Emulating sending a value into the iterator.\n"
             "throw(exc) or throw(type, arg, traceback)\n"
             "    Emulates throwing an exception into the iterator.\n"
             "close()\n"
             "    Emulates closing the iterator.\n");

PyTypeObject PyCoiter_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "cotoolz._coiter.coiter",                   /* tp_name */
    sizeof(coiter),                             /* tp_basicsize */
    0,                                          /* tp_itemsize */
    (destructor) coiter_dealloc,                /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_reserved */
    0,                                          /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    PyObject_GenericGetAttr,                    /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE |
    Py_TPFLAGS_HAVE_GC,                         /* tp_flags */
    coiter_doc,                                 /* tp_doc */
    (traverseproc) coiter_traverse,             /* tp_traverse */
    (inquiry) coiter_clear,                     /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    PyObject_SelfIter,                          /* tp_iter */
    (iternextfunc) coiter_iternext,             /* tp_iternext */
    coiter_methods,                             /* tp_methods */
    coiter_members,                             /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    0,                                          /* tp_init */
    PyType_GenericAlloc,                        /* tp_alloc */
    (newfunc) coiter_new,                       /* tp_new */
    PyObject_GC_Del,                            /* tp_free */
};

/* module setup ------------------------------------------------------------ */

PyDoc_STRVAR(module_doc,
             "coiter is a wrapper around either an iterator or a coroutine\n"
             "that ensures that the result respects the coroutine protocol.");

static struct PyModuleDef _coiter_module = {
    PyModuleDef_HEAD_INIT,
    "cotoolz._coiter",
    module_doc,
    -1,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static PyCoiter_Exported exported_symbols = {
    PyCoiter_New,
    PyCoiter_Send,
    PyCoiter_Throw,
    PyCoiter_Close,
};

PyMODINIT_FUNC
PyInit__coiter(void)
{
    PyObject *m;
    PyObject *symbols;
    int err;

    if (PyType_Ready(&PyCoiter_Type)) {
        return NULL;
    }

    if (PyEmptyCoroutine_Import()) {
        return NULL;
    }

    if (!(symbols = PyCapsule_New(&exported_symbols,
                                  "cotoolz._coiter._exported_symbols",
                                  NULL))) {
        return NULL;
    }

    if (!(m = PyModule_Create(&_coiter_module))) {
        Py_DECREF(symbols);
        return NULL;
    }

    err = PyObject_SetAttrString(m, "_exported_symbols", symbols);
    Py_DECREF(symbols);
    if (err) {
        Py_DECREF(m);
        return NULL;
    }

    if (PyObject_SetAttrString(m, "coiter", (PyObject*) &PyCoiter_Type)) {
        Py_DECREF(m);
        return NULL;
    }
    return m;
}
