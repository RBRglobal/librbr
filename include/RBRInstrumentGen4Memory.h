/**
 * \file RBRInstrumentGen4Memory.h
 *
 * \brief Instrument commands and structures pertaining to memory and data
 * retrieval.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/memory-and-data-retrieval
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTGEN4MEMORY_H
#define LIBRBR_RBRINSTRUMENTGEN4MEMORY_H

#include "RBRGen4.h"
#include "RBRInstrumentGen4Configuration.h"
#ifdef __cplusplus
extern "C" {
#endif

/** \brief The maximum number of datasets the library can enumerate. */
#define RBRINSTRUMENTGEN4_DATASET_COUNT_MAX 32

/**
 * \brief Possible storage access modes for the instrument's data memory.
 *
 * \see RBRInstrumentGen4Storage
 */
typedef enum RBRInstrumentGen4StorageAccess
{
    /** The instrument currently has access to its own data memory. */
    RBRINSTRUMENTGEN4_STORAGE_ACCESS_INSTRUMENT,
    /**
     * A USB host currently has access to the instrument's data memory as
     * a mass storage device.
     */
    RBRINSTRUMENTGEN4_STORAGE_ACCESS_USBHOST,
    /** The number of storage access modes. */
    RBRINSTRUMENTGEN4_STORAGE_ACCESS_COUNT,
    /** An unknown or unrecognized storage access mode. */
    RBRINSTRUMENTGEN4_UNKNOWN_STORAGE_ACCESS
} RBRInstrumentGen4StorageAccess;

/**
 * \brief Get a human-readable string name for a storage access mode.
 *
 * \param [in] access the storage access mode
 * \return a string name for the storage access mode
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4StorageAccess_name(
    RBRInstrumentGen4StorageAccess access);

/**
 * \brief Instrument `storage` command parameters.
 *
 * \see RBRInstrumentGen4_getStorage()
 * \see RBRInstrumentGen4_setStorage()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828279/storage
 */
typedef struct RBRInstrumentGen4Storage
{
    /**
     * \brief The number of bytes used for storage.
     * \readonly
     */
    int64_t used;
    /**
     * \brief The number of bytes still available for storage.
     * \readonly
     */
    int64_t remaining;
    /**
     * \brief The maximum total size of the memory in bytes.
     * \readonly
     */
    int64_t size;
    /** \brief The storage access mode of the instrument's data memory. */
    RBRInstrumentGen4StorageAccess access;
} RBRInstrumentGen4Storage;

/**
 * \brief Get information about the usage and characteristics of data memory.
 *
 * \note Issues the `storage` command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] storage data memory information
 * \return #RBRGEN4_SUCCESS when the parameters are successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_setStorage()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828279/storage
 */
RBRGen4Error RBRInstrumentGen4_getStorage(
    RBRGen4 *instrument,
    RBRInstrumentGen4Storage *storage);

/**
 * \brief Set the instrument storage parameters.
 *
 * Sends `access`, the command's only writable parameter.
 *
 * \note Issues the `storage` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] storage the storage parameters to write
 * \return #RBRGEN4_SUCCESS when the parameters are successfully
 *                                    written
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the storage access
 *                                                    mode is invalid
 * \see RBRInstrumentGen4_getStorage()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828279/storage
 */
RBRGen4Error RBRInstrumentGen4_setStorage(
    RBRGen4 *instrument,
    const RBRInstrumentGen4Storage *storage);

/**
 * \brief Possible dataset statuses.
 *
 * \see RBRInstrumentGen4Dataset
 */
typedef enum RBRInstrumentGen4DatasetStatus
{
    /** The dataset is for a deployment currently in progress. */
    RBRINSTRUMENTGEN4_DATASET_STATUS_OPEN,
    /** The dataset is for a deployment that has ended. */
    RBRINSTRUMENTGEN4_DATASET_STATUS_CLOSED,
    /** The number of specific dataset statuses. */
    RBRINSTRUMENTGEN4_DATASET_STATUS_COUNT,
    /** An unknown or unrecognized dataset status. */
    RBRINSTRUMENTGEN4_UNKNOWN_DATASET_STATUS
} RBRInstrumentGen4DatasetStatus;

/**
 * \brief Get a human-readable string name for a dataset status.
 *
 * \param [in] status the dataset status
 * \return a string name for the dataset status
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4DatasetStatus_name(
    RBRInstrumentGen4DatasetStatus status);

/**
 * \brief `dataset <dataset_label>` command parameters.
 *
 * \see RBRInstrumentGen4_getDataset()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890208/dataset
 */
typedef struct RBRInstrumentGen4Dataset
{
    /**
     * \brief The dataset's label.
     *
     * Set by the caller to select the dataset to read.
     */
    char label[RBRGEN4_LABEL_NAME_MAX + 1];

    /** \brief Whether the dataset's deployment is still in progress. */
    RBRInstrumentGen4DatasetStatus status;

    /** \brief The total memory usage of the dataset in bytes. */
    int64_t byteCount;

    /** \brief The numerical format used for data storage in this dataset. */
    RBRGen4DataType dataType;
} RBRInstrumentGen4Dataset;

/**
 * \brief `dataset` command parameters. The `list` is stored in a user provided
 * buffer (#pool).
 *
 * \see RBRInstrumentGen4_getDatasetPool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890208/dataset
 */
typedef struct RBRInstrumentGen4DatasetPool
{
    /** \brief The number of datasets #pool can hold. */
    int32_t size;

    /**
     * \brief The number of datasets stored in the instrument's memory.
     *
     * \warning This field will be larger than #size when
     * #RBRGEN4_TRUNCATED is returned by the getter. Care should be
     * taken to avoid out-of-bounds access when iterating over #pool.
     */
    int32_t count;

    /**
     * \brief The maximum number of datasets that the instrument can store
     * in its memory.
     */
    int32_t maxCount;

    /** \brief User provided buffer of the datasets stored. */
    RBRInstrumentGen4Dataset *pool;
} RBRInstrumentGen4DatasetPool;

/**
 * \brief Populate the pool of datasets with the labels of the datasets
 * stored in the instrument's memory.
 *
 * Only the labels are populated: read the remaining parameters of a pool
 * entry with RBRInstrumentGen4_getDataset().
 *
 * \note Issues the `dataset` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in,out] datasetPool the datasets in storage, labels only
 * \return #RBRGEN4_SUCCESS when the parameters are successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_TRUNCATED when \a datasetPool cannot hold every
 *                                      reported dataset; the first `size` are
 *                                      stored, and `count` is set to the value
 *                                      reported by the instrument which WILL
 *                                      exceed `size`
 * \see RBRInstrumentGen4_getDataset()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890208/dataset
 */
RBRGen4Error RBRInstrumentGen4_getDatasetPool(
    RBRGen4 *instrument,
    RBRInstrumentGen4DatasetPool *datasetPool);

/**
 * \brief Populate the parameters of a dataset.
 *
 * The caller sets RBRInstrumentGen4Dataset.label to select the dataset.
 *
 * \note Issues the `dataset <dataset_label>` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in,out] dataset the dataset to read, selected by its label
 * \param [out] scheduleList the schedules run by the dataset, or `NULL` to
 *                           skip them
 * \return #RBRGEN4_SUCCESS when the parameters are successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the label is empty
 * \return #RBRGEN4_TRUNCATED when \a scheduleList cannot hold every
 *                                      reported schedule; the first `size` are
 *                                      stored, and `count` is set to the value
 *                                      reported by the instrument which WILL
 *                                      exceed `size`
 * \return #RBRGEN4_HARDWARE_ERROR when the dataset does not exist
 * \see RBRInstrumentGen4_getDatasetPool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890208/dataset
 */
RBRGen4Error RBRInstrumentGen4_getDataset(
    RBRGen4 *instrument,
    RBRInstrumentGen4Dataset *dataset,
    RBRGen4LabelList *scheduleList);

/**
 * \brief `dataset <dataset_label>/[<schedule_label>/]events` command
 * parameters.
 *
 * \see RBRInstrumentGen4Dataset_getEventsBlock()
 * \see RBRInstrumentGen4Dataset_getScheduleEventsBlock()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890208/dataset
 */
typedef struct RBRInstrumentGen4DatasetEventsBlock
{
    /** \brief The memory usage of the events block in bytes. */
    int64_t byteCount;
    /** \brief The memory usage of the events block in events. */
    int64_t eventCount;
} RBRInstrumentGen4DatasetEventsBlock;

/**
 * \brief `dataset <dataset_label>/meta` command parameters.
 *
 * \see RBRInstrumentGen4Dataset_getMetaBlock()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890208/dataset
 */
typedef struct RBRInstrumentGen4DatasetMetaBlock
{
    /** \brief The memory usage of the metadata block in bytes. */
    int64_t byteCount;
} RBRInstrumentGen4DatasetMetaBlock;

/**
 * \brief `dataset <dataset_label>/<schedule_label>` command parameters.
 *
 * \see RBRInstrumentGen4Dataset_getScheduleBlock()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890208/dataset
 */
typedef struct RBRInstrumentGen4DatasetScheduleBlock
{
    /** \brief The memory usage of the schedule's blocks in bytes. */
    int64_t byteCount;
} RBRInstrumentGen4DatasetScheduleBlock;

/**
 * \brief `dataset <dataset_label>/<schedule_label>/data` command parameters.
 *
 * \see RBRInstrumentGen4Dataset_getScheduleDataBlock()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890208/dataset
 */
typedef struct RBRInstrumentGen4DatasetDataBlock
{
    /** \brief The memory usage of the sample data block in bytes. */
    int64_t byteCount;
    /** \brief The memory usage of the sample data block in samples. */
    int64_t sampleCount;
} RBRInstrumentGen4DatasetDataBlock;

/**
 * \brief Get the memory usage of all of a dataset's events, including
 * those not tied to any schedule.
 *
 * \note Issues the `dataset <dataset_label>/events` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] dataset the dataset, selected by its label
 * \param [out] block the memory usage of the events block
 * \return #RBRGEN4_SUCCESS when the parameters are successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the label is empty
 * \return #RBRGEN4_HARDWARE_ERROR when the dataset does not exist
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890208/dataset
 */
RBRGen4Error RBRInstrumentGen4Dataset_getEventsBlock(
    RBRGen4 *instrument,
    const RBRInstrumentGen4Dataset *dataset,
    RBRInstrumentGen4DatasetEventsBlock *block);

/**
 * \brief Get the memory usage of all of a dataset's metadata.
 *
 * \note Issues the `dataset <dataset_label>/meta` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] dataset the dataset, selected by its label
 * \param [out] block the memory usage of the metadata block
 * \return #RBRGEN4_SUCCESS when the parameters are successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the label is empty
 * \return #RBRGEN4_HARDWARE_ERROR when the dataset does not exist
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890208/dataset
 */
RBRGen4Error RBRInstrumentGen4Dataset_getMetaBlock(
    RBRGen4 *instrument,
    const RBRInstrumentGen4Dataset *dataset,
    RBRInstrumentGen4DatasetMetaBlock *block);

/**
 * \brief Get the memory usage of one of a dataset's schedules, summed over
 * all of its block types.
 *
 * \note Issues the `dataset <dataset_label>/<schedule_label>` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] dataset the dataset, selected by its label
 * \param [in] scheduleLabel the schedule, as listed by
 *                           RBRInstrumentGen4_getDataset()
 * \param [out] block the memory usage of the schedule's blocks
 * \return #RBRGEN4_SUCCESS when the parameters are successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when either label is
 *                                                    empty
 * \return #RBRGEN4_HARDWARE_ERROR when the dataset or schedule does
 *                                           not exist
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890208/dataset
 */
RBRGen4Error RBRInstrumentGen4Dataset_getScheduleBlock(
    RBRGen4 *instrument,
    const RBRInstrumentGen4Dataset *dataset,
    const char *scheduleLabel,
    RBRInstrumentGen4DatasetScheduleBlock *block);

/**
 * \brief Get the memory usage of one schedule's events within a dataset.
 *
 * \note Issues the `dataset <dataset_label>/<schedule_label>/events`
 * command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] dataset the dataset, selected by its label
 * \param [in] scheduleLabel the schedule, as listed by
 *                           RBRInstrumentGen4_getDataset()
 * \param [out] block the memory usage of the schedule's events block
 * \return #RBRGEN4_SUCCESS when the parameters are successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when either label is
 *                                                    empty
 * \return #RBRGEN4_HARDWARE_ERROR when the dataset or schedule does
 *                                           not exist
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890208/dataset
 */
RBRGen4Error RBRInstrumentGen4Dataset_getScheduleEventsBlock(
    RBRGen4 *instrument,
    const RBRInstrumentGen4Dataset *dataset,
    const char *scheduleLabel,
    RBRInstrumentGen4DatasetEventsBlock *block);

/**
 * \brief Get the memory usage of one schedule's sample data within a
 * dataset.
 *
 * \note Issues the `dataset <dataset_label>/<schedule_label>/data` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] dataset the dataset, selected by its label
 * \param [in] scheduleLabel the schedule, as listed by
 *                           RBRInstrumentGen4_getDataset()
 * \param [out] block the memory usage of the schedule's sample data block
 * \return #RBRGEN4_SUCCESS when the parameters are successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when either label is
 *                                                    empty
 * \return #RBRGEN4_HARDWARE_ERROR when the dataset or schedule does
 *                                           not exist
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890208/dataset
 */
RBRGen4Error RBRInstrumentGen4Dataset_getScheduleDataBlock(
    RBRGen4 *instrument,
    const RBRInstrumentGen4Dataset *dataset,
    const char *scheduleLabel,
    RBRInstrumentGen4DatasetDataBlock *block);

/** \brief It determines the type of information retrieved for the specific schedule. 
 * There are three keywoards: data|events|meta.
 * \see https://docs.rbr-global.com/L3commandreference/commands/memory-and-data-retrieval/dataset
*/
typedef enum RBRInstrumentGen4Block{
    /** Used to report memory usage for sample data. */
    RBRINSTRUMENTGEN4_BLOCK_DATA,
    /** Used to report memory usege for events. */
    RBRINSTRUMENTGEN4_BLOCK_EVENTS,
    /** Used to report this schedule's meory usage for metadata. */
    RBRINSTRUMENTGEN4_BLOCK_META,
    /** The number of specific type of blocks.*/
    RBRINSTRUMENTGEN4_BLOCK_COUNT,
    /** The unknown or unrecognized block. */
    RBRINSTRUMENTGEN4_BLOCK_UNKNOWN
}RBRInstrumentGen4Block;

/** \brief Get a human-readable block name.
 * \param [in] block the block.
 * \return a string name for the block.
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4Block_name(RBRInstrumentGen4Block block);

/**
 * \brief Delete one dataset from the instrument's memory.
 * \note Issues the `dataset delete <dataset_label>` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] label the label of the dataset to delete
 * \return #RBRGEN4_SUCCESS when the dataset is deleted
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the label is empty
 * \return #RBRGEN4_HARDWARE_ERROR when the dataset does not exist
 * \see RBRInstrumentGen4_deleteDatasetAll()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890208/dataset
 */
RBRGen4Error RBRInstrumentGen4_deleteDataset(
    RBRGen4 *instrument,
    const char *label);

/**
 * \brief Delete every dataset from the instrument's memory.
 *
 * \note Issues the `dataset delete all` command.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRGEN4_SUCCESS when the datasets are deleted
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_deleteDataset()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890208/dataset
 */
RBRGen4Error RBRInstrumentGen4_deleteDatasetAll(
    RBRGen4 *instrument);

/**
 * \brief The unit in which a sample data download's count and start offset
 * are measured.
 *
 * \see RBRInstrumentGen4DownloadData
 */
typedef enum RBRInstrumentGen4DownloadDataUnit
{
    /** The transfer is measured in bytes (`bytecount`/`bytestart`). */
    RBRINSTRUMENTGEN4_DOWNLOAD_DATA_UNIT_BYTES,
    /** The transfer is measured in samples (`samplecount`/`samplestart`). */
    RBRINSTRUMENTGEN4_DOWNLOAD_DATA_UNIT_SAMPLES,
    /** The number of sample data download units. */
    RBRINSTRUMENTGEN4_DOWNLOAD_DATA_UNIT_COUNT,
    /** An unknown or unrecognized sample data download unit. */
    RBRINSTRUMENTGEN4_UNKNOWN_DOWNLOAD_DATA_UNIT
} RBRInstrumentGen4DownloadDataUnit;

/**
 * \brief Get a human-readable string name for a sample data download unit.
 *
 * \param [in] unit the download unit
 * \return a string name for the download unit
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4DownloadDataUnit_name(
    RBRInstrumentGen4DownloadDataUnit unit);

/**
 * \brief The unit in which an events download's count and start offset are
 * measured.
 *
 * \see RBRInstrumentGen4DownloadEvents
 */
typedef enum RBRInstrumentGen4DownloadEventsUnit
{
    /** The transfer is measured in bytes (`bytecount`/`bytestart`). */
    RBRINSTRUMENTGEN4_DOWNLOAD_EVENTS_UNIT_BYTES,
    /** The transfer is measured in events (`eventcount`/`eventstart`). */
    RBRINSTRUMENTGEN4_DOWNLOAD_EVENTS_UNIT_EVENTS,
    /** The number of events download units. */
    RBRINSTRUMENTGEN4_DOWNLOAD_EVENTS_UNIT_COUNT,
    /** An unknown or unrecognized events download unit. */
    RBRINSTRUMENTGEN4_UNKNOWN_DOWNLOAD_EVENTS_UNIT
} RBRInstrumentGen4DownloadEventsUnit;

/**
 * \brief Get a human-readable string name for an events download unit.
 *
 * \param [in] unit the download unit
 * \return a string name for the download unit
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4DownloadEventsUnit_name(
    RBRInstrumentGen4DownloadEventsUnit unit);

/**
 * \brief `download <dataset_label>/<schedule_label>/data` command
 * parameters.
 *
 * \see RBRInstrumentGen4Dataset_downloadScheduleData()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830106/download
 */
typedef struct RBRInstrumentGen4DownloadData
{
    /** \brief The unit for both #count and #start. */
    RBRInstrumentGen4DownloadDataUnit unit;

    /**
     * \brief The amount of data to transfer, in #unit: the requested
     * amount, updated to the amount the instrument reports.
     */
    int64_t count;

    /** \brief The offset to begin reading from, in #unit, from 0. */
    int64_t start;

    /**
     * \brief The number of bytes transferred, excluding the trailing CRC.
     * \readonly
     */
    int64_t byteCount;

    /** \brief The caller-provided buffer receiving the transferred data. */
    void *data;

    /**
     * \brief The capacity of #data in bytes. Nothing is written to #data
     * beyond it.
     */
    int64_t dataSize;
} RBRInstrumentGen4DownloadData;

/**
 * \brief `download <dataset_label>[/<schedule_label>]/events` command
 * parameters.
 *
 * \see RBRInstrumentGen4Dataset_downloadEvents()
 * \see RBRInstrumentGen4Dataset_downloadScheduleEvents()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830106/download
 */
typedef struct RBRInstrumentGen4DownloadEvents
{
    /** \brief The unit for both #count and #start. */
    RBRInstrumentGen4DownloadEventsUnit unit;

    /**
     * \brief The amount of data to transfer, in #unit: the requested
     * amount, updated to the amount the instrument reports.
     */
    int64_t count;

    /** \brief The offset to begin reading from, in #unit, from 0. */
    int64_t start;

    /**
     * \brief The number of bytes transferred, excluding the trailing CRC.
     * \readonly
     */
    int64_t byteCount;

    /** \brief The caller-provided buffer receiving the transferred data. */
    void *data;

    /**
     * \brief The capacity of #data in bytes. Nothing is written to #data
     * beyond it.
     */
    int64_t dataSize;
} RBRInstrumentGen4DownloadEvents;

/**
 * \brief `download <dataset_label>/meta` command parameters.
 *
 * \see RBRInstrumentGen4Dataset_downloadMeta()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830106/download
 */
typedef struct RBRInstrumentGen4DownloadMeta
{
    /**
     * \brief The number of bytes to transfer: the requested amount,
     * updated to the amount the instrument reports. Excludes the trailing
     * CRC.
     */
    int64_t byteCount;

    /** \brief The offset to begin reading from, in bytes, from 0. */
    int64_t byteStart;

    /** \brief The caller-provided buffer receiving the transferred data. */
    void *data;

    /**
     * \brief The capacity of #data in bytes. Nothing is written to #data
     * beyond it.
     */
    int64_t dataSize;
} RBRInstrumentGen4DownloadMeta;

/**
 * \brief Download part of one schedule's sample data within a dataset.
 *
 * The transfer is checked against its trailing CRC before returning.
 *
 * \note Issues the `download <dataset_label>/<schedule_label>/data`
 * command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] dataset the dataset, selected by its label
 * \param [in] scheduleLabel the schedule, as listed by
 *                           RBRInstrumentGen4_getDataset()
 * \param [in,out] download the download request: the caller populates the
 *                         unit, count, offset, and buffer fields to say what
 *                         to transfer and where to put it; the counts are
 *                         updated with what the instrument returned
 * \return #RBRGEN4_SUCCESS when the data is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when a label is empty,
 *                                                    the unit is invalid, or
 *                                                    the count or offset is
 *                                                    negative
 * \return #RBRGEN4_BUFFER_TOO_SMALL when the response byte count
 *                                             exceeds the buffer capacity
 * \return #RBRGEN4_CHECKSUM_ERROR when the transfer fails its CRC
 *                                           check
 * \return #RBRGEN4_HARDWARE_ERROR when the dataset or schedule
 *                                           does not exist
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830106/download
 */
RBRGen4Error RBRInstrumentGen4Dataset_downloadScheduleData(
    RBRGen4 *instrument,
    const RBRInstrumentGen4Dataset *dataset,
    const char *scheduleLabel,
    RBRInstrumentGen4DownloadData *download);

/**
 * \brief Download a dataset's events, including those not tied to any
 * schedule.
 *
 * \note Issues the `download <dataset_label>/events` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] dataset the dataset, selected by its label
 * \param [in,out] download the download request: the caller populates the
 *                         unit, count, offset, and buffer fields to say what
 *                         to transfer and where to put it; the counts are
 *                         updated with what the instrument returned
 * \return #RBRGEN4_SUCCESS when the data is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the label is
 *                                                    empty, the unit is
 *                                                    invalid, or the count
 *                                                    or offset is negative
 * \return #RBRGEN4_BUFFER_TOO_SMALL when the response byte count
 *                                             exceeds the buffer capacity
 * \return #RBRGEN4_CHECKSUM_ERROR when the transfer fails its CRC
 *                                           check
 * \return #RBRGEN4_HARDWARE_ERROR when the dataset does not exist
 * \see RBRInstrumentGen4Dataset_downloadScheduleEvents()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830106/download
 */
RBRGen4Error RBRInstrumentGen4Dataset_downloadEvents(
    RBRGen4 *instrument,
    const RBRInstrumentGen4Dataset *dataset,
    RBRInstrumentGen4DownloadEvents *download);

/**
 * \brief Download part of one schedule's events within a dataset.
 *
 * \note Issues the `download <dataset_label>/<schedule_label>/events`
 * command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] dataset the dataset, selected by its label
 * \param [in] scheduleLabel the schedule, as listed by
 *                           RBRInstrumentGen4_getDataset()
 * \param [in,out] download the download request: the caller populates the
 *                         unit, count, offset, and buffer fields to say what
 *                         to transfer and where to put it; the counts are
 *                         updated with what the instrument returned
 * \return #RBRGEN4_SUCCESS when the data is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when a label is empty,
 *                                                    the unit is invalid, or
 *                                                    the count or offset is
 *                                                    negative
 * \return #RBRGEN4_BUFFER_TOO_SMALL when the response byte count
 *                                             exceeds the buffer capacity
 * \return #RBRGEN4_CHECKSUM_ERROR when the transfer fails its CRC
 *                                           check
 * \return #RBRGEN4_HARDWARE_ERROR when the dataset or schedule
 *                                           does not exist
 * \see RBRInstrumentGen4Dataset_downloadEvents()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830106/download
 */
RBRGen4Error RBRInstrumentGen4Dataset_downloadScheduleEvents(
    RBRGen4 *instrument,
    const RBRInstrumentGen4Dataset *dataset,
    const char *scheduleLabel,
    RBRInstrumentGen4DownloadEvents *download);

/**
 * \brief Download a dataset's metadata.
 *
 * \note Issues the `download <dataset_label>/meta` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] dataset the dataset, selected by its label
 * \param [in,out] download the download request: the caller populates the
 *                         count, offset, and buffer fields to say what to
 *                         transfer and where to put it; the count is updated
 *                         with what the instrument returned
 * \return #RBRGEN4_SUCCESS when the data is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the label is
 *                                                    empty or the count or
 *                                                    offset is negative
 * \return #RBRGEN4_BUFFER_TOO_SMALL when the response byte count
 *                                             exceeds the buffer capacity
 * \return #RBRGEN4_CHECKSUM_ERROR when the transfer fails its CRC
 *                                           check
 * \return #RBRGEN4_HARDWARE_ERROR when the dataset does not exist
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830106/download
 */
RBRGen4Error RBRInstrumentGen4Dataset_downloadMeta(
    RBRGen4 *instrument,
    const RBRInstrumentGen4Dataset *dataset,
    RBRInstrumentGen4DownloadMeta *download);

/**
 * \brief Calculate the 16-bit CRC using the CCITT polynomial f(x)=x^16+x^12+x^5+1
 * feeding bytes into the generator LSB first and using 0xFFFF as a seed value,
 * is then transmitted.
 *
 * \param [in] data the data string used to calculate the CRC
 * \param [in] size the number of characters in the string used to calculate the CRC
 * \return calculated CRC
 * \see https://docs.rbr-global.com/L3commandreference/commands/memory-and-data-retrieval/postprocessing
 */
uint16_t RBRInstrumentGen4_calculateCrc(
    const void *data,
    int64_t size);   

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTGEN4MEMORY_H */
