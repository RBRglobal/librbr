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

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Internal.h"
#include "RBRInstrumentGen4Deployment.h"

RBRInstrumentGen4Error RBRInstrumentGen4_getClock(RBRInstrumentGen4 *instrument,
                                                 RBRInstrumentGen4Clock *clock)
{
    clock->dateTime = 0;
    clock->offsetFromUtc = NAN;

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "clock"));

    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "datetime") == 0)
        {
            RBR_TRY(RBRInstrumentGen4DateTime_parseScheduleTime(
                        parameter.value,
                        &clock->dateTime,
                        NULL));
        }
        else if (strcmp(parameter.key, "offsetfromutc") == 0)
        {
            clock->offsetFromUtc = strtod(parameter.value, NULL);
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setClock(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Clock *clock)
{
    if (clock->dateTime < RBRINSTRUMENTGEN4_DATETIME_MIN
        || clock->dateTime > RBRINSTRUMENTGEN4_DATETIME_MAX)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    if (isnan(clock->offsetFromUtc))
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    char dateTime[RBRINSTRUMENTGEN4_SCHEDULE_TIME_LEN + 1];
    RBRInstrumentGen4DateTime_toScheduleTime(clock->dateTime, dateTime);

    return RBRInstrumentGen4_converse(instrument,
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

RBRInstrumentGen4Error RBRInstrumentGen4_getDeployment(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Deployment *deployment)
{
    memset(deployment, 0, sizeof(RBRInstrumentGen4Deployment));

    /* Cast away const to reach the read-only parameters of the response. */
    RBRInstrumentGen4DeploymentStatus *status =
        (RBRInstrumentGen4DeploymentStatus *) &deployment->status;
    bool *simulation = (bool *) &deployment->simulation;

    *status = RBRINSTRUMENTGEN4_UNKNOWN_DEPLOYMENT_STATUS;
    deployment->gate = RBRINSTRUMENTGEN4_UNKNOWN_GATE;

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "deployment"));

    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "starttime") == 0)
        {
            RBR_TRY(RBRInstrumentGen4DateTime_parseScheduleTime(
                        parameter.value,
                        &deployment->startTime,
                        NULL));
        }
        else if (strcmp(parameter.key, "status") == 0)
        {
            *status = RBRInstrumentGen4DeploymentStatus_parse(parameter.value);
        }
        else if (strcmp(parameter.key, "gate") == 0)
        {
            deployment->gate = RBRInstrumentGen4Gate_parse(parameter.value);
        }
        else if (strcmp(parameter.key, "simulation") == 0)
        {
            *simulation = strcmp(parameter.value, "on") == 0;
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setDeployment(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Deployment *deployment)
{
    if (deployment->gate < 0
        || deployment->gate >= RBRINSTRUMENTGEN4_GATE_COUNT)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    /* `starttime` is only available under time gating. */
    if (deployment->gate != RBRINSTRUMENTGEN4_GATE_TIME)
    {
        return RBRInstrumentGen4_converse(
            instrument,
            "deployment gate=%s",
            RBRInstrumentGen4Gate_name(deployment->gate));
    }

    if (deployment->startTime < RBRINSTRUMENTGEN4_DATETIME_MIN
        || deployment->startTime > RBRINSTRUMENTGEN4_DATETIME_MAX)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    char startTime[RBRINSTRUMENTGEN4_SCHEDULE_TIME_LEN + 1];
    RBRInstrumentGen4DateTime_toScheduleTime(deployment->startTime, startTime);

    return RBRInstrumentGen4_converse(
        instrument,
        "deployment gate=%s starttime=%s",
        RBRInstrumentGen4Gate_name(deployment->gate),
        startTime);
}

RBRInstrumentGen4Error RBRInstrumentGen4_pause(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4DeploymentStatus *status)
{
    (void)instrument;
    (void)status;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_resume(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4DeploymentStatus *status)
{
    (void)instrument;
    (void)status;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

static RBRInstrumentGen4Error RBRInstrumentGen4_parseDeploymentResponse(
    RBRInstrumentGen4 *instrument,
    const char *deploymentCommand,
    RBRInstrumentGen4InstrumentState *state)
{
    //GEN4 todo: maybe make this consistent with other code, like get xxx name function.
    /** 
     * This function gets a human readable name for status. 
     * used in functions RBRInstrumentGen4_verify(),RBRInstrumentGen4_enable(), RBRInstrumentGen4_disable()
     * */
    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "status") != 0
                 && strcmp(parameter.key, deploymentCommand) != 0)
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

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_verify(
        RBRInstrumentGen4 *instrument,
        const RBRInstrumentGen4Config *config, 
        const char datasetLabel[],
        RBRInstrumentGen4InstrumentState *status)
{
    RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                   "verify config=%s dataset=%s", config->label, datasetLabel));

    return RBRInstrumentGen4_parseDeploymentResponse(instrument,
                                                 "verify",
                                                 status);
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
        return "sampling mode count";
    case RBRINSTRUMENTGEN4_UNKNOWN_STORAGEMODE:
    default:
        return "unknown sampling mode";
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_enable(
        RBRInstrumentGen4 *instrument,
        const RBRInstrumentGen4Config *config,
        const char datasetLabel[], 
        const RBRInstrumentGen4DeploymentStoragemode storageMode, 
        RBRInstrumentGen4DatasetPool *datasetPool,
        RBRInstrumentGen4Dataset **newDataset,
        RBRInstrumentGen4InstrumentState *state)
{
    if (storageMode != RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL
        && storageMode != RBRINSTRUMENTGEN4_STORAGEMODE_CALIBRATION)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    RBR_TRY(RBRInstrumentGen4_converse(
        instrument,
        "enable config=%s dataset=%s storagemode=%s",
        config->label,
        datasetLabel,
        RBRInstrumentGen4DeploymentStoragemode_name(storageMode)));

    RBRInstrumentGen4_parseDeploymentResponse(instrument,
                                              "enable",
                                              state);
    
    *newDataset = &datasetPool->pool[datasetPool->count];
    datasetPool->count += 1;
    strcpy((*newDataset)->label, datasetLabel);

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_disable(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4InstrumentState *state)
{
    RBR_TRY(RBRInstrumentGen4_converse(instrument, "disable"));

    return RBRInstrumentGen4_parseDeploymentResponse(instrument,
                                                 "disable",
                                                 state);
}
