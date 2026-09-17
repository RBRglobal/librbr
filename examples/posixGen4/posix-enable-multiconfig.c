/**
 * \file posix-enable-multiconfig.c
 *
 * \brief Example of using the library to configure an instrument with several
 * schedules shared between two configurations: a fast "ascent" configuration
 * and a slow "park" configuration, as a profiling float might use.
 *
 * Both configurations are defined; the instrument is enabled with the ascent
 * configuration. Re-enabling with the park configuration only requires a
 * different label in RBRGen4_enable().
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

/* Tune these labels to your instrument. This example is intended to work with
 * any instrument that has pressure and temperature channels. */
#define PRESSURE    "pressure_00"
#define TEMPERATURE "temperature_00"

#define GROUP_PT_LABEL "gr_pt"
#define GROUP_PT_CHANNELS     \
    (RBRGen4Label[])          \
    {                         \
        PRESSURE, TEMPERATURE \
    }
#define GROUP_PT_CHANNEL_COUNT 2

#define GROUP_P_LABEL "gr_p"
#define GROUP_P_CHANNELS \
    (RBRGen4Label[])     \
    {                    \
        PRESSURE         \
    }
#define GROUP_P_CHANNEL_COUNT 1

/* Each schedule samples one group. The ascent schedules sample quickly, the
 * park schedules slowly. Periods are in milliseconds. */
#define SCHEDULE_ASC_PT_LABEL   "sch_asc_pt"
#define SCHEDULE_ASC_PT_PERIOD  1000
#define SCHEDULE_ASC_P_LABEL    "sch_asc_p"
#define SCHEDULE_ASC_P_PERIOD   5000
#define SCHEDULE_PARK_PT_LABEL  "sch_park_pt"
#define SCHEDULE_PARK_PT_PERIOD (6 * 60 * 60 * 1000)
#define SCHEDULE_PARK_P_LABEL   "sch_park_p"
#define SCHEDULE_PARK_P_PERIOD  (12 * 60 * 60 * 1000)

#define CONFIG_ASCENT_LABEL "cf_ascent"
#define CONFIG_ASCENT_SCHEDULES                     \
    (RBRGen4Label[])                                \
    {                                               \
        SCHEDULE_ASC_PT_LABEL, SCHEDULE_ASC_P_LABEL \
    }
#define CONFIG_ASCENT_SCHEDULE_COUNT 2

#define CONFIG_PARK_LABEL "cf_park"
#define CONFIG_PARK_SCHEDULES                         \
    (RBRGen4Label[])                                  \
    {                                                 \
        SCHEDULE_PARK_PT_LABEL, SCHEDULE_PARK_P_LABEL \
    }
#define CONFIG_PARK_SCHEDULE_COUNT 2

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

/* Create a group and give it its channels. */
RBRGen4Error createGroup(RBRGen4 *conn, const char *label, RBRGen4Label channels[],
                         int32_t channelCount)
{
    RBRGen4Error err = RBRGen4_createGroup(conn, label);
    if (err) {
        logCmdError(conn, err, "Failed to create group");
        return err;
    }

    RBRGen4Group group;
    snprintf(group.label, sizeof(group.label), "%s", label);
    const RBRGen4LabelList channelList = {
        .size = channelCount,
        .count = channelCount,
        .labels = channels,
    };
    err = RBRGen4_setGroup(conn, &group, &channelList);
    if (err) {
        logCmdError(conn, err, "Failed to set new group");
    }
    return err;
}

/* Create a continuous schedule sampling one group at the given period. The
 * remaining parameters are left at the instrument's defaults. */
RBRGen4Error createContinuousSchedule(RBRGen4 *conn, const char *label, const char *groupLabel,
                                      RBRGen4Period period)
{
    RBRGen4Error err = RBRGen4_createSchedule(conn, label);
    if (err) {
        logCmdError(conn, err, "Failed to create new schedule");
        return err;
    }

    /* Get default instrument schedule parameters */
    RBRGen4Schedule schedule;
    snprintf(schedule.label, sizeof(schedule.label), "%s", label);
    err = RBRGen4_getSchedule(conn, &schedule, NULL);
    if (err) {
        logCmdError(conn, err, "Failed to get new schedule");
        return err;
    }

    /* Set desired instrument schedule parameters */
    schedule.mode = RBRGEN4_SCHEDULE_MODE_CONTINUOUS;
    schedule.parameters.continuous.period = period;
    RBRGen4Label groups[1];
    snprintf(groups[0], sizeof(groups[0]), "%s", groupLabel);
    const RBRGen4LabelList groupList = {
        .size = 1,
        .count = 1,
        .labels = groups,
    };
    err = RBRGen4_setSchedule(conn, &schedule, &groupList);
    if (err) {
        logCmdError(conn, err, "Failed to set new schedule");
    }
    return err;
}

/* Create a config and give it its schedules. */
RBRGen4Error createConfig(RBRGen4 *conn, const char *label, RBRGen4Label schedules[],
                          int32_t scheduleCount)
{
    RBRGen4Error err = RBRGen4_createConfig(conn, label);
    if (err) {
        logCmdError(conn, err, "Failed to create new config");
        return err;
    }

    RBRGen4Config config;
    snprintf(config.label, sizeof(config.label), "%s", label);
    const RBRGen4LabelList scheduleList = {
        .size = scheduleCount,
        .count = scheduleCount,
        .labels = schedules,
    };
    err = RBRGen4_setConfig(conn, &config, &scheduleList);
    if (err) {
        logCmdError(conn, err, "Failed to set new config");
    }
    return err;
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

    /* Create the groups */
    err = createGroup(&conn, GROUP_PT_LABEL, GROUP_PT_CHANNELS, GROUP_PT_CHANNEL_COUNT);
    if (err) {
        goto instrumentCleanup;
    }
    err = createGroup(&conn, GROUP_P_LABEL, GROUP_P_CHANNELS, GROUP_P_CHANNEL_COUNT);
    if (err) {
        goto instrumentCleanup;
    }

    /* Create the schedules: two fast for the ascent, two slow for parking */
    err = createContinuousSchedule(
        &conn, SCHEDULE_ASC_PT_LABEL, GROUP_PT_LABEL, SCHEDULE_ASC_PT_PERIOD);
    if (err) {
        goto instrumentCleanup;
    }
    err =
        createContinuousSchedule(&conn, SCHEDULE_ASC_P_LABEL, GROUP_P_LABEL, SCHEDULE_ASC_P_PERIOD);
    if (err) {
        goto instrumentCleanup;
    }
    err = createContinuousSchedule(
        &conn, SCHEDULE_PARK_PT_LABEL, GROUP_PT_LABEL, SCHEDULE_PARK_PT_PERIOD);
    if (err) {
        goto instrumentCleanup;
    }
    err = createContinuousSchedule(
        &conn, SCHEDULE_PARK_P_LABEL, GROUP_P_LABEL, SCHEDULE_PARK_P_PERIOD);
    if (err) {
        goto instrumentCleanup;
    }

    /* Create the configs */
    err = createConfig(
        &conn, CONFIG_ASCENT_LABEL, CONFIG_ASCENT_SCHEDULES, CONFIG_ASCENT_SCHEDULE_COUNT);
    if (err) {
        goto instrumentCleanup;
    }
    err = createConfig(&conn, CONFIG_PARK_LABEL, CONFIG_PARK_SCHEDULES, CONFIG_PARK_SCHEDULE_COUNT);
    if (err) {
        goto instrumentCleanup;
    }

    /* Start sampling as soon as the instrument is enabled */
    RBRGen4Deployment deployment;
    err = RBRGen4_getDeployment(&conn, &deployment);
    if (err) {
        logCmdError(&conn, err, "Failed to get deployment");
        goto instrumentCleanup;
    }
    deployment.gate = RBRGEN4_GATE_NONE;
    err = RBRGen4_setDeployment(&conn, &deployment);
    if (err) {
        logCmdError(&conn, err, "Failed to set deployment");
        goto instrumentCleanup;
    }

    /* Verify both configurations for enablement */
    const RBRGen4Config configAscent = {
        .label = CONFIG_ASCENT_LABEL,
    };
    const RBRGen4Config configPark = {
        .label = CONFIG_PARK_LABEL,
    };
    err = RBRGen4_verify(
        &conn, &configAscent, NEW_DATASET_LABEL, RBRGEN4_STORAGE_MODE_NORMAL, &loggingState);
    if (err) {
        logCmdError(&conn, err, "Failed to verify ascent configuration");
        goto instrumentCleanup;
    }
    err = RBRGen4_verify(
        &conn, &configPark, NEW_DATASET_LABEL, RBRGEN4_STORAGE_MODE_NORMAL, &loggingState);
    if (err) {
        logCmdError(&conn, err, "Failed to verify park configuration");
        goto instrumentCleanup;
    }
    printf("%s: Both instrument configurations verified\n", programName);

    /* Enable the instrument with the ascent configuration */
    err = RBRGen4_enable(
        &conn, &configAscent, NEW_DATASET_LABEL, RBRGEN4_STORAGE_MODE_NORMAL, &loggingState);
    if (err) {
        logCmdError(&conn, err, "Failed to enable instrument");
        goto instrumentCleanup;
    }
    printf("%s: Instrument enablement state is: %s\n",
           programName,
           RBRGen4InstrumentState_name(loggingState));

instrumentCleanup:
    RBRGen4_close(&conn);

fileCleanup:
    close(instrumentFd);

    return (err == RBRGEN4_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
}
