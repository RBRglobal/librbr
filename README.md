# libRBR

## Introduction

libRBR provides an interface
for simplified communication
with RBR instruments.
The library isolates the user
from the low-level details
of instrument communication
by wrapping each instrument command
in a function with fully typed arguments.
Familiarity with the [**instrument command set**](https://docs.rbr-global.com/L3commandreference)
is still required
in order to know which commands to send,
but the library handles the intricacies
of waking the instrument,
response parsing,
etc.

As of version 2.0.0,
the library contains two independent APIs,
one per instrument generation:
the Gen3 API
(`RBRInstrumentGen3_…`,
for Logger2/Logger3 instruments,
the libRBR 1.x API with every identifier suffixed `Gen3`)
and the Gen4 API
(`RBRGen4…`,
for Generation 4 instruments,
under active development).
Applications choose the API to use per instrument;
both generations are compiled
into the same library
by default
(see the Building section below).

For example:

~~~{.c}
RBRGen3Sampling sampling;
RBRGen3_getSampling(instrument, &sampling);
printf("The instrument is performing %s sampling every %" PRIi32 "ms.\n",
       RBRGen3SamplingMode_name(sampling.mode),
       sampling.period);
~~~

The library also offers basic parsing
for [EasyParse] sample data and events.

The library tries to be platform-agnostic.
It targets C99
and should be compilable
on any compliant compiler.

If you happen to have stumbled across this file
in a source tree somewhere,
you might be interested to know
that this project is maintained
[on Bitbucket].

[instrument command set]: https://docs.rbr-global.com/L3commandreference
[EasyParse]: https://docs.rbr-global.com/L3commandreference/format-of-stored-data/overview/easyparse-format
[on Bitbucket]: https://bitbucket.org/rbr/librbr





## Support

The library is intended to support
the following firmware types and versions.
The library should have good forwards compatibility
as breaking changes between firmware versions
are rare and made only when absolutely necessary.

| Firmware Type           | Generation | Version |
| ----------------------- | ---------- | ------- |
| 103 (Logger2, standard) | Early 2015 |  v1.440 |
| 104 (Logger3, standard) |  Late 2017 |  v1.102 and up |
| 130/131 (RBRsolo⁴)      |       Gen4 | in development |
| 140 (SEN⁴)              |       Gen4 | in development |
| 150 (Logger4, standard) |       Gen4 | in development |

The Gen4 API targets the Generation 4 instrument command set;
it is under active development
and its surface may still change.


The standalone dynamic correction library supports:

| Firmware Type           | Generation | Version |
| ----------------------- | ---------- | ------- |
| 104 (Logger3, standard) |  Late 2021 |  v1.136 and up |


## Building

The library can be built with GNU Make.
For documentation on Make targets,
see the [Makefile].

Both instrument generation APIs
are compiled into `bin/libRBR.a` by default.
The `GEN3` and `GEN4` Makefile options
select the generations to include:

~~~{.sh}
# Both generations (the default):
$ make lib
# Gen3 only:
$ make GEN4=0 lib
# Gen4 only:
$ make GEN3=0 lib
~~~

At least one generation must be enabled.
Coming from libRBR 1.2.x?
The Gen3 API is the 1.x API
with `Gen3` appended to every file name
and every `RBRInstrument`, `RBRParser`,
and `RBRDynamicCorrection` identifier
(`RBRInstrument_open()` is now `RBRGen3_open()`,
`RBRINSTRUMENT_SUCCESS` is now `RBRGEN3_SUCCESS`,
and `RBRInstrument.h` is now `RBRGen3.h`);
the behaviour is unchanged.
When building as a Zephyr module,
the equivalent Kconfig options are
`CONFIG_LIBRBR_GEN3` and `CONFIG_LIBRBR_GEN4`.

Library compilation requires a C99-compliant C compiler;
The library makes a few assumptions
about its host platform.
For details, see [the documentation on porting][porting].

API document compilation requires [Doxygen].

Platform-specific instructions and advice
are available:

* [Cygwin]

[Makefile]: Makefile.html
[porting]: porting.md
[Doxygen]: http://doxygen.org/
[Cygwin]: cygwin.md

In most cases,
the library can be built
in a few steps.

First, check out the code with Git:

~~~{.sh}
$ git clone https://bitbucket.org/rbr/librbr.git
$ cd <PATH>/librbr
~~~

### option 1 (recommended): build librbr with dynamic correction feature
Assuming cygwin is used, and current path is `<PATH>/librbr`.
Then use either `make tests` or `make all` to build the libraries:
~~~{.sh}
# Build and execute tests. Also builds the library if necessary:
$ make tests
# Build Doxygen documentation:
$ make docs
# Does all of the above. Build both libraries - librbr and libRBRDynamicCorrection:
$ make all
~~~

continue with commands below if one wants to use the posix example with dynamic correction:
~~~{.sh}
$ cd <PATH>/librbr/examples/posixGen3
# Build all the posix example:
# (ignore errors if any)
$ make all
~~~

To test posix-parse-file-dynamiccorrection example:
~~~{.sh}
$ ./posix-parse-file-dynamiccorrection ../sampledata/dynamiccorrection-sample.bin 4
~~~

or if one wants to try posix-streaming-dynamiccorrection example, use commands below:
~~~{.sh}
# first connect USB, get the port:
$ ls /dev/tty*
/dev/tty /dev/ttyS<number>

# this command would make the instrument start streaming:
$ ./posix-stream-dynamiccorrection /dev/ttyS<number>
~~~

or test with .csv file:
~~~{.sh}
$ cd <PATH>/librbr/examples/dynamicCorrectionGen3
# Build the example:
$ make
# Test with the example file:
$ ./dynamicCorrection-example ../sampledata/dynamiccorrection-sample.csv
~~~

### option 2: build standalone dynamic correction library only
Assuming cygwin is used, and current path is `<PATH>/librbr`:
~~~{.sh}
# Build just the standalone dynamic correction library:
$ make libdynamiccorrection

# Continue with commands below if one wants to use the example provided:
$ cd <PATH>/librbr/examples/dynamiccorrection
# Build the example:
$ make
# Test with the example file:
$ ./dynamicCorrection-example ../sampledata/dynamiccorrection-sample.csv
~~~


### option 3: build libRBR without dynamic correction feature
Assuming cywin is used, current path is `<PATH>/librbr`.
Then commands below shows how to use the library:

~~~{.sh}
# Build just the library - libRBR:
$ make lib

# Continue with commands below if one wants to use the posix example:
$ cd <PATH>/librbr/examples/posixGen3
# Build the exmamples:
$ make example
~~~

Take streaming as example:
~~~{.sh}
# First connect USB, get the port:
$ ls /dev/tty*
/dev/tty /dev/ttyS<number>
# This command would make the instrument start streaming:
$ ./posix-stream /dev/ttyS<number>
~~~

### option 4: build librbr with dynamic correction feature, but no malloc() used
Assuming cygwin is used, and current path is `<PATH>/librbr`:
~~~{.sh}
# Build both librbr and libRBRDynamicCorrection:
$ make nomalloc

# Note: If this is used, please build posix examples with "$ make nomalloc" too.
~~~

## Using

See [the introduction]
for an overview of library conventions.

API documentation is built into the `docs/` subdirectory.
Prebuilt API documentation corresponding to the latest release
is available at **https://docs.rbr-global.com/librbr/.**

For examples,
please see the `examples/` subdirectory.

You may prefer to integrate
the entire library source
into your codebase
rather than link against the library.
In that case,
we strongly suggest doing so
via a [Git submodule]
where possible
to make updating easier.

[the introduction]: introduction.md
[Git submodule]: https://git-scm.com/docs/git-submodule

## Contributing

The library is primarily maintained by RBR
and development is directed by our needs
and the needs of our [OEM] customers.
However, we're happy to take [contributions] generally.

[OEM]: https://rbr-global.com/products/oem
[contributions]: CONTRIBUTING.md

## License

This project is licensed under the terms
of the Apache License, Version 2.0;
see https://www.apache.org/licenses/LICENSE-2.0.

* The license is not “viral”.
  You can include it
  either as source
  or by linking against it,
  statically or dynamically,
  without affecting the licensing
  of your own code.
* You do not need to include RBR's copyright notice
  in your documentation,
  nor do you need to display it
  at program runtime.
  You must retain RBR's copyright notice
  in library source files.
* You are under no legal obligation
  to share your own modifications
  (although we would appreciate it
  if you did so).
* If you make changes to the source,
  in addition to retaining RBR's copyright notice,
  you must add a notice stating that you changed it.
  You may add your own copyright notices.
