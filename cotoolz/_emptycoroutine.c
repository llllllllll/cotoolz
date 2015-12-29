#include <Python.h>

#include "cotoolz/emptycoroutine.h"

static PyObject *
emptycoroutine_iternext(PyObject *self)
{
    return NULL;
}

static PyObject *
emptycoroutine_reduce(PyObject *self, PyObject *args)
{
    return PyUnicode_FromString("emptycoroutine");
}

static PyObject *
emptycoroutine_repr(PyObject *self)
{
    return PyUnicode_FromString("emptycoroutine");
}

static PyObject *
emptycoroutine_send(PyObject *self, PyObject *_)
{
    PyErr_SetString(PyExc_StopIteration, "");
    return NULL;
}

static PyObject *
emptycoroutine_throw(PyObject *self, PyObject *args)
{
    _ctz_set_exc_from_tuple(args);
    return NULL;
}

static PyObject *
emptycoroutine_close(PyObject *self, PyObject *_)
{
    Py_RETURN_NONE;
}

static PyMethodDef emptycoroutine_methods[] = {
    {"send", (PyCFunction) emptycoroutine_send, METH_O, ""},
    {"throw", (PyCFunction) emptycoroutine_throw, METH_VARARGS, ""},
    {"close", (PyCFunction) emptycoroutine_close, METH_NOARGS, ""},
    {"__reduce__", (PyCFunction) emptycoroutine_reduce, METH_VARARGS, ""},
    {NULL},
};

PyDoc_STRVAR(emptycoroutine_doc,
             "An empty coroutine singleton value.\n"
             "\n"
             "Examples\n"
             "--------\n"
             ">>> tuple(emptycoroutine)\n"
             "()\n"
             ">>> list(emptycoroutine)\n"
             "[]\n");

static PyTypeObject emptycoroutine_type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "cotoolz._emptycoroutine.emptycoroutine",   /* tp_name */
    sizeof(PyObject),                           /* tp_basicsize */
    0,                                          /* tp_itemsize */
    (destructor) PyObject_Del,                  /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_reserved */
    emptycoroutine_repr,                        /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    emptycoroutine_repr,                        /* tp_str */
    PyObject_GenericGetAttr,                    /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                         /* tp_flags */
    emptycoroutine_doc,                         /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    PyObject_SelfIter,                          /* tp_coroutine */
    (iternextfunc) emptycoroutine_iternext,     /* tp_coroutinenext */
    emptycoroutine_methods,                     /* tp_methods */
};

PyObject _Py_EmptyCoroutineObject = {
    _PyObject_EXTRA_INIT
    1,
    &emptycoroutine_type,
};

/* module setup ------------------------------------------------------------ */

static struct PyModuleDef _emptycoroutine_module = {
    PyModuleDef_HEAD_INIT,
    "cotoolz._emptycoroutine",
    "",
    -1,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};


PyMODINIT_FUNC
PyInit__emptycoroutine(void)
{
    PyObject *m;
    if (PyType_Ready(&emptycoroutine_type)) {
        return NULL;
    }
    if (!(m = PyModule_Create(&_emptycoroutine_module))) {
        return NULL;
    }
    if (PyObject_SetAttrString(m,
                               "emptycoroutine",
                               &_Py_EmptyCoroutineObject)) {
        Py_DECREF(m);
        return NULL;
    }
    return m;
}
