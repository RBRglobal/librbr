/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file posix-download.c
 *
 * \brief Example of using the library to download one schedule's sample data
 * from a dataset to a file.
 *
 * Intended to run after posix-enable has recorded some
 * data. Nothing is written to the instrument.
 */

/* Prerequisite for PATH_MAX in limits.h. */
#define _POSIX_C_SOURCE 200112L

/* Required for errno. */
#include <errno.h>
/* Required for open. */
#include <fcntl.h>
/* Required for PRId32, PRId64. */
#include <inttypes.h>
/* Required for PATH_MAX. */
#include <limits.h>
/* Required for fprintf, printf, snprintf. */
#include <stdio.h>
/* Required for EXIT_SUCCESS, etc. */
#include <stdlib.h>
/* Required for strcmp, strerror. */
#include <string.h>
/* Required for clock_gettime, struct timespec. */
#include <time.h>
/* Required for close, write. */
#include <unistd.h>

#include "RBRGen4.h"
#include "RBRGen4Commands.h"
#include "posix-shared.h"

/* == Customer defined parameters == */

/* The dataset and schedule recorded by posix-enable. */
#define DATASET_LABEL  "ds_ascent"
#define SCHEDULE_LABEL "sch_asc_pt"

/* Tune these to your instrument. */
#define DATASET_COUNT  16
#define SCHEDULE_COUNT 16

/* The number of bytes requested per `download` command. */
#define CHUNK_SIZE 4096

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

double elapsedSeconds(const struct timespec *start)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (double) (now.tv_sec - start->tv_sec) + (double) (now.tv_nsec - start->tv_nsec) / 1e9;
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

    int status = EXIT_SUCCESS;
    RBRGen4Error err = RBRGEN4_SUCCESS;
    RBRGen4 conn;
    uint8_t commandBuffer[RBRGEN4_COMMAND_BUFFER_DEFAULT];
    uint8_t responseBuffer[RBRGEN4_RESPONSE_BUFFER_DEFAULT];
    const RBRGen4Environment environment = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite,
        .command = commandBuffer,
        .commandCapacity = sizeof(commandBuffer),
        .response = responseBuffer,
        .responseCapacity = sizeof(responseBuffer),
    };

    err =
        RBRGen4_open(&conn, &environment, INSTRUMENT_COMMAND_TIMEOUT_MSEC, (void *) &instrumentFd);
    if (err) {
        logCmdError(&conn, err, "Failed to establish instrument connection");
        goto fileCleanup;
    }

    /* Find the dataset among those stored on the instrument */
    RBRGen4Dataset datasetPoolBuf[DATASET_COUNT];
    RBRGen4DatasetPool datasetPool = {
        .size = DATASET_COUNT,
        .pool = datasetPoolBuf,
    };
    err = RBRGen4_getDatasetPool(&conn, &datasetPool);
    if (err == RBRGEN4_TRUNCATED) {
        int32_t count;
        err = RBRGen4_getDatasetCount(&conn, &count);
        if (err != RBRGEN4_SUCCESS) {
            logCmdError(&conn, err, "Failed to get dataset count");
            goto instrumentCleanup;
        }

        printf("%s: Warning: not enough space in dataset pool to store all"
               " datasets; only the first %" PRId32 " are stored out of the"
               " instrument's %" PRId32 " datasets\n",
               programName,
               datasetPool.len,
               count);
    } else if (err) {
        logCmdError(&conn,
                    err,
                    "Failed to get dataset pool -- does this instrument"
                    " support storing data?");
        goto instrumentCleanup;
    }

    RBRGen4Dataset *dataset = NULL;
    for (int32_t i = 0; i < datasetPool.len; i++) {
        if (strcmp(datasetPool.pool[i].label, DATASET_LABEL) == 0) {
            dataset = &datasetPool.pool[i];
            break;
        }
    }
    if (dataset == NULL) {
        fprintf(stderr,
                "%s: Dataset %s not found; run"
                " posix-enable first\n",
                programName,
                DATASET_LABEL);
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    /* Read the dataset's parameters, including the schedules it ran */
    RBRGen4Label scheduleListBuf[SCHEDULE_COUNT];
    RBRGen4LabelList scheduleList = {
        .size = SCHEDULE_COUNT,
        .labels = scheduleListBuf,
    };
    err = RBRGen4_getDataset(&conn, dataset, &scheduleList);
    if (err == RBRGEN4_TRUNCATED) {
        printf("%s: Warning: not enough space in schedule list to store all"
               " schedules; only the first %" PRId32 " are stored\n",
               programName,
               scheduleList.len);
    } else if (err) {
        logCmdError(&conn, err, "Failed to get dataset");
        goto instrumentCleanup;
    }
    printf("Dataset %s is %s, %" PRId64 " bytes, stored as %s, and ran"
           " schedules:",
           dataset->label,
           RBRGen4DatasetStatus_name(dataset->status),
           dataset->byteCount,
           RBRGen4DataType_name(dataset->dataType));
    for (int32_t i = 0; i < scheduleList.len; i++) {
        printf(" %s", scheduleList.labels[i]);
    }
    printf("\n");

    /* Get the size of the schedule's sample data. This fails if the schedule
     * is not in the dataset. */
    RBRGen4DatasetDataBlock dataBlock;
    err = RBRGen4_getDatasetScheduleDataBlock(&conn, dataset, SCHEDULE_LABEL, &dataBlock);
    if (err) {
        logCmdError(&conn, err, "Failed to get schedule data block");
        goto instrumentCleanup;
    }
    printf("Schedule %s has %" PRId64 " samples in %" PRId64 " bytes.\n",
           SCHEDULE_LABEL,
           dataBlock.sampleCount,
           dataBlock.byteCount);

    /* Create the output file, e.g. 999999_ds_ascent_sch_asc_pt.bin */
    RBRGen4Id4 id;
    err = RBRGen4_getId4(&conn, &id);
    if (err) {
        logCmdError(&conn, err, "Failed to get id");
        goto instrumentCleanup;
    }
    char filename[PATH_MAX + 1];
    snprintf(filename,
             sizeof(filename),
             "%06" PRId32 "_%s_%s.bin",
             id.sn,
             dataset->label,
             SCHEDULE_LABEL);
    int downloadFd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (downloadFd < 0) {
        fprintf(stderr,
                "%s: Failed to open output file %s: %s!\n",
                programName,
                filename,
                strerror(errno));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }
    printf("Downloading to %s:\n", filename);

    /* Download the sample data in chunks, from the beginning */
    uint8_t buf[CHUNK_SIZE];
    RBRGen4DownloadData download = {
        .unit = RBRGEN4_DOWNLOAD_DATA_UNIT_BYTES,
        .start = 0,
        .data = buf,
        .dataSize = sizeof(buf),
    };

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (download.start < dataBlock.byteCount) {
        download.count = sizeof(buf);
        err = RBRGen4_downloadDatasetScheduleData(&conn, dataset, SCHEDULE_LABEL, &download);
        if (err == RBRGEN4_TIMEOUT || err == RBRGEN4_CHECKSUM_ERROR) {
            /* The chunk is requested again from the same offset. */
            printf("\n%s: Warning: %s; retrying\n", programName, RBRGen4Error_name(err));
            continue;
        } else if (err) {
            logCmdError(&conn, err, "Failed to download schedule data");
            goto downloadCleanup;
        }

        /* The count is updated to what the instrument actually sent. */
        if (write(downloadFd, download.data, download.byteCount) != download.byteCount) {
            fprintf(stderr, "%s: Failed to write output file: %s!\n", programName, strerror(errno));
            status = EXIT_FAILURE;
            goto downloadCleanup;
        }
        download.start += download.count;

        double elapsed = elapsedSeconds(&start);
        printf("\r%5.1f%% (%" PRId64 "/%" PRId64 " bytes; %.1fs; %.0f B/s)",
               100.0 * (double) download.start / (double) dataBlock.byteCount,
               download.start,
               dataBlock.byteCount,
               elapsed,
               elapsed > 0.0 ? (double) download.start / elapsed : 0.0);
        fflush(stdout);
    }
    printf("\nDone.\n");

downloadCleanup:
    close(downloadFd);

instrumentCleanup:
    RBRGen4_close(&conn);

fileCleanup:
    close(instrumentFd);

    return (err == RBRGEN4_SUCCESS) ? status : EXIT_FAILURE;
}
