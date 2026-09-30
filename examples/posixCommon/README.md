<!-- Copyright (c) 2026 RBR Ltd. -->
<!-- SPDX-License-Identifier: Apache-2.0 -->

# Examples using both instrument generations

These examples use the Gen3 (`RBRGen3…`) and Gen4 (`RBRGen4…`) APIs in one
program. Build libRBR with both generations enabled (the default `make lib`),
then run `make` in this directory.

## posix-shared-buffers

Opens a Gen3 and a Gen4 instrument on two serial devices and alternates
commands between them through one shared command buffer and one shared
response buffer. It shows the two rules for sharing:

* Size the buffers for the larger of the two generations' needs.
* Before a connection resumes after the other has used the buffers, call
  `RBRGen3_resetResponseBuffer()` or `RBRGen4_resetResponseBuffer()` so it
  does not parse the other instrument's leftovers as its own.

Neither instrument streams here, so switching loses nothing. A streaming
connection loses any samples not yet delivered each time the buffer changes
hands, so it should have a response buffer of its own.

~~~{.sh}
./posix-shared-buffers /dev/ttyACM1 /dev/ttyACM0
~~~

The first argument is the Gen3 instrument's device, the second the Gen4's.
The baud rate is irrelevant over USB; over a serial line, adjust
`openSerialFd()` to match each instrument.
