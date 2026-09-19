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
    const struct RBRGen4 *conn,
    const struct RBRGen4Sample *const sample)
{
    /* Unused. */
    (void) conn;

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
    RBRGen4 *conn = NULL;
    
    RBRGen4 instrumentSpace;
    conn = &instrumentSpace;

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
             conn,
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

    if ((err = RBRGen4_sleep(conn)) != RBRGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to put instrument to sleep: %s!\n",
                programName,
                RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    RBRGen4Link link;
    if ((err = RBRGen4_getLink(conn, &link))
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
            RBRGen4_getLinkSerial(conn, &serial);
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

    printf("generation: %s, id: model=%s version=%s serial=%u fwtype=%u"
           " apiversion=%s,"
           " outputformat: sn=%s schedulelabel=%s datetime=%s crc=%s"
           " datatype=%s\n",
           RBRGen4Generation_name(conn->generation),
           conn->id.model,
           conn->id.fwversion,
           conn->id.sn,
           conn->id.fwtype,
           conn->id.apiversion,
           conn->outputFormat.sn ? "on" : "off",
           conn->outputFormat.scheduleLabel ? "on" : "off",
           conn->outputFormat.dateTime ? "on" : "off",
           conn->outputFormat.crc ? "on" : "off",
           RBRGen4DataType_name(conn->outputFormat.dataType));

    RBRGen4Instrument info;
    if ((err = RBRGen4_getInstrument(conn, &info)) != RBRGEN4_SUCCESS)
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

    RBRGen4PowerSource powerSource;
    if ((err = 
    RBRGen4_getPowerSource(conn, &powerSource)
    ) != RBRGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to get power source: %s!\n",
                programName,
                RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }
    printf("powerSource: %s\n", RBRGen4PowerSource_name(powerSource));

    /* Internal battery */

    if ((err = RBRGen4_setPowerInternalBatteryType(conn, RBRGEN4_INTERNAL_BATTERY_LIFES2)) != RBRGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to set internal battery type: %s!\n",
                programName,
                RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    if ((err = RBRGen4_resetPowerInternalUsed(conn)) != RBRGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to reset internal battery usage: %s!\n",
                programName,
                RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    RBRGen4PowerInternal powerInternal;
    if ((err = RBRGen4_getPowerInternal(conn, &powerInternal)) != RBRGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to get internal power info: %s!\n",
                programName,
                RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }
    printf("powerInternal: voltage=%f, batteryType=%s (%s), used=%f\n",
           powerInternal.voltage,
           RBRGen4InternalBatteryType_name(powerInternal.batteryType),
           RBRGen4InternalBatteryType_displayName(powerInternal.batteryType),
           powerInternal.used);

    /* External battery */

    if ((err = RBRGen4_setPowerExternalBatteryType(conn, RBRGEN4_EXTERNAL_BATTERY_FERMATA_NIMH)) != RBRGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to set external battery type: %s!\n",
                programName,
                RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    if ((err = RBRGen4_resetPowerExternalUsed(conn)) != RBRGEN4_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to reset external battery usage: %s!\n",
                programName,
                RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    RBRGen4PowerExternal powerExternal;
    if ((err = 
    RBRGen4_getPowerExternal(conn, &powerExternal)
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
           RBRGen4ExternalBatteryType_name(powerExternal.batteryType),
           RBRGen4ExternalBatteryType_displayName(powerExternal.batteryType),
           powerExternal.used);

instrumentCleanup:
    RBRGen4_close(conn);

fileCleanup:
    close(instrumentFd);
    fprintf(stderr, "close: instrumentFd = %d (>=0 means success)\n",instrumentFd);
    fprintf(stderr, "close: status = %d\n",status);

    return status;
}
