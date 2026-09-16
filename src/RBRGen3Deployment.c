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
    RBRGen3 *conn,
    const char *deploymentCommand,
    RBRGen3DeploymentStatus *status)
{
    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true)
    {
        RBRGen3_parseResponse(conn,
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

        for (int i = 0; i < RBRGEN3_STATUS_COUNT; i++)
        {
            if (strcmp(RBRGen3DeploymentStatus_name(i),
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
    RBRGen3 *conn,
    bool eraseMemory,
    RBRGen3DeploymentStatus *status)
{
    *status = RBRGEN3_UNKNOWN_STATUS;

    RBR_TRY(RBRGen3_converse(conn,
                                   "verify erasememory = %s",
                                   (eraseMemory) ? "true" : "false"));

    return RBRGen3_parseDeploymentResponse(conn,
                                                 "verify",
                                                 status);
}

RBRGen3Error RBRGen3_enable(
    RBRGen3 *conn,
    bool eraseMemory,
    RBRGen3DeploymentStatus *status)
{
    *status = RBRGEN3_UNKNOWN_STATUS;

    RBR_TRY(RBRGen3_converse(conn,
                                   "enable erasememory = %s",
                                   (eraseMemory) ? "true" : "false"));

    return RBRGen3_parseDeploymentResponse(conn,
                                                 "enable",
                                                 status);
}

RBRGen3Error RBRGen3_disable(
    RBRGen3 *conn,
    RBRGen3DeploymentStatus *status)
{
    *status = RBRGEN3_UNKNOWN_STATUS;

    const char *disableCommand;
    if (conn->generation == RBRGEN3_LOGGER2)
    {
        disableCommand = "stop";
    }
    else
    {
        disableCommand = "disable";
    }

    RBR_TRY(RBRGen3_converse(conn, disableCommand));

    return RBRGen3_parseDeploymentResponse(conn,
                                                 disableCommand,
                                                 status);
}

RBRGen3Error RBRGen3_getSimulation(
    RBRGen3 *conn,
    RBRGen3Simulation *simulation)
{
    memset(simulation, 0, sizeof(RBRGen3Simulation));

    RBR_TRY(RBRGen3_converse(conn, "simulation"));

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true)
    {
        RBRGen3_parseResponse(conn,
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
    RBRGen3 *conn,
    const RBRGen3Simulation *simulation)
{
    if (simulation->period <= 0)
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    RBR_TRY(RBRGen3_permit(conn, "simulation"));
    RBR_TRY(RBRGen3_converse(conn,
                                   "simulation state = %s, period = %i",
                                   (simulation->state) ? "on" : "off",
                                   simulation->period));
    return RBRGEN3_SUCCESS;
}
