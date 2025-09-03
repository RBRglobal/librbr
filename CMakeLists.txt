cmake_minimum_required(VERSION 3.12)

# Read version from VERSION file
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" LIB_VERSION_RAW)
string(STRIP "${LIB_VERSION_RAW}" LIB_VERSION)
string(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+" LIB_VERSION "${LIB_VERSION}")

# Project configuration
project(libRBR 
    VERSION ${LIB_VERSION}
    DESCRIPTION "RBR Instrument Library"
    LANGUAGES C
)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Project variables (can be overridden)
set(LIB_NAME "libRBR" CACHE STRING "Library name")
set(LIB_VERSION "${PROJECT_VERSION}" CACHE STRING "Library version")

# Get build date
string(TIMESTAMP LIB_BUILD_DATE "%Y-%m-%dT%H:%M:%S%z")

# C standard and compiler flags
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags matching the Makefile
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wall -Wextra -pedantic -pedantic-errors -Wwrite-strings")

# Debug symbols (equivalent to -g in Makefile)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g")

# Include directories
include_directories(include)

# Define preprocessor macros
add_definitions(
    -DRBR_LIB_NAME="${LIB_NAME}"
    -DRBR_LIB_VERSION="${LIB_VERSION}"
    -DRBR_LIB_BUILD_DATE="${LIB_BUILD_DATE}"
)

# Source files for main library
set(RBR_SOURCES
    src/RBRInstrument.c
    src/RBRInstrumentCommunication.c
    src/RBRInstrumentConfiguration.c
    src/RBRInstrumentDeployment.c
    src/RBRInstrumentFetching.c
    src/RBRInstrumentGating.c
    src/RBRInstrumentHardwareErrors.c
    src/RBRInstrumentInternal.c
    src/RBRInstrumentMemory.c
    src/RBRInstrumentOther.c
    src/RBRInstrumentPauseresume.c
    src/RBRInstrumentSchedule.c
    src/RBRInstrumentSecurity.c
    src/RBRInstrumentStreaming.c
    src/RBRInstrumentVehicle.c
    src/RBRParser.c
)

# Source files for dynamic correction library
set(RBR_DYNAMIC_CORRECTION_SOURCES
    src/RBRDynamicCorrection.c
)

# Create main static library
add_library(RBR STATIC ${RBR_SOURCES})
set_target_properties(RBR PROPERTIES
    OUTPUT_NAME "RBR"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Create dynamic correction static library
add_library(RBRDynamicCorrection STATIC ${RBR_DYNAMIC_CORRECTION_SOURCES})
set_target_properties(RBRDynamicCorrection PROPERTIES
    OUTPUT_NAME "RBRDynamicCorrection"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Test configuration (optional)
option(BUILD_TESTS "Build tests" OFF)

if(BUILD_TESTS)
    message(STATUS "Tests are disabled in this simplified CMake configuration")
    message(STATUS "Use the original Makefile for running tests: make tests")
endif()

# Documentation with Doxygen
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    option(BUILD_DOCS "Build documentation" ON)
    
    if(BUILD_DOCS)
        # Regular documentation
        add_custom_target(docs
            COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tools/Doxyfile"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating documentation with Doxygen"
        )
        
        # Developer documentation
        add_custom_target(devdocs
            COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tools/Doxyfile-devdocs"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating developer documentation with Doxygen"
        )
    endif()
endif()

# Install configuration
install(TARGETS RBR RBRDynamicCorrection
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Print configuration summary
message(STATUS "libRBR Configuration Summary:")
message(STATUS "  Library Name: ${LIB_NAME}")
message(STATUS "  Version: ${LIB_VERSION}")
message(STATUS "  Build Date: ${LIB_BUILD_DATE}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Libraries: libRBR.a, libRBRDynamicCorrection.a")
if(DOXYGEN_FOUND)
    message(STATUS "  Build Docs: ${BUILD_DOCS}")
else()
    message(STATUS "  Build Docs: OFF (Doxygen not found)")
endif()
message(STATUS "  Output Directory: ${CMAKE_BINARY_DIR}/bin")
message(STATUS "")
message(STATUS "Available targets:")
message(STATUS "  make RBR                    - Build main library")
message(STATUS "  make RBRDynamicCorrection   - Build dynamic correction library") 
message(STATUS "  make all                    - Build both libraries")
if(DOXYGEN_FOUND)
    message(STATUS "  make docs                   - Generate documentation")
    message(STATUS "  make devdocs                - Generate developer documentation")
endif()
message(STATUS "")
message(STATUS "For tests, use the original Makefile: make tests")