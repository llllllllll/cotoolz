#ifndef COTOOLZ_COITER_H
#define COTOOLZ_COITER_H

typedef struct {
    PyObject_HEAD
    PyObject *ci_it;
    PyObject *ci_send;
    PyObject *ci_throw;
    PyObject *ci_close;
} coiter;

extern PyTypeObject PyCoiter_Type;

#define PyCoiter_Check(obj) PyObject_IsInstance(obj, (PyObject*) &PyCoiter_Type)
#define PyCoiter_CheckExact(obj) (Py_TYPE(obj) == &PyCoiter_Type)

typedef struct{

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
    PyObject *(*new)(PyObject *it);

    /* Send a value into a coiter.
     *
     * Paramaters
     * ----------
     * value : any
     *     The value to send in, if this is not wrapping a coroutine
     *     then the value is ignored.
     *
     * Returns
     * -------
     * y : any
     *     A new reference to the next yielded value.
     *
     * Raises
     * ------
     * StopIteration
     *     When there are no more values in the inner iterator.
     */
    PyObject *(*send)(PyObject *ci, PyObject *value);

    /* Throw an exception into a coroutine.
     *
     * Paramaters
     * ----------
     * exc : Exception
     *     The exception to raise.
     * -OR-
     * type : Exception class
     *     The type of exception to raise.
     * arg : any
     *     The argument to ``type``.
     * tb : traceback
     *     The traceback to raise the exception with.
     *
     * Raises
     * ------
     * exc
     *     The exception that was passed in if not caught.
     * PyObject *PyCoiter_Throw(PyObject *ci, PyObject *excinfo);
     */
    PyObject *(*throw)(PyObject *ci, PyObject *excinfo);

    /* Close a coiter.
     * This closes the inner data.
     *
     * Returns
     * -------
     * err : int
     *     zero on success, non-zero on failure.
     */
    int (*close)(PyObject *ci);

}PyCoiter_Exported;

/* Internal use ------------------------------------------------------------- */

/* build and set and exception out of an excinfo tuple.
 *
 * Paramaters
 * ----------
 * args : tuple
 *     excinfo tuple
 */
void _ctz_set_exc_from_tuple(PyObject *args);

#endif
