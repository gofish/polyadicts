
## Building

Uses Python3 and setuptools.  See Makefile and setup.py for more.  The following should be enough:

    $ python3 setup.py build

This should produce a shared library implementing the `polyadicts` Python
module under `build/`.

e.g.
:  `build/lib.linux-x86_64-3.4/polyadicts.cpython-34m.so`.

## Tests

Some (limited) tests are implemented for basic wire format checks.

This project is not yet considered safe to pack or unpack untrusted data.

## Bugs

There is inconsistency between the wire formats of the two basic tuple types.
The more recently added type, `polyid`, contains a leading element count. This
has not been implemented in the `polyad` type yet, which relies entirely on the
supplied buffer length argument. There is some benefit to having the wire
format fully specify it's own length as well as its data.

After this change, each `polyad` on the wire will consist of a valid `polyid`
as header followed by the relevant data segments of each tuple entry.
