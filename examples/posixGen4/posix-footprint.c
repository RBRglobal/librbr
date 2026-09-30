/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file posix-footprint.c
 *
 * \brief Print the memory footprint of the library's structures.
 *
 * No instrument is needed.
 */

/* Required for printf. */
#include <stdio.h>
/* Required for EXIT_SUCCESS. */
#include <stdlib.h>

#include "RBRGen4.h"
#include "RBRGen4Commands.h"

#define PRINT_SIZE(type) printf("%-26s %6zu bytes\n", #type, sizeof(type))

int main(void)
{
    /* The instrument connection, including its command and response
     * buffers. */
    PRINT_SIZE(RBRGen4);

    /* One entry of each user-provided pool. The pool structures themselves
     * hold only a pointer to the caller's buffer, so the footprint of a pool
     * is the entry size multiplied by the number of entries allocated. */
    PRINT_SIZE(RBRGen4Channel);
    PRINT_SIZE(RBRGen4Group);
    PRINT_SIZE(RBRGen4Schedule);
    PRINT_SIZE(RBRGen4Config);
    PRINT_SIZE(RBRGen4Dataset);
    PRINT_SIZE(RBRGen4Label);

    PRINT_SIZE(RBRGen4Calibration);
    PRINT_SIZE(RBRGen4Sample);

    return EXIT_SUCCESS;
}
