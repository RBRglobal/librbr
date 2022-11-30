/**
 * \file posix-parse-file.c
 *
 * \brief Example of using the library to parse the contents of instrument data
 * from a file in a POSIX environment.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Prerequisite for gmtime_r in time.h. */
#define _POSIX_C_SOURCE 200112L

/* Required for errno. */
#include <errno.h>
/* Required for open. */
#include <fcntl.h>
/* Required for open. */
#include <sys/stat.h>
/* Required for fprintf, printf. */
#include <stdio.h>
/* Required for strerror. */
#include <string.h>
/* Required for gmtime_r, nanosleep, time_t, strftime. */
#include <time.h>
/* Required for close. */
#include <unistd.h>

#include "posix-shared.h"
#include "RBRParser.h"

RBRInstrumentError parserSample(
    const struct RBRParser *parser,
    const struct RBRInstrumentSample *const sample)
{
    /* Unused. */
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

    return RBRINSTRUMENT_SUCCESS;
}

int main(int argc, char *argv[])
{
    char *programName = argv[0];
    char *filePath;
    int32_t channels;

    int status = EXIT_SUCCESS;
    int datasetFd;

    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s file channels\n", argv[0]);
        return EXIT_FAILURE;
    }

    filePath = argv[1];
    channels = strtol(argv[2], NULL, 10);

    if ((datasetFd = open(filePath, O_RDONLY)) < 0)
    {
        fprintf(stderr, "%s: Failed to open file: %s!\n",
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

    RBRParser *parser = NULL;
    #ifdef RBR_LIB_NODYNAMICMEMORYALLOCATION
    RBRParser parserSpace;
    parser = &parserSpace;
    #endif

    RBRInstrumentSample sampleBuffer;
    RBRParserCallbacks parserCallbacks = {
        .sample = parserSample,
        .sampleBuffer = &sampleBuffer
    };

    RBRParserConfig parserConfig = {
        .format = RBRINSTRUMENT_MEMFORMAT_CALBIN00,
        .formatConfig = {
            .easyParse = {
                .channels = channels
            }
        }
    };

    RBRInstrumentError err;
    if ((err = RBRParser_init(
             &parser,
             &parserCallbacks,
             &parserConfig,
             NULL)) != RBRINSTRUMENT_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to initialize parser: %s!\n",
                programName,
                RBRInstrumentError_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    uint8_t buf[1024];
    int32_t bufSize = 0;
    int32_t readSize;
    int32_t parsedSize;

    while (true)
    {
        readSize = read(datasetFd, buf + bufSize, sizeof(buf) - bufSize);
        if (readSize < 0 && errno == EAGAIN)
        {
            fprintf(stderr, "\nRetrying...\n");
            continue;
        }
        else if (readSize < 0)
        {
            printf("\nError: %s", strerror(errno));
            break;
        }
        else if (readSize == 0)
        {
            break;
        }

        bufSize += readSize;
        parsedSize = bufSize;
        RBRParser_parse(parser,
                        RBRINSTRUMENT_DATASET_EASYPARSE_SAMPLE_DATA,
                        buf,
                        &parsedSize);
        bufSize -= parsedSize;
        memmove(buf, buf + parsedSize, bufSize);
    }

    RBRParser_destroy(parser);
fileCleanup:
    close(datasetFd);

    return status;
}
