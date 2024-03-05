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
/* Required for strerror. */
#include <string.h>
/* Required for close. */
#include <unistd.h>

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
    RBRInstrumentGen4WiFi wifi;

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
    case RBRINSTRUMENTGEN4_LINK_WIFI:
    {
        RBRInstrumentGen4_getWiFi(instrument, &wifi);
        printf("Connected in WiFi mode at %s baud. Timeout is %d\n",
               RBRInstrumentGen4SerialBaudRate_name(wifi.baudRate),
               wifi.commandTimeout);
        break;
    }
    default:
        fprintf(stderr,
                "Warning: connection method to the instrument is unclear, so"
                " poll can't be executed.\n");
        goto instrumentCleanup;
    }

    /************ ensure default state ************/
    RBRInstrumentGen4DeploymentStatus deploymentStatus = RBRINSTRUMENTGEN4_STATUS_UNKNOWN;
    RBRInstrumentGen4_disable(instrument, deploymentStatus);

    RBRInstrumentGen4Datasets datasets;
    RBRInstrumentGen4_getDatasets(instrument, &datasets);
    RBRInstrumentGen4_deleteDatasetAll(instrument, &datasets);

    RBRInstrumentGen4Configs configs;
    RBRInstrumentGen4_getConfigs(instrument, &configs);
    RBRInstrumentGen4_deleteConfigAll(instrument, &configs);

    RBRInstrumentGen4Schedules schedules;
    RBRInstrumentGen4_getSchedules(instrument, &schedules);
    RBRInstrumentGen4_deleteScheduleAll(instrument, &schedules);

    RBRInstrumentGen4Groups groups;
    RBRInstrumentGen4_getGroups(instrument, &groups);
    RBRInstrumentGen4_deleteGroupAll(instrument, &groups);

    /************ group definition ************/
    // populate all channels and calibrations
    RBRInstrumentGen4Channels channels;
    RBRInstrumentGen4_getChannels(instrument, &channels);

    // specify grouplabel, channel labels, and create group instance
    RBRInstrumentGen4Group group_pts;
    init_groupStructure(instrument,
                        GROUP_PTS_LABEL,
                        GROUP_PTS_CHANNELS,
                        &channels,
                        &group_pts,
                        &groups); // warning: need to read error!!!

    /************ schedule definition ************/
    RBRInstrumentGen4Schedule schedule_pts;
    // warning: read error for init_scheduleStructure!!!
    init_schedule_regimes(instrument,
                          SCHEDULE_PTS_LABEL,
                          SCHEDULE_PTS_GROUPS,
                          SCHEDULE_PTS_MODE,
                          &groups,
                          SCHEDULE_PTS_REGIMES,
                          &schedule_pts,
                          &schedules);

    // warning: read error for init_scheduleStructure!!!
    RBRInstrumentGen4_setSchedule(instrument, &schedule_pts);
    // if ((err=RBRInstrumentGen4_createSchedule(instrument, &schedule, &schedules)) != RBRINSTRUMENTGEN4_SUCCESS)
    // {
    //     fprintf(stderr, "%s: Failed to establish instrument connection: %s!\n",
    //             programName,
    //             RBRInstrumentGen4Error_name(err));
    //     status = EXIT_FAILURE;
    //     goto instrumentCleanup;
    // }

    /************ configuration definition ************/
    RBRInstrumentGen4Config config_ascent;
    init_configStructure(instrument,
                         CONFIG_ASCENT_LABEL,
                         CONFIG_ASCENT_SCHEDULES,
                         &schedules,
                         &config_ascent,
                         &configs);

    /************ deployment parameters ************/
    RBRInstrumentGen4Deployment deployment;
    RBRInstrumentGen4_getDeployment(instrument, &deployment);
    //GEN4 TODO: if it's pending, could we modify parameters???
    if (deployment.status == RBRINSTRUMENTGEN4_STATUS_LOGGING || deployment.status == RBRINSTRUMENTGEN4_STATUS_PENDING){
        printf("%s: Instrument is logging/pending. I'm going to disable it first.\n",
               programName);
        RBRInstrumentGen4_disable(instrument, deploymentStatus);
    }
    // str_to_deploymentDatetime(&deployment.startTime, STARTTIME);
    // str_to_deploymentDatetime(&deployment.endTime, ENDTIME);
    RBRInstrumentGen4_setDeployment(instrument, &deployment);
    // printf("deployment starttime:%" PRId64 "\n", deployment.starttime);

    /************ start of ascent ************/
    // ensures the memory is cleared first
    RBRInstrumentGen4_deleteDatasetAll(instrument, &datasets);

    // verify the configurations for enable
    RBRInstrumentGen4_verify(instrument,
                             &config_ascent,
                             NEW_DATASET_LABEL,
                             deploymentStatus);

    // enable the instrument
    RBRInstrumentGen4_enable(instrument,
                             &config_ascent,
                             NEW_DATASET_LABEL,
                             false,
                             RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL,
                             &datasets,
                             deploymentStatus); // false, normal are default.

instrumentCleanup:
    RBRInstrumentGen4_close(instrument);

fileCleanup:
    close(instrumentFd);

    return status;
}
