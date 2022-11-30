/**
 * \file posix-download.c
 *
 * \brief Example of using the library to download instrument data in a POSIX
 * environment.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Prerequisite for PATH_MAX in limits.h. */
#define _POSIX_C_SOURCE 200112L

/* Required for errno. */
#include <errno.h>
/* Required for open. */
#include <fcntl.h>
/* Required for PATH_MAX. */
#include <limits.h>
/* Required for fprintf, printf, snprintf. */
#include <stdio.h>
/* Required for strerror. */
#include <string.h>
/* Required for open. */
#include <sys/stat.h>
/* Required for clock_gettime. */
#include <time.h>
/* Required for close, write. */
#include <unistd.h>

#include "posix-shared.h"

#define CHUNK_SIZE 4096

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
    printf(
        "Looks like I'm connected to a %s instrument.\n",
        RBRInstrumentGeneration_name(RBRInstrument_getGeneration(instrument)));

    RBRInstrumentId id;
    RBRInstrument_getId(instrument, &id);
    printf("The instrument is an %s (fwtype %d), serial number %06d, with "
           "firmware v%s.\n",
           id.model,
           id.fwtype,
           id.serial,
           id.version);

    RBRInstrumentHardwareRevision hwrev;
    RBRInstrument_getHardwareRevision(instrument, &hwrev);
    printf("It's PCB rev%c, CPU rev%s, BSL v%c.\n",
           hwrev.pcb,
           hwrev.cpu,
           hwrev.bsl);

    RBRInstrumentMemoryInfo meminfo;
    meminfo.dataset = RBRINSTRUMENT_DATASET_STANDARD;
    RBRInstrument_getMemoryInfo(instrument, &meminfo);
    printf("Dataset %s is %0.2f%% full (%" PRIi32 "B used).\n",
           RBRInstrumentDataset_name(meminfo.dataset),
           ((float) meminfo.used) / meminfo.size * 100,
           meminfo.used);

    RBRInstrumentMemoryFormat memformat;
    RBRInstrument_getAvailableMemoryFormats(instrument, &memformat);
    printf("It supports these memory formats:\n");
    for (int i = RBRINSTRUMENT_MEMFORMAT_NONE + 1;
         i <= RBRINSTRUMENT_MEMFORMAT_MAX;
         i <<= 1)
    {
        if (memformat & i)
        {
            printf("\t%s\n", RBRInstrumentMemoryFormat_name(i));
        }
    }

    RBRInstrument_getCurrentMemoryFormat(instrument, &memformat);
    printf("It's currently storing data of format %s.\n",
           RBRInstrumentMemoryFormat_name(memformat));

    char filename[PATH_MAX + 1];
    snprintf(filename, sizeof(filename), "%06d.bin", id.serial);
    int downloadFd;
    if ((downloadFd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644)) < 0)
    {
        fprintf(stderr, "%s: Failed to open output file: %s!\n",
                programName,
                strerror(errno));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    struct stat stat;
    if (fstat(downloadFd, &stat) < 0)
    {
        fprintf(stderr, "%s: Failed to stat output file: %s!\n",
                programName,
                strerror(errno));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }
    int32_t initialOffset = stat.st_size;

    if (initialOffset == 0)
    {
        printf("It looks like the output file, %s, is new. Downloading from "
               "the beginning of instrument memory.\n",
               filename);
    }
    else
    {
        printf("It looks like the output file, %s, already contains %" PRIi32
               "B. I'll resume the instrument download from there.\n",
               filename,
               initialOffset);
    }

    uint8_t buf[CHUNK_SIZE];
    RBRInstrumentData data = {
        .dataset = meminfo.dataset,
        .offset  = initialOffset,
        .data    = buf
    };

    printf("Downloading:\n");

    struct timespec start;
    struct timespec now;
    double elapsed = 0.0;
    double rate = 0.0;
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (data.offset < meminfo.used)
    {
        data.size = sizeof(buf);
        err = RBRInstrument_readData(instrument, &data);
        if (err == RBRINSTRUMENT_SUCCESS)
        {
            write(downloadFd, data.data, data.size);
            data.offset += data.size;
        }
        else if (err == RBRINSTRUMENT_TIMEOUT)
        {
            printf("\nWarning: timeout. Retrying...\n");
        }
        else
        {
            printf("\nError: %s", RBRInstrumentError_name(err));
            break;
        }

        clock_gettime(CLOCK_MONOTONIC, &now);

        elapsed  = now.tv_sec - start.tv_sec;
        elapsed *= 1000000000L;
        elapsed += now.tv_nsec - start.tv_nsec;
        elapsed /= 1000000000L;

        rate = elapsed > 0.0 ? (data.offset - initialOffset) / elapsed : 0.0;

        printf("\r%0.2f%% (%" PRIi32 "B/%" PRIi32 "B; %0.3fs elapsed; "
               "%0.3fB/s)",
               (((float) data.offset) / meminfo.used) * 100,
               data.offset,
               meminfo.used,
               elapsed,
               rate);
    }

    printf("\nDone. Downloaded %" PRIi32 "B in %0.3fs (%0.3fB/s).\n",
           data.offset,
           elapsed,
           rate);

fileCleanup:
    close(downloadFd);
instrumentCleanup:
    RBRInstrument_close(instrument);
serialCleanup:
    close(instrumentFd);

    return status;
}
