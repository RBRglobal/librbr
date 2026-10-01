/*
 * Copyright (c) 2024 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file posix-communications.c
 *
 * \brief Example of opening an instrument connection, reporting how the
 * instrument is connected and powered, and putting it to sleep.
 *
 * Nothing is written to the instrument.
 */

/* Required for errno. */
#include <errno.h>
/* Required for PRId32. */
#include <inttypes.h>
/* Required for fprintf, printf. */
#include <stdio.h>
/* Required for EXIT_SUCCESS, etc. */
#include <stdlib.h>
/* Required for strerror. */
#include <string.h>
/* Required for close. */
#include <unistd.h>

#include "RBRGen4.h"
#include "RBRGen4Commands.h"
#include "posix-shared.h"

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
    uint8_t commandBuffer[RBRGEN4_COMMAND_BUFFER_DEFAULT];
    uint8_t responseBuffer[RBRGEN4_RESPONSE_BUFFER_DEFAULT];
    const RBRGen4Environment environment = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite,
        .command = commandBuffer,
        .commandCapacity = sizeof(commandBuffer),
        .response = responseBuffer,
        .responseCapacity = sizeof(responseBuffer),
    };

    err =
        RBRGen4_open(&conn, &environment, INSTRUMENT_COMMAND_TIMEOUT_MSEC, (void *) &instrumentFd);
    if (err) {
        logCmdError(&conn, err, "Failed to establish instrument connection");
        goto fileCleanup;
    }

    /* Identify the instrument */
    RBRGen4Instrument instrument;
    err = RBRGen4_getInstrument(&conn, &instrument);
    if (err) {
        logCmdError(&conn, err, "Failed to get instrument");
        goto instrumentCleanup;
    }
    printf("Connected to %s serial number %06" PRId32 " running firmware"
           " v%s (%s) with command API v%s; logging is %s.\n",
           instrument.model,
           instrument.sn,
           instrument.fwVersion,
           instrument.semver,
           instrument.apiVersion,
           RBRGen4InstrumentState_name(instrument.state));

    /* Report how the instrument is connected */
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

    /* Report how the instrument is powered */
    RBRGen4PowerSource powerSource;
    err = RBRGen4_getPowerSource(&conn, &powerSource);
    if (err) {
        logCmdError(&conn, err, "Failed to get power source");
        goto instrumentCleanup;
    }
    printf("Powered from %s.\n", RBRGen4PowerSource_name(powerSource));

    RBRGen4PowerInternal powerInternal;
    err = RBRGen4_getPowerInternal(&conn, &powerInternal);
    if (err) {
        logCmdError(&conn, err, "Failed to get internal power");
        goto instrumentCleanup;
    }
    printf("Internal battery: %s at %.2fV, %.2f used.\n",
           RBRGen4InternalBatteryType_displayName(powerInternal.batteryType),
           (double) powerInternal.voltage,
           (double) powerInternal.used);

    RBRGen4PowerExternal powerExternal;
    err = RBRGen4_getPowerExternal(&conn, &powerExternal);
    if (err) {
        logCmdError(&conn, err, "Failed to get external power");
        goto instrumentCleanup;
    }
    printf("External battery: %s at %.2fV, %.2f used.\n",
           RBRGen4ExternalBatteryType_displayName(powerExternal.batteryType),
           (double) powerExternal.voltage,
           (double) powerExternal.used);

    /* Put the instrument to sleep now that we are done with it */
    err = RBRGen4_sleep(&conn);
    if (err) {
        logCmdError(&conn, err, "Failed to put instrument to sleep");
        goto instrumentCleanup;
    }
    printf("%s: Instrument is asleep\n", programName);

instrumentCleanup:
    RBRGen4_close(&conn);

fileCleanup:
    close(instrumentFd);

    return (err == RBRGEN4_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
}
