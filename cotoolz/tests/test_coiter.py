import pytest

from cotoolz._coiter import coiter


def test_coiter_iter():
    assert tuple(coiter((1, 2, 3))) == (1, 2, 3)
    it = coiter((1, 2, 3))
    for n in (1, 2, 3):
        assert it.send(None) == n


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


def test_coiter_send():
    c = coiter(co())
    assert next(c) == 1
    for n in (2, 3):
        assert c.send(n) == n


def test_coiter_throw():
    c = coiter(co_throwable())
    assert next(c) == 1
    e = ValueError()
    assert c.throw(e) is e

    d = coiter((1, 2, 3))
    with pytest.raises(ValueError) as exc:
        d.throw(e)
    assert exc.value is e

    f = coiter((1, 2, 3))
    with pytest.raises(ValueError) as exc:
        f.throw(ValueError, 'v')
    assert exc.value.args == ('v',)


def test_coiter_close():
    c = coiter(gen())
    assert next(c) == 1
    c.close()
    assert tuple(c) == ()
