/**
 * \file posix-generation.c
 *
 * \brief Example of using the library to check generation of an instrument.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for errno. */
#include <errno.h>
/* Required for isnan. */
#include <math.h>
/* Required for fprintf, printf, snprintf. */
#include <stdio.h>
/* Required for EXIT_SUCCESS, etc. */
#include <stdlib.h>
/* Required for strerror. */
#include <string.h>
/* Required for close. */
#include <unistd.h>

#include "posix-shared.h"

int main(int argc, char *argv[])
{
    char *programName = argv[0];
    char *devicePath;

    int status = EXIT_SUCCESS;
    int instrumentFd;

    RBRGen4Error err;
    RBRGen4 *conn = NULL;
    //no dynamic allocation case:
    RBRGen4 instrumentSpace;
    conn = &instrumentSpace;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s device\n", argv[0]);
        return EXIT_FAILURE;
    }

    devicePath = argv[1];

    if ((instrumentFd = openSerialFd(devicePath)) < 0)
    {
        fprintf(stderr, "%s: Failed to open serial device: %s!\n",
                programName,
                strerror(errno));
        return EXIT_FAILURE;
    }

    fprintf(stderr,
            "%s: Using %s v%s (built %s).\n",
            programName,
            RBRGEN4_LIB_NAME,
            RBRGEN4_LIB_VERSION,
            RBRGEN4_LIB_BUILD_DATE);

    RBRGen4Callbacks callbacks = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite
    };

    if ((err = RBRGen4_open(
             conn,
             &callbacks,
             INSTRUMENT_COMMAND_TIMEOUT_MSEC,
             (void *) &instrumentFd)) != RBRGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to establish instrument connection: %s!\n",
                programName,
                RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup; //Failure case, memory allocated by this constructor is freed.
    }

 RBRGen4Generation generation;
 generation =  RBRGen4_getGeneration(conn);
 if (generation != RBRGEN4_LOGGER4)
 {
    fprintf(stderr, "%s: Instrument generation %s not supported. Please check libRBR version.\n",
            programName,
            RBRGen4Generation_name(generation));
            status = EXIT_FAILURE;
            goto instrumentCleanup;
 }

instrumentCleanup:
    RBRGen4_close(conn);
fileCleanup:
    close(instrumentFd);
    return status;
}
