## \file Makefile
##
## \brief Build automation.
##
## This makefile provides three different targets of interest to the end user:
##
## - `lib` will build the library (`bin/libRBR.a`)
## - `docs` will generate the documentation via Doxygen (in `docs/`)
## - `tests` will run library tests (from `tests/`)
##
## Additional targets may be useful to developers:
##
## - `clean` will remove any compiled binaries and documentation
## - `devdocs` will generate the documentation inclusive of content only of
##   interest to library developers
##
## \copyright
## Copyright (c) 2018 RBR Ltd.
## Licensed under the Apache License, Version 2.0.

## \brief The project name.
##
## Embedded into the library and documentation.
##
## Project forks might like to override this by setting the `LIB_NAME`
## environment variable before invoking `make(1)` to easily identify which
## library variant is in use.
export LIB_NAME ?= libRBR

## \brief The project version.
##
## Embedded into the library and documentation. Determined from the contents of
## the VERSION file.
export LIB_VERSION ?= $(shell head -n 1 VERSION)

## \brief The project build date.
##
## Embedded into the library. Probably shouldn't be overridden, unless you want
## to match the build date or timestamp format of a containing project.
export LIB_BUILD_DATE ?= $(shell date '+%FT%T%z')

## \brief Archiver flags.
##
## “Archive”, in this case, refers to the `.a` archive produced by building the
## library.
##
## - `c`: create the archive if necessary
## - `r`: replace existing contents of archive
## - `s`: create/update archive index
##
## In pursuit of reproducible builds, recent versions of Debian (and therefore
## derivatives, including Ubuntu) ship a version of ar(1) which produces
## “deterministic” archives; i.e., the UID/GID/timestamp/mode file attributes
## are 0'd out. Unfortunately, because make(1) decides whether or not a target
## needs to be built by comparing target and dependency modification times,
## this completely breaks incremental builds: with deterministic archives left
## enabled, every invocation of `make lib` causes the entire library to be
## rebuilt. As a workaround, we'll explicitly disable deterministic mode on
## systems using GNU ar(1):
##
## - `U`: maintain original UID/GID/timestamp/mode of archive contents
##
## The build is nondeterministic anyway because of the embedded build date, so
## we gain nothing from leaving deterministic mode on, and we save a lot of
## developer time on the edit/compile/test loop by turning it off.
ARFLAGS := -c -r -s
ARFLAGS += $(shell \
    ar --version 2>&1 | grep -q GNU \
    && ar --help 2>&1 | grep -q '\[U\]' \
    && echo -U)

## \brief C compilation flags.
##
## Conventionally, extensive warnings and warnings-as-errors would only be
## enabled for debug/development builds to broaden compiler compatibility.
## (Some compilers might generate warnings for different things than others,
## and with `-Werror`, that might prevent compilation.) However, to help ensure
## high code quality everywhere, we're leaving them all on by default. Please
## let us know if you encounter issues: we want to fix them.
##
## We're also turning on debug symbol generation by default so you don't have
## to go out of your way to enable them. If you don't want them, we suggest
## using `strip(1)` on the archives as opposed to changing this makefile to
## minimize the friction of pulling in library updates.
CFLAGS := -Werror \
          -Wall \
          -Wextra \
          -pedantic \
          -pedantic-errors \
          -Wwrite-strings \
          -std=c99 \
          -g

CFLAGS += -DRBR_LIB_NAME=\""$(LIB_NAME)"\" \
          -DRBR_LIB_VERSION=\""$(LIB_VERSION)"\" \
          -DRBR_LIB_BUILD_DATE=\"$(LIB_BUILD_DATE)\" \
          -Iinclude

all: lib docs tests

lib: bin/libRBR.a

bin/libRBR.a: bin bin/libRBR.a(src/RBRInstrument.o \
                               src/RBRInstrumentCommunication.o \
                               src/RBRInstrumentConfiguration.o \
                               src/RBRInstrumentDeployment.o \
                               src/RBRInstrumentFetching.o \
                               src/RBRInstrumentGating.o \
                               src/RBRInstrumentHardwareErrors.o \
                               src/RBRInstrumentInternal.o \
                               src/RBRInstrumentMemory.o \
                               src/RBRInstrumentOther.o \
                               src/RBRInstrumentSchedule.o \
                               src/RBRInstrumentSecurity.o \
                               src/RBRInstrumentStreaming.o \
                               src/RBRInstrumentVehicle.o \
                               src/RBRParser.o)

.PHONY: docs
docs:
	doxygen tools/Doxyfile

.PHONY: devdocs
devdocs:
	doxygen tools/Doxyfile-devdocs

tests: CFLAGS += -Wno-error=unused-parameter -Wno-unused-parameter
tests: LDFLAGS += -Lbin
tests: LDLIBS += -lRBR
.PHONY: tests
tests: bin bin/tests
	./bin/tests

## \brief Test modules.
##
## Each one of these names corresponds to a C source file in the `tests/`
## directory. Tests declared within these files (using the `TEST_LOGGER2` and
## `TEST_LOGGER3` macros) are automatically discovered at build time and
## included in the test suite.
TEST_MODULES := communication \
                configuration \
                deployment \
                fetching \
                gating \
                memory \
                other \
                schedule \
                security \
                streaming \
                vehicle \
                parser

bin/tests: bin/libRBR.a \
           tests/main.o \
           tests/tests.o \
           $(foreach module,$(TEST_MODULES),tests/$(module).o)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

tests/tests.c: $(foreach module,$(TEST_MODULES),tests/$(module).c)
	@echo "/* This file is automatically generated. */" >$@
	@echo '#include "tests.h"' >>$@
	@grep -ho 'TEST_\(LOGGER[23]\|PARSER\)([A-Za-z_][A-Za-z0-9_]*\(, .*\)\?)' \
			$^ \
		| sed -e 's/$$/;/' >>$@
	@echo "InstrumentTest instrumentTests[] = {" >>$@
	@grep -ho 'TEST_LOGGER[23]([A-Za-z_][A-Za-z0-9_]*)' $^ \
		| sed -e 's/^TEST_LOGGER\([^(]*\)(\([^)]*\))/    {"\2", RBRINSTRUMENT_LOGGER\1, test_\2_l\1},/' \
		>>$@
	@echo "    {0}" >>$@
	@echo "};" >>$@

	@grep -ho 'TEST_PARSER_CONFIG([A-Za-z_][A-Za-z0-9_]*)' $^ \
		| sed -e 's/^TEST_PARSER_CONFIG(\([^,]*\))/extern const RBRParserConfig test_\1_parser_config;/' \
		>>$@

	@echo "ParserTest parserTests[] = {" >>$@
	@grep -ho 'TEST_PARSER([A-Za-z_][A-Za-z0-9_]*, .*)' $^ \
		| sed -e 's/^TEST_PARSER(\([^,]*\), \(.*\))/    {"\1", \&test_\2_parser_config, test_\1_parser},/' \
		>>$@
	@echo "    {0}" >>$@
	@echo "};" >>$@

bin:
	mkdir bin

.PHONY: clean
clean:
	rm -Rf src/*.o bin/ tests/tests.c tests/*.o docs/
