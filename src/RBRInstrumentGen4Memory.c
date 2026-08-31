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
/* Required for PRId32. */
#include <inttypes.h>

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Configuration.h"
#include "RBRInstrumentGen4Internal.h"
#include "RBRInstrumentGen4Memory.h"

const char *RBRInstrumentGen4StorageAccess_name(
    RBRInstrumentGen4StorageAccess access)
{
    switch (access)
    {
    case RBRINSTRUMENTGEN4_STORAGE_ACCESS_INSTRUMENT:
        return "instrument";
    case RBRINSTRUMENTGEN4_STORAGE_ACCESS_USBHOST:
        return "usbhost";
    case RBRINSTRUMENTGEN4_STORAGE_ACCESS_COUNT:
        return "storage access count";
    case RBRINSTRUMENTGEN4_UNKNOWN_STORAGE_ACCESS:
    default:
        return "unknown storage access";
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_getStorage(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Storage *storage)
{
    memset(storage, 0, sizeof(RBRInstrumentGen4Storage));
    storage->access = RBRINSTRUMENTGEN4_UNKNOWN_STORAGE_ACCESS;

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "storage"));

    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "used") == 0)
        {
            storage->used = strtoll(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "remaining") == 0)
        {
            storage->remaining = strtoll(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "size") == 0)
        {
            storage->size = strtoll(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "access") == 0)
        {
            for (int i = 0;
                 i < RBRINSTRUMENTGEN4_STORAGE_ACCESS_COUNT;
                 i++)
            {
                if (strcmp(parameter.value,
                           RBRInstrumentGen4StorageAccess_name(i)) == 0)
                {
                    storage->access = i;
                    break;
                }
            }
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setStorage(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Storage *storage)
{
    if (storage->access < 0
        || storage->access >= RBRINSTRUMENTGEN4_STORAGE_ACCESS_COUNT)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrumentGen4_converse(
        instrument,
        "storage access=%s",
        RBRInstrumentGen4StorageAccess_name(storage->access));
}

const char *RBRInstrumentGen4DatasetStatus_name(
    RBRInstrumentGen4DatasetStatus status)
{
    switch (status)
    {
    case RBRINSTRUMENTGEN4_DATASET_STATUS_OPEN:
        return "open";
    case RBRINSTRUMENTGEN4_DATASET_STATUS_CLOSED:
        return "closed";
    case RBRINSTRUMENTGEN4_DATASET_STATUS_COUNT:
        return "dataset status count";
    case RBRINSTRUMENTGEN4_UNKNOWN_DATASET_STATUS:
    default:
        return "unknown dataset status";
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_getDatasetPool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4DatasetPool *datasetPool)
{
    memset(datasetPool, 0, sizeof(RBRInstrumentGen4DatasetPool));

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "dataset"));

    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "count") == 0)
        {
            datasetPool->count = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "maxcount") == 0)
        {
            datasetPool->maxCount = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "list") == 0)
        {
            /* An empty pool reports `none`. */
            if (strcmp(parameter.value, "none") == 0)
            {
                continue;
            }

            char *value = parameter.value;
            for (int32_t dataset = 0;
                 value != NULL
                 && dataset < RBRINSTRUMENTGEN4_DATASET_COUNT_MAX;
                 dataset++)
            {
                char *nextValue = RBRInstrumentGen4_splitListValue(value);

                snprintf(datasetPool->pool[dataset].label,
                         sizeof(datasetPool->pool[dataset].label),
                         "%s",
                         value);

                value = nextValue;
            }
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getDataset(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Dataset *dataset)
{
    if (dataset->label[0] == '\0')
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    dataset->status = RBRINSTRUMENTGEN4_UNKNOWN_DATASET_STATUS;
    dataset->scheduleCount = 0;
    memset(dataset->scheduleList, 0, sizeof(dataset->scheduleList));
    dataset->byteCount = 0;
    dataset->dataType = RBRINSTRUMENTGEN4_UNKNOWN_DATATYPE;

    RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                       "dataset %s",
                                       dataset->label));

    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "status") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENTGEN4_DATASET_STATUS_COUNT; i++)
            {
                if (strcmp(parameter.value,
                           RBRInstrumentGen4DatasetStatus_name(i)) == 0)
                {
                    dataset->status = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "schedulelist") == 0)
        {
            char *value = parameter.value;
            int32_t schedule = 0;
            while (value != NULL
                   && schedule < RBRINSTRUMENTGEN4_SCHEDULE_COUNT_MAX)
            {
                char *nextValue = RBRInstrumentGen4_splitListValue(value);

                snprintf(dataset->scheduleList[schedule],
                         sizeof(dataset->scheduleList[schedule]),
                         "%s",
                         value);

                value = nextValue;
                schedule++;
            }
            dataset->scheduleCount = schedule;
        }
        else if (strcmp(parameter.key, "bytecount") == 0)
        {
            dataset->byteCount = strtoll(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "datatype") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENTGEN4_DATATYPE_COUNT; i++)
            {
                if (strcmp(parameter.value,
                           RBRInstrumentGen4DataType_name(i)) == 0)
                {
                    dataset->dataType = i;
                    break;
                }
            }
        }
    }

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
        case RBRINSTRUMENTGEN4_BLOCK_COUNT:
            return "block count";
        case RBRINSTRUMENTGEN4_BLOCK_UNKNOWN:
        default:
            return "unknown block";
    }
}

/**
 * \brief Parse the counts out of a `dataset` block query response.
 *
 * A count the caller does not expect for the block type is passed as NULL
 * and left unparsed.
 */
static void RBRInstrumentGen4Dataset_parseBlockResponse(
    RBRInstrumentGen4 *instrument,
    int64_t *byteCount,
    int64_t *sampleCount,
    int64_t *eventCount)
{
    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (byteCount != NULL
                 && strcmp(parameter.key, "bytecount") == 0)
        {
            *byteCount = strtoll(parameter.value, NULL, 10);
        }
        else if (sampleCount != NULL
                 && strcmp(parameter.key, "samplecount") == 0)
        {
            *sampleCount = strtoll(parameter.value, NULL, 10);
        }
        else if (eventCount != NULL
                 && strcmp(parameter.key, "eventcount") == 0)
        {
            *eventCount = strtoll(parameter.value, NULL, 10);
        }
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4Dataset_getEventsBlock(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Dataset *dataset,
    RBRInstrumentGen4DatasetEventsBlock *block)
{
    if (dataset->label[0] == '\0')
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    memset(block, 0, sizeof(RBRInstrumentGen4DatasetEventsBlock));

    RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                       "dataset %s/events",
                                       dataset->label));

    RBRInstrumentGen4Dataset_parseBlockResponse(instrument,
                                                &block->byteCount,
                                                NULL,
                                                &block->eventCount);

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4Dataset_getMetaBlock(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Dataset *dataset,
    RBRInstrumentGen4DatasetMetaBlock *block)
{
    if (dataset->label[0] == '\0')
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    memset(block, 0, sizeof(RBRInstrumentGen4DatasetMetaBlock));

    RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                       "dataset %s/meta",
                                       dataset->label));

    RBRInstrumentGen4Dataset_parseBlockResponse(instrument,
                                                &block->byteCount,
                                                NULL,
                                                NULL);

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4Dataset_getScheduleBlock(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Dataset *dataset,
    const char *scheduleLabel,
    RBRInstrumentGen4DatasetScheduleBlock *block)
{
    if (dataset->label[0] == '\0' || scheduleLabel[0] == '\0')
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    memset(block, 0, sizeof(RBRInstrumentGen4DatasetScheduleBlock));

    RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                       "dataset %s/%s",
                                       dataset->label,
                                       scheduleLabel));

    RBRInstrumentGen4Dataset_parseBlockResponse(instrument,
                                                &block->byteCount,
                                                NULL,
                                                NULL);

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4Dataset_getScheduleEventsBlock(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Dataset *dataset,
    const char *scheduleLabel,
    RBRInstrumentGen4DatasetEventsBlock *block)
{
    if (dataset->label[0] == '\0' || scheduleLabel[0] == '\0')
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    memset(block, 0, sizeof(RBRInstrumentGen4DatasetEventsBlock));

    RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                       "dataset %s/%s/events",
                                       dataset->label,
                                       scheduleLabel));

    RBRInstrumentGen4Dataset_parseBlockResponse(instrument,
                                                &block->byteCount,
                                                NULL,
                                                &block->eventCount);

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4Dataset_getScheduleDataBlock(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Dataset *dataset,
    const char *scheduleLabel,
    RBRInstrumentGen4DatasetDataBlock *block)
{
    if (dataset->label[0] == '\0' || scheduleLabel[0] == '\0')
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    memset(block, 0, sizeof(RBRInstrumentGen4DatasetDataBlock));

    RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                       "dataset %s/%s/data",
                                       dataset->label,
                                       scheduleLabel));

    RBRInstrumentGen4Dataset_parseBlockResponse(instrument,
                                                &block->byteCount,
                                                &block->sampleCount,
                                                NULL);

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteDataset(
    RBRInstrumentGen4 *instrument,
    const char *label)
{
    if (label[0] == '\0')
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrumentGen4_converse(instrument,
                                      "dataset delete %s",
                                      label);
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteDatasetAll(
    RBRInstrumentGen4 *instrument)
{
    return RBRInstrumentGen4_converse(instrument, "dataset delete all");
}

const char *RBRInstrumentGen4DownloadDataUnit_name(
    RBRInstrumentGen4DownloadDataUnit unit)
{
    switch (unit)
    {
    case RBRINSTRUMENTGEN4_DOWNLOAD_DATA_UNIT_BYTES:
        return "bytes";
    case RBRINSTRUMENTGEN4_DOWNLOAD_DATA_UNIT_SAMPLES:
        return "samples";
    case RBRINSTRUMENTGEN4_DOWNLOAD_DATA_UNIT_COUNT:
        return "download data unit count";
    case RBRINSTRUMENTGEN4_UNKNOWN_DOWNLOAD_DATA_UNIT:
    default:
        return "unknown download data unit";
    }
}

const char *RBRInstrumentGen4DownloadEventsUnit_name(
    RBRInstrumentGen4DownloadEventsUnit unit)
{
    switch (unit)
    {
    case RBRINSTRUMENTGEN4_DOWNLOAD_EVENTS_UNIT_BYTES:
        return "bytes";
    case RBRINSTRUMENTGEN4_DOWNLOAD_EVENTS_UNIT_EVENTS:
        return "events";
    case RBRINSTRUMENTGEN4_DOWNLOAD_EVENTS_UNIT_COUNT:
        return "download events unit count";
    case RBRINSTRUMENTGEN4_UNKNOWN_DOWNLOAD_EVENTS_UNIT:
    default:
        return "unknown download events unit";
    }
}

/**
 * \brief Keep retrying reads until we retrieve a fixed amount of data.
 *
 * This function first drains data out of RBRInstrumentGen4.responseBuffer,
 * then begins to read from the instrument. As a result, \a data must not be
 * RBRInstrumentGen4.responseBuffer!
 *
 * \param [in] instrument the instrument connection
 * \param [out] data the buffer to write into
 * \param [in] size the amount of data to write into the buffer
 */
static RBRInstrumentGen4Error RBRInstrumentGen4_fixedRead(
    struct RBRInstrumentGen4 *instrument,
    void *data,
    int64_t size)
{
    int64_t bufferLength = 0;
    int32_t readLength;

    /* Can we steal from the response buffer? */
    if (instrument->lastResponseLength < instrument->responseBufferLength)
    {
        readLength = instrument->responseBufferLength
                     - instrument->lastResponseLength;
        if (readLength > size)
        {
            readLength = (int32_t) size;
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
        readLength = size - bufferLength > INT32_MAX
                     ? INT32_MAX
                     : (int32_t) (size - bufferLength);

        RBR_TRY(instrument->callbacks.read(
                    instrument,
                    ((uint8_t *) data) + bufferLength,
                    &readLength));

        bufferLength += readLength;
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

/**
 * \brief Perform a download: send the command, parse its echo, read the
 * binary transfer, and verify its trailing CRC.
 *
 * The echo always reports `bytecount`; when \a countKey is not `bytecount`,
 * the count in the requested unit is reported under \a countKey as well.
 *
 * \param [in] instrument the instrument connection
 * \param [in] countKey the count key sent with the command
 * \param [in,out] count the requested amount; updated to the amount reported
 * \param [out] byteCount the byte count reported
 * \param [out] data the buffer receiving the transfer
 * \param [in] dataSize the capacity of \a data in bytes
 */
static RBRInstrumentGen4Error RBRInstrumentGen4Dataset_downloadCommon(
    RBRInstrumentGen4 *instrument,
    const char *countKey,
    int64_t *count,
    int64_t *byteCount,
    void *data,
    int64_t dataSize)
{
    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "bytecount") == 0)
        {
            *byteCount = strtol(parameter.value, NULL, 10);
        }

        if (strcmp(parameter.key, countKey) == 0)
        {
            *count = strtoll(parameter.value, NULL, 10);
        }
    }

    if (*byteCount > dataSize)
    {
        /*
         * We requested more data than we have room for, so we need to drain
         * all data buffered on the link, CRC included, before we can return an 
         * error. Otherwise, the next conversation with the instrument would
         * read it. We'll use the command buffer as temporary storage.
         */
        int64_t rest = *byteCount + (int64_t) sizeof(uint16_t);
        while (rest > 0)
        {
            int64_t chunk =
                rest > (int64_t) sizeof(instrument->commandBuffer)
                ? (int64_t) sizeof(instrument->commandBuffer)
                : rest;
            RBR_TRY(RBRInstrumentGen4_fixedRead(instrument,
                                                instrument->commandBuffer,
                                                chunk));
            rest -= chunk;
        }
        return RBRINSTRUMENTGEN4_BUFFER_TOO_SMALL;
    }

    RBR_TRY(RBRInstrumentGen4_fixedRead(instrument, data, *byteCount));

    /* The instrument transmits the 16-bit CRC most significant byte first. */
    uint8_t crc[2];
    RBR_TRY(RBRInstrumentGen4_fixedRead(instrument, crc, 2));

    uint16_t reportedCrc = (uint16_t) ((crc[0] << 8) | crc[1]);
    if (calculateCrcGen4(data, *byteCount) != reportedCrc)
    {
        return RBRINSTRUMENTGEN4_CHECKSUM_ERROR;
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4Dataset_downloadScheduleData(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Dataset *dataset,
    const char *scheduleLabel,
    RBRInstrumentGen4DownloadData *download)
{
    if (dataset->label[0] == '\0'
        || scheduleLabel[0] == '\0'
        || (download->unit != RBRINSTRUMENTGEN4_DOWNLOAD_DATA_UNIT_BYTES
            && download->unit != RBRINSTRUMENTGEN4_DOWNLOAD_DATA_UNIT_SAMPLES)
        || download->count < 0
        || download->start < 0
        || download->data == NULL)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    bool bytes = download->unit == RBRINSTRUMENTGEN4_DOWNLOAD_DATA_UNIT_BYTES;
    const char *countKey = bytes ? "bytecount" : "samplecount";

    download->byteCount = 0;

    RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                       "download %s/%s/data"
                                       " %s=%" PRId64 " %s=%" PRId64,
                                       dataset->label,
                                       scheduleLabel,
                                       countKey,
                                       download->count,
                                       bytes ? "bytestart" : "samplestart",
                                       download->start));

    RBR_TRY(RBRInstrumentGen4Dataset_downloadCommon(
                instrument,
                countKey,
                &download->count,
                &download->byteCount,
                download->data,
                download->dataSize));

    return RBRINSTRUMENTGEN4_SUCCESS;
}

/**
 * \brief Perform an events download from the whole dataset (\a scheduleLabel
 * NULL) or from one schedule.
 *
 * \param [in] instrument the instrument connection
 * \param [in] dataset the dataset, selected by its label
 * \param [in] scheduleLabel the schedule, or NULL for the whole dataset
 * \param [in,out] download the download request and its result
 */
static RBRInstrumentGen4Error RBRInstrumentGen4Dataset_downloadEventsCommon(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Dataset *dataset,
    const char *scheduleLabel,
    RBRInstrumentGen4DownloadEvents *download)
{
    if (dataset->label[0] == '\0'
        || (download->unit != RBRINSTRUMENTGEN4_DOWNLOAD_EVENTS_UNIT_BYTES
            && download->unit
               != RBRINSTRUMENTGEN4_DOWNLOAD_EVENTS_UNIT_EVENTS)
        || download->count < 0
        || download->start < 0
        || download->data == NULL)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    bool bytes
        = download->unit == RBRINSTRUMENTGEN4_DOWNLOAD_EVENTS_UNIT_BYTES;
    const char *countKey = bytes ? "bytecount" : "eventcount";

    download->byteCount = 0;

    if (scheduleLabel == NULL)
    {
        RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                           "download %s/events"
                                           " %s=%" PRId64 " %s=%" PRId64,
                                           dataset->label,
                                           countKey,
                                           download->count,
                                           bytes ? "bytestart" : "eventstart",
                                           download->start));
    }
    else
    {
        RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                           "download %s/%s/events"
                                           " %s=%" PRId64 " %s=%" PRId64,
                                           dataset->label,
                                           scheduleLabel,
                                           countKey,
                                           download->count,
                                           bytes ? "bytestart" : "eventstart",
                                           download->start));
    }

    RBR_TRY(RBRInstrumentGen4Dataset_downloadCommon(
                instrument,
                countKey,
                &download->count,
                &download->byteCount,
                download->data,
                download->dataSize));

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4Dataset_downloadEvents(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Dataset *dataset,
    RBRInstrumentGen4DownloadEvents *download)
{
    return RBRInstrumentGen4Dataset_downloadEventsCommon(instrument,
                                                       dataset,
                                                       NULL,
                                                       download);
}

RBRInstrumentGen4Error RBRInstrumentGen4Dataset_downloadScheduleEvents(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Dataset *dataset,
    const char *scheduleLabel,
    RBRInstrumentGen4DownloadEvents *download)
{
    if (scheduleLabel[0] == '\0')
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrumentGen4Dataset_downloadEventsCommon(instrument,
                                                       dataset,
                                                       scheduleLabel,
                                                       download);
}

RBRInstrumentGen4Error RBRInstrumentGen4Dataset_downloadMeta(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Dataset *dataset,
    RBRInstrumentGen4DownloadMeta *download)
{
    if (dataset->label[0] == '\0'
        || download->byteCount < 0
        || download->byteStart < 0
        || download->data == NULL)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                       "download %s/meta"
                                       " bytecount=%" PRId64
                                       " bytestart=%" PRId64,
                                       dataset->label,
                                       download->byteCount,
                                       download->byteStart));

    int64_t byteCount = 0;
    RBR_TRY(RBRInstrumentGen4Dataset_downloadCommon(instrument,
                                                    "bytecount",
                                                    &download->byteCount,
                                                    &byteCount,
                                                    download->data,
                                                    download->dataSize));

    return RBRINSTRUMENTGEN4_SUCCESS;
}

/* CRC-CCITT */
uint16_t calculateCrcGen4(const void *data, int64_t size)
{
#define CRC_POLYNOMIAL 0x1021

    uint16_t crc = 0xFFFF;

    for (int64_t i = 0; i < size; i++)
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
