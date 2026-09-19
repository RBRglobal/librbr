/**
 * \file posix-enable.c
 *
 * \brief Example of using the library to configure an instrument with a single
 * schedule and a single configuration measuring temperature and pressure.
 *
 * \warning Clears all groups, schedules, configs, and datasets, then tries to
 * enable the instrument.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for errno. */
#include <errno.h>
/* Required for PRId32. */
#include <inttypes.h>
/* Required for fprintf, printf, snprintf. */
#include <stdio.h>
/* Required for EXIT_SUCCESS, etc. */
#include <stdlib.h>
/* Required for strerror. */
#include <string.h>
/* Required for close. */
#include <unistd.h>

#include "RBRGen4.h"
#include "posix-shared.h"

/* == Customer defined parameters == */

/* Tune this value to your instrument. This example is intended to work with
 * any instrument that has pressure and temperature channels, so a large
 * channel count is used to be as compatible as possible off-the-shelf. */
#define CHANNEL_COUNT 32

#define PRESSURE    "pressure_00"
#define TEMPERATURE "temperature_00"

#define GROUP_PT_LABEL "gr_pt"
#define GROUP_PT_CHANNELS     \
    (RBRGen4Label[])          \
    {                         \
        PRESSURE, TEMPERATURE \
    }
#define GROUP_PT_CHANNEL_COUNT 2

#define SCHEDULE_PT_LABEL  "sch_asc_pt"
#define SCHEDULE_PT_PERIOD 1000

#define SCHEDULE_PT_GROUPS \
    (RBRGen4Label[])       \
    {                      \
        GROUP_PT_LABEL     \
    }
#define SCHEDULE_PT_GROUP_COUNT 1

#define CONFIG_ASCENT_LABEL "cf_ascent"
#define CONFIG_ASCENT_SCHEDULES \
    (RBRGen4Label[])            \
    {                           \
        SCHEDULE_PT_LABEL       \
    }
#define CONFIG_ASCENT_SCHEDULE_COUNT 1

#define START_DELAY_MS ((RBRGen4DateTime) 5000)

#define NEW_DATASET_LABEL "ds_ascent"

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

/* Disable the instrument and ignore the "instrument state is already disabled"
 * warning if received. */
RBRGen4Error disableIgnoreWarning(RBRGen4 *conn, RBRGen4InstrumentState *state)
{
    RBRGen4Error err = RBRGen4_disable(conn, state);
    if (err) {
        const bool isAlreadyDisabledWarning =
            (err == RBRGEN4_HARDWARE_ERROR) &&
            (RBRGen4_getLastHardwareError(conn) ==
             RBRGEN4_HARDWARE_ERROR_INSTRUMENT_STATE_IS_ALREADY_DISABLED);
        if (!isAlreadyDisabledWarning) {
            return err;
        }
    }
    return RBRGEN4_SUCCESS;
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

    RBRGen4Error err = RBRGEN4_SUCCESS;
    RBRGen4 conn;
    const RBRGen4Callbacks callbacks = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite,
    };

    err = RBRGen4_open(&conn, &callbacks, INSTRUMENT_COMMAND_TIMEOUT_MSEC, (void *) &instrumentFd);
    if (err) {
        logCmdError(&conn, err, "Failed to establish instrument connection");
        goto fileCleanup;
    }

    RBRGen4Link link;
    err = RBRGen4_getLink(&conn, &link);
    if (err) {
        logCmdError(&conn, err, "Failed to get link");
        goto instrumentCleanup;
    }
    printf("Connected to the instrument via %s.\n", RBRGen4LinkType_name(link.type));

    switch (link.type) {
    case RBRGEN4_LINK_TYPE_USB:
        // case RBRGEN4_LINK_TYPE_WIFI:
        break;
    case RBRGEN4_LINK_TYPE_SERIAL: {
        RBRGen4LinkSerial serial;
        err = RBRGen4_getLinkSerial(&conn, &serial);
        if (err) {
            logCmdError(&conn, err, "Failed to get link serial");
            goto instrumentCleanup;
        }
        printf("Connected in %s mode at %s baud.\n",
               RBRGen4LinkSerialMode_name(serial.mode),
               RBRGen4LinkSerialBaudRate_name(serial.baudRate));
        break;
    }
    default:
        fprintf(stderr, "Warning: connection method to the instrument is unclear\n");
    }

    /* Ensure instrument is disabled before trying to run Unsafe commands
     * (commands that cannot be run while the instrument is enabled) */
    RBRGen4InstrumentState loggingState;
    err = disableIgnoreWarning(&conn, &loggingState);
    if (err) {
        logCmdError(&conn, err, "Failed to disable instrument");
        goto instrumentCleanup;
    }

    /* Clear any existing configuration state */
    err = RBRGen4_deleteDatasetAll(&conn);
    if (err) {
        logCmdError(&conn,
                    err,
                    "Failed to delete all datasets -- does this instrument"
                    " support storing data?");
        goto instrumentCleanup;
    }
    err = RBRGen4_deleteConfigAll(&conn);
    if (err) {
        logCmdError(&conn, err, "Failed to delete all configs");
        goto instrumentCleanup;
    }
    err = RBRGen4_deleteScheduleAll(&conn);
    if (err) {
        logCmdError(&conn, err, "Failed to delete all schedules");
        goto instrumentCleanup;
    }
    err = RBRGen4_deleteGroupAll(&conn);
    if (err) {
        logCmdError(&conn, err, "Failed to delete all groups");
        goto instrumentCleanup;
    }

    /* Read the channel pool */
    RBRGen4Channel channelPoolBuf[CHANNEL_COUNT];
    RBRGen4ChannelPool channelPool = {
        .size = CHANNEL_COUNT,
        .pool = channelPoolBuf,
    };
    err = RBRGen4_getChannelPool(&conn, &channelPool);
    if (err == RBRGEN4_TRUNCATED) {
        int32_t count;
        err = RBRGen4_getChannelCount(&conn, &count);
        if (err != RBRGEN4_SUCCESS) {
            logCmdError(&conn, err, "Failed to get channel count");
            goto instrumentCleanup;
        }

        printf("%s: Warning: not enough space in channel pool to store all"
               " channels; only the first %" PRId32 " are stored out of the"
               " instrument's %" PRId32 " channels\n",
               programName,
               channelPool.len,
               count);
    } else if (err) {
        logCmdError(&conn, err, "Failed to get channel pool");
        goto instrumentCleanup;
    }

    /* Print the label and type of each channel */
    for (int32_t i = 0; i < channelPool.len; i++) {
        RBRGen4Channel *channel = &channelPool.pool[i];
        err = RBRGen4_getChannel(&conn, channel);
        if (err) {
            logCmdError(&conn, err, "Failed to get channel");
            goto instrumentCleanup;
        }
        printf("Channel %s has type %s\n", channel->label, channel->type);
    }

    /* Create a group with our desired pressure and temperature channels */
    RBRGen4Group groupPt = {
        .label = GROUP_PT_LABEL,
    };
    err = RBRGen4_createGroup(&conn, groupPt.label);
    if (err) {
        logCmdError(&conn, err, "Failed to create group");
        goto instrumentCleanup;
    }
    const RBRGen4LabelList groupPtChannelList = {
        .size = GROUP_PT_CHANNEL_COUNT,
        .len = GROUP_PT_CHANNEL_COUNT,
        .labels = GROUP_PT_CHANNELS,
    };
    err = RBRGen4_setGroup(&conn, &groupPt, &groupPtChannelList);
    if (err) {
        logCmdError(&conn, err, "Failed to set new group");
        goto instrumentCleanup;
    }

    /* Create a schedule with our new group */
    RBRGen4Schedule schedule = {
        .label = SCHEDULE_PT_LABEL,
    };
    err = RBRGen4_createSchedule(&conn, schedule.label);
    if (err) {
        logCmdError(&conn, err, "Failed to create new schedule");
        goto instrumentCleanup;
    }
    /* Get default instrument schedule parameters */
    err = RBRGen4_getSchedule(&conn, &schedule, NULL);
    if (err) {
        logCmdError(&conn, err, "Failed to get new schedule");
        goto instrumentCleanup;
    }
    /* Set desired instrument schedule parameters */
    schedule.mode = RBRGEN4_SCHEDULE_MODE_CONTINUOUS;
    schedule.parameters.continuous.period = SCHEDULE_PT_PERIOD;
    const RBRGen4LabelList scheduleGroupList = {
        .size = SCHEDULE_PT_GROUP_COUNT,
        .len = SCHEDULE_PT_GROUP_COUNT,
        .labels = SCHEDULE_PT_GROUPS,
    };
    err = RBRGen4_setSchedule(&conn, &schedule, &scheduleGroupList);
    if (err) {
        logCmdError(&conn, err, "Failed to set new schedule");
        goto instrumentCleanup;
    }

    /* Create a config with our new schedule */
    RBRGen4Config config = {
        .label = CONFIG_ASCENT_LABEL,
    };
    err = RBRGen4_createConfig(&conn, config.label);
    if (err) {
        logCmdError(&conn, err, "Failed to create new config");
        goto instrumentCleanup;
    }
    const RBRGen4LabelList configScheduleList = {
        .size = CONFIG_ASCENT_SCHEDULE_COUNT,
        .len = CONFIG_ASCENT_SCHEDULE_COUNT,
        .labels = CONFIG_ASCENT_SCHEDULES,
    };
    err = RBRGen4_setConfig(&conn, &config, &configScheduleList);
    if (err) {
        logCmdError(&conn, err, "Failed to set new config");
        goto instrumentCleanup;
    }

    /* Read instrument's clock */
    RBRGen4Clock clock;
    err = RBRGen4_getClock(&conn, &clock);
    if (err) {
        logCmdError(&conn, err, "Failed to get clock");
        goto instrumentCleanup;
    }

    /* Set a deployment start time */
    RBRGen4Deployment deployment;
    err = RBRGen4_getDeployment(&conn, &deployment);
    if (err) {
        logCmdError(&conn, err, "Failed to get deployment");
        goto instrumentCleanup;
    }
    deployment.gate = RBRGEN4_DEPLOYMENT_GATE_TIME;
    deployment.startTime = clock.dateTime + START_DELAY_MS;
    err = RBRGen4_setDeployment(&conn, &deployment);
    if (err) {
        logCmdError(&conn, err, "Failed to set deployment");
        goto instrumentCleanup;
    }

    /* Verify instrument configuration for enablement */
    err = RBRGen4_verify(
        &conn, &config, NEW_DATASET_LABEL, RBRGEN4_STORAGE_MODE_NORMAL, &loggingState);
    if (err) {
        logCmdError(&conn, err, "Failed to verify instrument configuration");
        goto instrumentCleanup;
    }
    printf("%s: Instrument configuration verified\n", programName);

    /* Enable the instrument */
    err = RBRGen4_enable(
        &conn, &config, NEW_DATASET_LABEL, RBRGEN4_STORAGE_MODE_NORMAL, &loggingState);
    if (err) {
        logCmdError(&conn, err, "Failed to enable instrument");
        goto instrumentCleanup;
    }
    printf("%s: Instrument enablement state is: %s\n",
           programName,
           RBRGen4InstrumentState_name(loggingState));

    /* Check deployment status (depending on the value of deployment.startTime
     * set above, we expect to either be gated or sampling). */
    err = RBRGen4_getDeployment(&conn, &deployment);
    if (err) {
        logCmdError(&conn, err, "Failed to get deployment");
        goto instrumentCleanup;
    }
    printf(
        "%s: deployment status=%s\n", programName, RBRGen4DeploymentStatus_name(deployment.status));

instrumentCleanup:
    RBRGen4_close(&conn);

fileCleanup:
    close(instrumentFd);

    return (err == RBRGEN4_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
}
