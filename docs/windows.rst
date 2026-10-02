.. Copyright (c) 2026 RBR Ltd.
.. SPDX-License-Identifier: Apache-2.0

Build and Run libRBR on Windows
===============================

libRBR can be built and run on Windows under two options: WSL2, or `Cygwin <https://cygwin.com/>`__.

WSL2
----

Prerequisits
~~~~~~~~~~~~

In addition to the base WSL2 installation, the following additional packages are required:

- ``gcc`` and set CC=gcc to compile the library.
- ``linux-tools-virtual hwdata`` to forward the port. (Reference: https://github.com/dorssel/usbipd-win/wiki/WSL-support)

Optionally, you can install:

- ``doxygen``, ``python3`` and ``pip`` to compile the library documentation,
  along with the Python packages listed in ``docs/requirements.txt``.

Cygwin
------

Prerequisites
~~~~~~~~~~~~~

In addition to the base Cygwin install,
you'll need to install these additional packages
(and their dependencies):

- ``make`` to build the project.
- ``gcc-core`` or ``clang`` to compile the library.

Optionally, you can install:

- ``doxygen``, ``python3`` and ``pip`` to compile the library documentation,
  along with the Python packages listed in ``docs/requirements.txt``.
- ``git``, to retrieve the project source.
- ``libSDL2-devel`` to successfully compile
  the ``posix-stream-sdl`` example.
  Note that the example does not launch under Cygwin;
  installing the library and development headers
  serves only to avoid compilation failures
  when building all of the examples.
