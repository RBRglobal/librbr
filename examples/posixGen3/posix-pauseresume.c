/*
 * Copyright (c) 2022 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file posix-pauseresume.c
 *
 * \brief Example of using the library to pauseresume instrument data in a POSIX
 * environment.
 */

/* Required for errno. */
#include <errno.h>
/* Required for fprintf, printf. */
#include <stdio.h>
/* Required for EXIT_FAILURE, EXIT_SUCCESS. */
#include <stdlib.h>
/* Required for strerror. */
#include <string.h>
/* Required for gmtime_r, time_t, strftime. */
#include <time.h>
/* Required for close. */
#include <unistd.h>

#include "posix-shared.h"
#include "RBRGen3Commands.h"

int main(int argc, char *argv[])
{
    /* first argv is program name, second argv is devicePath */
    char *programName = argv[0];
    char *devicePath;

    int status = EXIT_SUCCESS;
    int instrumentFd;

    RBRGen3Error err;
    RBRGen3 conn;
    uint8_t commandBuffer[RBRGEN3_COMMAND_BUFFER_DEFAULT];
    uint8_t responseBuffer[RBRGEN3_RESPONSE_BUFFER_DEFAULT];

    if (argc < 2) {
        fprintf(stderr, "Usage: %s device\n", argv[0]);
        return EXIT_FAILURE;
    }

    devicePath = argv[1];

    if ((instrumentFd = openSerialFd(devicePath)) < 0) {
        fprintf(stderr, "%s: Failed to open serial device: %s!\n", programName, strerror(errno));
        return EXIT_FAILURE;
    }

    fprintf(stderr, "%s: Using %s v%s.\n", programName, RBRGEN3_LIB_NAME, RBRGEN3_LIB_VERSION);

    RBRGen3Environment environment = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite,
        .command = commandBuffer,
        .commandCapacity = sizeof(commandBuffer),
        .response = responseBuffer,
        .responseCapacity = sizeof(responseBuffer),
    };

    if ((err = RBRGen3_open(
             &conn, &environment, INSTRUMENT_COMMAND_TIMEOUT_MSEC, (void *) &instrumentFd)) !=
        RBRGEN3_SUCCESS) {
        fprintf(stderr,
                "%s: Failed to establish instrument connection: %s!\n",
                programName,
                RBRGen3Error_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    RBRGen3Link link;
    RBRGen3_getLink(&conn, &link);
    printf("Connected to the instrument via %s.\n", RBRGen3Link_name(link));

    switch (link) {
    case RBRGEN3_LINK_USB:
        RBRGen3_setUSBStreamingState(&conn, true);
        break;
    case RBRGEN3_LINK_SERIAL:
    case RBRGEN3_LINK_WIFI: {
        RBRGen3Serial serial;
        RBRGen3_getSerial(&conn, &serial);
        printf("Connected in %s mode at %s baud.\n",
               RBRGen3SerialMode_name(serial.mode),
               RBRGen3SerialBaudRate_name(serial.baudRate));

        RBRGen3_setSerialStreamingState(&conn, true);
        break;
    }
    default:
        fprintf(stderr,
                "I don't know how I'm connected to the instrument, so I can't"
                " enable streaming. Giving up.\n");
        goto instrumentCleanup;
    }

    /* Get pauseresume state and report error (if any) according to response. */
    RBRGen3PauseResumeState state;
    state = RBRGEN3_UNKNOWN_PAUSE_RESUME;
    /* pauseStatus will be used to decide if needs to proceed with "resume". */
    RBRGen3PauseStatus pauseStatus;
    pauseStatus = RBRGEN3_UNKNOWN_PAUSE;

    if ((err = RBRGen3_getPauseResume(&conn, &state)) != RBRGEN3_SUCCESS) {
        /* if this isn't an RBR instrument, or if the firmware in use doesn't support pauseresume.*/
        fprintf(stderr, "%s: Feature not supported: %s! \n", programName, RBRGen3Error_name(err));
        status = EXIT_FAILURE;
        fprintf(stderr, "E%d %s\n", conn.response.error, conn.response.response);
        goto fileCleanup;
    };

    const char *stateName = RBRGen3PauseResumeState_name(state);
    printf("pauseresume state=%s\n", stateName);

    /* code below: print out human-readable errors. */
    if (state == RBRGEN3_PAUSE_RESUME_NA) {
        printf("(Either the deployment has not been enabled, or the sampling mode is 'regimes',"
               " or more than one gating condition is enabled.)\n");
    }

    /* Proceeds with command 'pause' in this case. */
    else if (state == RBRGEN3_PAUSE_RESUME_RUNNING) {
        err = RBRGen3_pause(&conn, &pauseStatus);
        const char *statusName = RBRGen3PauseStatus_name(pauseStatus);
        printf("pause status=%s\n", statusName);
        if (pauseStatus == RBRGEN3_UNKNOWN_PAUSE) {
            fprintf(stderr, "E%d %s\n", conn.response.error, conn.response.response);
        }
    }

    /* Proceeds with command 'resume' in this case. */
    if (state == RBRGEN3_PAUSE_RESUME_PAUSED || pauseStatus == RBRGEN3_PAUSE_PAUSED) {
        RBRGen3ResumeStatus resumeStatus;
        resumeStatus = RBRGEN3_UNKNOWN_RESUME;
        err = RBRGen3_resume(&conn, &resumeStatus);
        const char *statusName = RBRGen3ResumeStatus_name(resumeStatus);
        printf("resume status=%s\n", statusName);
        if (resumeStatus == RBRGEN3_UNKNOWN_RESUME) {
            fprintf(stderr, "E%d %s\n", conn.response.error, conn.response.response);
        }
    }

instrumentCleanup:
    RBRGen3_close(&conn);
fileCleanup:
    close(instrumentFd);
    return status;
}
