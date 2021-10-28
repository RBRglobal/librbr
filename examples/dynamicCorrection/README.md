# How to use dynamicCorrection example

## Setup
Runtime environment: cygwin

## Build this example
Assuming librbr is already built.(if not, go to librbr directory, and then use cygwin command "make")
Go to librbr/examples/dynanicCorrection directory, then use sygwin command "make". Ignore the error you see.

## Tips before you start:
(1) How to clean the built files:
To clean the .a, .o, .exe files one built, use cygwin command "make clean" in that folder directory.

## Usage for this example:
File name   |      command to use it | things to know
------------- | ------------- | -------------
posix-parse-file-dynamiccorrection.c    | ./dynamicCorrection-example ../sampledata/dynamiccorrection-sample.csv | the sample .bin file columns have to be: Cmeas(mS/cm), Tmeas(°C), Pmeas(sea pressure, dbar), Tcond(°C)

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
