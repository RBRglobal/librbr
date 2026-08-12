/**
 * \file posix-singleScheduleSingleConfig.c
 *
 * \brief Example of using the library to enable instrument with sigle schedule and single configuration.
 * see Gen4 command reference quick start example 1.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for errno. */
#include <errno.h>
/* Required for isnan. */
#include <math.h>
/* Required for fprintf, printf, snprintf. */
#include <stdio.h>
/* Required for EXIT_SUCCESS, etc. */
#include <stdlib.h>
/* Required for strerror. */
#include <string.h>
/* Required for close. */
#include <unistd.h>

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Configuration.h"
#include "RBRInstrumentGen4Memory.h"
#include "RBRInstrumentGen4Schedule.h"
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
            TEMPERATURE,                                \
            SALINITY_DYNCORR                            \
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
#define CONFIG_ASCENT_SCHEDULE_COUNT 1

#define STARTTIME "20000101000000"
#define ENDTIME "20991231235959"

#define NEW_DATASET_LABEL "ds_ascent"

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
           RBRInstrumentGen4Link_name(link));

    RBRInstrumentGen4Serial serial;

    switch (link)
    {
    case RBRINSTRUMENTGEN4_LINK_USB:
        break;
    case RBRINSTRUMENTGEN4_LINK_SERIAL:
    {
        RBRInstrumentGen4_getSerial(instrument, &serial);
        printf("Connected in %s mode at %s baud.\n",
               RBRInstrumentGen4SerialMode_name(serial.mode),
               RBRInstrumentGen4SerialBaudRate_name(serial.baudRate));
        break;
    }
    default:
        fprintf(stderr,
                "Warning: connection method to the instrument is unclear, so"
                " poll can't be executed.\n");
        goto instrumentCleanup;
    }

    /************ ensure default state ************/
    RBRInstrumentGen4LoggingState loggingState = RBRINSTRUMENTGEN4_UNKNOWN_LOGGING_STATE;
    RBRInstrumentGen4_disable(instrument, &loggingState);

    RBRInstrumentGen4DatasetPool datasetPool;
    RBRInstrumentGen4_deleteDatasetAll(instrument, &datasetPool);

    RBRInstrumentGen4ConfigPool configPool;
    RBRInstrumentGen4_deleteConfigAll(instrument, &configPool);

    RBRInstrumentGen4SchedulePool schedulePool;
    RBRInstrumentGen4_deleteScheduleAll(instrument, &schedulePool);

    RBRInstrumentGen4GroupPool groupPool;
    RBRInstrumentGen4_deleteGroupAll(instrument, &groupPool);

    /************ group definition ************/
    /* populate all channelPool and calibrations */
    RBRInstrumentGen4ChannelPool channelPool;
    RBRInstrumentGen4_getChannelPool(instrument, &channelPool);

    for (int32_t i = 0; i < channelPool.count; i++)
    {
        RBRInstrumentGen4_getChannel(instrument, &channelPool.pool[i]);
        printf(
            "%s,%s,%u,%d,%d,%d,%s,%s,%f,%u", 
            channelPool.pool[i].label,
            channelPool.pool[i].type,
            channelPool.pool[i].address,
            channelPool.pool[i].settlingTime,
            channelPool.pool[i].readTime,
            channelPool.pool[i].guardTime,
            channelPool.pool[i].equation,
            channelPool.pool[i].userUnits,
            channelPool.pool[i].gain.currentGain,
            channelPool.pool[i].derived
        );
    }
    for (int32_t i = 0; i < channelPool.count; i++)
    {
        RBRInstrumentGen4_getCalibration(instrument, &channelPool.pool[i].calibration);
    }

    /* specify groupLabel, channel labels, and create group instance */
    RBRInstrumentGen4Group* group_pts = NULL;

    RBRInstrumentGen4_initNewGroup(instrument,
                        GROUP_PTS_LABEL,
                        GROUP_PTS_CHANNELS,
                        GROUP_PTS_CHANNEL_COUNT,
                        &channelPool,
                        &groupPool,
                        &group_pts);

    /************ schedule definition ************/
    RBRInstrumentGen4Schedule* schedule;
    RBRInstrumentGen4_initNewScheduleRegimes(instrument,
                         SCHEDULE_PTS_LABEL,
                         SCHEDULE_PTS_GROUPS,
                         SCHEDULE_PTS_GROUP_COUNT,
                         SCHEDULE_PTS_MODE,
                         SCHEDULE_PTS_REGIMES,
                         &groupPool,
                         &schedulePool,
                         &schedule);

    /************ configuration definition ************/
    RBRInstrumentGen4Config* config;
    RBRInstrumentGen4_initNewConfig(instrument,
                        CONFIG_ASCENT_LABEL,
                        CONFIG_ASCENT_SCHEDULES,
                        CONFIG_ASCENT_SCHEDULE_COUNT,
                        &schedulePool,
                        &configPool,
                        &config);

    /************ deployment parameters ************/
    RBRInstrumentGen4Deployment deployment;
    RBRInstrumentGen4_getDeployment(instrument, &deployment);

    str_to_deploymentDatetime(&deployment.startTime, STARTTIME);
    RBRInstrumentGen4_setDeployment(instrument, &deployment);

    /************ start of ascent ************/
    /* verify the configurations for enable */
    RBRInstrumentGen4_verify(instrument,
                             config,
                             NEW_DATASET_LABEL,
                             &loggingState);

    /* enable the instrument */
    RBRInstrumentGen4Dataset *dataset;
    RBRInstrumentGen4_enable(instrument,
                             config,
                             NEW_DATASET_LABEL,
                             RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL,
                             &datasetPool,
                             &dataset,
                             &loggingState);

instrumentCleanup:
    RBRInstrumentGen4_close(instrument);

fileCleanup:
    close(instrumentFd);

    return status;
}
