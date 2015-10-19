
## Description

The `polyadicts` C library and Python module implements a simplistic and
powerful API for encapsulation of arbitrary binary data within *n*-tuples.

LISP fans will be well-versed with the representability of data and
functions as tuples. Python has always had support for tuples as a basic
data structure. Unfortunately, no equivalent binary encapsulation has been
incorporated to the standard library.

It is the authors belief that a bare-bones *n*-tuple implementation will
provide a solid foundation for simple and beautiful encapsulation. Many
other options exist, such as JSON, XML, pickling, struct, protobufs, etc..
`polyadicts` aims to be simpler than these at its core but rich in
expressiveness.

The binary format makes use of 7-bit variable length unsigned integers for
a compact representation. Effort has been made to reduce memory allocation
and copying when loading data from buffers.

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
which this warning will be removed but no warranty will be provided.
