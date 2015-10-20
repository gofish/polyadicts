
## Overview

The `polyadicts` C library and Python module implements a simple API for
encapsulation of arbitrary binary data within *n*-tuples.

The binary format makes use of 7-bit variable length unsigned integers
(varint). Effort has been made to reduce memory allocation and copying on
reads via field indexing and shared buffers.

## Building

Uses Python3 and setuptools.

    $ make

The above will execute the following.

    $ python3 setup.py build

This should produce the `polyadicts` Python module under `build/`.

e.g.
: `build/lib.linux-x86_64-3.4/polyadicts.cpython-34m.so`

## Notes

Some (limited) tests are implemented for basic wire format checks.

This project is not considered safe to pack or unpack untrusted data.
A thorough review is planned for the latest API implementation, after
which this warning will be removed, but no warranty will be provided.

## Rational

A bare-bones *n*-tuple implementation can provide a solid foundation for
simple and beautiful data encapsulation. Many other formats exist already,
such as JSON, XML, pickling, struct, protobufs, etc.. `polyadicts` is
simpler than most of these at its core while remaining rich in
expressiveness, compact in representation, and efficient overall.

JSON with its spare format supports an incredibly rich encapsulation API.
It is compact, human-readable, and composable. `polyadicts` cannot replace
JSON but instead implements a similar compact, machine-readable, and
composable binary format. It is intended that data representable in one
format is also representable in the other, and vice versa, but they are not
necessarily directly convertible.

`polyadicts` uses a binary format in order to obtain faster processing and
a better pre-compression storage ratio than JSON. The format includes a
field index that allows for random access to or skipping of elements within
a tuple. Unlike JSON the format makes no distinction between lists (tuples)
and objects (maps). The latter may be represented instead as a *k*-tuple of
2-tuples, or as a 2-tuple of *k*-tuples, without loss of generality.

## Format

The `ntuple` is a fixed ordered sequence of unsigned integers (natural
numbers). The binary format for an `ntuple` is written as the rank -- i.e.
*n*, the number of elements in the tuple -- followed by the *n* numbers
themselves, all written as 7-bit varints.

    (0, 1, 2, 3) <=> b'\x04\x00\x01\x02\x03'
    (0, 64, 128) <=> b'\x03\x00\x40\x80\x01'

Similarly, a `polyad` is a fixed ordered sequence of binary data segments.
The binary format consists of a valid `ntuple` header specifying the rank
and *n* size values, one for each binary segment the `polyad` contains.
The binary segments follow the header directly in order without padding.

    (b'hello', b'world') <=> b'\x02\x05\x05helloworld'

The `ntuple` supports unsigned integers that can fit into a `size_t` type.
For 64-bit programs up to 9 bytes of 7-bit varints are supported, handling
all unsigned values from 0 to 63 bits. For 32-bit programs, up to 4 bytes
are permitted, handling unsigned values from 0 to 28 bits. Negative values
may be stored using zig-zag encoding:

    >>> tuple(map(zig, range(-3, 4)))
    (5, 3, 1, 0, 2, 4, 6)
    >>> tuple(map(zag, _))
    (-3, -2, -1, 0, 1, 2, 3)

The `polyad` type shares buffers on reads, provides zero-copy access to
each binary element, and is fully composable. In Python, the `polyad` is
a cross between `tuple` and `bytes`. The `len()` operator will return the
rank of the polyad and the index operator (`p[0]`) will return a memory
view (buffer) for the specified element. When a `polyad` is treated as a
buffer however, e.g. via `bytes` or `IOBase.write()`, the polyad exposes
its own buffer as a memory view, including the `ntuple` header.

    >>> p = polyad((b'hello', b'world'))
    >>> len(p)
    2
    >>> bytes(p[0])
    b'hello'
    >>> bytes(p[1]))
    b'world'
    >>> len(memoryview(p))
    13
    >>> bytes(p)
    b'\x02\x05\x05helloworld'
