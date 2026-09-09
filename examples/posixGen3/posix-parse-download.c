/**
 * \file posix-parse-download.c
 *
 * \brief Example of using the library to continuously download and parse
 * instrument data in a POSIX environment.
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
#include "RBRParserGen3.h"

#define CHUNK_SIZE 1024

RBRGen3Error parserSample(
    const struct RBRParserGen3 *parser,
    const struct RBRGen3Sample *const sample)
{
    (void) parser;

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

    return RBRGEN3_SUCCESS;
}

int main(int argc, char *argv[])
{
    char *programName = argv[0];
    char *devicePath;

    int status = EXIT_SUCCESS;
    int instrumentFd;

    RBRGen3Error err;
    RBRGen3 *instrument = NULL;
    #ifdef RBR_LIB_NODYNAMICMEMORYALLOCATION
    RBRGen3 instrumentSpace;
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
            "%s: Using %s v%s.\n",
            programName,
            RBRGEN3_LIB_NAME,
            RBRGEN3_LIB_VERSION);

    RBRGen3Callbacks instrumentCallbacks = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite
    };

    if ((err = RBRGen3_open(
             &instrument,
             &instrumentCallbacks,
             INSTRUMENT_COMMAND_TIMEOUT_MSEC,
             (void *) &instrumentFd)) != RBRGEN3_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to establish instrument connection: %s!\n",
                programName,
                RBRGen3Error_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    RBRInstrumentGen3_setUSBStreamingState(instrument, false);
    RBRInstrumentGen3_setSerialStreamingState(instrument, false);

    if ((err = instrumentStart(instrument)) != RBRGEN3_SUCCESS)
    {
        fprintf(stderr,
                "%s: Failed to start instrument: %s!\n",
                programName,
                RBRGen3Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    RBRGen3Channels channels;
    RBRGen3_getChannels(instrument, &channels);

    RBRParserGen3 *parser = NULL;

    RBRGen3Sample sampleBuffer;
    RBRParserGen3Callbacks parserCallbacks = {
        .sample = parserSample,
        .sampleBuffer = &sampleBuffer
    };

    RBRParserGen3Config parserConfig = {
        .format = RBRINSTRUMENTGEN3_MEMFORMAT_CALBIN00,
        .formatConfig = {
            .easyParse = {
                .channels = channels.on
            }
        }
    };

    if ((err = RBRParserGen3_init(
             &parser,
             &parserCallbacks,
             &parserConfig,
             NULL)) != RBRGEN3_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to initialize parser: %s!\n",
                programName,
                RBRGen3Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    uint8_t buf[CHUNK_SIZE];
    int32_t bufSize = 0;
    RBRInstrumentGen3Data data = {
        .dataset = RBRINSTRUMENTGEN3_DATASET_EASYPARSE_SAMPLE_DATA,
        .offset  = 0
    };
    int32_t parsedSize;

    while (true)
    {
        data.data = buf + bufSize;
        data.size = sizeof(buf) - bufSize;
        err = RBRInstrumentGen3_readData(instrument, &data);
        if (err == RBRGEN3_TIMEOUT)
        {
            printf("\nWarning: timeout. Retrying...\n");
            continue;
        }
        else if (err != RBRGEN3_SUCCESS)
        {
            printf("\nError: %s", RBRGen3Error_name(err));
            break;
        }

        data.offset += data.size;

        bufSize += data.size;
        parsedSize = bufSize;
        RBRParserGen3_parse(parser,
                        RBRINSTRUMENTGEN3_DATASET_EASYPARSE_SAMPLE_DATA,
                        buf,
                        &parsedSize);
        bufSize -= parsedSize;
        memmove(buf, buf + parsedSize, bufSize);

        /* We don't need to constantly hammer the instrument with download
         * requests. We'll wait just a little bit between download attempts. */
        struct timespec sleep = {
            .tv_sec  = 0,
            .tv_nsec = 32000000LL
        };
        nanosleep(&sleep, NULL);
    }
instrumentCleanup:
    RBRGen3_close(instrument);
fileCleanup:
    close(instrumentFd);

    return status;
}
