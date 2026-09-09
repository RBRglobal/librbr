/**
 * \file RBRInstrumentPauseresume.c
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

#include "RBRInstrumentGen3.h"
#include "RBRInstrumentGen3Internal.h"

const char *RBRInstrumentPauseresumeState_name(RBRInstrumentPauseresumeState state)
{
    switch (state)
    {
    /* Either the deployment has not been enabled or the samling mode is 'regimes' */
    case RBRINSTRUMENT_PAUSERESUME_NA:
        return "n/a";
    /* The deployment has been enabled and is paused */
    case RBRINSTRUMENT_PAUSERESUME_PAUSED:
        return "paused";
    /* The deployment has been enabled and is not paused */
    case RBRINSTRUMENT_PAUSERESUME_RUNNING:
        return "running";
    /* The feature is not allowed on this instrument */
    case RBRINSTRUMENT_UNKNOWN_PAUSERESUME:
    default:
        return "unknown pauseresume state";
    }
}

const char *RBRInstrumentPauseStatus_name(RBRInstrumentPauseStatus status)
{
    switch (status)
    {
    case RBRINSTRUMENT_PAUSE_PAUSED:
        return "paused";
    case RBRINSTRUMENT_UNKNOWN_PAUSE:
    default:
        return "unknown pause status";
    }
}

const char *RBRInstrumentResumeStatus_name(RBRInstrumentResumeStatus status)
{
    switch (status)
    {
    case RBRINSTRUMENT_RESUME_PENDING:
        return "pending";
    case RBRINSTRUMENT_RESUME_LOGGING:
        return "logging";
    case RBRINSTRUMENT_UNKNOWN_RESUME:
    default:
        return "unknown resume status";
    }
}

RBRInstrumentError RBRInstrument_getPauseresume(RBRInstrument *instrument,
                                                RBRInstrumentPauseresumeState *state)
{
    /** To be safe, make *state = RBRINSTRUMENT_UNKNOWN_PAUSERESUME
     *  before using this function.
     */
    RBR_TRY(RBRInstrument_converse(instrument, "pauseresume"));

    char *command = NULL;
    RBRInstrumentResponseParameter parameter;

    RBRInstrument_parseResponse(instrument, &command, &parameter);

    if (strcmp(parameter.key, "state") == 0)
    {
        for (int i = RBRINSTRUMENT_PAUSERESUME_NA; i < RBRINSTRUMENT_UNKNOWN_PAUSERESUME; i++)
        {
            /* refer to RBRInstrumentPauseresumeState_name */
            if (strcmp(RBRInstrumentPauseresumeState_name(i), parameter.value) == 0)
            {
                *state = i;
                return RBRINSTRUMENT_SUCCESS;
            }
        }
    }
    else
    {
        char *end = command + strlen(command);
        return RBRInstrument_errorCheckResponse(instrument, command, end);
    }

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_pause(RBRInstrument *instrument,
                                       RBRInstrumentPauseStatus *status)
{
    RBR_TRY(RBRInstrument_converse(instrument, "pause"));

    *status = RBRINSTRUMENT_UNKNOWN_PAUSE;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    RBRInstrument_parseResponse(instrument, &command, &parameter);
    if (strcmp(parameter.key, "status") == 0)
    {
        int i = RBRINSTRUMENT_PAUSE_PAUSED;
        /* refer to RBRInstrumentPauseStatus_name */
        if (strcmp(RBRInstrumentPauseStatus_name(i), parameter.value) == 0)
        {
            *status = i;
            return RBRINSTRUMENT_SUCCESS;
        }
    }
    else
    {
        char *end = command + strlen(command);
        return RBRInstrument_errorCheckResponse(instrument, command, end);
    }
    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_resume(RBRInstrument *instrument,
                                        RBRInstrumentResumeStatus *status)
{
    RBR_TRY(RBRInstrument_converse(instrument, "resume"));

    *status = RBRINSTRUMENT_UNKNOWN_RESUME;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    RBRInstrument_parseResponse(instrument, &command, &parameter);
    if (strcmp(parameter.key, "status") == 0)
    {
        for (int i = RBRINSTRUMENT_RESUME_PENDING; i < RBRINSTRUMENT_UNKNOWN_RESUME; i++)
        {
            /* refer to RBRInstrumentResumeStatus_name */
            if (strcmp(RBRInstrumentResumeStatus_name(i), parameter.value) == 0)
            {
                *status = i;
                return RBRINSTRUMENT_SUCCESS;
            }
        }
    }
    else
    {
        char *end = command + strlen(command);
        return RBRInstrument_errorCheckResponse(instrument, command, end);
    }
    return RBRINSTRUMENT_SUCCESS;
}
