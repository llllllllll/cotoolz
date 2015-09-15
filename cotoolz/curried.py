from toolz.functoolz import curry, known_numargs

from .include import get_include
from ._coiter import coiter
from ._comap import comap
from ._cozip import cozip
from ._emptycoroutine import emptycoroutine


__all__ = [
    'coiter',
    'comap',
    'cozip',
    'emptycoroutine',
    'get_include',
]


known_numargs[comap] = 2
comap = curry(comap)
del curry, known_numargs
