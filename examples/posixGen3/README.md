# libRBR POSIX examples

## Requirements

* Hardware:
  RBR L3 board stack,
  RS-232/RS-485/USB connection
  (~12V power supply required for RS-232/RS-485)
* Firmware: RBR firmware (1.135+ required for some dynamic correction examples)

> ℹ️ The file parsing examples do not require a connected device.

* Environment: GNU Make and a C99-compliant C compiler; this can be [Cygwin]

[Cygwin]: ../../docs/windows.rst

## Building

These examples link statically
against the libRBR library archives
(`libRBR.a` and `libRBRDynamicCorrection.a`)
built from the sources in the grandparent directory.
Before building any examples, ensure these have been built:

~~~{.sh}
$ pwd
$ /path/to/librbr/examples/posixGen3
$ make --directory=../../ lib libdynamiccorrection
make: Leaving directory '/path/to/librbr'
~~~

To build examples,
invoke one of three Make targets:

* To build all examples,
  invoke the default target, aka “all”:

  ~~~{.sh}
  $ make
  $ make all
  ~~~

* To build examples that do not depend on SDL,
  invoke the “nosdl” target:

  ~~~{.sh}
  $ make nosdl
  ~~~

## Tips

### Check the baud rate

By default, these examples attempt communication at 9600 baud.
If your instrument is configured to use 9600 baud, no change is necessary.
Otherwise, you'll need to modify
the `cfsetospeed(3)` call in `./posix-shared.c`.
For example, to use 115,200 baud:

~~~{.diff}
diff --git a/examples/posixGen3/posix-shared.c b/examples/posixGen3/posix-shared.c
index c50c366..8cb1b71 100644
--- a/examples/posixGen3/posix-shared.c
+++ b/examples/posixGen3/posix-shared.c
@@ -64,7 +64,7 @@ int openSerialFd(char *devicePath)
     /*important!!!
      change baudrate below if one is using 115200:
      */
-    cfsetospeed(&portSettings, B9600);
+    cfsetospeed(&portSettings, B115200);

     /* Input baud rate of 0 causes the output baud rate to be used. */
     cfsetispeed(&portSettings, B0);
~~~

### Confirm which port is in use

Windows COM ports are 1-based, while Cygwin ports are 0-based.
E.g., Cygwin exposes COM6 as /dev/ttyS5.

* To list all Windows COM ports, use the [mode] command.
* To list all Cygwin serial ports, run `ls /dev/ttyS*`.

[mode]: https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/mode

### How to clean the built files

The library and example build artifacts (`.a`, `.o`, and `.exe`)
can be removed by invoking the “clean” Make target
in the corresponding source directory:

~~~
$ make --directory=../../ clean
$ make clean
~~~

### Dynamic correction channel dependency

For `posix-stream-dynamiccorrection.c` example,
make sure these channels are ON:

* conductivity_00
* temperature_00
* pressure_00/seapressure_00
* conductivitycelltemperature_00

### Downloading from postprocessed data

For the `posix-parse-download-dataset.c` example,
when downloading from dataset 4,
ensure the number of channels expected by the example
matches the number of channels configured in the instrument.
For example, if we have configured five postprocessing channels
in the instrument:

~~~
>> postprocessing channels = mean(temperature_00_dyn_corr)|mean(pressure_00)|mean(salinity_00_dyn_corr)|mean(salinity_00)|mean(conductivitycelltemperature_00)
~~~

The example source must also expect five channels:

~~~{.c}
// posix-parse-download-dataset.c:
        // Important! This must match the postprocessing channels configured
        // in the instrument.
        enabledChannels = 5;
~~~

## Usage for each example

| File name | Invocation | Notes
| --------- | ---------- | -----
| `posix-parse-file-dynamiccorrection.c` | `./posix-parse-file-dynamiccorrection ../sampledata/dynamiccorrection-sample.bin 4` | The sample `.bin` file columns have to be: Cmeas(mS/cm), Tmeas(°C), Pmeas(sea pressure, dbar), Tcond(°C). |
| `posix-stream-dynamiccorrection.c` | `./posix-stream-dynamiccorrection /dev/ttyS5` | See above: “Check the baud rate”, “Dynamic correction channel dependency”. |
| `posix-parse-download-dataset.c` | `./posix-parse-download-dataset /dev/ttyS5 1` | See above: “Check the baud rate”, “Downloading from postprocessed data”. |

(to be continued...)

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
