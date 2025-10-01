# Cygwin

libRBR can be built and run on Windows under [Cygwin].

[Cygwin]: https://cygwin.com/

## Prerequisites

In addition to the base Cygwin install,
you'll need to install these additional packages
(and their dependencies):

* `make` to build the project.
* `gcc-core` or `clang` to compile the library.

Optionally, you can install:

* `doxygen` to compile the library documentation.
* `git`, to retrieve the project source.
* `libSDL2-devel` to successfully compile
  the `posix-stream-sdl` example.
  Note that the example does not launch under Cygwin;
  installing the library and development headers
  serves only to avoid compilation failures
  when building all of the examples.
