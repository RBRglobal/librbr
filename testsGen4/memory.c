/**
 * \file memory.c
 *
 * \brief Tests for instrument memory commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for PRId64. */
#include <inttypes.h>

#include "tests.h"
#include "RBRGen4Memory.h"

typedef struct GetStorageTest
{
    const char *response;
    RBRGen4Error expectedError;
    RBRGen4Storage expected;
} GetStorageTest;

TEST_LOGGER4(getStorage)
{
    GetStorageTest tests[] = {
        /* A bare query reports all four parameters. */
        { "storage used=15372 remaining=61016097780 size=61016113152"
          " access=instrument" RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS,
          { .used = 15372LL,
            .remaining = 61016097780LL,
            .size = 61016113152LL,
            .access = RBRGEN4_STORAGE_ACCESS_INSTRUMENT } },
        { "storage used=1528 remaining=134216192 size=134217728"
          " access=usbhost" RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS,
          { .used = 1528LL,
            .remaining = 134216192LL,
            .size = 134217728LL,
            .access = RBRGEN4_STORAGE_ACCESS_USBHOST } },
        /* An unreported parameter keeps its unset value. */
        { "storage used=15372" RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS,
          { .used = 15372LL,
            .remaining = 0LL,
            .size = 0LL,
            .access = RBRGEN4_UNKNOWN_STORAGE_ACCESS } },
        /* An unrecognized access location parses to the unknown member. */
        { "storage used=15372 remaining=61016097780 size=61016113152"
          " access=cloud" RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS,
          { .used = 15372LL,
            .remaining = 61016097780LL,
            .size = 61016113152LL,
            .access = RBRGEN4_UNKNOWN_STORAGE_ACCESS } },
        /* Keys the library does not model are ignored. */
        { "storage used=15372 remaining=61016097780 size=61016113152"
          " access=instrument bogus=1" RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS,
          { .used = 15372LL,
            .remaining = 61016097780LL,
            .size = 61016113152LL,
            .access = RBRGEN4_STORAGE_ACCESS_INSTRUMENT } },
        { NULL, 0, { 0, 0, 0, 0 } }
    };

    RBRGen4Error err;
    RBRGen4Storage actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4_getStorage(conn, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRGen4Error);
        TEST_ASSERT_STR_EQ("storage" COMMAND_TERMINATOR,
                           buffers->writeBuffer);
        TEST_ASSERT_EQ(tests[i].expected.used, actual.used, "%" PRId64);
        TEST_ASSERT_EQ(tests[i].expected.remaining,
                       actual.remaining,
                       "%" PRId64);
        TEST_ASSERT_EQ(tests[i].expected.size, actual.size, "%" PRId64);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.access,
                            actual.access,
                            RBRGen4StorageAccess);
    }

    return true;
}

typedef struct SetStorageTest
{
    RBRGen4StorageAccess access;
    const char *command;
    const char *response;
    RBRGen4Error expectedError;
} SetStorageTest;

TEST_LOGGER4(setStorage)
{
    SetStorageTest tests[] = {
        { RBRGEN4_STORAGE_ACCESS_INSTRUMENT,
          "storage access=instrument" COMMAND_TERMINATOR,
          "storage access=instrument" RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS },
        { RBRGEN4_STORAGE_ACCESS_USBHOST,
          "storage access=usbhost" COMMAND_TERMINATOR,
          "storage access=usbhost" RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS },
        /*
         * Selecting the location already in use is a warning, which the
         * library surfaces as a hardware error with the response type
         * distinguishing it.
         */
        { RBRGEN4_STORAGE_ACCESS_INSTRUMENT,
          "storage access=instrument" COMMAND_TERMINATOR,
          "WRN-305 storage access already at selected location"
          RESPONSE_TERMINATOR,
          RBRGEN4_HARDWARE_ERROR },
        /* Sentinel members are refused before the command. */
        { RBRGEN4_STORAGE_ACCESS_COUNT,
          "",
          "",
          RBRGEN4_INVALID_PARAMETER_VALUE },
        { RBRGEN4_UNKNOWN_STORAGE_ACCESS,
          "",
          "",
          RBRGEN4_INVALID_PARAMETER_VALUE },
        { 0, NULL, NULL, 0 }
    };

    RBRGen4Error err;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        RBRGen4Storage storage = {
            .access = tests[i].access
        };
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4_setStorage(conn, &storage);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }

    return true;
}

typedef struct GetDatasetPoolTest
{
    const char *response;
    RBRGen4Error expectedError;
    int32_t expectedCount;
    int32_t expectedMaxCount;
    const char *expectedLabels[3];
} GetDatasetPoolTest;

TEST_LOGGER4(getDatasetPool)
{
    GetDatasetPoolTest tests[] = {
        { "dataset count=3 maxcount=4 list=d1|d2|d5" RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS,
          3, 4, { "d1", "d2", "d5" } },
        { "dataset count=1 maxcount=20 list=DeepCove" RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS,
          1, 20, { "DeepCove" } },
        /* An empty pool reports `none`. */
        { "dataset count=0 maxcount=4 list=none" RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS,
          0, 4, { NULL } },
        /* Keys the library does not model are ignored. */
        { "dataset count=1 maxcount=4 list=d1 bogus=1" RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS,
          1, 4, { "d1" } },
        { NULL, 0, 0, 0, { NULL } }
    };

    RBRGen4Error err;
    RBRGEN4_DATASET_POOL_DECL(actual, 3);

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4_getDatasetPool(conn, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRGen4Error);
        TEST_ASSERT_STR_EQ("dataset" COMMAND_TERMINATOR,
                           buffers->writeBuffer);
        TEST_ASSERT_EQ(tests[i].expectedCount, actual.count, "%" PRId32);
        TEST_ASSERT_EQ(tests[i].expectedMaxCount,
                       actual.maxCount,
                       "%" PRId32);
        for (int32_t dataset = 0;
             dataset < actual.size && dataset < tests[i].expectedCount;
             dataset++)
        {
            TEST_ASSERT_STR_EQ(tests[i].expectedLabels[dataset],
                               actual.pool[dataset].label);
        }
    }

    /* A pool which cannot hold every dataset is truncated and reported. */
    RBRGen4DatasetPool shortPool = {
        .size = 2,
        .pool = actualBuffer
    };
    TestIOBuffers_init(buffers,
                       "dataset count=3 maxcount=4 list=d1|d2|d5"
                       RESPONSE_TERMINATOR,
                       0);
    err = RBRGen4_getDatasetPool(conn, &shortPool);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_TRUNCATED,
                        err,
                        RBRGen4Error);
    TEST_ASSERT_EQ(3, shortPool.count, "%" PRId32);
    TEST_ASSERT_STR_EQ("d1", shortPool.pool[0].label);
    TEST_ASSERT_STR_EQ("d2", shortPool.pool[1].label);

    return true;
}

typedef struct GetDatasetTest
{
    const char *response;
    RBRGen4Error expectedError;
    RBRGen4Dataset expected;
    int32_t expectedScheduleCount;
    const char *expectedScheduleList[2];
} GetDatasetTest;

TEST_LOGGER4(getDataset)
{
    GetDatasetTest tests[] = {
        { "dataset d1 status=closed schedulelist=s_cont bytecount=5604"
          " datatype=float64" RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS,
          { .label = "d1",
            .status = RBRGEN4_DATASET_STATUS_CLOSED,
            .byteCount = 5604LL,
            .dataType = RBRGEN4_DATA_TYPE_FLOAT64 },
          1,
          { "s_cont" } },
        { "dataset d1 status=open schedulelist=tides_schedule|DO_schedule"
          " bytecount=3749498 datatype=float32" RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS,
          { .label = "d1",
            .status = RBRGEN4_DATASET_STATUS_OPEN,
            .byteCount = 3749498LL,
            .dataType = RBRGEN4_DATA_TYPE_FLOAT32 },
          2,
          { "tides_schedule", "DO_schedule" } },
        /* Values the library does not model parse to the unknown members. */
        { "dataset d1 status=bogus schedulelist=s_cont bytecount=0"
          " datatype=bogus" RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS,
          { .label = "d1",
            .status = RBRGEN4_UNKNOWN_DATASET_STATUS,
            .byteCount = 0LL,
            .dataType = RBRGEN4_UNKNOWN_DATA_TYPE },
          1,
          { "s_cont" } },
        /* A dataset which does not exist is a hardware error. */
        { "ERR-304 dataset not found: 'd1'" RESPONSE_TERMINATOR,
          RBRGEN4_HARDWARE_ERROR,
          { .label = "d1",
            .status = RBRGEN4_UNKNOWN_DATASET_STATUS,
            .byteCount = 0LL,
            .dataType = RBRGEN4_UNKNOWN_DATA_TYPE },
          0,
          { NULL } },
        { NULL, 0, { .label = "" }, 0, { NULL } }
    };

    RBRGen4Error err;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        RBRGen4Dataset actual = { .label = "d1" };
        RBRGEN4_LABEL_LIST_DECL(scheduleList, 2);
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4_getDataset(conn,
                                           &actual,
                                           &scheduleList);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRGen4Error);
        TEST_ASSERT_STR_EQ("dataset d1" COMMAND_TERMINATOR,
                           buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.status,
                            actual.status,
                            RBRGen4DatasetStatus);
        TEST_ASSERT_EQ(tests[i].expectedScheduleCount,
                       scheduleList.count,
                       "%" PRId32);
        for (int32_t schedule = 0;
             schedule < tests[i].expectedScheduleCount;
             schedule++)
        {
            TEST_ASSERT_STR_EQ(tests[i].expectedScheduleList[schedule],
                               scheduleList.labels[schedule]);
        }
        TEST_ASSERT_EQ(tests[i].expected.byteCount,
                       actual.byteCount,
                       "%" PRId64);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.dataType,
                            actual.dataType,
                            RBRGen4DataType);
    }

    /* An empty label is refused before the command. */
    RBRGen4Dataset unlabelled = { .label = "" };
    TestIOBuffers_init(buffers, "", 0);
    err = RBRGen4_getDataset(conn, &unlabelled, NULL);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE,
                        err,
                        RBRGen4Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    /* The schedule list may be skipped. */
    RBRGen4Dataset unlisted = { .label = "d1" };
    TestIOBuffers_init(buffers,
                       "dataset d1 status=closed schedulelist=s_cont"
                       " bytecount=5604 datatype=float64" RESPONSE_TERMINATOR,
                       0);
    err = RBRGen4_getDataset(conn, &unlisted, NULL);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS,
                        err,
                        RBRGen4Error);
    TEST_ASSERT_EQ((int64_t) 5604, unlisted.byteCount, "%" PRId64);

    /* A schedule list which does not fit is truncated and reported. */
    RBRGen4Dataset overfull = { .label = "d1" };
    RBRGEN4_LABEL_LIST_DECL(shortList, 1);
    TestIOBuffers_init(buffers,
                       "dataset d1 status=closed"
                       " schedulelist=tides_schedule|DO_schedule"
                       " bytecount=5604 datatype=float64" RESPONSE_TERMINATOR,
                       0);
    err = RBRGen4_getDataset(conn, &overfull, &shortList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_TRUNCATED,
                        err,
                        RBRGen4Error);
    TEST_ASSERT_EQ(2, shortList.count, "%" PRId32);
    TEST_ASSERT_STR_EQ("tides_schedule", shortList.labels[0]);

    return true;
}

typedef struct GetBlockTest
{
    const char *scheduleLabel;
    const char *command;
    const char *response;
    RBRGen4Error expectedError;
    int64_t expectedByteCount;
    int64_t expectedOtherCount;
} GetBlockTest;

TEST_LOGGER4(datasetGetEventsBlock)
{
    GetBlockTest tests[] = {
        { NULL,
          "dataset d1/events" COMMAND_TERMINATOR,
          "dataset d1/events bytecount=120 eventcount=5" RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS, 120, 5 },
        /* A dataset with no events reports zero counts. */
        { NULL,
          "dataset d1/events" COMMAND_TERMINATOR,
          "dataset d1/events bytecount=0 eventcount=0" RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS, 0, 0 },
        { NULL,
          "dataset d1/events" COMMAND_TERMINATOR,
          "ERR-304 dataset not found: 'd1'" RESPONSE_TERMINATOR,
          RBRGEN4_HARDWARE_ERROR, 0, 0 },
        { NULL, NULL, NULL, 0, 0, 0 }
    };

    RBRGen4Error err;
    RBRGen4Dataset dataset = { .label = "d1" };
    RBRGen4DatasetEventsBlock block;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4Dataset_getEventsBlock(conn,
                                                      &dataset,
                                                      &block);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
        TEST_ASSERT_EQ(tests[i].expectedByteCount,
                       block.byteCount,
                       "%" PRId64);
        TEST_ASSERT_EQ(tests[i].expectedOtherCount,
                       block.eventCount,
                       "%" PRId64);
    }

    return true;
}

TEST_LOGGER4(datasetGetMetaBlock)
{
    GetBlockTest tests[] = {
        { NULL,
          "dataset d1/meta" COMMAND_TERMINATOR,
          "dataset d1/meta bytecount=4836" RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS, 4836, 0 },
        { NULL,
          "dataset d1/meta" COMMAND_TERMINATOR,
          "ERR-304 dataset not found: 'd1'" RESPONSE_TERMINATOR,
          RBRGEN4_HARDWARE_ERROR, 0, 0 },
        { NULL, NULL, NULL, 0, 0, 0 }
    };

    RBRGen4Error err;
    RBRGen4Dataset dataset = { .label = "d1" };
    RBRGen4DatasetMetaBlock block;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4Dataset_getMetaBlock(conn,
                                                    &dataset,
                                                    &block);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
        TEST_ASSERT_EQ(tests[i].expectedByteCount,
                       block.byteCount,
                       "%" PRId64);
    }

    return true;
}

TEST_LOGGER4(datasetGetScheduleBlock)
{
    GetBlockTest tests[] = {
        { "s_cont",
          "dataset d1/s_cont" COMMAND_TERMINATOR,
          "dataset d1/s_cont bytecount=768" RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS, 768, 0 },
        /*
         * A schedule the dataset does not know reports the dataset as not
         * found, naming the dataset rather than the schedule.
         */
        { "nosuch",
          "dataset d1/nosuch" COMMAND_TERMINATOR,
          "ERR-304 dataset not found: 'd1'" RESPONSE_TERMINATOR,
          RBRGEN4_HARDWARE_ERROR, 0, 0 },
        { NULL, NULL, NULL, 0, 0, 0 }
    };

    RBRGen4Error err;
    RBRGen4Dataset dataset = { .label = "d1" };
    RBRGen4DatasetScheduleBlock block;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4Dataset_getScheduleBlock(conn,
                                                        &dataset,
                                                        tests[i].scheduleLabel,
                                                        &block);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
        TEST_ASSERT_EQ(tests[i].expectedByteCount,
                       block.byteCount,
                       "%" PRId64);
    }

    /* An empty schedule label is refused before the command. */
    TestIOBuffers_init(buffers, "", 0);
    err = RBRGen4Dataset_getScheduleBlock(conn,
                                                    &dataset,
                                                    "",
                                                    &block);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE,
                        err,
                        RBRGen4Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(datasetGetScheduleEventsBlock)
{
    GetBlockTest tests[] = {
        { "s_cont",
          "dataset d1/s_cont/events" COMMAND_TERMINATOR,
          "dataset d1/s_cont/events bytecount=120 eventcount=5"
          RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS, 120, 5 },
        /*
         * A schedule the dataset does not know reports the dataset as not
         * found, naming the dataset rather than the schedule.
         */
        { "nosuch",
          "dataset d1/nosuch/events" COMMAND_TERMINATOR,
          "ERR-304 dataset not found: 'd1'" RESPONSE_TERMINATOR,
          RBRGEN4_HARDWARE_ERROR, 0, 0 },
        { NULL, NULL, NULL, 0, 0, 0 }
    };

    RBRGen4Error err;
    RBRGen4Dataset dataset = { .label = "d1" };
    RBRGen4DatasetEventsBlock block;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4Dataset_getScheduleEventsBlock(
            conn,
            &dataset,
            tests[i].scheduleLabel,
            &block);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
        TEST_ASSERT_EQ(tests[i].expectedByteCount,
                       block.byteCount,
                       "%" PRId64);
        TEST_ASSERT_EQ(tests[i].expectedOtherCount,
                       block.eventCount,
                       "%" PRId64);
    }

    return true;
}

TEST_LOGGER4(datasetGetScheduleDataBlock)
{
    GetBlockTest tests[] = {
        { "s_cont",
          "dataset d1/s_cont/data" COMMAND_TERMINATOR,
          "dataset d1/s_cont/data bytecount=648 samplecount=27"
          RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS, 648, 27 },
        /* A schedule which never sampled reports zero counts. */
        { "s_cont",
          "dataset d1/s_cont/data" COMMAND_TERMINATOR,
          "dataset d1/s_cont/data bytecount=0 samplecount=0"
          RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS, 0, 0 },
        { "nosuch",
          "dataset d1/nosuch/data" COMMAND_TERMINATOR,
          "ERR-304 dataset not found: 'd1'" RESPONSE_TERMINATOR,
          RBRGEN4_HARDWARE_ERROR, 0, 0 },
        { NULL, NULL, NULL, 0, 0, 0 }
    };

    RBRGen4Error err;
    RBRGen4Dataset dataset = { .label = "d1" };
    RBRGen4DatasetDataBlock block;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4Dataset_getScheduleDataBlock(
            conn,
            &dataset,
            tests[i].scheduleLabel,
            &block);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
        TEST_ASSERT_EQ(tests[i].expectedByteCount,
                       block.byteCount,
                       "%" PRId64);
        TEST_ASSERT_EQ(tests[i].expectedOtherCount,
                       block.sampleCount,
                       "%" PRId64);
    }

    return true;
}

typedef struct DeleteDatasetTest
{
    const char *label;
    const char *command;
    const char *response;
    RBRGen4Error expectedError;
} DeleteDatasetTest;

TEST_LOGGER4(deleteDataset)
{
    DeleteDatasetTest tests[] = {
        { "d5",
          "dataset delete d5" COMMAND_TERMINATOR,
          "dataset delete d5" RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS },
        { "nosuch",
          "dataset delete nosuch" COMMAND_TERMINATOR,
          "ERR-304 dataset not found: 'nosuch'" RESPONSE_TERMINATOR,
          RBRGEN4_HARDWARE_ERROR },
        { NULL, NULL, NULL, 0 }
    };

    RBRGen4Error err;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4_deleteDataset(conn, tests[i].label);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }

    /* An empty label is refused before the command. */
    TestIOBuffers_init(buffers, "", 0);
    err = RBRGen4_deleteDataset(conn, "");
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE,
                        err,
                        RBRGen4Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(deleteDatasetAll)
{
    TestIOBuffers_init(buffers,
                       "dataset delete all" RESPONSE_TERMINATOR,
                       0);
    RBRGen4Error err = RBRGen4_deleteDatasetAll(
        conn);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS,
                        err,
                        RBRGen4Error);
    TEST_ASSERT_STR_EQ("dataset delete all" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(datasetDownloadScheduleData)
{
    RBRGen4Error err;
    RBRGen4Dataset dataset = { .label = "d1" };
    char data[16];

    /* A transfer in bytes: the payload follows the echo, then the CRC. */
    const char *response =
        "download d1/s_cont/data bytecount=8 bytestart=0" RESPONSE_TERMINATOR
        "AAAAAAAA\x25\x94";
    RBRGen4DownloadData download = {
        .unit = RBRGEN4_DOWNLOAD_DATA_UNIT_BYTES,
        .count = 8,
        .start = 0,
        .data = data,
        .dataSize = sizeof(data)
    };
    TestIOBuffers_init(buffers, response, 0);
    err = RBRGen4Dataset_downloadScheduleData(conn,
                                                        &dataset,
                                                        "s_cont",
                                                        &download);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS,
                        err,
                        RBRGen4Error);
    TEST_ASSERT_STR_EQ("download d1/s_cont/data bytecount=8 bytestart=0"
                       COMMAND_TERMINATOR,
                       buffers->writeBuffer);
    TEST_ASSERT_EQ(INT64_C(8), download.count, "%" PRId64);
    TEST_ASSERT_EQ(INT64_C(8), download.byteCount, "%" PRId64);
    TEST_ASSERT(memcmp(data, "AAAAAAAA", 8) == 0);

    /*
     * A transfer in samples: the echo appends the byte count, and an
     * overrunning request is clamped to what the schedule holds.
     */
    response =
        "download d1/s_cont/data samplecount=2 samplestart=0 bytecount=8"
        RESPONSE_TERMINATOR
        "AAAAAAAA\x25\x94";
    download = (RBRGen4DownloadData) {
        .unit = RBRGEN4_DOWNLOAD_DATA_UNIT_SAMPLES,
        .count = 100,
        .start = 0,
        .data = data,
        .dataSize = sizeof(data)
    };
    TestIOBuffers_init(buffers, response, 0);
    err = RBRGen4Dataset_downloadScheduleData(conn,
                                                        &dataset,
                                                        "s_cont",
                                                        &download);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS,
                        err,
                        RBRGen4Error);
    TEST_ASSERT_STR_EQ("download d1/s_cont/data samplecount=100 samplestart=0"
                       COMMAND_TERMINATOR,
                       buffers->writeBuffer);
    TEST_ASSERT_EQ(INT64_C(2), download.count, "%" PRId64);
    TEST_ASSERT_EQ(INT64_C(8), download.byteCount, "%" PRId64);

    /* A corrupted transfer fails its CRC check. */
    response =
        "download d1/s_cont/data bytecount=8 bytestart=0" RESPONSE_TERMINATOR
        "AAAAAAAB\x25\x94";
    download = (RBRGen4DownloadData) {
        .unit = RBRGEN4_DOWNLOAD_DATA_UNIT_BYTES,
        .count = 8,
        .start = 0,
        .data = data,
        .dataSize = sizeof(data)
    };
    TestIOBuffers_init(buffers, response, 0);
    err = RBRGen4Dataset_downloadScheduleData(conn,
                                                        &dataset,
                                                        "s_cont",
                                                        &download);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_CHECKSUM_ERROR,
                        err,
                        RBRGen4Error);

    /* A response larger than the buffer is refused before writing it. */
    char small[4];
    response =
        "download d1/s_cont/data bytecount=8 bytestart=0" RESPONSE_TERMINATOR
        "AAAAAAAA\x25\x94";
    download = (RBRGen4DownloadData) {
        .unit = RBRGEN4_DOWNLOAD_DATA_UNIT_BYTES,
        .count = 8,
        .start = 0,
        .data = small,
        .dataSize = sizeof(small)
    };
    TestIOBuffers_init(buffers, response, 0);
    err = RBRGen4Dataset_downloadScheduleData(conn,
                                                        &dataset,
                                                        "s_cont",
                                                        &download);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_BUFFER_TOO_SMALL,
                        err,
                        RBRGen4Error);

    /* A dataset the instrument does not know is a hardware error. */
    response = "ERR-304 dataset not found: 'd1'" RESPONSE_TERMINATOR;
    download = (RBRGen4DownloadData) {
        .unit = RBRGEN4_DOWNLOAD_DATA_UNIT_BYTES,
        .count = 8,
        .start = 0,
        .data = data,
        .dataSize = sizeof(data)
    };
    TestIOBuffers_init(buffers, response, 0);
    err = RBRGen4Dataset_downloadScheduleData(conn,
                                                        &dataset,
                                                        "s_cont",
                                                        &download);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_HARDWARE_ERROR,
                        err,
                        RBRGen4Error);

    /* An invalid request is refused before the command. */
    download = (RBRGen4DownloadData) {
        .unit = RBRGEN4_UNKNOWN_DOWNLOAD_DATA_UNIT,
        .count = 8,
        .start = 0,
        .data = data,
        .dataSize = sizeof(data)
    };
    TestIOBuffers_init(buffers, "", 0);
    err = RBRGen4Dataset_downloadScheduleData(conn,
                                                        &dataset,
                                                        "s_cont",
                                                        &download);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE,
                        err,
                        RBRGen4Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(datasetDownloadEvents)
{
    RBRGen4Error err;
    RBRGen4Dataset dataset = { .label = "d1" };
    char data[24];

    /* A whole-dataset transfer measured in events. */
    const char *response =
        "download d1/events eventcount=2 eventstart=0 bytecount=16"
        RESPONSE_TERMINATOR
        "EVENTDATA0123456\xb8\x0d";
    RBRGen4DownloadEvents download = {
        .unit = RBRGEN4_DOWNLOAD_EVENTS_UNIT_EVENTS,
        .count = 2,
        .start = 0,
        .data = data,
        .dataSize = sizeof(data)
    };
    TestIOBuffers_init(buffers, response, 0);
    err = RBRGen4Dataset_downloadEvents(conn,
                                                  &dataset,
                                                  &download);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS,
                        err,
                        RBRGen4Error);
    TEST_ASSERT_STR_EQ("download d1/events eventcount=2 eventstart=0"
                       COMMAND_TERMINATOR,
                       buffers->writeBuffer);
    TEST_ASSERT_EQ(INT64_C(2), download.count, "%" PRId64);
    TEST_ASSERT_EQ(INT64_C(16), download.byteCount, "%" PRId64);
    TEST_ASSERT(memcmp(data, "EVENTDATA0123456", 16) == 0);

    return true;
}

TEST_LOGGER4(datasetDownloadScheduleEvents)
{
    RBRGen4Error err;
    RBRGen4Dataset dataset = { .label = "d1" };
    char data[24];

    const char *response =
        "download d1/s_cont/events eventcount=2 eventstart=0 bytecount=16"
        RESPONSE_TERMINATOR
        "EVENTDATA0123456\xb8\x0d";
    RBRGen4DownloadEvents download = {
        .unit = RBRGEN4_DOWNLOAD_EVENTS_UNIT_EVENTS,
        .count = 2,
        .start = 0,
        .data = data,
        .dataSize = sizeof(data)
    };
    TestIOBuffers_init(buffers, response, 0);
    err = RBRGen4Dataset_downloadScheduleEvents(conn,
                                                          &dataset,
                                                          "s_cont",
                                                          &download);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS,
                        err,
                        RBRGen4Error);
    TEST_ASSERT_STR_EQ("download d1/s_cont/events eventcount=2 eventstart=0"
                       COMMAND_TERMINATOR,
                       buffers->writeBuffer);
    TEST_ASSERT_EQ(INT64_C(2), download.count, "%" PRId64);
    TEST_ASSERT_EQ(INT64_C(16), download.byteCount, "%" PRId64);

    return true;
}

TEST_LOGGER4(datasetDownloadMeta)
{
    RBRGen4Error err;
    RBRGen4Dataset dataset = { .label = "d1" };
    char data[16];

    const char *response =
        "download d1/meta bytecount=8 bytestart=0" RESPONSE_TERMINATOR
        "METAMETA\xc3\x14";
    RBRGen4DownloadMeta download = {
        .byteCount = 8,
        .byteStart = 0,
        .data = data,
        .dataSize = sizeof(data)
    };
    TestIOBuffers_init(buffers, response, 0);
    err = RBRGen4Dataset_downloadMeta(conn,
                                                &dataset,
                                                &download);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS,
                        err,
                        RBRGen4Error);
    TEST_ASSERT_STR_EQ("download d1/meta bytecount=8 bytestart=0"
                       COMMAND_TERMINATOR,
                       buffers->writeBuffer);
    TEST_ASSERT_EQ(INT64_C(8), download.byteCount, "%" PRId64);
    TEST_ASSERT(memcmp(data, "METAMETA", 8) == 0);

    return true;
}
