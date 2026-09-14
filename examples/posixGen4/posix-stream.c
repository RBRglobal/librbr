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

RBRGen4Error instrumentSample(
    const struct RBRGen4 *instrument,
    const struct RBRGen4Sample *const sample)
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

    return RBRGEN4_SUCCESS;
}

int main(int argc, char *argv[])
{
    char *programName = argv[0];
    char *devicePath;

    int status = EXIT_SUCCESS;
    int instrumentFd;

    RBRGen4Error err;
    RBRGen4 *instrument = NULL;
    // no dynamic allocation case:
    RBRGen4 instrumentSpace;
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
            RBRGEN4_LIB_NAME,
            RBRGEN4_LIB_VERSION,
            RBRGEN4_LIB_BUILD_DATE);

    RBRGen4Sample sampleBuffer;
    RBRGen4Callbacks callbacks = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite,
        .sample = instrumentSample,
        .sampleBuffer = &sampleBuffer
    };

    if ((err = RBRGen4_open(
             &instrument,
             &callbacks,
             INSTRUMENT_COMMAND_TIMEOUT_MSEC,
             (void *) &instrumentFd)) != RBRGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to establish instrument connection: %s!\n", programName, RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    // set streaming in real time during deployment:
    RBRGen4Link link;
    RBRGen4_getLink(instrument, &link);
    printf("Connected to the instrument via %s.\n",
           RBRGen4LinkType_name(link.type));

    //(optional) get details about the connection.
    switch (link.type)
    {
    case RBRGEN4_LINK_TYPE_USB:
        break;
    case RBRGEN4_LINK_TYPE_SERIAL:
    {
        RBRGen4LinkSerial serial;
        RBRGen4_getLinkSerial(instrument, &serial);
        printf("Connected in %s mode at %s baud.\n",
               RBRGen4LinkSerialMode_name(serial.mode),
               RBRGen4LinkSerialBaudRate_name(serial.baudRate));
        break;
    }
    /* WiFi is not yet implemented */
    #if 0
    case RBRINSTRUMENTGEN4_LINK_TYPE_WIFI:
    {
        RBRGen4WiFi wifi;
        RBRGen4_getWiFi(instrument, &wifi);
        printf("Connected in WiFi mode at %s baud. Timeout is %d\n",
               RBRGen4LinkSerialBaudRate_name(wifi.baudRate),
               wifi.commandTimeout);
        break;
    }
    #endif
    default:
        fprintf(stderr,
                "%s: connection method to the instrument is unclear, so"
                " streaming can't be enabled.\n",
                programName);
        status = EXIT_FAILURE;
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
    RBRGen4InstrumentState loggingState
        = RBRGEN4_UNKNOWN_INSTRUMENT_STATE;
    RBRGen4_disable(instrument, &loggingState);

    RBRInstrumentGen4_deleteDatasetAll(instrument);

    RBRGen4_deleteConfigAll(instrument);
    RBRGen4_deleteScheduleAll(instrument);
    RBRGen4_deleteGroupAll(instrument);

    /************ group definition ************/
    // read the channel pool
    RBRGen4Channel channelBuf[RBRGEN4_CHANNEL_MAX];
    RBRGen4ChannelPool channelPool = {
        .size = RBRGEN4_CHANNEL_MAX,
        .pool = channelBuf
    };
    RBRGen4_getChannelPool(instrument, &channelPool);

    // specify groupLabel, channel labels, and create group instance
    RBRGen4Group group_pts;
    RBRInstrumentGen4_initNewGroup(instrument,
                        GROUP_PTS_LABEL,
                        GROUP_PTS_CHANNELS,
                        GROUP_PTS_CHANNEL_COUNT,
                        &channelPool,
                        &group_pts); // warning: need to read error!!!

    /************ schedule definition ************/
    RBRGen4Schedule schedule_pts;
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
                          &schedule_pts);

    /* The link we are connected over is where this schedule should stream. */
    switch (link.type)
    {
    case RBRGEN4_LINK_TYPE_USB:
        schedule_pts.stream = RBRGEN4_SCHEDULE_STREAM_USB;
        break;
    case RBRGEN4_LINK_TYPE_SERIAL:
        schedule_pts.stream = RBRGEN4_SCHEDULE_STREAM_SERIAL;
        break;
    default:
        /*
         * Unreachable: the switch above stops on any other link type. The
         * case has to exist for the compiler, so make it fatal rather than
         * quietly configure a schedule which streams nowhere, which would
         * leave this example with nothing to show.
         */
        fprintf(stderr,
                "%s: cannot stream over link type %s.\n",
                programName,
                RBRGen4LinkType_name(link.type));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }
    // warning: read error for RBRInstrumentGen4_initNewSchedule!!!
    RBRGen4_setSchedule(instrument, &schedule_pts, &groupList);

    /************ configuration definition ************/
    RBRGen4Config config_ascent;
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
                         &config_ascent);

    // specify outputformat. The setter sends every parameter of the command,
    // so read the current format and change only the sample fields.
    RBRGen4OutputFormat outputformat;
    RBRGen4_getOutputFormat(instrument, &outputformat);
    outputformat.sn = true;
    outputformat.scheduleLabel = true;
    outputformat.dateTime = false;
    outputformat.crc = true;
    RBRGen4_setOutputFormat(instrument, &outputformat);

    /************ deployment parameters ************/
    // need to stop if it's logging.
    RBRGen4Deployment deployment;
    RBRGen4_getDeployment(instrument, &deployment);

    str_to_deploymentDatetime(&deployment.startTime, STARTTIME);
    RBRGen4_setDeployment(instrument, &deployment);

    // verify the configurations for enable
    RBRGen4_verify(instrument,
                             &config_ascent,
                             NEW_DATASET_LABEL,
                             RBRGEN4_STORAGEMODE_NORMAL,
                             &loggingState);

    printf("%s: Start instrument logging with default_config.\n",
           programName);
    if ((err = RBRGen4_enable(instrument,
                                        &config_ascent,
                                        NEW_DATASET_LABEL,
                                        RBRGEN4_STORAGEMODE_NORMAL,
                                        &loggingState)) != RBRGEN4_SUCCESS)
    {
        fprintf(stderr,
                "%s: Failed to start instrument: %s!\n",
                programName,
                RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    };

    while (!terminate)
    {
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);
        if ((err = RBRInstrumentGen4_readSample(instrument)) != RBRGEN4_SUCCESS)
        {
            fprintf(stderr, "Error: %s\n", RBRGen4Error_name(err));
        }
    }

instrumentCleanup:
    RBRGen4_close(instrument);
fileCleanup:
    close(instrumentFd);
    return status;
}
