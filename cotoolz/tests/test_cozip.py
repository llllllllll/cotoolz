import pytest

from cotoolz._cozip import cozip


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


def test_cozip_ziplike():
    cz = cozip((1, 2, 3), (1, 2, 3))
    assert isinstance(cz, zip)
    assert tuple(cz) == tuple(zip((1, 2, 3), (1, 2, 3))) == (
        (1, 1), (2, 2), (3, 3),
    )

    dz = cozip((1, 2, 3))
    assert tuple(dz) == tuple(zip((1, 2, 3))) == ((1,), (2,), (3,))


def test_cozip_send():
    cz = cozip(co())
    assert next(cz) == (1,)
    for n in (2, 3):
        assert cz.send(n) == (n,)

    dz = cozip(co(), co())
    assert next(dz) == (1, 1)
    for n in (2, 3):
        assert dz.send(n) == (n, n)


def test_cozip_throw():
    cz = cozip(co_throwable())
    assert next(cz) == (1,)
    e = ValueError()
    assert cz.throw(e) == (e,)

    dz = cozip(co_throwable(), co_throwable())
    assert next(dz) == (1, 1)
    e = ValueError()
    assert dz.throw(e) == (e, e)

    ez = cozip(co_throwable(), gen())
    assert next(ez) == (1, 1)
    with pytest.raises(ValueError) as exc:
        ez.throw(e)
    assert exc.value is e


def test_cozip_close():
    cz = cozip(gen())
    assert next(cz) == (1,)
    cz.close()
    assert tuple(cz) == ()

    dz = cozip(gen(), gen())
    assert next(dz) == (1, 1)
    dz.close()
    assert tuple(dz) == ()
