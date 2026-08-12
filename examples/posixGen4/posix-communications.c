/**
 * \file posix-communications.c
 *
 * \brief Example of opening an instrument connection and exercising the
 * communication commands (sleep, link).
 *
 * \copyright
 * Copyright (c) 2024 RBR Ltd.
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

#include "posix-shared.h"

bool terminate=0;
void sig_handler(int signo){
    if(signo == SIGINT || signo == SIGTERM){
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
        fprintf(stderr, "%s: Failed to open serial device: %s!\n",
                programName,
                strerror(errno));
        return EXIT_FAILURE;
    }

    fprintf(stderr,"open: instrumentFd = %d (>=0 means success)\n",instrumentFd);
    
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
        fprintf(stderr, "%s: Failed to establish instrument connection: %s!\n",
                programName,
                RBRInstrumentGen4Error_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    if ((err = RBRInstrumentGen4_sleep(instrument)) != RBRINSTRUMENTGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to put instrument to sleep: %s!\n",
                programName,
                RBRInstrumentGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    RBRInstrumentGen4Link link;
    if ((err = RBRInstrumentGen4_getLink(instrument, &link)) != RBRINSTRUMENTGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to get instrument link: %s!\n",
                programName,
                RBRInstrumentGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }
    printf("Connected to the instrument via %s.\n",
           RBRInstrumentGen4Link_name(link));

    switch (link)
    {
    case RBRINSTRUMENTGEN4_LINK_USB:
    #if 0
        RBRInstrumentGen4_setUSBStreamingState(instrument, true);
    #endif
        break;
    case RBRINSTRUMENTGEN4_LINK_SERIAL:
    case RBRINSTRUMENTGEN4_LINK_WIFI:
        {
            RBRInstrumentGen4Serial serial;
            RBRInstrumentGen4_getSerial(instrument, &serial);
            printf("Connected in %s mode at %s baud.\n",
                   RBRInstrumentGen4SerialMode_name(serial.mode),
                   RBRInstrumentGen4SerialBaudRate_name(serial.baudRate));
#if 0
            RBRInstrumentGen4_setSerialStreamingState(instrument, true);
#endif
            break;
        }
    default:
        fprintf(stderr,
                "I don't know how I'm connected to the instrument, so I can't"
                " enable streaming. Giving up.\n");
        goto instrumentCleanup;
    }

    char name[64];
    RBRInstrumentGen4OutputFormat_name(instrument->outputFormat, name);
    printf("generation: %s, id: model=%s version=%s serial=%u fwtype=%u, outputformat: %s\n",
           RBRInstrumentGen4Generation_name(instrument->generation),
           instrument->id.model,
           instrument->id.fwversion,
           instrument->id.sn,
           instrument->id.fwtype,
           name);

    RBRInstrumentGen4Instrument info;
    if ((err = RBRInstrumentGen4_getInstrument(instrument, &info)) != RBRINSTRUMENTGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to get instrument info: %s!\n",
                programName,
                RBRInstrumentGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }
    printf("info: dataType=%s, fwlock=%s, pn=%s\n",
           RBRInstrumentGen4DataType_name(info.dataType),
           info.fwLock ? "true" : "false",
           info.pn);

    /* Power source */

    RBRInstrumentGen4PowerSource powerSource;
    if ((err = 
    RBRInstrumentGen4_getPowerSource(instrument, &powerSource)
    ) != RBRINSTRUMENTGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to get power source: %s!\n",
                programName,
                RBRInstrumentGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }
    printf("powerSource: %s\n", RBRInstrumentGen4PowerSource_name(powerSource));

    /* Internal battery */

    if ((err = RBRInstrumentGen4_setPowerInternalBatteryType(instrument, RBRINSTRUMENTGEN4_INTERNAL_BATTERY_LIFES2)) != RBRINSTRUMENTGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to set internal battery type: %s!\n",
                programName,
                RBRInstrumentGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    if ((err = RBRInstrumentGen4_resetPowerInternalUsed(instrument)) != RBRINSTRUMENTGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to reset internal battery usage: %s!\n",
                programName,
                RBRInstrumentGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    RBRInstrumentGen4PowerInternal powerInternal;
    if ((err = RBRInstrumentGen4_getPowerInternal(instrument, &powerInternal)) != RBRINSTRUMENTGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to get internal power info: %s!\n",
                programName,
                RBRInstrumentGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }
    printf("powerInternal: voltage=%f, batteryType=%s (%s), capacity=%f, used=%f\n",
           powerInternal.voltage,
           RBRInstrumentGen4InternalBatteryType_name(powerInternal.batteryType),
           RBRInstrumentGen4InternalBatteryType_displayName(powerInternal.batteryType),
           powerInternal.capacity,
           powerInternal.used);

    /* External battery */

    if ((err = RBRInstrumentGen4_setPowerExternalBatteryType(instrument, RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMATA_NIMH)) != RBRINSTRUMENTGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to set external battery type: %s!\n",
                programName,
                RBRInstrumentGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    if ((err = RBRInstrumentGen4_resetPowerExternalUsed(instrument)) != RBRINSTRUMENTGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to reset external battery usage: %s!\n",
                programName,
                RBRInstrumentGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    RBRInstrumentGen4PowerExternal powerExternal;
    if ((err = 
    RBRInstrumentGen4_getPowerExternal(instrument, &powerExternal)
    ) != RBRINSTRUMENTGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to get external power info: %s!\n",
                programName,
                RBRInstrumentGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }
    printf("powerExternal: voltage=%f, batteryType=%s (%s), capacity=%f, used=%f",
           powerExternal.voltage,
           RBRInstrumentGen4ExternalBatteryType_name(powerExternal.batteryType),
           RBRInstrumentGen4ExternalBatteryType_displayName(powerExternal.batteryType),
           powerExternal.capacity,
           powerExternal.used);


#if 0
    RBRInstrumentGen4Deployment deployment;
    RBRInstrumentGen4_getDeployment(instrument, &deployment);
    if (deployment.status != RBRINSTRUMENTGEN4_STATUS_LOGGING)
    {
        printf("%s: Instrument is %s, not logging. I'm going to start it.\n",
               programName,
               RBRInstrumentGen4DeploymentStatus_name(deployment.status));

        if ((err = instrumentStart(instrument)) != RBRINSTRUMENTGEN4_SUCCESS)
        {
            fprintf(stderr,
                    "%s: Failed to start instrument: %s!\n",
                    programName,
                    RBRInstrumentGen4Error_name(err));
            status = EXIT_FAILURE;
            goto instrumentCleanup;
        }
    }
    goto fileCleanup;
#endif

instrumentCleanup:
    RBRInstrumentGen4_close(instrument);

fileCleanup:
    close(instrumentFd);
    fprintf(stderr, "close: instrumentFd = %d (>=0 means success)\n",instrumentFd);
    fprintf(stderr, "close: status = %d\n",status);

    return status;
}
