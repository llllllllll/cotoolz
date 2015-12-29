#ifndef COTOOLZ_EMPTYCOROUTINE_H
#define COTOOLZ_EMPTYCOROUTINE_H


PyObject *Py_EmptyCoroutine = NULL;

#define Py_RETURN_EMPTYCOROUTINE                     \
    Py_INCREF(Py_EmptyCoroutine);                    \
    return Py_EmptyCoroutine;


/* Import the `Py_EmptyCoroutine` object into the current object.
 *
 * Returns
 * -------
 * err : int
 *     zero on success, non-zero on failure.
 */
int
PyEmptyCoroutine_Import(void)
{
    PyObject *m;

    if (!(m = PyImport_ImportModule("cotoolz._emptycoroutine"))) {
        return 1;
    }
    Py_EmptyCoroutine = PyObject_GetAttrString(m, "emptycoroutine");
    Py_DECREF(m);
    return !Py_EmptyCoroutine;
}

void _ctz_set_exc_from_tuple(PyObject *args)
{
    PyObject *type;
    PyObject *value = NULL;
    PyObject *tb = NULL;

    if (!PyArg_ParseTuple(args, "O|OO:throw", &type, &value, &tb)) {
        return;
    }

    if (PyObject_IsInstance(type, (PyObject*) PyExc_Exception)) {
        if (value) {
            PyErr_SetString(PyExc_TypeError,
                            "throw either takes an exception instance or"
                            " type, value, tb");
            return;
        }
        PyErr_SetObject((PyObject*) Py_TYPE(type), type);
        return;
    }
    PyErr_NormalizeException(&type, &value, &tb);
    if (tb) {
        PyException_SetTraceback(value, tb);
    }
    PyErr_Restore(type, value, tb);
}

#endif
