/**
 * \file RBRGen4Memory.c
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

#include "RBRGen4.h"
#include "RBRGen4Configuration.h"
#include "RBRGen4Internal.h"
#include "RBRGen4Memory.h"

const char *RBRGen4StorageAccess_name(
    RBRGen4StorageAccess access)
{
    switch (access)
    {
    case RBRGEN4_STORAGE_ACCESS_INSTRUMENT:
        return "instrument";
    case RBRGEN4_STORAGE_ACCESS_USBHOST:
        return "usbhost";
    case RBRGEN4_STORAGE_ACCESS_COUNT:
        return "storage access count";
    case RBRGEN4_UNKNOWN_STORAGE_ACCESS:
    default:
        return "unknown storage access";
    }
}

RBRGen4Error RBRGen4_getStorage(
    RBRGen4 *conn,
    RBRGen4Storage *storage)
{
    memset(storage, 0, sizeof(RBRGen4Storage));
    storage->access = RBRGEN4_UNKNOWN_STORAGE_ACCESS;

    RBR_TRY(RBRGen4_converse(conn, "storage"));

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(conn,
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
                 i < RBRGEN4_STORAGE_ACCESS_COUNT;
                 i++)
            {
                if (strcmp(parameter.value,
                           RBRGen4StorageAccess_name(i)) == 0)
                {
                    storage->access = i;
                    break;
                }
            }
        }
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_setStorage(
    RBRGen4 *conn,
    const RBRGen4Storage *storage)
{
    if (storage->access < 0
        || storage->access >= RBRGEN4_STORAGE_ACCESS_COUNT)
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGen4_converse(
        conn,
        "storage access=%s",
        RBRGen4StorageAccess_name(storage->access));
}

const char *RBRGen4DatasetStatus_name(
    RBRGen4DatasetStatus status)
{
    switch (status)
    {
    case RBRGEN4_DATASET_STATUS_OPEN:
        return "open";
    case RBRGEN4_DATASET_STATUS_CLOSED:
        return "closed";
    case RBRGEN4_DATASET_STATUS_COUNT:
        return "dataset status count";
    case RBRGEN4_UNKNOWN_DATASET_STATUS:
    default:
        return "unknown dataset status";
    }
}

RBRGen4Error RBRGen4_getDatasetPool(
    RBRGen4 *conn,
    RBRGen4DatasetPool *datasetPool)
{
    datasetPool->count = 0;
    datasetPool->maxCount = 0;
    memset(datasetPool->pool,
           0,
           datasetPool->size * sizeof(RBRGen4Dataset));

    RBR_TRY(RBRGen4_converse(conn, "dataset"));

    RBRGen4Error err = RBRGEN4_SUCCESS;
    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(conn,
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
            if (strcmp(parameter.value, RBRGEN4_EMPTY_LIST) == 0)
            {
                continue;
            }

            /* Datasets past the pool's capacity are discarded. */
            char *value = parameter.value;
            for (int32_t i = 0; value != NULL; i++)
            {
                if (i >= datasetPool->size)
                {
                    err = RBRGEN4_TRUNCATED;
                    break;
                }

                char *nextValue = RBRGen4_splitListValue(value);

                snprintf(datasetPool->pool[i].label,
                         sizeof(datasetPool->pool[i].label),
                         "%s",
                         value);

                value = nextValue;
            }
        }
    }

    return err;
}

RBRGen4Error RBRGen4_getDataset(
    RBRGen4 *conn,
    RBRGen4Dataset *dataset,
    RBRGen4LabelList *scheduleList)
{
    if (dataset->label[0] == '\0')
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    RBR_RESET_EXCEPT(dataset, label);
    dataset->status = RBRGEN4_UNKNOWN_DATASET_STATUS;
    dataset->dataType = RBRGEN4_UNKNOWN_DATATYPE;
    if (scheduleList != NULL)
    {
        scheduleList->count = 0;
    }

    RBR_TRY(RBRGen4_converse(conn,
                                       "dataset %s",
                                       dataset->label));

    RBRGen4Error err = RBRGEN4_SUCCESS;
    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(conn,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "status") == 0)
        {
            for (int i = 0; i < RBRGEN4_DATASET_STATUS_COUNT; i++)
            {
                if (strcmp(parameter.value,
                           RBRGen4DatasetStatus_name(i)) == 0)
                {
                    dataset->status = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "schedulelist") == 0
                 && scheduleList != NULL)
        {
            err = RBRGen4_copyLabelList(scheduleList,
                                                  parameter.value);
        }
        else if (strcmp(parameter.key, "bytecount") == 0)
        {
            dataset->byteCount = strtoll(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "datatype") == 0)
        {
            for (int i = 0; i < RBRGEN4_DATATYPE_COUNT; i++)
            {
                if (strcmp(parameter.value,
                           RBRGen4DataType_name(i)) == 0)
                {
                    dataset->dataType = i;
                    break;
                }
            }
        }
    }

    return err;
}

const char *RBRGen4Block_name(RBRGen4Block block){
    switch(block)
    {
        case RBRGEN4_BLOCK_DATA:
            return "data";
        case RBRGEN4_BLOCK_EVENTS:
            return "events";
        case RBRGEN4_BLOCK_META:
            return "meta";
        case RBRGEN4_BLOCK_COUNT:
            return "block count";
        case RBRGEN4_BLOCK_UNKNOWN:
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
static void RBRGen4Dataset_parseBlockResponse(
    RBRGen4 *conn,
    int64_t *byteCount,
    int64_t *sampleCount,
    int64_t *eventCount)
{
    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(conn,
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

RBRGen4Error RBRGen4Dataset_getEventsBlock(
    RBRGen4 *conn,
    const RBRGen4Dataset *dataset,
    RBRGen4DatasetEventsBlock *block)
{
    if (dataset->label[0] == '\0')
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    memset(block, 0, sizeof(RBRGen4DatasetEventsBlock));

    RBR_TRY(RBRGen4_converse(conn,
                                       "dataset %s/events",
                                       dataset->label));

    RBRGen4Dataset_parseBlockResponse(conn,
                                                &block->byteCount,
                                                NULL,
                                                &block->eventCount);

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4Dataset_getMetaBlock(
    RBRGen4 *conn,
    const RBRGen4Dataset *dataset,
    RBRGen4DatasetMetaBlock *block)
{
    if (dataset->label[0] == '\0')
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    memset(block, 0, sizeof(RBRGen4DatasetMetaBlock));

    RBR_TRY(RBRGen4_converse(conn,
                                       "dataset %s/meta",
                                       dataset->label));

    RBRGen4Dataset_parseBlockResponse(conn,
                                                &block->byteCount,
                                                NULL,
                                                NULL);

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4Dataset_getScheduleBlock(
    RBRGen4 *conn,
    const RBRGen4Dataset *dataset,
    const char *scheduleLabel,
    RBRGen4DatasetScheduleBlock *block)
{
    if (dataset->label[0] == '\0' || scheduleLabel[0] == '\0')
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    memset(block, 0, sizeof(RBRGen4DatasetScheduleBlock));

    RBR_TRY(RBRGen4_converse(conn,
                                       "dataset %s/%s",
                                       dataset->label,
                                       scheduleLabel));

    RBRGen4Dataset_parseBlockResponse(conn,
                                                &block->byteCount,
                                                NULL,
                                                NULL);

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4Dataset_getScheduleEventsBlock(
    RBRGen4 *conn,
    const RBRGen4Dataset *dataset,
    const char *scheduleLabel,
    RBRGen4DatasetEventsBlock *block)
{
    if (dataset->label[0] == '\0' || scheduleLabel[0] == '\0')
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    memset(block, 0, sizeof(RBRGen4DatasetEventsBlock));

    RBR_TRY(RBRGen4_converse(conn,
                                       "dataset %s/%s/events",
                                       dataset->label,
                                       scheduleLabel));

    RBRGen4Dataset_parseBlockResponse(conn,
                                                &block->byteCount,
                                                NULL,
                                                &block->eventCount);

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4Dataset_getScheduleDataBlock(
    RBRGen4 *conn,
    const RBRGen4Dataset *dataset,
    const char *scheduleLabel,
    RBRGen4DatasetDataBlock *block)
{
    if (dataset->label[0] == '\0' || scheduleLabel[0] == '\0')
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    memset(block, 0, sizeof(RBRGen4DatasetDataBlock));

    RBR_TRY(RBRGen4_converse(conn,
                                       "dataset %s/%s/data",
                                       dataset->label,
                                       scheduleLabel));

    RBRGen4Dataset_parseBlockResponse(conn,
                                                &block->byteCount,
                                                &block->sampleCount,
                                                NULL);

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_deleteDataset(
    RBRGen4 *conn,
    const char *label)
{
    if (label[0] == '\0')
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGen4_converse(conn,
                                      "dataset delete %s",
                                      label);
}

RBRGen4Error RBRGen4_deleteDatasetAll(
    RBRGen4 *conn)
{
    return RBRGen4_converse(conn, "dataset delete all");
}

const char *RBRGen4DownloadDataUnit_name(
    RBRGen4DownloadDataUnit unit)
{
    switch (unit)
    {
    case RBRGEN4_DOWNLOAD_DATA_UNIT_BYTES:
        return "bytes";
    case RBRGEN4_DOWNLOAD_DATA_UNIT_SAMPLES:
        return "samples";
    case RBRGEN4_DOWNLOAD_DATA_UNIT_COUNT:
        return "download data unit count";
    case RBRGEN4_UNKNOWN_DOWNLOAD_DATA_UNIT:
    default:
        return "unknown download data unit";
    }
}

const char *RBRGen4DownloadEventsUnit_name(
    RBRGen4DownloadEventsUnit unit)
{
    switch (unit)
    {
    case RBRGEN4_DOWNLOAD_EVENTS_UNIT_BYTES:
        return "bytes";
    case RBRGEN4_DOWNLOAD_EVENTS_UNIT_EVENTS:
        return "events";
    case RBRGEN4_DOWNLOAD_EVENTS_UNIT_COUNT:
        return "download events unit count";
    case RBRGEN4_UNKNOWN_DOWNLOAD_EVENTS_UNIT:
    default:
        return "unknown download events unit";
    }
}

/**
 * \brief Keep retrying reads until we retrieve a fixed amount of data.
 *
 * This function first drains data out of RBRGen4.responseBuffer,
 * then begins to read from the instrument. As a result, \a data must not be
 * RBRGen4.responseBuffer!
 *
 * \param [in] conn the instrument connection
 * \param [out] data the buffer to write into
 * \param [in] size the amount of data to write into the buffer
 */
static RBRGen4Error RBRGen4_fixedRead(
    struct RBRGen4 *conn,
    void *data,
    int64_t size)
{
    int64_t bufferLength = 0;
    int32_t readLength;

    /* Can we steal from the response buffer? */
    if (conn->lastResponseLength < conn->responseBufferLength)
    {
        readLength = conn->responseBufferLength
                     - conn->lastResponseLength;
        if (readLength > size)
        {
            readLength = (int32_t) size;
        }

        memcpy(data,
               ((uint8_t *) conn->responseBuffer)
               + conn->lastResponseLength,
               readLength);

        bufferLength = readLength;
        conn->lastResponseLength += readLength;
    }

    /* Now poll the instrument. */
    while (bufferLength < size)
    {
        readLength = size - bufferLength > INT32_MAX
                     ? INT32_MAX
                     : (int32_t) (size - bufferLength);

        RBR_TRY(conn->callbacks.read(
                    conn,
                    ((uint8_t *) data) + bufferLength,
                    &readLength));

        bufferLength += readLength;
    }

    return RBRGEN4_SUCCESS;
}

/**
 * \brief Perform a download: send the command, parse its echo, read the
 * binary transfer, and verify its trailing CRC.
 *
 * The echo always reports `bytecount`; when \a countKey is not `bytecount`,
 * the count in the requested unit is reported under \a countKey as well.
 *
 * \param [in] conn the instrument connection
 * \param [in] countKey the count key sent with the command
 * \param [in,out] count the requested amount; updated to the amount reported
 * \param [out] byteCount the byte count reported
 * \param [out] data the buffer receiving the transfer
 * \param [in] dataSize the capacity of \a data in bytes
 */
static RBRGen4Error RBRGen4Dataset_downloadCommon(
    RBRGen4 *conn,
    const char *countKey,
    int64_t *count,
    int64_t *byteCount,
    void *data,
    int64_t dataSize)
{
    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(conn,
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
                rest > (int64_t) sizeof(conn->commandBuffer)
                ? (int64_t) sizeof(conn->commandBuffer)
                : rest;
            RBR_TRY(RBRGen4_fixedRead(conn,
                                                conn->commandBuffer,
                                                chunk));
            rest -= chunk;
        }
        return RBRGEN4_BUFFER_TOO_SMALL;
    }

    RBR_TRY(RBRGen4_fixedRead(conn, data, *byteCount));

    /* The instrument transmits the 16-bit CRC most significant byte first. */
    uint8_t crc[2];
    RBR_TRY(RBRGen4_fixedRead(conn, crc, 2));

    uint16_t reportedCrc = (uint16_t) ((crc[0] << 8) | crc[1]);
    if (RBRGen4_calculateCrc(data, *byteCount) != reportedCrc)
    {
        return RBRGEN4_CHECKSUM_ERROR;
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4Dataset_downloadScheduleData(
    RBRGen4 *conn,
    const RBRGen4Dataset *dataset,
    const char *scheduleLabel,
    RBRGen4DownloadData *download)
{
    if (dataset->label[0] == '\0'
        || scheduleLabel[0] == '\0'
        || (download->unit != RBRGEN4_DOWNLOAD_DATA_UNIT_BYTES
            && download->unit != RBRGEN4_DOWNLOAD_DATA_UNIT_SAMPLES)
        || download->count < 0
        || download->start < 0
        || download->data == NULL)
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    bool bytes = download->unit == RBRGEN4_DOWNLOAD_DATA_UNIT_BYTES;
    const char *countKey = bytes ? "bytecount" : "samplecount";

    download->byteCount = 0;

    RBR_TRY(RBRGen4_converse(conn,
                                       "download %s/%s/data"
                                       " %s=%" PRId64 " %s=%" PRId64,
                                       dataset->label,
                                       scheduleLabel,
                                       countKey,
                                       download->count,
                                       bytes ? "bytestart" : "samplestart",
                                       download->start));

    RBR_TRY(RBRGen4Dataset_downloadCommon(
                conn,
                countKey,
                &download->count,
                &download->byteCount,
                download->data,
                download->dataSize));

    return RBRGEN4_SUCCESS;
}

/**
 * \brief Perform an events download from the whole dataset (\a scheduleLabel
 * NULL) or from one schedule.
 *
 * \param [in] conn the instrument connection
 * \param [in] dataset the dataset, selected by its label
 * \param [in] scheduleLabel the schedule, or NULL for the whole dataset
 * \param [in,out] download the download request and its result
 */
static RBRGen4Error RBRGen4Dataset_downloadEventsCommon(
    RBRGen4 *conn,
    const RBRGen4Dataset *dataset,
    const char *scheduleLabel,
    RBRGen4DownloadEvents *download)
{
    if (dataset->label[0] == '\0'
        || (download->unit != RBRGEN4_DOWNLOAD_EVENTS_UNIT_BYTES
            && download->unit
               != RBRGEN4_DOWNLOAD_EVENTS_UNIT_EVENTS)
        || download->count < 0
        || download->start < 0
        || download->data == NULL)
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    bool bytes
        = download->unit == RBRGEN4_DOWNLOAD_EVENTS_UNIT_BYTES;
    const char *countKey = bytes ? "bytecount" : "eventcount";

    download->byteCount = 0;

    if (scheduleLabel == NULL)
    {
        RBR_TRY(RBRGen4_converse(conn,
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
        RBR_TRY(RBRGen4_converse(conn,
                                           "download %s/%s/events"
                                           " %s=%" PRId64 " %s=%" PRId64,
                                           dataset->label,
                                           scheduleLabel,
                                           countKey,
                                           download->count,
                                           bytes ? "bytestart" : "eventstart",
                                           download->start));
    }

    RBR_TRY(RBRGen4Dataset_downloadCommon(
                conn,
                countKey,
                &download->count,
                &download->byteCount,
                download->data,
                download->dataSize));

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4Dataset_downloadEvents(
    RBRGen4 *conn,
    const RBRGen4Dataset *dataset,
    RBRGen4DownloadEvents *download)
{
    return RBRGen4Dataset_downloadEventsCommon(conn,
                                                       dataset,
                                                       NULL,
                                                       download);
}

RBRGen4Error RBRGen4Dataset_downloadScheduleEvents(
    RBRGen4 *conn,
    const RBRGen4Dataset *dataset,
    const char *scheduleLabel,
    RBRGen4DownloadEvents *download)
{
    if (scheduleLabel[0] == '\0')
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGen4Dataset_downloadEventsCommon(conn,
                                                       dataset,
                                                       scheduleLabel,
                                                       download);
}

RBRGen4Error RBRGen4Dataset_downloadMeta(
    RBRGen4 *conn,
    const RBRGen4Dataset *dataset,
    RBRGen4DownloadMeta *download)
{
    if (dataset->label[0] == '\0'
        || download->byteCount < 0
        || download->byteStart < 0
        || download->data == NULL)
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    RBR_TRY(RBRGen4_converse(conn,
                                       "download %s/meta"
                                       " bytecount=%" PRId64
                                       " bytestart=%" PRId64,
                                       dataset->label,
                                       download->byteCount,
                                       download->byteStart));

    int64_t byteCount = 0;
    RBR_TRY(RBRGen4Dataset_downloadCommon(conn,
                                                    "bytecount",
                                                    &download->byteCount,
                                                    &byteCount,
                                                    download->data,
                                                    download->dataSize));

    return RBRGEN4_SUCCESS;
}

/* CRC-CCITT */
uint16_t RBRGen4_calculateCrc(const void *data, int64_t size)
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
