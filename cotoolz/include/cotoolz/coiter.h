#ifndef COTOOLZ_COITER_H
#define COTOOLZ_COITER_H

typedef struct {
    PyObject_HEAD
    PyObject *ci_it;
    PyObject *ci_send;
    PyObject *ci_throw;
    PyObject *ci_close;
} coiter;

PyTypeObject PyCoiter_Type;

#define PyCoiter_Check(obj) PyObject_IsInstance(obj, (PyObject*) &PyCoiter_Type)
#define PyCoiter_CheckExact(obj) (Py_TYPE(obj) == &PyCoiter_Type)

/* Construct a new coiter object by wrapping an existing iterator or coroutine.
 *
 * Paramaters
 * ----------
 * it : iterable
 *     The iterator or coroutine to wrap.
 *
 * Returns
 * -------
 * ci : coiter
 *    A new reference to a coiter.
 */
PyObject *PyCoiter_New(PyObject *it);

PyObject *PyCoiter_Send(PyObject *ci, PyObject *value);

PyObject *PyCoiter_Throw(PyObject *ci, PyObject *excinfo);

void PyCoiter_Close(PyObject *ci);

/* Internal user ----------------------------------------------------------- */

/* build and set and exception out of an excinfo tuple.
 *
 * Paramaters
 * ----------
 * args : tuple
 *     excinfo tuple
 */
void _ctz_set_exc_from_tuple(PyObject *args);

#endif
