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
/* Required for strerror. */
#include <string.h>
/* Required for gmtime_r, time_t, strftime. */
#include <time.h>
/* Required for close. */
#include <unistd.h>

#include "posix-shared.h"

RBRInstrumentError instrumentSample(
    const struct RBRInstrument *instrument,
    const struct RBRInstrumentSample *const sample)
{
    /* Unused. */
    (void) instrument;

    char ftime[128];
    time_t sampleSeconds = (time_t) (sample->timestamp / 1000);
    struct tm sampleTime;
    gmtime_r(&sampleSeconds, &sampleTime);
    strftime(ftime, sizeof(ftime), "%F %T", &sampleTime);

    printf("%s.%03" PRIi64, ftime, sample->timestamp % 1000);
    for (int32_t i = 0; i < sample->channels; i++)
    {
        printf(", %lf", sample->readings[i]);
    }
    printf("\n");

    return RBRINSTRUMENT_SUCCESS;
}

int main(int argc, char *argv[])
{
    char *programName = argv[0];
    char *devicePath;

    int status = EXIT_SUCCESS;
    int instrumentFd;

    RBRInstrumentError err;
    RBRInstrument *instrument = NULL;
    #ifdef RBR_LIB_NODYNAMICMEMORYALLOCATION
    RBRInstrument instrumentSpace;
    instrument = &instrumentSpace;
    #endif

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
            RBRINSTRUMENT_LIB_NAME,
            RBRINSTRUMENT_LIB_VERSION,
            RBRINSTRUMENT_LIB_BUILD_DATE);

    RBRInstrumentSample sampleBuffer;
    RBRInstrumentCallbacks callbacks = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite,
        .sample = instrumentSample,
        .sampleBuffer = &sampleBuffer
    };

    if ((err = RBRInstrument_open(
             &instrument,
             &callbacks,
             INSTRUMENT_COMMAND_TIMEOUT_MSEC,
             (void *) &instrumentFd)) != RBRINSTRUMENT_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to establish instrument connection: %s!\n",
                programName,
                RBRInstrumentError_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    RBRInstrumentLink link;
    RBRInstrument_getLink(instrument, &link);
    printf("Connected to the instrument via %s.\n",
           RBRInstrumentLink_name(link));

    switch (link)
    {
    case RBRINSTRUMENT_LINK_USB:
        RBRInstrument_setUSBStreamingState(instrument, true);
        break;
    case RBRINSTRUMENT_LINK_SERIAL:
    case RBRINSTRUMENT_LINK_WIFI:
        {
            RBRInstrumentSerial serial;
            RBRInstrument_getSerial(instrument, &serial);
            printf("Connected in %s mode at %s baud.\n",
                   RBRInstrumentSerialMode_name(serial.mode),
                   RBRInstrumentSerialBaudRate_name(serial.baudRate));

            RBRInstrument_setSerialStreamingState(instrument, true);
            break;
        }
    default:
        fprintf(stderr,
                "I don't know how I'm connected to the instrument, so I can't"
                " enable streaming. Giving up.\n");
        goto instrumentCleanup;
    }

    RBRInstrumentDeployment deployment;
    RBRInstrument_getDeployment(instrument, &deployment);
    if (deployment.status != RBRINSTRUMENT_STATUS_LOGGING)
    {
        printf("%s: Instrument is %s, not logging. I'm going to start it.\n",
               programName,
               RBRInstrumentDeploymentStatus_name(deployment.status));

        if ((err = instrumentStart(instrument)) != RBRINSTRUMENT_SUCCESS)
        {
            fprintf(stderr,
                    "%s: Failed to start instrument: %s!\n",
                    programName,
                    RBRInstrumentError_name(err));
            status = EXIT_FAILURE;
            goto instrumentCleanup;
        }
    }

    while (true)
    {
        if ((err = RBRInstrument_readSample(instrument)) != RBRINSTRUMENT_SUCCESS)
        {
            fprintf(stderr, "Error: %s\n", RBRInstrumentError_name(err));
        }
    }

instrumentCleanup:
    RBRInstrument_close(instrument);
fileCleanup:
    close(instrumentFd);

    return status;
}
