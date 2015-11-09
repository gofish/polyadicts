#!/usr/bin/env python3
import platform
import struct
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

    test_zig()
    test_zag()
    test_varyad()
    test_varyad_default()
    test_varyad_to_polyad()

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
    b = b'\x02\x05\x05helloworld'
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
    assert(b'\x02\x05\x05helloworld' == bytes(p))
    g = [bytes(x, 'ascii') for x in ('hello', 'world')]
    p = pd.polyad(g)
    assert(s == list(p))
    p = pd.polyad(('hello', 'world'))
    assert(g == list(p))
    p = pd.polyad('foo')
    assert(3 == len(p))
    assert(b'foo' == b''.join(map(bytes, p)))

def test_polyad_from_other():
    assert_raises(TypeError, pd.polyad, None)
    assert_raises(TypeError, pd.polyad, 1)
    assert_raises(TypeError, pd.polyad, range(2))

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

def zigrange(start, stop, *vargs):
    step = 1
    if len(vargs) > 1:
        raise TypeError("expected at most 3 arguments, got {}".format(2 + len(vargs)))
    if vargs:
        step, = vargs
    for i in range(start, stop, step):
        if i < 0:
            yield(-2 * i - 1)
        else:
            yield(2 * i)

def zagrange(start, stop, *vargs):
    step = 1
    low = min(0, start, stop)
    if low:
        raise TypeError("OverflowError: can't convert negative {} to unsigned".format(type(low).__name__))
    if len(vargs) > 1:
        raise TypeError("expected at most 3 arguments, got {}".format(2 + len(vargs)))
    if vargs:
        step, = vargs
    for i in range(start, stop, step):
        if i % 2:
            yield(-i // 2)
        else:
            yield(i // 2)

def test_zig():
    assert(0 == pd.zig( 0))
    assert(1 == pd.zig(-1))
    assert(2 == pd.zig( 1))
    assert((0, 1, 2) == pd.zig(0, -1, 1))
    assert((0, 1, 2) == pd.zig((0, -1, 1)))
    def test(l, h):
        assert(tuple(zigrange(l, h)) == pd.zig(range(l, h)))
    test(-50, 50)
    test((-1 << 63), (-1 << 63) + 100)
    test((1 << 63) - 100, (1 << 63))

def test_zag():
    assert( 0 == pd.zag(0))
    assert(-1 == pd.zag(1))
    assert( 1 == pd.zag(2))
    assert((0, -1, 1) == pd.zag(0, 1, 2))
    assert((0, -1, 1) == pd.zag((0, 1, 2)))
    def test(l, h):
        assert(tuple(zagrange(l, h)) == pd.zag(range(l, h)))
    test(0, 100)
    test((1 << 64) -10, (1 << 64))

def test_varyad():
    v = pd.varyad(0)
    assert(0 == len(v))
    b = bytes(v)
    assert(16 == len(b))
    assert((0, 16) == struct.unpack("PP", b[:16]))
    v.push(b'')
    assert(1 == len(v))
    assert(0 == len(v[0]))
    b = bytes(v)
    assert(32 == len(b))
    assert((1, 32) == struct.unpack("PP", b[:16]))
    v.push(b'hello')
    assert(2 == len(v))
    assert(5 == len(v[1]))
    assert(b'hello' == bytes(v[1]))
    b = bytes(v)
    assert(64 == len(b))
    assert((2, 64) == struct.unpack("PP", b[:16]))

def test_varyad_default():
    v = pd.varyad()
    assert(0 == len(v))
    b = bytes(v)
    assert(512 == len(b))
    assert((0, 512) == struct.unpack("PP", b[:16]))
    assert(b[:16] + (496 * b'\x00') == b)

def test_varyad_to_polyad():
    v = pd.varyad(16)
    v.push(b'hello')
    v.push(b'world')
    p = pd.polyad(tuple(v))
    assert(2 == len(p))
    assert(b'hello' == bytes(p[0]))
    assert(b'world' == bytes(p[1]))
    assert(b'\x02\x05\x05helloworld' == bytes(p))

if __name__ == '__main__':
    main(*sys.argv[1:])
