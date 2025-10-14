# Copyright (c) 2025 RBR Ltd.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(librbr_example)

target_sources(app PRIVATE
    src/instrument.c
    src/io.c
    src/main.c
    src/time.c
)

# Enforce the use of 64-bit time_t when building for 32-bit x86_64 on Linux
# 5.1+ and glibc 2.34+.
#
# See:
# - https://sourceware.org/glibc/manual/2.34/html_node/Feature-Test-Macros.html#index-_005fTIME_005fBITS
# - https://sourceware.org/glibc/manual/2.34/html_node/Feature-Test-Macros.html#index-_005fFILE_005fOFFSET_005fBITS
#
# We check the concatenation of the BOARD and its BOARD_QUALIFIERS here because
# CONFIG_X86 and/or CONFIG_64BIT are not set alongside CONFIG_NATIVE_BUILD.
if("${BOARD}${BOARD_QUALIFIERS}" STREQUAL "native_sim/native")
    # We use zephyr_compile_options() here instead of target_compile_options()
    # to apply this to all compilation units – ours, Zephyr's, and modules'.
    zephyr_compile_options(-D_TIME_BITS=64 -D_FILE_OFFSET_BITS=64)
endif()
