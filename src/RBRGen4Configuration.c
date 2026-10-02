/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file RBRGen4Configuration.c
 *
 * \brief Library implementation.
 */

/* Required for PRId32, PRId64. */
#include <inttypes.h>
/* Required for snprintf. */
#include <stdio.h>
/* Required for strtof, strtol, strtoll. */
#include <stdlib.h>
/* Required for memcpy, memset, strcat, strcmp. */
#include <string.h>

#include "RBRGen4.h"
#include "RBRGen4Internal.h"
#include "RBRGen4Configuration.h"

RBRGen4Error RBRGen4_getCalibration(RBRGen4 *conn, RBRGen4Calibration *calibration)
{
    /* The label selects the calibration to read, so it survives the reset. */
    RBR_RESET_EXCEPT(calibration, label);

    RBR_TRY(RBRGen4_converse(conn, "calibration %s", calibration->label));

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true) {
        RBRGen4_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        }

        /* A coefficient key is a group letter and an index: `a0`, `m11`. */
        char *end = NULL;
        char group = parameter.key[0];
        int32_t index = strtol(parameter.key + 1, &end, 10);
        bool coefficient = (parameter.key[1] != '\0' && *end == '\0' && index >= 0 &&
                            index < RBRGEN4_CALIBRATION_COEFFICIENT_MAX);

        if (strcmp(parameter.key, "equation") == 0) {
            snprintf(calibration->equation, sizeof(calibration->equation), "%s", parameter.value);
        } else if (strcmp(parameter.key, "datetime") == 0) {
            calibration->dateTime = strtoll(parameter.value, NULL, 10);
        } else if (strcmp(parameter.key, "offset") == 0) {
            calibration->userOffset = strtof(parameter.value, NULL);
        } else if (strcmp(parameter.key, "slope") == 0) {
            calibration->userSlope = strtof(parameter.value, NULL);
        } else if (coefficient && group == 'a') {
            calibration->a[index] = strtof(parameter.value, NULL);
            if (index >= calibration->aCount) {
                calibration->aCount = index + 1;
            }
        } else if (coefficient && group == 'b') {
            calibration->b[index] = strtof(parameter.value, NULL);
            if (index >= calibration->bCount) {
                calibration->bCount = index + 1;
            }
        } else if (coefficient && group == 'm') {
            /* An unused reference is reported as `none`. */
            if (strcmp(parameter.value, "none") == 0) {
                continue;
            }

            snprintf(calibration->m[index], sizeof(calibration->m[index]), "%s", parameter.value);
            if (index >= calibration->mCount) {
                calibration->mCount = index + 1;
            }
        }
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_setCalibration(RBRGen4 *conn, const RBRGen4Calibration *calibration)
{
    if (calibration->aCount < 0 || calibration->aCount > RBRGEN4_CALIBRATION_COEFFICIENT_MAX ||
        calibration->bCount < 0 || calibration->bCount > RBRGEN4_CALIBRATION_COEFFICIENT_MAX ||
        calibration->mCount < 0 || calibration->mCount > RBRGEN4_CALIBRATION_COEFFICIENT_MAX) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    RBRGen4_beginCommand(conn);
    RBR_TRY(RBRGen4_appendCommand(conn,
                                  "calibration %s datetime=%014" PRId64 " offset=%.9g slope=%.9g",
                                  calibration->label,
                                  calibration->dateTime,
                                  (double) calibration->userOffset,
                                  (double) calibration->userSlope));

    for (int32_t a = 0; a < calibration->aCount; ++a) {
        RBR_TRY(RBRGen4_appendCommand(conn, " a%" PRId32 "=%.9g", a, (double) calibration->a[a]));
    }
    for (int32_t b = 0; b < calibration->bCount; ++b) {
        RBR_TRY(RBRGen4_appendCommand(conn, " b%" PRId32 "=%.9g", b, (double) calibration->b[b]));
    }
    /* An empty reference label is sent as `none`, which is how the instrument
     * reports a reference the equation does not use. */
    for (int32_t m = 0; m < calibration->mCount; ++m) {
        RBR_TRY(RBRGen4_appendCommand(conn,
                                      " m%" PRId32 "=%s",
                                      m,
                                      calibration->m[m][0] == '\0' ? "none" : calibration->m[m]));
    }

    return RBRGen4_converseBuffer(conn);
}

const char *RBRGen4ScheduleStream_name(RBRGen4ScheduleStream stream)
{
    switch (stream) {
    case RBRGEN4_SCHEDULE_STREAM_OFF:
        return "off";
    case RBRGEN4_SCHEDULE_STREAM_USB:
        return "usb";
    case RBRGEN4_SCHEDULE_STREAM_SERIAL:
        return "serial";
    case RBRGEN4_SCHEDULE_STREAM_COUNT:
        return "stream destination count";
    case RBRGEN4_UNKNOWN_SCHEDULE_STREAM:
    default:
        return "unknown stream destination";
    }
}

const char *RBRGen4ChannelNature_name(RBRGen4ChannelNature nature)
{
    switch (nature) {
    case RBRGEN4_CHANNEL_NATURE_SCIENTIFIC:
        return "scientific";
    case RBRGEN4_CHANNEL_NATURE_SYSTEM:
        return "system";
    case RBRGEN4_CHANNEL_NATURE_COUNT:
        return "channel nature count";
    case RBRGEN4_UNKNOWN_CHANNEL_NATURE:
    default:
        return "unknown channel nature";
    }
}

static RBRGen4ChannelNature RBRGen4ChannelNature_parse(const char *value)
{
    for (int32_t nature = 0; nature < RBRGEN4_CHANNEL_NATURE_COUNT; ++nature) {
        if (strcmp(value, RBRGen4ChannelNature_name(nature)) == 0) {
            return nature;
        }
    }

    return RBRGEN4_UNKNOWN_CHANNEL_NATURE;
}

/**
 * \brief Read the labels of a `channel` response into a pool.
 *
 * Sets RBRGen4ChannelPool.len; the caller resets the pool's contents.
 */
static RBRGen4Error RBRGen4_parseChannelPool(RBRGen4 *conn, RBRGen4ChannelPool *channelPool)
{
    RBRGen4Error err = RBRGEN4_SUCCESS;
    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true) {
        RBRGen4_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "list") == 0) {
            /* An instrument with no channels reports `none`, not an empty
             * list. */
            if (strcmp(parameter.value, RBRGEN4_EMPTY_LIST) == 0) {
                continue;
            }

            /* Channels past the pool's capacity are discarded. */
            channelPool->len = 0;
            char *value = parameter.value;
            while (value != NULL) {
                if (channelPool->len >= channelPool->size) {
                    err = RBRGEN4_TRUNCATED;
                    break;
                }

                char *nextValue = RBRGen4_splitListValue(value);

                snprintf(channelPool->pool[channelPool->len].label,
                         sizeof(channelPool->pool[channelPool->len].label),
                         "%s",
                         value);
                channelPool->len++;

                value = nextValue;
            }
        }
    }

    return err;
}

RBRGen4Error RBRGen4_getChannel(RBRGen4 *conn, RBRGen4Channel *channel)
{
    if (channel->label[0] == '\0') {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    RBR_RESET_EXCEPT(channel, label);

    RBR_TRY(RBRGen4_converse(conn, "channel %s", channel->label));

    channel->nature = RBRGEN4_UNKNOWN_CHANNEL_NATURE;

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true) {
        RBRGen4_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "type") == 0) {
            snprintf(channel->type, sizeof(channel->type), "%s", parameter.value);
        } else if (strcmp(parameter.key, "settlingtime") == 0) {
            channel->settlingTime = strtol(parameter.value, NULL, 10);
        } else if (strcmp(parameter.key, "measuringtime") == 0) {
            channel->measuringTime = strtol(parameter.value, NULL, 10);
        } else if (strcmp(parameter.key, "readouttime") == 0) {
            channel->readOutTime = strtol(parameter.value, NULL, 10);
        } else if (strcmp(parameter.key, "userunits") == 0) {
            snprintf(channel->userUnits, sizeof(channel->userUnits), "%s", parameter.value);
        } else if (strcmp(parameter.key, "nature") == 0) {
            channel->nature = RBRGen4ChannelNature_parse(parameter.value);
        } else if (strcmp(parameter.key, "derived") == 0) {
            channel->derived = (strcmp(parameter.value, "true") == 0);
        }
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_setChannel(RBRGen4 *conn, const RBRGen4Channel *channel)
{
    if (channel->userUnits[0] == '\0') {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGen4_converse(conn, "channel %s userunits=%s", channel->label, channel->userUnits);
}

RBRGen4Error RBRGen4_getChannelPool(RBRGen4 *conn, RBRGen4ChannelPool *channelPool)
{
    if (channelPool->pool == NULL || channelPool->size <= 0) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }
    channelPool->len = 0;
    memset(channelPool->pool, 0, channelPool->size * sizeof(RBRGen4Channel));

    RBR_TRY(RBRGen4_converse(conn, "channel"));

    return RBRGen4_parseChannelPool(conn, channelPool);
}

RBRGen4Error RBRGen4_getChannelPoolByNature(RBRGen4 *conn, RBRGen4ChannelNature nature,
                                            RBRGen4ChannelPool *channelPool)
{
    if (nature < 0 || nature >= RBRGEN4_CHANNEL_NATURE_COUNT) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    if (channelPool->pool == NULL || channelPool->size <= 0) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }
    channelPool->len = 0;
    memset(channelPool->pool, 0, channelPool->size * sizeof(RBRGen4Channel));

    RBR_TRY(RBRGen4_converse(conn, "channel %s", RBRGen4ChannelNature_name(nature)));

    return RBRGen4_parseChannelPool(conn, channelPool);
}

RBRGen4Error RBRGen4_getChannelCount(RBRGen4 *conn, int32_t *count)
{
    return RBRGen4_getInt(conn, "channel", "count", count);
}

const char *RBRGen4SettingsState_name(RBRGen4SettingsState state)
{
    switch (state) {
    case RBRGEN4_SETTINGS_STATE_OFF:
        return "off";
    case RBRGEN4_SETTINGS_STATE_ON:
        return "on";
    case RBRGEN4_SETTINGS_STATE_COUNT:
        return "setting state count";
    case RBRGEN4_SETTINGS_STATE_UNAVAILABLE:
        return "unavailable setting state";
    case RBRGEN4_UNKNOWN_SETTINGS_STATE:
    default:
        return "unknown setting state";
    }
}

/**
 * \brief Find the setting state a response value names.
 *
 * \param [in] value the response value
 * \return the state, or #RBRGEN4_UNKNOWN_SETTINGS_STATE
 */
static RBRGen4SettingsState RBRGen4SettingsState_parse(const char *value)
{
    for (int i = 0; i < RBRGEN4_SETTINGS_STATE_COUNT; i++) {
        if (strcmp(RBRGen4SettingsState_name(i), value) == 0) {
            return i;
        }
    }

    return RBRGEN4_UNKNOWN_SETTINGS_STATE;
}

RBRGen4Error RBRGen4_getSettings(RBRGen4 *conn, RBRGen4Settings *settings)
{
    memset(settings, 0, sizeof(RBRGen4Settings));
    settings->prompt = RBRGEN4_SETTINGS_STATE_UNAVAILABLE;
    settings->confirmation = RBRGEN4_SETTINGS_STATE_UNAVAILABLE;

    RBR_TRY(RBRGen4_converse(conn, "settings"));

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true) {
        RBRGen4_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "prompt") == 0) {
            settings->prompt = RBRGen4SettingsState_parse(parameter.value);
        } else if (strcmp(parameter.key, "confirmation") == 0) {
            settings->confirmation = RBRGen4SettingsState_parse(parameter.value);
        } else if (strcmp(parameter.key, "pollpoweroffdelay") == 0) {
            settings->pollPowerOffDelay = strtol(parameter.value, NULL, 10);
        }
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_setSettings(RBRGen4 *conn, const RBRGen4Settings *settings)
{
    if (settings->pollPowerOffDelay < 0 ||
        (settings->prompt >= RBRGEN4_SETTINGS_STATE_COUNT &&
         settings->prompt != RBRGEN4_SETTINGS_STATE_UNAVAILABLE) ||
        (settings->confirmation >= RBRGEN4_SETTINGS_STATE_COUNT &&
         settings->confirmation != RBRGEN4_SETTINGS_STATE_UNAVAILABLE)) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    RBRGen4_beginCommand(conn);
    RBR_TRY(RBRGen4_appendCommand(conn, "settings "));

    /* An unavailable parameter is left out. */
    if (settings->prompt < RBRGEN4_SETTINGS_STATE_COUNT) {
        RBR_TRY(
            RBRGen4_appendCommand(conn, "prompt=%s ", RBRGen4SettingsState_name(settings->prompt)));
    }
    if (settings->confirmation < RBRGEN4_SETTINGS_STATE_COUNT) {
        RBR_TRY(RBRGen4_appendCommand(
            conn, "confirmation=%s ", RBRGen4SettingsState_name(settings->confirmation)));
    }

    RBR_TRY(RBRGen4_appendCommand(conn, "pollpoweroffdelay=%" PRId32, settings->pollPowerOffDelay));

    /* The instrument answers with nothing at all once confirmation is off. */
    if (settings->confirmation == RBRGEN4_SETTINGS_STATE_OFF) {
        RBR_TRY(RBRGen4_appendCommand(conn, RBRGEN4_SEND_COMMAND_TERMINATOR));
        return RBRGen4_sendBuffer(conn);
    }

    return RBRGen4_converseBuffer(conn);
}

RBRGen4Error RBRGen4_getParameters(RBRGen4 *conn, RBRGen4Parameters *parameters)
{
    memset(parameters, 0, sizeof(RBRGen4Parameters));

    RBR_TRY(RBRGen4_converse(conn, "parameters"));

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true) {
        RBRGen4_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "altitude") == 0) {
            parameters->altitude = strtof(parameter.value, NULL);
        } else if (strcmp(parameter.key, "atmosphere") == 0) {
            parameters->atmosphere = strtof(parameter.value, NULL);
        } else if (strcmp(parameter.key, "avgsoundspeed") == 0) {
            parameters->avgSoundSpeed = strtof(parameter.value, NULL);
        } else if (strcmp(parameter.key, "density") == 0) {
            parameters->density = strtof(parameter.value, NULL);
        } else if (strcmp(parameter.key, "pressure") == 0) {
            parameters->pressure = strtof(parameter.value, NULL);
        } else if (strcmp(parameter.key, "salinity") == 0) {
            parameters->salinity = strtof(parameter.value, NULL);
        } else if (strcmp(parameter.key, "speccondtempco") == 0) {
            parameters->specCondTempCo = strtof(parameter.value, NULL);
        } else if (strcmp(parameter.key, "temperature") == 0) {
            parameters->temperature = strtof(parameter.value, NULL);
        }
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_setParameters(RBRGen4 *conn, const RBRGen4Parameters *parameters)
{
    /* The instrument bounds these values; %.9g round-trips a float. */
    return RBRGen4_converse(conn,
                            "parameters altitude=%.9g atmosphere=%.9g avgsoundspeed=%.9g "
                            "density=%.9g pressure=%.9g salinity=%.9g speccondtempco=%.9g "
                            "temperature=%.9g",
                            (double) parameters->altitude,
                            (double) parameters->atmosphere,
                            (double) parameters->avgSoundSpeed,
                            (double) parameters->density,
                            (double) parameters->pressure,
                            (double) parameters->salinity,
                            (double) parameters->specCondTempCo,
                            (double) parameters->temperature);
}

RBRGen4Error RBRGen4_getGroup(RBRGen4 *conn, RBRGen4Group *group, RBRGen4LabelList *channelList)
{
    if (group->label[0] == '\0' ||
        (channelList != NULL && (channelList->labels == NULL || channelList->size <= 0))) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    RBR_RESET_EXCEPT(group, label);
    if (channelList != NULL) {
        channelList->len = 0;
    }

    RBR_TRY(RBRGen4_converse(conn, "group %s", group->label));

    RBRGen4Error err = RBRGEN4_SUCCESS;
    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true) {
        RBRGen4_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "channellist") == 0 && channelList != NULL) {
            err = RBRGen4_copyLabelList(channelList, parameter.value);
        }
    }

    return err;
}

RBRGen4Error RBRGen4_setGroup(RBRGen4 *conn, const RBRGen4Group *group,
                              const RBRGen4LabelList *channelList)
{
    if (group->label[0] == '\0') {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    RBRGen4_beginCommand(conn);
    RBR_TRY(RBRGen4_appendCommand(conn, "group %s channellist=", group->label));
    RBR_TRY(RBRGen4_appendLabelList(conn, channelList));
    return RBRGen4_converseBuffer(conn);
}

RBRGen4Error RBRGen4_getGroupPool(RBRGen4 *conn, RBRGen4GroupPool *groupPool)
{
    if (groupPool->pool == NULL || groupPool->size <= 0) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }
    groupPool->len = 0;
    memset(groupPool->pool, 0, groupPool->size * sizeof(RBRGen4Group));

    RBR_TRY(RBRGen4_converse(conn, "group"));

    RBRGen4Error err = RBRGEN4_SUCCESS;
    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true) {
        RBRGen4_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "list") == 0) {
            /* An empty pool reports `none`. */
            if (strcmp(parameter.value, RBRGEN4_EMPTY_LIST) == 0) {
                continue;
            }

            /* Groups past the pool's capacity are discarded. */
            groupPool->len = 0;
            char *value = parameter.value;
            while (value != NULL) {
                if (groupPool->len >= groupPool->size) {
                    err = RBRGEN4_TRUNCATED;
                    break;
                }

                char *nextValue = RBRGen4_splitListValue(value);

                snprintf(groupPool->pool[groupPool->len].label,
                         sizeof(groupPool->pool[groupPool->len].label),
                         "%s",
                         value);
                groupPool->len++;

                value = nextValue;
            }
        }
    }

    return err;
}

RBRGen4Error RBRGen4_getGroupCount(RBRGen4 *conn, int32_t *count)
{
    return RBRGen4_getInt(conn, "group", "count", count);
}

RBRGen4Error RBRGen4_getGroupMaxCount(RBRGen4 *conn, int32_t *maxCount)
{
    return RBRGen4_getInt(conn, "group", "maxcount", maxCount);
}

RBRGen4Error RBRGen4_createGroup(RBRGen4 *conn, const char *label)
{
    if (label[0] == '\0') {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGen4_converse(conn, "group create %s", label);
}

RBRGen4Error RBRGen4_deleteGroup(RBRGen4 *conn, const char *label)
{
    if (label[0] == '\0') {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGen4_converse(conn, "group delete %s", label);
}

RBRGen4Error RBRGen4_deleteGroupAll(RBRGen4 *conn)
{
    return RBRGen4_converse(conn, "group delete all");
}

RBRGen4Error RBRGen4_getConfig(RBRGen4 *conn, RBRGen4Config *config, RBRGen4LabelList *scheduleList)
{
    if (config->label[0] == '\0' ||
        (scheduleList != NULL && (scheduleList->labels == NULL || scheduleList->size <= 0))) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    RBR_RESET_EXCEPT(config, label);
    if (scheduleList != NULL) {
        scheduleList->len = 0;
    }

    RBR_TRY(RBRGen4_converse(conn, "config %s", config->label));

    RBRGen4Error err = RBRGEN4_SUCCESS;
    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true) {
        RBRGen4_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "schedulelist") == 0 && scheduleList != NULL) {
            err = RBRGen4_copyLabelList(scheduleList, parameter.value);
        }
    }

    return err;
}

RBRGen4Error RBRGen4_setConfig(RBRGen4 *conn, const RBRGen4Config *config,
                               const RBRGen4LabelList *scheduleList)
{
    if (config->label[0] == '\0') {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    RBRGen4_beginCommand(conn);
    RBR_TRY(RBRGen4_appendCommand(conn, "config %s schedulelist=", config->label));
    RBR_TRY(RBRGen4_appendLabelList(conn, scheduleList));
    return RBRGen4_converseBuffer(conn);
}

RBRGen4Error RBRGen4_getConfigPool(RBRGen4 *conn, RBRGen4ConfigPool *configPool)
{
    if (configPool->pool == NULL || configPool->size <= 0) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }
    configPool->len = 0;
    memset(configPool->pool, 0, configPool->size * sizeof(RBRGen4Config));

    RBR_TRY(RBRGen4_converse(conn, "config"));

    RBRGen4Error err = RBRGEN4_SUCCESS;
    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true) {
        RBRGen4_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "list") == 0) {
            /* An empty pool reports `none`. */
            if (strcmp(parameter.value, RBRGEN4_EMPTY_LIST) == 0) {
                continue;
            }

            /* Configurations past the pool's capacity are discarded. */
            configPool->len = 0;
            char *value = parameter.value;
            while (value != NULL) {
                if (configPool->len >= configPool->size) {
                    err = RBRGEN4_TRUNCATED;
                    break;
                }

                char *nextValue = RBRGen4_splitListValue(value);

                snprintf(configPool->pool[configPool->len].label,
                         sizeof(configPool->pool[configPool->len].label),
                         "%s",
                         value);
                configPool->len++;

                value = nextValue;
            }
        }
    }

    return err;
}

RBRGen4Error RBRGen4_getConfigCount(RBRGen4 *conn, int32_t *count)
{
    return RBRGen4_getInt(conn, "config", "count", count);
}

RBRGen4Error RBRGen4_getConfigMaxCount(RBRGen4 *conn, int32_t *maxCount)
{
    return RBRGen4_getInt(conn, "config", "maxcount", maxCount);
}

RBRGen4Error RBRGen4_createConfig(RBRGen4 *conn, const char *label)
{
    if (label[0] == '\0') {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGen4_converse(conn, "config create %s", label);
}

RBRGen4Error RBRGen4_deleteConfig(RBRGen4 *conn, const char *label)
{
    if (label[0] == '\0') {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGen4_converse(conn, "config delete %s", label);
}

RBRGen4Error RBRGen4_deleteConfigAll(RBRGen4 *conn)
{
    return RBRGen4_converse(conn, "config delete all");
}

const char *RBRGen4ScheduleMode_name(RBRGen4ScheduleMode mode)
{
    switch (mode) {
    case RBRGEN4_SCHEDULE_MODE_CONTINUOUS:
        return "continuous";
    case RBRGEN4_SCHEDULE_MODE_AVERAGE:
        return "average";
    case RBRGEN4_SCHEDULE_MODE_BURST:
        return "burst";
    case RBRGEN4_SCHEDULE_MODE_TIDE:
        return "tide";
    case RBRGEN4_SCHEDULE_MODE_WAVE:
        return "wave";
    case RBRGEN4_SCHEDULE_MODE_DDSAMPLING:
        return "ddsampling";
    case RBRGEN4_SCHEDULE_MODE_REGIMES:
        return "regimes";
    case RBRGEN4_SCHEDULE_MODE_NONE:
    default:
        return "unknown schedule mode";
    }
}

const char *RBRGen4ScheduleStorage_name(RBRGen4ScheduleStorage storage)
{
    switch (storage) {
    case RBRGEN4_SCHEDULE_STORAGE_OFF:
        return "off";
    case RBRGEN4_SCHEDULE_STORAGE_ON:
        return "on";
    case RBRGEN4_SCHEDULE_STORAGE_COUNT:
        return "schedule storage count";
    case RBRGEN4_SCHEDULE_STORAGE_UNAVAILABLE:
        return "unavailable schedule storage";
    case RBRGEN4_UNKNOWN_SCHEDULE_STORAGE:
    default:
        return "unknown schedule storage";
    }
}

/**
 * \brief Find the stream destination a response value names.
 *
 * \param [in] value the response value
 * \return the destination, or #RBRGEN4_UNKNOWN_SCHEDULE_STREAM
 */
static RBRGen4ScheduleStream RBRGen4ScheduleStream_parse(const char *value)
{
    for (int i = 0; i < RBRGEN4_SCHEDULE_STREAM_COUNT; i++) {
        if (strcmp(RBRGen4ScheduleStream_name(i), value) == 0) {
            return i;
        }
    }

    return RBRGEN4_UNKNOWN_SCHEDULE_STREAM;
}

/**
 * \brief Find the storage state a response value names.
 *
 * \param [in] value the response value
 * \return the state, or #RBRGEN4_UNKNOWN_SCHEDULE_STORAGE
 */
static RBRGen4ScheduleStorage RBRGen4ScheduleStorage_parse(const char *value)
{
    for (int i = 0; i < RBRGEN4_SCHEDULE_STORAGE_COUNT; i++) {
        if (strcmp(RBRGen4ScheduleStorage_name(i), value) == 0) {
            return i;
        }
    }

    return RBRGEN4_UNKNOWN_SCHEDULE_STORAGE;
}

/**
 * \brief Find the schedule mode a response value names.
 *
 * \param [in] value the response value
 * \return the mode, or #RBRGEN4_SCHEDULE_MODE_NONE
 */
static RBRGen4ScheduleMode RBRGen4ScheduleMode_parse(const char *value)
{
    for (int i = RBRGEN4_SCHEDULE_MODE_NONE + 1; i <= RBRGEN4_SCHEDULE_MODE_MAX; i <<= 1) {
        if (strcmp(RBRGen4ScheduleMode_name(i), value) == 0) {
            return i;
        }
    }

    return RBRGEN4_SCHEDULE_MODE_NONE;
}

RBRGen4Error RBRGen4_getSchedule(RBRGen4 *conn, RBRGen4Schedule *schedule,
                                 RBRGen4LabelList *groupList)
{
    if (schedule->label[0] == '\0' ||
        (groupList != NULL && (groupList->labels == NULL || groupList->size <= 0))) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    RBR_RESET_EXCEPT(schedule, label);
    schedule->stream = RBRGEN4_UNKNOWN_SCHEDULE_STREAM;
    schedule->storage = RBRGEN4_SCHEDULE_STORAGE_UNAVAILABLE;
    schedule->mode = RBRGEN4_SCHEDULE_MODE_NONE;
    if (groupList != NULL) {
        groupList->len = 0;
    }

    RBR_TRY(RBRGen4_converse(conn, "schedule %s", schedule->label));

    RBRGen4Period period = 0;
    RBRGen4Period measurementPeriod = 0;
    int32_t measurementCount = 0;

    RBRGen4Error err = RBRGEN4_SUCCESS;
    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true) {
        RBRGen4_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "grouplist") == 0 && groupList != NULL) {
            err = RBRGen4_copyLabelList(groupList, parameter.value);
        } else if (strcmp(parameter.key, "stream") == 0) {
            schedule->stream = RBRGen4ScheduleStream_parse(parameter.value);
        } else if (strcmp(parameter.key, "storage") == 0) {
            schedule->storage = RBRGen4ScheduleStorage_parse(parameter.value);
        } else if (strcmp(parameter.key, "castdetection") == 0) {
            schedule->castDetection = strcmp(parameter.value, "on") == 0;
        } else if (strcmp(parameter.key, "mode") == 0) {
            schedule->mode = RBRGen4ScheduleMode_parse(parameter.value);
        } else if (strcmp(parameter.key, "period") == 0) {
            period = strtol(parameter.value, NULL, 10);
        } else if (strcmp(parameter.key, "measurementperiod") == 0) {
            measurementPeriod = strtol(parameter.value, NULL, 10);
        } else if (strcmp(parameter.key, "measurementcount") == 0) {
            measurementCount = strtol(parameter.value, NULL, 10);
        }
    }

    switch (schedule->mode) {
    case RBRGEN4_SCHEDULE_MODE_CONTINUOUS:
        schedule->parameters.continuous.period = period;
        break;
    case RBRGEN4_SCHEDULE_MODE_AVERAGE:
    case RBRGEN4_SCHEDULE_MODE_BURST:
    case RBRGEN4_SCHEDULE_MODE_TIDE:
    case RBRGEN4_SCHEDULE_MODE_WAVE:
        schedule->parameters.bursting.period = period;
        schedule->parameters.bursting.measurementPeriod = measurementPeriod;
        schedule->parameters.bursting.measurementCount = measurementCount;
        break;
    default:
        break;
    }

    return err;
}

RBRGen4Error RBRGen4_setSchedule(RBRGen4 *conn, const RBRGen4Schedule *schedule,
                                 const RBRGen4LabelList *groupList)
{
    /* A schedule runs in exactly one mode. */
    if (schedule->label[0] == '\0' || schedule->stream >= RBRGEN4_SCHEDULE_STREAM_COUNT ||
        (schedule->storage >= RBRGEN4_SCHEDULE_STORAGE_COUNT &&
         schedule->storage != RBRGEN4_SCHEDULE_STORAGE_UNAVAILABLE) ||
        schedule->mode == RBRGEN4_SCHEDULE_MODE_NONE ||
        schedule->mode > RBRGEN4_SCHEDULE_MODE_MAX ||
        (schedule->mode & (schedule->mode - 1)) != 0) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    RBRGen4Period period;
    RBRGen4Period measurementPeriod = 0;
    int32_t measurementCount = 0;

    switch (schedule->mode) {
    case RBRGEN4_SCHEDULE_MODE_CONTINUOUS:
        period = schedule->parameters.continuous.period;
        break;
    case RBRGEN4_SCHEDULE_MODE_AVERAGE:
    case RBRGEN4_SCHEDULE_MODE_BURST:
    case RBRGEN4_SCHEDULE_MODE_TIDE:
    case RBRGEN4_SCHEDULE_MODE_WAVE:
        period = schedule->parameters.bursting.period;
        measurementPeriod = schedule->parameters.bursting.measurementPeriod;
        measurementCount = schedule->parameters.bursting.measurementCount;
        break;
    default:
        /* No parameters are modelled for `ddsampling` or `regimes`, so
         * writing either would silently drop them. */
        return RBRGEN4_UNSUPPORTED;
    }

    RBRGen4_beginCommand(conn);
    RBR_TRY(RBRGen4_appendCommand(conn, "schedule %s ", schedule->label));

    /* A NULL list leaves the instrument's group list unchanged. */
    if (groupList != NULL) {
        RBR_TRY(RBRGen4_appendCommand(conn, "grouplist="));
        RBR_TRY(RBRGen4_appendLabelList(conn, groupList));
        RBR_TRY(RBRGen4_appendCommand(conn, " "));
    }

    /* An unavailable `storage` is left out. */
    const char *storage = "";
    if (schedule->storage != RBRGEN4_SCHEDULE_STORAGE_UNAVAILABLE) {
        storage = schedule->storage == RBRGEN4_SCHEDULE_STORAGE_ON ? "storage=on " : "storage=off ";
    }

    RBR_TRY(RBRGen4_appendCommand(conn,
                                  "stream=%s %scastdetection=%s mode=%s period=%" PRId32,
                                  RBRGen4ScheduleStream_name(schedule->stream),
                                  storage,
                                  schedule->castDetection ? "on" : "off",
                                  RBRGen4ScheduleMode_name(schedule->mode),
                                  period));
    if (schedule->mode != RBRGEN4_SCHEDULE_MODE_CONTINUOUS) {
        RBR_TRY(RBRGen4_appendCommand(conn,
                                      " measurementcount=%" PRId32 " measurementperiod=%" PRId32,
                                      measurementCount,
                                      measurementPeriod));
    }

    return RBRGen4_converseBuffer(conn);
}

RBRGen4Error RBRGen4_getSchedulePool(RBRGen4 *conn, RBRGen4SchedulePool *schedulePool)
{
    if (schedulePool->pool == NULL || schedulePool->size <= 0) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }
    schedulePool->len = 0;
    memset(schedulePool->pool, 0, schedulePool->size * sizeof(RBRGen4Schedule));
    schedulePool->maxRegimes = 0;

    RBR_TRY(RBRGen4_converse(conn, "schedule"));

    RBRGen4Error err = RBRGEN4_SUCCESS;
    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true) {
        RBRGen4_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "maxregimes") == 0) {
            schedulePool->maxRegimes = strtol(parameter.value, NULL, 10);
        } else if (strcmp(parameter.key, "list") == 0) {
            /* An empty pool reports `none`. */
            if (strcmp(parameter.value, RBRGEN4_EMPTY_LIST) == 0) {
                continue;
            }

            /* Schedules past the pool's capacity are discarded. */
            schedulePool->len = 0;
            char *value = parameter.value;
            while (value != NULL) {
                if (schedulePool->len >= schedulePool->size) {
                    err = RBRGEN4_TRUNCATED;
                    break;
                }

                char *nextValue = RBRGen4_splitListValue(value);

                snprintf(schedulePool->pool[schedulePool->len].label,
                         sizeof(schedulePool->pool[schedulePool->len].label),
                         "%s",
                         value);
                schedulePool->len++;

                value = nextValue;
            }
        }
    }

    return err;
}

RBRGen4Error RBRGen4_getScheduleCount(RBRGen4 *conn, int32_t *count)
{
    return RBRGen4_getInt(conn, "schedule", "count", count);
}

RBRGen4Error RBRGen4_getScheduleMaxCount(RBRGen4 *conn, int32_t *maxCount)
{
    return RBRGen4_getInt(conn, "schedule", "maxcount", maxCount);
}

RBRGen4Error RBRGen4_createSchedule(RBRGen4 *conn, const char *label)
{
    if (label[0] == '\0') {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGen4_converse(conn, "schedule create %s", label);
}

RBRGen4Error RBRGen4_deleteSchedule(RBRGen4 *conn, const char *label)
{
    if (label[0] == '\0') {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGen4_converse(conn, "schedule delete %s", label);
}

RBRGen4Error RBRGen4_deleteScheduleAll(RBRGen4 *conn)
{
    return RBRGen4_converse(conn, "schedule delete all");
}
