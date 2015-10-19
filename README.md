
## Building

Uses Python3 and setuptools. See Makefile and setup.py for more.

The following should be enough to produce a shared library implementing the `polyadicts Python module under `build`/

    $ python3 setup.py build

On Ubuntu 15.04 with Python 3.4, this appears as

    build/lib.linux-x86_64-3.4/polyadicts.cpython-34m.so

## Notes

Some (limited) tests are implemented for basic wire format checks. In general,
this project is not considered safe to pack or unpack untrusted data until a
thorough review can be performed on the latest API implementation.
