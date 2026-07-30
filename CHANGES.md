# Changes

## v1.3.0

Release TBD

### Added

* Added Zephyr module definition
  permitting inclusion in a Zephyr workspace via [West].
* Added `RBRInstrument_getChannelsWithoutCalibrations()` method
  to retrieve channel information without calibration data
  to save bandwidth and time
  when calibration information is unnecessary.

[West]: https://docs.zephyrproject.org/latest/develop/west/index.html

### Changed

* added support for no dynamic memory allocation.
* fixed bug where wake up sequence not occuring after RBRInstrument_fetch() usage.
* Poll each channel individually, rather than requesting `channel all`.
  This reduces the minimum response buffer size
  when communicating with instruments with many channels
  (at the cost of slightly increased interrogation time).
* Make “Ready: ” prompt matching case-insensitive.
  This is a precursor to Gen4 support.
* Parse RBR_coda_ timestamps
  (continuously-incrementing millisecond counter).
* Revised default coefficient count limits
  to better match the numbers used by calibration equations
  (now 8/16/8 _c_/_x_/_n_ coefficients;
  previously 24/8/8).
* Make the sizes of array fields on channels overrideable
  without source modification.
* Updated CI targets:
  reintroduced GCC 6;
  added GCC 12–15;
  updated to Clang 20;
  updated documentation step to GCC 15.
* Fixed parallel builds with Make `-j, --jobs`,
  and made `.a`rchives compatible with deterministic archivers
  (llvm-ar v10.0.0+, and most distro builds of GNU binutils ar v2.23+).
* Improve error handling in `RBRInstrument_setCalibration()`
  (validate that the date/time is in range,
  and informing the user if no coefficients were set).
* The library version number
  (stored in the string constant `RBRINSTRUMENT_LIB_VERSION`)
  is now based on the Git version information
  (`git describe --dirty`)
  where possible.
  The `VERSION` file is still used as a fallback
  when Git or the repository metadata is unavailable.
* The library build date
  (stored in the string constant `RBRINSTRUMENT_LIB_BUILD_DATE`)
  has been deprecated
  to make library builds deterministic
  and object contents compatible with reproducible build processes.
  Per SemVer API compatibility guarantees,
  the symbol will not be removed from the API surface
  before the next major version increment;
  however, when the library is built with the default, in-tree build process,
  its value will always be “unknown”.

### Fixed

* The sample parsing helper function used internal by `RBRParser_parse()`
  previously cleared an incorrect buffer size
  before writing the parsed sample.
  In the typical case
  where `sizeof(RBRInstrumentSample) > sizeof(RBRInstrumentEvent)`,
  this could have left stale data at the end of `sample->readings`;
  in suitable a non-default configuration
  (e.g., where `RBRINSTRUMENT_CHANNEL_MAX < 3`),
  this could have resulted in writes
  past the end of the `data` buffer.

## v1.2.4

Released 2025-04-07

### Added

* fixed timestamp datatype in dynamiccorrection algorithm (RBRDynamicCorrection.c and .h files) so that they can batter span long periods without trancation /rounding errors.
* fixed the timestamp comparision logic in dynamiccorrection algorithm (RBRDynamicCorrection.c and .h files) so that pressure velocity calculation starts with the second sample.

### Changed

* changed _isFasterSampling in dynamiccorrection algorithm to cached value to improve performance.
* changed pressure velocity calculation in dynamiccorrection algorithm for one path with no unused calculation to improve performance.
* changed calculation of RBRDynamicCorrectionParams in dynamiccorrection algorithm to eliminate duplicate conversions to improve performance.

### Removed

* Removed support for gcc6 compilation in pipeline due to Jessie(Debian 8) EOL (jessie/main amd64 Packages not found in http://deb.debian.org, which blocks installation of libsdl2-dev).

## v1.2.3

Released 2022-11-25

### Added

* support for variable ascent rate in dynamic correction algorithm.
* support for outputformat caltext07 for LOGGER3 with fw 1.109 or later.
* support pause/resume feature for LOGGER3 with fw 1.116 or later.
* added example bash file to auto connect to RBRinstrument wifi.
* added error code RBRINSTRUMENT_COMMUNICATION_ERROR.

### Changed

* changed command terminator to \r instead of \r\n.
* checks if offset in response matches request.
* updated description of function RBRInstrumentSleepCallBack in RBRInstrument.h.

## v1.2.2

Released 2021-12-01

### Changed

* default coefficients for dynamic correction updated.

## v1.2.1

Released 2021-10-21

### Changed

* method of dynamic correction updated.
* default coefficients updated.
* added README.md for the posix examples and the example in dynamicCorrection folder.

## v1.2.0

Released 2021-06-04

### Added

* new feature: standalone library for dynamic correction provided:

 > Dynamic correction corrects all the dynamic errors affecting the salinity estimates for a profiling CTD, such as response time and sensor misalignments, or thermal mass errors.

 > A standalone dynamic correction library is provided, together with various examples on how to apply the dynamic correction to a .csv data file, or to loggers during/after logging.

 > Refer to `README.md` for more details.

* Download over Wi-Fi example.

## v1.1.2

Released 2020-04-23

### Added

* Bitbucket Pipelines: build with GCC 9 and Clang 9.

### Changed

* Moved developer tools into `tools/`.
  An attempt to keep only universally interesting things
  in the top level of the project directory.
* Added explicit array index subscript to unsigned char to remove char-subscripts warning.
* PRIi64 use in sscanf is avoided.

## v1.1.0

Released 2019-05-22.

### Added

* Added support for the `id mode` parameter.
* Added support for the `postprocessing` command.

### Changed

* Building: Better detection of `-U` flag support for `ar(1)`.
* Added missing parameter range validation
  for some memory commands.

## v1.0.5

Released 2019-03-12.

### Changed

* More compatible wake behaviour.
  Wake-from-sleep should now be more broadly compatible
  with alternative transport layers,
  particularly those with conservative/infrequent packetization.
* Escaping of special characters
  in string comparisons
  shown upon test failures.

### Fixed

* Fixed uninitialized variable in POSIX examples.
* Reset instrument activity timer when rebooting the instrument
  (in `RBRInstrument_reboot()`).
  This will cause the library
  to attempt to wake the instrument
  before performing any subsequent operations.

## v1.0.4

Released 2019-02-15.

### Fixed

* Fixed compatibility error with Logger2 instruments
  in `RBRInstrument_setClock()`.

## v1.0.3

Released 2019-01-29.

### Changed

* Moved public headers out of the `src/` directory
  into `include/`.
  This helps enforce the distinction
  between public and internal APIs
  and makes it slightly easier
  for consumer projects
  to include the library headers.

## v1.0.2

Released 2019-01-03.

### Changed

* Rebrand slightly from librbr to libRBR.
  This is reflected
  by the name of the library archive,
  which has changed
  from `librbr.a`
  to `libRBR.a`
  (and subsequently,
  must now be linked with `-lRBR`
  instead of `-lrbr`).

## v1.0.1

Released 2018-12-06.

### Added

* To the readme:
    * Added a short example of what user code might look like.
    * Added firmware support list.

### Fixed

* Added release date for v1.0.0 to this changelog.

## v1.0.0

Released 2018-12-05.

The initial library release version.
