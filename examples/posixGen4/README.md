# How to use posix examples

## Setup
* Hardware: RBR L3 board stack, RS232/RS485/USB connection (~12V power supply if it's RS232/RS485)
* Firmware: RBR firmware. (requires 1.135 or up for some dynamic correction examples)
~~~{.sh}
Info: not all examples requires hardware/firmware.
~~~
* Runtime environment: cygwin

## Build all posix example
Assuming librbr is already built.(if not, go to librbr directory, and then use cygwin command "make")
Go to librbr/examples/posix directory, then use sygwin command "make". Ignore the error you see.

## Tips before you start:
(1) Check the baudrate:
If it's 9600, it's all good. If not, you'll need to modify librbr/examples/posix/posix-shared.c:
~~~{.c}
#ifndef B115200
#define B115200 115200
#define B9600 9600
#endif

    /*important!!!
     change baudrate below if one is using 115200:
     */
    cfsetospeed(&portSettings, B9600);
~~~
(2) Confirm which port is in use:
If terminal tool suggest COM6, it's most likely /dev/ttyS5 in cygwin.
Alternatively, one can use cygwin command "ls /dev/ttyS*", then try it out. (ttyS5 is used as example below.)

(3) How to clean the built files:
To clean the .a, .o, .exe files one built, use cygwin command "$ make clean" in that folder directory.

(4) For posix-stream-dynamiccorrection.c example, make sure these channels are ON:
~~~{.sh}
conductivity_00, temperature_00, pressure_00/seapressure_00, conductivitycelltemperature_00
~~~
(5) For posix-parse-download-dataset.c example, if downloading from dataset4, make sure the channels is set the same as number of channels in output.
for example, if we set in firmware:
~~~{.sh}
>> postprocessing channels = mean(temperature_00_dyn_corr)|mean(pressure_00)|mean(salinity_00_dyn_corr)|mean(salinity_00)|mean(conductivitycelltemperature_00)
~~~
Then we set:
~~~{.c}
RBRInstrumentGen4Channels channels;
    //important!!!
    //channels.count should be set the same number with output channels.
    channels.count = 5;
    channels.on = 5;
~~~


## Usage for each example:
File name     |  command to use it | things to know                     
------------- | ------------- | -------------
posix-parse-file-dynamiccorrection.c    | ./posix-parse-file-dynamiccorrection ../sampledata/dynamiccorrection-sample.bin 4 | the sample .bin file columns have to be: Cmeas(mS/cm), Tmeas(°C), Pmeas(sea pressure, dbar), Tcond(°C)
posix-stream-dynamiccorrection.c |./posix-stream-dynamiccorrection /dev/ttyS5 | note (2) above
posix-parse-download-dataset.c |./posix-parse-download-dataset /dev/ttyS5 1 | note (2) (5) above

(to be continued...)


## Contributing

The library is primarily maintained by RBR, and development is directed by our needs and the needs of our [OEM] customers.
However, we're happy to take [contributions] generally.

[OEM]: https://rbr-global.com/products/oem
[contributions]: CONTRIBUTING.md

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
