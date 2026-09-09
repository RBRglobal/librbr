/**
 * \file RBRInstrumentGen3Pauseresume.c
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

const char *RBRInstrumentGen3PauseresumeState_name(RBRInstrumentGen3PauseresumeState state)
{
    switch (state)
    {
    /* Either the deployment has not been enabled or the samling mode is 'regimes' */
    case RBRINSTRUMENTGEN3_PAUSERESUME_NA:
        return "n/a";
    /* The deployment has been enabled and is paused */
    case RBRINSTRUMENTGEN3_PAUSERESUME_PAUSED:
        return "paused";
    /* The deployment has been enabled and is not paused */
    case RBRINSTRUMENTGEN3_PAUSERESUME_RUNNING:
        return "running";
    /* The feature is not allowed on this instrument */
    case RBRINSTRUMENTGEN3_UNKNOWN_PAUSERESUME:
    default:
        return "unknown pauseresume state";
    }
}

const char *RBRInstrumentGen3PauseStatus_name(RBRInstrumentGen3PauseStatus status)
{
    switch (status)
    {
    case RBRINSTRUMENTGEN3_PAUSE_PAUSED:
        return "paused";
    case RBRINSTRUMENTGEN3_UNKNOWN_PAUSE:
    default:
        return "unknown pause status";
    }
}

const char *RBRInstrumentGen3ResumeStatus_name(RBRInstrumentGen3ResumeStatus status)
{
    switch (status)
    {
    case RBRINSTRUMENTGEN3_RESUME_PENDING:
        return "pending";
    case RBRINSTRUMENTGEN3_RESUME_LOGGING:
        return "logging";
    case RBRINSTRUMENTGEN3_UNKNOWN_RESUME:
    default:
        return "unknown resume status";
    }
}

RBRGen3Error RBRInstrumentGen3_getPauseresume(RBRGen3 *instrument,
                                                RBRInstrumentGen3PauseresumeState *state)
{
    /** To be safe, make *state = RBRINSTRUMENTGEN3_UNKNOWN_PAUSERESUME
     *  before using this function.
     */
    RBR_TRY(RBRGen3_converse(instrument, "pauseresume"));

    char *command = NULL;
    RBRGen3ResponseParameter parameter;

    RBRGen3_parseResponse(instrument, &command, &parameter);

    if (strcmp(parameter.key, "state") == 0)
    {
        for (int i = RBRINSTRUMENTGEN3_PAUSERESUME_NA; i < RBRINSTRUMENTGEN3_UNKNOWN_PAUSERESUME; i++)
        {
            /* refer to RBRInstrumentGen3PauseresumeState_name */
            if (strcmp(RBRInstrumentGen3PauseresumeState_name(i), parameter.value) == 0)
            {
                *state = i;
                return RBRGEN3_SUCCESS;
            }
        }
    }
    else
    {
        char *end = command + strlen(command);
        return RBRGen3_errorCheckResponse(instrument, command, end);
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRInstrumentGen3_pause(RBRGen3 *instrument,
                                       RBRInstrumentGen3PauseStatus *status)
{
    RBR_TRY(RBRGen3_converse(instrument, "pause"));

    *status = RBRINSTRUMENTGEN3_UNKNOWN_PAUSE;
    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    RBRGen3_parseResponse(instrument, &command, &parameter);
    if (strcmp(parameter.key, "status") == 0)
    {
        int i = RBRINSTRUMENTGEN3_PAUSE_PAUSED;
        /* refer to RBRInstrumentGen3PauseStatus_name */
        if (strcmp(RBRInstrumentGen3PauseStatus_name(i), parameter.value) == 0)
        {
            *status = i;
            return RBRGEN3_SUCCESS;
        }
    }
    else
    {
        char *end = command + strlen(command);
        return RBRGen3_errorCheckResponse(instrument, command, end);
    }
    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRInstrumentGen3_resume(RBRGen3 *instrument,
                                        RBRInstrumentGen3ResumeStatus *status)
{
    RBR_TRY(RBRGen3_converse(instrument, "resume"));

    *status = RBRINSTRUMENTGEN3_UNKNOWN_RESUME;
    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    RBRGen3_parseResponse(instrument, &command, &parameter);
    if (strcmp(parameter.key, "status") == 0)
    {
        for (int i = RBRINSTRUMENTGEN3_RESUME_PENDING; i < RBRINSTRUMENTGEN3_UNKNOWN_RESUME; i++)
        {
            /* refer to RBRInstrumentGen3ResumeStatus_name */
            if (strcmp(RBRInstrumentGen3ResumeStatus_name(i), parameter.value) == 0)
            {
                *status = i;
                return RBRGEN3_SUCCESS;
            }
        }
    }
    else
    {
        char *end = command + strlen(command);
        return RBRGen3_errorCheckResponse(instrument, command, end);
    }
    return RBRGEN3_SUCCESS;
}
