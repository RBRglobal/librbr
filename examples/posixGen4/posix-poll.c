/**
 * \file posix-poll.c
 *
 * \brief Example of using the library to poll instrument data in a POSIX
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
/* Required for EXIT_SUCCESS, etc. */
#include <stdlib.h>
/* Required for strerror. */
#include <string.h>
/* Required for close. */
#include <unistd.h>

#include "posix-shared.h"

//**************** customer defined parameters ******************************** //
#define PRESSURE "pressure_00"
#define TEMPERATURE "temperature_00"
#define SALINITY_DYNCORR "salinitydyncorr_00"

#define GROUP_PTS_LABEL "gr_pts"
#define GROUP_PTS_CHANNELS \
(const char[][RBRINSTRUMENTGEN4_CHANNEL_LABEL_MAX]) \
{   \
    PRESSURE, \
        TEMPERATURE, \
        SALINITY_DYNCORR \
}

#define OUTPUTFORMAT RBRINSTRUMENTGEN4_OUTPUTFORMAT_SERIAL | RBRINSTRUMENTGEN4_OUTPUTFORMAT_SCHEDULELABEL | RBRINSTRUMENTGEN4_OUTPUTFORMAT_CRC

int main(int argc, char *argv[])
{
    // check all arguments are provided
    char *programName = argv[0];
    char *devicePath;

    int status = EXIT_SUCCESS;
    int instrumentFd;

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4 instrumentSpace;
    RBRInstrumentGen4 *instrument = &instrumentSpace;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s device\n", argv[0]);
        return EXIT_FAILURE;
    }

    // check port communication
    devicePath = argv[1];
    if ((instrumentFd = openSerialFd(devicePath)) < 0)
    {
        fprintf(stderr, "%s: Failed to open serial device: %s!\n", programName, strerror(errno));
        return EXIT_FAILURE;
    }

    fprintf(stderr,
            "%s: Using %s v%s (built %s).\n",
            programName,
            RBRINSTRUMENTGEN4_LIB_NAME,
            RBRINSTRUMENTGEN4_LIB_VERSION,
            RBRINSTRUMENTGEN4_LIB_BUILD_DATE);

    // check instrument communication is fine.
    RBRInstrumentGen4Callbacks callbacks = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite
    };

    if ((err = RBRInstrumentGen4_open(
             &instrument,
             &callbacks,
             INSTRUMENT_COMMAND_TIMEOUT_MSEC,
             (void *) &instrumentFd)) != RBRINSTRUMENTGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to establish instrument connection: %s!\n", programName, RBRInstrumentGen4Error_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup; // Failure case, memory allocated by this constructor is freed.
    }

    //------(optional) get link type: USB/serial/wifi---------------------------------------------
    RBRInstrumentGen4Link link;
    RBRInstrumentGen4_getLink(instrument, &link);
    printf("Connected to the instrument via %s.\n",
           RBRInstrumentGen4Link_name(link));

    RBRInstrumentGen4Serial serial;

    switch (link)
    {
    case RBRINSTRUMENTGEN4_LINK_USB:
        break;
    case RBRINSTRUMENTGEN4_LINK_SERIAL:
    {
        RBRInstrumentGen4_getSerial(instrument, &serial);
        printf("Connected in %s mode at %s baud.\n",
               RBRInstrumentGen4SerialMode_name(serial.mode),
               RBRInstrumentGen4SerialBaudRate_name(serial.baudRate));
        break;
    }
    default:
        fprintf(stderr,
                "Warning: connection method to the instrument is unclear, so"
                " poll can't be executed.\n");
        goto instrumentCleanup;
    }

    // populate all channels and calibrations.
    RBRInstrumentGen4ChannelPool channelPool;
    RBRInstrumentGen4_getChannelPool(instrument, &channelPool);

    // create group
    RBRInstrumentGen4GroupPool groupPool;
    RBRInstrumentGen4_getGroupPool(instrument, &groupPool);

    // specify outputformat
    RBRInstrumentGen4Outputformat outputformat = 0;
    RBRInstrumentGen4_getOutputformat(instrument, &outputformat);
    outputformat = OUTPUTFORMAT;
    RBRInstrumentGen4_setOutputformat(instrument, outputformat);

    // poll data and print in console
    RBRInstrumentGen4Sample sample;
    while (true)
    {
        // poll one group
        err = RBRInstrumentGen4_pollOneGroup(instrument, groupPool.pool[0].label, &sample);
        if (err != RBRINSTRUMENTGEN4_SUCCESS)
        {
            fprintf(stderr, "Error: %s\n", RBRInstrumentGen4Error_name(err));
        }
        else
        {
            printf("%" PRIi64, sample.timestamp);
            for (int32_t i = 0; i < sample.channelCount; i++)
            {
                switch (RBRInstrumentGen4Reading_getFlag(sample.readings[i]))
                {
                case RBRINSTRUMENTGEN4_READING_FLAG_UNCALIBRATED:
                    printf(", ###");
                    break;
                case RBRINSTRUMENTGEN4_READING_FLAG_ERROR:
                    printf(", Error-%2d", RBRInstrumentGen4Reading_getError(sample.readings[i]));
                    break;
                case RBRINSTRUMENTGEN4_READING_FLAG_NONE:
                default:
                    printf(", %lf", sample.readings[i]);
                    break;
                }
            }
            printf("\n");
        }
    }
    goto instrumentCleanup;

instrumentCleanup:
    RBRInstrumentGen4_close(instrument);

fileCleanup:
    close(instrumentFd);

    return status;
}
