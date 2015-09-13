#ifndef COTOOLZ_EMPTYCOROUTINE_H
#define COTOOLZ_EMPTYCOROUTINE_H

PyAPI_DATA(PyObject) _Py_EmptyCoroutineObject;
#define Py_EmptyCoroutine (&_Py_EmptyCoroutineObject)
#define Py_RETURN_EMPTYCOROUTINE                     \
    Py_INCREF(Py_EmptyCoroutine);                    \
    return Py_EmptyCoroutine;

#endif
