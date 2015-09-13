from pickle import dumps, loads

import pytest

from cotoolz._emptycoroutine import emptycoroutine


def test_empty():
    assert tuple(emptycoroutine) == ()


def test_iterself():
    assert iter(emptycoroutine) is emptycoroutine


def test_pickle():
    assert tuple(loads(dumps(emptycoroutine))) == ()


def test_send():
    with pytest.raises(StopIteration):
        emptycoroutine.send(None)


def test_throw():
    e = ValueError()
    with pytest.raises(ValueError) as exc:
        emptycoroutine.throw(e)
    assert exc.value is e

    with pytest.raises(ValueError) as exc:
        emptycoroutine.throw(ValueError, 'test')
    assert exc.value.args == ('test',)


def test_close():
    for n in range(3):
        emptycoroutine.close()
    assert tuple(emptycoroutine) == ()
