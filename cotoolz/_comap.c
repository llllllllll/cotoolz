#include <stdarg.h>

#include <Python.h>

#include "cotoolz/coiter.h"
#include "cotoolz/comap.h"

PyCoiter_Exported *PyCoiter_API;

static PyObject *
inner_comap_new(PyTypeObject *cls, Py_ssize_t n, PyObject *args)
{
    PyObject *crs;
    PyObject *cr;
    comap *cm;
    PyObject *func;

    assert(PyTuple_Check(args));
    if (!(crs = PyTuple_New(n))) {
        return NULL;
    }

    for (;n > 0;--n) {
        if (!(cr = PyCoiter_API->new(PyTuple_GET_ITEM(args, n)))) {
            Py_DECREF(crs);
            return NULL;
        }
        PyTuple_SET_ITEM(crs, n - 1, cr);
    }

    if (!(cm = (comap*) cls->tp_alloc(cls, 0))) {
        Py_DECREF(crs);
        return NULL;
    }
    cm->cm_crs = crs;
    func = PyTuple_GET_ITEM(args, 0);
    Py_INCREF(func);
    cm->cm_func = func;

    return (PyObject*) cm;
}

PyObject *
PyComap_New(PyObject *func, Py_ssize_t n, ...)
{
    PyObject *args;
    PyObject *item;
    Py_ssize_t m;
    va_list vargs;

    if (n < 1) {
        PyErr_BadInternalCall();
        return NULL;
    }

    if (!(args = PyTuple_New(n + 1))) {
        return NULL;
    }

    Py_INCREF(func);
    PyTuple_SET_ITEM(args, 0, func);

    va_start(vargs, n);
    for (m = 1;m <= n;++m) {
        item = va_arg(vargs, PyObject*);
        Py_INCREF(item);
        PyTuple_SET_ITEM(args, m, item);
    }
    va_end(vargs);

    item = inner_comap_new(&PyComap_Type, n, args);
    Py_DECREF(args);
    return item;
}

static PyObject *
comap_new(PyTypeObject *cls, PyObject *args, PyObject *kwargs)
{
    Py_ssize_t n;

    if (cls == &PyComap_Type && !_PyArg_NoKeywords("comap()", kwargs)) {
        return NULL;
    }

    assert(PyTuple_Check(args));
    if ((n = PyTuple_GET_SIZE(args)) < 2) {
        PyErr_SetString(PyExc_TypeError,
                        "comap() must have at least two arguments.");
        return NULL;
    }
    return inner_comap_new(cls, n - 1, args);
}

PyDoc_STRVAR(comap_send_doc,
             "Send a value into the comap.\n"
             "\n"
             "Paramaters\n"
             "----------\n"
             "value : any\n"
             "    The value to send into the comap.\n"
             "\n"
             "Returns\n"
             "-------\n"
             "y : any\n"
             "    The result of applying the comap's function over the result(s)\n"
             "    of sending the value into the inner coroutine(s).\n");

static PyObject *
comap_send(comap *self, PyObject *value)
{
    Py_ssize_t n;
    PyObject *valuetuple = NULL;
    PyObject *argtuple;
    PyObject *y;
    PyObject *sendstr = NULL;
    PyObject *send;
    PyObject *ret = NULL;

    n = PyTuple_GET_SIZE(self->cm_crs);
    if (!(argtuple = PyTuple_New(n))) {
        return NULL;
    }
    if (!(sendstr = PyUnicode_FromString("send"))) {
        goto error;
    }
    if (!(valuetuple = PyTuple_Pack(1, value))) {
        goto error;
    }

    for (;n;--n) {
        if (!(send = PyObject_GetAttr(PyTuple_GET_ITEM(self->cm_crs, n - 1),
                                      sendstr))) {
            goto error;
        }
        y = PyObject_Call(send, valuetuple, NULL);
        Py_DECREF(send);
        if (!y) {
            goto error;
        }
        PyTuple_SET_ITEM(argtuple, n - 1, y);
    }

    ret = PyObject_Call(self->cm_func, argtuple, NULL);
error:
    Py_XDECREF(argtuple);
    Py_XDECREF(sendstr);
    Py_XDECREF(valuetuple);
    return ret;
}

PyObject *
PyComap_Send(PyObject *cm, PyObject *value)
{
    if (!PyComap_Check(cm)) {
        PyErr_BadInternalCall();
        return NULL;
    }
    return comap_send((comap*) cm, value);
}

static PyObject *
comap_iternext(comap *self)
{
    return comap_send(self, Py_None);
}

PyDoc_STRVAR(comap_throw_doc,
             "Throw an exception into the comap.\n"
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
             "Returns\n"
             "-------\n"
             "y : any\n"
             "    The result of applying the comap's function over the result(s)\n"
             "    of throwing the exception into the inner coroutine(s).\n");

static PyObject *
comap_throw(comap *self, PyObject *args)
{
    Py_ssize_t n;
    PyObject *argtuple;
    PyObject *y;
    PyObject *throwstr = NULL;
    PyObject *throw;
    PyObject *ret = NULL;

    n = PyTuple_GET_SIZE(self->cm_crs);
    if (!(argtuple = PyTuple_New(n))) {
        return NULL;
    }
    if (!(throwstr = PyUnicode_FromString("throw"))) {
        goto error;
    }

    for (;n;--n) {
        if (!(throw = PyObject_GetAttr(PyTuple_GET_ITEM(self->cm_crs, n - 1),
                                       throwstr))) {
            goto error;
        }
        y = PyObject_Call(throw, args, NULL);
        Py_DECREF(throw);
        if (!y) {
            goto error;
        }
        PyTuple_SET_ITEM(argtuple, n - 1, y);
    }

    ret = PyObject_Call(self->cm_func, argtuple, NULL);
error:
    Py_DECREF(argtuple);
    Py_XDECREF(throwstr);
    return ret;
}

PyObject *
PyComap_Throw(PyObject *cm, PyObject *excinfo)
{
    if (!PyComap_Check(cm)) {
        PyErr_BadInternalCall();
        return NULL;
    }
    return comap_throw((comap*) cm, excinfo);
}

PyDoc_STRVAR(comap_close_doc,
             "Close the comap."
             "\n"
             "This closes all of the inner coroutines.\n");

static PyObject *
comap_close(comap *self, PyObject *_)
{
    Py_ssize_t n;
    PyObject *argtuple;
    PyObject *closestr = NULL;
    PyObject *close;

    n = PyTuple_GET_SIZE(self->cm_crs);
    if (!(closestr = PyUnicode_FromString("close"))) {
        return NULL;
    }
    if (!(argtuple = PyTuple_New(0))) {
        Py_DECREF(closestr);
        return NULL;
    }

    for (;n;--n) {
        if (!(close = PyObject_GetAttr(PyTuple_GET_ITEM(self->cm_crs, n - 1),
                                       closestr))) {
            Py_DECREF(closestr);
            Py_DECREF(argtuple);
            return NULL;
        }
        PyObject_Call(close, argtuple, NULL);
    }

    Py_DECREF(argtuple);
    Py_RETURN_NONE;
}

int
PyComap_Close(PyObject *cm)
{
    if (!PyComap_Check(cm)) {
        PyErr_BadInternalCall();
        return 1;
    }
    Py_XDECREF(comap_close((comap*) cm, NULL));
    return 0;
}

static PyMethodDef comap_methods[] = {
    {"send", (PyCFunction) comap_send, METH_O, comap_send_doc},
    {"throw", (PyCFunction) comap_throw, METH_VARARGS, comap_throw_doc},
    {"close", (PyCFunction) comap_close, METH_NOARGS, comap_close_doc},
    {NULL},
};

PyDoc_STRVAR(comap_doc,
             "map that acts on coroutines.\n"
             "\n"
             "Paramaters\n"
             "----------\n"
             "func : callable\n"
             "    The n-ary function where n is the number of\n"
             "    coroutines passed.\n"
             "*coroutines\n"
             "    The coroutines to map func over.\n"
             "\n"
             "Methods\n"
             "-------\n"
             "send(value)\n"
             "    Sends a value into the inner coroutines and calls func on\n"
             "    the results.\n"
             "throw(exc) or throw(type, arg, traceback)\n"
             "    Throws an exception into the inner coroutines and calls\n"
             "    func on the results.\n"
             "close()\n"
             "    Closes the comap by closing all of the inner coroutines.\n");

PyTypeObject PyComap_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "cotoolz._comap.comap",             /* tp_name */
    sizeof(comap),                      /* tp_basicsize */
    0,                                  /* tp_itemsize */
    0,                                  /* tp_dealloc */
    0,                                  /* tp_print */
    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,                                  /* tp_reserved */
    0,                                  /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    0,                                  /* tp_hash */
    0,                                  /* tp_call */
    0,                                  /* tp_str */
    0,                                  /* tp_getattro */
    0,                                  /* tp_setattro */
    0,                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE,                /* tp_flags */
    comap_doc,                          /* tp_doc */
    0,                                  /* tp_traverse */
    0,                                  /* tp_clear */
    0,                                  /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    0,                                  /* tp_iter */
    (iternextfunc) comap_iternext,      /* tp_iternext */
    comap_methods,                      /* tp_methods */
    0,                                  /* tp_members */
    0,                                  /* tp_getset */
    &PyMap_Type,                        /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    0,                                  /* tp_init */
    0,                                  /* tp_alloc */
    comap_new,                          /* tp_new */
};

PyDoc_STRVAR(module_doc, "comap is a map that acts on coroutines.");


static PyComap_Exported exported_symbols = {
  PyComap_New,
  PyComap_Send,
  PyComap_Throw,
  PyComap_Close,
};

static struct PyModuleDef _comap_module = {
    PyModuleDef_HEAD_INIT,
    "cotoolz._comap",
    module_doc,
    -1,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

PyMODINIT_FUNC
PyInit__comap(void)
{
    PyObject *m;
    PyObject *symbols;
    int err;

    /* Assert that our custom comap struct definition is the same as
     * the mapobject struct.
     */
    assert(sizeof(comap) == PyMap_Type.tp_basicsize);

    if (PyType_Ready(&PyComap_Type)) {
        return NULL;
    }

    if (!(PyCoiter_API =
          PyCapsule_Import("cotoolz._coiter._exported_symbols", 0))) {
        return NULL;
    }

    if (!(symbols = PyCapsule_New(&exported_symbols,
                                  "cotoolz._comap._exported_symbols",
                                  NULL))) {
        return NULL;
    }

    if (!(m = PyModule_Create(&_comap_module))) {
        Py_DECREF(symbols);
        return NULL;
    }

    err = PyObject_SetAttrString(m, "_exported_symbols", symbols);
    Py_DECREF(symbols);
    if (err) {
        Py_DECREF(m);
        return NULL;
    }

    if (PyObject_SetAttrString(m, "comap", (PyObject*) &PyComap_Type)) {
        Py_DECREF(m);
        return NULL;
    }
    return m;
}
