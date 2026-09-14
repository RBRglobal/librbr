/**
 * \file posix-download-datablock-bySchedule.c
 *
 * \brief Example of using the library to download instrument data from specified dataset
 *  for a specific schedule in a POSIX environment.
 *
 * This example is supposed to run after using posix-multiScheduleDiffConfig example.
 * Each download would generate a new datafile.
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
/* Required for EXIT_SUCCESS, etc. */
#include <stdlib.h>
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

//********************************* customer defined ************************************//
#define DATASET_LABEL "ds_ascent"
#define SCHEDULE_LABEL "sch_asc_pts"

void getCurrentTimestamp(char currentTimestamp[])
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(currentTimestamp, 16, "%Y%m%d_%H%M%S", t);
}

int main(int argc, char *argv[])
{
    // check all arguments are provided
    char *programName = argv[0];
    char *devicePath;

    int status = EXIT_SUCCESS;
    int instrumentFd;

    RBRGen4Error err;
    RBRGen4 instrumentSpace;
    RBRGen4 *instrument = &instrumentSpace;

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
            RBRGEN4_LIB_NAME,
            RBRGEN4_LIB_VERSION,
            RBRGEN4_LIB_BUILD_DATE);

    // check instrument communication is fine.
    RBRGen4Callbacks callbacks = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite
    };

    if ((err = RBRGen4_open(
             &instrument,
             &callbacks,
             INSTRUMENT_COMMAND_TIMEOUT_MSEC,
             (void *) &instrumentFd)) != RBRGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to establish instrument connection: %s!\n", programName, RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup; // Failure case, memory allocated by this constructor is freed.
    }

    //******************seems unnecessary********************//
    RBRGen4Id4 id;
    RBRGen4_getId4(instrument, &id);
    printf("The instrument is an %s (fwtype %d), serial number %06d, with "
           "firmware v%s.\n",
           id.model,
           id.fwtype,
           id.sn,
           id.fwversion);

    // create a file to store downloaded data
    char filename[PATH_MAX + 1];
    char currentTimestamp[16];
    getCurrentTimestamp(currentTimestamp);
    snprintf(filename, sizeof(filename), "%06d_%s.bin", id.sn, currentTimestamp); // specify the file name. e.g. 999999_20231023_143711.bin
    int downloadFd;
    if ((downloadFd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644)) < 0)
    {
        fprintf(stderr, "%s: Failed to open output file: %s!\n", programName, strerror(errno));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    struct stat stat;
    if (fstat(downloadFd, &stat) < 0)
    {
        fprintf(stderr, "%s: Failed to stat output file: %s!\n", programName, strerror(errno));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }
    printf("The output file is %s. Downloading from "
           "the beginning of instrument memory.\n",
           filename);

    //*******************download data from instrument. Support only bytecount. *******************//
    // quit if there's no dataset available.
    RBRInstrumentGen4Dataset datasetBuf[RBRINSTRUMENTGEN4_DATASET_COUNT_MAX];
    RBRInstrumentGen4DatasetPool datasetPool = {
        .size = RBRINSTRUMENTGEN4_DATASET_COUNT_MAX,
        .pool = datasetBuf
    };
    err = RBRInstrumentGen4_getDatasetPool(instrument, &datasetPool);
    if (err != RBRGEN4_SUCCESS || datasetPool.count <= 0)
    {
        printf("Error: There's no dataset available in this instrument. Quit.\n");
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    // Need dataset struct instance to find out the bytecount.
    RBRInstrumentGen4Dataset *targetDataset;
    if (RBRInstrumentGen4_getDatasetFromPool(&targetDataset,
                                             &datasetPool,
                                             DATASET_LABEL)
        != RBRGEN4_SUCCESS)
    {
        printf("Error: Dataset %s not found. Quit.\n", DATASET_LABEL);
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    // Read the dataset's parameters, including the schedules it ran.
    RBRGen4Label
        scheduleLabelBuf[RBRGEN4_SCHEDULE_COUNT_MAX];
    RBRGen4LabelList scheduleList = {
        .size = RBRGEN4_SCHEDULE_COUNT_MAX,
        .labels = scheduleLabelBuf
    };
    err = RBRInstrumentGen4_getDataset(instrument,
                                       targetDataset,
                                       &scheduleList);
    if (err != RBRGEN4_SUCCESS)
    {
        printf("Error: Failed to read dataset %s: %s. Quit.\n",
               DATASET_LABEL,
               RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    printf("Dataset %s contains data from following schedules: ", targetDataset->label);
    for (int32_t i = 0; i < scheduleList.count && i < scheduleList.size; i++)
    {
        printf("%s ", scheduleList.labels[i]);
    }
    printf("\n");

    // Get the data block info of specified dataset and schedule.
    // This will fail if the target schedule is not in the target dataset.
    RBRInstrumentGen4DatasetDataBlock dataBlock;
    err = RBRInstrumentGen4Dataset_getScheduleDataBlock(instrument,
                                                        targetDataset,
                                                        SCHEDULE_LABEL,
                                                        &dataBlock);
    if (err != RBRGEN4_SUCCESS)
    {
        printf("Error: Failed to read schedule %s data block: %s. Quit.\n",
               SCHEDULE_LABEL,
               RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }
    printf("Dataset %s schedule %s data contains: "
           "%" PRIi64 "B data, "
           "%" PRIi64 " samples. "
           "Data format is %s.\n",
           targetDataset->label,
           SCHEDULE_LABEL,
           dataBlock.byteCount,
           dataBlock.sampleCount,
           RBRGen4DataType_name(targetDataset->dataType));

    uint8_t buf[CHUNK_SIZE];
    RBRInstrumentGen4DownloadData download = {
        .unit = RBRINSTRUMENTGEN4_DOWNLOAD_DATA_UNIT_BYTES,
        .start = 0,
        .data = buf,
        .dataSize = sizeof(buf)
    };

    printf("Downloading:\n");

    struct timespec start;
    struct timespec now;
    double elapsed = 0.0;
    double rate = 0.0;
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (download.start < dataBlock.byteCount) // if not downloaded all data from targetDataset/schedule/datablock
    {
        download.count = sizeof(buf); // specify download bytes
        err = RBRInstrumentGen4Dataset_downloadScheduleData(instrument,
                                                            targetDataset,
                                                            SCHEDULE_LABEL,
                                                            &download);
        if (err == RBRGEN4_SUCCESS)
        {
            write(downloadFd, download.data, download.count);
            download.start += download.count;
        }
        else if (err == RBRGEN4_TIMEOUT)
        {
            printf("\nWarning: timeout. Retrying...\n");
        }
        else
        {
            printf("\nError: %s", RBRGen4Error_name(err));
            break;
        }

        clock_gettime(CLOCK_MONOTONIC, &now);

        elapsed = now.tv_sec - start.tv_sec;
        elapsed *= 1000000000L;
        elapsed += now.tv_nsec - start.tv_nsec;
        elapsed /= 1000000000L;

        rate = elapsed > 0.0 ? (download.start - 0) / elapsed : 0.0;

        printf("\r%0.2f%% (%" PRIi64 "B/%" PRIi64 "B; %0.3fs elapsed; "
               "%0.3fB/s)",
               (((float) download.start) / dataBlock.byteCount) * 100,
               download.start,
               dataBlock.byteCount,
               elapsed,
               rate);
    }

    printf("\nDone. Downloaded %" PRIi64 "B in %0.3fs (%0.3fB/s).\n",
           download.start,
           elapsed,
           rate);

fileCleanup:
    close(downloadFd);
instrumentCleanup:
    RBRGen4_close(instrument);

    return status;
}
