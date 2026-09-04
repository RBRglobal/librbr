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
(const RBRInstrumentGen4Label[]) \
{   \
    PRESSURE, \
        TEMPERATURE, \
        SALINITY_DYNCORR \
}


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
           RBRInstrumentGen4LinkType_name(link.type));

    RBRInstrumentGen4LinkSerial serial;

    switch (link.type)
    {
    case RBRINSTRUMENTGEN4_LINK_TYPE_USB:
        break;
    case RBRINSTRUMENTGEN4_LINK_TYPE_SERIAL:
    {
        RBRInstrumentGen4_getLinkSerial(instrument, &serial);
        printf("Connected in %s mode at %s baud.\n",
               RBRInstrumentGen4LinkSerialMode_name(serial.mode),
               RBRInstrumentGen4LinkSerialBaudRate_name(serial.baudRate));
        break;
    }
    default:
        fprintf(stderr,
                "Warning: connection method to the instrument is unclear, so"
                " poll can't be executed.\n");
        goto instrumentCleanup;
    }

    // read the channel pool
    RBRInstrumentGen4Channel channelBuf[RBRINSTRUMENTGEN4_CHANNEL_MAX];
    RBRInstrumentGen4ChannelPool channelPool = {
        .size = RBRINSTRUMENTGEN4_CHANNEL_MAX,
        .pool = channelBuf
    };
    RBRInstrumentGen4_getChannelPool(instrument, &channelPool);

    // Only the first group is polled, so only one label is kept; the pool
    // reports that the rest were discarded, which is expected here.
    RBRInstrumentGen4Group groupBuf[1];
    RBRInstrumentGen4GroupPool groupPool = {
        .size = 1,
        .pool = groupBuf
    };
    err = RBRInstrumentGen4_getGroupPool(instrument, &groupPool);
    if (err != RBRINSTRUMENTGEN4_SUCCESS
        && err != RBRINSTRUMENTGEN4_TRUNCATED)
    {
        fprintf(stderr,
                "%s: Failed to read the group pool: %s!\n",
                programName,
                RBRInstrumentGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    // specify outputformat. The setter sends every parameter of the command,
    // so read the current format and change only the sample fields.
    RBRInstrumentGen4OutputFormat outputformat;
    RBRInstrumentGen4_getOutputFormat(instrument, &outputformat);
    outputformat.sn = true;
    outputformat.scheduleLabel = true;
    outputformat.dateTime = true;
    outputformat.crc = true;
    RBRInstrumentGen4_setOutputFormat(instrument, &outputformat);

    // poll data and print in console
    RBRInstrumentGen4Sample sample;
    while (true)
    {
        // poll one group
        err = RBRInstrumentGen4_pollGroups(instrument,
                                           groupPool.pool[0].label,
                                           &sample);
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
