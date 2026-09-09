/**
 * \file RBRInstrumentGen3Configuration.c
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

#include "RBRInstrumentGen3.h"
#include "RBRInstrumentGen3Internal.h"

const char *RBRInstrumentGen3ChannelRangingMode_name(
    RBRInstrumentGen3ChannelRangingMode mode)
{
    switch (mode)
    {
    case RBRINSTRUMENTGEN3_RANGING_NONE:
        return "none";
    case RBRINSTRUMENTGEN3_RANGING_MANUAL:
        return "manual";
    case RBRINSTRUMENTGEN3_RANGING_AUTO:
        return "auto";
    case RBRINSTRUMENTGEN3_RANGING_COUNT:
        return "ranging mode count";
    case RBRINSTRUMENTGEN3_UNKNOWN_RANGING:
    default:
        return "unknown ranging mode";
    }
}

static RBRInstrumentGen3Error RBRInstrumentGen3_clearChannel(RBRInstrumentGen3Channel *channel)
{
    channel->gain.currentGain = NAN;

    for (int32_t gain = 0; gain < RBRINSTRUMENTGEN3_CHANNEL_GAINS_MAX; ++gain)
    {
        channel->gain.availableGains[gain] = NAN;
    }

    snprintf(channel->label, sizeof(channel->label), "%s", "none");

    for (int32_t c = 0;
         c < RBRINSTRUMENTGEN3_CALIBRATION_C_COEFFICIENT_MAX;
         ++c)
    {
        channel->calibration.c[c] = NAN;
    }
    for (int32_t x = 0;
         x < RBRINSTRUMENTGEN3_CALIBRATION_X_COEFFICIENT_MAX;
         ++x)
    {
        channel->calibration.x[x] = NAN;
    }

    return RBRINSTRUMENTGEN3_SUCCESS;
}

static RBRInstrumentGen3Error RBRInstrumentGen3_getChannelCoefficients(
    RBRInstrumentGen3 *instrument,
    int32_t channelIndex,
    RBRInstrumentGen3Channel *channel)
{
    RBR_TRY(RBRInstrumentGen3_converse(instrument, "calibration %d all",
                                   channelIndex + 1));

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

        if (strcmp(parameter.key, "datetime") == 0)
        {
            RBR_TRY(RBRInstrumentGen3DateTime_parseScheduleTime(
                        parameter.value,
                        &channel->calibration.dateTime,
                        NULL));
        }
        else if (parameter.key[0] != 'c'
                 && parameter.key[0] != 'x'
                 && parameter.key[0] != 'n')
        {
            continue;
        }

        int32_t index = strtol(&parameter.key[1], NULL, 10);

        if (parameter.key[0] == 'c'
            && index >= 0
            && index < RBRINSTRUMENTGEN3_CALIBRATION_C_COEFFICIENT_MAX)
        {
            channel->calibration.c[index] = strtod(parameter.value, NULL);
        }
        else if (parameter.key[0] == 'x'
                 && index >= 0
                 && index < RBRINSTRUMENTGEN3_CALIBRATION_X_COEFFICIENT_MAX)
        {
            channel->calibration.x[index] = strtod(parameter.value, NULL);
        }
        else if (parameter.key[0] == 'n'
                 && index >= 0
                 && index < RBRINSTRUMENTGEN3_CALIBRATION_N_COEFFICIENT_MAX)
        {
            RBRInstrumentGen3ChannelIndex coefficient;
            if (strcmp(parameter.value, "value") == 0)
            {
                coefficient = RBRINSTRUMENTGEN3_VALUE_COEFFICIENT;
            }
            else
            {
                coefficient = strtol(parameter.value, NULL, 10);
            }

            channel->calibration.n[index] = coefficient;
        }
    }

    return RBRINSTRUMENTGEN3_SUCCESS;
}

static RBRInstrumentGen3Error RBRInstrumentGen3_getChannel(
    RBRInstrumentGen3 *instrument,
    int32_t channelIndex,
    RBRInstrumentGen3Channel *channel)
{
    if (instrument->generation == RBRINSTRUMENTGEN3_LOGGER2)
    {
        RBR_TRY(RBRInstrumentGen3_converse(
                    instrument,
                    "channel %d all derived gain gainsavailable",
                    channelIndex + 1));
    }
    else
    {
        RBR_TRY(RBRInstrumentGen3_converse(
                    instrument,
                    "channel %d all",
                    channelIndex + 1));
    }

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

        if (strcmp(parameter.key, "type") == 0)
        {
            snprintf(channel->type,
                     sizeof(channel->type),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "module") == 0)
        {
            channel->module = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "status") == 0)
        {
            channel->status = (strcmp(parameter.value, "on") == 0);
        }
        else if (strcmp(parameter.key, "settlingtime") == 0
                 || strcmp(parameter.key, "latency") == 0)
        {
            channel->settlingTime = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "readtime") == 0)
        {
            channel->readTime = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "equation") == 0)
        {
            snprintf(channel->equation,
                     sizeof(channel->equation),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "userunits") == 0)
        {
            snprintf(channel->userUnits,
                     sizeof(channel->userUnits),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "derived") == 0)
        {
            channel->derived = (strcmp(parameter.value, "on") == 0);
        }
        else if (strcmp(parameter.key, "gain") == 0)
        {
            if (strcmp(parameter.value, "none") == 0)
            {
                continue;
            }
            else if (strcmp(parameter.value, "auto") == 0)
            {
                channel->gain.rangingMode = RBRINSTRUMENTGEN3_RANGING_AUTO;
            }
            else
            {
                channel->gain.rangingMode = RBRINSTRUMENTGEN3_RANGING_MANUAL;
                channel->gain.currentGain = strtod(parameter.value, NULL);
            }
        }
        else if (strcmp(parameter.key, "availablegains") == 0
                 || strcmp(parameter.key, "gainsavailable") == 0)
        {
            if (strcmp(parameter.value, "none") == 0)
            {
                continue;
            }

            char *gains = parameter.value;
            int32_t gainCount = 0;
            char *gain = NULL;
            while ((gain = strtok(gains, "|")) != NULL
                   && gainCount < RBRINSTRUMENTGEN3_CHANNEL_GAINS_MAX)
            {
                gains = NULL;
                channel->gain.availableGains[gainCount++] = strtod(gain, NULL);
            }
        }
        else if (strcmp(parameter.key, "label") == 0)
        {
            snprintf(channel->label,
                     sizeof(channel->label),
                     "%s",
                     parameter.value);
        }
    }

    return RBRINSTRUMENTGEN3_SUCCESS;
}

typedef enum RBRInstrumentGen3ChannelDensity
{
    /** Channel has no additional information populated. */
    RBRINSTRUMENTGEN3_CHANNEL_SPARSE = 0,
    /** Channel's calibration information is populated. */
    RBRINSTRUMENTGEN3_CHANNEL_CALIBRATION = 1 << 0,
} RBRInstrumentGen3ChannelDensity;

static RBRInstrumentGen3Error RBRInstrumentGen3_getChannelAll(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3Channels *channels,
    RBRInstrumentGen3ChannelDensity density)
{
    int32_t channel_count = channels->count;
    if (channel_count > RBRINSTRUMENTGEN3_CHANNEL_MAX)
    {
        channel_count = RBRINSTRUMENTGEN3_CHANNEL_MAX;
    }

    for (int32_t idx = 0; idx < channel_count; ++idx)
    {
        RBRInstrumentGen3Channel *channel = &channels->channels[idx];
        RBRInstrumentGen3_clearChannel(channel);
        RBR_TRY(RBRInstrumentGen3_getChannel(instrument, idx, channel));

        if (density & RBRINSTRUMENTGEN3_CHANNEL_CALIBRATION)
        {
            RBR_TRY(RBRInstrumentGen3_getChannelCoefficients(instrument,
                                                         idx,
                                                         channel));
        }
    }

    return RBRINSTRUMENTGEN3_SUCCESS;
}

static RBRInstrumentGen3Error RBRInstrumentGen3_getChannelsWithDensity(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3Channels *channels,
    RBRInstrumentGen3ChannelDensity density)
{
    memset(channels, 0, sizeof(RBRInstrumentGen3Channels));

    RBR_TRY(RBRInstrumentGen3_converse(instrument, "channels"));

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

        if (strcmp(parameter.key, "count") == 0)
        {
            channels->count = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "on") == 0)
        {
            channels->on = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "settlingtime") == 0
                 || strcmp(parameter.key, "latency") == 0)
        {
            channels->settlingTime = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "readtime") == 0)
        {
            channels->readTime = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "minperiod") == 0)
        {
            channels->minimumPeriod = strtol(parameter.value, NULL, 10);
        }
    }

    RBR_TRY(RBRInstrumentGen3_getChannelAll(instrument, channels, density));

    return RBRINSTRUMENTGEN3_SUCCESS;
}

RBRInstrumentGen3Error RBRInstrumentGen3_getChannels(RBRInstrumentGen3 *instrument,
                                             RBRInstrumentGen3Channels *channels)
{
    return RBRInstrumentGen3_getChannelsWithDensity(
        instrument,
        channels,
        RBRINSTRUMENTGEN3_CHANNEL_CALIBRATION);
}

RBRInstrumentGen3Error RBRInstrumentGen3_getChannelsWithoutCalibrations(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3Channels *channels)
{
    return RBRInstrumentGen3_getChannelsWithDensity(
        instrument,
        channels,
        RBRINSTRUMENTGEN3_CHANNEL_SPARSE);
}

RBRInstrumentGen3Error RBRInstrumentGen3_setChannelStatus(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3ChannelIndex channel,
    bool status)
{
    return RBRInstrumentGen3_converse(instrument,
                                  "channel %d status = %s",
                                  channel,
                                  status ? "on" : "off");
}

RBRInstrumentGen3Error RBRInstrumentGen3_setChannelGain(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3ChannelIndex channel,
    RBRInstrumentGen3ChannelGain *gain)
{
    if (gain->rangingMode == RBRINSTRUMENTGEN3_RANGING_MANUAL)
    {
        bool validGain = false;
        int32_t i;
        for (i = 0;
             i < RBRINSTRUMENTGEN3_CHANNEL_GAINS_MAX
             && !isnan(gain->availableGains[i]);
             ++i)
        {
            if (gain->currentGain == gain->availableGains[i])
            {
                validGain = true;
                break;
            }
        }

        if (i > 0 && !validGain)
        {
            return RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE;
        }

        return RBRInstrumentGen3_converse(instrument,
                                      "channel %d gain = %0.1f",
                                      channel,
                                      (double) gain->currentGain);
    }
    else if (gain->rangingMode == RBRINSTRUMENTGEN3_RANGING_AUTO)
    {
        return RBRInstrumentGen3_converse(instrument,
                                      "channel %d gain = auto",
                                      channel);
    }
    else
    {
        return RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE;
    }
}

RBRInstrumentGen3Error RBRInstrumentGen3_setCalibration(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3ChannelIndex channel,
    const RBRInstrumentGen3Calibration *calibration)
{
    if (calibration->dateTime < RBRINSTRUMENTGEN3_DATETIME_MIN
        || calibration->dateTime > RBRINSTRUMENTGEN3_DATETIME_MAX)
    {
        return RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE;
    }

    char calibrationDateTime[RBRINSTRUMENTGEN3_SCHEDULE_TIME_LEN + 1];
    RBRInstrumentGen3DateTime_toScheduleTime(calibration->dateTime,
                                         calibrationDateTime);

    const char *calibrationCommand = "calibration %d datetime = %s, %c%d = %g";

    bool populated = false;

    for (int32_t c = 0;
         c < RBRINSTRUMENTGEN3_CALIBRATION_C_COEFFICIENT_MAX
         && !isnan(calibration->c[c]);
         ++c)
    {
        populated = true;
        RBR_TRY(RBRInstrumentGen3_converse(instrument,
                                       calibrationCommand,
                                       channel,
                                       calibrationDateTime,
                                       'c',
                                       c,
                                       (double) calibration->c[c]));
    }
    for (int32_t x = 0;
         x < RBRINSTRUMENTGEN3_CALIBRATION_X_COEFFICIENT_MAX
         && !isnan(calibration->x[x]);
         ++x)
    {
        populated = true;
        RBR_TRY(RBRInstrumentGen3_converse(instrument,
                                       calibrationCommand,
                                       channel,
                                       calibrationDateTime,
                                       'x',
                                       x,
                                       (double) calibration->x[x]));
    }

    if (!populated)
    {
        return RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE;
    }

    return RBRINSTRUMENTGEN3_SUCCESS;
}

RBRInstrumentGen3Error RBRInstrumentGen3_getFetchPowerOffDelay(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3Period *fetchPowerOffDelay)
{
    return RBRInstrumentGen3_getInt(instrument,
                                "settings",
                                "fetchpoweroffdelay",
                                fetchPowerOffDelay);
}

RBRInstrumentGen3Error RBRInstrumentGen3_setFetchPowerOffDelay(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3Period fetchPowerOffDelay)
{
    RBR_TRY(RBRInstrumentGen3_permit(instrument, "settings"));
    RBR_TRY(RBRInstrumentGen3_converse(instrument,
                                   "settings fetchpoweroffdelay = %d",
                                   fetchPowerOffDelay));
    return RBRINSTRUMENTGEN3_SUCCESS;
}

RBRInstrumentGen3Error RBRInstrumentGen3_isSensorPowerAlwaysOn(
    RBRInstrumentGen3 *instrument,
    bool *sensorPowerAlwaysOn)
{
    return RBRInstrumentGen3_getBool(instrument,
                                 "settings",
                                 "sensorpoweralwayson",
                                 sensorPowerAlwaysOn);
}

RBRInstrumentGen3Error RBRInstrumentGen3_setSensorPowerAlwaysOn(
    RBRInstrumentGen3 *instrument,
    bool sensorPowerAlwaysOn)
{
    RBR_TRY(RBRInstrumentGen3_permit(instrument, "settings"));
    RBR_TRY(RBRInstrumentGen3_converse(instrument,
                                   "settings sensorpoweralwayson = %s",
                                   sensorPowerAlwaysOn ? "on" : "off"));
    return RBRINSTRUMENTGEN3_SUCCESS;
}

RBRInstrumentGen3Error RBRInstrumentGen3_getCastDetection(RBRInstrumentGen3 *instrument,
                                                  bool *castDetection)
{
    return RBRInstrumentGen3_getBool(instrument,
                                 "settings",
                                 "castdetection",
                                 castDetection);
}

RBRInstrumentGen3Error RBRInstrumentGen3_setCastDetection(RBRInstrumentGen3 *instrument,
                                                  bool castDetection)
{
    RBR_TRY(RBRInstrumentGen3_permit(instrument, "settings"));
    RBR_TRY(RBRInstrumentGen3_converse(instrument,
                                   "settings castdetection = %s",
                                   castDetection ? "on" : "off"));
    return RBRINSTRUMENTGEN3_SUCCESS;
}

RBRInstrumentGen3Error RBRInstrumentGen3_getInputTimeout(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3Period *inputTimeout)
{
    return RBRInstrumentGen3_getInt(instrument,
                                "settings",
                                "inputtimeout",
                                inputTimeout);
}

RBRInstrumentGen3Error RBRInstrumentGen3_setInputTimeout(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3Period inputTimeout)
{
    if (inputTimeout < RBRINSTRUMENTGEN3_INPUT_TIMEOUT_MIN
        || inputTimeout > RBRINSTRUMENTGEN3_INPUT_TIMEOUT_MAX)
    {
        return RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE;
    }

    RBR_TRY(RBRInstrumentGen3_permit(instrument, "settings"));
    RBR_TRY(RBRInstrumentGen3_converse(instrument,
                                   "settings inputtimeout = %d",
                                   inputTimeout));
    return RBRINSTRUMENTGEN3_SUCCESS;
}

const char *RBRInstrumentGen3ValueSetting_name(RBRInstrumentGen3ValueSetting setting)
{
    switch (setting)
    {
    case RBRINSTRUMENTGEN3_SETTING_CONDUCTIVITY:
        return "conductivity";
    case RBRINSTRUMENTGEN3_SETTING_SPECCONDTEMPCO:
        return "speccondtempco";
    case RBRINSTRUMENTGEN3_SETTING_ALTITUDE:
        return "altitude";
    case RBRINSTRUMENTGEN3_SETTING_TEMPERATURE:
        return "temperature";
    case RBRINSTRUMENTGEN3_SETTING_PRESSURE:
        return "pressure";
    case RBRINSTRUMENTGEN3_SETTING_ATMOSPHERE:
        return "atmosphere";
    case RBRINSTRUMENTGEN3_SETTING_DENSITY:
        return "density";
    case RBRINSTRUMENTGEN3_SETTING_SALINITY:
        return "salinity";
    case RBRINSTRUMENTGEN3_SETTING_AVGSOUNDSPEED:
        return "avgsoundspeed";
    case RBRINSTRUMENTGEN3_SETTING_COUNT:
        return "setting count";
    case RBRINSTRUMENTGEN3_UNKNOWN_SETTING:
    default:
        return "unknown setting";
    }
}

RBRInstrumentGen3Error RBRInstrumentGen3_getValueSetting(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3ValueSetting setting,
    float *value)
{
    if (setting < 0 || setting >= RBRINSTRUMENTGEN3_SETTING_COUNT)
    {
        return RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrumentGen3_getFloat(instrument,
                                  "settings",
                                  RBRInstrumentGen3ValueSetting_name(setting),
                                  value);
}

RBRInstrumentGen3Error RBRInstrumentGen3_setValueSetting(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3ValueSetting setting,
    float value)
{
    if (setting < 0 || setting >= RBRINSTRUMENTGEN3_SETTING_COUNT)
    {
        return RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE;
    }

    RBR_TRY(RBRInstrumentGen3_permit(instrument, "settings"));
    RBR_TRY(RBRInstrumentGen3_converse(instrument,
                                   "settings %s = %f",
                                   RBRInstrumentGen3ValueSetting_name(setting),
                                   (double) value));
    return RBRINSTRUMENTGEN3_SUCCESS;
}

RBRInstrumentGen3Error RBRInstrumentGen3_getSensorParameter(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3ChannelIndex channel,
    RBRInstrumentGen3SensorParameter *parameter)
{
    memset(parameter->value, 0, sizeof(parameter->value));

    RBRInstrumentGen3Error err;
    /* Logger2 returns “E0501 item is not configured” when the requested
     * parameter doesn't exist, so we can't wrap the conversation in RBR_TRY
     * because we need to suppress that error. */
    err = RBRInstrumentGen3_converse(instrument,
                                 "sensor %d %s",
                                 channel,
                                 parameter->key);

    if (instrument->generation == RBRINSTRUMENTGEN3_LOGGER2
        && err == RBRINSTRUMENTGEN3_HARDWARE_ERROR
        && (instrument->response.error ==
            RBRINSTRUMENTGEN3_HARDWARE_ERROR_ITEM_IS_NOT_CONFIGURED))
    {
        snprintf(parameter->value,
                 sizeof(parameter->value),
                 "n/a");
        instrument->response.type = RBRINSTRUMENTGEN3_RESPONSE_INFO;
        return RBRINSTRUMENTGEN3_SUCCESS;
    }
    else if (err != RBRINSTRUMENTGEN3_SUCCESS)
    {
        return err;
    }

    char *command = NULL;
    RBRInstrumentGen3ResponseParameter responseParameter;
    while (true)
    {
        RBRInstrumentGen3_parseResponse(instrument,
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

    return RBRINSTRUMENTGEN3_SUCCESS;
}

RBRInstrumentGen3Error RBRInstrumentGen3_getSensorParameters(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3ChannelIndex channel,
    RBRInstrumentGen3SensorParameter *parameters,
    int32_t *size)
{
    int32_t maxSize = *size;
    *size = 0;

    if (channel < 1 || channel > RBRINSTRUMENTGEN3_CHANNEL_MAX)
    {
        return RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE;
    }

    memset(parameters, 0, sizeof(RBRInstrumentGen3SensorParameter) * maxSize);

    RBRInstrumentGen3Error err;
    /* Logger2 returns “E0109 feature not available” for channels which have no
     * sensor parameters, so we can't wrap the conversation in RBR_TRY because
     * we need to suppress that error. */
    err = RBRInstrumentGen3_converse(instrument, "sensor %d", channel);

    if (instrument->generation == RBRINSTRUMENTGEN3_LOGGER2
        && err == RBRINSTRUMENTGEN3_HARDWARE_ERROR
        && (instrument->response.error ==
            RBRINSTRUMENTGEN3_HARDWARE_ERROR_FEATURE_NOT_AVAILABLE))
    {
        instrument->response.type = RBRINSTRUMENTGEN3_RESPONSE_INFO;
        return RBRINSTRUMENTGEN3_SUCCESS;
    }
    else if (err != RBRINSTRUMENTGEN3_SUCCESS)
    {
        return err;
    }

    char channelStr[RBRINSTRUMENTGEN3_CHANNEL_MAX_LEN];
    snprintf(channelStr, sizeof(channelStr), "%i", channel);

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

        /*
         * Logger3 returns the exact command when there are no sensor
         * parameters for a channel. E.g.,
         *
         *     >> sensor 1
         *     << sensor 1
         *
         * The response parser confuses the “1” as being the key for a
         * parameter with no value as opposed to seeing it as the index
         * parameter value. Rather than add a special case to the parser, we'll
         * just swallow this response if it happens.
         */
        if (strcmp(parameter.key, channelStr) == 0
            && strlen(parameter.value) == 0)
        {
            break;
        }

        snprintf(parameters[*size].key,
                 sizeof(parameters[*size].key),
                 "%s",
                 parameter.key);

        snprintf(parameters[*size].value,
                 sizeof(parameters[*size].value),
                 "%s",
                 parameter.value);

        if (++(*size) >= maxSize)
        {
            break;
        }
    }

    return RBRINSTRUMENTGEN3_SUCCESS;
}

RBRInstrumentGen3Error RBRInstrumentGen3_setSensorParameter(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3ChannelIndex channel,
    RBRInstrumentGen3SensorParameter *parameter)
{
    return RBRInstrumentGen3_converse(instrument,
                                  "sensor %d %s = %s",
                                  channel,
                                  parameter->key,
                                  parameter->value);
}
