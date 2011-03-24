#!/usr/bin/env python3
import os
import platform
import sys

def main(buildroot='build'):
    if buildroot is not None:
        dopath(buildroot)

    global pd
    try:
        import polyadicts as pd
    except ImportError:
        raise SystemExit('fatal: polyadicts is missing!\n\tbuildroot: %r' %
                buildroot)

    test_polyid_from_bytes()
    test_polyid_from_sequence()
    test_polyad_from_bytes()
    test_polyad_from_sequence()

def dopath(buildroot):
    lsystem = platform.system().lower()
    machine = platform.machine()
    version = '.'.join(platform.python_version_tuple()[:2])
    libpath = '%(buildroot)s/lib.%(lsystem)s-%(machine)s-%(version)s'
    sys.path.append(libpath % locals())

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

if __name__ == '__main__':
    main(*sys.argv[1:])
