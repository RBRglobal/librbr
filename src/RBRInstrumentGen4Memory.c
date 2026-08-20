/**
 * \file RBRInstrumentGen4Memory.c
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

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Configuration.h"
#include "RBRInstrumentGen4Instrument.h"
#include "RBRInstrumentGen4Internal.h"
#include "RBRInstrumentGen4Memory.h"

RBRInstrumentGen4Error RBRInstrumentGen4_getStorage(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Storage *storage)
{
    //GEN4 todo: revisit the logic. Seems alright now.
        (void)instrument;
        (void)storage;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getDatasetPool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4DatasetPool *datasetPool){
        (void)instrument;
        (void)datasetPool;
        return RBRINSTRUMENTGEN4_SUCCESS;
    }

RBRInstrumentGen4Error RBRInstrumentGen4_getDataset(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4ConfigPool *configPool,
    RBRInstrumentGen4Dataset *dataset)
{
    //GEN4 todo: need to add logic.
    (void)instrument;
    (void)configPool;
    (void)dataset;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

const char *RBRInstrumentGen4Block_name(RBRInstrumentGen4Block block){
    switch(block)
    {
        case RBRINSTRUMENTGEN4_BLOCK_DATA:
            return "data";
        case RBRINSTRUMENTGEN4_BLOCK_EVENTS:
            return "events";
        case RBRINSTRUMENTGEN4_BLOCK_META:
            return "meta";
        case RBRINSTRUMENTGEN4_BLOCK_UNKNOWN:
        default:
            return "unknown block";
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_getDatasetByBlock(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Block block,
    RBRInstrumentGen4Dataset *dataset,
    RBRInstrumentGen4DatasetInfo *datasetInfo)
{
    char _blockname[RBRINSTRUMENTGEN4_DATABLOCK_NAME_MAX+1];
    switch(block){
        //need end to be \0. 
        case RBRINSTRUMENTGEN4_BLOCK_DATA:
            strncpy(_blockname, "data", sizeof(_blockname));
            break;
        case RBRINSTRUMENTGEN4_BLOCK_EVENTS:
            strncpy(_blockname, "events", sizeof(_blockname));
            break;
        case RBRINSTRUMENTGEN4_BLOCK_META:
            strncpy(_blockname, "meta", sizeof(_blockname));
            break;
        case RBRINSTRUMENTGEN4_BLOCK_UNKNOWN:
        default:
            return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }
    (void)instrument;
    (void)dataset;
    (void)datasetInfo;
    return RBRINSTRUMENTGEN4_SUCCESS;
}


RBRInstrumentGen4Error RBRInstrumentGen4_getDatasetBySchedule(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Schedule *schedule,
    RBRInstrumentGen4Dataset *dataset,
    RBRInstrumentGen4DatasetInfo *datasetInfo)
{
        (void)instrument;
        (void)schedule;
        (void)dataset;
        (void)datasetInfo;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getDatasetByScheduleBlock(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Schedule *schedule,
    RBRInstrumentGen4Block block,
    RBRInstrumentGen4Dataset *dataset,
    RBRInstrumentGen4DatasetInfo *datasetInfo)
{
    (void)instrument;
    (void)schedule;
    (void)block;
    (void)dataset;
    (void)datasetInfo;
    return RBRINSTRUMENTGEN4_SUCCESS;
}


RBRInstrumentGen4Error RBRInstrumentGen4_deleteDataset(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Dataset *dataset)
{
    //GEN4 todo: need to add logic.
        (void)instrument;
        (void)dataset;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteDatasetAll(
    RBRInstrumentGen4 *instrument, 
    RBRInstrumentGen4DatasetPool *datasetPool)
{
    //GEN4 todo: need to add logic.
    //need to "permit command=deletedataset" first.
        (void)instrument;
        (void)datasetPool;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

const char *RBRInstrumentGen4CountKey_name(RBRInstrumentGen4CountKey countKey)
{
    switch(countKey){
        case RBRINSTRUMENTGEN4_COUNTKEY_BYTECOUNT:
            return "bytecount";
        case RBRINSTRUMENTGEN4_COUNTKEY_SAMPLECOUNT:
            return "samplecount";
        case RBRINSTRUMENTGEN4_COUNTKEY_COUNT:
            return "eventcount";
        case RBRINSTRUMENTGEN4_UNKNOWN_COUNTKEY:
        default:
            return "unknown countkey";
    }
}

/*********************************************************************/
/**
 * \brief Keep retrying reads until we retrieve a fixed amount of data.
 *
 * This function first drains data out of RBRInstrument.responseBuffer, then
 * begins to read from the instrument. As a result, \a data must not be
 * RBRInstrument.responseBuffer!
 *
 * \param [in] instrument the instrument connection
 * \param [out] data the buffer to write into
 * \param [in] size the amount of data to write into the buffer
 */
static RBRInstrumentGen4Error RBRInstrumentGen4_fixedRead(
    struct RBRInstrumentGen4 *instrument,
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

    return RBRINSTRUMENTGEN4_SUCCESS;
}

/* CRC-CCITT */
uint16_t calculateCrcGen4(const void *data, int32_t size)
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
/**********************************************************************/
/** If using multiple instances of the command, one can use only
 * `download` or `download start=value` or `download countKey=value`. 
 * However, if any portion of source is changed, or countKey is changed, ALL
 * parameters are required.
*/
RBRInstrumentGen4Error RBRInstrumentGen4_download(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Download *download)
{
    const char *generationCommand;
    generationCommand = "download source=%s/%s/%s"
                        "%s=%" PRId32
                        "start=%" PRId32;

    const char *block = RBRInstrumentGen4Block_name(download->block);
    const char *countKey = RBRInstrumentGen4CountKey_name(download->countKey);
    RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                   generationCommand,
                                   download->dataset->label,
                                   download->dataset->config->scheduleList[0],
                                   block,
                                   countKey,
                                   download->countValue,
                                   download->startOffset));
    
    /* Fill the user-provided buffer. RBRInstrumentGen4_fixedRead() will first pull
     * leftover data from RBRInstrument.responseBuffer, then read from the
     * instrument. */
    RBR_TRY(RBRInstrumentGen4_fixedRead(instrument, download->data, download->startOffset));

    /* CRC check the last two bytes. */
    union
    {
        uint8_t buf[2];
        uint16_t value;
    }
    crc;

    RBR_TRY(RBRInstrumentGen4_fixedRead(instrument, crc.buf, 2));
    /* The logger reports the CRC as big-endian. Under the assumption that the
     * host is little-endian, we'll byte swap it before using it for
     * comparison. ntohs() is POSIX but not part of the C standard, and we want
     * to target pure C99, so we can't use it here. */

    crc.value = (crc.value >> 8) | (crc.value << 8);

    uint16_t calculatedCrc = calculateCrcGen4(download->data, download->countValue);
    if (calculatedCrc != crc.value)
    {
        return RBRINSTRUMENTGEN4_CHECKSUM_ERROR;
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

/***************************************************************************/
const char *RBRInstrumentGen4PostprocessingAggregate_name(
    RBRInstrumentGen4PostprocessingAggregate function)
{
    switch (function)
    {
    case RBRINSTRUMENTGEN4_POSTPROCESSING_AGGREGATE_MEAN:
        return "mean";
    case RBRINSTRUMENTGEN4_POSTPROCESSING_AGGREGATE_STD:
        return "std";
    case RBRINSTRUMENTGEN4_POSTPROCESSING_AGGREGATE_SAMPLE_COUNT:
        return "count";
    case RBRINSTRUMENTGEN4_POSTPROCESSING_AGGREGATE_COUNT:
        return "post-processing aggregate function count";
    case RBRINSTRUMENTGEN4_UNKNOWN_POSTPROCESSING_AGGREGATE:
    default:
        return "unknown post-processing aggregate function";
    }
}

const char *RBRInstrumentGen4PostprocessingStatus_name(
    RBRInstrumentGen4PostprocessingStatus status)
{
    switch (status)
    {
    case RBRINSTRUMENTGEN4_POSTPROCESSING_STATUS_IDLE:
        return "idle";
    case RBRINSTRUMENTGEN4_POSTPROCESSING_STATUS_PROCESSING:
        return "processing";
    case RBRINSTRUMENTGEN4_POSTPROCESSING_STATUS_COMPLETED:
        return "completed";
    case RBRINSTRUMENTGEN4_POSTPROCESSING_STATUS_ABORTED:
        return "aborted";
    case RBRINSTRUMENTGEN4_POSTPROCESSING_STATUS_COUNT:
        return "post-processing status count";
    case RBRINSTRUMENTGEN4_UNKNOWN_POSTPROCESSING_STATUS:
    default:
        return "unknown post-processing status";
    }
}

const char *RBRInstrumentGen4PostprocessingCommand_name(
    RBRInstrumentGen4PostprocessingCommand command)
{
    switch (command)
    {
    case RBRINSTRUMENTGEN4_POSTPROCESSING_COMMAND_START:
        return "start";
    case RBRINSTRUMENTGEN4_POSTPROCESSING_COMMAND_RESET:
        return "reset";
    case RBRINSTRUMENTGEN4_POSTPROCESSING_COMMAND_ABORT:
        return "abort";
    case RBRINSTRUMENTGEN4_POSTPROCESSING_COMMAND_COUNT:
        return "post-processing command count";
    case RBRINSTRUMENTGEN4_UNKNOWN_POSTPROCESSING_COMMAND:
    default:
        return "unknown post-processing command";
    }
}

const char *RBRInstrumentGen4PostprocessingBinFilter_name(
    RBRInstrumentGen4PostprocessingBinFilter filter)
{
    switch (filter)
    {
    case RBRINSTRUMENTGEN4_POSTPROCESSING_BINFILTER_NONE:
        return "none";
    case RBRINSTRUMENTGEN4_POSTPROCESSING_BINFILTER_ASCENTONLY:
        return "ascentonly";
    case RBRINSTRUMENTGEN4_POSTPROCESSING_BINFILTER_DESCENTONLY:
        return "descentonly";
    case RBRINSTRUMENTGEN4_POSTPROCESSING_BINFILTER_COUNT:
        return "post-processing bin filter count";
    case RBRINSTRUMENTGEN4_UNKNOWN_POSTPROCESSING_BINFILTER:
    default:
        return "unknown post-processing bin filter";
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_getPostprocessing(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Postprocessing *postprocessing)
{
    memset(postprocessing, 0, sizeof(RBRInstrumentGen4Postprocessing));
    postprocessing->status = RBRINSTRUMENTGEN4_UNKNOWN_POSTPROCESSING_STATUS;
    postprocessing->binFilter = RBRINSTRUMENTGEN4_UNKNOWN_POSTPROCESSING_BINFILTER;

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "postprocessing all"));

    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "status") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENTGEN4_POSTPROCESSING_STATUS_COUNT; ++i)
            {
                if (strcmp(RBRInstrumentGen4PostprocessingStatus_name(i),
                           parameter.value) == 0)
                {
                    postprocessing->status = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "channels") == 0)
        {
            RBRInstrumentGen4PostprocessingChannelList *channelList =
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
                     i < RBRINSTRUMENTGEN4_POSTPROCESSING_AGGREGATE_COUNT;
                     ++i)
                {
                    if (strcmp(RBRInstrumentGen4PostprocessingAggregate_name(i),
                               functionStart) == 0)
                    {
                        channelList->channels[channel].function = i;
                        break;
                    }
                }

                snprintf(channelList->channels[channel].label,
                         sizeof(channelList->channels[channel].label),
                         "%s",
                         labelStart);

                next = strtok(NULL, "(");
            }
            channelList->count = channel;
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
                 i < RBRINSTRUMENTGEN4_POSTPROCESSING_BINFILTER_COUNT;
                 ++i)
            {
                if (strcmp(RBRInstrumentGen4PostprocessingBinFilter_name(i),
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
            RBRInstrumentGen4DateTime_parseScheduleTime(parameter.value,
                                                    &postprocessing->tstampMin,
                                                    NULL);
        }
        else if (strcmp(parameter.key, "tstamp_max") == 0)
        {
            RBRInstrumentGen4DateTime_parseScheduleTime(parameter.value,
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

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setPostprocessing(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Postprocessing *postprocessing)
{
    bool timeBinning = strcmp(postprocessing->binReference, "tstamp") == 0;

    if (postprocessing->channels.count < 0
        || postprocessing->channels.count >=
        RBRINSTRUMENTGEN4_POSTPROCESSING_CHANNEL_MAX
        || postprocessing->binFilter <
        RBRINSTRUMENTGEN4_POSTPROCESSING_BINFILTER_NONE
        || postprocessing->binFilter >=
        RBRINSTRUMENTGEN4_POSTPROCESSING_BINFILTER_COUNT
        || postprocessing->binSize < 0
        || postprocessing->tstampMin < RBRINSTRUMENTGEN4_DATETIME_MIN
        || postprocessing->tstampMin > RBRINSTRUMENTGEN4_DATETIME_MAX
        || postprocessing->tstampMax < RBRINSTRUMENTGEN4_DATETIME_MIN
        || postprocessing->tstampMax > RBRINSTRUMENTGEN4_DATETIME_MAX
        || postprocessing->tstampMin > postprocessing->tstampMax
        || (timeBinning /* Depth parameters can be invalid when unused */
            && postprocessing->depthMin > postprocessing->depthMax))
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    const RBRInstrumentGen4PostprocessingChannelList *channelList =
        &postprocessing->channels;

    for (int channel = 0; channel < channelList->count; ++channel)
    {
        if (channelList->channels[channel].function <
            RBRINSTRUMENTGEN4_POSTPROCESSING_AGGREGATE_MEAN
            || channelList->channels[channel].function >=
            RBRINSTRUMENTGEN4_POSTPROCESSING_AGGREGATE_COUNT)
        {
            return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
        }
    }

    /* The default command buffer is 120B, and even without providing channels,
     * a typical postprocessing command exceeds that length. We'll send
     * parameters in groups instead of all at once. */
    RBR_TRY(RBRInstrumentGen4_converse(
        instrument,
        "postprocessing binreference=%s binfilter=%s binsize=%.1f",
        postprocessing->binReference,
        RBRInstrumentGen4PostprocessingBinFilter_name(postprocessing->binFilter),
        (double) postprocessing->binSize));

    char tstamp[RBRINSTRUMENTGEN4_SCHEDULE_TIME_LEN + 1];

    RBRInstrumentGen4DateTime_toScheduleTime(postprocessing->tstampMin, tstamp);
    RBR_TRY(RBRInstrumentGen4_converse(
        instrument,
        "postprocessing tstamp_min=%s",
        tstamp));

    RBRInstrumentGen4DateTime_toScheduleTime(postprocessing->tstampMax, tstamp);
    RBR_TRY(RBRInstrumentGen4_converse(
        instrument,
        "postprocessing tstamp_max=%s",
        tstamp));

    RBR_TRY(RBRInstrumentGen4_converse(
        instrument,
        "postprocessing depth_min=%.1f depth_max=%.1f",
        (double) postprocessing->depthMin,
        (double) postprocessing->depthMax));

    /* on-board dynamic correction only available for firmware 1.134 and above */
    if ( instrument->id.fwtype == 104 && RBRInstrumentGen4Version_compare(instrument->id.fwversion, "1.134") >= 0 )
    {
       RBR_TRY(RBRInstrumentGen4_converse(
            instrument,
            "postprocessing dc_alpha=%.3f dc_tau=%.3f dc_tdelay=%.3f dc_ctcoeff=%.4e",
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
        "postprocessing channels=");

    /* As with fetching, we want to be cautious that we don't exceed the length
     * of the command buffer when configuring many channels. We'll defensively
     * add each to the buffer, flushing as necessary. */
    char separator = ' ';
    const char *functionName;
    for (int channel = 0; channel < channelList->count; ++channel)
    {
        functionName = RBRInstrumentGen4PostprocessingAggregate_name(
            channelList->channels[channel].function);

        if (*commandBufferLength
            + 3 /* separator + paren pair */
            + strlen(functionName)
            + strlen(channelList->channels[channel].label)
            > sizeof(instrument->commandBuffer))
        {
            RBR_TRY(RBRInstrumentGen4_sendBuffer(instrument));
            *commandBufferLength = 0;
        }

        *commandBufferLength += snprintf(
            commandBuffer + *commandBufferLength,
            sizeof(instrument->commandBuffer) - *commandBufferLength,
            "%c%s(%s)",
            separator,
            functionName,
            channelList->channels[channel].label);
        separator = '|';
    }

    if ((size_t) *commandBufferLength + RBRINSTRUMENTGEN4_SEND_COMMAND_TERMINATOR_LEN
        > sizeof(instrument->commandBuffer))
    {
        RBR_TRY(RBRInstrumentGen4_sendBuffer(instrument));
        *commandBufferLength = 0;
    }

    *commandBufferLength += snprintf(
        commandBuffer + *commandBufferLength,
        sizeof(instrument->commandBuffer) - *commandBufferLength,
        RBRINSTRUMENTGEN4_SEND_COMMAND_TERMINATOR);

    RBR_TRY(RBRInstrumentGen4_sendBuffer(instrument));

    /* We don't need anything back from the response, but we do want to make
     * sure that an appropriate response is received. Because we're not issuing
     * the command via RBRInstrumentGen4_converse(), we need to do this ourselves
     * by looping on RBRInstrumentGen4_readResponse(). */
    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBR_TRY(RBRInstrumentGen4_readResponse(instrument, false, NULL));
        RBRInstrumentGen4_parseResponse(instrument, &command, &parameter);
        if (strcmp(command, "postprocessing") == 0)
        {
            break;
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setPostprocessingCommand(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4PostprocessingCommand command,
    RBRInstrumentGen4PostprocessingStatus *status)
{
    if (command < RBRINSTRUMENTGEN4_POSTPROCESSING_COMMAND_START
        || command >= RBRINSTRUMENTGEN4_POSTPROCESSING_COMMAND_COUNT)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    *status = RBRINSTRUMENTGEN4_UNKNOWN_POSTPROCESSING_STATUS;

    RBR_TRY(RBRInstrumentGen4_converse(
                instrument,
                "postprocessing command=%s",
                RBRInstrumentGen4PostprocessingCommand_name(command)));

    char *instrumentCommand = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (RBRInstrumentGen4_parseResponse(instrument,
                                       &instrumentCommand,
                                       &parameter),
           parameter.key != NULL && parameter.value != NULL)
    {
        if (strcmp(parameter.key, "status") != 0)
        {
            continue;
        }

        for (int i = 0; i < RBRINSTRUMENTGEN4_POSTPROCESSING_STATUS_COUNT; ++i)
        {
            if (strcmp(RBRInstrumentGen4PostprocessingStatus_name(i),
                       parameter.value) == 0)
            {
                *status = i;
                break;
            }
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}
/*******************************************************************************/
