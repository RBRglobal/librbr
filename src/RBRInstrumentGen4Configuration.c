/**
 * \file RBRInstrumentGen4Configuration.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for NAN. */
#include <math.h>
/* Required for snprintf. */
#include <stdio.h>
/* Required for memset, strcmp. */
#include <stdlib.h>
#include <string.h>

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Internal.h"
#include "RBRInstrumentGen4Configuration.h"

/*
RBRInstrumentGen4Error RBRInstrumentGen4_getCalibration(
    RBRInstrumentGen4 *instrument,
    const char *channelLabel,
    RBRInstrumentGen4Calibration *calibration)
{
    memset(calibration, 0, sizeof(RBRInstrumentGen4Calibration));

    RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                       "calibration %s",
                                       channelLabel));
    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    do
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);
        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "fwversion") == 0)
        {
            snprintf((char *)(id->fwversion),
                     sizeof(id->fwversion),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "sn") == 0)
        {
            id->sn = strtol(parameter.value, NULL, 10);
        }
    } while (true);

    return RBRINSTRUMENTGEN4_SUCCESS;
}
*/

RBRInstrumentGen4Error RBRInstrumentGen4_getCalibration(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Calibration *calibration)
{
    memset(calibration, 0, sizeof(RBRInstrumentGen4Calibration));

    calibration->userOffset = NAN;
    calibration->userSlope = NAN;

    for (int32_t c = 0;
         c < RBRINSTRUMENTGEN4_CALIBRATION_C_COEFFICIENT_MAX;
         ++c)
    {
        calibration->c[c] = NAN;
    }
    for (int32_t x = 0;
         x < RBRINSTRUMENTGEN4_CALIBRATION_X_COEFFICIENT_MAX;
         ++x)
    {
        calibration->x[x] = NAN;
    }

    RBRInstrumentGen4Channel *parent = (RBRInstrumentGen4Channel *)(calibration->parent);
    RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                       "calibration %s",
                                       parent->label));
    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    do
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
            calibration->dateTime = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "offset") == 0)
        {
            calibration->userOffset = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "slope") == 0)
        {
            calibration->userSlope = strtol(parameter.value, NULL, 10);
        }
        else if (parameter.key[0] != 'c'
                 && parameter.key[0] != 'x'
                 && parameter.key[0] != 'n')
        {
            continue;
        }

        int32_t index = strtol(&parameter.key[1], NULL, 10);

        if (parameter.key[0] == 'c'
            && index < RBRINSTRUMENTGEN4_CALIBRATION_C_COEFFICIENT_MAX)
        {
            calibration->c[index] = strtod(parameter.value, NULL);
        }
        else if (parameter.key[0] == 'x'
                 && index < RBRINSTRUMENTGEN4_CALIBRATION_X_COEFFICIENT_MAX)
        {
            calibration->x[index] = strtod(parameter.value, NULL);
        }
        else if (parameter.key[0] == 'n'
                 && index < RBRINSTRUMENTGEN4_CALIBRATION_N_COEFFICIENT_MAX)
        {
            /* GEN4TODO: Get the dependents by reference */
            /*
            snprintf(calibration->n[index],
                     sizeof(calibration->n[index]),
                     "%s",
                     parameter.value);
            */
        }

    } while (true);

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setCalibration(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Calibration *calibration)
{
    //GEN4 todo: add logic.
    char calibrationDateTime[RBRINSTRUMENTGEN4_SCHEDULE_TIME_LEN + 1];
    RBRInstrumentGen4DateTime_toScheduleTime(calibration->dateTime,
                                         calibrationDateTime);

    const char *calibrationCommand = "calibration %s datetime = %s, %c%d = %g";

    RBRInstrumentGen4Channel *parent = (RBRInstrumentGen4Channel *)(calibration->parent);
    RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                "calibration %s datetime=%s, useroffset=%d, userslope=%d",
                                parent->label,
                                calibrationDateTime,
                                (double) calibration->userOffset,
                                (double) calibration->userSlope));

    for (int32_t c = 0;
         c < RBRINSTRUMENTGEN4_CALIBRATION_C_COEFFICIENT_MAX
         && !isnan(calibration->c[c]);
         ++c)
    {
        RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                       calibrationCommand,
                                       parent->label,
                                       calibrationDateTime,
                                       'c',
                                       c,
                                       (double) calibration->c[c]));
    }
    for (int32_t x = 0;
         x < RBRINSTRUMENTGEN4_CALIBRATION_X_COEFFICIENT_MAX
         && !isnan(calibration->x[x]);
         ++x)
    {
        RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                       calibrationCommand,
                                       parent->label,
                                       calibrationDateTime,
                                       'x',
                                       x,
                                       (double) calibration->x[x]));
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}


const char *RBRInstrumentGen4ChannelGainMode_name(
    RBRInstrumentGen4ChannelGainMode mode)
{
    switch (mode)
    {
    case RBRINSTRUMENTGEN4_GAIN_NONE:
        return "none";
    case RBRINSTRUMENTGEN4_GAIN_FIXED:
        return "manual";
    case RBRINSTRUMENTGEN4_GAIN_AUTO:
        return "auto";
    case RBRINSTRUMENTGEN4_GAIN_COUNT:
        return "gain mode count";
    case RBRINSTRUMENTGEN4_UNKNOWN_GAIN:
    default:
        return "unknown gain mode";
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_getChannel(RBRInstrumentGen4 *instrument,
                                                    RBRInstrumentGen4Channel *channel){
    //GEN4 todo: add logic
    (void)instrument;
    (void)channel;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getChannelPool(RBRInstrumentGen4 *instrument,
                                             RBRInstrumentGen4ChannelPool *channelPool)
{
    //GEN4 todo: add logic.
    //populate all channel instances, which included the calbration instances.
    (void)instrument;
    (void)channelPool;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4Settings_getSettings(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Settings *settings){
        (void)instrument;
        (void)settings;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4Settings_setSettings(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Settings *settings){
        (void)instrument;
        (void)settings;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

#if 0
RBRInstrumentGen4Error RBRInstrumentGen4_getSensorParameter(
    RBRInstrumentGen4 *instrument,
    char *channelLabel,
    RBRInstrumentGen4SensorParameter *parameter)
{
    memset(parameter->value, 0, sizeof(parameter->value));

    RBRInstrumentGen4Error err;
    /* Logger2 returns “E0501 item is not configured” when the requested
     * parameter doesn't exist, so we can't wrap the conversation in RBR_TRY
     * because we need to suppress that error. */
    err = RBRInstrumentGen4_converse(instrument,
                                 "sensor %s %s",
                                 channelLabel,
                                 parameter->key);

    if (instrument->generation == RBRINSTRUMENTGEN4_LOGGER2
        && err == RBRINSTRUMENTGEN4_HARDWARE_ERROR
        && (instrument->response.error ==
            RBRINSTRUMENTGEN4_HARDWARE_ERROR_ITEM_IS_NOT_CONFIGURED))
    {
        snprintf(parameter->value,
                 sizeof(parameter->value),
                 "n/a");
        instrument->response.type = RBRINSTRUMENTGEN4_RESPONSE_INFO;
        return RBRINSTRUMENTGEN4_SUCCESS;
    }
    else if (err != RBRINSTRUMENTGEN4_SUCCESS)
    {
        return err;
    }

    char *command = NULL;
    RBRInstrumentGen4ResponseParameter responseParameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                    &command,
                                    &responseParameter);

        if (responseParameter.key == NULL)
        {
            break;
        }

        snprintf(parameter->key,
                 sizeof(parameter->key),
                 "%s",
                 responseParameter.key);

        snprintf(parameter->value,
                 sizeof(parameter->value),
                 "%s",
                 responseParameter.value);
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}


RBRInstrumentGen4Error RBRInstrumentGen4_getSensorParameters(
    RBRInstrumentGen4 *instrument,
    const char *channelLabel,
    int32_t *size,
    RBRInstrumentGen4SensorParameter *parameters)
{
    (void)instrument;
    (void)channelLabel;
    (void)size;
    (void)parameters;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setSensorParameter(
    RBRInstrumentGen4 *instrument,
    const char *channelLabel,
    const RBRInstrumentGen4SensorParameter *parameter)
{
    return RBRInstrumentGen4_converse(instrument,
                                  "sensor %s %s=%s",
                                  channelLabel,
                                  parameter->key,
                                  parameter->value);
}
#endif

const char *RBRInstrumentGen4UvledCommand_name(RBRInstrumentGen4UvledCommand uvledCommand)
{
            switch(uvledCommand){
            case RBRINSTRUMENTGEN4_UVLED_ACTIVATE:
                return "activate";
            case RBRINSTRUMENTGEN4_UVLED_DEACTIVATE:
                return "deactivate";
            case RBRINSTRUMENTGEN4_UVLED_STATUS:
                return "status";
            case RBRINSTRUMENTGEN4_UVLED_COUNT:
                return "uvled command count";
            case RBRINSTRUMENTGEN4_UNKNOWN_UVLED:
            default:
                return "unknown uvled command";
            }
}

RBRInstrumentGen4Error RBRInstrumentGen4_getUvled(RBRInstrumentGen4 *instrument, RBRInstrumentGen4Uvled *uvled){
        (void)instrument;
        (void)uvled;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4Uvled_setUvled(RBRInstrumentGen4 *instrument, const RBRInstrumentGen4Uvled *uvled){
        (void)instrument;
        (void)uvled;
        return RBRINSTRUMENTGEN4_SUCCESS;
}


RBRInstrumentGen4Error RBRInstrumentGen4_getGroup(RBRInstrumentGen4 *instrument,
                                                  RBRInstrumentGen4ChannelPool *channelPool,
                                                  RBRInstrumentGen4Group *group){
        (void)instrument;
        (void)channelPool;
        (void)group;
        return RBRINSTRUMENTGEN4_SUCCESS;
}
RBRInstrumentGen4Error RBRInstrumentGen4_setGroup(RBRInstrumentGen4 *instrument, 
                                                  RBRInstrumentGen4Group *group){
        (void)instrument;
        (void)group;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_createGroup(
    RBRInstrumentGen4 *instrument,
    const char *newGroupLabel,
    RBRInstrumentGen4GroupPool *groupPool,
    RBRInstrumentGen4Group **newGroup){
        (void)instrument;
        (void)newGroupLabel;
        (void)groupPool;
        (void)newGroup;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteGroup(
    RBRInstrumentGen4 *instrument, 
    RBRInstrumentGen4Group *groupToDelete){
        (void)instrument;
        (void)groupToDelete;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteGroupAll(RBRInstrumentGen4 *instrument,
                                                    RBRInstrumentGen4GroupPool *groupPool){
    (void)instrument;
    (void)groupPool;                                                            
    return RBRINSTRUMENTGEN4_SUCCESS;
}                                                    
RBRInstrumentGen4Error RBRInstrumentGen4_getGroupPool(RBRInstrumentGen4 *instrument, RBRInstrumentGen4GroupPool *groupPool){
        (void)instrument;
        (void)groupPool;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getConfig(
    RBRInstrumentGen4 *instrument, 
    RBRInstrumentGen4SchedulePool *schedulePool,
    RBRInstrumentGen4Config *config){
        (void)instrument;
        (void)schedulePool;
        (void)config;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setConfig(RBRInstrumentGen4 *instrument, 
                                                   RBRInstrumentGen4Config *config)
{
        (void)instrument;
        (void)config;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getConfigPool(RBRInstrumentGen4 *instrument, RBRInstrumentGen4ConfigPool *configPool){
        (void)instrument;
        (void)configPool;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_createConfig(
    RBRInstrumentGen4 *instrument, 
    const char *newConfigLabel,
    RBRInstrumentGen4ConfigPool *configPool,
    RBRInstrumentGen4Config **newConfig)
{
        (void)instrument;
        (void)newConfigLabel;
        (void)configPool;
        (void)newConfig;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteConfig(RBRInstrumentGen4 *instrument, 
                                                    RBRInstrumentGen4Config *config)
{
        (void)instrument;
        (void)config;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteConfigAll(RBRInstrumentGen4 *instrument,
                                                    RBRInstrumentGen4ConfigPool *configPool){
        (void)instrument;
        (void)configPool;
        return RBRINSTRUMENTGEN4_SUCCESS;                                                
}
