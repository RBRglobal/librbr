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

#include "RBRGen4.h"
#include "RBRGen4Configuration.h"
#include "RBRGen4Deployment.h"
#include "RBRGen4Instrument.h"
#include "RBRGen4Memory.h"
#include "RBRGen4Parser.h"
#include "posix-shared.h"

//************************************* customer defined parameters *************************************//
#define PRESSURE "pressure_00"
#define TEMPERATURE "temperature_00"
#define SALINITY_DYNCORR "salinitydyncorr_00"

#define GROUP_PTS_LABEL "gr_pts"
#define GROUP_PTS_CHANNELS                              \
    (const RBRGen4Label[]) \
    {                                                   \
        PRESSURE,                                       \
        SALINITY_DYNCORR,                               \
        TEMPERATURE                                     \
    }
#define GROUP_PTS_CHANNEL_COUNT 3

#define SCHEDULE_PTS_LABEL "sch_asc_pts"
#define SCHEDULE_PTS_MODE RBRGEN4_SCHEDULE_MODE_REGIMES
#define SCHEDULE_PTS_DIR RBRGEN4_DIRECTION_ASCENDING
#define SCHEDULE_PTS_REF RBRGEN4_REFERENCE_SEAPRESSURE
#define SCHEDULE_PTS_COUNT 3

#define SCHEDULE_PTS_GROUPS                          \
    (const RBRGen4Label[]) \
    {                                                \
        GROUP_PTS_LABEL                              \
    }
#define SCHEDULE_PTS_GROUP_COUNT 1
#define SCHEDULE_PTS_REGIMES           \
    (RBRGen4Regimes)         \
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
    (const RBRGen4Label[]) \
    {                                                \
        SCHEDULE_PTS_LABEL                           \
    }

#define STARTTIME "20000101000000"

#define NEW_DATASET_LABEL "ds_ascent"

#define CHUNK_LEN_BYTES 1000

RBRGen4Error parserSample(
    const struct RBRGen4Parser *parser,
    const struct RBRGen4Sample *const sample)
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

    return RBRGEN4_SUCCESS;
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
    RBRGen4 *conn = &instrumentSpace;

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
             conn,
             &callbacks,
             INSTRUMENT_COMMAND_TIMEOUT_MSEC,
             (void *) &instrumentFd)) != RBRGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to establish instrument connection: %s!\n", programName, RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup; // Failure case, memory allocated by this constructor is freed.
    }

    //------(optional) get link type: USB/serial/wifi---------------------------------------------
    RBRGen4Link link;
    RBRGen4_getLink(conn, &link);
    printf("Connected to the instrument via %s.\n",
           RBRGen4LinkType_name(link.type));

    RBRGen4LinkSerial serial;

    switch (link.type)
    {
    case RBRGEN4_LINK_TYPE_USB:
        break;
    case RBRGEN4_LINK_TYPE_SERIAL:
    {
        RBRGen4_getLinkSerial(conn, &serial);
        printf("Connected in %s mode at %s baud.\n",
               RBRGen4LinkSerialMode_name(serial.mode),
               RBRGen4LinkSerialBaudRate_name(serial.baudRate));
        break;
    }
    default:
        fprintf(stderr,
                "Warning: connection method to the instrument is unclear, so"
                " poll can't be executed.\n");
        goto instrumentCleanup;
    }

    /************ ensure default state ************/
    RBRGen4InstrumentState loggingState
        = RBRGEN4_UNKNOWN_INSTRUMENT_STATE;
    RBRGen4_disable(conn, &loggingState);

    RBRGen4DatasetPool datasetPool;
    RBRGen4_deleteDatasetAll(conn, &datasetPool);

    RBRGen4ConfigPool configPool;
    RBRGen4_deleteConfigAll(conn);

    RBRGen4SchedulePool schedulePool;
    RBRGen4_deleteScheduleAll(conn, &schedulePool);

    RBRGen4GroupPool groupPool;
    RBRGen4_deleteGroupAll(conn);

    /************ group definition ************/
    /* populate all channels and calibrations */
    RBRGen4ChannelPool channelPool;
    RBRGen4_getChannelPool(conn, &channelPool);

    for (int32_t i = 0; i < channelPool.count; i++)
    {
        RBRGen4_getChannel(conn, &channelPool.pool[i]);
        printf(
            "%s,%s,%d,%d,%d,%s,%s,%s,%u",
            channelPool.pool[i].label,
            channelPool.pool[i].type,
            channelPool.pool[i].settlingTime,
            channelPool.pool[i].measuringTime,
            channelPool.pool[i].readOutTime,
            channelPool.pool[i].userUnits,
            RBRGen4ChannelNature_name(channelPool.pool[i].nature),
            channelPool.pool[i].device,
            channelPool.pool[i].derived
        );
    }
    for (int32_t i = 0; i < channelPool.count; i++)
    {
        RBRGen4Calibration calibration;
        snprintf(calibration.label,
                 sizeof(calibration.label),
                 "%s",
                 channelPool.pool[i].label);
        RBRGen4_getCalibration(conn, &calibration);
    }

    /* specify groupLabel, channel labels, and create group instance */
    RBRGen4Group group_pts;

    RBRGen4_initNewGroup(conn,
                        GROUP_PTS_LABEL,
                        GROUP_PTS_CHANNELS,
                        GROUP_PTS_CHANNEL_COUNT,
                        &channelPool,
                        &group_pts);

    /************ schedule definition ************/
    RBRGen4Schedule* schedule_asc_pts;
    RBRGen4_initNewScheduleRegimes(conn,
                         SCHEDULE_PTS_LABEL,
                         SCHEDULE_PTS_GROUPS,
                         SCHEDULE_PTS_GROUP_COUNT,
                         SCHEDULE_PTS_MODE,
                         SCHEDULE_PTS_REGIMES,
                         &groupPool,
                         &schedulePool,
                         &schedule_asc_pts);

    /************ configuration definition ************/
    RBRGen4Config config_ascent;
    RBRGen4_initNewConfig(conn,
                        GROUP_PTS_LABEL,
                        GROUP_PTS_CHANNELS,
                        GROUP_PTS_CHANNEL_COUNT,
                        &schedulePool,
                        &config_ascent);

    /************ deployment parameters ************/
    RBRGen4Deployment deployment;
    deployment.gate = RBRGEN4_GATE_NONE;
    RBRGen4_getDeployment(conn, &deployment);

    str_to_deploymentDatetime(&deployment.startTime, STARTTIME);
    RBRGen4_setDeployment(conn, &deployment);

    /************ start of ascent ************/
    /* enable the instrument */
    RBRGen4Dataset dataset_ascent_value = {
        .label = NEW_DATASET_LABEL
    };
    RBRGen4Dataset *dataset_ascent = &dataset_ascent_value;
    RBRGen4_enable(conn,
                             &config_ascent,
                             NEW_DATASET_LABEL,
                             RBRGEN4_STORAGE_MODE_NORMAL,
                             &loggingState);

    /************ end of ascent ************/
    /* Stop the current deployment */
    RBRGen4_disable(conn, &loggingState);
    /* Determine how much memory has been used */
    RBRGen4_getDataset(conn,
                                 &configPool,
                                 dataset_ascent);
    RBRGen4DatasetInfo dataset_asc_info;
    RBRGen4_getDatasetByScheduleBlock(
        conn,
        schedule_asc_pts,
        RBRGEN4_BLOCK_DATA,
        dataset_ascent,
        &dataset_asc_info);
    /* Get the data type */
    RBRGen4Instrument info;
    RBRGen4_getInstrument(conn, &info);
    /* Prepare the parser */
    RBRGen4Parser parserSpace;
    RBRGen4Parser* parser = &parserSpace;
    RBRGen4ParserConfig config = {
        .channelCount = group_pts.channelCount,
        .dataType = info.dataType
    };
    RBRGen4Sample sampleBuffer;
    RBRGen4ParserCallbacks parserCallbacks = {
        .sample = parserSample,
        .sampleBuffer = &sampleBuffer
    };
    RBRGen4Parser_init(&parser,
                       &parserCallbacks,
                       &config,
                       NULL);
    /* Now loop over the data to download it in chunks */
    uint8_t buf[CHUNK_LEN_BYTES];
    RBRGen4Download download_data_pts = {
        .dataset = dataset_ascent,
        .schedule = schedule_asc_pts,
        .block = RBRGEN4_BLOCK_DATA,
        .countKey = RBRGEN4_COUNTKEY_BYTECOUNT,
        .countValue = CHUNK_LEN_BYTES,
        .startKey = RBRGEN4_COUNTKEY_BYTECOUNT,
        .startOffset = 1,
        .data = buf
    };
    while (download_data_pts.startOffset < dataset_ascent->byteCount) // if not downloaded all data from targetDataset/schedule/datablock
    {
        RBRGen4_download(conn,
                                   &download_data_pts);
        RBRGen4Parser_parse(parser,
                            download_data_pts.block,
                            download_data_pts.data,
                            &download_data_pts.countValue);
    }

instrumentCleanup:
    RBRGen4_close(conn);

fileCleanup:
    close(instrumentFd);

    return status;
}
