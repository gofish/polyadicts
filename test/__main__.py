#!/usr/bin/env python3
import os
import platform
import sys

def main(buildroot='build'):
    if buildroot is not None:
        dopath(buildroot)

    global pd, libpath
    try:
        import polyadicts as pd
    except ImportError:
        raise
        raise SystemExit(
                'fatal: polyadicts is missing!\n'
                '\tbuildroot: %r\n'
                '\tlibpath: %r\n' %
                (buildroot, libpath))

    test_polyid_from_bytes()
    test_polyid_from_sequence()
    test_polyid_range()
    test_polyid_erange()
    test_polyid_einval()

    test_polyad_from_bytes()
    test_polyad_from_sequence()
    test_polyad_einval()

def dopath(buildroot):
    global libpath
    lsystem = platform.system().lower()
    machine = platform.machine()
    version = '.'.join(platform.python_version_tuple()[:2])
    libpath = '%(buildroot)s/lib.%(lsystem)s-%(machine)s-%(version)s'
    libpath %= locals()
    sys.path.append(libpath)

def test_polyid_from_bytes():
    b = b'\x01\x00'
    p = pd.polyid(b)
    assert(b == bytes(p))
    assert(1 == len(p))
    assert(0 == p[0])
    b = b'\x04\x00\x01\x02\x03'
    p = pd.polyid(b)
    assert(b == bytes(p))
    assert(4 == len(p))
    for i in range(4):
        assert(i == p[i])

def test_polyid_from_sequence():
    p = pd.polyid([42])
    assert(1 == len(p))
    assert(42 == p[0])
    p = pd.polyid([1,1,2,3,5,8])
    assert(6 == len(p))
    a, b = 1, 1
    for i in range(6):
        assert(a == p[i])
        a, b = b, a + b

def test_polyid_range():
    n = (1 << 56) - 1
    b = b'\x01\xff\xff\xff\xff\xff\xff\xff\x7f'
    p = pd.polyid([n])
    assert(1 == len(p))
    assert(n == p[0])
    assert(b == bytes(p))
    p = pd.polyid(b)
    assert(1 == len(p))
    assert(n == p[0])
    assert(b == bytes(p))

def test_polyid_erange():
    try:
        p = pd.polyid([1 << 64])
    except OverflowError:
        pass
    else:
        assert(False)
    try:
        p = pd.polyid([1 << 56])
    except OverflowError:
        pass
    else:
        assert(False)
    try:
        p = pd.polyid(b'\x01\xff\xff\xff\xff\xff\xff\xff\xff\x01')
    except OverflowError:
        pass
    else:
        assert(False);

def test_polyid_einval():
    try:
        p = pd.polyid(b'\x01')
    except ValueError:
        pass
    else:
        assert(False)
    try:
        p = pd.polyid(b'\x01\xff')
    except ValueError:
        pass
    else:
        assert(False)

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

def test_polyad_einval():
    try:
        p = pd.polyad(b'\x05')
    except ValueError:
        pass
    else:
        assert(False)

if __name__ == '__main__':
    main(*sys.argv[1:])
