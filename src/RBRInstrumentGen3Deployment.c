/**
 * \file RBRInstrumentGen3Deployment.c
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
#include "RBRInstrumentGen3Internal.h"

static RBRInstrumentGen3Error RBRInstrumentGen3_parseDeploymentResponse(
    RBRInstrumentGen3 *instrument,
    const char *deploymentCommand,
    RBRInstrumentGen3DeploymentStatus *status)
{
    char *command = NULL;
    RBRInstrumentGen3ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen3_parseResponse(instrument,
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

        for (int i = 0; i < RBRINSTRUMENTGEN3_STATUS_COUNT; i++)
        {
            if (strcmp(RBRInstrumentGen3DeploymentStatus_name(i),
                       parameter.value) == 0)
            {
                *status = i;
                break;
            }
        }

        break;
    }

    return RBRINSTRUMENTGEN3_SUCCESS;
}

RBRInstrumentGen3Error RBRInstrumentGen3_verify(
    RBRInstrumentGen3 *instrument,
    bool eraseMemory,
    RBRInstrumentGen3DeploymentStatus *status)
{
    *status = RBRINSTRUMENTGEN3_UNKNOWN_STATUS;

    RBR_TRY(RBRInstrumentGen3_converse(instrument,
                                   "verify erasememory = %s",
                                   (eraseMemory) ? "true" : "false"));

    return RBRInstrumentGen3_parseDeploymentResponse(instrument,
                                                 "verify",
                                                 status);
}

RBRInstrumentGen3Error RBRInstrumentGen3_enable(
    RBRInstrumentGen3 *instrument,
    bool eraseMemory,
    RBRInstrumentGen3DeploymentStatus *status)
{
    *status = RBRINSTRUMENTGEN3_UNKNOWN_STATUS;

    RBR_TRY(RBRInstrumentGen3_converse(instrument,
                                   "enable erasememory = %s",
                                   (eraseMemory) ? "true" : "false"));

    return RBRInstrumentGen3_parseDeploymentResponse(instrument,
                                                 "enable",
                                                 status);
}

RBRInstrumentGen3Error RBRInstrumentGen3_disable(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3DeploymentStatus *status)
{
    *status = RBRINSTRUMENTGEN3_UNKNOWN_STATUS;

    const char *disableCommand;
    if (instrument->generation == RBRINSTRUMENTGEN3_LOGGER2)
    {
        disableCommand = "stop";
    }
    else
    {
        disableCommand = "disable";
    }

    RBR_TRY(RBRInstrumentGen3_converse(instrument, disableCommand));

    return RBRInstrumentGen3_parseDeploymentResponse(instrument,
                                                 disableCommand,
                                                 status);
}

RBRInstrumentGen3Error RBRInstrumentGen3_getSimulation(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3Simulation *simulation)
{
    memset(simulation, 0, sizeof(RBRInstrumentGen3Simulation));

    RBR_TRY(RBRInstrumentGen3_converse(instrument, "simulation"));

    char *command = NULL;
    RBRInstrumentGen3ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen3_parseResponse(instrument,
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

    return RBRINSTRUMENTGEN3_SUCCESS;
}

RBRInstrumentGen3Error RBRInstrumentGen3_setSimulation(
    RBRInstrumentGen3 *instrument,
    const RBRInstrumentGen3Simulation *simulation)
{
    if (simulation->period <= 0)
    {
        return RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE;
    }

    RBR_TRY(RBRInstrumentGen3_permit(instrument, "simulation"));
    RBR_TRY(RBRInstrumentGen3_converse(instrument,
                                   "simulation state = %s, period = %i",
                                   (simulation->state) ? "on" : "off",
                                   simulation->period));
    return RBRINSTRUMENTGEN3_SUCCESS;
}
