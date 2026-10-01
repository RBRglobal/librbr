/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file RBRGen3Configuration.c
 *
 * \brief Library implementation.
 */

/* Required for NAN. */
#include <math.h>
/* Required for snprintf. */
#include <stdio.h>
/* Required for strtod, strtol. */
#include <stdlib.h>
/* Required for memset, strcmp. */
#include <string.h>

#include "RBRGen3.h"
#include "RBRGen3Internal.h"
#include "RBRGen3Configuration.h"
#include "RBRGen3Security.h"

const char *RBRGen3ChannelRangingMode_name(RBRGen3ChannelRangingMode mode)
{
    switch (mode) {
    case RBRGEN3_RANGING_NONE:
        return "none";
    case RBRGEN3_RANGING_MANUAL:
        return "manual";
    case RBRGEN3_RANGING_AUTO:
        return "auto";
    case RBRGEN3_RANGING_COUNT:
        return "ranging mode count";
    case RBRGEN3_UNKNOWN_RANGING:
    default:
        return "unknown ranging mode";
    }
}

static RBRGen3Error RBRGen3_clearChannel(RBRGen3Channel *channel)
{
    channel->gain.currentGain = NAN;

    for (int32_t gain = 0; gain < RBRGEN3_CHANNEL_GAINS_MAX; ++gain) {
        channel->gain.availableGains[gain] = NAN;
    }

    snprintf(channel->label, sizeof(channel->label), "%s", "none");

    return RBRGEN3_SUCCESS;
}

static RBRGen3Error RBRGen3_getChannelCoefficients(RBRGen3 *conn, int32_t channelIndex,
                                                   RBRGen3Channel *channel)
{
    RBR_TRY(RBRGen3_converse(conn, "calibration %d all", channelIndex + 1));

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true) {
        RBRGen3_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        }

        if (strcmp(parameter.key, "datetime") == 0) {
            RBR_TRY(RBRGen3DateTime_parseScheduleTime(
                parameter.value, &channel->calibration.dateTime, NULL));
        } else if (parameter.key[0] != 'c' && parameter.key[0] != 'x' && parameter.key[0] != 'n') {
            continue;
        }

        /* A coefficient key is a group letter and an index: `c0`, `x15`. */
        char *end = NULL;
        int32_t index = strtol(&parameter.key[1], &end, 10);
        if (parameter.key[1] == '\0' || *end != '\0') {
            continue;
        }

        if (parameter.key[0] == 'c' && index >= 0 &&
            index < RBRGEN3_CALIBRATION_C_COEFFICIENT_MAX) {
            channel->calibration.c[index] = strtod(parameter.value, NULL);
            if (index >= channel->calibration.cCount) {
                channel->calibration.cCount = index + 1;
            }
        } else if (parameter.key[0] == 'x' && index >= 0 &&
                   index < RBRGEN3_CALIBRATION_X_COEFFICIENT_MAX) {
            channel->calibration.x[index] = strtod(parameter.value, NULL);
            if (index >= channel->calibration.xCount) {
                channel->calibration.xCount = index + 1;
            }
        } else if (parameter.key[0] == 'n' && index >= 0 &&
                   index < RBRGEN3_CALIBRATION_N_COEFFICIENT_MAX) {
            RBRGen3ChannelIndex coefficient;
            if (strcmp(parameter.value, "value") == 0) {
                coefficient = RBRGEN3_VALUE_COEFFICIENT;
            } else {
                coefficient = strtol(parameter.value, NULL, 10);
            }

            channel->calibration.n[index] = coefficient;
            if (index >= channel->calibration.nCount) {
                channel->calibration.nCount = index + 1;
            }
        }
    }

    return RBRGEN3_SUCCESS;
}

static RBRGen3Error RBRGen3_getChannel(RBRGen3 *conn, int32_t channelIndex, RBRGen3Channel *channel)
{
    if (conn->generation == RBRCOMMON_LOGGER2) {
        RBR_TRY(
            RBRGen3_converse(conn, "channel %d all derived gain gainsavailable", channelIndex + 1));
    } else {
        RBR_TRY(RBRGen3_converse(conn, "channel %d all", channelIndex + 1));
    }

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true) {
        RBRGen3_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        }

        if (strcmp(parameter.key, "type") == 0) {
            snprintf(channel->type, sizeof(channel->type), "%s", parameter.value);
        } else if (strcmp(parameter.key, "module") == 0) {
            channel->moduleAddr = strtol(parameter.value, NULL, 10);
        } else if (strcmp(parameter.key, "status") == 0) {
            channel->status = (strcmp(parameter.value, "on") == 0);
        } else if (strcmp(parameter.key, "settlingtime") == 0 ||
                   strcmp(parameter.key, "latency") == 0) {
            channel->settlingTime = strtol(parameter.value, NULL, 10);
        } else if (strcmp(parameter.key, "readtime") == 0) {
            channel->readTime = strtol(parameter.value, NULL, 10);
        } else if (strcmp(parameter.key, "equation") == 0) {
            snprintf(channel->equation, sizeof(channel->equation), "%s", parameter.value);
        } else if (strcmp(parameter.key, "userunits") == 0) {
            snprintf(channel->userUnits, sizeof(channel->userUnits), "%s", parameter.value);
        } else if (strcmp(parameter.key, "derived") == 0) {
            channel->derived = (strcmp(parameter.value, "on") == 0);
        } else if (strcmp(parameter.key, "gain") == 0) {
            if (strcmp(parameter.value, "none") == 0) {
                continue;
            } else if (strcmp(parameter.value, "auto") == 0) {
                channel->gain.rangingMode = RBRGEN3_RANGING_AUTO;
            } else {
                channel->gain.rangingMode = RBRGEN3_RANGING_MANUAL;
                channel->gain.currentGain = strtod(parameter.value, NULL);
            }
        } else if (strcmp(parameter.key, "availablegains") == 0 ||
                   strcmp(parameter.key, "gainsavailable") == 0) {
            if (strcmp(parameter.value, "none") == 0) {
                continue;
            }

            char *gains = parameter.value;
            int32_t gainCount = 0;
            char *gain = NULL;
            while ((gain = strtok(gains, "|")) != NULL && gainCount < RBRGEN3_CHANNEL_GAINS_MAX) {
                gains = NULL;
                channel->gain.availableGains[gainCount++] = strtod(gain, NULL);
            }
        } else if (strcmp(parameter.key, "label") == 0) {
            snprintf(channel->label, sizeof(channel->label), "%s", parameter.value);
        }
    }

    return RBRGEN3_SUCCESS;
}

typedef enum RBRGen3ChannelDensity {
    /** Channel has no additional information populated. */
    RBRGEN3_CHANNEL_SPARSE = 0,
    /** Channel's calibration information is populated. */
    RBRGEN3_CHANNEL_CALIBRATION = 1 << 0,
} RBRGen3ChannelDensity;

static RBRGen3Error RBRGen3_getChannelAll(RBRGen3 *conn, RBRGen3Channels *channels,
                                          RBRGen3ChannelDensity density)
{
    for (int32_t idx = 0; idx < channels->len; ++idx) {
        RBRGen3Channel *channel = &channels->channels[idx];
        RBRGen3_clearChannel(channel);
        RBR_TRY(RBRGen3_getChannel(conn, idx, channel));

        if (density & RBRGEN3_CHANNEL_CALIBRATION) {
            RBR_TRY(RBRGen3_getChannelCoefficients(conn, idx, channel));
        }
    }

    return RBRGEN3_SUCCESS;
}

static RBRGen3Error RBRGen3_getChannelsWithDensity(RBRGen3 *conn, RBRGen3Channels *channels,
                                                   RBRGen3ChannelDensity density)
{
    if (channels->channels == NULL || channels->size <= 0) {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }
    /* The channel storage belongs to the caller; clear it, not the pointer. */
    RBRGen3Channel *storage = channels->channels;
    int32_t size = channels->size;
    memset(storage, 0, (size_t) size * sizeof(*storage));
    memset(channels, 0, sizeof(RBRGen3Channels));
    channels->channels = storage;
    channels->size = size;

    RBR_TRY(RBRGen3_converse(conn, "channels"));

    /* Channels past the caller's storage are not fetched; the count reported
     * by the instrument decides whether the result is truncated. */
    int32_t count = 0;
    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true) {
        RBRGen3_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        }

        if (strcmp(parameter.key, "count") == 0) {
            count = strtol(parameter.value, NULL, 10);
            if (count < 0) {
                count = 0;
            }
        } else if (strcmp(parameter.key, "settlingtime") == 0 ||
                   strcmp(parameter.key, "latency") == 0) {
            channels->settlingTime = strtol(parameter.value, NULL, 10);
        } else if (strcmp(parameter.key, "readtime") == 0) {
            channels->readTime = strtol(parameter.value, NULL, 10);
        } else if (strcmp(parameter.key, "minperiod") == 0) {
            channels->minimumPeriod = strtol(parameter.value, NULL, 10);
        }
    }

    channels->len = count > channels->size ? channels->size : count;
    RBR_TRY(RBRGen3_getChannelAll(conn, channels, density));
    return count > channels->size ? RBRGEN3_TRUNCATED : RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_getChannelCount(RBRGen3 *conn, int32_t *count)
{
    return RBRGen3_getInt(conn, "channels", "count", count);
}

RBRGen3Error RBRGen3_getEnabledChannelCount(RBRGen3 *conn, int32_t *count)
{
    return RBRGen3_getInt(conn, "channels", "on", count);
}

RBRGen3Error RBRGen3_getChannels(RBRGen3 *conn, RBRGen3Channels *channels)
{
    return RBRGen3_getChannelsWithDensity(conn, channels, RBRGEN3_CHANNEL_CALIBRATION);
}

RBRGen3Error RBRGen3_getChannelsWithoutCalibrations(RBRGen3 *conn, RBRGen3Channels *channels)
{
    return RBRGen3_getChannelsWithDensity(conn, channels, RBRGEN3_CHANNEL_SPARSE);
}

RBRGen3Error RBRGen3_setChannelStatus(RBRGen3 *conn, RBRGen3ChannelIndex channel, bool status)
{
    return RBRGen3_converse(conn, "channel %d status = %s", channel, status ? "on" : "off");
}

RBRGen3Error RBRGen3_setChannelGain(RBRGen3 *conn, RBRGen3ChannelIndex channel,
                                    RBRGen3ChannelGain *gain)
{
    if (gain->rangingMode == RBRGEN3_RANGING_MANUAL) {
        bool validGain = false;
        int32_t i;
        for (i = 0; i < RBRGEN3_CHANNEL_GAINS_MAX && !isnan(gain->availableGains[i]); ++i) {
            if (gain->currentGain == gain->availableGains[i]) {
                validGain = true;
                break;
            }
        }

        if (i > 0 && !validGain) {
            return RBRGEN3_INVALID_PARAMETER_VALUE;
        }

        return RBRGen3_converse(
            conn, "channel %d gain = %0.1f", channel, (double) gain->currentGain);
    } else if (gain->rangingMode == RBRGEN3_RANGING_AUTO) {
        return RBRGen3_converse(conn, "channel %d gain = auto", channel);
    } else {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }
}

RBRGen3Error RBRGen3_setCalibration(RBRGen3 *conn, RBRGen3ChannelIndex channel,
                                    const RBRGen3Calibration *calibration)
{
    if (calibration->dateTime < RBRGEN3_DATETIME_MIN ||
        calibration->dateTime > RBRGEN3_DATETIME_MAX) {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    if (calibration->cCount < 0 || calibration->cCount > RBRGEN3_CALIBRATION_C_COEFFICIENT_MAX ||
        calibration->xCount < 0 || calibration->xCount > RBRGEN3_CALIBRATION_X_COEFFICIENT_MAX) {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    if (calibration->cCount == 0 && calibration->xCount == 0) {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    char calibrationDateTime[RBRGEN3_SCHEDULE_TIME_LEN + 1];
    RBRGen3DateTime_toScheduleTime(calibration->dateTime, calibrationDateTime);

    const char *calibrationCommand = "calibration %d datetime = %s, %c%d = %g";

    for (int32_t c = 0; c < calibration->cCount; ++c) {
        RBR_TRY(RBRGen3_converse(conn,
                                 calibrationCommand,
                                 channel,
                                 calibrationDateTime,
                                 'c',
                                 c,
                                 (double) calibration->c[c]));
    }
    for (int32_t x = 0; x < calibration->xCount; ++x) {
        RBR_TRY(RBRGen3_converse(conn,
                                 calibrationCommand,
                                 channel,
                                 calibrationDateTime,
                                 'x',
                                 x,
                                 (double) calibration->x[x]));
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_getFetchPowerOffDelay(RBRGen3 *conn, RBRGen3Period *fetchPowerOffDelay)
{
    return RBRGen3_getInt(conn, "settings", "fetchpoweroffdelay", fetchPowerOffDelay);
}

RBRGen3Error RBRGen3_setFetchPowerOffDelay(RBRGen3 *conn, RBRGen3Period fetchPowerOffDelay)
{
    RBR_TRY(RBRGen3_permit(conn, "settings"));
    RBR_TRY(RBRGen3_converse(conn, "settings fetchpoweroffdelay = %d", fetchPowerOffDelay));
    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_isSensorPowerAlwaysOn(RBRGen3 *conn, bool *sensorPowerAlwaysOn)
{
    return RBRGen3_getBool(conn, "settings", "sensorpoweralwayson", sensorPowerAlwaysOn);
}

RBRGen3Error RBRGen3_setSensorPowerAlwaysOn(RBRGen3 *conn, bool sensorPowerAlwaysOn)
{
    RBR_TRY(RBRGen3_permit(conn, "settings"));
    RBR_TRY(RBRGen3_converse(
        conn, "settings sensorpoweralwayson = %s", sensorPowerAlwaysOn ? "on" : "off"));
    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_getCastDetection(RBRGen3 *conn, bool *castDetection)
{
    return RBRGen3_getBool(conn, "settings", "castdetection", castDetection);
}

RBRGen3Error RBRGen3_setCastDetection(RBRGen3 *conn, bool castDetection)
{
    RBR_TRY(RBRGen3_permit(conn, "settings"));
    RBR_TRY(RBRGen3_converse(conn, "settings castdetection = %s", castDetection ? "on" : "off"));
    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_getInputTimeout(RBRGen3 *conn, RBRGen3Period *inputTimeout)
{
    return RBRGen3_getInt(conn, "settings", "inputtimeout", inputTimeout);
}

RBRGen3Error RBRGen3_setInputTimeout(RBRGen3 *conn, RBRGen3Period inputTimeout)
{
    if (inputTimeout < RBRGEN3_INPUT_TIMEOUT_MIN || inputTimeout > RBRGEN3_INPUT_TIMEOUT_MAX) {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    RBR_TRY(RBRGen3_permit(conn, "settings"));
    RBR_TRY(RBRGen3_converse(conn, "settings inputtimeout = %d", inputTimeout));
    return RBRGEN3_SUCCESS;
}

const char *RBRGen3ValueSetting_name(RBRGen3ValueSetting setting)
{
    switch (setting) {
    case RBRGEN3_SETTING_CONDUCTIVITY:
        return "conductivity";
    case RBRGEN3_SETTING_SPECCONDTEMPCO:
        return "speccondtempco";
    case RBRGEN3_SETTING_ALTITUDE:
        return "altitude";
    case RBRGEN3_SETTING_TEMPERATURE:
        return "temperature";
    case RBRGEN3_SETTING_PRESSURE:
        return "pressure";
    case RBRGEN3_SETTING_ATMOSPHERE:
        return "atmosphere";
    case RBRGEN3_SETTING_DENSITY:
        return "density";
    case RBRGEN3_SETTING_SALINITY:
        return "salinity";
    case RBRGEN3_SETTING_AVGSOUNDSPEED:
        return "avgsoundspeed";
    case RBRGEN3_SETTING_COUNT:
        return "setting count";
    case RBRGEN3_UNKNOWN_SETTING:
    default:
        return "unknown setting";
    }
}

RBRGen3Error RBRGen3_getValueSetting(RBRGen3 *conn, RBRGen3ValueSetting setting, float *value)
{
    if (setting < 0 || setting >= RBRGEN3_SETTING_COUNT) {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    return RBRGen3_getFloat(conn, "settings", RBRGen3ValueSetting_name(setting), value);
}

RBRGen3Error RBRGen3_setValueSetting(RBRGen3 *conn, RBRGen3ValueSetting setting, float value)
{
    if (setting < 0 || setting >= RBRGEN3_SETTING_COUNT) {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    RBR_TRY(RBRGen3_permit(conn, "settings"));
    RBR_TRY(RBRGen3_converse(
        conn, "settings %s = %f", RBRGen3ValueSetting_name(setting), (double) value));
    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_getSensorParameter(RBRGen3 *conn, RBRGen3ChannelIndex channel,
                                        RBRGen3SensorParameter *parameter)
{
    memset(parameter->value, 0, sizeof(parameter->value));

    RBRGen3Error err;
    /* Logger2 returns “E0501 item is not configured” when the requested
     * parameter doesn't exist, so we can't wrap the conversation in RBR_TRY
     * because we need to suppress that error. */
    err = RBRGen3_converse(conn, "sensor %d %s", channel, parameter->key);

    if (conn->generation == RBRCOMMON_LOGGER2 && err == RBRGEN3_HARDWARE_ERROR &&
        (conn->response.error == RBRGEN3_HARDWARE_ERROR_ITEM_IS_NOT_CONFIGURED)) {
        snprintf(parameter->value, sizeof(parameter->value), "n/a");
        conn->response.type = RBRGEN3_RESPONSE_INFO;
        return RBRGEN3_SUCCESS;
    } else if (err != RBRGEN3_SUCCESS) {
        return err;
    }

    char *command = NULL;
    RBRGen3ResponseParameter responseParameter;
    while (true) {
        RBRGen3_parseResponse(conn, &command, &responseParameter);

        if (responseParameter.key == NULL) {
            break;
        }

        snprintf(parameter->key, sizeof(parameter->key), "%s", responseParameter.key);

        snprintf(parameter->value, sizeof(parameter->value), "%s", responseParameter.value);
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_getSensorParameters(RBRGen3 *conn, RBRGen3ChannelIndex channel,
                                         RBRGen3SensorParameter *parameters, int32_t *size)
{
    int32_t maxSize = *size;
    *size = 0;

    if (channel < 1) {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    memset(parameters, 0, sizeof(RBRGen3SensorParameter) * maxSize);

    RBRGen3Error err;
    /* Logger2 returns “E0109 feature not available” for channels which have no
     * sensor parameters, so we can't wrap the conversation in RBR_TRY because
     * we need to suppress that error. */
    err = RBRGen3_converse(conn, "sensor %d", channel);

    if (conn->generation == RBRCOMMON_LOGGER2 && err == RBRGEN3_HARDWARE_ERROR &&
        (conn->response.error == RBRGEN3_HARDWARE_ERROR_FEATURE_NOT_AVAILABLE)) {
        conn->response.type = RBRGEN3_RESPONSE_INFO;
        return RBRGEN3_SUCCESS;
    } else if (err != RBRGEN3_SUCCESS) {
        return err;
    }

    /* Room for any RBRGen3ChannelIndex in decimal. */
    char channelStr[sizeof("255")];
    snprintf(channelStr, sizeof(channelStr), "%i", channel);

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true) {
        RBRGen3_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
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
        if (strcmp(parameter.key, channelStr) == 0 && strlen(parameter.value) == 0) {
            break;
        }

        snprintf(parameters[*size].key, sizeof(parameters[*size].key), "%s", parameter.key);

        snprintf(parameters[*size].value, sizeof(parameters[*size].value), "%s", parameter.value);

        if (++(*size) >= maxSize) {
            break;
        }
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_setSensorParameter(RBRGen3 *conn, RBRGen3ChannelIndex channel,
                                        RBRGen3SensorParameter *parameter)
{
    return RBRGen3_converse(conn, "sensor %d %s = %s", channel, parameter->key, parameter->value);
}
