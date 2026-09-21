/**
 * \file posix-parse-download-dataset.c
 *
 * \brief Example of using the library to download and parse
 * instrument data from dataset1 or dataset4 in a POSIX environment.
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
/* Required for EXIT_FAILURE, EXIT_SUCCESS, strtol. */
#include <stdlib.h>
/* Required for strerror. */
#include <string.h>
/* Required for gmtime_r, time_t, strftime. */
#include <time.h>
/* Required for close. */
#include <unistd.h>

#include "posix-shared.h"
#include "RBRGen3Parser.h"

RBRGen3Error parserSample(const struct RBRGen3Parser *parser,
                          const struct RBRGen3Sample *const sample)
{
    (void) parser;

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
    int _downloadFrom;

    int status = EXIT_SUCCESS;
    int instrumentFd;

    RBRGen3Error err;
    RBRGen3 conn;

    if (argc < 3) {
        fprintf(stderr, "command incomplete: %s\r\n", argv[0]);
        printf("usage: <path>/posix-parse-download-dataset <terminal> <dataset number>\r\n");
        printf("       %s\r\n", "e.g: ./posix-parse-download-dataset /dev/ttyS5 1");
        return EXIT_FAILURE;
    }

    devicePath = argv[1];
    if ((instrumentFd = openSerialFd(devicePath)) < 0) {
        fprintf(stderr, "%s: Failed to open serial device: %s!\n", programName, strerror(errno));
        return EXIT_FAILURE;
    }

    _downloadFrom = strtol(argv[2], NULL, 10);
    if (_downloadFrom != 1 && _downloadFrom != 4) {
        fprintf(stderr, "only dataset1 or dataset4 can be downloaded!");
        return EXIT_FAILURE;
    }
    if (_downloadFrom == 4) {
        fprintf(stderr,
                "Warning: this code requires 5 channels' output. Please modify \"channels.count\" "
                "and \"channels.on\" to match your settings.\n");
    }

    fprintf(stderr, "%s: Using %s v%s.\n", programName, RBRGEN3_LIB_NAME, RBRGEN3_LIB_VERSION);

    RBRGen3Callbacks instrumentCallbacks = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite,
    };

    if ((err = RBRGen3_open(&conn,
                            &instrumentCallbacks,
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

    RBRGen3Channels channels;
    if (_downloadFrom == 1) {
        RBRGen3_getChannels(&conn, &channels);
    } else if (_downloadFrom == 4) {
        // important!!!
        // channels should be set the same number with output channels.
        channels.count = 5;
        channels.on = 5;
        channels.settlingTime = 60;
        channels.readTime = 290;
        channels.minimumPeriod = 480;
    }

    RBRGen3Parser parser;

    RBRGen3Sample sampleBuffer;
    RBRGen3ParserCallbacks parserCallbacks = {
        .sample = parserSample,
        .sampleBuffer = &sampleBuffer,
    };

    RBRGen3ParserConfig parserConfig = {.format = RBRGEN3_MEMFORMAT_CALBIN00,
                                        .formatConfig = {.easyParse = {
                                                             .channels = channels.on,
                                                         }}};

    if ((err = RBRGen3Parser_init(&parser, &parserCallbacks, &parserConfig, NULL)) !=
        RBRGEN3_SUCCESS) {
        fprintf(
            stderr, "%s: Failed to initialize parser: %s!\n", programName, RBRGen3Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    uint8_t buf[1024];
    int32_t bufSize = 0;
    RBRGen3Dataset _numOfDataset = _downloadFrom;
    RBRGen3Data data = {.dataset = _numOfDataset, .offset = 0};
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
