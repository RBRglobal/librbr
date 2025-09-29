/**
 * \file posix-stream.c
 *
 * \brief Example of using the library to stream instrument data in a POSIX
 * environment.
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

    RBRInstrumentError err;
    RBRInstrument *instrument = NULL;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s device\n", argv[0]);
        return EXIT_FAILURE;
    }

    devicePath = argv[1];

    if ((instrumentFd = openSerialFd(devicePath)) < 0) {
        fprintf(stderr, "%s: Failed to open serial device: %s!\n", programName, strerror(errno));
        return EXIT_FAILURE;
    }

    fprintf(stderr,
            "%s: Using %s v%s (built %s).\n",
            programName,
            RBRINSTRUMENT_LIB_NAME,
            RBRINSTRUMENT_LIB_VERSION,
            RBRINSTRUMENT_LIB_BUILD_DATE);

    RBRInstrumentCallbacks callbacks = {.time = instrumentTime,
                                        .sleep = instrumentSleep,
                                        .read = instrumentRead,
                                        .write = instrumentWrite};

    if ((err = RBRInstrument_open(
             &instrument, &callbacks, INSTRUMENT_COMMAND_TIMEOUT_MSEC, (void *) &instrumentFd)) !=
        RBRINSTRUMENT_SUCCESS) {
        fprintf(stderr,
                "%s: Failed to establish instrument connection: %s!\n",
                programName,
                RBRInstrumentError_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    RBRInstrumentLink link;
    RBRInstrument_getLink(instrument, &link);
    printf("Connected to the instrument via %s.\n", RBRInstrumentLink_name(link));

    switch (link) {
    case RBRINSTRUMENT_LINK_USB:
        RBRInstrument_setUSBStreamingState(instrument, false);
        break;
    case RBRINSTRUMENT_LINK_SERIAL:
    case RBRINSTRUMENT_LINK_WIFI: {
        RBRInstrumentSerial serial;
        RBRInstrument_getSerial(instrument, &serial);
        printf("Connected in %s mode at %s baud.\n",
               RBRInstrumentSerialMode_name(serial.mode),
               RBRInstrumentSerialBaudRate_name(serial.baudRate));

        RBRInstrument_setSerialStreamingState(instrument, false);
        break;
    }
    default:
        fprintf(stderr,
                "Warning: I don't know how I'm connected to the instrument, so"
                " I can't disable streaming.\n");
        goto instrumentCleanup;
    }

    RBRInstrumentSample sample;
    while (true) {
        err = RBRInstrument_fetch(instrument, NULL, false, &sample);
        if (err != RBRINSTRUMENT_SUCCESS) {
            fprintf(stderr, "Error: %s\n", RBRInstrumentError_name(err));
        } else {
            printf("%" PRIi64, sample.timestamp);
            for (int32_t i = 0; i < sample.channels; i++) {
                switch (RBRInstrumentReading_getFlag(sample.readings[i])) {
                case RBRINSTRUMENT_READING_FLAG_UNCALIBRATED:
                    printf(", ###");
                    break;
                case RBRINSTRUMENT_READING_FLAG_ERROR:
                    printf(", Error-%2d", RBRInstrumentReading_getError(sample.readings[i]));
                    break;
                case RBRINSTRUMENT_READING_FLAG_NONE:
                default:
                    printf(", %lf", sample.readings[i]);
                    break;
                }
            }
            printf("\n");
        }
    }

instrumentCleanup:
    RBRInstrument_close(instrument);
fileCleanup:
    close(instrumentFd);

    return status;
}
