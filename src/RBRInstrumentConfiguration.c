/**
 * \file RBRInstrumentConfiguration.c
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

#include "RBRInstrument.h"
#include "RBRInstrumentInternal.h"

const char *RBRInstrumentChannelRangingMode_name(
    RBRInstrumentChannelRangingMode mode)
{
    switch (mode)
    {
    case RBRINSTRUMENT_RANGING_NONE:
        return "none";
    case RBRINSTRUMENT_RANGING_MANUAL:
        return "manual";
    case RBRINSTRUMENT_RANGING_AUTO:
        return "auto";
    case RBRINSTRUMENT_RANGING_COUNT:
        return "ranging mode count";
    case RBRINSTRUMENT_UNKNOWN_RANGING:
    default:
        return "unknown ranging mode";
    }
}

static RBRInstrumentError RBRInstrument_getCalibrations(
    RBRInstrument *instrument,
    RBRInstrumentChannels *channels)
{
    for (int32_t channel = 0; channel < channels->count; ++channel)
    {
        for (int32_t c = 0;
             c < RBRINSTRUMENT_CALIBRATION_C_COEFFICIENT_MAX;
             ++c)
        {
            channels->channels[channel].calibration.c[c] = NAN;
        }
        for (int32_t x = 0;
             x < RBRINSTRUMENT_CALIBRATION_X_COEFFICIENT_MAX;
             ++x)
        {
            channels->channels[channel].calibration.x[x] = NAN;
        }
    }

    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        RBR_TRY(RBRInstrument_converse(instrument, "calibration all"));
    }
    else
    {
        RBR_TRY(RBRInstrument_converse(instrument, "calibration allindices"));
    }

    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    int32_t channelIndex;
    RBRInstrumentChannel *channel;
    while (true)
    {
        RBRInstrument_parseResponse(instrument,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }

        channelIndex = parameter.index - 1;
        if (channelIndex < 0)
        {
            continue;
        }
        else if (channelIndex >= RBRINSTRUMENT_CHANNEL_MAX)
        {
            break;
        }
        else
        {
            channel = &channels->channels[channelIndex];
        }

        if (strcmp(parameter.key, "datetime") == 0)
        {
            RBR_TRY(RBRInstrumentDateTime_parseScheduleTime(
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
            && index < RBRINSTRUMENT_CALIBRATION_C_COEFFICIENT_MAX)
        {
            channel->calibration.c[index] = strtod(parameter.value, NULL);
        }
        else if (parameter.key[0] == 'x'
                 && index < RBRINSTRUMENT_CALIBRATION_X_COEFFICIENT_MAX)
        {
            channel->calibration.x[index] = strtod(parameter.value, NULL);
        }
        else if (parameter.key[0] == 'n'
                 && index < RBRINSTRUMENT_CALIBRATION_N_COEFFICIENT_MAX)
        {
            RBRInstrumentChannelIndex coefficient;
            if (strcmp(parameter.value, "value") == 0)
            {
                coefficient = RBRINSTRUMENT_VALUE_COEFFICIENT;
            }
            else
            {
                coefficient = strtol(parameter.value, NULL, 10);
            }

            channel->calibration.n[index] = coefficient;
        }
    }

    return RBRINSTRUMENT_SUCCESS;
}

static RBRInstrumentError RBRInstrument_getChannelAll(
    RBRInstrument *instrument,
    RBRInstrumentChannels *channels)
{
    for (int32_t channel = 0; channel < channels->count; ++channel)
    {
        channels->channels[channel].gain.currentGain = NAN;

        for (int32_t gain = 0; gain < RBRINSTRUMENT_CHANNEL_GAINS_MAX; ++gain)
        {
            channels->channels[channel].gain.availableGains[gain] = NAN;
        }

        snprintf(channels->channels[channel].label,
                 sizeof(channels->channels[channel].label),
                 "%s",
                 "none");
    }

    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        RBR_TRY(RBRInstrument_converse(
                    instrument,
                    "channel all all derived gain gainsavailable"));
    }
    else
    {
        RBR_TRY(RBRInstrument_converse(instrument, "channel allindices all"));
    }

    char *command = NULL;
    int32_t channelIndex;
    RBRInstrumentChannel *channel;
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

        channelIndex = parameter.index - 1;
        if (channelIndex < 0)
        {
            continue;
        }
        else if (channelIndex >= RBRINSTRUMENT_CHANNEL_MAX)
        {
            break;
        }
        else
        {
            channel = &channels->channels[channelIndex];
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
                channel->gain.rangingMode = RBRINSTRUMENT_RANGING_AUTO;
            }
            else
            {
                channel->gain.rangingMode = RBRINSTRUMENT_RANGING_MANUAL;
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
                   && gainCount < RBRINSTRUMENT_CHANNEL_GAINS_MAX)
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

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_getChannels(RBRInstrument *instrument,
                                             RBRInstrumentChannels *channels)
{
    memset(channels, 0, sizeof(RBRInstrumentChannels));

    RBR_TRY(RBRInstrument_converse(instrument, "channels"));

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

    RBR_TRY(RBRInstrument_getChannelAll(instrument, channels));
    RBR_TRY(RBRInstrument_getCalibrations(instrument, channels));

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_setChannelStatus(
    RBRInstrument *instrument,
    RBRInstrumentChannelIndex channel,
    bool status)
{
    return RBRInstrument_converse(instrument,
                                  "channel %d status = %s",
                                  channel,
                                  status ? "on" : "off");
}

RBRInstrumentError RBRInstrument_setChannelGain(
    RBRInstrument *instrument,
    RBRInstrumentChannelIndex channel,
    RBRInstrumentChannelGain *gain)
{
    if (gain->rangingMode == RBRINSTRUMENT_RANGING_MANUAL)
    {
        bool validGain = false;
        int32_t i;
        for (i = 0;
             i < RBRINSTRUMENT_CHANNEL_GAINS_MAX
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
            return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
        }

        return RBRInstrument_converse(instrument,
                                      "channel %d gain = %0.1f",
                                      channel,
                                      (double) gain->currentGain);
    }
    else if (gain->rangingMode == RBRINSTRUMENT_RANGING_AUTO)
    {
        return RBRInstrument_converse(instrument,
                                      "channel %d gain = auto",
                                      channel);
    }
    else
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }
}

RBRInstrumentError RBRInstrument_setCalibration(
    RBRInstrument *instrument,
    RBRInstrumentChannelIndex channel,
    const RBRInstrumentCalibration *calibration)
{
    char calibrationDateTime[RBRINSTRUMENT_SCHEDULE_TIME_LEN + 1];
    RBRInstrumentDateTime_toScheduleTime(calibration->dateTime,
                                         calibrationDateTime);

    const char *calibrationCommand = "calibration %d datetime = %s, %c%d = %g";

    for (int32_t c = 0;
         c < RBRINSTRUMENT_CALIBRATION_C_COEFFICIENT_MAX
         && !isnan(calibration->c[c]);
         ++c)
    {
        RBR_TRY(RBRInstrument_converse(instrument,
                                       calibrationCommand,
                                       channel,
                                       calibrationDateTime,
                                       'c',
                                       c,
                                       (double) calibration->c[c]));
    }
    for (int32_t x = 0;
         x < RBRINSTRUMENT_CALIBRATION_X_COEFFICIENT_MAX
         && !isnan(calibration->x[x]);
         ++x)
    {
        RBR_TRY(RBRInstrument_converse(instrument,
                                       calibrationCommand,
                                       channel,
                                       calibrationDateTime,
                                       'x',
                                       x,
                                       (double) calibration->x[x]));
    }

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_getFetchPowerOffDelay(
    RBRInstrument *instrument,
    RBRInstrumentPeriod *fetchPowerOffDelay)
{
    return RBRInstrument_getInt(instrument,
                                "settings",
                                "fetchpoweroffdelay",
                                fetchPowerOffDelay);
}

RBRInstrumentError RBRInstrument_setFetchPowerOffDelay(
    RBRInstrument *instrument,
    RBRInstrumentPeriod fetchPowerOffDelay)
{
    RBR_TRY(RBRInstrument_permit(instrument, "settings"));
    RBR_TRY(RBRInstrument_converse(instrument,
                                   "settings fetchpoweroffdelay = %d",
                                   fetchPowerOffDelay));
    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_isSensorPowerAlwaysOn(
    RBRInstrument *instrument,
    bool *sensorPowerAlwaysOn)
{
    return RBRInstrument_getBool(instrument,
                                 "settings",
                                 "sensorpoweralwayson",
                                 sensorPowerAlwaysOn);
}

RBRInstrumentError RBRInstrument_setSensorPowerAlwaysOn(
    RBRInstrument *instrument,
    bool sensorPowerAlwaysOn)
{
    RBR_TRY(RBRInstrument_permit(instrument, "settings"));
    RBR_TRY(RBRInstrument_converse(instrument,
                                   "settings sensorpoweralwayson = %s",
                                   sensorPowerAlwaysOn ? "on" : "off"));
    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_getCastDetection(RBRInstrument *instrument,
                                                  bool *castDetection)
{
    return RBRInstrument_getBool(instrument,
                                 "settings",
                                 "castdetection",
                                 castDetection);
}

RBRInstrumentError RBRInstrument_setCastDetection(RBRInstrument *instrument,
                                                  bool castDetection)
{
    RBR_TRY(RBRInstrument_permit(instrument, "settings"));
    RBR_TRY(RBRInstrument_converse(instrument,
                                   "settings castdetection = %s",
                                   castDetection ? "on" : "off"));
    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_getInputTimeout(
    RBRInstrument *instrument,
    RBRInstrumentPeriod *inputTimeout)
{
    return RBRInstrument_getInt(instrument,
                                "settings",
                                "inputtimeout",
                                inputTimeout);
}

RBRInstrumentError RBRInstrument_setInputTimeout(
    RBRInstrument *instrument,
    RBRInstrumentPeriod inputTimeout)
{
    if (inputTimeout < RBRINSTRUMENT_INPUT_TIMEOUT_MIN
        || inputTimeout > RBRINSTRUMENT_INPUT_TIMEOUT_MAX)
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    RBR_TRY(RBRInstrument_permit(instrument, "settings"));
    RBR_TRY(RBRInstrument_converse(instrument,
                                   "settings inputtimeout = %d",
                                   inputTimeout));
    return RBRINSTRUMENT_SUCCESS;
}

const char *RBRInstrumentValueSetting_name(RBRInstrumentValueSetting setting)
{
    switch (setting)
    {
    case RBRINSTRUMENT_SETTING_CONDUCTIVITY:
        return "conductivity";
    case RBRINSTRUMENT_SETTING_SPECCONDTEMPCO:
        return "speccondtempco";
    case RBRINSTRUMENT_SETTING_ALTITUDE:
        return "altitude";
    case RBRINSTRUMENT_SETTING_TEMPERATURE:
        return "temperature";
    case RBRINSTRUMENT_SETTING_PRESSURE:
        return "pressure";
    case RBRINSTRUMENT_SETTING_ATMOSPHERE:
        return "atmosphere";
    case RBRINSTRUMENT_SETTING_DENSITY:
        return "density";
    case RBRINSTRUMENT_SETTING_SALINITY:
        return "salinity";
    case RBRINSTRUMENT_SETTING_AVGSOUNDSPEED:
        return "avgsoundspeed";
    case RBRINSTRUMENT_SETTING_COUNT:
        return "setting count";
    case RBRINSTRUMENT_UNKNOWN_SETTING:
    default:
        return "unknown setting";
    }
}

RBRInstrumentError RBRInstrument_getValueSetting(
    RBRInstrument *instrument,
    RBRInstrumentValueSetting setting,
    float *value)
{
    if (setting < 0 || setting >= RBRINSTRUMENT_SETTING_COUNT)
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrument_getFloat(instrument,
                                  "settings",
                                  RBRInstrumentValueSetting_name(setting),
                                  value);
}

RBRInstrumentError RBRInstrument_setValueSetting(
    RBRInstrument *instrument,
    RBRInstrumentValueSetting setting,
    float value)
{
    if (setting < 0 || setting >= RBRINSTRUMENT_SETTING_COUNT)
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    RBR_TRY(RBRInstrument_permit(instrument, "settings"));
    RBR_TRY(RBRInstrument_converse(instrument,
                                   "settings %s = %f",
                                   RBRInstrumentValueSetting_name(setting),
                                   (double) value));
    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_getSensorParameter(
    RBRInstrument *instrument,
    RBRInstrumentChannelIndex channel,
    RBRInstrumentSensorParameter *parameter)
{
    memset(parameter->value, 0, sizeof(parameter->value));

    RBRInstrumentError err;
    /* Logger2 returns “E0501 item is not configured” when the requested
     * parameter doesn't exist, so we can't wrap the conversation in RBR_TRY
     * because we need to suppress that error. */
    err = RBRInstrument_converse(instrument,
                                 "sensor %d %s",
                                 channel,
                                 parameter->key);

    if (instrument->generation == RBRINSTRUMENT_LOGGER2
        && err == RBRINSTRUMENT_HARDWARE_ERROR
        && (instrument->response.error ==
            RBRINSTRUMENT_HARDWARE_ERROR_ITEM_IS_NOT_CONFIGURED))
    {
        snprintf(parameter->value,
                 sizeof(parameter->value),
                 "n/a");
        instrument->response.type = RBRINSTRUMENT_RESPONSE_INFO;
        return RBRINSTRUMENT_SUCCESS;
    }
    else if (err != RBRINSTRUMENT_SUCCESS)
    {
        return err;
    }

    char *command = NULL;
    RBRInstrumentResponseParameter responseParameter;
    while (true)
    {
        RBRInstrument_parseResponse(instrument,
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

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_getSensorParameters(
    RBRInstrument *instrument,
    RBRInstrumentChannelIndex channel,
    RBRInstrumentSensorParameter *parameters,
    int32_t *size)
{
    int32_t maxSize = *size;
    *size = 0;

    if (channel < 1 || channel > RBRINSTRUMENT_CHANNEL_MAX)
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    memset(parameters, 0, sizeof(RBRInstrumentSensorParameter) * maxSize);

    RBRInstrumentError err;
    /* Logger2 returns “E0109 feature not available” for channels which have no
     * sensor parameters, so we can't wrap the conversation in RBR_TRY because
     * we need to suppress that error. */
    err = RBRInstrument_converse(instrument, "sensor %d", channel);

    if (instrument->generation == RBRINSTRUMENT_LOGGER2
        && err == RBRINSTRUMENT_HARDWARE_ERROR
        && (instrument->response.error ==
            RBRINSTRUMENT_HARDWARE_ERROR_FEATURE_NOT_AVAILABLE))
    {
        instrument->response.type = RBRINSTRUMENT_RESPONSE_INFO;
        return RBRINSTRUMENT_SUCCESS;
    }
    else if (err != RBRINSTRUMENT_SUCCESS)
    {
        return err;
    }

    char channelStr[RBRINSTRUMENT_CHANNEL_MAX_LEN];
    snprintf(channelStr, sizeof(channelStr), "%i", channel);

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

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_setSensorParameter(
    RBRInstrument *instrument,
    RBRInstrumentChannelIndex channel,
    RBRInstrumentSensorParameter *parameter)
{
    return RBRInstrument_converse(instrument,
                                  "sensor %d %s = %s",
                                  channel,
                                  parameter->key,
                                  parameter->value);
}
