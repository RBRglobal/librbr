/**
 * \file RBRInstrumentGating.c
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

#include "RBRInstrument.h"
#include "RBRInstrumentInternal.h"

const char *RBRInstrumentGatingState_name(RBRInstrumentGatingState state)
{
    switch (state)
    {
    case RBRINSTRUMENT_GATING_NA:
        return "n/a";
    case RBRINSTRUMENT_GATING_PAUSED:
        return "paused";
    case RBRINSTRUMENT_GATING_RUNNING:
        return "running";
    case RBRINSTRUMENT_GATING_COUNT:
        return "gating state count";
    case RBRINSTRUMENT_UNKNOWN_GATING:
    default:
        return "unknown gating state";
    }
}

const char *RBRInstrumentThresholdingChannelSelection_name(RBRInstrumentThresholdingChannelSelection selection)
{
    switch (selection)
    {
    case RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_INDEX:
        return "index";
    case RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_LABEL:
        return "label";
    default:
        return "unknown thresholding channel selection";
    }
}

const char *RBRInstrumentThresholdingCondition_name(RBRInstrumentThresholdingCondition condition)
{
    switch (condition)
    {
    case RBRINSTRUMENT_THRESHOLDING_ABOVE:
        return "above";
    case RBRINSTRUMENT_THRESHOLDING_BELOW:
        return "below";
    case RBRINSTRUMENT_THRESHOLDING_COUNT:
        return "thresholding condition count";
    case RBRINSTRUMENT_UNKNOWN_THRESHOLDING:
    default:
        return "unknown thresholding condition";
    }
}

RBRInstrumentError RBRInstrument_getThresholding(RBRInstrument *instrument,
                                                 RBRInstrumentThresholding *threshold)
{
    memset(threshold, 0, sizeof(RBRInstrumentThresholding));

    RBRInstrumentGatingState *state = (RBRInstrumentGatingState *) &threshold->state;
    *state = RBRINSTRUMENT_UNKNOWN_GATING;

    RBRInstrumentThresholdingCondition *condition =
        (RBRInstrumentThresholdingCondition *) &threshold->condition;
    *condition = RBRINSTRUMENT_UNKNOWN_THRESHOLDING;

    RBR_TRY(RBRInstrument_converse(instrument, "thresholding"));

    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    while (true)
    {
        RBRInstrument_parseResponse(instrument, &command, &parameter);

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
            if (instrument->generation == RBRINSTRUMENT_LOGGER2)
            {
                threshold->enabled = (strcmp(parameter.value, "on") == 0);
            }
            else
            {
                for (int i = RBRINSTRUMENT_GATING_NA; i < RBRINSTRUMENT_GATING_COUNT; i++)
                {
                    if (strcmp(RBRInstrumentGatingState_name(i), parameter.value) == 0)
                    {
                        *state = i;
                        break;
                    }
                }
            }
        }
        else if (strcmp(parameter.key, "channelindex") == 0 ||
                 strcmp(parameter.key, "channel") == 0)
        {
            threshold->channelIndex = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "channellabel") == 0)
        {
            snprintf(
                threshold->channelLabel, sizeof(threshold->channelLabel), "%s", parameter.value);
        }
        else if (strcmp(parameter.key, "condition") == 0)
        {
            for (int i = RBRINSTRUMENT_THRESHOLDING_ABOVE; i < RBRINSTRUMENT_THRESHOLDING_COUNT;
                 i++)
            {
                if (strcmp(RBRInstrumentThresholdingCondition_name(i), parameter.value) == 0)
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

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_setThresholding(RBRInstrument *instrument,
                                                 const RBRInstrumentThresholding *threshold)
{
    if (threshold->channelSelection < RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_INDEX ||
        threshold->channelSelection > RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_LABEL ||
        (threshold->channelSelection == RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_INDEX &&
         (threshold->channelIndex < 1 || threshold->channelIndex > RBRINSTRUMENT_CHANNEL_MAX)) ||
        (threshold->channelSelection == RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_LABEL &&
         (instrument->generation == RBRINSTRUMENT_LOGGER2 ||
          strlen(threshold->channelLabel) == 0)) ||
        threshold->condition < RBRINSTRUMENT_THRESHOLDING_ABOVE ||
        threshold->condition > RBRINSTRUMENT_THRESHOLDING_BELOW || threshold->interval <= 0 ||
        threshold->interval > RBRINSTRUMENT_SAMPLING_PERIOD_MAX ||
        (threshold->interval >= 1000 && threshold->interval % 1000 != 0))
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    const char *enabledParameter;
    const char *enabledValue;
    const char *channelParameter;
    char channelValue[RBRINSTRUMENT_CHANNEL_LABEL_MAX + 1];

    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
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

    if (threshold->channelSelection == RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_INDEX)
    {
        snprintf(channelValue, sizeof(channelValue), "%" PRIi32, threshold->channelIndex);
    }
    else
    {
        channelParameter = "channellabel";
        snprintf(channelValue, sizeof(channelValue), "%s", threshold->channelLabel);
    }

    return RBRInstrument_converse(instrument,
                                  "thresholding %s = %s, %s = %s, condition = %s, value = %0.4f, "
                                  "interval = %d",
                                  enabledParameter,
                                  enabledValue,
                                  channelParameter,
                                  channelValue,
                                  RBRInstrumentThresholdingCondition_name(threshold->condition),
                                  (double) threshold->value,
                                  threshold->interval);
}

RBRInstrumentError RBRInstrument_getTwistActivation(RBRInstrument *instrument,
                                                    RBRInstrumentTwistActivation *twistActivation)
{
    memset(twistActivation, 0, sizeof(RBRInstrumentTwistActivation));

    RBRInstrumentGatingState *state = (RBRInstrumentGatingState *) &twistActivation->state;
    *state = RBRINSTRUMENT_UNKNOWN_GATING;

    RBR_TRY(RBRInstrument_converse(instrument, "twistactivation"));

    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    while (true)
    {
        RBRInstrument_parseResponse(instrument, &command, &parameter);

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
            if (instrument->generation == RBRINSTRUMENT_LOGGER2)
            {
                twistActivation->enabled = (strcmp(parameter.value, "on") == 0);
            }
            else
            {
                for (int i = RBRINSTRUMENT_GATING_NA; i < RBRINSTRUMENT_GATING_COUNT; i++)
                {
                    if (strcmp(RBRInstrumentGatingState_name(i), parameter.value) == 0)
                    {
                        *state = i;
                        break;
                    }
                }
            }
        }
    }

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_setTwistActivation(RBRInstrument *instrument,
                                                    const RBRInstrumentTwistActivation *twistActivation)
{
    const char *enabledParameter;
    const char *enabledValue;

    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        enabledParameter = "state";
        enabledValue = (twistActivation->enabled) ? "on" : "off";
    }
    else
    {
        enabledParameter = "enabled";
        enabledValue = (twistActivation->enabled) ? "true" : "false";
    }

    return RBRInstrument_converse(
        instrument, "twistactivation %s = %s", enabledParameter, enabledValue);
}
