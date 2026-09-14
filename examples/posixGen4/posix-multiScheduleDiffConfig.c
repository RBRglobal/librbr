/**
 * \file posix-multiScheduleDiffConfig.c
 *
 * \brief Example of using the library to enable instrument with multiple schedules and different configurations.
 * It can be useful for RBRargo BGC. See Gen4 command reference quick start example 2.
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

#include "posix-shared.h"

//***************************** Customer defined ***************************//
#define PRESSURE "pressure_00"
#define TEMPERATURE "temperature_00"
#define SALINITY_DYNCORR "salinitydyncorr_00"
#define ODO_CONCENTRATION "oxygenconcentration_00"
#define ODO_TEMP "odotemperature_00"
#define PH "ph_00"
#define BACKSCATTER "backscatter_00"
#define CHLOROPHYLL "chlorophyll_00"
#define FDOM "fdom_00"
#define PAR "par_00"
#define QUAD_1 "irradiance_00"
#define QUAD_2 "irradiance_01"
#define QUAD_3 "irradiance_02"

#define GROUP_PTS_LABEL "gr_pts"
#define GROUP_PTS_CHANNELS                              \
    (const RBRGen4Label[]) \
    {                                                   \
        PRESSURE,                                       \
            TEMPERATURE,                                \
            SALINITY_DYNCORR                            \
    }

#define GROUP_ODO_LABEL "gr_odo"
#define GROUP_ODO_CHANNELS                              \
    (const RBRGen4Label[]) \
    {                                                   \
        PRESSURE,                                       \
            ODO_CONCENTRATION,                          \
            ODO_TEMP                                    \
    }

#define GROUP_PH_LABEL "gr_ph"
#define GROUP_PH_CHANNELS                               \
    (const RBRGen4Label[]) \
    {                                                   \
        PRESSURE,                                       \
            PH                                          \
    }

#define GROUP_BBPFL_LABEL "gr_bbpfl"
#define GROUP_BBPFL_CHANNELS                            \
    (const RBRGen4Label[]) \
    {                                                   \
        PRESSURE,                                       \
            BACKSCATTER,                                \
            CHLOROPHYLL,                                \
            FDOM                                        \
    }

#define GROUP_RADIOMETRY_LABEL "gr_radiometry"
#define GROUP_RADIOMETRY_CHANNELS                       \
    (const RBRGen4Label[]) \
    {                                                   \
        PRESSURE,                                       \
            PAR,                                        \
            QUAD_1,                                     \
            QUAD_2,                                     \
            QUAD_3                                      \
    }

#define SCHEDULE_PTS_LABEL "sch_asc_pts"
#define SCHEDULE_PTS_GROUPS                          \
    (const RBRGen4Label[]) \
    {                                                \
        GROUP_PTS_LABEL                              \
    }
#define SCHEDULE_PTS_MODE RBRGEN4_SCHEDULE_MODE_REGIMES
#define SCHEDULE_PTS_REF RBRGEN4_REFERENCE_SEAPRESSURE
#define SCHEDULE_PTS_DIR RBRGEN4_DIRECTION_ASCENDING
#define SCHEDULE_PTS_COUNT 3
#define SCHEDULE_PTS_REGIME            \
    (RBRGen4Regimes)         \
    {                                  \
        .direction = SCHEDULE_PTS_DIR, \
        .count = SCHEDULE_PTS_COUNT,   \
        .reference = SCHEDULE_PTS_REF, \
        .boundary1 = 2000,             \
        .binSize1 = 10,                \
        .period1 = 1000,               \
        .boundary2 = 1000,             \
        .binSize2 = 1,                 \
        .period2 = 1000,               \
        .boundary3 = 50,               \
        .binSize3 = 0,                 \
        .period3 = 1000                \
    }

#define SCHEDULE_ODO_LABEL "sch_asc_odo"
#define SCHEDULE_ODO_GROUPS                          \
    (const RBRGen4Label[]) \
    {                                                \
        GROUP_ODO_LABEL                              \
    }
#define SCHEDULE_ODO_MODE RBRGEN4_SCHEDULE_MODE_REGIMES
#define SCHEDULE_ODO_REF RBRGEN4_REFERENCE_SEAPRESSURE
#define SCHEDULE_ODO_DIR RBRGEN4_DIRECTION_ASCENDING
#define SCHEDULE_ODO_COUNT 2
#define SCHEDULE_ODO_REGIME            \
    (RBRGen4Regimes)         \
    {                                  \
        .direction = SCHEDULE_ODO_DIR, \
        .count = SCHEDULE_ODO_COUNT,   \
        .reference = SCHEDULE_ODO_REF, \
        .boundary1 = 1000,             \
        .binSize1 = 10,                \
        .period1 = 20000,              \
        .boundary2 = 250,              \
        .binSize2 = 2,                 \
        .period2 = 5000                \
    }

#define SCHEDULE_PH_LABEL "sch_asc_ph"
#define SCHEDULE_PH_GROUPS                           \
    (const RBRGen4Label[]) \
    {                                                \
        GROUP_PH_LABEL                               \
    }
#define SCHEDULE_PH_MODE RBRGEN4_SCHEDULE_MODE_REGIMES
#define SCHEDULE_PH_REF RBRGEN4_REFERENCE_SEAPRESSURE
#define SCHEDULE_PH_DIR RBRGEN4_DIRECTION_ASCENDING
#define SCHEDULE_PH_COUNT 2
#define SCHEDULE_PH_REGIME            \
    (RBRGen4Regimes)        \
    {                                 \
        .direction = SCHEDULE_PH_DIR, \
        .count = SCHEDULE_PH_COUNT,   \
        .reference = SCHEDULE_PH_REF, \
        .boundary1 = 1000,            \
        .binSize1 = 0,                \
        .period1 = 500000,            \
        .boundary2 = 250,             \
        .binSize2 = 0,                \
        .period2 = 20000              \
    }

#define SCHEDULE_BBPFL_LABEL "sch_asc_bbpfl"
#define SCHEDULE_BBPFL_GROUPS                        \
    (const RBRGen4Label[]) \
    {                                                \
        GROUP_BBPFL_LABEL                            \
    }
#define SCHEDULE_BBPFL_MODE RBRGEN4_SCHEDULE_MODE_REGIMES
#define SCHEDULE_BBPFL_REF RBRGEN4_REFERENCE_SEAPRESSURE
#define SCHEDULE_BBPFL_DIR RBRGEN4_DIRECTION_ASCENDING
#define SCHEDULE_BBPFL_COUNT 2
#define SCHEDULE_BBPFL_REGIME            \
    (RBRGen4Regimes)           \
    {                                    \
        .direction = SCHEDULE_BBPFL_DIR, \
        .count = SCHEDULE_BBPFL_COUNT,   \
        .reference = SCHEDULE_BBPFL_REF, \
        .boundary1 = 2000,               \
        .binSize1 = 10,                  \
        .period1 = 10000,                \
        .boundary2 = 250,                \
        .binSize2 = 2,                   \
        .period2 = 500                   \
    }

#define SCHEDULE_RADIOMETRY_LABEL "sch_asc_radiometry"
#define SCHEDULE_RADIOMETRY_GROUPS                   \
    (const RBRGen4Label[]) \
    {                                                \
        GROUP_RADIOMETRY_LABEL                       \
    }
#define SCHEDULE_RADIOMETRY_MODE RBRGEN4_SCHEDULE_MODE_REGIMES
#define SCHEDULE_RADIOMETRY_REF RBRGEN4_REFERENCE_SEAPRESSURE
#define SCHEDULE_RADIOMETRY_DIR RBRGEN4_DIRECTION_ASCENDING
#define SCHEDULE_RADIOMETRY_COUNT 1
#define SCHEDULE_RADIOMETRY_REGIME            \
    (RBRGen4Regimes)                \
    {                                         \
        .direction = SCHEDULE_RADIOMETRY_DIR, \
        .count = SCHEDULE_RADIOMETRY_COUNT,   \
        .reference = SCHEDULE_RADIOMETRY_REF, \
        .boundary1 = 250,                     \
        .binSize1 = 2,                        \
        .period1 = 5000                       \
    }

#define SCHEDULE_PARK_PTS_LABEL "sch_park_pts"
#define SCHEDULE_PARK_PTS_GROUPS                     \
    (const RBRGen4Label[]) \
    {                                                \
        GROUP_PTS_LABEL                              \
    }
#define SCHEDULE_PARK_PTS_MODE RBRGEN4_SCHEDULE_MODE_CONTINUOUS
#define SCHEDULE_PARK_PTS_PERIOD 21600000
#define SCHEDULE_PARK_PTS_CASTDETECTION false

#define SCHEDULE_PARK_ODO_LABEL "sch_park_odo"
#define SCHEDULE_PARK_ODO_GROUPS                     \
    (const RBRGen4Label[]) \
    {                                                \
        GROUP_ODO_LABEL                              \
    }
#define SCHEDULE_PARK_ODO_MODE RBRGEN4_SCHEDULE_MODE_CONTINUOUS
#define SCHEDULE_PARK_ODO_PERIOD 43200000
#define SCHEDULE_PARK_ODO_CASTDETECTION false

#define CONFIG_ASCENT_LABEL "cf_ascent"
#define CONFIG_ASCENT_SCHEDULES                      \
    (const RBRGen4Label[]) \
    {                                                \
        SCHEDULE_PTS_LABEL,                          \
            SCHEDULE_ODO_LABEL,                      \
            SCHEDULE_PH_LABEL,                       \
            SCHEDULE_BBPFL_LABEL,                    \
            SCHEDULE_RADIOMETRY_LABEL                \
    }

#define CONFIG_PARK_LABEL "cf_park"
#define CONFIG_PARK_SCHEDULES                        \
    (const RBRGen4Label[]) \
    {                                                \
        SCHEDULE_PARK_PTS_LABEL,                     \
            SCHEDULE_PARK_ODO_LABEL,                 \
    }

#define STARTTIME "20000101000000"
#define ENDTIME "20991231235959"
#define DATASET_ASCENT_LABEL "ds_ascent"

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
        goto fileCleanup; // Failure case, memory allocated by this constructor is freed.
    }

    //------(optional) get link type: USB/serial/wifi---------------------------------------------
    RBRGen4Link link;
    RBRGen4_getLink(instrument, &link);
    printf("Connected to the instrument via %s.\n",
           RBRGen4LinkType_name(link.type));

    RBRGen4LinkSerial serial;
    RBRGen4WiFi wifi;

    switch (link.type)
    {
    case RBRGEN4_LINK_TYPE_USB:
        break;
    case RBRGEN4_LINK_TYPE_SERIAL:
    {
        RBRGen4_getLinkSerial(instrument, &serial);
        printf("Connected in %s mode at %s baud.\n",
               RBRGen4LinkSerialMode_name(serial.mode),
               RBRGen4LinkSerialBaudRate_name(serial.baudRate));
        break;
    }
    case RBRGEN4_LINK_TYPE_WIFI:
    {
        RBRGen4_getWiFi(instrument, &wifi);
        printf("Connected in WiFi mode at %s baud. Timeout is %d\n",
               RBRGen4LinkSerialBaudRate_name(wifi.baudRate),
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
    RBRGen4DeploymentStatus deploymentStatus = RBRGEN4_STATUS_UNKNOWN;
    RBRGen4_disable(instrument, deploymentStatus);

    RBRGen4DatasetPool datasetPool;
    RBRGen4_getDatasetPool(instrument, &datasetPool);
    RBRGen4_deleteDatasetAll(instrument, &datasetPool);

    RBRGen4Configs configPool;
    RBRGen4_getConfigs(instrument, &configPool);
    RBRGen4_deleteConfigAll(instrument, &configPool);

    RBRGen4SchedulePool schedulePool;
    RBRGen4_getSchedulePool(instrument, &schedulePool);
    RBRGen4_deleteScheduleAll(instrument, &schedulePool);

    RBRGen4GroupPool groupPool;
    RBRGen4_getGroupPool(instrument, &groupPool);
    RBRGen4_deleteGroupAll(instrument, &groupPool);

    /************ group definition ************/
    // populate all channelPool and calibrations
    RBRGen4ChannelPool channelPool;
    RBRGen4_getChannelPool(instrument, &channelPool);

    // specify groupLabel, channel labels, and create group instance
    RBRGen4Group group_pts;
    RBRGen4_initNewGroup(instrument,
                        GROUP_PTS_LABEL,
                        GROUP_PTS_CHANNELS,
                        &channelPool,
                        &group_pts,
                        &groupPool); // warning: need to read error!!!

    RBRGen4Group group_odo;
    RBRGen4_initNewGroup(instrument,
                        GROUP_ODO_LABEL,
                        GROUP_ODO_CHANNELS,
                        &channelPool,
                        &group_odo,
                        &groupPool); // warning: need to read error!!!

    RBRGen4Group group_ph;
    RBRGen4_initNewGroup(instrument,
                        GROUP_PH_LABEL,
                        GROUP_PH_CHANNELS,
                        &channelPool,
                        &group_ph,
                        &groupPool); // warning: need to read error!!!

    RBRGen4Group group_bbpfl;
    RBRGen4_initNewGroup(instrument,
                        GROUP_BBPFL_LABEL,
                        GROUP_BBPFL_CHANNELS,
                        &channelPool,
                        &group_bbpfl,
                        &groupPool); // warning: need to read error!!!

    RBRGen4Group group_radiometry;
    RBRGen4_initNewGroup(instrument,
                        GROUP_RADIOMETRY_LABEL,
                        GROUP_RADIOMETRY_CHANNELS,
                        &channelPool,
                        &group_radiometry,
                        &groupPool); // warning: need to read error!!!

    /************ schedule definition ************/
    RBRGen4Schedule schedule_pts;
    RBRGen4_initNewScheduleRegimes(instrument,
                          SCHEDULE_PTS_LABEL,
                          SCHEDULE_PTS_GROUPS,
                          SCHEDULE_PTS_MODE,
                          &groupPool,
                          SCHEDULE_PTS_REGIME,
                          &schedule_pts,
                          &schedulePool);
    RBRGen4_setSchedule(instrument, &schedule_pts);

    RBRGen4Schedule schedule_odo;
    RBRGen4_initNewScheduleRegimes(instrument,
                          SCHEDULE_ODO_LABEL,
                          SCHEDULE_ODO_GROUPS,
                          SCHEDULE_ODO_MODE,
                          &groupPool,
                          SCHEDULE_ODO_REGIME,
                          &schedule_odo,
                          &schedulePool);
    RBRGen4_setSchedule(instrument, &schedule_odo);

    RBRGen4Schedule schedule_ph;
    RBRGen4_initNewScheduleRegimes(instrument,
                          SCHEDULE_PH_LABEL,
                          SCHEDULE_PH_GROUPS,
                          SCHEDULE_PH_MODE,
                          &groupPool,
                          SCHEDULE_PH_REGIME,
                          &schedule_ph,
                          &schedulePool);
    RBRGen4_setSchedule(instrument, &schedule_ph);

    RBRGen4Schedule schedule_BBPFL;
    RBRGen4_initNewScheduleRegimes(instrument,
                          SCHEDULE_BBPFL_LABEL,
                          SCHEDULE_BBPFL_GROUPS,
                          SCHEDULE_BBPFL_MODE,
                          &groupPool,
                          SCHEDULE_BBPFL_REGIME,
                          &schedule_BBPFL,
                          &schedulePool);
    RBRGen4_setSchedule(instrument, &schedule_BBPFL);

    RBRGen4Schedule schedule_radiometry;
    RBRGen4_initNewScheduleRegimes(instrument,
                          SCHEDULE_RADIOMETRY_LABEL,
                          SCHEDULE_RADIOMETRY_GROUPS,
                          SCHEDULE_RADIOMETRY_MODE,
                          &groupPool,
                          SCHEDULE_RADIOMETRY_REGIME,
                          &schedule_radiometry,
                          &schedulePool);
    RBRGen4_setSchedule(instrument, &schedule_radiometry);

    RBRGen4Schedule schedule_pts_park;
    RBRGen4_initNewScheduleContinuous(instrument,
                             SCHEDULE_PARK_PTS_LABEL,
                             SCHEDULE_PTS_GROUPS,
                             SCHEDULE_PARK_PTS_MODE,
                             &groupPool,
                             SCHEDULE_PARK_PTS_PERIOD,
                             SCHEDULE_PARK_PTS_CASTDETECTION,
                             &schedule_pts_park,
                             &schedulePool);
    RBRGen4_setSchedule(instrument, &schedule_pts_park);

    RBRGen4Schedule schedule_park_odo;
    RBRGen4_initNewScheduleContinuous(instrument,
                             SCHEDULE_PARK_ODO_LABEL,
                             SCHEDULE_PARK_ODO_GROUPS,
                             SCHEDULE_PARK_ODO_MODE,
                             &groupPool,
                             SCHEDULE_PARK_ODO_PERIOD,
                             SCHEDULE_PARK_ODO_CASTDETECTION,
                             &schedule_park_odo,
                             &schedulePool);
    RBRGen4_setSchedule(instrument, &schedule_park_odo);

    /************ configuration definition ************/
    RBRGen4Config config_ascent;
    RBRGen4_initNewConfig(instrument,
                         CONFIG_ASCENT_LABEL,
                         CONFIG_ASCENT_SCHEDULES,
                         &schedulePool,
                         &config_ascent,
                         &configPool);

    RBRGen4Config config_park;
    RBRGen4_initNewConfig(instrument,
                         CONFIG_PARK_LABEL,
                         CONFIG_PARK_SCHEDULES,
                         &schedulePool,
                         &config_park,
                         &configPool);

    /************ deployment parameters ************/
    RBRGen4Deployment deployment;
    RBRGen4_getDeployment(instrument, &deployment);
    // GEN4 TODO: if it's pending, could we modify parameters???
    if (deployment.status == RBRGEN4_STATUS_LOGGING || deployment.status == RBRGEN4_STATUS_PENDING)
    {
        printf("%s: Instrument is logging/pending. I'm going to disable it first.\n",
               programName);
        RBRGen4_disable(instrument, deploymentStatus);
    }
    str_to_deploymentDatetime(&deployment.startTime, STARTTIME);
    str_to_deploymentDatetime(&deployment.endTime, ENDTIME);
    // printf("deployment starttime:%" PRId64 "\n", deployment.starttime);
    RBRGen4_setDeployment(instrument, &deployment);

    /************ start of ascent ************/
    // ensures the memory is cleared first
    RBRGen4_deleteDatasetAll(instrument, &datasetPool);

    // verify the configurations for enable
    RBRGen4_verify(instrument,
                             &config_ascent,
                             DATASET_ASCENT_LABEL,
                             RBRGEN4_STORAGEMODE_NORMAL,
                             deploymentStatus);

    // enable the instrument
    RBRGen4_enable(instrument,
                             &config_ascent,
                             DATASET_ASCENT_LABEL,
                             RBRGEN4_STORAGEMODE_NORMAL,
                             deploymentStatus);

instrumentCleanup:
    RBRGen4_close(instrument);
fileCleanup:
    close(instrumentFd);
    return status;
}
