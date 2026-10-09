/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file posix-parse-download.c
 *
 * \brief Example of using the library to continuously download and parse
 * instrument data in a POSIX environment.
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
#include "RBRGen3Parser.h"
#include "RBRGen3Commands.h"

/* Readings storage for as many channels as this application expects. */
#define CHANNEL_MAX 32

#define CHUNK_SIZE 1024

RBRGen3Error parserSample(const RBRGen3Parser *parser, const RBRGen3Sample *const sample)
{
    (void) parser;

    char ftime[128];
    time_t sampleSeconds = (time_t) (sample->timestamp / 1000);
    struct tm sampleTime;
    gmtime_r(&sampleSeconds, &sampleTime);
    strftime(ftime, sizeof(ftime), "%F %T", &sampleTime);

    printf("%s.%03" PRIi64, ftime, sample->timestamp % 1000);
    for (int32_t i = 0; i < sample->channelCount; i++) {
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

    RBRGen3Environment instrumentEnvironment = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite,
        .command = commandBuffer,
        .commandCapacity = sizeof(commandBuffer),
        .response = responseBuffer,
        .responseCapacity = sizeof(responseBuffer),
    };

    if ((err = RBRGen3_open(&conn,
                            &instrumentEnvironment,
                            INSTRUMENT_COMMAND_TIMEOUT_MSEC,
                            (void *) &instrumentFd)) != RBRGEN3_SUCCESS) {
        fprintf(stderr,
                "%s: Failed to establish instrument connection: %s!\n",
                programName,
                RBRGen3Error_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    RBRGen3_setUSBStreamingState(&conn, false);
    RBRGen3_setSerialStreamingState(&conn, false);

    if ((err = instrumentStart(&conn)) != RBRGEN3_SUCCESS) {
        fprintf(
            stderr, "%s: Failed to start instrument: %s!\n", programName, RBRGen3Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    /* The parser needs to know how many channels each sample holds. */
    int32_t enabledChannels = 0;
    RBRGen3_getEnabledChannelCount(&conn, &enabledChannels);

    RBRGen3Parser parser;

    double sampleReadings[CHANNEL_MAX];
    RBRGen3Sample sampleBuffer = {.size = CHANNEL_MAX, .readings = sampleReadings};
    RBRGen3ParserCallbacks parserCallbacks = {
        .sample = parserSample,
        .sampleBuffer = &sampleBuffer,
    };

    RBRGen3ParserConfig parserConfig = {
        .format = RBRGEN3_MEMFORMAT_CALBIN00,
        .formatConfig =
            {
                .easyParse =
                    {
                        .channels = enabledChannels,
                    },
            },
    };

    if ((err = RBRGen3Parser_init(&parser, &parserCallbacks, &parserConfig, NULL)) !=
        RBRGEN3_SUCCESS) {
        fprintf(
            stderr, "%s: Failed to initialize parser: %s!\n", programName, RBRGen3Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    uint8_t buf[CHUNK_SIZE];
    int32_t bufSize = 0;
    RBRGen3Data data = {.dataset = RBRGEN3_DATASET_EASYPARSE_SAMPLE_DATA, .offset = 0};
    int32_t parsedSize;

    while (true) {
        data.data = buf + bufSize;
        data.size = sizeof(buf) - bufSize;
        err = RBRGen3_readData(&conn, &data);
        if (err == RBRGEN3_TIMEOUT) {
            printf("\nWarning: timeout. Retrying...\n");
            continue;
        } else if (err != RBRGEN3_SUCCESS) {
            printf("\nError: %s", RBRGen3Error_name(err));
            break;
        }

        data.offset += data.size;

        bufSize += data.size;
        parsedSize = bufSize;
        RBRGen3Parser_parse(&parser, RBRGEN3_DATASET_EASYPARSE_SAMPLE_DATA, buf, &parsedSize);
        bufSize -= parsedSize;
        memmove(buf, buf + parsedSize, bufSize);

        /* We don't need to constantly hammer the instrument with download
         * requests. We'll wait just a little bit between download attempts. */
        struct timespec sleep = {
            .tv_sec = 0,
            .tv_nsec = 32000000LL,
        };
        nanosleep(&sleep, NULL);
    }
instrumentCleanup:
    RBRGen3_close(&conn);
fileCleanup:
    close(instrumentFd);

    return status;
}
