/**
 * \file posix-postprocessing.c
 *
 * \brief Example of using the library to invoke the post-processing
 * functionality introduced in 3rd-generation instruments in firmware v1.102.
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
/* Required for time. */
#include <time.h>
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

    RBRInstrumentCallbacks callbacks = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite
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
        goto serialCleanup;
    }

    RBRInstrumentMemoryInfo meminfo;
    meminfo.dataset = RBRINSTRUMENT_DATASET_EASYPARSE_SAMPLE_DATA;
    RBRInstrument_getMemoryInfo(instrument, &meminfo);
    printf("Dataset %s is %0.2f%% full (%" PRIi32 "B used).\n",
           RBRInstrumentDataset_name(meminfo.dataset),
           ((float) meminfo.used) / meminfo.size * 100,
           meminfo.used);

    if (meminfo.used == 0)
    {
        fprintf(stderr,
                "%s: Can't perform post-processing without data! Giving up.\n",
                programName);
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    RBRInstrumentMemoryFormat memformat;
    RBRInstrument_getCurrentMemoryFormat(instrument, &memformat);
    printf("It's currently storing data of format %s.\n",
           RBRInstrumentMemoryFormat_name(memformat));

    if (memformat != RBRINSTRUMENT_MEMFORMAT_CALBIN00)
    {
        fprintf(stderr,
                "%s: Post-processing can only operate on EasyParse datasets! "
                "Giving up.\n",
                programName);
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    RBRInstrumentPostprocessing postprocessing;
    if ((err = RBRInstrument_getPostprocessing(
             instrument,
             &postprocessing))
        != RBRINSTRUMENT_SUCCESS)
    {
        fprintf(stderr,
                "%s: Failure retrieving post-processing configuration: %s!\n",
                programName,
                RBRInstrumentError_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    if (postprocessing.status != RBRINSTRUMENT_POSTPROCESSING_STATUS_IDLE)
    {
        if ((err = RBRInstrument_setPostprocessingCommand(
                 instrument,
                 RBRINSTRUMENT_POSTPROCESSING_COMMAND_RESET,
                 &postprocessing.status))
            != RBRINSTRUMENT_SUCCESS)
        {
            fprintf(stderr,
                    "%s: Failure resetting post-processing state: %s!\n",
                    programName,
                    RBRInstrumentError_name(err));
            status = EXIT_FAILURE;
            goto instrumentCleanup;
        }
    }

    RBRInstrumentDateTime now = time(NULL);
    now *= 1000;

    postprocessing = (RBRInstrumentPostprocessing) {
        .channels = {
            .count = 3,
            .channels = {
                {
                    .function = RBRINSTRUMENT_POSTPROCESSING_AGGREGATE_SAMPLE_COUNT,
                    .label = "pressure_00"
                },
                {
                    .function = RBRINSTRUMENT_POSTPROCESSING_AGGREGATE_MEAN,
                    .label = "temperature_00"
                },
                {
                    .function = RBRINSTRUMENT_POSTPROCESSING_AGGREGATE_STD,
                    .label = "temperature_00"
                }
            }
        },
        .binReference = "tstamp",
        .binFilter = RBRINSTRUMENT_POSTPROCESSING_BINFILTER_NONE,
        .binSize = 0,
        .tstampMin = now - 1800000LL /* data from the last half-hour */,
        .tstampMax = now,
        .depthMin = 0.0,
        .depthMax = 0.0,
        .dcAlpha = 0.08,
        .dcTau = 8.0,
        .dcTdelay = 0.35,
        .dcCtCoeff = 2.4e-4
    };

    if ((err = RBRInstrument_setPostprocessing(
             instrument,
             &postprocessing) != RBRINSTRUMENT_SUCCESS)
        != RBRINSTRUMENT_SUCCESS)
    {
        fprintf(stderr,
                "%s: Failure setting post-processing configuration: %s!\n",
                programName,
                RBRInstrumentError_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    if ((err = RBRInstrument_setPostprocessingCommand(
             instrument,
             RBRINSTRUMENT_POSTPROCESSING_COMMAND_START,
             &postprocessing.status))
        != RBRINSTRUMENT_SUCCESS)
    {
        fprintf(stderr,
                "%s: Failure starting post-processing: %s!\n",
                programName,
                RBRInstrumentError_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    do
    {
        sleep(1);

        printf("Checking post-processing status...\n");

        if ((err = RBRInstrument_getPostprocessing(
                 instrument,
                 &postprocessing))
            != RBRINSTRUMENT_SUCCESS)
        {
            fprintf(stderr,
                    "%s: Failure retrieving post-processing configuration: %s!\n",
                    programName,
                    RBRInstrumentError_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
        }
    } while (postprocessing.status == RBRINSTRUMENT_POSTPROCESSING_STATUS_PROCESSING);

    if (postprocessing.status != RBRINSTRUMENT_POSTPROCESSING_STATUS_COMPLETED)
    {
        fprintf(stderr,
                "%s: Expected to find that the post-processing had completed, "
                "but instead found that it was %s!\n",
                programName,
                RBRInstrumentPostprocessingStatus_name(postprocessing.status));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }
    else
    {
        printf("%s: Post-processing has concluded. See `posix-download` for an "
               "example of downloading data.\n",
               programName);
    }

instrumentCleanup:
    RBRInstrument_close(instrument);
serialCleanup:
    close(instrumentFd);

    return status;
}
