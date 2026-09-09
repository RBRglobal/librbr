/**
 * \file RBRInstrumentDeployment.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for memset, strcmp. */
#include <string.h>

#include "RBRInstrumentGen3.h"
#include "RBRInstrumentInternal.h"

static RBRInstrumentError RBRInstrument_parseDeploymentResponse(
    RBRInstrument *instrument,
    const char *deploymentCommand,
    RBRInstrumentDeploymentStatus *status)
{
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    while (true)
    {
        RBRInstrument_parseResponse(instrument,
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

        for (int i = 0; i < RBRINSTRUMENT_STATUS_COUNT; i++)
        {
            if (strcmp(RBRInstrumentDeploymentStatus_name(i),
                       parameter.value) == 0)
            {
                *status = i;
                break;
            }
        }

        break;
    }

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_verify(
    RBRInstrument *instrument,
    bool eraseMemory,
    RBRInstrumentDeploymentStatus *status)
{
    *status = RBRINSTRUMENT_UNKNOWN_STATUS;

    RBR_TRY(RBRInstrument_converse(instrument,
                                   "verify erasememory = %s",
                                   (eraseMemory) ? "true" : "false"));

    return RBRInstrument_parseDeploymentResponse(instrument,
                                                 "verify",
                                                 status);
}

RBRInstrumentError RBRInstrument_enable(
    RBRInstrument *instrument,
    bool eraseMemory,
    RBRInstrumentDeploymentStatus *status)
{
    *status = RBRINSTRUMENT_UNKNOWN_STATUS;

    RBR_TRY(RBRInstrument_converse(instrument,
                                   "enable erasememory = %s",
                                   (eraseMemory) ? "true" : "false"));

    return RBRInstrument_parseDeploymentResponse(instrument,
                                                 "enable",
                                                 status);
}

RBRInstrumentError RBRInstrument_disable(
    RBRInstrument *instrument,
    RBRInstrumentDeploymentStatus *status)
{
    *status = RBRINSTRUMENT_UNKNOWN_STATUS;

    const char *disableCommand;
    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        disableCommand = "stop";
    }
    else
    {
        disableCommand = "disable";
    }

    RBR_TRY(RBRInstrument_converse(instrument, disableCommand));

    return RBRInstrument_parseDeploymentResponse(instrument,
                                                 disableCommand,
                                                 status);
}

RBRInstrumentError RBRInstrument_getSimulation(
    RBRInstrument *instrument,
    RBRInstrumentSimulation *simulation)
{
    memset(simulation, 0, sizeof(RBRInstrumentSimulation));

    RBR_TRY(RBRInstrument_converse(instrument, "simulation"));

    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    while (true)
    {
        RBRInstrument_parseResponse(instrument,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "state") == 0)
        {
            simulation->state = (strcmp(parameter.value, "on") == 0);
        }
        else if (strcmp(parameter.key, "period") == 0)
        {
            simulation->period = strtol(parameter.value, NULL, 10);
        }
    }

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_setSimulation(
    RBRInstrument *instrument,
    const RBRInstrumentSimulation *simulation)
{
    if (simulation->period <= 0)
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    RBR_TRY(RBRInstrument_permit(instrument, "simulation"));
    RBR_TRY(RBRInstrument_converse(instrument,
                                   "simulation state = %s, period = %i",
                                   (simulation->state) ? "on" : "off",
                                   simulation->period));
    return RBRINSTRUMENT_SUCCESS;
}
