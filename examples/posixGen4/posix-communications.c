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
        fprintf(stderr, "%s: Failed to open serial device: %s!\n",
                programName,
                strerror(errno));
        return EXIT_FAILURE;
    }

    fprintf(stderr,"open: instrumentFd = %d (>=0 means success)\n",instrumentFd);
    
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
        fprintf(stderr, "%s: Failed to establish instrument connection: %s!\n",
                programName,
                RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    if ((err = RBRGen4_sleep(instrument)) != RBRGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to put instrument to sleep: %s!\n",
                programName,
                RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    RBRGen4Link link;
    if ((err = RBRGen4_getLink(instrument, &link))
        != RBRGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to get instrument link: %s!\n",
                programName,
                RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }
    printf("Connected to the instrument via %s.\n",
           RBRGen4LinkType_name(link.type));

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
    default:
        fprintf(stderr,
                "I don't know how I'm connected to the instrument, so I can't"
                " enable streaming. Giving up.\n");
        goto instrumentCleanup;
    }

    printf("generation: %s, id: model=%s version=%s serial=%u fwtype=%u,"
           " outputformat: sn=%s schedulelabel=%s datetime=%s crc=%s"
           " encoding=%s datatype=%s\n",
           RBRGen4Generation_name(instrument->generation),
           instrument->id.model,
           instrument->id.fwversion,
           instrument->id.sn,
           instrument->id.fwtype,
           instrument->outputFormat.sn ? "on" : "off",
           instrument->outputFormat.scheduleLabel ? "on" : "off",
           instrument->outputFormat.dateTime ? "on" : "off",
           instrument->outputFormat.crc ? "on" : "off",
           RBRGen4Encoding_name(instrument->outputFormat.encoding),
           RBRGen4DataType_name(instrument->outputFormat.dataType));

    RBRInstrumentGen4Instrument info;
    if ((err = RBRInstrumentGen4_getInstrument(instrument, &info)) != RBRGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to get instrument info: %s!\n",
                programName,
                RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }
    printf("info: dataType=%s, fwlock=%s, pn=%s\n",
           RBRGen4DataType_name(info.dataType),
           info.fwLock ? "true" : "false",
           info.pn);

    /* Power source */

    RBRInstrumentGen4PowerSource powerSource;
    if ((err = 
    RBRInstrumentGen4_getPowerSource(instrument, &powerSource)
    ) != RBRGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to get power source: %s!\n",
                programName,
                RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }
    printf("powerSource: %s\n", RBRInstrumentGen4PowerSource_name(powerSource));

    /* Internal battery */

    if ((err = RBRInstrumentGen4_setPowerInternalBatteryType(instrument, RBRINSTRUMENTGEN4_INTERNAL_BATTERY_LIFES2)) != RBRGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to set internal battery type: %s!\n",
                programName,
                RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    if ((err = RBRInstrumentGen4_resetPowerInternalUsed(instrument)) != RBRGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to reset internal battery usage: %s!\n",
                programName,
                RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    RBRInstrumentGen4PowerInternal powerInternal;
    if ((err = RBRInstrumentGen4_getPowerInternal(instrument, &powerInternal)) != RBRGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to get internal power info: %s!\n",
                programName,
                RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }
    printf("powerInternal: voltage=%f, batteryType=%s (%s), used=%f\n",
           powerInternal.voltage,
           RBRInstrumentGen4InternalBatteryType_name(powerInternal.batteryType),
           RBRInstrumentGen4InternalBatteryType_displayName(powerInternal.batteryType),
           powerInternal.used);

    /* External battery */

    if ((err = RBRInstrumentGen4_setPowerExternalBatteryType(instrument, RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMATA_NIMH)) != RBRGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to set external battery type: %s!\n",
                programName,
                RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    if ((err = RBRInstrumentGen4_resetPowerExternalUsed(instrument)) != RBRGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to reset external battery usage: %s!\n",
                programName,
                RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    RBRInstrumentGen4PowerExternal powerExternal;
    if ((err = 
    RBRInstrumentGen4_getPowerExternal(instrument, &powerExternal)
    ) != RBRGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to get external power info: %s!\n",
                programName,
                RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }
    printf("powerExternal: voltage=%f, batteryType=%s (%s), used=%f",
           powerExternal.voltage,
           RBRInstrumentGen4ExternalBatteryType_name(powerExternal.batteryType),
           RBRInstrumentGen4ExternalBatteryType_displayName(powerExternal.batteryType),
           powerExternal.used);

instrumentCleanup:
    RBRGen4_close(instrument);

fileCleanup:
    close(instrumentFd);
    fprintf(stderr, "close: instrumentFd = %d (>=0 means success)\n",instrumentFd);
    fprintf(stderr, "close: status = %d\n",status);

    return status;
}
