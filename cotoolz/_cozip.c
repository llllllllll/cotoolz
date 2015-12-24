#include <stdarg.h>

#include <Python.h>

#include "cotoolz/coiter.h"
#include "cotoolz/cozip.h"

PyCoiter_Exported *PyCoiter_API;

static PyObject *
inner_cozip_new(PyTypeObject *cls, Py_ssize_t tuplesize, PyObject *args)
{
    cozip *cz;
    Py_ssize_t n;
    PyObject *crs;
    PyObject *res;
    PyObject *cr;

    if (!(crs = PyTuple_New(tuplesize))) {
        return NULL;
    }
    for (n = 0;n < tuplesize;++n) {
        if (!(cr = PyCoiter_API->new(PyTuple_GET_ITEM(args, n)))) {
            if (PyErr_ExceptionMatches(PyExc_TypeError))
                PyErr_Format(PyExc_TypeError,
                             "cozip argument #%zd must support iteration",
                             n + 1);
            Py_DECREF(crs);
            return NULL;
        }
        PyTuple_SET_ITEM(crs, n, cr);
    }

    if (!(res = PyTuple_New(tuplesize))) {
        Py_DECREF(crs);
        return NULL;
    }
    for (n = 0;n < tuplesize;++n) {
        Py_INCREF(Py_None);
        PyTuple_SET_ITEM(res, n, Py_None);
    }

    if (!(cz = (cozip*) cls->tp_alloc(cls, 0))) {
        Py_DECREF(crs);
        Py_DECREF(res);
        return NULL;
    }
    cz->cz_crs = crs;
    cz->cz_tuplesize = tuplesize;
    cz->cz_res = res;

    return (PyObject*) cz;
}

PyObject *
PyCozip_New(Py_ssize_t n, ...)
{
    va_list vcrs;
    PyObject *crs;
    Py_ssize_t m;
    PyObject *item;

    if (!(crs = PyTuple_New(n))) {
        return NULL;
    }

    va_start(vcrs, n);
    for (m = 0;m < n;++m) {
        item = va_arg(vcrs, PyObject*);
        Py_INCREF(item);
        PyTuple_SET_ITEM(crs, m, item);
    }
    va_end(vcrs);

    item = inner_cozip_new(&PyCozip_Type, n, crs);
    Py_DECREF(crs);
    return item;
}

static PyObject *
cozip_new(PyTypeObject *cls, PyObject *args, PyObject *kwargs)
{
    if (cls == &PyCozip_Type && !_PyArg_NoKeywords("cozip()", kwargs)) {
        return NULL;
    }
    assert(PyTuple_Check(args));
    return inner_cozip_new(cls, PyTuple_GET_SIZE(args), args);
}

PyDoc_STRVAR(cozip_send_doc,
             "Send a value into the cozip.\n"
             "\n"
             "Paramaters\n"
             "----------\n"
             "value : any\n"
             "    The value to send into the cozip.\n"
             "\n"
             "Returns\n"
             "-------\n"
             "y : any\n"
             "    The zipped results of sending the valueinto the inner\n"
             "    coroutine(s).\n");

static PyObject *
cozip_send(cozip *cz, PyObject *value)
{
    PyObject *sendstr;
    PyObject *send;
    Py_ssize_t n;
    PyObject *item;
    Py_ssize_t tuplesize = cz->cz_tuplesize;
    PyObject *crs = cz->cz_crs;
    PyObject *res = cz->cz_res;
    PyObject *argtuple;
    PyObject *ret = NULL;

    if (!tuplesize) {
        return NULL;
    }
    if (!(sendstr = PyUnicode_FromString("send"))) {
        return NULL;
    }
    if (!(argtuple = PyTuple_Pack(1, value))) {
        Py_DECREF(sendstr);
        return NULL;
    }

    if (Py_REFCNT(res) == 1) {
        Py_INCREF(res);
        for (n = 0;n < cz->cz_tuplesize;++n) {
            if (!(send = PyObject_GetAttr(PyTuple_GET_ITEM(crs, n), sendstr))) {
                goto error;
            }
            if (Py_REFCNT(argtuple) != 1) {
                Py_DECREF(argtuple);
                if (!(argtuple = PyTuple_Pack(1, value))) {
                    goto error;
                }
            }
            if (!(item = PyObject_Call(send, argtuple, NULL))) {
                goto error;
            }
            Py_DECREF(PyTuple_GET_ITEM(res, n));
            PyTuple_SET_ITEM(res, n, item);
        }
    }
    else {
        res = PyTuple_New(tuplesize);
        for (n = 0;n < cz->cz_tuplesize;++n) {
            if (!(send = PyObject_GetAttr(PyTuple_GET_ITEM(crs, n), sendstr))) {
                goto error;
            }
            if (Py_REFCNT(argtuple) != 1) {
                Py_DECREF(argtuple);
                if (!(argtuple = PyTuple_Pack(1, value))) {
                    goto error;
                }
            }
            if (!(item = PyObject_Call(send, argtuple, NULL))) {
                goto error;
            }
            PyTuple_SET_ITEM(res, n, item);
        }
    }

    ret = res;
error:
    Py_DECREF(sendstr);
    Py_DECREF(argtuple);
    return ret;
}

PyObject *
PyCozip_Send(PyObject *cz, PyObject *value)
{
    if (!(PyObject_IsInstance(cz, (PyObject*) &PyCozip_Type))) {
        PyErr_BadInternalCall();
        return NULL;
    }
    return cozip_send((cozip*) cz, value);
}

static PyObject *
cozip_next(cozip *cz)
{
    return cozip_send(cz, Py_None);
}

PyDoc_STRVAR(cozip_throw_doc,
             "Throw an exception into the cozip.\n"
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
             "    The zipped the results of throwing the exception into the\n"
             "    inner coroutine(s).\n");

static PyObject *
cozip_throw(cozip *self, PyObject *args)
{
    PyObject *throwstr;
    PyObject *throw;
    Py_ssize_t n;
    PyObject *item;
    Py_ssize_t tuplesize = self->cz_tuplesize;
    PyObject *crs = self->cz_crs;
    PyObject *res = self->cz_res;
    PyObject *ret = NULL;

    if (!tuplesize) {
        return NULL;
    }
    if (!(throwstr = PyUnicode_FromString("throw"))) {
        return NULL;
    }
    if (Py_REFCNT(res) == 1) {
        Py_INCREF(res);
        for (n = 0;n < self->cz_tuplesize;++n) {
            if (!(throw = PyObject_GetAttr(PyTuple_GET_ITEM(crs, n),
                                           throwstr))) {
                goto error;
            }
            if (!(item = PyObject_Call(throw, args, NULL))) {
                goto error;
            }
            Py_DECREF(PyTuple_GET_ITEM(res, n));
            PyTuple_SET_ITEM(res, n, item);
        }
    }
    else {
        res = PyTuple_New(tuplesize);
        for (n = 0;n < self->cz_tuplesize;++n) {
            if (!(throw = PyObject_GetAttr(PyTuple_GET_ITEM(crs, n),
                                           throwstr))) {
                goto error;
            }
            if (!(item = PyObject_Call(throw, args, NULL))) {
                goto error;
            }
            PyTuple_SET_ITEM(res, n, item);
        }
    }

    ret = res;
error:
    Py_DECREF(throwstr);
    return ret;
}

PyObject *
PyCozip_Throw(PyObject *cz, PyObject *excinfo)
{
    if (!PyCozip_Check(cz)) {
        PyErr_BadInternalCall();
        return NULL;
    }
    return cozip_throw((cozip*) cz, excinfo);
}

PyDoc_STRVAR(cozip_close_doc,
             "Close the cozip."
             "\n"
             "This closes all of the inner coroutines.\n");

static PyObject *
cozip_close(cozip *self, PyObject *_)
{
    Py_ssize_t n;
    PyObject *argtuple;
    PyObject *closestr = NULL;
    PyObject *close;

    n = PyTuple_GET_SIZE(self->cz_crs);
    if (!(closestr = PyUnicode_FromString("close"))) {
        return NULL;
    }
    if (!(argtuple = PyTuple_New(0))) {
        Py_DECREF(closestr);
        return NULL;
    }

    for (;n;--n) {
        if (!(close = PyObject_GetAttr(PyTuple_GET_ITEM(self->cz_crs, n - 1),
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
PyCozip_Close(PyObject *cz)
{
    if (!PyCozip_Check(cz)) {
        PyErr_BadInternalCall();
        return 1;
    }
    Py_XDECREF(cozip_close((cozip*) cz, NULL));
    return 0;
}

static PyMethodDef cozip_methods[] = {
    {"send", (PyCFunction) cozip_send, METH_O, cozip_send_doc},
    {"throw", (PyCFunction) cozip_throw, METH_VARARGS, cozip_throw_doc},
    {"close", (PyCFunction) cozip_close, METH_NOARGS, cozip_close_doc},
    {NULL},
};

PyDoc_STRVAR(cozip_doc,
             "zip that acts on coroutines.\n"
             "\n"
             "Paramaters\n"
             "----------\n"
             "*coroutines\n"
             "    The coroutines to zip together.\n"
             "\n"
             "Methods\n"
             "-------\n"
             "send(value)\n"
             "    Sends a value into the inner coroutines and zips the results\n"
             "throw(exc) or throw(type, arg, traceback)\n"
             "    Throws an exception into the inner coroutines and zips\n"
             "    the results.\n"
             "close()\n"
             "    Closes the cozip by closing all of the inner coroutines.\n"
    );

PyTypeObject PyCozip_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "cotoolz._cozip.cozip",             /* tp_name */
    sizeof(cozip),                      /* tp_basicsize */
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
    cozip_doc,                          /* tp_doc */
    0,                                  /* tp_traverse */
    0,                                  /* tp_clear */
    0,                                  /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    0,                                  /* tp_iter */
    (iternextfunc) cozip_next,          /* tp_iternext */
    cozip_methods,                      /* tp_methods */
    0,                                  /* tp_members */
    0,                                  /* tp_getset */
    &PyZip_Type,                        /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    0,                                  /* tp_init */
    0,                                  /* tp_alloc */
    cozip_new,                          /* tp_new */
};

PyDoc_STRVAR(module_doc, "cozip is a zip that acts on coroutines.");

static struct PyModuleDef _cozip_module = {
    PyModuleDef_HEAD_INIT,
    "cotoolz._cozip",
    module_doc,
    -1,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static PyCozip_Exported exported_symbols = {
    PyCozip_New,
    PyCozip_Send,
    PyCozip_Throw,
    PyCozip_Close,
};

PyMODINIT_FUNC
PyInit__cozip(void)
{
    PyObject *m;
    PyObject *symbols;
    int err;

    /* Assert that our custom cozip struct definition is the same as
     * the zipobject struct.
     */
    assert(sizeof(cozip) == PyZip_Type.tp_basicsize);

    if (PyType_Ready(&PyCozip_Type)) {
        return NULL;
    }

    if (!(PyCoiter_API =
          PyCapsule_Import("cotoolz._coiter._exported_symbols", 0))) {
        return NULL;
    }

    if (!(symbols = PyCapsule_New(&exported_symbols,
                                  "cotoolz._cozip._exported_symbols",
                                  NULL))) {
        return NULL;
    }

    if (!(m = PyModule_Create(&_cozip_module))) {
        Py_DECREF(symbols);
        return NULL;
    }

    err = PyObject_SetAttrString(m, "_exported_symbols", symbols);
    Py_DECREF(symbols);
    if (err) {
        Py_DECREF(m);
        return NULL;
    }

    if (PyObject_SetAttrString(m, "cozip", (PyObject*) &PyCozip_Type)) {
        Py_DECREF(m);
        return NULL;
    }
    return m;
}
