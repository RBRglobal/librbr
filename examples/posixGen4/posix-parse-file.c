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
/* Required for EXIT_SUCCESS, etc. */
#include <stdlib.h>
/* Required for strerror. */
#include <string.h>
/* Required for gmtime_r, nanosleep, time_t, strftime. */
#include <time.h>
/* Required for close. */
#include <unistd.h>

#include "posix-shared.h"
#include "RBRParserGen4.h"

RBRInstrumentGen4Error parserSample(
    const struct RBRParserGen4 *parser,
    const struct RBRInstrumentGen4Sample *const sample)
{
    /* Unused. */
    (void) parser;

    char ftime[128];
    time_t sampleSeconds = (time_t) (sample->timestamp / 1000);
    struct tm sampleTime;
    gmtime_r(&sampleSeconds, &sampleTime);
    strftime(ftime, sizeof(ftime), "%F %T", &sampleTime);

    printf("%s.%03" PRIi64, ftime, sample->timestamp % 1000);
    for (int32_t i = 0; i < sample->channelCount; i++)
    {
        printf(", %lf", sample->readings[i]);
    }
    printf("\n");

    return RBRINSTRUMENTGEN4_SUCCESS;
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
            RBRINSTRUMENTGEN4_LIB_NAME,
            RBRINSTRUMENTGEN4_LIB_VERSION,
            RBRINSTRUMENTGEN4_LIB_BUILD_DATE);

    RBRParserGen4 *parser = NULL;
    #ifdef RBR_LIB_NODYNAMICMEMORYALLOCATION
    RBRParserGen4 parserSpace;
    parser = &parserSpace;
    #endif

    RBRInstrumentGen4Sample sampleBuffer;
    RBRParserGen4Callbacks parserCallbacks = {
        .sample = parserSample,
        .sampleBuffer = &sampleBuffer
    };

    RBRParserGen4Config parserConfig = {
        .channelCount = channels,
        .datatype = RBRINSTRUMENTGEN4_DATATYPE_FLOAT32
    };

    RBRInstrumentGen4Error err;
    if ((err = RBRParserGen4_init(
             &parser,
             &parserCallbacks,
             &parserConfig,
             NULL)) != RBRINSTRUMENTGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to initialize parser: %s!\n",
                programName,
                RBRInstrumentGen4Error_name(err));
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
        RBRParserGen4_parse(parser,
                        RBRINSTRUMENTGEN4_BLOCK_DATA,
                        buf,
                        &parsedSize);
        bufSize -= parsedSize;
        memmove(buf, buf + parsedSize, bufSize);
    }

    RBRParserGen4_destroy(parser);
fileCleanup:
    close(datasetFd);

    return status;
}
