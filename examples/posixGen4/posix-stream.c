/**
 * \file posix-stream.c
 *
 * \brief Example of using the library to configure an instrument to stream
 * pressure and temperature over the connected link, then receive the
 * streamed samples until interrupted.
 *
 * \warning Clears all groups, schedules, configs, and datasets, then enables
 * the instrument. The instrument is disabled again on exit.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Prerequisite for gmtime_r in time.h. */
#define _POSIX_C_SOURCE 200112L

/* Required for errno. */
#include <errno.h>
/* Required for PRId32, PRId64. */
#include <inttypes.h>
/* Required for signal, sig_atomic_t. */
#include <signal.h>
/* Required for fprintf, printf. */
#include <stdio.h>
/* Required for EXIT_SUCCESS, etc. */
#include <stdlib.h>
/* Required for strerror. */
#include <string.h>
/* Required for gmtime_r, strftime, struct tm, time_t. */
#include <time.h>
/* Required for close. */
#include <unistd.h>

#include "RBRGen4.h"
#include "posix-shared.h"

/* == Customer defined parameters == */

#define PRESSURE    "pressure_00"
#define TEMPERATURE "temperature_00"

#define GROUP_PT_LABEL "gr_pt"
#define GROUP_PT_CHANNELS     \
    (RBRGen4Label[])          \
    {                         \
        PRESSURE, TEMPERATURE \
    }
#define GROUP_PT_CHANNEL_COUNT 2

#define SCHEDULE_PT_LABEL  "sch_stream_pt"
#define SCHEDULE_PT_PERIOD 1000

#define SCHEDULE_PT_GROUPS \
    (RBRGen4Label[])       \
    {                      \
        GROUP_PT_LABEL     \
    }
#define SCHEDULE_PT_GROUP_COUNT 1

#define CONFIG_STREAM_LABEL "cf_stream"
#define CONFIG_STREAM_SCHEDULES \
    (RBRGen4Label[])            \
    {                           \
        SCHEDULE_PT_LABEL       \
    }
#define CONFIG_STREAM_SCHEDULE_COUNT 1

#define NEW_DATASET_LABEL "ds_stream"

const char *programName = "";

/* The instrument's output format, which decides what each sample carries. */
RBRGen4OutputFormat outputFormat;

/* Set by the signal handler to stop the streaming loop. */
volatile sig_atomic_t terminate = 0;

void handleSignal(int signo)
{
    (void) signo;
    terminate = 1;
}

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

/* Print the column headers matching the output format and the streamed
 * group's channels. */
void printHeader(void)
{
    const char *separator = "";
    if (outputFormat.scheduleLabel) {
        printf("schedule");
        separator = ", ";
    }
    if (outputFormat.dateTime) {
        printf("%stimestamp", separator);
        separator = ", ";
    }
    for (int32_t i = 0; i < GROUP_PT_CHANNEL_COUNT; i++) {
        printf("%s%s", separator, GROUP_PT_CHANNELS[i]);
        separator = ", ";
    }
    printf("\n");
}

/* Called by the library for every streamed sample it reads. The schedule
 * label and timestamp are only present when the output format includes them.
 * Error readings are NaNs carrying an error code; report the code instead of
 * the value. */
RBRGen4Error instrumentSample(const struct RBRGen4 *conn, const struct RBRGen4Sample *const sample)
{
    /* Unused. */
    (void) conn;

    const char *separator = "";
    if (outputFormat.scheduleLabel) {
        printf("%s", sample->scheduleLabel);
        separator = ", ";
    }
    if (outputFormat.dateTime) {
        char ftime[32];
        time_t sampleSeconds = (time_t) (sample->timestamp / 1000);
        struct tm sampleTime;
        gmtime_r(&sampleSeconds, &sampleTime);
        strftime(ftime, sizeof(ftime), "%F %T", &sampleTime);
        printf("%s%s.%03" PRId64, separator, ftime, sample->timestamp % 1000);
        separator = ", ";
    }
    for (int32_t i = 0; i < sample->channelCount; i++) {
        if (RBRGen4Reading_isError(sample->readings[i])) {
            printf("%serror %d", separator, RBRGen4Reading_getError(sample->readings[i]));
        } else {
            printf("%s%f", separator, sample->readings[i]);
        }
        separator = ", ";
    }
    printf("\n");

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
    /* Streamed samples are parsed into this buffer before being passed to
     * the sample callback. */
    RBRGen4Sample sampleBuffer;
    const RBRGen4Callbacks callbacks = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite,
        .sample = instrumentSample,
        .sampleBuffer = &sampleBuffer,
    };

    err = RBRGen4_open(&conn, &callbacks, INSTRUMENT_COMMAND_TIMEOUT_MSEC, (void *) &instrumentFd);
    if (err) {
        logCmdError(&conn, err, "Failed to establish instrument connection");
        goto fileCleanup;
    }

    /* Samples are printed according to the output format */
    err = RBRGen4_getOutputFormat(&conn, &outputFormat);
    if (err) {
        logCmdError(&conn, err, "Failed to get output format");
        goto instrumentCleanup;
    }

    /* The schedule streams over whichever link we are connected by */
    RBRGen4Link link;
    err = RBRGen4_getLink(&conn, &link);
    if (err) {
        logCmdError(&conn, err, "Failed to get link");
        goto instrumentCleanup;
    }
    printf("Connected to the instrument via %s.\n", RBRGen4LinkType_name(link.type));

    RBRGen4ScheduleStream stream;
    switch (link.type) {
    case RBRGEN4_LINK_TYPE_USB:
        stream = RBRGEN4_SCHEDULE_STREAM_USB;
        break;
    case RBRGEN4_LINK_TYPE_SERIAL:
        stream = RBRGEN4_SCHEDULE_STREAM_SERIAL;
        break;
    default:
        fprintf(stderr,
                "%s: connection method to the instrument is unclear, so"
                " streaming can't be enabled.\n",
                programName);
        err = RBRGEN4_UNSUPPORTED;
        goto instrumentCleanup;
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
        .count = GROUP_PT_CHANNEL_COUNT,
        .labels = GROUP_PT_CHANNELS,
    };
    err = RBRGen4_setGroup(&conn, &groupPt, &groupPtChannelList);
    if (err) {
        logCmdError(&conn, err, "Failed to set new group");
        goto instrumentCleanup;
    }

    /* Create a schedule with our new group, streaming over our link */
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
    schedule.stream = stream;
    schedule.mode = RBRGEN4_SCHEDULE_MODE_CONTINUOUS;
    schedule.parameters.continuous.period = SCHEDULE_PT_PERIOD;
    const RBRGen4LabelList scheduleGroupList = {
        .size = SCHEDULE_PT_GROUP_COUNT,
        .count = SCHEDULE_PT_GROUP_COUNT,
        .labels = SCHEDULE_PT_GROUPS,
    };
    err = RBRGen4_setSchedule(&conn, &schedule, &scheduleGroupList);
    if (err) {
        logCmdError(&conn, err, "Failed to set new schedule");
        goto instrumentCleanup;
    }

    /* Create a config with our new schedule */
    RBRGen4Config config = {
        .label = CONFIG_STREAM_LABEL,
    };
    err = RBRGen4_createConfig(&conn, config.label);
    if (err) {
        logCmdError(&conn, err, "Failed to create new config");
        goto instrumentCleanup;
    }
    const RBRGen4LabelList configScheduleList = {
        .size = CONFIG_STREAM_SCHEDULE_COUNT,
        .count = CONFIG_STREAM_SCHEDULE_COUNT,
        .labels = CONFIG_STREAM_SCHEDULES,
    };
    err = RBRGen4_setConfig(&conn, &config, &configScheduleList);
    if (err) {
        logCmdError(&conn, err, "Failed to set new config");
        goto instrumentCleanup;
    }

    /* Start sampling as soon as the instrument is enabled */
    RBRGen4Deployment deployment;
    err = RBRGen4_getDeployment(&conn, &deployment);
    if (err) {
        logCmdError(&conn, err, "Failed to get deployment");
        goto instrumentCleanup;
    }
    deployment.gate = RBRGEN4_DEPLOYMENT_GATE_NONE;
    err = RBRGen4_setDeployment(&conn, &deployment);
    if (err) {
        logCmdError(&conn, err, "Failed to set deployment");
        goto instrumentCleanup;
    }

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

    /* Receive streamed samples until interrupted. Each sample is delivered
     * to instrumentSample(). */
    signal(SIGINT, handleSignal);
    signal(SIGTERM, handleSignal);
    printf("%s: Streaming; press Ctrl-C to stop.\n", programName);
    printHeader();
    while (!terminate) {
        err = RBRGen4_readSample(&conn);
        if (terminate) {
            /* The signal interrupted the read; that is not a failure. */
            err = RBRGEN4_SUCCESS;
        } else if (err == RBRGEN4_TIMEOUT) {
            /* Nothing arrived within the command timeout; keep waiting. */
            continue;
        } else if (err) {
            logCmdError(&conn, err, "Failed to read sample");
            break;
        }
    }

    /* Stop the deployment so the instrument is not left streaming */
    RBRGen4Error disableErr = disableIgnoreWarning(&conn, &loggingState);
    if (disableErr) {
        logCmdError(&conn, disableErr, "Failed to disable instrument");
        err = disableErr;
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
