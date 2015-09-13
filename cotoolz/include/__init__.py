import os


def get_include():
    """Return the directory that contains the cotoolz \*.h header files.

    Extension modules that need to compile against cotoolz should use this
    function to locate the appropriate include directory.

    Notes
    -----
    When using ``distutils``, for example in ``setup.py``.
    ::

        import cotoolz as ctz
        ...
        Extension(
            'extension_name', ...
            include_dirs=[ctz.get_include()],
        )
        ...
    """
    return os.path.dirname(__file__)
