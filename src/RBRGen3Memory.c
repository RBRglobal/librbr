/**
 * \file RBRGen3Memory.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for memcpy, memset, strcmp. */
#include <string.h>
/* Required for sscanf. */
#include <stdio.h>
/* Required for strtol. */
#include <stdlib.h>

#include "RBRGen3.h"
#include "RBRGen3Internal.h"

const char *RBRGen3Dataset_name(RBRGen3Dataset dataset)
{
    switch (dataset)
    {
    case RBRGEN3_DATASET_EASYPARSE_EVENTS:
        return "EasyParse events";
    case RBRGEN3_DATASET_STANDARD:
        return "standard or EasyParse data";
    case RBRGEN3_DATASET_EASYPARSE_DEPLOYMENT_HEADER:
        return "EasyParse deployment header";
    case RBRGEN3_DATASET_POSTPROCESSING_SAMPLE_DATA:
        return "post-processing sample data";
    case RBRGEN3_DATASET_UNKNOWN_DATASET1:
    case RBRGEN3_UNKNOWN_DATASET:
    default:
        return "unknown dataset";
    }
}

RBRGen3Error RBRGen3_getMemoryInfo(
    RBRGen3 *instrument,
    RBRGen3MemoryInfo *memoryInfo)
{
    if (memoryInfo->dataset < RBRGEN3_DATASET_EASYPARSE_EVENTS
        || memoryInfo->dataset == RBRGEN3_DATASET_UNKNOWN_DATASET1
        || memoryInfo->dataset >= RBRGEN3_DATASET_COUNT)
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    RBRGen3Dataset dataset = memoryInfo->dataset;
    memset(memoryInfo, 0, sizeof(RBRGen3MemoryInfo));

    RBR_TRY(RBRGen3_converse(instrument,
                                   "meminfo dataset = %d",
                                   dataset));

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
        else if (strcmp(parameter.key, "dataset") == 0)
        {
            memoryInfo->dataset = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "used") == 0)
        {
            memoryInfo->used = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "remaining") == 0)
        {
            memoryInfo->remaining = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "size") == 0)
        {
            memoryInfo->size = strtol(parameter.value, NULL, 10);
        }
    }

    return RBRGEN3_SUCCESS;
}

static RBRGen3Error RBRGen3L2_parseDataResponse(
    RBRGen3 *instrument,
    RBRGen3Data *data)
{
    sscanf(instrument->response.response,
           "data %d %d %d",
           (int *) &data->dataset,
           &data->size,
           &data->offset);
    return RBRGEN3_SUCCESS;
}

static RBRGen3Error RBRGen3L3_parseDataResponse(
    RBRGen3 *instrument,
    RBRGen3Data *data)
{
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
        else if (strcmp(parameter.key, "dataset") == 0)
        {
            data->dataset = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "size") == 0)
        {
            data->size = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "offset") == 0)
        {
            data->offset = strtol(parameter.value, NULL, 10);
        }
    }

    return RBRGEN3_SUCCESS;
}

/**
 * \brief Keep retrying reads until we retrieve a fixed amount of data.
 *
 * This function first drains data out of RBRGen3.responseBuffer, then
 * begins to read from the instrument. As a result, \a data must not be
 * RBRGen3.responseBuffer!
 *
 * \param [in] instrument the instrument connection
 * \param [out] data the buffer to write into
 * \param [in] size the amount of data to write into the buffer
 */
static RBRGen3Error RBRGen3_fixedRead(
    struct RBRGen3 *instrument,
    void *data,
    int32_t size)
{
    int32_t bufferLength = 0;
    int32_t readLength;

    /* Can we steal from the response buffer? */
    if (instrument->lastResponseLength < instrument->responseBufferLength)
    {
        readLength = instrument->responseBufferLength
                     - instrument->lastResponseLength;
        if (readLength > size)
        {
            readLength = size;
        }

        memcpy(data,
               ((uint8_t *) instrument->responseBuffer)
               + instrument->lastResponseLength,
               readLength);

        bufferLength = readLength;
        instrument->lastResponseLength += readLength;
    }

    /* Now poll the instrument. */
    while (bufferLength < size)
    {
        readLength = size - bufferLength;

        RBR_TRY(instrument->callbacks.read(
                    instrument,
                    ((uint8_t *) data) + bufferLength,
                    &readLength));

        bufferLength += readLength;
    }

    return RBRGEN3_SUCCESS;
}

/* CRC-CCITT */
uint16_t calculateCrc(const void *data, int32_t size)
{
#define CRC_POLYNOMIAL 0x1021

    uint16_t crc = 0xFFFF;

    for (int32_t i = 0; i < size; i++)
    {
        uint8_t b = ((uint8_t *) data)[i];

        for (int_fast8_t j = 0; j < 8; j++)
        {
            uint8_t bit = ((b   >> (7 - j) & 1) == 1);
            uint8_t c15 = ((crc >> 15      & 1) == 1);

            crc <<= 1;

            if (c15 ^ bit)
            {
                crc ^= CRC_POLYNOMIAL;
            }
        }
    }

    return crc;
}

RBRGen3Error RBRGen3_readData(RBRGen3 *instrument,
                                          RBRGen3Data *data)
{
    if (data->dataset < RBRGEN3_DATASET_EASYPARSE_EVENTS
        || data->dataset == RBRGEN3_DATASET_UNKNOWN_DATASET1
        || data->dataset >= RBRGEN3_DATASET_COUNT)
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    RBRGen3Data workingData;
    memcpy(&workingData, data, sizeof(RBRGen3Data));
    data->size = 0;

    /*
     * The `read` command became `readdata` between L2 and L3, and the
     * parameter format changed a little too.
     *
     * The L2 command:
     *
     *     >> read data <dataset> <size> <offset>
     *     << data <dataset> <size> <offset>
     *
     * The L3 command:
     *
     *     >> readdata dataset = <dataset>, size = <size>, offset = <offset>
     *     << readdata dataset = <dataset>, size = <size>, offset = <offset>
     */
    const char *generationCommand;
    if (instrument->generation == RBRGEN3_LOGGER2)
    {
        generationCommand = "read data %d %" PRId32 " %" PRId32;
    }
    else
    {
        generationCommand = "readdata dataset = %d"
                            ", size = %" PRId32
                            ", offset = %" PRId32;
    }

    RBR_TRY(RBRGen3_converse(instrument,
                                   generationCommand,
                                   workingData.dataset,
                                   workingData.size,
                                   workingData.offset));

    /* Because the response format for L2 is so nonstandard, we'll have to
     * parse it with sscanf. We can just do things the normal way for L3. */
    if (instrument->generation == RBRGEN3_LOGGER2)
    {
        RBR_TRY(RBRGen3L2_parseDataResponse(instrument, &workingData));
    }
    else
    {
        RBR_TRY(RBRGen3L3_parseDataResponse(instrument, &workingData));
    }

    /* check if offset in response matches requested offset. If not, return error. */
    if(workingData.offset !=data->offset){
        return RBRGEN3_COMMUNICATION_ERROR;
    }

    /* Fill the user-provided buffer. RBRGen3_fixedRead() will first pull
     * leftover data from RBRGen3.responseBuffer, then read from the
     * instrument. */
    RBR_TRY(RBRGen3_fixedRead(instrument, data->data, workingData.size));

    /* CRC check the last two bytes. */
    union
    {
        uint8_t buf[2];
        uint16_t value;
    }
    crc;

    RBR_TRY(RBRGen3_fixedRead(instrument, crc.buf, 2));
    /* The logger reports the CRC as big-endian. Under the assumption that the
     * host is little-endian, we'll byte swap it before using it for
     * comparison. ntohs() is POSIX but not part of the C standard, and we want
     * to target pure C99, so we can't use it here. */

    crc.value = (crc.value >> 8) | (crc.value << 8);

    uint16_t calculatedCrc = calculateCrc(data->data, workingData.size);
    if (calculatedCrc != crc.value)
    {
        return RBRGEN3_CHECKSUM_ERROR;
    }

    memcpy(data, &workingData, sizeof(RBRGen3Data));

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_memoryClear(RBRGen3 *instrument)
{
    RBR_TRY(RBRInstrumentGen3_permit(instrument, "memclear"));
    RBR_TRY(RBRGen3_converse(instrument, "memclear"));
    return RBRGEN3_SUCCESS;
}

const char *RBRGen3MemoryFormat_name(RBRGen3MemoryFormat format)
{
    switch (format)
    {
    case RBRGEN3_MEMFORMAT_NONE:
        return "none";
    case RBRGEN3_MEMFORMAT_RAWBIN00:
        return "rawbin00";
    case RBRGEN3_MEMFORMAT_CALBIN00:
        return "calbin00";
    default:
        return "unknown memory format";
    }
}

RBRGen3Error RBRGen3_getAvailableMemoryFormats(
    RBRGen3 *instrument,
    RBRGen3MemoryFormat *memoryFormats)
{
    *memoryFormats = RBRGEN3_MEMFORMAT_NONE;

    /*
     * The subcommand of `memformat` to retrieve available types changed
     * between L2 and L3, as did the format of the response.
     *
     * The L2 command:
     *
     *     >> memformat support
     *     << memformat support = rawbin00, calbin00
     *
     * The L3 command:
     *
     *     >> memformat availabletypes
     *     << memformat availabletypes = rawbin00|calbin00
     */
    const char *generationCommand;
    const char *separator;
    int32_t separatorLen;
    if (instrument->generation == RBRGEN3_LOGGER2)
    {
        generationCommand = "memformat support";
        separator = ", ";
        separatorLen = 2;
    }
    else
    {
        generationCommand = "memformat availabletypes";
        separator = "|";
        separatorLen = 1;
    }
    RBR_TRY(RBRGen3_converse(instrument, generationCommand));

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
        else if (strcmp(parameter.key, "availabletypes") != 0
                 && strcmp(parameter.key, "support") != 0)
        {
            continue;
        }

        char *nextValue;
        do
        {
            if ((nextValue = strstr(parameter.value, separator)) != NULL)
            {
                *nextValue = '\0';
                nextValue += separatorLen;
            }

            for (int i = RBRGEN3_MEMFORMAT_NONE + 1;
                 i <= RBRGEN3_MEMFORMAT_MAX;
                 i <<= 1)
            {
                if (strcmp(RBRGen3MemoryFormat_name(i),
                           parameter.value) == 0)
                {
                    *memoryFormats |= i;
                }
            }

            parameter.value = nextValue;
        } while (nextValue != NULL);

        break;
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_getCurrentMemoryFormat(
    RBRGen3 *instrument,
    RBRGen3MemoryFormat *memoryFormat)
{
    *memoryFormat = RBRGEN3_MEMFORMAT_NONE;

    RBR_TRY(RBRGen3_converse(instrument, "memformat type"));

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

        for (int i = RBRGEN3_MEMFORMAT_NONE + 1;
             i <= RBRGEN3_MEMFORMAT_MAX;
             i <<= 1)
        {
            if (strcmp(RBRGen3MemoryFormat_name(i),
                       parameter.value) == 0)
            {
                *memoryFormat = i;
                break;
            }
        }

        break;
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_getNewMemoryFormat(
    RBRGen3 *instrument,
    RBRGen3MemoryFormat *memoryFormat)
{
    *memoryFormat = RBRGEN3_MEMFORMAT_NONE;

    RBR_TRY(RBRGen3_converse(instrument, "memformat newtype"));

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
        else if (strcmp(parameter.key, "newtype") != 0)
        {
            continue;
        }

        for (int i = RBRGEN3_MEMFORMAT_RAWBIN00 + 1;
             i <= RBRGEN3_MEMFORMAT_MAX;
             i <<= 1)
        {
            if (strcmp(RBRGen3MemoryFormat_name(i),
                       parameter.value) == 0)
            {
                *memoryFormat = i;
                break;
            }
        }

        break;
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_setNewMemoryFormat(
    RBRGen3 *instrument,
    RBRGen3MemoryFormat memoryFormat)
{
    if (memoryFormat < RBRGEN3_MEMFORMAT_NONE
        || memoryFormat > RBRGEN3_MEMFORMAT_MAX)
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    const char *formatName = RBRGen3MemoryFormat_name(memoryFormat);
    return RBRGen3_converse(instrument,
                                  "memformat newtype = %s",
                                  formatName);
}

const char *RBRGen3PostprocessingAggregate_name(
    RBRGen3PostprocessingAggregate function)
{
    switch (function)
    {
    case RBRGEN3_POSTPROCESSING_AGGREGATE_MEAN:
        return "mean";
    case RBRGEN3_POSTPROCESSING_AGGREGATE_STD:
        return "std";
    case RBRGEN3_POSTPROCESSING_AGGREGATE_SAMPLE_COUNT:
        return "count";
    case RBRGEN3_POSTPROCESSING_AGGREGATE_COUNT:
        return "post-processing aggregate function count";
    case RBRGEN3_UNKNOWN_POSTPROCESSING_AGGREGATE:
    default:
        return "unknown post-processing aggregate function";
    }
}

const char *RBRGen3PostprocessingStatus_name(
    RBRGen3PostprocessingStatus status)
{
    switch (status)
    {
    case RBRGEN3_POSTPROCESSING_STATUS_IDLE:
        return "idle";
    case RBRGEN3_POSTPROCESSING_STATUS_PROCESSING:
        return "processing";
    case RBRGEN3_POSTPROCESSING_STATUS_COMPLETED:
        return "completed";
    case RBRGEN3_POSTPROCESSING_STATUS_ABORTED:
        return "aborted";
    case RBRGEN3_POSTPROCESSING_STATUS_COUNT:
        return "post-processing status count";
    case RBRGEN3_UNKNOWN_POSTPROCESSING_STATUS:
    default:
        return "unknown post-processing status";
    }
}

const char *RBRGen3PostprocessingCommand_name(
    RBRGen3PostprocessingCommand command)
{
    switch (command)
    {
    case RBRGEN3_POSTPROCESSING_COMMAND_START:
        return "start";
    case RBRGEN3_POSTPROCESSING_COMMAND_RESET:
        return "reset";
    case RBRGEN3_POSTPROCESSING_COMMAND_ABORT:
        return "abort";
    case RBRGEN3_POSTPROCESSING_COMMAND_COUNT:
        return "post-processing command count";
    case RBRGEN3_UNKNOWN_POSTPROCESSING_COMMAND:
    default:
        return "unknown post-processing command";
    }
}

const char *RBRGen3PostprocessingBinFilter_name(
    RBRGen3PostprocessingBinFilter filter)
{
    switch (filter)
    {
    case RBRGEN3_POSTPROCESSING_BINFILTER_NONE:
        return "none";
    case RBRGEN3_POSTPROCESSING_BINFILTER_ASCENTONLY:
        return "ascentonly";
    case RBRGEN3_POSTPROCESSING_BINFILTER_DESCENTONLY:
        return "descentonly";
    case RBRGEN3_POSTPROCESSING_BINFILTER_COUNT:
        return "post-processing bin filter count";
    case RBRGEN3_UNKNOWN_POSTPROCESSING_BINFILTER:
    default:
        return "unknown post-processing bin filter";
    }
}

RBRGen3Error RBRGen3_getPostprocessing(
    RBRGen3 *instrument,
    RBRGen3Postprocessing *postprocessing)
{
    memset(postprocessing, 0, sizeof(RBRGen3Postprocessing));
    postprocessing->status = RBRGEN3_UNKNOWN_POSTPROCESSING_STATUS;
    postprocessing->binFilter = RBRGEN3_UNKNOWN_POSTPROCESSING_BINFILTER;

    RBR_TRY(RBRGen3_converse(instrument, "postprocessing all"));

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true)
    {
        RBRGen3_parseResponse(instrument, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "status") == 0)
        {
            for (int i = 0; i < RBRGEN3_POSTPROCESSING_STATUS_COUNT; ++i)
            {
                if (strcmp(RBRGen3PostprocessingStatus_name(i),
                           parameter.value) == 0)
                {
                    postprocessing->status = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "channels") == 0)
        {
            RBRGen3PostprocessingChannelsList *channelsList =
                &postprocessing->channels;

            char *functionStart;
            char *labelStart;
            int32_t channel;
            char *next = strtok(parameter.value, "(");
            for (channel = 0;; channel++)
            {
                functionStart = next;
                if (next != NULL && next[0] == '|')
                {
                    ++functionStart;
                }
                labelStart = strtok(NULL, ")");

                if (functionStart == NULL || labelStart == NULL)
                {
                    break;
                }

                for (int i = 0;
                     i < RBRGEN3_POSTPROCESSING_AGGREGATE_COUNT;
                     ++i)
                {
                    if (strcmp(RBRGen3PostprocessingAggregate_name(i),
                               functionStart) == 0)
                    {
                        channelsList->channels[channel].function = i;
                        break;
                    }
                }

                snprintf(channelsList->channels[channel].label,
                         sizeof(channelsList->channels[channel].label),
                         "%s",
                         labelStart);

                next = strtok(NULL, "(");
            }
            channelsList->count = channel;
        }
        else if (strcmp(parameter.key, "binreference") == 0)
        {
            snprintf(postprocessing->binReference,
                     sizeof(postprocessing->binReference),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "binfilter") == 0)
        {
            for (int i = 0;
                 i < RBRGEN3_POSTPROCESSING_BINFILTER_COUNT;
                 ++i)
            {
                if (strcmp(RBRGen3PostprocessingBinFilter_name(i),
                           parameter.value) == 0)
                {
                    postprocessing->binFilter = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "binsize") == 0)
        {
            postprocessing->binSize = strtod(parameter.value, NULL);
        }
        else if (strcmp(parameter.key, "tstamp_min") == 0)
        {
            RBRGen3DateTime_parseScheduleTime(parameter.value,
                                                    &postprocessing->tstampMin,
                                                    NULL);
        }
        else if (strcmp(parameter.key, "tstamp_max") == 0)
        {
            RBRGen3DateTime_parseScheduleTime(parameter.value,
                                                    &postprocessing->tstampMax,
                                                    NULL);
        }
        else if (strcmp(parameter.key, "depth_min") == 0)
        {
            postprocessing->depthMin = strtod(parameter.value, NULL);
        }
        else if (strcmp(parameter.key, "depth_max") == 0)
        {
            postprocessing->depthMax = strtod(parameter.value, NULL);
        }
        else if (strcmp(parameter.key, "dc_alpha") == 0)
        {
            postprocessing->dcAlpha = strtod(parameter.value, NULL);
        }
        else if (strcmp(parameter.key, "dc_tau") == 0)
        {
            postprocessing->dcTau = strtod(parameter.value, NULL);
        }
        else if (strcmp(parameter.key, "dc_tdelay") == 0)
        {
            postprocessing->dcTdelay = strtod(parameter.value, NULL);
        }
        else if (strcmp(parameter.key, "dc_ctcoeff") == 0)
        {
            postprocessing->dcCtCoeff = strtod(parameter.value, NULL);
        }
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_setPostprocessing(
    RBRGen3 *instrument,
    const RBRGen3Postprocessing *postprocessing)
{
    bool timeBinning = strcmp(postprocessing->binReference, "tstamp") == 0;

    if (postprocessing->channels.count < 0
        || postprocessing->channels.count >=
        RBRGEN3_POSTPROCESSING_CHANNEL_MAX
        || postprocessing->binFilter <
        RBRGEN3_POSTPROCESSING_BINFILTER_NONE
        || postprocessing->binFilter >=
        RBRGEN3_POSTPROCESSING_BINFILTER_COUNT
        || postprocessing->binSize < 0
        || postprocessing->tstampMin < RBRGEN3_DATETIME_MIN
        || postprocessing->tstampMin > RBRGEN3_DATETIME_MAX
        || postprocessing->tstampMax < RBRGEN3_DATETIME_MIN
        || postprocessing->tstampMax > RBRGEN3_DATETIME_MAX
        || postprocessing->tstampMin > postprocessing->tstampMax
        || (timeBinning /* Depth parameters can be invalid when unused */
            && postprocessing->depthMin > postprocessing->depthMax))
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    const RBRGen3PostprocessingChannelsList *channelsList =
        &postprocessing->channels;

    for (int channel = 0; channel < channelsList->count; ++channel)
    {
        if (channelsList->channels[channel].function <
            RBRGEN3_POSTPROCESSING_AGGREGATE_MEAN
            || channelsList->channels[channel].function >=
            RBRGEN3_POSTPROCESSING_AGGREGATE_COUNT)
        {
            return RBRGEN3_INVALID_PARAMETER_VALUE;
        }
    }

    /* The default command buffer is 120B, and even without providing channels,
     * a typical postprocessing command exceeds that length. We'll send
     * parameters in groups instead of all at once. */
    RBR_TRY(RBRGen3_converse(
        instrument,
        "postprocessing binreference = %s, binfilter = %s, binsize = %.1f",
        postprocessing->binReference,
        RBRGen3PostprocessingBinFilter_name(postprocessing->binFilter),
        (double) postprocessing->binSize));

    char tstamp[RBRGEN3_SCHEDULE_TIME_LEN + 1];

    RBRGen3DateTime_toScheduleTime(postprocessing->tstampMin, tstamp);
    RBR_TRY(RBRGen3_converse(
        instrument,
        "postprocessing tstamp_min = %s",
        tstamp));

    RBRGen3DateTime_toScheduleTime(postprocessing->tstampMax, tstamp);
    RBR_TRY(RBRGen3_converse(
        instrument,
        "postprocessing tstamp_max = %s",
        tstamp));

    RBR_TRY(RBRGen3_converse(
        instrument,
        "postprocessing depth_min = %.1f, depth_max = %.1f",
        (double) postprocessing->depthMin,
        (double) postprocessing->depthMax));

    /* on-board dynamic correction only available for firmware 1.134 and above */
    if ( instrument->id.fwtype == 104 && RBRGen3Version_compare(instrument->id.version, "1.134") >= 0 )
    {
       RBR_TRY(RBRGen3_converse(
            instrument,
            "postprocessing dc_alpha = %.3f, dc_tau = %.3f, dc_tdelay = %.3f, dc_ctcoeff = %.4e",
            (double) postprocessing->dcAlpha,
            (double) postprocessing->dcTau,
            (double) postprocessing->dcTdelay,
            (double) postprocessing->dcCtCoeff));
    }

    char *commandBuffer = (char *) instrument->commandBuffer;
    int32_t *commandBufferLength = &instrument->commandBufferLength;

    *commandBufferLength = snprintf(
        commandBuffer,
        sizeof(instrument->commandBuffer),
        "postprocessing channels =");

    /* As with fetching, we want to be cautious that we don't exceed the length
     * of the command buffer when configuring many channels. We'll defensively
     * add each to the buffer, flushing as necessary. */
    char separator = ' ';
    const char *functionName;
    for (int channel = 0; channel < channelsList->count; ++channel)
    {
        functionName = RBRGen3PostprocessingAggregate_name(
            channelsList->channels[channel].function);

        if (*commandBufferLength
            + 3 /* separator + paren pair */
            + strlen(functionName)
            + strlen(channelsList->channels[channel].label)
            > sizeof(instrument->commandBuffer))
        {
            RBR_TRY(RBRGen3_sendBuffer(instrument));
            *commandBufferLength = 0;
        }

        *commandBufferLength += snprintf(
            commandBuffer + *commandBufferLength,
            sizeof(instrument->commandBuffer) - *commandBufferLength,
            "%c%s(%s)",
            separator,
            functionName,
            channelsList->channels[channel].label);
        separator = '|';
    }

    if ((size_t) *commandBufferLength + RBRGEN3_SEND_COMMAND_TERMINATOR_LEN
        > sizeof(instrument->commandBuffer))
    {
        RBR_TRY(RBRGen3_sendBuffer(instrument));
        *commandBufferLength = 0;
    }

    *commandBufferLength += snprintf(
        commandBuffer + *commandBufferLength,
        sizeof(instrument->commandBuffer) - *commandBufferLength,
        RBRGEN3_SEND_COMMAND_TERMINATOR);

    RBR_TRY(RBRGen3_sendBuffer(instrument));

    /* We don't need anything back from the response, but we do want to make
     * sure that an appropriate response is received. Because we're not issuing
     * the command via RBRGen3_converse(), we need to do this ourselves
     * by looping on RBRGen3_readResponse(). */
    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true)
    {
        RBR_TRY(RBRGen3_readResponse(instrument, false, NULL));
        RBRGen3_parseResponse(instrument, &command, &parameter);
        if (strcmp(command, "postprocessing") == 0)
        {
            break;
        }
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_setPostprocessingCommand(
    RBRGen3 *instrument,
    RBRGen3PostprocessingCommand command,
    RBRGen3PostprocessingStatus *status)
{
    if (command < RBRGEN3_POSTPROCESSING_COMMAND_START
        || command >= RBRGEN3_POSTPROCESSING_COMMAND_COUNT)
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    *status = RBRGEN3_UNKNOWN_POSTPROCESSING_STATUS;

    RBR_TRY(RBRGen3_converse(
                instrument,
                "postprocessing command = %s",
                RBRGen3PostprocessingCommand_name(command)));

    char *instrumentCommand = NULL;
    RBRGen3ResponseParameter parameter;
    while (RBRGen3_parseResponse(instrument,
                                       &instrumentCommand,
                                       &parameter),
           parameter.key != NULL && parameter.value != NULL)
    {
        if (strcmp(parameter.key, "status") != 0)
        {
            continue;
        }

        for (int i = 0; i < RBRGEN3_POSTPROCESSING_STATUS_COUNT; ++i)
        {
            if (strcmp(RBRGen3PostprocessingStatus_name(i),
                       parameter.value) == 0)
            {
                *status = i;
                break;
            }
        }
    }

    return RBRGEN3_SUCCESS;
}
