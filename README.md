
## Overview

The `polyadicts` C library and Python module provides an elemental API for
encapsulation of binary data as packed *n*-tuples. The format makes use of
7-bit variable length unsigned integers (varints) for size information.

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

## Format

The `ntuple` function maps an ordered sequence of unsigned integers
(natural numbers) to 7-bit varints in binary form, or vice-versa. The
first number in the sequence is preceded by the "rank", i.e. *n* - the
number of elements in the tuple, followed by the *n* numbers themselves.

    (0, 1, 2, 3) <=> b'\x04\x00\x01\x02\x03'
    (0, 64, 128) <=> b'\x03\x00\x40\x80\x01'

The `ntuple` supports unsigned integers that can fit into a `size_t` type.
For 64-bit programs up to 9 bytes of 7-bit varints are supported, handling
all unsigned values from 0 to 63 bits. For 32-bit programs, up to 4 bytes
are permitted, handling unsigned values from 0 to 28 bits. Negative values
may be stored using zig-zag encoding:

    >>> tuple(map(zig, range(-3, 4)))
    (5, 3, 1, 0, 2, 4, 6)
    >>> tuple(map(zag, _))
    (-3, -2, -1, 0, 1, 2, 3)

The `polyad` format is a fixed ordered sequence of binary elements, stored
in a contiguous buffer. The format consists of a valid `ntuple` header that
specifies the rank and the *n* binary element sizes. Element data follows
immediately after the header, in order and without padding.

    (b'hello', b'world') <=> b'\x02\x05\x05helloworld'

The `polyad` type shares buffers on reads, provides access to each element
data vector, and is fully composable. In Python, the `polyad` implements
both the sequence and buffer APIs. The `len()` operator will return the
rank of the polyad and the index operator (`p[0]`) will return a memory
view (buffer) for the specified element. When a `polyad` is passed to a
function expecting a buffer (e.g. `memoryview()`, `bytes()` or
`IOBase.write()`) the polyad exposes its entire backing buffer, including
the `ntuple` header.

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
