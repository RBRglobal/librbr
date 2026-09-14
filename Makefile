## \file Makefile
##
## \brief Build automation.
##
## This makefile provides three different targets of interest to the end user:
##
## - `lib` will build the library (`bin/libRBR.a`)
## - `docs` will generate the documentation via Doxygen (in `docs/`)
## - `tests` will run library tests (from `testsGen3/` and `testsGen4/`)
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
## Embedded into the library and documentation. Based on the output of the `git
## describe --dirty` command where possible, or else the contents of the
## VERSION file.
export LIB_VERSION ?= $(shell ./tools/version.sh)

## \brief The project build date.
##
## \deprecated As of libRBR v1.3.0, this builds with the value “unknown” to
##             support deterministic builds.
#export LIB_BUILD_DATE

## \brief Instrument generation selection.
##
## The library contains two independent instrument APIs:
##
## - `GEN3`: the `RBRInstrumentGen3_`-prefixed API for Logger2/Logger3
##   instruments — the libRBR 1.x API, suffixed `Gen3`;
## - `GEN4`: the `RBRInstrumentGen4_`-prefixed API for Generation 4
##   (SL4/SEN4/L4) instruments.
##
## Both are enabled (`1`) by default and both land in the same
## `bin/libRBR.a`; disable one by passing `GEN3=0` or `GEN4=0` to `make(1)`.
## At least one generation must be enabled.
GEN3 ?= 1
GEN4 ?= 1

ifeq ($(GEN3),0)
ifeq ($(GEN4),0)
$(error At least one instrument generation must be enabled: set GEN3=1 and/or GEN4=1)
endif
endif

## \brief Archiver flags.
##
## “Archive”, in this case, refers to the `.a` archive produced by building the
## library.
##
## - `c`: create the archive if necessary
## - `r`: replace existing contents of archive
## - `s`: create/update archive index
ARFLAGS := -c -r -s

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
          -Wdouble-promotion \
          -Wwrite-strings \
          -std=c99 \
          -g

CFLAGS += -DRBR_LIB_NAME=\""$(LIB_NAME)"\" \
          -DRBR_LIB_VERSION=\""$(LIB_VERSION)"\" \
          -Iinclude

ifneq ($(LIB_BUILD_DATE),)
CFLAGS += -DRBR_LIB_BUILD_DATE=\"$(LIB_BUILD_DATE)\"
endif

all: lib libdynamiccorrection docs tests

libdynamiccorrection: bin/libRBRDynamicCorrection.a

lib: bin/libRBR.a

## \brief Objects for the Gen3 (Logger2/Logger3) API.
GEN3_OBJECTS := src/RBRInstrumentGen3.o \
                src/RBRInstrumentGen3Communication.o \
                src/RBRInstrumentGen3Configuration.o \
                src/RBRInstrumentGen3Deployment.o \
                src/RBRInstrumentGen3Fetching.o \
                src/RBRInstrumentGen3Gating.o \
                src/RBRInstrumentGen3HardwareErrors.o \
                src/RBRInstrumentGen3Internal.o \
                src/RBRInstrumentGen3Memory.o \
                src/RBRInstrumentGen3Other.o \
                src/RBRInstrumentGen3Pauseresume.o \
                src/RBRInstrumentGen3Schedule.o \
                src/RBRInstrumentGen3Security.o \
                src/RBRInstrumentGen3Streaming.o \
                src/RBRInstrumentGen3Vehicle.o \
                src/RBRParserGen3.o

## \brief Objects for the Gen4 (SL4/SEN4/L4) API.
GEN4_OBJECTS := src/RBRGen4.o \
                src/RBRGen4Communication.o \
                src/RBRGen4Configuration.o \
                src/RBRGen4Deployment.o \
                src/RBRGen4HardwareErrors.o \
                src/RBRGen4Instrument.o \
                src/RBRGen4Internal.o \
                src/RBRInstrumentGen4Memory.o \
                src/RBRInstrumentGen4Realtime.o \
                src/RBRParserGen4.o

LIB_OBJECTS :=
DYNAMICCORRECTION_OBJECTS :=
ifeq ($(GEN3),1)
LIB_OBJECTS += $(GEN3_OBJECTS)
DYNAMICCORRECTION_OBJECTS += src/RBRDynamicCorrectionGen3.o
endif
ifeq ($(GEN4),1)
LIB_OBJECTS += $(GEN4_OBJECTS)
DYNAMICCORRECTION_OBJECTS += src/RBRDynamicCorrectionGen4.o
endif

# Due to incompatibility between parallel builds (-j, --jobs) and Make's
# archive syntax, and between incremental builds and deterministically-produced
# archives (as written out by llvm-ar v10.0.0+ and most distro builds of GNU
# binutils ar v2.23+), we'll not use “member” syntax at all, and define our
# archive rules with normal prerequisites.
#
# We can, however, still share the recipe between both of our archive rules.
# And we'll filter for objects, and not any other files, so we don't try to
# stuff bin/ – which is also a dependency of our archives – into the archives:
%.a:
	$(AR) $(ARFLAGS) $@ $(filter %.o,$?)

bin/libRBR.a: $(LIB_OBJECTS) | bin

bin/libRBRDynamicCorrection.a: $(DYNAMICCORRECTION_OBJECTS) | bin

.PHONY: docs
docs:
	doxygen tools/Doxyfile

.PHONY: devdocs
devdocs:
	doxygen tools/Doxyfile-devdocs

TEST_BINARIES :=
ifeq ($(GEN3),1)
TEST_BINARIES += bin/testsGen3
endif
ifeq ($(GEN4),1)
TEST_BINARIES += bin/testsGen4
endif

tests: CFLAGS += -Wno-error=unused-parameter -Wno-unused-parameter
tests: LDFLAGS += -Lbin
tests: LDLIBS += -lRBR -lRBRDynamicCorrection -lm
.PHONY: tests
tests: bin $(TEST_BINARIES)
	$(foreach test,$(TEST_BINARIES),./$(test) &&) true

nomalloc: CFLAGS += -DRBR_LIB_NODYNAMICMEMORYALLOCATION
nomalloc: lib libdynamiccorrection docs tests

## \brief Gen3 test modules.
##
## Each one of these names corresponds to a C source file in the `testsGen3/`
## directory. Tests declared within these files (using the `TEST_LOGGER2` and
## `TEST_LOGGER3` macros) are automatically discovered at build time and
## included in the test suite.
GEN3_TEST_MODULES := communication \
                     configuration \
                     deployment \
                     dynamiccorrection \
                     fetching \
                     gating \
                     memory \
                     other \
                     schedule \
                     security \
                     streaming \
                     vehicle \
                     parser \
                     pauseresume

## \brief Gen4 test modules.
##
## As for the Gen3 suite, each one of these names corresponds to a C source
## file in the `testsGen4/` directory (using the `TEST_LOGGER4` macro).
GEN4_TEST_MODULES := communication \
                     configuration \
                     deployment \
                     instrument \
                     memory \
                     realtime

bin/testsGen3: bin/libRBR.a \
           bin/libRBRDynamicCorrection.a \
           testsGen3/main.o \
           testsGen3/tests.o \
           $(foreach module,$(GEN3_TEST_MODULES),testsGen3/$(module).o)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

testsGen3/tests.c: $(foreach module,$(GEN3_TEST_MODULES),testsGen3/$(module).c)
	@echo "/* This file is automatically generated. */" >$@
	@echo '#include "tests.h"' >>$@
	@grep -ho 'TEST_\(LOGGER[23]\|PARSER\)([A-Za-z_][A-Za-z0-9_]*\(, .*\)\?)' \
			$^ \
		| sed -e 's/$$/;/' >>$@
	@echo "InstrumentTest instrumentTests[] = {" >>$@
	@grep -ho 'TEST_LOGGER[23]([A-Za-z_][A-Za-z0-9_]*)' $^ \
		| sed -e 's/^TEST_LOGGER\([^(]*\)(\([^)]*\))/    {"\2", RBRINSTRUMENTGEN3_LOGGER\1, test_\2_l\1},/' \
		>>$@
	@echo "    {0}" >>$@
	@echo "};" >>$@

	@grep -ho 'TEST_PARSER_CONFIG([A-Za-z_][A-Za-z0-9_]*)' $^ \
		| sed -e 's/^TEST_PARSER_CONFIG(\([^,]*\))/extern const RBRParserGen3Config test_\1_parser_config;/' \
		>>$@

	@echo "ParserTest parserTests[] = {" >>$@
	@grep -ho 'TEST_PARSER([A-Za-z_][A-Za-z0-9_]*, .*)' $^ \
		| sed -e 's/^TEST_PARSER(\([^,]*\), \(.*\))/    {"\1", \&test_\2_parser_config, test_\1_parser},/' \
		>>$@
	@echo "    {0}" >>$@
	@echo "};" >>$@

bin/testsGen4: bin/libRBR.a \
               bin/libRBRDynamicCorrection.a \
               testsGen4/main.o \
               testsGen4/tests.o \
               $(foreach module,$(GEN4_TEST_MODULES),testsGen4/$(module).o)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

testsGen4/tests.c: $(foreach module,$(GEN4_TEST_MODULES),testsGen4/$(module).c)
	@echo "/* This file is automatically generated. */" >$@
	@echo '#include "tests.h"' >>$@
	@grep -ho 'TEST_\(LOGGER[4]\|PARSER\)([A-Za-z_][A-Za-z0-9_]*\(, .*\)\?)' \
			$^ \
		| sed -e 's/$$/;/' >>$@
	@echo "InstrumentTest instrumentTests[] = {" >>$@
	@grep -ho 'TEST_LOGGER[4]([A-Za-z_][A-Za-z0-9_]*)' $^ \
		| sed -e 's/^TEST_LOGGER\([^(]*\)(\([^)]*\))/    {"\2", RBRGEN4_LOGGER\1, test_\2_l\1},/' \
		>>$@
	@echo "    {0}" >>$@
	@echo "};" >>$@

	@grep -ho 'TEST_PARSER_CONFIG([A-Za-z_][A-Za-z0-9_]*)' $^ \
		| sed -e 's/^TEST_PARSER_CONFIG(\([^,]*\))/extern const RBRParserGen4Config test_\1_parser_config;/' \
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
	rm -Rf src/*.o bin/ testsGen3/tests.c testsGen3/*.o testsGen4/tests.c testsGen4/*.o docs/
