/**
 * \file RBRInstrumentGen4Deployment.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for memset, strcmp. */
#include <string.h>
/* Required for snprintf. */
#include <stdio.h>

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Internal.h"
#include "RBRInstrumentGen4Deployment.h"

static RBRInstrumentGen4Error RBRInstrumentGen4_parseDeploymentResponse(
    RBRInstrumentGen4 *instrument,
    const char *deploymentCommand,
    RBRInstrumentGen4DeploymentStatus *status)
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

        for (int i = 0; i < RBRINSTRUMENTGEN4_STATUS_COUNT; i++)
        {
            if (strcmp(RBRInstrumentGen4DeploymentStatus_name(i),
                       parameter.value) == 0)
            {
                *status = i;
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
        RBRInstrumentGen4DeploymentStatus *status)
{
    RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                   "verify config = %s, dataset = %s", config->label, datasetLabel));

    return RBRInstrumentGen4_parseDeploymentResponse(instrument,
                                                 "verify",
                                                 status);
}

const char *RBRInstrumentGen4DeploymentStoragemode_name(RBRInstrumentGen4DeploymentStoragemode storagemode)
{
    switch (storagemode)
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
        const bool simulation, 
        const RBRInstrumentGen4DeploymentStoragemode storagemode, 
        RBRInstrumentGen4Datasets *datasets,
        RBRInstrumentGen4DeploymentStatus *status)
{
    RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                   "enable config = %s, dataset = %s, simulation = %s, storagemode = %s", 
                                   config->label, datasetLabel,
                                   simulation?"on":"off",
                                   storagemode?"normal":"calibration"
                                   ));

    RBRInstrumentGen4_parseDeploymentResponse(instrument,
                                                 "enable",
                                                 status);
    
    RBRInstrumentGen4Dataset newDataset;
    strcpy(newDataset.label, datasetLabel);
    datasets->datasetlist.count += 1;
    datasets->datasetlist.datasets[datasets->datasetlist.count-1] = &newDataset;

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_disable(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4DeploymentStatus *status)
{
    RBR_TRY(RBRInstrumentGen4_converse(instrument, "disable"));

    return RBRInstrumentGen4_parseDeploymentResponse(instrument,
                                                 "disable",
                                                 status);
}

RBRInstrumentGen4Error RBRInstrumentGen4_getSimulation(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Simulation *simulation)
{
    RBR_TRY(RBRInstrumentGen4_converse(instrument, "simulation"));

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
        else if (strcmp(parameter.key, "state") == 0)
        {
            simulation->state = (strcmp(parameter.value, "on") == 0);
        }
        else if (strcmp(parameter.key, "period") == 0)
        {
            simulation->period = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "channellist")==0)
        {
            strcpy(simulation->channellabellist, parameter.value);
        }
    }
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setSimulation(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Simulation *simulation)
{
    if (simulation->period <= 0)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    RBR_TRY(RBRInstrumentGen4_permit(instrument, "simulation"));
    RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                   "simulation state = %s, period = %i, channellist = %s",
                                   (simulation->state) ? "on" : "off",
                                   simulation->period,
                                   simulation->channellabellist));
    return RBRINSTRUMENTGEN4_SUCCESS;
}
