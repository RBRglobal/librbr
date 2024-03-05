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
    (const char[][RBRINSTRUMENTGEN4_CHANNEL_LABEL_MAX]) \
    {                                                   \
        PRESSURE,                                       \
            TEMPERATURE,                                \
            SALINITY_DYNCORR                            \
    }

#define GROUP_ODO_LABEL "gr_odo"
#define GROUP_ODO_CHANNELS                              \
    (const char[][RBRINSTRUMENTGEN4_CHANNEL_LABEL_MAX]) \
    {                                                   \
        PRESSURE,                                       \
            ODO_CONCENTRATION,                          \
            ODO_TEMP                                    \
    }

#define GROUP_PH_LABEL "gr_ph"
#define GROUP_PH_CHANNELS                               \
    (const char[][RBRINSTRUMENTGEN4_CHANNEL_LABEL_MAX]) \
    {                                                   \
        PRESSURE,                                       \
            PH                                          \
    }

#define GROUP_BBPFL_LABEL "gr_bbpfl"
#define GROUP_BBPFL_CHANNELS                            \
    (const char[][RBRINSTRUMENTGEN4_CHANNEL_LABEL_MAX]) \
    {                                                   \
        PRESSURE,                                       \
            BACKSCATTER,                                \
            CHLOROPHYLL,                                \
            FDOM                                        \
    }

#define GROUP_RADIOMETRY_LABEL "gr_radiometry"
#define GROUP_RADIOMETRY_CHANNELS                       \
    (const char[][RBRINSTRUMENTGEN4_CHANNEL_LABEL_MAX]) \
    {                                                   \
        PRESSURE,                                       \
            PAR,                                        \
            QUAD_1,                                     \
            QUAD_2,                                     \
            QUAD_3                                      \
    }

#define SCHEDULE_PTS_LABEL "sch_asc_pts"
#define SCHEDULE_PTS_GROUPS                          \
    (const char[][RBRINSTRUMENTGEN4_LABEL_NAME_MAX]) \
    {                                                \
        GROUP_PTS_LABEL                              \
    }
#define SCHEDULE_PTS_MODE RBRINSTRUMENTGEN4_SAMPLING_REGIMES
#define SCHEDULE_PTS_REF RBRINSTRUMENTGEN4_REFERENCE_SEAPRESSURE
#define SCHEDULE_PTS_DIR RBRINSTRUMENTGEN4_DIRECTION_ASCENDING
#define SCHEDULE_PTS_COUNT 3
#define SCHEDULE_PTS_REGIME            \
    (RBRInstrumentGen4Regimes)         \
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
    (const char[][RBRINSTRUMENTGEN4_LABEL_NAME_MAX]) \
    {                                                \
        GROUP_ODO_LABEL                              \
    }
#define SCHEDULE_ODO_MODE RBRINSTRUMENTGEN4_SAMPLING_REGIMES
#define SCHEDULE_ODO_REF RBRINSTRUMENTGEN4_REFERENCE_SEAPRESSURE
#define SCHEDULE_ODO_DIR RBRINSTRUMENTGEN4_DIRECTION_ASCENDING
#define SCHEDULE_ODO_COUNT 2
#define SCHEDULE_ODO_REGIME            \
    (RBRInstrumentGen4Regimes)         \
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
    (const char[][RBRINSTRUMENTGEN4_LABEL_NAME_MAX]) \
    {                                                \
        GROUP_PH_LABEL                               \
    }
#define SCHEDULE_PH_MODE RBRINSTRUMENTGEN4_SAMPLING_REGIMES
#define SCHEDULE_PH_REF RBRINSTRUMENTGEN4_REFERENCE_SEAPRESSURE
#define SCHEDULE_PH_DIR RBRINSTRUMENTGEN4_DIRECTION_ASCENDING
#define SCHEDULE_PH_COUNT 2
#define SCHEDULE_PH_REGIME            \
    (RBRInstrumentGen4Regimes)        \
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
    (const char[][RBRINSTRUMENTGEN4_LABEL_NAME_MAX]) \
    {                                                \
        GROUP_BBPFL_LABEL                            \
    }
#define SCHEDULE_BBPFL_MODE RBRINSTRUMENTGEN4_SAMPLING_REGIMES
#define SCHEDULE_BBPFL_REF RBRINSTRUMENTGEN4_REFERENCE_SEAPRESSURE
#define SCHEDULE_BBPFL_DIR RBRINSTRUMENTGEN4_DIRECTION_ASCENDING
#define SCHEDULE_BBPFL_COUNT 2
#define SCHEDULE_BBPFL_REGIME            \
    (RBRInstrumentGen4Regimes)           \
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
    (const char[][RBRINSTRUMENTGEN4_LABEL_NAME_MAX]) \
    {                                                \
        GROUP_RADIOMETRY_LABEL                       \
    }
#define SCHEDULE_RADIOMETRY_MODE RBRINSTRUMENTGEN4_SAMPLING_REGIMES
#define SCHEDULE_RADIOMETRY_REF RBRINSTRUMENTGEN4_REFERENCE_SEAPRESSURE
#define SCHEDULE_RADIOMETRY_DIR RBRINSTRUMENTGEN4_DIRECTION_ASCENDING
#define SCHEDULE_RADIOMETRY_COUNT 1
#define SCHEDULE_RADIOMETRY_REGIME            \
    (RBRInstrumentGen4Regimes)                \
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
    (const char[][RBRINSTRUMENTGEN4_LABEL_NAME_MAX]) \
    {                                                \
        GROUP_PTS_LABEL                              \
    }
#define SCHEDULE_PARK_PTS_MODE RBRINSTRUMENTGEN4_SAMPLING_CONTINUOUS
#define SCHEDULE_PARK_PTS_PERIOD 21600000
#define SCHEDULE_PARK_PTS_CASTDETECTION false

#define SCHEDULE_PARK_ODO_LABEL "sch_park_odo"
#define SCHEDULE_PARK_ODO_GROUPS                     \
    (const char[][RBRINSTRUMENTGEN4_LABEL_NAME_MAX]) \
    {                                                \
        GROUP_ODO_LABEL                              \
    }
#define SCHEDULE_PARK_ODO_MODE RBRINSTRUMENTGEN4_SAMPLING_CONTINUOUS
#define SCHEDULE_PARK_ODO_PERIOD 43200000
#define SCHEDULE_PARK_ODO_CASTDETECTION false

#define CONFIG_ASCENT_LABEL "cf_ascent"
#define CONFIG_ASCENT_SCHEDULES                      \
    (const char[][RBRINSTRUMENTGEN4_LABEL_NAME_MAX]) \
    {                                                \
        SCHEDULE_PTS_LABEL,                          \
            SCHEDULE_ODO_LABEL,                      \
            SCHEDULE_PH_LABEL,                       \
            SCHEDULE_BBPFL_LABEL,                    \
            SCHEDULE_RADIOMETRY_LABEL                \
    }

#define CONFIG_PARK_LABEL "cf_park"
#define CONFIG_PARK_SCHEDULES                        \
    (const char[][RBRINSTRUMENTGEN4_LABEL_NAME_MAX]) \
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

    RBRInstrumentGen4Group group_odo;
    init_groupStructure(instrument,
                        GROUP_ODO_LABEL,
                        GROUP_ODO_CHANNELS,
                        &channels,
                        &group_odo,
                        &groups); // warning: need to read error!!!

    RBRInstrumentGen4Group group_ph;
    init_groupStructure(instrument,
                        GROUP_PH_LABEL,
                        GROUP_PH_CHANNELS,
                        &channels,
                        &group_ph,
                        &groups); // warning: need to read error!!!

    RBRInstrumentGen4Group group_bbpfl;
    init_groupStructure(instrument,
                        GROUP_BBPFL_LABEL,
                        GROUP_BBPFL_CHANNELS,
                        &channels,
                        &group_bbpfl,
                        &groups); // warning: need to read error!!!

    RBRInstrumentGen4Group group_radiometry;
    init_groupStructure(instrument,
                        GROUP_RADIOMETRY_LABEL,
                        GROUP_RADIOMETRY_CHANNELS,
                        &channels,
                        &group_radiometry,
                        &groups); // warning: need to read error!!!

    /************ schedule definition ************/
    RBRInstrumentGen4Schedule schedule_pts;
    init_schedule_regimes(instrument,
                          SCHEDULE_PTS_LABEL,
                          SCHEDULE_PTS_GROUPS,
                          SCHEDULE_PTS_MODE,
                          &groups,
                          SCHEDULE_PTS_REGIME,
                          &schedule_pts,
                          &schedules);
    RBRInstrumentGen4_setSchedule(instrument, &schedule_pts);

    RBRInstrumentGen4Schedule schedule_odo;
    init_schedule_regimes(instrument,
                          SCHEDULE_ODO_LABEL,
                          SCHEDULE_ODO_GROUPS,
                          SCHEDULE_ODO_MODE,
                          &groups,
                          SCHEDULE_ODO_REGIME,
                          &schedule_odo,
                          &schedules);
    RBRInstrumentGen4_setSchedule(instrument, &schedule_odo);

    RBRInstrumentGen4Schedule schedule_ph;
    init_schedule_regimes(instrument,
                          SCHEDULE_PH_LABEL,
                          SCHEDULE_PH_GROUPS,
                          SCHEDULE_PH_MODE,
                          &groups,
                          SCHEDULE_PH_REGIME,
                          &schedule_ph,
                          &schedules);
    RBRInstrumentGen4_setSchedule(instrument, &schedule_ph);

    RBRInstrumentGen4Schedule schedule_BBPFL;
    init_schedule_regimes(instrument,
                          SCHEDULE_BBPFL_LABEL,
                          SCHEDULE_BBPFL_GROUPS,
                          SCHEDULE_BBPFL_MODE,
                          &groups,
                          SCHEDULE_BBPFL_REGIME,
                          &schedule_BBPFL,
                          &schedules);
    RBRInstrumentGen4_setSchedule(instrument, &schedule_BBPFL);

    RBRInstrumentGen4Schedule schedule_radiometry;
    init_schedule_regimes(instrument,
                          SCHEDULE_RADIOMETRY_LABEL,
                          SCHEDULE_RADIOMETRY_GROUPS,
                          SCHEDULE_RADIOMETRY_MODE,
                          &groups,
                          SCHEDULE_RADIOMETRY_REGIME,
                          &schedule_radiometry,
                          &schedules);
    RBRInstrumentGen4_setSchedule(instrument, &schedule_radiometry);

    RBRInstrumentGen4Schedule schedule_pts_park;
    init_schedule_continuous(instrument,
                             SCHEDULE_PARK_PTS_LABEL,
                             SCHEDULE_PTS_GROUPS,
                             SCHEDULE_PARK_PTS_MODE,
                             &groups,
                             SCHEDULE_PARK_PTS_PERIOD,
                             SCHEDULE_PARK_PTS_CASTDETECTION,
                             &schedule_pts_park,
                             &schedules);
    RBRInstrumentGen4_setSchedule(instrument, &schedule_pts_park);

    RBRInstrumentGen4Schedule schedule_park_odo;
    init_schedule_continuous(instrument,
                             SCHEDULE_PARK_ODO_LABEL,
                             SCHEDULE_PARK_ODO_GROUPS,
                             SCHEDULE_PARK_ODO_MODE,
                             &groups,
                             SCHEDULE_PARK_ODO_PERIOD,
                             SCHEDULE_PARK_ODO_CASTDETECTION,
                             &schedule_park_odo,
                             &schedules);
    RBRInstrumentGen4_setSchedule(instrument, &schedule_park_odo);

    /************ configuration definition ************/
    RBRInstrumentGen4Config config_ascent;
    init_configStructure(instrument,
                         CONFIG_ASCENT_LABEL,
                         CONFIG_ASCENT_SCHEDULES,
                         &schedules,
                         &config_ascent,
                         &configs);

    RBRInstrumentGen4Config config_park;
    init_configStructure(instrument,
                         CONFIG_PARK_LABEL,
                         CONFIG_PARK_SCHEDULES,
                         &schedules,
                         &config_park,
                         &configs);

    /************ deployment parameters ************/
    RBRInstrumentGen4Deployment deployment;
    RBRInstrumentGen4_getDeployment(instrument, &deployment);
    // GEN4 TODO: if it's pending, could we modify parameters???
    if (deployment.status == RBRINSTRUMENTGEN4_STATUS_LOGGING || deployment.status == RBRINSTRUMENTGEN4_STATUS_PENDING)
    {
        printf("%s: Instrument is logging/pending. I'm going to disable it first.\n",
               programName);
        RBRInstrumentGen4_disable(instrument, deploymentStatus);
    }
    str_to_deploymentDatetime(&deployment.startTime, STARTTIME);
    str_to_deploymentDatetime(&deployment.endTime, ENDTIME);
    // printf("deployment starttime:%" PRId64 "\n", deployment.starttime);
    RBRInstrumentGen4_setDeployment(instrument, &deployment);

    /************ start of ascent ************/
    // ensures the memory is cleared first
    RBRInstrumentGen4_deleteDatasetAll(instrument, &datasets);

    // verify the configurations for enable
    RBRInstrumentGen4_verify(instrument,
                             &config_ascent,
                             DATASET_ASCENT_LABEL,
                             deploymentStatus);

    // enable the instrument
    RBRInstrumentGen4_enable(instrument,
                             &config_ascent,
                             DATASET_ASCENT_LABEL,
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
