/**
 * \file RBRGen3Pauseresume.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2022 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for snprintf. */
#include <stdio.h>
/* Required for memset. */
#include <string.h>

#include "RBRGen3.h"
#include "RBRGen3Internal.h"
#include "RBRGen3Pauseresume.h"

const char *RBRGen3PauseresumeState_name(RBRGen3PauseresumeState state)
{
    switch (state) {
    /* Either the deployment has not been enabled or the samling mode is 'regimes' */
    case RBRGEN3_PAUSERESUME_NA:
        return "n/a";
    /* The deployment has been enabled and is paused */
    case RBRGEN3_PAUSERESUME_PAUSED:
        return "paused";
    /* The deployment has been enabled and is not paused */
    case RBRGEN3_PAUSERESUME_RUNNING:
        return "running";
    /* The feature is not allowed on this instrument */
    case RBRGEN3_UNKNOWN_PAUSERESUME:
    default:
        return "unknown pauseresume state";
    }
}

const char *RBRGen3PauseStatus_name(RBRGen3PauseStatus status)
{
    switch (status) {
    case RBRGEN3_PAUSE_PAUSED:
        return "paused";
    case RBRGEN3_UNKNOWN_PAUSE:
    default:
        return "unknown pause status";
    }
}

const char *RBRGen3ResumeStatus_name(RBRGen3ResumeStatus status)
{
    switch (status) {
    case RBRGEN3_RESUME_PENDING:
        return "pending";
    case RBRGEN3_RESUME_LOGGING:
        return "logging";
    case RBRGEN3_UNKNOWN_RESUME:
    default:
        return "unknown resume status";
    }
}

RBRGen3Error RBRGen3_getPauseresume(RBRGen3 *conn, RBRGen3PauseresumeState *state)
{
    /** To be safe, make *state = RBRGEN3_UNKNOWN_PAUSERESUME
     *  before using this function.
     */
    RBR_TRY(RBRGen3_converse(conn, "pauseresume"));

    char *command = NULL;
    RBRGen3ResponseParameter parameter;

    RBRGen3_parseResponse(conn, &command, &parameter);

    if (strcmp(parameter.key, "state") == 0) {
        for (int i = RBRGEN3_PAUSERESUME_NA; i < RBRGEN3_UNKNOWN_PAUSERESUME; i++) {
            /* refer to RBRGen3PauseresumeState_name */
            if (strcmp(RBRGen3PauseresumeState_name(i), parameter.value) == 0) {
                *state = i;
                return RBRGEN3_SUCCESS;
            }
        }
    } else {
        char *end = command + strlen(command);
        return RBRGen3_errorCheckResponse(conn, command, end);
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_pause(RBRGen3 *conn, RBRGen3PauseStatus *status)
{
    RBR_TRY(RBRGen3_converse(conn, "pause"));

    *status = RBRGEN3_UNKNOWN_PAUSE;
    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    RBRGen3_parseResponse(conn, &command, &parameter);
    if (strcmp(parameter.key, "status") == 0) {
        int i = RBRGEN3_PAUSE_PAUSED;
        /* refer to RBRGen3PauseStatus_name */
        if (strcmp(RBRGen3PauseStatus_name(i), parameter.value) == 0) {
            *status = i;
            return RBRGEN3_SUCCESS;
        }
    } else {
        char *end = command + strlen(command);
        return RBRGen3_errorCheckResponse(conn, command, end);
    }
    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_resume(RBRGen3 *conn, RBRGen3ResumeStatus *status)
{
    RBR_TRY(RBRGen3_converse(conn, "resume"));

    *status = RBRGEN3_UNKNOWN_RESUME;
    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    RBRGen3_parseResponse(conn, &command, &parameter);
    if (strcmp(parameter.key, "status") == 0) {
        for (int i = RBRGEN3_RESUME_PENDING; i < RBRGEN3_UNKNOWN_RESUME; i++) {
            /* refer to RBRGen3ResumeStatus_name */
            if (strcmp(RBRGen3ResumeStatus_name(i), parameter.value) == 0) {
                *status = i;
                return RBRGEN3_SUCCESS;
            }
        }
    } else {
        char *end = command + strlen(command);
        return RBRGen3_errorCheckResponse(conn, command, end);
    }
    return RBRGEN3_SUCCESS;
}
