<!-- Copyright (c) 2026 RBR Ltd. -->
<!-- SPDX-License-Identifier: Apache-2.0 -->

# How to use the Gen4 posix examples

## Setup
* Hardware: an RBR Generation 4 instrument (SL4, SEN4, or L4) with a
  serial or USB connection
* Runtime environment: Linux, macOS, or Cygwin

Not all examples require hardware (`posix-footprint` runs standalone).

## Build all posix examples
Assuming libRBR is already built with Gen4 support
(if not, go to the librbr directory, then run `make GEN4=1`).
Go to the librbr/examples/posixGen4 directory, then run `make`.

## Tips before you start
(1) Check the baud rate:
The examples open the serial port at 115200 baud, the instrument default. If
your instrument is configured for another rate, change the `cfsetospeed()`
call in `posix-shared.c` to match (e.g. `B9600`). The baud rate is
irrelevant over a USB connection.

(2) Confirm which port is in use:
On Linux the instrument usually appears as `/dev/ttyUSB0` or `/dev/ttyACM0`.
In Cygwin, if a terminal tool suggests `COM6`, it's most likely `/dev/ttyS5`;
alternatively, run `ls /dev/ttyS*` and try each one.

(3) How to clean the built files:
To clean the built .o files and executables, run `make clean` in this
directory.

## Usage for each example
File name     |  command to use it | things to know
------------- | ------------- | -------------
`posix-communications.c` | `./posix-communications <device>` | reports how the instrument is connected and powered, then puts it to sleep; writes nothing
`posix-download.c` | `./posix-download <device>` | downloads one schedule's data from the dataset recorded by posix-enable; writes nothing
`posix-enable-multiconfig.c` | `./posix-enable-multiconfig <device>` | **clears the instrument configuration**, defines ascent and park configurations, and enables the ascent one
`posix-enable.c` | `./posix-enable <device>` | **clears the instrument configuration** and enables a deployment with a single schedule and configuration
`posix-footprint.c` | `./posix-footprint` | prints the memory footprint of the library structures; no instrument needed
`posix-generation.c` | `./posix-generation <device>` | reports the instrument generation
`posix-poll.c` | `./posix-poll <device>` | polls on-demand samples; writes nothing
`posix-stream.c` | `./posix-stream <device>` | **clears the instrument configuration**, enables a streaming deployment, prints samples until Ctrl-C, then disables

## Contributing

The library is primarily maintained by RBR, and development is directed by our needs and the needs of our [OEM] customers.
However, we're happy to take [contributions] generally.

[OEM]: https://rbr-global.com/products/oem
[contributions]: ../../CONTRIBUTING.rst

## License

This project is licensed under the terms of the Apache License, Version 2.0;
see https://www.apache.org/licenses/LICENSE-2.0.

* The license is not “viral”.
  You can include it either as source or by linking against it, statically or dynamically, without affecting the licensing
  of your own code.
* You do not need to include RBR's copyright notice in your documentation, nor do you need to display it at program runtime.
  You must retain RBR's copyright notice in library source files.
* You are under no legal obligation to share your own modifications (although we would appreciate it if you did so).
* If you make changes to the source, in addition to retaining RBR's copyright notice,
  you must add a notice stating that you changed it.
  You may add your own copyright notices.
