import pytest
from toolz import identity
from toolz.curried import operator as op

from cotoolz._comap import comap


def gen():
    yield 1
    yield 2  # pragma: no cover
    yield 3  # pragma: no cover


def co():
    yield (yield (yield 1))


def co_throwable():
    try:
        yield 1
    except Exception as e:
        yield e


def test_comap_maplike():
    cm = comap(op.add(1), (1, 2, 3))
    assert isinstance(cm, map)
    assert tuple(cm) == tuple(map(op.add(1), (1, 2, 3))) == (2, 3, 4)

    dm = comap(op.add, (1, 2, 3), (1, 2, 3))
    assert tuple(dm) == (2, 4, 6)


def test_comap_send():
    cm = comap(op.add(1), co())

    assert next(cm) == 2
    for n in (2, 3):
        assert cm.send(n) == n + 1

    dm = comap(op.add, co(), co())
    assert next(dm) == 2
    for n in (2, 3):
        assert dm.send(n) == 2 * n


def test_comap_throw():
    cm = comap(identity, co_throwable())
    assert next(cm) == 1
    e = ValueError()
    assert cm.throw(e) is e

    dm = comap(lambda *a: a, co_throwable(), co_throwable())
    assert next(dm) == (1, 1)
    e = ValueError()
    assert dm.throw(e) == (e, e)

    em = comap(op.add, co_throwable(), gen())
    assert next(em) == 2
    with pytest.raises(ValueError) as exc:
        em.throw(e)
    assert exc.value is e


def test_comap_close():
    cm = comap(identity, gen())
    assert next(cm) == 1
    cm.close()
    assert tuple(cm) == ()
