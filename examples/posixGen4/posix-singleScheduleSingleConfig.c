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

#include "RBRGen4.h"
#include "RBRGen4Configuration.h"
#include "RBRInstrumentGen4Memory.h"
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
            TEMPERATURE,                                \
            SALINITY_DYNCORR                            \
    }
#define GROUP_PTS_CHANNEL_COUNT 3

#define SCHEDULE_PTS_LABEL "sch_asc_pts"
#define SCHEDULE_PTS_MODE RBRGEN4_SCHEDULE_MODE_CONTINUOUS
#define SCHEDULE_PTS_PERIOD 1000
#define SCHEDULE_PTS_CASTDETECTION false

#define SCHEDULE_PTS_GROUPS                          \
    (const RBRGen4Label[]) \
    {                                                \
        GROUP_PTS_LABEL                              \
    }
#define SCHEDULE_PTS_GROUP_COUNT 1

#define CONFIG_ASCENT_LABEL "cf_ascent"
#define CONFIG_ASCENT_SCHEDULES                      \
    (const RBRGen4Label[]) \
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
    default:
        fprintf(stderr,
                "Warning: connection method to the instrument is unclear, so"
                " poll can't be executed.\n");
        goto instrumentCleanup;
    }

    /************ ensure default state ************/
    RBRInstrumentGen4InstrumentState loggingState
        = RBRINSTRUMENTGEN4_UNKNOWN_INSTRUMENT_STATE;
    RBRGen4_disable(instrument, &loggingState);

    RBRInstrumentGen4_deleteDatasetAll(instrument);

    RBRGen4_deleteConfigAll(instrument);

    RBRGen4_deleteScheduleAll(instrument);
    RBRGen4_deleteGroupAll(instrument);

    /************ group definition ************/
    /* read the channel pool, then each channel and its calibration */
    RBRGen4Channel channelBuf[RBRGEN4_CHANNEL_MAX];
    RBRGen4ChannelPool channelPool = {
        .size = RBRGEN4_CHANNEL_MAX,
        .pool = channelBuf
    };
    RBRGen4_getChannelPool(instrument, &channelPool);

    /* Only min(count, size) channels are stored when the instrument reports
     * more than the buffer holds. */
    int32_t channelCount = channelPool.count < channelPool.size
        ? channelPool.count : channelPool.size;
    for (int32_t i = 0; i < channelCount; i++)
    {
        RBRGen4Channel *channel = &channelPool.pool[i];
        RBRGen4_getChannel(instrument, channel);
        printf(
            "%s,%s,%d,%d,%d,%s,%s,%s,%u",
            channel->label,
            channel->type,
            channel->settlingTime,
            channel->measuringTime,
            channel->readOutTime,
            channel->userUnits,
            RBRGen4ChannelNature_name(channel->nature),
            channel->device,
            channel->derived
        );

        RBRGen4Calibration calibration;
        snprintf(calibration.label,
                 sizeof(calibration.label),
                 "%s",
                 channel->label);
        RBRGen4_getCalibration(instrument, &calibration);
    }

    /* specify groupLabel, channel labels, and create group instance */
    RBRGen4Group group_pts;

    RBRInstrumentGen4_initNewGroup(instrument,
                        GROUP_PTS_LABEL,
                        GROUP_PTS_CHANNELS,
                        GROUP_PTS_CHANNEL_COUNT,
                        &channelPool,
                        &group_pts);

    /************ schedule definition ************/
    RBRGen4Schedule schedule;
    RBRGen4Label groupLabelBuf[SCHEDULE_PTS_GROUP_COUNT];
    RBRGen4LabelList groupList = {
        .size = SCHEDULE_PTS_GROUP_COUNT,
        .labels = groupLabelBuf
    };
    RBRInstrumentGen4_initNewScheduleContinuous(instrument,
                         SCHEDULE_PTS_LABEL,
                         SCHEDULE_PTS_GROUPS,
                         SCHEDULE_PTS_GROUP_COUNT,
                         SCHEDULE_PTS_MODE,
                         SCHEDULE_PTS_PERIOD,
                         SCHEDULE_PTS_CASTDETECTION,
                         &groupList,
                         &schedule);

    /************ configuration definition ************/
    RBRGen4Config config;
    RBRGen4Label scheduleLabelBuf[CONFIG_ASCENT_SCHEDULE_COUNT];
    RBRGen4LabelList scheduleList = {
        .size = CONFIG_ASCENT_SCHEDULE_COUNT,
        .labels = scheduleLabelBuf
    };
    RBRInstrumentGen4_initNewConfig(instrument,
                        CONFIG_ASCENT_LABEL,
                        CONFIG_ASCENT_SCHEDULES,
                        CONFIG_ASCENT_SCHEDULE_COUNT,
                        &scheduleList,
                        &config);

    /************ deployment parameters ************/
    RBRGen4Deployment deployment;
    RBRGen4_getDeployment(instrument, &deployment);

    str_to_deploymentDatetime(&deployment.startTime, STARTTIME);
    RBRGen4_setDeployment(instrument, &deployment);

    /************ start of ascent ************/
    /* verify the configurations for enable */
    RBRGen4_verify(instrument,
                             &config,
                             NEW_DATASET_LABEL,
                             RBRGEN4_STORAGEMODE_NORMAL,
                             &loggingState);

    /* enable the instrument */
    RBRGen4_enable(instrument,
                             &config,
                             NEW_DATASET_LABEL,
                             RBRGEN4_STORAGEMODE_NORMAL,
                             &loggingState);

instrumentCleanup:
    RBRGen4_close(instrument);

fileCleanup:
    close(instrumentFd);

    return status;
}
