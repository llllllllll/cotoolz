#ifndef COTOOLZ_COZIP_H
#define COTOOLZ_COZIP_H

typedef struct {
    PyObject_HEAD
    Py_ssize_t cz_tuplesize;
    PyObject *cz_crs;
    PyObject *cz_res;
} cozip;

PyTypeObject PyCozip_Type;

#define PyCozip_Check(obj) PyObject_IsInstance(obj, (PyObject*) &PyCozip_Type)
#define PyCozip_CheckExact(obj) (Py_TYPE(obj) == &PyCozip_Type)

/* Construct a new cozip from n coroutines.
 *
 * Paramaters
 * ----------
 * n : Py_ssize_t
 *     The number of coroutines to zip together.
 * *coroutines : iterable of coroutine
 *     The coroutines to zip together.
 *
 * Returns
 * -------
 * cz : cozip
 *     A new reference to a cozip.
 */
PyObject *PyCozip_New(Py_ssize_t n, ...);

/* Send a value into a cozip.
 *
 * Paramaters
 * ----------
 * cm : cozip
 *     The cozip to send the value into.
 * value : any
 *     The value to send into the comap.
 *
 * Returns
 * -------
 * y : any
 *     The zipped results of sending the valueinto the inner coroutine(s).
 *     In python:
 *     tuple(*map(methodcaller('send', value), cz.cz_crs))
 */
PyObject *PyCozip_Send(PyObject *cz, PyObject *value);

/* Throw an exception into a cozip.
 *
 * Paramaters
 * ----------
 * cz : cozip
 *     The cozip to throw the exception into.
 * exc : Exception
 *     The exception to raise.
 * -OR-
 * cz : cozip
 *     The cozip to throw the exception into.
 * type : Exception class
 *     The type of exception to raise.
 * arg : any
 *     The argument to ``type``.
 * tb : traceback
 *     The traceback to raise the exception with.
 *
 * Returns
 * -------
 * y : any
 *     The zipped the results of throwing the exception into the inner
 *     coroutine(s).
 *     In python:
 *     tuple(*map(methodcaller('throw', *excinfo), cz.cz_crs))
 */
PyObject *PyCozip_Throw(PyObject *cz, PyObject *excinfo);

/* Close a cozip.
 *
 * This closes all of the inner coroutines.
 */
void PyCozip_Close(PyObject *cz);

#endif
