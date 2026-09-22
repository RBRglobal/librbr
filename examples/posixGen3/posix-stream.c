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

/* Prerequisite for gmtime_r in time.h. */
#define _POSIX_C_SOURCE 200112L

/* Required for errno. */
#include <errno.h>
/* Required for fprintf, printf. */
#include <stdio.h>
/* Required for EXIT_FAILURE, EXIT_SUCCESS. */
#include <stdlib.h>
/* Required for strerror. */
#include <string.h>
/* Required for gmtime_r, time_t, strftime. */
#include <time.h>
/* Required for close. */
#include <unistd.h>

#include "posix-shared.h"

RBRGen3Error instrumentSample(const struct RBRGen3 *conn, const struct RBRGen3Sample *const sample)
{
    /* Unused. */
    (void) conn;

    char ftime[128];
    time_t sampleSeconds = (time_t) (sample->timestamp / 1000);
    struct tm sampleTime;
    gmtime_r(&sampleSeconds, &sampleTime);
    strftime(ftime, sizeof(ftime), "%F %T", &sampleTime);

    printf("%s.%03" PRIi64, ftime, sample->timestamp % 1000);
    for (int32_t i = 0; i < sample->channels; i++) {
        printf(", %lf", sample->readings[i]);
    }
    printf("\n");

    return RBRGEN3_SUCCESS;
}

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

    RBRGen3Sample sampleBuffer;
    RBRGen3Environment environment = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite,
        .sample = instrumentSample,
        .sampleBuffer = &sampleBuffer,
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
        RBRGen3_setUSBStreamingState(&conn, true);
        break;
    case RBRGEN3_LINK_SERIAL:
    case RBRGEN3_LINK_WIFI: {
        RBRGen3Serial serial;
        RBRGen3_getSerial(&conn, &serial);
        printf("Connected in %s mode at %s baud.\n",
               RBRGen3SerialMode_name(serial.mode),
               RBRGen3SerialBaudRate_name(serial.baudRate));

        RBRGen3_setSerialStreamingState(&conn, true);
        break;
    }
    default:
        fprintf(stderr,
                "I don't know how I'm connected to the instrument, so I can't"
                " enable streaming. Giving up.\n");
        goto instrumentCleanup;
    }

    RBRGen3Deployment deployment;
    RBRGen3_getDeployment(&conn, &deployment);
    if (deployment.status != RBRGEN3_STATUS_LOGGING) {
        printf("%s: Instrument is %s, not logging. I'm going to start it.\n",
               programName,
               RBRGen3DeploymentStatus_name(deployment.status));

        if ((err = instrumentStart(&conn)) != RBRGEN3_SUCCESS) {
            fprintf(stderr,
                    "%s: Failed to start instrument: %s!\n",
                    programName,
                    RBRGen3Error_name(err));
            status = EXIT_FAILURE;
            goto instrumentCleanup;
        }
    }

    while (true) {
        if ((err = RBRGen3_readSample(&conn)) != RBRGEN3_SUCCESS) {
            fprintf(stderr, "Error: %s\n", RBRGen3Error_name(err));
        }
    }

instrumentCleanup:
    RBRGen3_close(&conn);
fileCleanup:
    close(instrumentFd);

    return status;
}
