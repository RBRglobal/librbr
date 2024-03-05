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
#include <string.h>

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Internal.h"
#include "RBRInstrumentGen4Configuration.h"


RBRInstrumentGen4Error RBRInstrumentGen4_getCalibration(
    RBRInstrumentGen4 *instrument,
    const char *channellabel,
    RBRInstrumentGen4Calibration *calibration)
{
    (void)instrument;
    (void)channellabel;
    (void)calibration;
    //GEN4 todo: need to add logic.
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setCalibration(
    RBRInstrumentGen4 *instrument,
    const char *channellabel,
    const RBRInstrumentGen4Calibration *calibration)
{
    //GEN4 todo: add logic.
    char calibrationDateTime[RBRINSTRUMENTGEN4_SCHEDULE_TIME_LEN + 1];
    RBRInstrumentGen4DateTime_toScheduleTime(calibration->dateTime,
                                         calibrationDateTime);

    const char *calibrationCommand = "calibration %s datetime = %s, %c%d = %g";

    RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                "calibration %s datetime=%s, useroffset=%d, userslope=%d",
                                channellabel,
                                calibrationDateTime,
                                calibration->useroffset,
                                calibration->userslope));

    for (int32_t c = 0;
         c < RBRINSTRUMENTGEN4_CALIBRATION_C_COEFFICIENT_MAX
         && !isnan(calibration->c[c]);
         ++c)
    {
        RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                       calibrationCommand,
                                       channellabel,
                                       calibrationDateTime,
                                       'c',
                                       c,
                                       calibration->c[c]));
    }
    for (int32_t x = 0;
         x < RBRINSTRUMENTGEN4_CALIBRATION_X_COEFFICIENT_MAX
         && !isnan(calibration->x[x]);
         ++x)
    {
        RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                       calibrationCommand,
                                       channellabel,
                                       calibrationDateTime,
                                       'x',
                                       x,
                                       calibration->x[x]));
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

RBRInstrumentGen4Error RBRInstrumentGen4_getChannel(RBRInstrumentGen4 *instrument, const char *channellabel, RBRInstrumentGen4Channel *channel){
    //GEN4 todo: add logic
    (void)instrument;
    (void)channellabel;
    (void)channel;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getChannels(RBRInstrumentGen4 *instrument,
                                             RBRInstrumentGen4Channels *channels)
{
    //GEN4 todo: add logic.
    //populate all channel instances, which included the calbration instances.
    (void)instrument;
    (void)channels;
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

RBRInstrumentGen4Error RBRInstrumentGen4_getSensorParameter(
    RBRInstrumentGen4 *instrument,
    char *channellabel,
    RBRInstrumentGen4SensorParameter *parameter)
{
    memset(parameter->value, 0, sizeof(parameter->value));

    RBRInstrumentGen4Error err;
    /* Logger2 returns “E0501 item is not configured” when the requested
     * parameter doesn't exist, so we can't wrap the conversation in RBR_TRY
     * because we need to suppress that error. */
    err = RBRInstrumentGen4_converse(instrument,
                                 "sensor %s %s",
                                 channellabel,
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
    const char *channellabel,
    int32_t *size,
    RBRInstrumentGen4SensorParameter *parameters)
{
    (void)instrument;
    (void)channellabel;
    (void)size;
    (void)parameters;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setSensorParameter(
    RBRInstrumentGen4 *instrument,
    const char *channellabel,
    const RBRInstrumentGen4SensorParameter *parameter)
{
    return RBRInstrumentGen4_converse(instrument,
                                  "sensor %s %s = %s",
                                  channellabel,
                                  parameter->key,
                                  parameter->value);
}

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


RBRInstrumentGen4Error RBRInstrumentGen4_getGroup(RBRInstrumentGen4 *instrument, const char *grouplabel, RBRInstrumentGen4Group *group){
        (void)instrument;
        (void)grouplabel;
        (void)group;
        return RBRINSTRUMENTGEN4_SUCCESS;
}
RBRInstrumentGen4Error RBRInstrumentGen4_setGroup(RBRInstrumentGen4 *instrument, 
                                                const char *grouplabel,
                                                RBRInstrumentGen4Group *group){
        (void)instrument;
        (void)grouplabel;
        (void)group;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_createGroup(RBRInstrumentGen4 *instrument,  
                                                    const RBRInstrumentGen4Group *group, 
                                                    RBRInstrumentGen4Groups *groups){
        (void)instrument;
        (void)group;
        (void)groups;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteGroup(RBRInstrumentGen4 *instrument, 
                                                    const char *grouplabel, 
                                                    RBRInstrumentGen4Groups *groups){
        (void)instrument;
        (void)grouplabel;
        (void)groups;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteGroupMultiple(RBRInstrumentGen4 *instrument, 
                                                    const char *grouplabellist,
                                                    RBRInstrumentGen4Groups *groups){
        (void)instrument;
        (void)grouplabellist;
        (void)groups;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteGroupAll(RBRInstrumentGen4 *instrument,
                                                    RBRInstrumentGen4Groups *groups){
    (void)instrument;
    (void)groups;                                                            
    return RBRINSTRUMENTGEN4_SUCCESS;
}                                                    
RBRInstrumentGen4Error RBRInstrumentGen4_getGroups(RBRInstrumentGen4 *instrument, RBRInstrumentGen4Groups *groups){
        (void)instrument;
        (void)groups;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getConfig(RBRInstrumentGen4 *instrument, const char *configlabel, RBRInstrumentGen4Config *config){
        (void)instrument;
        (void)configlabel;
        (void)config;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setConfig(RBRInstrumentGen4 *instrument, 
                                                    const char *configlabel,
                                                    const RBRInstrumentGen4Config *config)
{
        (void)instrument;
        (void)configlabel;
        (void)config;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getConfigs(RBRInstrumentGen4 *instrument, RBRInstrumentGen4Configs *configs){
        (void)instrument;
        (void)configs;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_createConfig(RBRInstrumentGen4 *instrument, 
                                                    const RBRInstrumentGen4Config *config,
                                                    RBRInstrumentGen4Configs *configs)
{
        (void)instrument;
        (void)config;
        (void)configs;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteConfig(RBRInstrumentGen4 *instrument, 
                                                    const char *configlabel,
                                                    RBRInstrumentGen4Configs *configs)
{
        (void)instrument;
        (void)configlabel;
        (void)configs;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteConfigMultiple(RBRInstrumentGen4 *instrument, 
                                                    const char *configlabellist,
                                                    RBRInstrumentGen4Configs *configs){
        (void)instrument;
        (void)configlabellist;
        (void)configs;
        return RBRINSTRUMENTGEN4_SUCCESS;
}
RBRInstrumentGen4Error RBRInstrumentGen4_deleteConfigAll(RBRInstrumentGen4 *instrument,
                                                    RBRInstrumentGen4Configs *configs){
        (void)instrument;
        (void)configs;
        return RBRINSTRUMENTGEN4_SUCCESS;                                                
}

RBRInstrumentGen4Error RBRInstrumentGen4_factoryReset(RBRInstrumentGen4 *instrument){
        (void)instrument;
        return RBRINSTRUMENTGEN4_SUCCESS;
}