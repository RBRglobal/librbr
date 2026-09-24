/**
 * \file posix-detect.c
 *
 * \brief Example of detecting an instrument's generation and opening the
 * matching API.
 *
 * Calls RBRGen3_open() over a set of Gen3 callbacks. If the instrument is
 * unsupported by the Gen3 API, RBRGen3_getGeneration() reports the generation
 * that was detected in the process; if it is Logger4, the connection is
 * reopened with RBRGen4_open(). This is the pattern an application built with
 * both generation APIs follows to pick one per instrument.
 *
 * \copyright
 * Copyright (c) 2026 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for errno. */
#include <errno.h>
/* Required for fprintf, printf. */
#include <stdio.h>
/* Required for EXIT_SUCCESS, etc. */
#include <stdlib.h>
/* Required for strerror. */
#include <string.h>
/* Required for close. */
#include <unistd.h>

#include "posix-shared.h"

const char *programName = "";

/* Report a failed Gen3 library call. A hardware error also carries the
 * instrument's own message, which says what it objected to. */
static void logGen3Error(const RBRGen3 *conn, RBRGen3Error err, const char *msg)
{
    fprintf(stderr, "%s: %s (%s)\n", programName, msg, RBRGen3Error_name(err));
    if (err == RBRGEN3_HARDWARE_ERROR) {
        fprintf(stderr,
                "%s: Instrument reported: %s\n",
                programName,
                RBRGen3_getLastHardwareErrorMessage(conn));
    }
}

/* Report a failed Gen4 library call. A hardware error also carries the
 * instrument's own message, which says what it objected to. */
static void logGen4Error(const RBRGen4 *conn, RBRGen4Error err, const char *msg)
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

    const RBRGen3Callbacks gen3Callbacks = {
        .time = gen3InstrumentTime,
        .sleep = gen3InstrumentSleep,
        .read = gen3InstrumentRead,
        .write = gen3InstrumentWrite,
    };

    RBRGen3 gen3Conn;
    RBRGen3Error err3 = RBRGen3_open(
        &gen3Conn, &gen3Callbacks, INSTRUMENT_COMMAND_TIMEOUT_MSEC, (void *) &instrumentFd);
    if (err3 == RBRGEN3_SUCCESS) {
        RBRGen3Id id;
        err3 = RBRGen3_getId(&gen3Conn, &id);
        if (err3 != RBRGEN3_SUCCESS) {
            logGen3Error(&gen3Conn, err3, "Failed to get instrument identification");
            RBRGen3_close(&gen3Conn);
            close(instrumentFd);
            return EXIT_FAILURE;
        }

        printf("Opened with the Gen3 API\n");
        printf("Generation: %s\n", RBRCommonGeneration_name(RBRGen3_getGeneration(&gen3Conn)));
        printf("Model:      %s\n", id.model);
        printf("Serial:     %06" PRIu32 "\n", id.serial);
        printf("Firmware:   v%s (type %" PRIu16 ")\n", id.version, id.fwtype);
        RBRGen3_close(&gen3Conn);
        close(instrumentFd);
        return EXIT_SUCCESS;
    } else if (err3 != RBRGEN3_UNSUPPORTED) {
        logGen3Error(&gen3Conn, err3, "Failed to open Gen3 connection");
        close(instrumentFd);
        return EXIT_FAILURE;
    }

    /* RBRGEN3_UNSUPPORTED: RBRGen3_open() already identified the instrument
     * before rejecting it. */
    RBRCommonGeneration generation = RBRGen3_getGeneration(&gen3Conn);
    RBRGen3_close(&gen3Conn);

    if (generation == RBRCOMMON_UNKNOWN_GENERATION) {
        fprintf(stderr, "%s: Could not identify the instrument.\n", programName);
        close(instrumentFd);
        return EXIT_FAILURE;
    } else if (generation != RBRCOMMON_LOGGER4) {
        fprintf(stderr,
                "%s: Instrument generation %s is not supported by either API.\n",
                programName,
                RBRCommonGeneration_name(generation));
        close(instrumentFd);
        return EXIT_FAILURE;
    }

    const RBRGen4Callbacks gen4Callbacks = {
        .time = gen4InstrumentTime,
        .sleep = gen4InstrumentSleep,
        .read = gen4InstrumentRead,
        .write = gen4InstrumentWrite,
    };

    RBRGen4 gen4Conn;
    RBRGen4Error err4 = RBRGen4_open(
        &gen4Conn, &gen4Callbacks, INSTRUMENT_COMMAND_TIMEOUT_MSEC, (void *) &instrumentFd);
    if (err4 != RBRGEN4_SUCCESS) {
        logGen4Error(&gen4Conn, err4, "Failed to open Gen4 connection");
        close(instrumentFd);
        return EXIT_FAILURE;
    }

    RBRGen4Id4 id;
    err4 = RBRGen4_getId4(&gen4Conn, &id);
    if (err4 != RBRGEN4_SUCCESS) {
        logGen4Error(&gen4Conn, err4, "Failed to get instrument identification");
        RBRGen4_close(&gen4Conn);
        close(instrumentFd);
        return EXIT_FAILURE;
    }

    printf("Opened with the Gen4 API\n");
    printf("Generation: %s\n", RBRCommonGeneration_name(RBRGen4_getGeneration(&gen4Conn)));
    printf("Model:      %s\n", id.model);
    printf("Serial:     %06" PRId32 "\n", id.sn);
    printf("Firmware:   v%s (type %" PRId32 ")\n", id.fwversion, id.fwtype);
    RBRGen4_close(&gen4Conn);
    close(instrumentFd);
    return EXIT_SUCCESS;
}
