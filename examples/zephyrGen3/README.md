# Zephyr example

This [Zephyr freestanding application][freestanding]
provides an example of how to consume libRBR
in a project using Zephyr,
an embedded RTOS.
Out of the box,
this example targets [the native simulator][native_sim]
and a selection of STM32 development boards,
but it can easily be extended
to run on any of [the multitude of boards supported by Zephyr][boards]
or on custom hardware.

Functionally, this example reproduces the `posix-stream` example
in a Zephyr environment:
connects to the instrument,
configures streaming,
and starts logging.
Samples produced by the instrument
are relayed to the Zephyr console.
Communication with the instrument leverages DMA
by using Zephyr's async UART API.

[freestanding]: https://docs.zephyrproject.org/4.2.0/develop/application/index.html#zephyr-freestanding-application
[native_sim]: https://docs.zephyrproject.org/4.2.0/boards/native/native_sim/doc/index.html
[boards]: https://docs.zephyrproject.org/4.2.0/boards/index.html

## Organization

This example is laid out like the [T2 topology].
This directory (`examples/zephyrGen3/`) is the Zephyr workspace;
the `application/` subdirectory holds the application.

[T2 topology]: https://docs.zephyrproject.org/4.2.0/develop/west/workspaces.html#t2-star-topology-application-is-the-manifest-repository

## Building

First, install [West], the Zephyr build tool:

~~~{.txt}
$ pipx install west
  installed package west 1.5.0, installed using Python 3.13.7
  These apps are now globally available
    - west
done! ✨ 🌟 ✨
~~~

Then initialize a West workspace
in this directory:

~~~{.txt}
$ west init --local application/
=== Initializing from existing manifest repository application
--- Creating /path/to/librbr/examples/zephyrGen3/.west and local configuration file
=== Initialized. Now run "west update" inside /path/to/librbr/examples/zephyrGen3.
$ west update
--- zephyr: initializing
...
~~~

Now, from within its subdirectory,
you can build the example application:

~~~{.txt}
$ cd application/
$ west build --board=native_sim -- -DEXTRA_CONF_FILE=debug.conf
-- west build: generating a build system
Loading Zephyr default modules (Zephyr base).
-- Application: /path/to/librbr/examples/zephyrGen3/application
...
-- west build: building application
...
[116/118] Linking C executable zephyr/zephyr.elf; Logical command for additional byproducts on target: zephyr_pre0
Generating files from /path/to/librbr/examples/zephyrGen3/application/build/zephyr/zephyr.elf for board: native_sim
[118/118] Running utility command for native_runner_executable
~~~

[West]: https://docs.zephyrproject.org/4.2.0/develop/west/index.html

## Running

`west flash` will attempt to flash and run the built image.
When building for the native simulator,
it will invoke the executable locally.

~~~{.txt}
$ west flash
-- west flash: rebuilding
[2/2] Running utility command for native_runner_executable
-- west flash: using runner native
uart_1 connected to pseudotty: /dev/pts/1
*** Booting Zephyr OS build v4.2.0 ***
[00:00:00.000,000] <inf> main: using libRBR v1.2.4 (built 2025-10-07T01:27:19Z)
...
[00:00:10.190,000] <err> io: read: timeout
[00:00:10.190,000] <err> main: opening instrument: unsupported
~~~

You can use [socat]
to proxy traffic between the sample and a real instrument.
As the pseudoterminal device is not available until the simulator is running,
and the device path may change with each invocation,
the native executable can [run a program] once the pty is available.
Unfortunately, there is no way to pass this flag through `west flash`,
so we will need to invoke the native executable directly.
For example, if your instrument is connected to `/dev/ttyACM0`,
you could run:

~~~{.txt}
$ ./build/zephyr/zephyr.exe -uart_1_attach_uart_cmd="socat /dev/ttyACM0 %s &"
uart_1 connected to pseudotty: /dev/pts/3
*** Booting Zephyr OS build v4.2.0 ***
[00:00:00.000,000] <inf> main: using libRBR v1.2.4 (built 2025-10-07T01:27:19Z)
[00:00:00.240,000] <inf> main: connected via usb
[00:00:00.320,000] <inf> main: instrument is stopped, not logging; I'm going to start it
2025-10-07 01:30:24.250, nan
2025-10-07 01:30:24.375, nan
2025-10-07 01:30:24.500, nan
^C
Stopped at 3.040s
~~~

(The executable is called `zephyr.exe` even on POSIX systems.)

Ctrl+C (SIGINT) will terminate the process.

[socat]: http://www.dest-unreach.org/socat/
[run a program]: https://docs.zephyrproject.org/4.2.0/boards/native/native_sim/doc/index.html#pty-uart
