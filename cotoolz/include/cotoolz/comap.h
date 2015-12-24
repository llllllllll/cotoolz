#ifndef COTOOLZ_COMAP_H
#define COTOOLZ_COMAP_H

typedef struct {
    PyObject_HEAD
    PyObject *cm_crs;
    PyObject *cm_func;
} comap;

extern PyTypeObject PyComap_Type;

#define PyComap_Check(obj) PyObject_IsInstance(obj, (PyObject*) &PyComap_Type)
#define PyComap_CheckExact(obj) (Py_TYPE(obj) == &PyComap_Type)

typedef struct{

    /* Construct a new comap object from a function and a variable amount of
     * coroutines.
     *
     * Paramaters
     * ----------
     * func : callable
     *     The function to map over the coroutines.
     * n : Py_ssize_t
     *     The number of coroutines.
     * crs : var * any
     *     The coroutines to be mapped over.
     *
     * Returns
     * -------
     * cm : comap
     *     A new reference to a comap.
     */
    PyObject *(*PyComap_New)(PyObject *func, Py_ssize_t n, ...);

    /* Send a value into a comap.
     *
     * Paramaters
     * ----------
     * cm : comap
     *     The comap to send the value into.
     * value : any
     *     The value to send into the comap.
     *
     * Returns
     * -------
     * y : any
     *     The result of applying the comap's function over the result(s)
     *     of sending the value into the inner coroutine(s). In python:
     *     cm.cm_func(*map(methodcaller('send', value), cm.cm_crs))
     */
    PyObject *(*PyComap_Send)(PyObject *cm, PyObject *value);

    /* Throw an exception into a comap.
     *
     * Paramaters
     * ----------
     * cm : comap
     *     The comap to throw the exception into.
     * exc : Exception
     *     The exception to raise.
     * -OR-
     * cm : comap
     *     The comap to throw the exception into.
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
     *     The result of applying the comap's function over the result(s)
     *     of throwing the exception into the inner coroutine(s). In python:
     *     cm.cm_func(*map(methodcaller('throw', *excinfo), cm.cm_crs))
     */
    PyObject *(*PyComap_Throw)(PyObject *cm, PyObject *excinfo);

    /* Close a comap.
     * This closes all of the inner coroutines.
     *
     * Returns
     * -------
     * err : int
     *     zero on success, non-zero on failure.
     */
    int (*PyComap_Close)(PyObject *cm);
}PyComap_Exported;

#endif
