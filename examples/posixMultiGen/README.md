<!-- Copyright (c) 2026 RBR Ltd. -->
<!-- SPDX-License-Identifier: Apache-2.0 -->

# How to use the multi-generation posix examples

These examples are for an application built with both the Gen3 and Gen4
APIs, so they require the library to have been built with both generations
enabled (the default configuration).

## Build
Assuming libRBR is already built (if not, go to the librbr directory and run
`make lib`), go to the librbr/examples/posixMultiGen directory and run
`make`.

The serial port is opened at 115200 baud, the instrument default; the rate is
irrelevant over a USB connection. On Linux the instrument usually appears as
`/dev/ttyUSB0` or `/dev/ttyACM0`.

## Usage for each example:
File name     |  command to use it | things to know
------------- | ------------- | -------------
`posix-detect.c` | `./posix-detect <device>` | calls `RBRGen3_open()`, falling back to `RBRGen4_open()` if `RBRGen3_getGeneration()` reports Logger4 on `RBRGEN3_UNSUPPORTED`, then reports its identification; changes no instrument settings
