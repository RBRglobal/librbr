/**
 * \file RBRGen3Gating.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for snprintf. */
#include <stdio.h>
/* Required for memset, strcmp. */
#include <string.h>

#include "RBRGen3.h"
#include "RBRGen3Internal.h"

const char *RBRGen3GatingState_name(RBRGen3GatingState state)
{
    switch (state)
    {
    case RBRGEN3_GATING_NA:
        return "n/a";
    case RBRGEN3_GATING_PAUSED:
        return "paused";
    case RBRGEN3_GATING_RUNNING:
        return "running";
    case RBRGEN3_GATING_COUNT:
        return "gating state count";
    case RBRGEN3_UNKNOWN_GATING:
    default:
        return "unknown gating state";
    }
}

const char *RBRGen3ThresholdingChannelSelection_name(
    RBRGen3ThresholdingChannelSelection selection)
{
    switch (selection)
    {
    case RBRGEN3_THRESHOLD_CHANNEL_BY_INDEX:
        return "index";
    case RBRGEN3_THRESHOLD_CHANNEL_BY_LABEL:
        return "label";
    default:
        return "unknown thresholding channel selection";
    }
}

const char *RBRGen3ThresholdingCondition_name(
    RBRGen3ThresholdingCondition condition)
{
    switch (condition)
    {
    case RBRGEN3_THRESHOLDING_ABOVE:
        return "above";
    case RBRGEN3_THRESHOLDING_BELOW:
        return "below";
    case RBRGEN3_THRESHOLDING_COUNT:
        return "thresholding condition count";
    case RBRGEN3_UNKNOWN_THRESHOLDING:
    default:
        return "unknown thresholding condition";
    }
}

RBRGen3Error RBRGen3_getThresholding(
    RBRGen3 *instrument,
    RBRGen3Thresholding *threshold)
{
    memset(threshold, 0, sizeof(RBRGen3Thresholding));

    RBRGen3GatingState *state =
        (RBRGen3GatingState *) &threshold->state;
    *state = RBRGEN3_UNKNOWN_GATING;

    RBRGen3ThresholdingCondition *condition =
        (RBRGen3ThresholdingCondition *) &threshold->condition;
    *condition = RBRGEN3_UNKNOWN_THRESHOLDING;

    RBR_TRY(RBRGen3_converse(instrument, "thresholding"));

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
        else if (strcmp(parameter.key, "enabled") == 0)
        {
            threshold->enabled = (strcmp(parameter.value, "true") == 0);
        }
        else if (strcmp(parameter.key, "state") == 0)
        {
            if (instrument->generation == RBRGEN3_LOGGER2)
            {
                threshold->enabled = (strcmp(parameter.value, "on") == 0);
            }
            else
            {
                for (int i = RBRGEN3_GATING_NA;
                     i < RBRGEN3_GATING_COUNT;
                     i++)
                {
                    if (strcmp(RBRGen3GatingState_name(i),
                               parameter.value) == 0)
                    {
                        *state = i;
                        break;
                    }
                }
            }
        }
        else if (strcmp(parameter.key, "channelindex") == 0
                 || strcmp(parameter.key, "channel") == 0)
        {
            threshold->channelIndex = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "channellabel") == 0)
        {
            snprintf(threshold->channelLabel,
                     sizeof(threshold->channelLabel),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "condition") == 0)
        {
            for (int i = RBRGEN3_THRESHOLDING_ABOVE;
                 i < RBRGEN3_THRESHOLDING_COUNT;
                 i++)
            {
                if (strcmp(RBRGen3ThresholdingCondition_name(i),
                           parameter.value) == 0)
                {
                    *condition = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "value") == 0)
        {
            threshold->value = strtod(parameter.value, NULL);
        }
        else if (strcmp(parameter.key, "interval") == 0)
        {
            threshold->interval = strtol(parameter.value, NULL, 10);
        }
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_setThresholding(
    RBRGen3 *instrument,
    const RBRGen3Thresholding *threshold)
{
    if (threshold->channelSelection < RBRGEN3_THRESHOLD_CHANNEL_BY_INDEX
        || threshold->channelSelection >
        RBRGEN3_THRESHOLD_CHANNEL_BY_LABEL
        || (threshold->channelSelection ==
            RBRGEN3_THRESHOLD_CHANNEL_BY_INDEX
            && (threshold->channelIndex < 1
                || threshold->channelIndex > RBRGEN3_CHANNEL_MAX))
        || (threshold->channelSelection ==
            RBRGEN3_THRESHOLD_CHANNEL_BY_LABEL
            && (instrument->generation == RBRGEN3_LOGGER2
                || strlen(threshold->channelLabel) == 0))
        || threshold->condition < RBRGEN3_THRESHOLDING_ABOVE
        || threshold->condition > RBRGEN3_THRESHOLDING_BELOW
        || threshold->interval <= 0
        || threshold->interval > RBRGEN3_SAMPLING_PERIOD_MAX
        || (threshold->interval >= 1000 && threshold->interval % 1000 != 0))
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    const char *enabledParameter;
    const char *enabledValue;
    const char *channelParameter;
    char channelValue[RBRGEN3_CHANNEL_LABEL_MAX + 1];

    if (instrument->generation == RBRGEN3_LOGGER2)
    {
        enabledParameter = "state";
        enabledValue = (threshold->enabled) ? "on" : "off";
        channelParameter = "channel";
    }
    else
    {
        enabledParameter = "enabled";
        enabledValue = (threshold->enabled) ? "true" : "false";
        channelParameter = "channelindex";
    }

    if (threshold->channelSelection ==
        RBRGEN3_THRESHOLD_CHANNEL_BY_INDEX)
    {
        snprintf(channelValue,
                 sizeof(channelValue),
                 "%" PRIi32,
                 threshold->channelIndex);
    }
    else
    {
        channelParameter = "channellabel";
        snprintf(channelValue,
                 sizeof(channelValue),
                 "%s",
                 threshold->channelLabel);
    }

    return RBRGen3_converse(
        instrument,
        "thresholding %s = %s, %s = %s, condition = %s, value = %0.4f, "
        "interval = %d",
        enabledParameter,
        enabledValue,
        channelParameter,
        channelValue,
        RBRGen3ThresholdingCondition_name(threshold->condition),
        (double) threshold->value,
        threshold->interval);
}

RBRGen3Error RBRGen3_getTwistActivation(
    RBRGen3 *instrument,
    RBRGen3TwistActivation *twistActivation)
{
    memset(twistActivation, 0, sizeof(RBRGen3TwistActivation));

    RBRGen3GatingState *state =
        (RBRGen3GatingState *) &twistActivation->state;
    *state = RBRGEN3_UNKNOWN_GATING;

    RBR_TRY(RBRGen3_converse(instrument, "twistactivation"));

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
        else if (strcmp(parameter.key, "enabled") == 0)
        {
            twistActivation->enabled = (strcmp(parameter.value, "true") == 0);
        }
        else if (strcmp(parameter.key, "state") == 0)
        {
            if (instrument->generation == RBRGEN3_LOGGER2)
            {
                twistActivation->enabled =
                    (strcmp(parameter.value, "on") == 0);
            }
            else
            {
                for (int i = RBRGEN3_GATING_NA;
                     i < RBRGEN3_GATING_COUNT;
                     i++)
                {
                    if (strcmp(RBRGen3GatingState_name(i),
                               parameter.value) == 0)
                    {
                        *state = i;
                        break;
                    }
                }
            }
        }
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_setTwistActivation(
    RBRGen3 *instrument,
    const RBRGen3TwistActivation *twistActivation)
{
    const char *enabledParameter;
    const char *enabledValue;

    if (instrument->generation == RBRGEN3_LOGGER2)
    {
        enabledParameter = "state";
        enabledValue = (twistActivation->enabled) ? "on" : "off";
    }
    else
    {
        enabledParameter = "enabled";
        enabledValue = (twistActivation->enabled) ? "true" : "false";
    }

    return RBRGen3_converse(
        instrument,
        "twistactivation %s = %s",
        enabledParameter,
        enabledValue);
}
