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
#include "RBRParserGen3.h"

RBRGen3Error parserSample(
    const struct RBRParserGen3 *parser,
    const struct RBRGen3Sample *const sample)
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

    return RBRGEN3_SUCCESS;
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
            "%s: Using %s v%s.\n",
            programName,
            RBRGEN3_LIB_NAME,
            RBRGEN3_LIB_VERSION);

    RBRParserGen3 *parser = NULL;
    #ifdef RBR_LIB_NODYNAMICMEMORYALLOCATION
    RBRParserGen3 parserSpace;
    parser = &parserSpace;
    #endif

    RBRGen3Sample sampleBuffer;
    RBRParserGen3Callbacks parserCallbacks = {
        .sample = parserSample,
        .sampleBuffer = &sampleBuffer
    };

    RBRParserGen3Config parserConfig = {
        .format = RBRGEN3_MEMFORMAT_CALBIN00,
        .formatConfig = {
            .easyParse = {
                .channels = channels
            }
        }
    };

    RBRGen3Error err;
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
        RBRParserGen3_parse(parser,
                        RBRGEN3_DATASET_EASYPARSE_SAMPLE_DATA,
                        buf,
                        &parsedSize);
        bufSize -= parsedSize;
        memmove(buf, buf + parsedSize, bufSize);
    }

    RBRParserGen3_destroy(parser);
fileCleanup:
    close(datasetFd);

    return status;
}
