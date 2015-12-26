#ifndef COTOOLZ_EMPTYCOROUTINE_H
#define COTOOLZ_EMPTYCOROUTINE_H


PyObject *Py_EmptyCoroutine;

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
    return Py_EmptyCoroutine != NULL;
}

#endif
