/**
 * \file RBRGen3Deployment.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for memset, strcmp. */
#include <string.h>

#include "RBRGen3.h"
#include "RBRGen3Internal.h"

static RBRGen3Error RBRGen3_parseDeploymentResponse(
    RBRGen3 *instrument,
    const char *deploymentCommand,
    RBRInstrumentGen3DeploymentStatus *status)
{
    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true)
    {
        RBRGen3_parseResponse(instrument,
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

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_verify(
    RBRGen3 *instrument,
    bool eraseMemory,
    RBRInstrumentGen3DeploymentStatus *status)
{
    *status = RBRINSTRUMENTGEN3_UNKNOWN_STATUS;

    RBR_TRY(RBRGen3_converse(instrument,
                                   "verify erasememory = %s",
                                   (eraseMemory) ? "true" : "false"));

    return RBRGen3_parseDeploymentResponse(instrument,
                                                 "verify",
                                                 status);
}

RBRGen3Error RBRGen3_enable(
    RBRGen3 *instrument,
    bool eraseMemory,
    RBRInstrumentGen3DeploymentStatus *status)
{
    *status = RBRINSTRUMENTGEN3_UNKNOWN_STATUS;

    RBR_TRY(RBRGen3_converse(instrument,
                                   "enable erasememory = %s",
                                   (eraseMemory) ? "true" : "false"));

    return RBRGen3_parseDeploymentResponse(instrument,
                                                 "enable",
                                                 status);
}

RBRGen3Error RBRGen3_disable(
    RBRGen3 *instrument,
    RBRInstrumentGen3DeploymentStatus *status)
{
    *status = RBRINSTRUMENTGEN3_UNKNOWN_STATUS;

    const char *disableCommand;
    if (instrument->generation == RBRGEN3_LOGGER2)
    {
        disableCommand = "stop";
    }
    else
    {
        disableCommand = "disable";
    }

    RBR_TRY(RBRGen3_converse(instrument, disableCommand));

    return RBRGen3_parseDeploymentResponse(instrument,
                                                 disableCommand,
                                                 status);
}

RBRGen3Error RBRGen3_getSimulation(
    RBRGen3 *instrument,
    RBRGen3Simulation *simulation)
{
    memset(simulation, 0, sizeof(RBRGen3Simulation));

    RBR_TRY(RBRGen3_converse(instrument, "simulation"));

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true)
    {
        RBRGen3_parseResponse(instrument,
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

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_setSimulation(
    RBRGen3 *instrument,
    const RBRGen3Simulation *simulation)
{
    if (simulation->period <= 0)
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    RBR_TRY(RBRInstrumentGen3_permit(instrument, "simulation"));
    RBR_TRY(RBRGen3_converse(instrument,
                                   "simulation state = %s, period = %i",
                                   (simulation->state) ? "on" : "off",
                                   simulation->period));
    return RBRGEN3_SUCCESS;
}
