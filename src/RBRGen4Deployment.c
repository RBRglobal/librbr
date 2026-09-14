/**
 * \file RBRGen4Deployment.c
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
#include "RBRGen4Deployment.h"

RBRGen4Error RBRGen4_getClock(RBRGen4 *instrument,
                                                 RBRGen4Clock *clock)
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

RBRGen4Error RBRGen4_setClock(
    RBRGen4 *instrument,
    const RBRGen4Clock *clock)
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

const char *RBRGen4DeploymentStatus_name(
    RBRGen4DeploymentStatus status)
{
    switch (status)
    {
    case RBRGEN4_DEPLOYMENT_STATUS_SAMPLING:
        return "sampling";
    case RBRGEN4_DEPLOYMENT_STATUS_GATED:
        return "gated";
    case RBRGEN4_DEPLOYMENT_STATUS_PAUSED:
        return "paused";
    case RBRGEN4_DEPLOYMENT_STATUS_INACTIVE:
        return "inactive";
    case RBRGEN4_DEPLOYMENT_STATUS_COUNT:
        return "deployment status count";
    case RBRGEN4_UNKNOWN_DEPLOYMENT_STATUS:
    default:
        return "unknown deployment status";
    }
}

/**
 * \brief Find the deployment status a response value names.
 *
 * \param [in] value the response value
 * \return the status, or #RBRGEN4_UNKNOWN_DEPLOYMENT_STATUS
 */
static RBRGen4DeploymentStatus
RBRGen4DeploymentStatus_parse(const char *value)
{
    for (int i = 0; i < RBRGEN4_DEPLOYMENT_STATUS_COUNT; i++)
    {
        if (strcmp(RBRGen4DeploymentStatus_name(i), value) == 0)
        {
            return i;
        }
    }

    return RBRGEN4_UNKNOWN_DEPLOYMENT_STATUS;
}

const char *RBRGen4Gate_name(RBRGen4Gate gate)
{
    switch (gate)
    {
    case RBRGEN4_GATE_NONE:
        return "none";
    case RBRGEN4_GATE_TIME:
        return "time";
    case RBRGEN4_GATE_TWISTACTIVATION:
        return "twistactivation";
    case RBRGEN4_GATE_WETSWITCH:
        return "wetswitch";
    case RBRGEN4_GATE_COUNT:
        return "gate count";
    case RBRGEN4_UNKNOWN_GATE:
    default:
        return "unknown gate";
    }
}

/**
 * \brief Find the gating condition a response value names.
 *
 * \param [in] value the response value
 * \return the condition, or #RBRGEN4_UNKNOWN_GATE
 */
static RBRGen4Gate RBRGen4Gate_parse(const char *value)
{
    for (int i = 0; i < RBRGEN4_GATE_COUNT; i++)
    {
        if (strcmp(RBRGen4Gate_name(i), value) == 0)
        {
            return i;
        }
    }

    return RBRGEN4_UNKNOWN_GATE;
}

RBRGen4Error RBRGen4_getDeployment(
    RBRGen4 *instrument,
    RBRGen4Deployment *deployment)
{
    memset(deployment, 0, sizeof(RBRGen4Deployment));

    deployment->status = RBRGEN4_UNKNOWN_DEPLOYMENT_STATUS;
    deployment->gate = RBRGEN4_UNKNOWN_GATE;

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
                RBRGen4DeploymentStatus_parse(parameter.value);
        }
        else if (strcmp(parameter.key, "gate") == 0)
        {
            deployment->gate = RBRGen4Gate_parse(parameter.value);
        }
        else if (strcmp(parameter.key, "simulation") == 0)
        {
            deployment->simulation = strcmp(parameter.value, "on") == 0;
        }
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_setDeployment(
    RBRGen4 *instrument,
    const RBRGen4Deployment *deployment)
{
    if (deployment->gate < 0
        || deployment->gate >= RBRGEN4_GATE_COUNT)
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    /* `starttime` is only available under time gating. */
    if (deployment->gate != RBRGEN4_GATE_TIME)
    {
        return RBRGen4_converse(
            instrument,
            "deployment gate=%s",
            RBRGen4Gate_name(deployment->gate));
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
        RBRGen4Gate_name(deployment->gate),
        startTime);
}

/**
 * \brief Read the deployment status a `pause` or `resume` response reports.
 *
 * \param [in] instrument the instrument connection
 * \param [out] status the reported status
 */
static void RBRGen4_parseDeploymentStatus(
    RBRGen4 *instrument,
    RBRGen4DeploymentStatus *status)
{
    *status = RBRGEN4_UNKNOWN_DEPLOYMENT_STATUS;

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
            *status = RBRGen4DeploymentStatus_parse(parameter.value);
            break;
        }
    }
}

RBRGen4Error RBRGen4_pause(
    RBRGen4 *instrument,
    RBRGen4DeploymentStatus *status)
{
    RBR_TRY(RBRGen4_converse(instrument, "pause"));

    RBRGen4_parseDeploymentStatus(instrument, status);

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_resume(
    RBRGen4 *instrument,
    RBRGen4DeploymentStatus *status)
{
    RBR_TRY(RBRGen4_converse(instrument, "resume"));

    RBRGen4_parseDeploymentStatus(instrument, status);

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
static void RBRGen4_parseInstrumentState(
    RBRGen4 *instrument,
    RBRGen4InstrumentState *state)
{
    *state = RBRGEN4_UNKNOWN_INSTRUMENT_STATE;

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

        for (int i = 0; i < RBRGEN4_INSTRUMENT_STATE_COUNT; i++)
        {
            if (strcmp(RBRGen4InstrumentState_name(i),
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
static RBRGen4Error RBRGen4_checkDeploymentParameters(
    const RBRGen4Config *config,
    const char *datasetLabel,
    RBRGen4DeploymentStoragemode storageMode)
{
    if (config == NULL
        || config->label[0] == '\0'
        || datasetLabel == NULL
        || datasetLabel[0] == '\0'
        || strlen(datasetLabel) > RBRGEN4_LABEL_NAME_MAX
        || (storageMode != RBRGEN4_STORAGEMODE_NORMAL
            && storageMode != RBRGEN4_STORAGEMODE_CALIBRATION))
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_verify(
    RBRGen4 *instrument,
    const RBRGen4Config *config,
    const char *datasetLabel,
    RBRGen4DeploymentStoragemode storageMode,
    RBRGen4InstrumentState *state)
{
    RBR_TRY(RBRGen4_checkDeploymentParameters(config,
                                                        datasetLabel,
                                                        storageMode));

    RBR_TRY(RBRGen4_converse(
                instrument,
                "verify config=%s dataset=%s storagemode=%s",
                config->label,
                datasetLabel,
                RBRGen4DeploymentStoragemode_name(storageMode)));

    RBRGen4_parseInstrumentState(instrument, state);

    return RBRGEN4_SUCCESS;
}

const char *RBRGen4DeploymentStoragemode_name(RBRGen4DeploymentStoragemode storageMode)
{
    switch (storageMode)
    {
    case RBRGEN4_STORAGEMODE_NORMAL:
        return "normal";
    case RBRGEN4_STORAGEMODE_CALIBRATION:
        return "calibration";
    case RBRGEN4_STORAGEMODE_COUNT:
        return "storage mode count";
    case RBRGEN4_UNKNOWN_STORAGEMODE:
    default:
        return "unknown storage mode";
    }
}

RBRGen4Error RBRGen4_enable(
    RBRGen4 *instrument,
    const RBRGen4Config *config,
    const char *datasetLabel,
    RBRGen4DeploymentStoragemode storageMode,
    RBRGen4InstrumentState *state)
{
    RBR_TRY(RBRGen4_checkDeploymentParameters(config,
                                                        datasetLabel,
                                                        storageMode));

    RBR_TRY(RBRGen4_converse(
                instrument,
                "enable config=%s dataset=%s storagemode=%s",
                config->label,
                datasetLabel,
                RBRGen4DeploymentStoragemode_name(storageMode)));

    RBRGen4_parseInstrumentState(instrument, state);

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_disable(
    RBRGen4 *instrument,
    RBRGen4InstrumentState *state)
{
    RBR_TRY(RBRGen4_converse(instrument, "disable"));

    RBRGen4_parseInstrumentState(instrument, state);

    return RBRGEN4_SUCCESS;
}
