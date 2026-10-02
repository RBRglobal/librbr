.. Copyright (c) 2026 RBR Ltd.
.. SPDX-License-Identifier: Apache-2.0

libRBR
======

Overview
--------

libRBR provides an interface
for simplified communication
with RBR instruments.
The library isolates the user
from the low-level details
of instrument communication
by wrapping each instrument command
in a function with fully typed arguments.
Familiarity with the instrument command set
is still required
in order to know which commands to send,
but the library handles the intricacies
of waking the instrument,
response parsing,
etc.
Command references
and instrument documentation
can be found at https://docs.rbr-global.com/.

For example:

.. code-block:: c

   RBRGen3Sampling sampling;
   RBRGen3_getSampling(instrument, &sampling);
   printf("The instrument is performing %s sampling every %" PRIi32 "ms.\n",
          RBRGen3SamplingMode_name(sampling.mode),
          sampling.period);

As of version 2.0.0,
the library contains two independent APIs:
the Gen3 API
(``RBRGen3…``,
for Logger2/Logger3 instruments;
this is libRBR 1.x with a few major API changes)
and the Gen4 API
(``RBRGen4…``,
for Generation 4 instruments).
Applications choose the API to use per instrument;
both generations are compiled
into the same library
by default
(see the Building section below).

The Gen3 API also offers basic parsing
for EasyParse sample data and events.

The library tries to be platform-agnostic.
It targets C99
and should be compilable
on any compliant compiler.

If you happen to have stumbled across this file
in a source tree somewhere,
you might be interested to know
that this project is maintained
`on Bitbucket <https://bitbucket.org/rbr/librbr>`__.

Support
-------

The library is intended to support
the following firmware types and versions.
The library should have good forwards compatibility
as breaking changes between firmware versions
are rare and made only when absolutely necessary.

Gen3
~~~~

======================= ========== ================
Firmware Type           Generation Firmware Version
======================= ========== ================
103 (Logger2, standard) Early 2015 v1.440
104 (Logger3, standard) Late 2017  v1.102 and up
======================= ========== ================

Gen4
~~~~

=======================  ================
Firmware Type            Gen4 API Version
=======================  ================
150 (Logger4, standard)  2.1
=======================  ================

The standalone dynamic correction library
is intended to be generation-agnostic.

Coming from libRBR 1.x?
-----------------------

The Gen3 side of libRBR 2.x is an updated version of libRBR 1.2.4.

Major changes include

- Naming: the ``RBRInstrument`` prefix found on most identifiers
  was shortened to ``RBRGen3``.
- The constructors ``RBRGen3_open()`` and ``RBRGen3Parser_init()``
  no longer offer the ability to dynamically allocate memory.
  libRBR has no dependency on ``malloc``, ``calloc``, ``realloc``, or ``free``.
- Buffers throughout the library are now provided by the user
  rather than being sized statically at compile time.
  This includes the command buffer,
  the response buffer,
  and buffers for pools of objects (e.g. channels)
  that can vary in size based on the specific instrument.

The Gen4 side of libRBR 2.x
is structurally similar to its Gen3 counterpart,
but uses Gen4 parsing logic
and models the Gen4 command set.

Building
--------

The library can be built with GNU Make.
For documentation on Make targets,
see the ``Makefile``.

Both instrument generation APIs
are compiled into ``bin/libRBR.a`` by default.
The ``GEN3`` and ``GEN4`` Makefile options
select the generations to include:

.. code-block:: sh

   # Both generations (the default)
   make lib
   # Gen3 only
   make GEN4=0 lib
   # Gen4 only
   make GEN3=0 lib

At least one generation must be enabled;
the ``RBRCommon`` helpers are compiled regardless.

Library compilation requires a C99-compliant C compiler.
The library makes a few assumptions
about its host platform.
For details, see `the documentation on porting`_.

API document compilation requires `Doxygen <http://doxygen.org/>`__
and the Python packages listed in ``docs/requirements.txt``.

Platform-specific instructions and advice
are available:

- Windows_

In most cases,
the library can be built
in a few steps.

First, check out the code with Git:

.. code-block:: sh

   git clone https://bitbucket.org/rbr/librbr.git

Option 1: Build libRBR with the Dynamic Correction module
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Assuming a Linux shell is used (Cygwin or WSL on Windows).

.. code-block:: sh

   cd path/to/librbr/

   # Build and execute tests
   # Also builds the library if necessary
   make tests
   # Build the documentation
   make docs
   # Does all of the above
   make all

Examples can be built similarly:

.. code-block:: sh

   # Build the Gen3 examples
   # The other examples can be built the same way
   cd examples/posixGen3
   make all

Most examples are intended to interact
with a real RBR instrument.
Be aware that some will write / reconfigure the instrument.
For an instrument connected via USB,
an example might be run like:

.. code-block:: sh

   # Change '/dev/ttyACM0' below to the desired port/device
   ./posix-stream /dev/ttyACM0

The ``posix-parse-file-dynamiccorrection`` example
in ``examples/posixGen3``
requires a dataset rather than an instrument.
Run it with the provided sample data:

.. code-block:: sh

   ./posix-parse-file-dynamiccorrection ../sampledata/dynamiccorrection-sample.bin 4

Option 2: Build the Dynamic Correction module standalone
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Assuming a Linux shell is used (Cygwin or WSL on Windows).

.. code-block:: sh

   cd path/to/librbr

   # Build just the standalone dynamic correction library
   make libdynamiccorrection

   # Build the example
   cd examples/dynamicCorrection
   make
   # Test with the example file
   ./dynamicCorrection-example ../sampledata/dynamiccorrection-sample.csv

Option 3: Build libRBR without the Dynamic Correction module
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Assuming a Linux shell is used (Cygwin or WSL on Windows).

.. code-block:: sh

   cd path/to/librbr

   # Build just the core Gen3/Gen4 library
   make lib

Using
-----

See `the introduction`_
for an overview of library conventions.

API documentation is built into the ``docs/_build/html/`` subdirectory.
Prebuilt API documentation corresponding to the latest release
is available at https://docs.rbr-global.com/librbr/\ **.**

For examples,
please see the ``examples/`` subdirectory.

You may prefer to integrate
the entire library source
into your codebase
rather than link against the library.
In that case,
we strongly suggest doing so
via a `Git submodule <https://git-scm.com/docs/git-submodule>`__
where possible
to make updating easier.

Contributing
------------

The library is primarily maintained by RBR
and development is directed by our needs
and the needs of our `OEM <https://rbr-global.com/products/oem>`__ customers.
However, we're happy to take contributions_ generally.

License
-------

This project is licensed under the terms
of the Apache License, Version 2.0;
see https://www.apache.org/licenses/LICENSE-2.0.

- The license is not “viral”.
  You can include it
  either as source
  or by linking against it,
  statically or dynamically,
  without affecting the licensing
  of your own code.
- You do not need to include RBR's copyright notice
  in your documentation,
  nor do you need to display it
  at program runtime.
  You must retain RBR's copyright notice
  in library source files.
- You are under no legal obligation
  to share your own modifications
  (although we would appreciate it
  if you did so).
- If you make changes to the source,
  in addition to retaining RBR's copyright notice,
  you must add a notice stating that you changed it.
  You may add your own copyright notices.

.. Links below are repository-relative; the Sphinx build (docs/) redefines them.

.. _the documentation on porting: docs/porting.rst
.. _Windows: docs/windows.rst
.. _the introduction: docs/introduction.rst
.. _contributions: CONTRIBUTING.rst
