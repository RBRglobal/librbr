# Changes

## v1.2.1

Release 2021-10-21

### Changed
* method of dynamic correction updated.
* default coefficients updated.
* added README.md for the posix examples and the example in dynamicCorrection folder.

## v1.2.0

Release 2021-06-04

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
