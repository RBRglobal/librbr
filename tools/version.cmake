# Copyright (c) 2026 RBR Ltd.
# SPDX-License-Identifier: Apache-2.0

## \file version.cmake
##
## \brief Determine the version number used for builds.
##
## CMake equivalent of version.sh.
##
## Tries to use the output of `git describe --dirty --always`. If that fails
## (e.g., Git unavailable, or no/invalid .git metadata), uses the contents of
## the VERSION file.

function(librbr_version WORKING_DIRECTORY VERSION_OUT)
    # Do we have Git, and do we have repository metadata for libRBR (not for
    # some parent Git repository vendoring it)?
    execute_process(
        COMMAND git rev-parse --show-prefix
        WORKING_DIRECTORY ${WORKING_DIRECTORY}
        RESULT_VARIABLE err
        OUTPUT_VARIABLE prefix
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(err EQUAL 0 AND prefix STREQUAL "")
        execute_process(
            COMMAND git describe --dirty
            WORKING_DIRECTORY ${WORKING_DIRECTORY}
            RESULT_VARIABLE err
            OUTPUT_VARIABLE version
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        if(err EQUAL 0)
            # We know about a tag, so use that (sans leading “v”, if present).
            string(SUBSTRING ${version} 0 1 prefix)
            if(prefix STREQUAL "v")
                string(SUBSTRING ${version} 1 -1 version)
            endif()
            set(${VERSION_OUT} ${version} PARENT_SCOPE)
            return()
        endif()

        execute_process(
            COMMAND git describe --dirty --always
            WORKING_DIRECTORY ${WORKING_DIRECTORY}
            RESULT_VARIABLE err
            OUTPUT_VARIABLE version
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        if(err EQUAL 0)
            # Couldn't find a tag; we're probably in a shallow clone. Prepend a
            # version number so that the output matches the same pattern.
            set(${VERSION_OUT} "0.0.0-0-g${version}" PARENT_SCOPE)
            return()
        endif()
    endif()

    if(EXISTS ${WORKING_DIRECTORY}/VERSION)
        file(STRINGS ${WORKING_DIRECTORY}/VERSION version_list LIMIT_COUNT 1)
        list(GET version_list 0 version)

        set(${VERSION_OUT} ${version} PARENT_SCOPE)
        return()
    endif()

    message(FATAL_ERROR "no Git information or VERSION file available!")
endfunction()
