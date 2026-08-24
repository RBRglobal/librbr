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
            RBR_TRY(RBRInstrumentGen4DateTime_parseScheduleTime(parameter.value,
                                                            &clock->dateTime,
                                                            NULL));
        }
        else if (strcmp(parameter.key, "offsetfromutc") == 0
                 && strcmp(parameter.value, "unknown") != 0)
        {
            clock->offsetFromUtc = strtod(parameter.value, NULL);
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setClock(RBRInstrumentGen4 *instrument,
                                          const RBRInstrumentGen4Clock *clock)
{
    if (clock->dateTime < RBRINSTRUMENTGEN4_DATETIME_MIN
        || clock->dateTime > RBRINSTRUMENTGEN4_DATETIME_MAX)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    char dateTime[RBRINSTRUMENTGEN4_SCHEDULE_TIME_LEN + 1];
    RBRInstrumentGen4DateTime_toScheduleTime(clock->dateTime, dateTime);

    if (instrument->generation == RBRINSTRUMENTGEN4_LOGGER4)
    {
        float offsetFromUtc = (clock->offsetFromUtc);
        if (!isnan(offsetFromUtc))
        {
            return RBRInstrumentGen4_converse(
                instrument,
                "clock datetime=%s offsetfromutc=%02f",
                dateTime,
                (double) offsetFromUtc);
        }
        else
        {
            return RBRInstrumentGen4_converse(instrument,
                                        "clock datetime=%s",
                                        dateTime);
        }
    }
    else
    {
        return RBRINSTRUMENTGEN4_UNSUPPORTED;
    }
}
const char *RBRInstrumentGen4DeploymentStatus_name(
    RBRInstrumentGen4DeploymentStatus status)
{
    switch (status)
    {
    case RBRINSTRUMENTGEN4_STATUS_SAMPLING:
        return "sampling";
    case RBRINSTRUMENTGEN4_STATUS_GATED:
        return "gated";
    case RBRINSTRUMENTGEN4_STATUS_PAUSED:
        return "paused";
    case RBRINSTRUMENTGEN4_STATUS_INACTIVE:
        return "inactive";
    case RBRINSTRUMENTGEN4_STATUS_UNKNOWN:
        return "unknown";
    case RBRINSTRUMENTGEN4_STATUS_COUNT:
        return "status count";
    case RBRINSTRUMENTGEN4_UNKNOWN_STATUS:
    default:
        return "unknown status";
    }
}

const char *RBRInstrumentGen4Gate_name(RBRInstrumentGen4Gate gate)
{
    switch (gate)
    {
    case RBRINSTRUMENTGEN4_GATE_NONE:
        return "none";
    case RBRINSTRUMENTGEN4_GATE_THRESHOLDING:
        return "thresholding";
    case RBRINSTRUMENTGEN4_GATE_TWISTACTIVATION:
        return "twistactivation";
    case RBRINSTRUMENTGEN4_GATE_INVALID:
        return "invalid";
    case RBRINSTRUMENTGEN4_GATE_COUNT:
        return "gate count";
    case RBRINSTRUMENTGEN4_UNKNOWN_GATE:
    default:
        return "unknown gate";
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_getDeployment(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Deployment *deployment)
{
    (void)instrument;
    (void)deployment;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setDeployment(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Deployment *deployment)
{
    if (deployment->startTime < RBRINSTRUMENTGEN4_DATETIME_MIN
        || deployment->startTime > RBRINSTRUMENTGEN4_DATETIME_MAX)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    char startTime[RBRINSTRUMENTGEN4_SCHEDULE_TIME_LEN + 1];
    RBRInstrumentGen4DateTime_toScheduleTime(deployment->startTime, startTime);

    if (instrument->generation == RBRINSTRUMENTGEN4_LOGGER4)
    {
        RBR_TRY(RBRInstrumentGen4_converse(
                    instrument,
                    "deployment starttime=%s",
                    startTime));
        return RBRINSTRUMENTGEN4_SUCCESS;
    }
    else
    {
        return RBRINSTRUMENTGEN4_UNSUPPORTED;
    }
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
