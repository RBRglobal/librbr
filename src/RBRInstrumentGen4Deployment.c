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
#include <stdlib.h>
#include <string.h>
/* Required for snprintf. */
#include <stdio.h>

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Internal.h"
#include "RBRInstrumentGen4Schedule.h"
#include "RBRInstrumentGen4Security.h"
#include "RBRInstrumentGen4Deployment.h"

static RBRInstrumentGen4Error RBRInstrumentGen4_parseDeploymentResponse(
    RBRInstrumentGen4 *instrument,
    const char *deploymentCommand,
    RBRInstrumentGen4LoggingState *state)
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
            if (strcmp(RBRInstrumentGen4LoggingState_name(i),
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
        RBRInstrumentGen4LoggingState *status)
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
        RBRInstrumentGen4LoggingState *state)
{
    RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                   "enable config=%s dataset=%s simulation=%s storagemode=%s", 
                                   config->label, datasetLabel,
                                   storageMode?"normal":"calibration"
                                   ));

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
    RBRInstrumentGen4LoggingState *state)
{
    RBR_TRY(RBRInstrumentGen4_converse(instrument, "disable"));

    return RBRInstrumentGen4_parseDeploymentResponse(instrument,
                                                 "disable",
                                                 state);
}

RBRInstrumentGen4Error RBRInstrumentGen4_getSimulation(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4ChannelPool *channelPool,
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
        else if (strcmp(parameter.key, "channellist") == 0)
        {
            /* Match the labels to channels from the pool */
            /* TODO: This would be great as a helper function */
            for (int32_t channel_idx = 0; channel_idx < channelPool->count; channel_idx++)
            {
                if (strcmp(channelPool->pool[channel_idx].label,
                       parameter.key) == 0)
                {
                    strcpy(channelPool->pool[channel_idx].label,
                           parameter.key);
                }
            }
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
    /* Iterate over channel label list */
    /*
    RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                   "simulation state=%s period=%i channellist=%s",
                                   (simulation->state) ? "on" : "off",
                                   simulation->period,
                                   simulation->channelLabelList));
    */
    return RBRINSTRUMENTGEN4_SUCCESS;
}
