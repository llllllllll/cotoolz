from toolz.functoolz import curry
from toolz._signatures import module_info, create_signature_registry

from ._coiter import coiter
from ._comap import comap
from ._cozip import cozip
from ._emptycoroutine import emptycoroutine
from .include import get_include


__all__ = [
    'coiter',
    'comap',
    'cozip',
    'emptycoroutine',
    'get_include',
]

module_info['cotoolz._comap'] = {
    'comap': [lambda func, coroutine, *coroutines: None],
}
create_signature_registry()

comap = curry(comap)
del curry, module_info, create_signature_registry
