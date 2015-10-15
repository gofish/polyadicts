#!/usr/bin/env python3
import platform
import sys

pd = None
libpath = None

def main(buildroot='build'):
    global pd, libpath
    if buildroot is not None:
        libpath = dopath(buildroot)

    try:
        import polyadicts as _pd
    except ImportError:
        raise SystemExit(
            'fatal: polyadicts is missing!\n'
            '\tbuildroot: %r\n'
            '\tlibpath: %r\n' %
            (buildroot, libpath))
    else:
        pd = _pd

    test_ntuple_from_bytes()
    test_ntuple_from_sequence()
    test_ntuple_from_other()
    test_ntuple_range()
    test_ntuple_erange()
    test_ntuple_einval()

    test_polyad_from_bytes()
    test_polyad_from_sequence()
    test_polyad_from_other()
    test_polyad_einval()
    test_polyad_enomem()

def dopath(buildroot):
    path = '%(buildroot)s/lib.%(lsystem)s-%(machine)s-%(version)s' % dict(
        buildroot=buildroot,
        lsystem=platform.system().lower(),
        machine=platform.machine(),
        version='.'.join(platform.python_version_tuple()[:2]),
        )
    sys.path.append(path)
    
def assert_raises(err, f, *args, **kwds):
    ret = None
    try:
        ret = f(*args, **kwds)
    except err:
        pass
    except Exception:
        if err:
            raise
    else:
        if err:
            raise AssertionError("did not raise %r" % err)
    return ret

def test_ntuple_from_bytes():
    b = b'\x01\x00'
    t = pd.ntuple(b)
    t = pd.ntuple(b)
    assert(b == pd.ntuple(t))
    assert(1 == len(t))
    assert(0 == t[0])
    b = b'\x04\x00\x01\x02\x03'
    t = pd.ntuple(b)
    assert(b == pd.ntuple(t))
    assert(4 == len(t))
    for i in range(4):
        assert(i == t[i])

def test_ntuple_from_sequence():
    b = pd.ntuple([42])
    assert(2 == len(b))
    assert(1 == b[0])
    assert(42 == b[1])
    b = pd.ntuple([1,1,2,3,5,8])
    assert(7 == len(b))
    assert(b'\x06\x01\x01\x02\x03\x05\x08' == b)
    b = pd.ntuple(range(4))
    assert(tuple(range(4)) == pd.ntuple(b))

def test_ntuple_from_other():
    assert_raises(TypeError, pd.ntuple, None)
    assert_raises(TypeError, pd.ntuple, 'foo')
    assert_raises(TypeError, pd.ntuple, ['hello', 'world'])

def test_ntuple_range():
    n = (1 << 56) - 1
    b = pd.ntuple([n])
    assert(b'\x01' + b'\xff' * 7 + b'\x7f' == b)
    t = pd.ntuple(b)
    assert(1 == len(t))
    assert(n == t[0])
    assert(b == pd.ntuple(t))

def test_ntuple_erange():
    assert_raises(OverflowError, pd.ntuple, [1 << 64])
    assert_raises(OverflowError, pd.ntuple, b'\x01' + 9 * b'\xff' + b'\x01')

    b = b'\x01' + 8 * b'\x80' + b'\x01'
    t = pd.ntuple(b)
    assert(t[0] == (1 << 56))
    t = pd.ntuple([t[0]])
    assert(b == bytes(t))

    b = b'\x01' + 8 * b'\xff' + b'\x7f'
    t = pd.ntuple(b)
    assert(t[0] == ((1 << 63) - 1))
    t = pd.ntuple([t[0]])
    assert(b == bytes(t))

def test_ntuple_einval():
    assert_raises(ValueError, pd.ntuple, b'\x01')
    assert_raises(ValueError, pd.ntuple, b'\x01\xff')

def test_polyad_from_bytes():
    b = b'\x05\x05helloworld'
    p = pd.polyad(b)
    assert(b == bytes(p))
    assert(2 == len(p))
    assert('hello' == str(p[0], 'ascii'))
    assert('world' == str(p[1], 'ascii'))

def test_polyad_from_sequence():
    s = [bytes(x, 'ascii') for x in ('hello', 'world')]
    p = pd.polyad(s)
    assert(2 == len(p))
    assert('hello' == str(p[0], 'ascii'))
    assert('world' == str(p[1], 'ascii'))
    assert(b'\x05\x05helloworld' == bytes(p))
    g = (bytes(x, 'ascii') for x in ('hello', 'world'))
    p = pd.polyad(g)
    assert(s == list(p))

def test_polyad_from_other():
    assert_raises(TypeError, pd.polyad, None)
    assert_raises(TypeError, pd.polyad, 1)
    assert_raises(TypeError, pd.polyad, 'foo')
    assert_raises(TypeError, pd.polyad, range(2))
    assert_raises(TypeError, pd.polyad, ['hello', 'world'])

def test_polyad_einval():
    assert_raises(ValueError, pd.polyad, b'\x05')
    for i in range(100):
        assert_raises(ValueError, pd.polyad, b'\xff' * 8)

def test_polyad_enomem():
    from resource import getrlimit, getrusage, setrlimit
    from resource import RLIMIT_AS
    n = 16 * (1 << 20)
    b = b'\x00' * n

    soft, hard = getrlimit(RLIMIT_AS)
    setrlimit(RLIMIT_AS, (n, hard))
    try:
        assert_raises(MemoryError, pd.polyad, [b])
    finally:
        setrlimit(RLIMIT_AS, (soft, hard))

if __name__ == '__main__':
    main(*sys.argv[1:])
