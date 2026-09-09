/**
 * \file RBRInstrumentGen3Streaming.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for isnan, NAN. */
#include <math.h>
/* Required for strchr, strcmp. */
#include <string.h>
/* Required for snprintf. */
#include <stdio.h>

#include "RBRGen3.h"
#include "RBRGen3Internal.h"

#define READING_FLAG_MASK     0x00FF0000
#define READING_FLAG_OFFSET  (2 * 8)
#define READING_ERROR_MASK    0x0000FFFF
#define READING_ERROR_OFFSET (0 * 8)

RBRGen3Error RBRInstrumentGen3_getChannelsList(
    RBRGen3 *instrument,
    RBRInstrumentGen3ChannelsList *channelsList)
{
    if (instrument->generation == RBRGEN3_LOGGER2)
    {
        return RBRGEN3_UNSUPPORTED;
    }

    memset(channelsList, 0, sizeof(RBRInstrumentGen3ChannelsList));

    RBR_TRY(RBRGen3_converse(instrument, "outputformat channelslist"));

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
        else if (strcmp(parameter.key, "channelslist") != 0)
        {
            continue;
        }

        char *nameStart;
        char *unitStart;
        int32_t channel;
        char *next = strtok(parameter.value, "(");
        for (channel = 0;; channel++)
        {
            nameStart = next != NULL && next[0] == '|' ? next + 1 : next;
            unitStart = strtok(NULL, ")");

            if (nameStart == NULL || unitStart == NULL)
            {
                break;
            }

            snprintf(channelsList->channels[channel].name,
                     sizeof(channelsList->channels[channel].name),
                     "%s",
                     nameStart);

            snprintf(channelsList->channels[channel].unit,
                     sizeof(channelsList->channels[channel].unit),
                     "%s",
                     unitStart);

            next = strtok(NULL, "(");
        }
        channelsList->count = channel;

        break;
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRInstrumentGen3_getLabelsList(
    RBRGen3 *instrument,
    RBRInstrumentGen3LabelsList *labelsList)
{
    if (instrument->generation == RBRGEN3_LOGGER2)
    {
        return RBRGEN3_UNSUPPORTED;
    }

    memset(labelsList, 0, sizeof(RBRInstrumentGen3LabelsList));

    RBR_TRY(RBRGen3_converse(instrument, "outputformat labelslist"));

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
        else if (strcmp(parameter.key, "labelslist") != 0)
        {
            continue;
        }

        int32_t label;
        char *labelStart = strtok(parameter.value, "|");
        for (label = 0; labelStart != NULL; label++)
        {
            snprintf(labelsList->labels[label],
                     sizeof(labelsList->labels[label]),
                     "%s",
                     labelStart);

            labelStart = strtok(NULL, "|");
        }
        labelsList->count = label;

        break;
    }

    return RBRGEN3_SUCCESS;
}

const char *RBRInstrumentGen3OutputFormat_name(RBRInstrumentGen3OutputFormat format)
{
    switch (format)
    {
    case RBRINSTRUMENTGEN3_OUTFORMAT_NONE:
        return "none";
    case RBRINSTRUMENTGEN3_OUTFORMAT_CALTEXT01:
        return "caltext01";
    case RBRINSTRUMENTGEN3_OUTFORMAT_CALTEXT02:
        return "caltext02";
    case RBRINSTRUMENTGEN3_OUTFORMAT_CALTEXT03:
        return "caltext03";
    case RBRINSTRUMENTGEN3_OUTFORMAT_CALTEXT04:
        return "caltext04";
    case RBRINSTRUMENTGEN3_OUTFORMAT_CALTEXT07:
        return "caltext07";
    default:
        return "unknown output format";
    }
}

RBRGen3Error RBRInstrumentGen3_getAvailableOutputFormats(
    RBRGen3 *instrument,
    RBRInstrumentGen3OutputFormat *outputFormats)
{
    *outputFormats = RBRINSTRUMENTGEN3_OUTFORMAT_NONE;

    const char *command;
    const char *searchKey;
    const char *separator;
    int32_t separatorLength;

    if (instrument->generation == RBRGEN3_LOGGER2)
    {
        command = "outputformat support";
        searchKey = "support";
        separator = ", ";
        separatorLength = 2;
    }
    else
    {
        command = "outputformat availabletypes";
        searchKey = "availabletypes";
        separator = "|";
        separatorLength = 1;
    }

    RBR_TRY(RBRGen3_converse(instrument, command));

    char *responseCommand = NULL;
    RBRGen3ResponseParameter parameter;
    while (true)
    {
        RBRGen3_parseResponse(instrument,
                                    &responseCommand,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, searchKey) != 0)
        {
            continue;
        }

        char *nextValue;
        do
        {
            if ((nextValue = strstr(parameter.value, separator)) != NULL)
            {
                *nextValue = '\0';
                nextValue += separatorLength;
            }

            for (int i = RBRINSTRUMENTGEN3_OUTFORMAT_NONE + 1;
                 i <= RBRINSTRUMENTGEN3_OUTFORMAT_MAX;
                 i <<= 1)
            {
                if (strcmp(RBRInstrumentGen3OutputFormat_name(i),
                           parameter.value) == 0)
                {
                    *outputFormats |= i;
                }
            }

            parameter.value = nextValue;
        } while (nextValue != NULL);

        break;
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRInstrumentGen3_getOutputFormat(
    RBRGen3 *instrument,
    RBRInstrumentGen3OutputFormat *outputFormat)
{
    *outputFormat = RBRINSTRUMENTGEN3_OUTFORMAT_NONE;

    RBR_TRY(RBRGen3_converse(instrument, "outputformat type"));

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
        else if (strcmp(parameter.key, "type") != 0)
        {
            continue;
        }

        for (int i = RBRINSTRUMENTGEN3_OUTFORMAT_NONE + 1;
             i <= RBRGEN3_MEMFORMAT_MAX;
             i <<= 1)
        {
            if (strcmp(RBRInstrumentGen3OutputFormat_name(i),
                       parameter.value) == 0)
            {
                *outputFormat = i;
                break;
            }
        }

        break;
    }

    return RBRGEN3_SUCCESS;
}


RBRGen3Error RBRInstrumentGen3_setOutputFormat(
    RBRGen3 *instrument,
    RBRInstrumentGen3OutputFormat outputFormat)
{
    const char *formatName = RBRInstrumentGen3OutputFormat_name(outputFormat);
    
    /* if it's caltext07, it is only available for LOGGER3 with fw 1.109 or later. */
    if (strcmp(formatName, "caltext07") == 0){
        RBRGen3Error err = RBRGen3_getId(instrument, &instrument->id);
        if (err != RBRGEN3_SUCCESS)
        {
            return RBRGEN3_UNSUPPORTED;
        }
        else{
            if (instrument->id.fwtype == 104
                && (atof)(instrument-> id.version) >= 1.109)
            {
                return RBRGen3_converse(instrument,
                                    "outputformat type = %s",
                                    formatName);
            }
            else {
                /* caltext07 is not supported by the firmware version in use. */
                return RBRGEN3_UNSUPPORTED;
            }
        }
    }
    return RBRGen3_converse(instrument,
                                  "outputformat type = %s",
                                  formatName);
}

RBRGen3Error RBRInstrumentGen3_getUSBStreamingState(
    RBRGen3 *instrument,
    bool *enabled)
{
    *enabled = false;
    return RBRGen3_getBool(instrument,
                                 "streamusb",
                                 "state",
                                 enabled);
}

RBRGen3Error RBRInstrumentGen3_setUSBStreamingState(
    RBRGen3 *instrument,
    bool enabled)
{
    return RBRGen3_converse(instrument,
                                  "streamusb state = %s",
                                  enabled ? "on" : "off");
}

RBRGen3Error RBRInstrumentGen3_getSerialStreamingState(
    RBRGen3 *instrument,
    bool *enabled)
{
    *enabled = false;
    return RBRGen3_getBool(instrument,
                                 "streamserial",
                                 "state",
                                 enabled);
}

RBRGen3Error RBRInstrumentGen3_setSerialStreamingState(
    RBRGen3 *instrument,
    bool enabled)
{
    return RBRGen3_converse(instrument,
                                  "streamserial state = %s",
                                  enabled ? "on" : "off");
}

const char *RBRInstrumentGen3AuxOutputActiveLevel_name(
    RBRInstrumentGen3AuxOutputActiveLevel level)
{
    switch (level)
    {
    case RBRINSTRUMENTGEN3_ACTIVE_HIGH:
        return "high";
    case RBRINSTRUMENTGEN3_ACTIVE_LOW:
        return "low";
    case RBRINSTRUMENTGEN3_ACTIVE_COUNT:
        return "active output level count";
    case RBRINSTRUMENTGEN3_UNKNOWN_ACTIVE:
    default:
        return "unknown active output level";
    }
}

const char *RBRInstrumentGen3AuxOutputSleepLevel_name(
    RBRInstrumentGen3AuxOutputSleepLevel level)
{
    switch (level)
    {
    case RBRINSTRUMENTGEN3_SLEEP_TRISTATE:
        return "tristate";
    case RBRINSTRUMENTGEN3_SLEEP_HIGH:
        return "high";
    case RBRINSTRUMENTGEN3_SLEEP_LOW:
        return "low";
    case RBRINSTRUMENTGEN3_SLEEP_COUNT:
        return "sleep output level count";
    case RBRINSTRUMENTGEN3_UNKNOWN_SLEEP:
    default:
        return "unknown sleep level";
    }
}

RBRGen3Error RBRInstrumentGen3_getAuxOutput(
    RBRGen3 *instrument,
    RBRInstrumentGen3AuxOutput *auxOutput)
{
    if (auxOutput->aux != 1)
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    uint8_t aux = auxOutput->aux;
    memset(auxOutput, 0, sizeof(RBRInstrumentGen3AuxOutput));
    auxOutput->active = RBRINSTRUMENTGEN3_UNKNOWN_ACTIVE;
    auxOutput->sleep = RBRINSTRUMENTGEN3_UNKNOWN_SLEEP;

    RBR_TRY(RBRGen3_converse(instrument,
                                   "streamserial aux%" PRIi8 "_all",
                                   aux));

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
        else if (strcmp(parameter.key, "aux1_state") == 0)
        {
            auxOutput->enabled = (strcmp(parameter.value, "on") == 0);
        }
        else if (strcmp(parameter.key, "aux1_enabled") == 0)
        {
            auxOutput->enabled = (strcmp(parameter.value, "true") == 0);
        }
        else if (strcmp(parameter.key, "aux1_setup") == 0)
        {
            auxOutput->setup = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "aux1_hold") == 0)
        {
            auxOutput->hold = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "aux1_active") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENTGEN3_ACTIVE_COUNT; i++)
            {
                if (strcmp(RBRInstrumentGen3AuxOutputActiveLevel_name(i),
                           parameter.value) == 0)
                {
                    auxOutput->active = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "aux1_sleep") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENTGEN3_SLEEP_COUNT; i++)
            {
                if (strcmp(RBRInstrumentGen3AuxOutputSleepLevel_name(i),
                           parameter.value) == 0)
                {
                    auxOutput->sleep = i;
                    break;
                }
            }
        }
    }
    auxOutput->aux = aux;

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRInstrumentGen3_setAuxOutput(
    RBRGen3 *instrument,
    const RBRInstrumentGen3AuxOutput *auxOutput)
{
    const char *enabledParameter;
    const char *enabledValue;
    if (instrument->generation == RBRGEN3_LOGGER2)
    {
        enabledParameter = "state";
        enabledValue = auxOutput->enabled ? "on" : "off";
    }
    else
    {
        enabledParameter = "enabled";
        enabledValue = auxOutput->enabled ? "true" : "false";
    }

    return RBRGen3_converse(
        instrument,
        "streamserial aux%" PRIi8 "_%s = %s, aux%" PRIi8 "_setup = %" PRIi32 ", "
        "aux%" PRIi8 "_hold = %" PRIi32 ", aux%" PRIi8 "_active = %s, "
        "aux%" PRIi8 "_sleep = %s",
        auxOutput->aux,
        enabledParameter,
        enabledValue,
        auxOutput->aux,
        auxOutput->setup,
        auxOutput->aux,
        auxOutput->hold,
        auxOutput->aux,
        RBRInstrumentGen3AuxOutputActiveLevel_name(auxOutput->active),
        auxOutput->aux,
        RBRInstrumentGen3AuxOutputSleepLevel_name(auxOutput->sleep));
}

const char *RBRInstrumentGen3ReadingFlag_name(RBRInstrumentGen3ReadingFlag flag)
{
    switch (flag)
    {
    case RBRINSTRUMENTGEN3_READING_FLAG_NONE:
        return "none";
    case RBRGEN3_READING_FLAG_UNCALIBRATED:
        return "uncalibrated";
    case RBRGEN3_READING_FLAG_ERROR:
        return "error";
    case RBRINSTRUMENTGEN3_READING_FLAG_COUNT:
        return "reading flag count";
    case RBRINSTRUMENTGEN3_UNKNOWN_READING_FLAG:
    default:
        return "unknown reading flag";
    }
}

inline RBRInstrumentGen3ReadingFlag RBRInstrumentGen3Reading_getFlag(double reading)
{
    if (!isnan(reading))
    {
        return RBRINSTRUMENTGEN3_READING_FLAG_NONE;
    }

    union
    {
        double reading;
        uint64_t raw;
    }
    alias;
    alias.reading = reading;

    return (alias.raw & READING_FLAG_MASK) >> READING_FLAG_OFFSET;
}

inline uint8_t RBRInstrumentGen3Reading_getError(double reading)
{
    if (!isnan(reading))
    {
        return 0;
    }

    union
    {
        double reading;
        uint64_t raw;
    }
    alias;
    alias.reading = reading;

    return (alias.raw & READING_ERROR_MASK) >> READING_ERROR_OFFSET;
}

inline double RBRInstrumentGen3Reading_setError(RBRInstrumentGen3ReadingFlag flag,
                                            uint8_t error)
{
    union
    {
        double reading;
        uint64_t raw;
    }
    alias;
    alias.reading = nan("");

    alias.raw |= ((flag << READING_FLAG_OFFSET) & READING_FLAG_MASK)
                 | ((error << READING_ERROR_OFFSET) & READING_ERROR_MASK);

    return alias.reading;
}

RBRGen3Error RBRInstrumentGen3_readSample(RBRGen3 *instrument)
{
    RBRGen3Error err;
    /* RBRGen3_readResponse() returns #RBRGEN3_SAMPLE when a sample
     * is read to the given sample pointer; a return of #RBRGEN3_SUCCESS
     * means that it found some other command response instead, so we'll loop
     * until we get a “failure” value (which we hope is SAMPLE). */
    do
    {
        err = RBRGen3_readResponse(instrument, true, NULL);
    } while (err == RBRGEN3_SUCCESS);
    /* SAMPLE is what we were hoping for, so we'll translate to SUCCESS. Any
     * other errors can really be errors. */
    if (err == RBRGEN3_SAMPLE)
    {
        err = RBRGEN3_SUCCESS;
    }

    return err;
}
