/**
 * \file posix-poll.c
 *
 * \brief Example of using the library to poll an instrument for on-demand
 * samples.
 *
 * Polls every channel, then just the pressure and temperature channels.
 * Nothing is written to the instrument, and polling works whether or not a
 * deployment is enabled.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for errno. */
#include <errno.h>
/* Required for PRId32, PRId64. */
#include <inttypes.h>
/* Required for fprintf, printf. */
#include <stdio.h>
/* Required for EXIT_SUCCESS, etc. */
#include <stdlib.h>
/* Required for strerror. */
#include <string.h>
/* Required for close. */
#include <unistd.h>

#include "RBRGen4.h"
#include "posix-shared.h"

/* == Customer defined parameters == */

/* Tune this value to your instrument. This example is intended to work with
 * any instrument that has pressure and temperature channels, so a large
 * channel count is used to be as compatible as possible off-the-shelf. */
#define CHANNEL_COUNT 32

#define PRESSURE    "pressure_00"
#define TEMPERATURE "temperature_00"

/* The channels to poll by list. */
#define POLL_PT_CHANNELS      \
    (RBRGen4Label[])          \
    {                         \
        PRESSURE, TEMPERATURE \
    }
#define POLL_PT_CHANNEL_COUNT 2

/* How many times to poll each way. */
#define POLL_COUNT 5

const char *programName = "";

/* Report a failed library call. A hardware error also carries the
 * instrument's own message, which says what it objected to. */
void logCmdError(const RBRGen4 *conn, RBRGen4Error err, const char *msg)
{
    fprintf(stderr, "%s: %s (%s)\n", programName, msg, RBRGen4Error_name(err));
    if (err == RBRGEN4_HARDWARE_ERROR) {
        fprintf(stderr,
                "%s: Instrument reported: %s\n",
                programName,
                RBRGen4_getLastHardwareErrorMessage(conn));
    }
}

/* Print a sample's timestamp and readings on one line. Error readings are
 * NaNs carrying an error code; report the code instead of the value. */
void printSample(const RBRGen4Sample *sample)
{
    printf("%" PRId64, sample->timestamp);
    for (int32_t i = 0; i < sample->channelCount; i++) {
        if (RBRGen4Reading_isError(sample->readings[i])) {
            printf(", error %d", RBRGen4Reading_getError(sample->readings[i]));
        } else {
            printf(", %f", sample->readings[i]);
        }
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    programName = argv[0];

    if (argc < 2) {
        fprintf(stderr, "Usage: %s device\n", programName);
        return EXIT_FAILURE;
    }

    int instrumentFd;
    char *devicePath = argv[1];

    if ((instrumentFd = openSerialFd(devicePath)) < 0) {
        fprintf(stderr, "%s: Failed to open serial device: %s!\n", programName, strerror(errno));
        return EXIT_FAILURE;
    }

    fprintf(stderr,
            "%s: Using %s v%s (built %s).\n",
            programName,
            RBRGEN4_LIB_NAME,
            RBRGEN4_LIB_VERSION,
            RBRGEN4_LIB_BUILD_DATE);

    RBRGen4Error err = RBRGEN4_SUCCESS;
    RBRGen4 conn;
    const RBRGen4Callbacks callbacks = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite,
    };

    err = RBRGen4_open(&conn, &callbacks, INSTRUMENT_COMMAND_TIMEOUT_MSEC, (void *) &instrumentFd);
    if (err) {
        logCmdError(&conn, err, "Failed to establish instrument connection");
        goto fileCleanup;
    }

    /* Polled samples are only distinguishable from streamed samples when the
     * output format includes the schedule label (polled samples are labelled
     * `polling`). Read the format rather than change it, and only insist on
     * the label when the instrument reports it. */
    RBRGen4OutputFormat outputFormat;
    err = RBRGen4_getOutputFormat(&conn, &outputFormat);
    if (err) {
        logCmdError(&conn, err, "Failed to get output format");
        goto instrumentCleanup;
    }
    const bool requireLabel = outputFormat.scheduleLabel;
    if (!requireLabel) {
        printf("%s: Warning: schedule labels are off, so a streamed sample"
               " may be returned in place of a polled one.\n",
               programName);
    }

    /* Read the channel pool so the readings can be labelled */
    RBRGen4Channel channelPoolBuf[CHANNEL_COUNT];
    RBRGen4ChannelPool channelPool = {
        .size = CHANNEL_COUNT,
        .pool = channelPoolBuf,
    };
    err = RBRGen4_getChannelPool(&conn, &channelPool);
    if (err == RBRGEN4_TRUNCATED) {
        int32_t count;
        err = RBRGen4_getChannelCount(&conn, &count);
        if (err != RBRGEN4_SUCCESS) {
            logCmdError(&conn, err, "Failed to get channel count");
            goto instrumentCleanup;
        }

        printf("%s: Warning: not enough space in channel pool to store all"
               " channels; only the first %" PRId32 " are stored out of the"
               " instrument's %" PRId32 " channels\n",
               programName,
               channelPool.len,
               count);
    } else if (err) {
        logCmdError(&conn, err, "Failed to get channel pool");
        goto instrumentCleanup;
    }

    /* Poll every channel. Readings are reported in channel pool order. */
    printf("timestamp");
    for (int32_t i = 0; i < channelPool.len; i++) {
        printf(", %s", channelPool.pool[i].label);
    }
    printf("\n");

    RBRGen4Sample sample;
    for (int32_t i = 0; i < POLL_COUNT; i++) {
        err = RBRGen4_poll(&conn, requireLabel, &sample);
        if (err) {
            logCmdError(&conn, err, "Failed to poll");
            goto instrumentCleanup;
        }
        printSample(&sample);
    }

    /* Poll only the pressure and temperature channels. Readings are reported
     * in the order requested. */
    printf("timestamp, %s, %s\n", PRESSURE, TEMPERATURE);
    const RBRGen4LabelList pollPtChannelList = {
        .size = POLL_PT_CHANNEL_COUNT,
        .len = POLL_PT_CHANNEL_COUNT,
        .labels = POLL_PT_CHANNELS,
    };
    for (int32_t i = 0; i < POLL_COUNT; i++) {
        err = RBRGen4_pollChannels(&conn, requireLabel, &pollPtChannelList, &sample);
        if (err) {
            logCmdError(&conn, err, "Failed to poll channels");
            goto instrumentCleanup;
        }
        printSample(&sample);
    }

instrumentCleanup:
    RBRGen4_close(&conn);

fileCleanup:
    close(instrumentFd);

    return (err == RBRGEN4_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
}
