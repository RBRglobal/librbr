# Copyright (c) 2018 RBR Ltd.
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_LIBRBR)

# Read version from VERSION file
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../VERSION" LIB_VERSION_RAW)
string(STRIP "${LIB_VERSION_RAW}" LIB_VERSION)
string(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+" LIB_VERSION "${LIB_VERSION}")

# Get build date
string(TIMESTAMP LIB_BUILD_DATE "%Y-%m-%dT%H:%M:%S%z")

# Set library name
set(LIB_NAME "librbr")

# Main RBR library sources
set(RBR_SOURCES
    ../src/RBRInstrument.c
    ../src/RBRInstrumentCommunication.c
    ../src/RBRInstrumentConfiguration.c
    ../src/RBRInstrumentDeployment.c
    ../src/RBRInstrumentFetching.c
    ../src/RBRInstrumentGating.c
    ../src/RBRInstrumentHardwareErrors.c
    ../src/RBRInstrumentInternal.c
    ../src/RBRInstrumentMemory.c
    ../src/RBRInstrumentOther.c
    ../src/RBRInstrumentPauseresume.c
    ../src/RBRInstrumentSchedule.c
    ../src/RBRInstrumentSecurity.c
    ../src/RBRInstrumentStreaming.c
    ../src/RBRInstrumentVehicle.c
    ../src/RBRParser.c
)

# Create the main RBR library
zephyr_library_named(rbr)
zephyr_library_sources(${RBR_SOURCES})

# Include directories
zephyr_library_include_directories(../include)

# Preprocessor definitions
zephyr_library_compile_definitions(
    RBR_LIB_NAME="${LIB_NAME}"
    RBR_LIB_VERSION="${LIB_VERSION}"
    RBR_LIB_BUILD_DATE="${LIB_BUILD_DATE}"
)

# Compiler flags to match original Makefile behavior
zephyr_library_compile_options(
    -Wall
    -Wextra
    -pedantic
    -Wwrite-strings
    -std=c17
    -Os
)

# Dynamic correction library (optional)
if(CONFIG_LIBRBR_DYNAMIC_CORRECTION)
    zephyr_library_named(rbr_dynamic_correction)
    zephyr_library_sources(../src/RBRDynamicCorrection.c)
    zephyr_library_include_directories(../include)
    
    # Same preprocessor definitions for dynamic correction
    zephyr_library_compile_definitions(
        RBR_LIB_NAME="${LIB_NAME}"
        RBR_LIB_VERSION="${LIB_VERSION}"
        RBR_LIB_BUILD_DATE="${LIB_BUILD_DATE}"
    )
    
    # Same compiler flags
    zephyr_library_compile_options(
        -Wall
        -Wextra
        -pedantic
        -Wwrite-strings
        -std=c17
        -Os
    )
endif()

endif() # CONFIG_LIBRBR