/**
 * \file RBRInstrumentGen4Deployment.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for isnan, NAN. */
#include <math.h>
/* Required for memset, strcmp. */
#include <stdlib.h>
#include <string.h>
/* Required for snprintf. */
#include <stdio.h>
/* Required for PRId32. */
#include <inttypes.h>

#include "RBRGen4.h"
#include "RBRGen4Internal.h"
#include "RBRInstrumentGen4Deployment.h"

RBRGen4Error RBRInstrumentGen4_getClock(RBRGen4 *instrument,
                                                 RBRInstrumentGen4Clock *clock)
{
    clock->dateTime = 0;
    clock->offsetFromUtc = NAN;

    RBR_TRY(RBRGen4_converse(instrument, "clock"));

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "datetime") == 0)
        {
            RBR_TRY(RBRGen4DateTime_parseScheduleTime(
                        parameter.value,
                        &clock->dateTime,
                        NULL));
        }
        else if (strcmp(parameter.key, "offsetfromutc") == 0)
        {
            clock->offsetFromUtc = strtod(parameter.value, NULL);
        }
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRInstrumentGen4_setClock(
    RBRGen4 *instrument,
    const RBRInstrumentGen4Clock *clock)
{
    if (clock->dateTime < RBRGEN4_DATETIME_MIN
        || clock->dateTime > RBRGEN4_DATETIME_MAX)
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    if (isnan(clock->offsetFromUtc))
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    char dateTime[RBRGEN4_SCHEDULE_TIME_LEN + 1];
    RBRGen4DateTime_toScheduleTime(clock->dateTime, dateTime);

    return RBRGen4_converse(instrument,
                                      "clock datetime=%s offsetfromutc=%.2f",
                                      dateTime,
                                      (double) clock->offsetFromUtc);
}

const char *RBRInstrumentGen4DeploymentStatus_name(
    RBRInstrumentGen4DeploymentStatus status)
{
    switch (status)
    {
    case RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_SAMPLING:
        return "sampling";
    case RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_GATED:
        return "gated";
    case RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_PAUSED:
        return "paused";
    case RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_INACTIVE:
        return "inactive";
    case RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_COUNT:
        return "deployment status count";
    case RBRINSTRUMENTGEN4_UNKNOWN_DEPLOYMENT_STATUS:
    default:
        return "unknown deployment status";
    }
}

/**
 * \brief Find the deployment status a response value names.
 *
 * \param [in] value the response value
 * \return the status, or #RBRINSTRUMENTGEN4_UNKNOWN_DEPLOYMENT_STATUS
 */
static RBRInstrumentGen4DeploymentStatus
RBRInstrumentGen4DeploymentStatus_parse(const char *value)
{
    for (int i = 0; i < RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_COUNT; i++)
    {
        if (strcmp(RBRInstrumentGen4DeploymentStatus_name(i), value) == 0)
        {
            return i;
        }
    }

    return RBRINSTRUMENTGEN4_UNKNOWN_DEPLOYMENT_STATUS;
}

const char *RBRInstrumentGen4Gate_name(RBRInstrumentGen4Gate gate)
{
    switch (gate)
    {
    case RBRINSTRUMENTGEN4_GATE_NONE:
        return "none";
    case RBRINSTRUMENTGEN4_GATE_TIME:
        return "time";
    case RBRINSTRUMENTGEN4_GATE_TWISTACTIVATION:
        return "twistactivation";
    case RBRINSTRUMENTGEN4_GATE_WETSWITCH:
        return "wetswitch";
    case RBRINSTRUMENTGEN4_GATE_COUNT:
        return "gate count";
    case RBRINSTRUMENTGEN4_UNKNOWN_GATE:
    default:
        return "unknown gate";
    }
}

/**
 * \brief Find the gating condition a response value names.
 *
 * \param [in] value the response value
 * \return the condition, or #RBRINSTRUMENTGEN4_UNKNOWN_GATE
 */
static RBRInstrumentGen4Gate RBRInstrumentGen4Gate_parse(const char *value)
{
    for (int i = 0; i < RBRINSTRUMENTGEN4_GATE_COUNT; i++)
    {
        if (strcmp(RBRInstrumentGen4Gate_name(i), value) == 0)
        {
            return i;
        }
    }

    return RBRINSTRUMENTGEN4_UNKNOWN_GATE;
}

RBRGen4Error RBRInstrumentGen4_getDeployment(
    RBRGen4 *instrument,
    RBRInstrumentGen4Deployment *deployment)
{
    memset(deployment, 0, sizeof(RBRInstrumentGen4Deployment));

    deployment->status = RBRINSTRUMENTGEN4_UNKNOWN_DEPLOYMENT_STATUS;
    deployment->gate = RBRINSTRUMENTGEN4_UNKNOWN_GATE;

    RBR_TRY(RBRGen4_converse(instrument, "deployment"));

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "starttime") == 0)
        {
            RBR_TRY(RBRGen4DateTime_parseScheduleTime(
                        parameter.value,
                        &deployment->startTime,
                        NULL));
        }
        else if (strcmp(parameter.key, "status") == 0)
        {
            deployment->status =
                RBRInstrumentGen4DeploymentStatus_parse(parameter.value);
        }
        else if (strcmp(parameter.key, "gate") == 0)
        {
            deployment->gate = RBRInstrumentGen4Gate_parse(parameter.value);
        }
        else if (strcmp(parameter.key, "simulation") == 0)
        {
            deployment->simulation = strcmp(parameter.value, "on") == 0;
        }
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRInstrumentGen4_setDeployment(
    RBRGen4 *instrument,
    const RBRInstrumentGen4Deployment *deployment)
{
    if (deployment->gate < 0
        || deployment->gate >= RBRINSTRUMENTGEN4_GATE_COUNT)
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    /* `starttime` is only available under time gating. */
    if (deployment->gate != RBRINSTRUMENTGEN4_GATE_TIME)
    {
        return RBRGen4_converse(
            instrument,
            "deployment gate=%s",
            RBRInstrumentGen4Gate_name(deployment->gate));
    }

    if (deployment->startTime < RBRGEN4_DATETIME_MIN
        || deployment->startTime > RBRGEN4_DATETIME_MAX)
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    char startTime[RBRGEN4_SCHEDULE_TIME_LEN + 1];
    RBRGen4DateTime_toScheduleTime(deployment->startTime, startTime);

    return RBRGen4_converse(
        instrument,
        "deployment gate=%s starttime=%s",
        RBRInstrumentGen4Gate_name(deployment->gate),
        startTime);
}

/**
 * \brief Read the deployment status a `pause` or `resume` response reports.
 *
 * \param [in] instrument the instrument connection
 * \param [out] status the reported status
 */
static void RBRInstrumentGen4_parseDeploymentStatus(
    RBRGen4 *instrument,
    RBRInstrumentGen4DeploymentStatus *status)
{
    *status = RBRINSTRUMENTGEN4_UNKNOWN_DEPLOYMENT_STATUS;

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "status") == 0)
        {
            *status = RBRInstrumentGen4DeploymentStatus_parse(parameter.value);
            break;
        }
    }
}

RBRGen4Error RBRInstrumentGen4_pause(
    RBRGen4 *instrument,
    RBRInstrumentGen4DeploymentStatus *status)
{
    RBR_TRY(RBRGen4_converse(instrument, "pause"));

    RBRInstrumentGen4_parseDeploymentStatus(instrument, status);

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRInstrumentGen4_resume(
    RBRGen4 *instrument,
    RBRInstrumentGen4DeploymentStatus *status)
{
    RBR_TRY(RBRGen4_converse(instrument, "resume"));

    RBRInstrumentGen4_parseDeploymentStatus(instrument, status);

    return RBRGEN4_SUCCESS;
}

/**
 * \brief Read the instrument state a deployment response reports.
 *
 * Every other parameter of the response is an echo of what was sent.
 *
 * \param [in] instrument the instrument connection
 * \param [out] state the reported state
 */
static void RBRInstrumentGen4_parseInstrumentState(
    RBRGen4 *instrument,
    RBRInstrumentGen4InstrumentState *state)
{
    *state = RBRINSTRUMENTGEN4_UNKNOWN_INSTRUMENT_STATE;

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "state") != 0)
        {
            continue;
        }

        for (int i = 0; i < RBRINSTRUMENTGEN4_INSTRUMENT_STATE_COUNT; i++)
        {
            if (strcmp(RBRInstrumentGen4InstrumentState_name(i),
                       parameter.value) == 0)
            {
                *state = i;
                break;
            }
        }

        break;
    }
}

/**
 * \brief Check the parameters `verify` and `enable` share.
 *
 * \param [in] config the configuration to deploy
 * \param [in] datasetLabel the label for the deployment's dataset
 * \param [in] storageMode the data storage mode
 * \return #RBRGEN4_SUCCESS when the parameters are all in range
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE otherwise
 */
static RBRGen4Error RBRInstrumentGen4_checkDeploymentParameters(
    const RBRGen4Config *config,
    const char *datasetLabel,
    RBRInstrumentGen4DeploymentStoragemode storageMode)
{
    if (config == NULL
        || config->label[0] == '\0'
        || datasetLabel == NULL
        || datasetLabel[0] == '\0'
        || strlen(datasetLabel) > RBRGEN4_LABEL_NAME_MAX
        || (storageMode != RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL
            && storageMode != RBRINSTRUMENTGEN4_STORAGEMODE_CALIBRATION))
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRInstrumentGen4_verify(
    RBRGen4 *instrument,
    const RBRGen4Config *config,
    const char *datasetLabel,
    RBRInstrumentGen4DeploymentStoragemode storageMode,
    RBRInstrumentGen4InstrumentState *state)
{
    RBR_TRY(RBRInstrumentGen4_checkDeploymentParameters(config,
                                                        datasetLabel,
                                                        storageMode));

    RBR_TRY(RBRGen4_converse(
                instrument,
                "verify config=%s dataset=%s storagemode=%s",
                config->label,
                datasetLabel,
                RBRInstrumentGen4DeploymentStoragemode_name(storageMode)));

    RBRInstrumentGen4_parseInstrumentState(instrument, state);

    return RBRGEN4_SUCCESS;
}

const char *RBRInstrumentGen4DeploymentStoragemode_name(RBRInstrumentGen4DeploymentStoragemode storageMode)
{
    switch (storageMode)
    {
    case RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL:
        return "normal";
    case RBRINSTRUMENTGEN4_STORAGEMODE_CALIBRATION:
        return "calibration";
    case RBRINSTRUMENTGEN4_STORAGEMODE_COUNT:
        return "storage mode count";
    case RBRINSTRUMENTGEN4_UNKNOWN_STORAGEMODE:
    default:
        return "unknown storage mode";
    }
}

RBRGen4Error RBRInstrumentGen4_enable(
    RBRGen4 *instrument,
    const RBRGen4Config *config,
    const char *datasetLabel,
    RBRInstrumentGen4DeploymentStoragemode storageMode,
    RBRInstrumentGen4InstrumentState *state)
{
    RBR_TRY(RBRInstrumentGen4_checkDeploymentParameters(config,
                                                        datasetLabel,
                                                        storageMode));

    RBR_TRY(RBRGen4_converse(
                instrument,
                "enable config=%s dataset=%s storagemode=%s",
                config->label,
                datasetLabel,
                RBRInstrumentGen4DeploymentStoragemode_name(storageMode)));

    RBRInstrumentGen4_parseInstrumentState(instrument, state);

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRInstrumentGen4_disable(
    RBRGen4 *instrument,
    RBRInstrumentGen4InstrumentState *state)
{
    RBR_TRY(RBRGen4_converse(instrument, "disable"));

    RBRInstrumentGen4_parseInstrumentState(instrument, state);

    return RBRGEN4_SUCCESS;
}
