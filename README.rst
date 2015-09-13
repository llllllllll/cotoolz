``cotoolz 0.1``
===============

|build status|

A set of utility functions for coroutines.

Example
-------

In this example we will be showing ``comap``.
``comap`` is like ``__builtins__.map``; however, it knows about the coroutine
protocol (``send``, ``throw``, and ``close``). Here we will show how you can
map over a coroutine while still being able to send into it.

.. code-block:: python

   >>> def my_coroutine():
   ...     yield (yield (yield 1))
   >>> from cotoolz import comap
   >>> cm = comap(lambda a: a + 1, my_coroutine())
   >>> next(cm)
   2
   >>> cm.send(2)
   3
   >>> cm.send(3)
   4
   >>> cm.send(4)
   Traceback (most recent call last):
       ...
   StopIteration


Dependencies
------------

``cotoolz`` depends on CPython 3 and some means of compiling C99.
We recommend using ``gcc`` to compile ``cotoolz``.

.. |build status| image:: https://travis-ci.org/llllllllll/cotoolz.svg?branch=master
   :target: https://travis-ci.org/llllllllll/cotoolz
