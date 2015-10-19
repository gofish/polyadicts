
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
