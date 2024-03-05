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

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4 instrumentSpace;
    RBRInstrumentGen4 *instrument = &instrumentSpace;

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
            RBRINSTRUMENTGEN4_LIB_NAME,
            RBRINSTRUMENTGEN4_LIB_VERSION,
            RBRINSTRUMENTGEN4_LIB_BUILD_DATE);

    // check instrument communication is fine.
    RBRInstrumentGen4Callbacks callbacks = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite
    };

    if ((err = RBRInstrumentGen4_open(
             &instrument,
             &callbacks,
             INSTRUMENT_COMMAND_TIMEOUT_MSEC,
             (void *) &instrumentFd)) != RBRINSTRUMENTGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to establish instrument connection: %s!\n", programName, RBRInstrumentGen4Error_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup; // Failure case, memory allocated by this constructor is freed.
    }

    //******************seems unnecessary********************//
    RBRInstrumentGen4Id id;
    RBRInstrumentGen4_getId(instrument, &id);
    printf("The instrument is an %s (fwtype %d), serial number %06d, with "
           "firmware v%s.\n",
           id.model,
           id.fwtype,
           id.serial,
           id.version);

    // RBRInstrumentGen4HardwareRevision hwrev;
    // RBRInstrumentGen4_getHardwareRevision(instrument, &hwrev);
    // printf("It's PCB rev%c, CPU rev%s, BSL v%c.\n",
    //        hwrev.pcb,
    //        hwrev.cpu,
    //        hwrev.bsl);
    //*********************** end of seems unnecessary part *****************//

    // create a file to store downloaded data
    char filename[PATH_MAX + 1];
    char currentTimestamp[16];
    getCurrentTimestamp(currentTimestamp);
    snprintf(filename, sizeof(filename), "%06d_%s.bin", id.serial, currentTimestamp); // specify the file name. e.g. 999999_20231023_143711.bin
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
    RBRInstrumentGen4Datasets datasets;
    RBRInstrumentGen4_getDatasets(instrument, &datasets);
    if (datasets.datasetlist.count <= 0)
    {
        printf("Error: There's no dataset available in this instrument. Quit.\n");
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }
    // download from specified dataset.
    RBRInstrumentGen4Dataset *targetDataset;

    // get the data block info of specified dataset and schedule.
    // need dataset struct instance to findout the bytecount.
    match_dataset_to_label(&targetDataset,
                           &datasets,
                           DATASET_LABEL);
    printf("Dataset %s contains data from following schedules: %s\n", targetDataset->label, targetDataset->schedulelist);

    // TODO: check the specified scheduleLabel is within dataset_to_download.schedulelist!!!!!!
    RBRInstrumentGen4_getDatasetByScheduleBlock(instrument,
                                                SCHEDULE_LABEL,
                                                RBRINSTRUMENTGEN4_DATA,
                                                targetDataset); // targetDataset.bytecount is now targetdataset/schedule/datablock.
    printf("Dataset %s schedule %s contains %" PRIi32 "B data. Data format is %s.\n", targetDataset->label, SCHEDULE_LABEL, targetDataset->bytecount, RBRInstrumentGen4Datatype_name(targetDataset->datatype));

    uint8_t buf[CHUNK_SIZE];
    // meaning: datablock, bytecount, download from beginning.
    // comparing to L3: data_L3{..., size, offset, *data}; download_data_gen4{..., countvalue, start, *data};
    RBRInstrumentGen4Download download_data_pts = {
        .block = RBRINSTRUMENTGEN4_DATA,
        .countkey = RBRINSTRUMENTGEN4_COUNTKEY_BYTECOUNT,
        .start = 1,
        .data = buf
    };

    printf("Downloading:\n");

    struct timespec start;
    struct timespec now;
    double elapsed = 0.0;
    double rate = 0.0;
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (download_data_pts.start < targetDataset->bytecount) // if not downloaded all data from targetDataset/schedule/datablock
    {
        download_data_pts.countvalue = sizeof(buf);                       // specify download bytes
        err = RBRInstrumentGen4_download(instrument, &download_data_pts); // comparing with L3: RBRInstrument_readdata(instrument, &data);
        if (err == RBRINSTRUMENTGEN4_SUCCESS)
        {
            write(downloadFd, download_data_pts.data, download_data_pts.countvalue);
            download_data_pts.start += download_data_pts.countvalue;
        }
        else if (err == RBRINSTRUMENTGEN4_TIMEOUT)
        {
            printf("\nWarning: timeout. Retrying...\n");
        }
        else
        {
            printf("\nError: %s", RBRInstrumentGen4Error_name(err));
            break;
        }

        clock_gettime(CLOCK_MONOTONIC, &now);

        elapsed = now.tv_sec - start.tv_sec;
        elapsed *= 1000000000L;
        elapsed += now.tv_nsec - start.tv_nsec;
        elapsed /= 1000000000L;

        rate = elapsed > 0.0 ? (download_data_pts.start - 0) / elapsed : 0.0;

        printf("\r%0.2f%% (%" PRIi32 "B/%" PRIi32 "B; %0.3fs elapsed; "
               "%0.3fB/s)",
               (((float) download_data_pts.start) / targetDataset->bytecount) * 100,
               download_data_pts.start,
               targetDataset->bytecount,
               elapsed,
               rate);
    }

    printf("\nDone. Downloaded %" PRIi32 "B in %0.3fs (%0.3fB/s).\n",
           download_data_pts.start,
           elapsed,
           rate);

fileCleanup:
    close(downloadFd);
instrumentCleanup:
    RBRInstrumentGen4_close(instrument);

    return status;
}
