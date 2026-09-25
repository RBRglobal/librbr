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
/* Required for EXIT_FAILURE, EXIT_SUCCESS. */
#include <stdlib.h>
/* Required for strerror. */
#include <string.h>
/* Required for close. */
#include <unistd.h>

#include "posix-shared.h"
#include "RBRGen3Commands.h"

int main(int argc, char *argv[])
{
    char *programName = argv[0];
    char *devicePath;

    int status = EXIT_SUCCESS;
    int instrumentFd;

    RBRGen3Error err;
    RBRGen3 conn;
    uint8_t commandBuffer[RBRGEN3_COMMAND_BUFFER_DEFAULT];
    uint8_t responseBuffer[RBRGEN3_RESPONSE_BUFFER_DEFAULT];

    if (argc < 2) {
        fprintf(stderr, "Usage: %s device\n", argv[0]);
        return EXIT_FAILURE;
    }

    devicePath = argv[1];

    if ((instrumentFd = openSerialFd(devicePath)) < 0) {
        fprintf(stderr, "%s: Failed to open serial device: %s!\n", programName, strerror(errno));
        return EXIT_FAILURE;
    }

    fprintf(stderr, "%s: Using %s v%s.\n", programName, RBRGEN3_LIB_NAME, RBRGEN3_LIB_VERSION);

    RBRGen3Environment environment = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite,
        .command = commandBuffer,
        .commandCapacity = sizeof(commandBuffer),
        .response = responseBuffer,
        .responseCapacity = sizeof(responseBuffer),
    };

    if ((err = RBRGen3_open(
             &conn, &environment, INSTRUMENT_COMMAND_TIMEOUT_MSEC, (void *) &instrumentFd)) !=
        RBRGEN3_SUCCESS) {
        fprintf(stderr,
                "%s: Failed to establish instrument connection: %s!\n",
                programName,
                RBRGen3Error_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    RBRGen3Link link;
    RBRGen3_getLink(&conn, &link);
    printf("Connected to the instrument via %s.\n", RBRGen3Link_name(link));

    switch (link) {
    case RBRGEN3_LINK_USB:
        RBRGen3_setUSBStreamingState(&conn, false);
        break;
    case RBRGEN3_LINK_SERIAL:
    case RBRGEN3_LINK_WIFI: {
        RBRGen3Serial serial;
        RBRGen3_getSerial(&conn, &serial);
        printf("Connected in %s mode at %s baud.\n",
               RBRGen3SerialMode_name(serial.mode),
               RBRGen3SerialBaudRate_name(serial.baudRate));

        RBRGen3_setSerialStreamingState(&conn, false);
        break;
    }
    default:
        fprintf(stderr,
                "Warning: I don't know how I'm connected to the instrument, so"
                " I can't disable streaming.\n");
        goto instrumentCleanup;
    }

    RBRGen3Sample sample;
    while (true) {
        err = RBRGen3_fetch(&conn, NULL, false, &sample);
        if (err != RBRGEN3_SUCCESS) {
            fprintf(stderr, "Error: %s\n", RBRGen3Error_name(err));
        } else {
            printf("%" PRIi64, sample.timestamp);
            for (int32_t i = 0; i < sample.channels; i++) {
                switch (RBRGen3Reading_getFlag(sample.readings[i])) {
                case RBRGEN3_READING_FLAG_UNCALIBRATED:
                    printf(", ###");
                    break;
                case RBRGEN3_READING_FLAG_ERROR:
                    printf(", Error-%2d", RBRGen3Reading_getError(sample.readings[i]));
                    break;
                case RBRGEN3_READING_FLAG_NONE:
                default:
                    printf(", %lf", sample.readings[i]);
                    break;
                }
            }
            printf("\n");
        }
    }

instrumentCleanup:
    RBRGen3_close(&conn);
fileCleanup:
    close(instrumentFd);

    return status;
}
