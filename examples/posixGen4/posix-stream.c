/**
 * \file posix-stream.c
 *
 * \brief Example of using the library to stream instrument data in a POSIX
 * environment.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Prerequisite for gmtime_r in time.h. */
#define _POSIX_C_SOURCE 200112L

/* Required for errno. */
#include <errno.h>
/* Required for fprintf, printf. */
#include <stdio.h>
/* Required for EXIT_SUCCESS, etc. */
#include <stdlib.h>
/* Required for strerror. */
#include <string.h>
/* Required for gmtime_r, time_t, strftime. */
#include <time.h>
/* Required for close. */
#include <unistd.h>

#include <signal.h>
#include <math.h>
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

//************************************* end of customer defined parameters *************************************//

bool terminate = 0;
void sig_handler(int signo)
{
    if (signo == SIGINT || signo == SIGTERM)
    {
        printf("Received SIGINT or SIGTERM.\n");
        terminate = 1;
    }
}

RBRInstrumentGen4Error instrumentSample(
    const struct RBRInstrumentGen4 *instrument,
    const struct RBRInstrumentGen4Sample *const sample)
{
    /* Unused. */
    (void) instrument;

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
    char *devicePath;

    int status = EXIT_SUCCESS;
    int instrumentFd;

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4 *instrument = NULL;
    // no dynamic allocation case:
    RBRInstrumentGen4 instrumentSpace;
    instrument = &instrumentSpace;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s device\n", argv[0]);
        return EXIT_FAILURE;
    }

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

    RBRInstrumentGen4Sample sampleBuffer;
    RBRInstrumentGen4Callbacks callbacks = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite,
        .sample = instrumentSample,
        .sampleBuffer = &sampleBuffer
    };

    if ((err = RBRInstrumentGen4_open(
             &instrument,
             &callbacks,
             INSTRUMENT_COMMAND_TIMEOUT_MSEC,
             (void *) &instrumentFd)) != RBRINSTRUMENTGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to establish instrument connection: %s!\n", programName, RBRInstrumentGen4Error_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    // set streaming in real time during deployment:
    RBRInstrumentGen4Link link;
    RBRInstrumentGen4_getLink(instrument, &link);
    printf("Connected to the instrument via %s.\n",
           RBRInstrumentGen4Link_name(link));

    //(optional) get details about the connection.
    switch (link)
    {
    case RBRINSTRUMENTGEN4_LINK_USB:
        break;
    case RBRINSTRUMENTGEN4_LINK_SERIAL:
    {
        RBRInstrumentGen4Serial serial;
        RBRInstrumentGen4_getSerial(instrument, &serial);
        printf("Connected in %s mode at %s baud.\n",
               RBRInstrumentGen4SerialMode_name(serial.mode),
               RBRInstrumentGen4SerialBaudRate_name(serial.baudRate));
        break;
    }
    /* WiFi is not yet implemented */
    #if 0
    case RBRINSTRUMENTGEN4_LINK_WIFI:
    {
        RBRInstrumentGen4WiFi wifi;
        RBRInstrumentGen4_getWiFi(instrument, &wifi);
        printf("Connected in WiFi mode at %s baud. Timeout is %d\n",
               RBRInstrumentGen4SerialBaudRate_name(wifi.baudRate),
               wifi.commandTimeout);
        break;
    }
    #endif
    default:
        fprintf(stderr,
                "Warning: connection method to the instrument is unclear, so"
                " streaming can't be enabled. Stop. \n");
        goto instrumentCleanup;
    }

    // check deployment status.

    //(optional) verify

    // enable with config and dataset.
    // group -> channelList
    // schedule -> groupList, stream (serial, usb, off), store (on), mode (continuous, average, tide, burst, wave, ddsampling, regimes)
    // config -> scheduleList
    // outputformat -> specify format for data output
    // verify -> config, dataset
    // delpoyment -> check status, set startime, endtime
    // enable -> config, dataset, simulation, storageMode

    // need to change all list to a struct (array of strings, and count)

    /************ ensure default state ************/
    RBRInstrumentGen4LoggingState loggingState = RBRINSTRUMENTGEN4_UNKNOWN_LOGGING_STATE;
    RBRInstrumentGen4_disable(instrument, &loggingState);

    RBRInstrumentGen4DatasetPool datasetPool;
    RBRInstrumentGen4_getDatasetPool(instrument, &datasetPool);
    RBRInstrumentGen4_deleteDatasetAll(instrument, &datasetPool);

    RBRInstrumentGen4ConfigPool configPool;
    RBRInstrumentGen4_getConfigPool(instrument, &configPool);
    RBRInstrumentGen4_deleteConfigAll(instrument, &configPool);

    RBRInstrumentGen4SchedulePool schedulePool;
    RBRInstrumentGen4_getSchedulePool(instrument, &schedulePool);
    RBRInstrumentGen4_deleteScheduleAll(instrument, &schedulePool);

    RBRInstrumentGen4GroupPool groupPool;
    RBRInstrumentGen4_getGroupPool(instrument, &groupPool);
    RBRInstrumentGen4_deleteGroupAll(instrument, &groupPool);

    /************ group definition ************/
    // populate all channelPool and calibrations
    RBRInstrumentGen4ChannelPool channelPool;
    RBRInstrumentGen4_getChannelPool(instrument, &channelPool);

    // specify groupLabel, channel labels, and create group instance
    RBRInstrumentGen4Group* group_pts;
    RBRInstrumentGen4_initNewGroup(instrument,
                        GROUP_PTS_LABEL,
                        GROUP_PTS_CHANNELS,
                        GROUP_PTS_CHANNEL_COUNT,
                        &channelPool,
                        &groupPool,
                        &group_pts); // warning: need to read error!!!

    /************ schedule definition ************/
    RBRInstrumentGen4Schedule* schedule_pts;
    RBRInstrumentGen4_initNewScheduleRegimes(instrument,
                          SCHEDULE_PTS_LABEL,
                          SCHEDULE_PTS_GROUPS,
                          SCHEDULE_PTS_GROUP_COUNT,
                          SCHEDULE_PTS_MODE,
                          SCHEDULE_PTS_REGIMES,
                          &groupPool,
                          &schedulePool,
                          &schedule_pts);

    schedule_pts->stream = link;
    // warning: read error for RBRInstrumentGen4_initNewSchedule!!!
    RBRInstrumentGen4_setSchedule(instrument, schedule_pts);

    /************ configuration definition ************/
    RBRInstrumentGen4Config *config_ascent;
    RBRInstrumentGen4_initNewConfig(instrument,
                         CONFIG_ASCENT_LABEL,
                         CONFIG_ASCENT_SCHEDULES,
                        CONFIG_ASCENT_SCHEDULE_COUNT,
                         &schedulePool,
                         &configPool,
                         &config_ascent);

    // specify outputformat. The setter sends every parameter of the command,
    // so read the current format and change only the sample fields.
    RBRInstrumentGen4OutputFormat outputformat;
    RBRInstrumentGen4_getOutputFormat(instrument, &outputformat);
    outputformat.sn = true;
    outputformat.scheduleLabel = true;
    outputformat.dateTime = false;
    outputformat.crc = true;
    RBRInstrumentGen4_setOutputFormat(instrument, &outputformat);

    /************ deployment parameters ************/
    // need to stop if it's logging.
    RBRInstrumentGen4Deployment deployment;
    RBRInstrumentGen4_getDeployment(instrument, &deployment);

    str_to_deploymentDatetime(&deployment.startTime, STARTTIME);
    RBRInstrumentGen4_setDeployment(instrument, &deployment);

    // verify the configurations for enable
    RBRInstrumentGen4_verify(instrument,
                             config_ascent,
                             NEW_DATASET_LABEL,
                             &loggingState);

    printf("%s: Start instrument logging with default_config.\n",
           programName);
    RBRInstrumentGen4Dataset *dataset;
    if ((err = RBRInstrumentGen4_enable(instrument,
                                        config_ascent,
                                        NEW_DATASET_LABEL,
                                        RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL,
                                        &datasetPool,
                                        &dataset,
                                        &loggingState)) != RBRINSTRUMENTGEN4_SUCCESS)
    {
        fprintf(stderr,
                "%s: Failed to start instrument: %s!\n",
                programName,
                RBRInstrumentGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    };

    while (!terminate)
    {
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);
        if ((err = RBRInstrumentGen4_readSample(instrument)) != RBRINSTRUMENTGEN4_SUCCESS)
        {
            fprintf(stderr, "Error: %s\n", RBRInstrumentGen4Error_name(err));
        }
    }

instrumentCleanup:
    RBRInstrumentGen4_close(instrument);
fileCleanup:
    close(instrumentFd);
    return status;
}
