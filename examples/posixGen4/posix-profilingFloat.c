/**
 * \file posix-profilingFloat.c
 *
 * \brief Example of using the library to enable an instrument with a
 * regimes-mode (profiling float) schedule.
 * see Gen4 command reference quick start example 1.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Prerequisite for gmtime_r in time.h. */
#define _POSIX_C_SOURCE 200112L

/* Required for errno. */
#include <errno.h>
/* Required for isnan. */
#include <math.h>
/* Required for fprintf, printf, snprintf. */
#include <stdio.h>
/* Required for strerror. */
#include <string.h>
#include <stdlib.h>
/* Required for close. */
#include <time.h>
#include <unistd.h>

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Configuration.h"
#include "RBRInstrumentGen4Deployment.h"
#include "RBRInstrumentGen4Instrument.h"
#include "RBRInstrumentGen4Memory.h"
#include "RBRInstrumentGen4Schedule.h"
#include "RBRParserGen4.h"
#include "posix-shared.h"

//************************************* customer defined parameters *************************************//
#define PRESSURE "pressure_00"
#define TEMPERATURE "temperature_00"
#define SALINITY_DYNCORR "salinitydyncorr_00"

#define GROUP_PTS_LABEL "gr_pts"
#define GROUP_PTS_CHANNELS                              \
    (const char[][RBRINSTRUMENTGEN4_CHANNEL_LABEL_MAX]) \
    {                                                   \
        PRESSURE,                                       \
        SALINITY_DYNCORR,                               \
        TEMPERATURE                                     \
    }
#define GROUP_PTS_CHANNEL_COUNT 3

#define SCHEDULE_PTS_LABEL "sch_asc_pts"
#define SCHEDULE_PTS_MODE RBRINSTRUMENTGEN4_SAMPLING_REGIMES
#define SCHEDULE_PTS_DIR RBRINSTRUMENTGEN4_DIRECTION_ASCENDING
#define SCHEDULE_PTS_REF RBRINSTRUMENTGEN4_REFERENCE_SEAPRESSURE
#define SCHEDULE_PTS_COUNT 3

#define SCHEDULE_PTS_GROUPS                          \
    (const char[][RBRINSTRUMENTGEN4_LABEL_NAME_MAX]) \
    {                                                \
        GROUP_PTS_LABEL                              \
    }
#define SCHEDULE_PTS_GROUP_COUNT 1
#define SCHEDULE_PTS_REGIMES           \
    (RBRInstrumentGen4Regimes)         \
    {                                  \
        .direction = SCHEDULE_PTS_DIR, \
        .count = SCHEDULE_PTS_COUNT,   \
        .reference = SCHEDULE_PTS_REF, \
        .boundary1 = 500.0,            \
        .binSize1 = 50.0,              \
        .period1 = 10000,              \
        .boundary2 = 200.0,            \
        .binSize2 = 20.0,              \
        .period2 = 1000,               \
        .boundary3 = 50.0,             \
        .binSize3 = 0.0,               \
        .period3 = 1000                \
    }

#define CONFIG_ASCENT_LABEL "cf_ascent"
#define CONFIG_ASCENT_SCHEDULES                      \
    (const char[][RBRINSTRUMENTGEN4_LABEL_NAME_MAX]) \
    {                                                \
        SCHEDULE_PTS_LABEL                           \
    }

#define STARTTIME "20000101000000"

#define NEW_DATASET_LABEL "ds_ascent"

#define CHUNK_LEN_BYTES 1000

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

    //------(optional) get link type: USB/serial/wifi---------------------------------------------
    RBRInstrumentGen4Link link;
    RBRInstrumentGen4_getLink(instrument, &link);
    printf("Connected to the instrument via %s.\n",
           RBRInstrumentGen4LinkType_name(link.type));

    RBRInstrumentGen4LinkSerial serial;

    switch (link.type)
    {
    case RBRINSTRUMENTGEN4_LINK_TYPE_USB:
        break;
    case RBRINSTRUMENTGEN4_LINK_TYPE_SERIAL:
    {
        RBRInstrumentGen4_getLinkSerial(instrument, &serial);
        printf("Connected in %s mode at %s baud.\n",
               RBRInstrumentGen4LinkSerialMode_name(serial.mode),
               RBRInstrumentGen4LinkSerialBaudRate_name(serial.baudRate));
        break;
    }
    default:
        fprintf(stderr,
                "Warning: connection method to the instrument is unclear, so"
                " poll can't be executed.\n");
        goto instrumentCleanup;
    }

    /************ ensure default state ************/
    RBRInstrumentGen4InstrumentState loggingState
        = RBRINSTRUMENTGEN4_UNKNOWN_INSTRUMENT_STATE;
    RBRInstrumentGen4_disable(instrument, &loggingState);

    RBRInstrumentGen4DatasetPool datasetPool;
    RBRInstrumentGen4_deleteDatasetAll(instrument, &datasetPool);

    RBRInstrumentGen4ConfigPool configPool;
    RBRInstrumentGen4_deleteConfigAll(instrument);

    RBRInstrumentGen4SchedulePool schedulePool;
    RBRInstrumentGen4_deleteScheduleAll(instrument, &schedulePool);

    RBRInstrumentGen4GroupPool groupPool;
    RBRInstrumentGen4_deleteGroupAll(instrument);

    /************ group definition ************/
    /* populate all channels and calibrations */
    RBRInstrumentGen4ChannelPool channelPool;
    RBRInstrumentGen4_getChannelPool(instrument, &channelPool);

    for (int32_t i = 0; i < channelPool.count; i++)
    {
        RBRInstrumentGen4_getChannel(instrument, &channelPool.pool[i]);
        printf(
            "%s,%s,%d,%d,%d,%s,%s,%s,%u",
            channelPool.pool[i].label,
            channelPool.pool[i].type,
            channelPool.pool[i].settlingTime,
            channelPool.pool[i].measuringTime,
            channelPool.pool[i].readOutTime,
            channelPool.pool[i].userUnits,
            RBRInstrumentGen4ChannelNature_name(channelPool.pool[i].nature),
            channelPool.pool[i].device,
            channelPool.pool[i].derived
        );
    }
    for (int32_t i = 0; i < channelPool.count; i++)
    {
        RBRInstrumentGen4Calibration calibration;
        snprintf(calibration.label,
                 sizeof(calibration.label),
                 "%s",
                 channelPool.pool[i].label);
        RBRInstrumentGen4_getCalibration(instrument, &calibration);
    }

    /* specify groupLabel, channel labels, and create group instance */
    RBRInstrumentGen4Group group_pts;

    RBRInstrumentGen4_initNewGroup(instrument,
                        GROUP_PTS_LABEL,
                        GROUP_PTS_CHANNELS,
                        GROUP_PTS_CHANNEL_COUNT,
                        &channelPool,
                        &group_pts);

    /************ schedule definition ************/
    RBRInstrumentGen4Schedule* schedule_asc_pts;
    RBRInstrumentGen4_initNewScheduleRegimes(instrument,
                         SCHEDULE_PTS_LABEL,
                         SCHEDULE_PTS_GROUPS,
                         SCHEDULE_PTS_GROUP_COUNT,
                         SCHEDULE_PTS_MODE,
                         SCHEDULE_PTS_REGIMES,
                         &groupPool,
                         &schedulePool,
                         &schedule_asc_pts);

    /************ configuration definition ************/
    RBRInstrumentGen4Config config_ascent;
    RBRInstrumentGen4_initNewConfig(instrument,
                        GROUP_PTS_LABEL,
                        GROUP_PTS_CHANNELS,
                        GROUP_PTS_CHANNEL_COUNT,
                        &schedulePool,
                        &config_ascent);

    /************ deployment parameters ************/
    RBRInstrumentGen4Deployment deployment;
    deployment.gate = RBRINSTRUMENTGEN4_GATE_NONE;
    RBRInstrumentGen4_getDeployment(instrument, &deployment);

    str_to_deploymentDatetime(&deployment.startTime, STARTTIME);
    RBRInstrumentGen4_setDeployment(instrument, &deployment);

    /************ start of ascent ************/
    /* enable the instrument */
    RBRInstrumentGen4Dataset *dataset_ascent;
    RBRInstrumentGen4_enable(instrument,
                             &config_ascent,
                             NEW_DATASET_LABEL,
                             RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL,
                             &datasetPool,
                             &dataset_ascent,
                             &loggingState);

    /************ end of ascent ************/
    /* Stop the current deployment */
    RBRInstrumentGen4_disable(instrument, &loggingState);
    /* Determine how much memory has been used */
    RBRInstrumentGen4_getDataset(instrument,
                                 &configPool,
                                 dataset_ascent);
    RBRInstrumentGen4DatasetInfo dataset_asc_info;
    RBRInstrumentGen4_getDatasetByScheduleBlock(
        instrument,
        schedule_asc_pts,
        RBRINSTRUMENTGEN4_BLOCK_DATA,
        dataset_ascent,
        &dataset_asc_info);
    /* Get the data type */
    RBRInstrumentGen4Instrument info;
    RBRInstrumentGen4_getInstrument(instrument, &info);
    /* Prepare the parser */
    RBRParserGen4 parserSpace;
    RBRParserGen4* parser = &parserSpace;
    RBRParserGen4Config config = {
        .channelCount = group_pts.channelCount,
        .datatype = info.dataType
    };
    RBRInstrumentGen4Sample sampleBuffer;
    RBRParserGen4Callbacks parserCallbacks = {
        .sample = parserSample,
        .sampleBuffer = &sampleBuffer
    };
    RBRParserGen4_init(&parser,
                       &parserCallbacks,
                       &config,
                       NULL);
    /* Now loop over the data to download it in chunks */
    uint8_t buf[CHUNK_LEN_BYTES];
    RBRInstrumentGen4Download download_data_pts = {
        .dataset = dataset_ascent,
        .schedule = schedule_asc_pts,
        .block = RBRINSTRUMENTGEN4_BLOCK_DATA,
        .countKey = RBRINSTRUMENTGEN4_COUNTKEY_BYTECOUNT,
        .countValue = CHUNK_LEN_BYTES,
        .startKey = RBRINSTRUMENTGEN4_COUNTKEY_BYTECOUNT,
        .startOffset = 1,
        .data = buf
    };
    while (download_data_pts.startOffset < dataset_ascent->byteCount) // if not downloaded all data from targetDataset/schedule/datablock
    {
        RBRInstrumentGen4_download(instrument,
                                   &download_data_pts);
        RBRParserGen4_parse(parser,
                            download_data_pts.block,
                            download_data_pts.data,
                            &download_data_pts.countValue);
    }

instrumentCleanup:
    RBRInstrumentGen4_close(instrument);

fileCleanup:
    close(instrumentFd);

    return status;
}
