/**
 * \file memory.c
 *
 * \brief Tests for instrument memory commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include "tests.h"
#include "RBRInstrumentGen4Memory.h"

/* Required for PRId64. */
#include <inttypes.h>

typedef struct GetStorageTest
{
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4Storage expected;
} GetStorageTest;

TEST_LOGGER4(getStorage)
{
    GetStorageTest tests[] = {
        /* A bare query reports all four parameters. */
        { "storage used=15372 remaining=61016097780 size=61016113152"
          " access=instrument" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 15372LL,
            61016097780LL,
            61016113152LL,
            RBRINSTRUMENTGEN4_STORAGE_ACCESS_INSTRUMENT } },
        { "storage used=1528 remaining=134216192 size=134217728"
          " access=usbhost" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 1528LL,
            134216192LL,
            134217728LL,
            RBRINSTRUMENTGEN4_STORAGE_ACCESS_USBHOST } },
        /* An unreported parameter keeps its unset value. */
        { "storage used=15372" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 15372LL, 0LL, 0LL, RBRINSTRUMENTGEN4_UNKNOWN_STORAGE_ACCESS } },
        /* An unrecognized access location parses to the unknown member. */
        { "storage used=15372 remaining=61016097780 size=61016113152"
          " access=cloud" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 15372LL,
            61016097780LL,
            61016113152LL,
            RBRINSTRUMENTGEN4_UNKNOWN_STORAGE_ACCESS } },
        /* Keys the library does not model are ignored. */
        { "storage used=15372 remaining=61016097780 size=61016113152"
          " access=instrument bogus=1" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 15372LL,
            61016097780LL,
            61016113152LL,
            RBRINSTRUMENTGEN4_STORAGE_ACCESS_INSTRUMENT } },
        { NULL, 0, { 0, 0, 0, 0 } }
    };

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Storage actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_getStorage(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ("storage" COMMAND_TERMINATOR,
                           buffers->writeBuffer);
        TEST_ASSERT_EQ(tests[i].expected.used, actual.used, "%" PRId64);
        TEST_ASSERT_EQ(tests[i].expected.remaining,
                       actual.remaining,
                       "%" PRId64);
        TEST_ASSERT_EQ(tests[i].expected.size, actual.size, "%" PRId64);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.access,
                            actual.access,
                            RBRInstrumentGen4StorageAccess);
    }

    return true;
}

typedef struct SetStorageTest
{
    RBRInstrumentGen4StorageAccess access;
    const char *command;
    const char *response;
    RBRInstrumentGen4Error expectedError;
} SetStorageTest;

TEST_LOGGER4(setStorage)
{
    SetStorageTest tests[] = {
        { RBRINSTRUMENTGEN4_STORAGE_ACCESS_INSTRUMENT,
          "storage access=instrument" COMMAND_TERMINATOR,
          "storage access=instrument" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS },
        { RBRINSTRUMENTGEN4_STORAGE_ACCESS_USBHOST,
          "storage access=usbhost" COMMAND_TERMINATOR,
          "storage access=usbhost" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS },
        /*
         * Selecting the location already in use is a warning, which the
         * library surfaces as a hardware error with the response type
         * distinguishing it.
         */
        { RBRINSTRUMENTGEN4_STORAGE_ACCESS_INSTRUMENT,
          "storage access=instrument" COMMAND_TERMINATOR,
          "WRN-305 storage access already at selected location"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR },
        /* Sentinel members are refused before the command. */
        { RBRINSTRUMENTGEN4_STORAGE_ACCESS_COUNT,
          "",
          "",
          RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE },
        { RBRINSTRUMENTGEN4_UNKNOWN_STORAGE_ACCESS,
          "",
          "",
          RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE },
        { 0, NULL, NULL, 0 }
    };

    RBRInstrumentGen4Error err;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        RBRInstrumentGen4Storage storage = {
            .access = tests[i].access
        };
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_setStorage(instrument, &storage);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }

    return true;
}

typedef struct GetDatasetPoolTest
{
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4DatasetPool expected;
} GetDatasetPoolTest;

TEST_LOGGER4(getDatasetPool)
{
    GetDatasetPoolTest tests[] = {
        { "dataset count=3 maxcount=4 list=d1|d2|d5" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 3, 4, { { .label = "d1" }, { .label = "d2" },
                    { .label = "d5" } } } },
        { "dataset count=1 maxcount=20 list=DeepCove" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 1, 20, { { .label = "DeepCove" } } } },
        /* An empty pool reports `none`. */
        { "dataset count=0 maxcount=4 list=none" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 0, 4, { { .label = "" } } } },
        /* Keys the library does not model are ignored. */
        { "dataset count=1 maxcount=4 list=d1 bogus=1" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 1, 4, { { .label = "d1" } } } },
        { NULL, 0, { 0, 0, { { .label = "" } } } }
    };

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4DatasetPool actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_getDatasetPool(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ("dataset" COMMAND_TERMINATOR,
                           buffers->writeBuffer);
        TEST_ASSERT_EQ(tests[i].expected.count, actual.count, "%" PRId32);
        TEST_ASSERT_EQ(tests[i].expected.maxCount,
                       actual.maxCount,
                       "%" PRId32);
        for (int32_t dataset = 0;
             dataset < RBRINSTRUMENTGEN4_DATASET_COUNT_MAX
             && dataset < tests[i].expected.count;
             dataset++)
        {
            TEST_ASSERT_STR_EQ(tests[i].expected.pool[dataset].label,
                               actual.pool[dataset].label);
        }
    }

    return true;
}

typedef struct GetDatasetTest
{
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4Dataset expected;
} GetDatasetTest;

TEST_LOGGER4(getDataset)
{
    GetDatasetTest tests[] = {
        { "dataset d1 status=closed schedulelist=s_cont bytecount=5604"
          " datatype=float64" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { .label = "d1",
            .status = RBRINSTRUMENTGEN4_DATASET_STATUS_CLOSED,
            .scheduleCount = 1,
            .scheduleList = { "s_cont" },
            .byteCount = 5604LL,
            .dataType = RBRINSTRUMENTGEN4_DATATYPE_FLOAT64 } },
        { "dataset d1 status=open schedulelist=tides_schedule|DO_schedule"
          " bytecount=3749498 datatype=float32" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { .label = "d1",
            .status = RBRINSTRUMENTGEN4_DATASET_STATUS_OPEN,
            .scheduleCount = 2,
            .scheduleList = { "tides_schedule", "DO_schedule" },
            .byteCount = 3749498LL,
            .dataType = RBRINSTRUMENTGEN4_DATATYPE_FLOAT32 } },
        /* Values the library does not model parse to the unknown members. */
        { "dataset d1 status=bogus schedulelist=s_cont bytecount=0"
          " datatype=bogus" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { .label = "d1",
            .status = RBRINSTRUMENTGEN4_UNKNOWN_DATASET_STATUS,
            .scheduleCount = 1,
            .scheduleList = { "s_cont" },
            .byteCount = 0LL,
            .dataType = RBRINSTRUMENTGEN4_UNKNOWN_DATATYPE } },
        /* A dataset which does not exist is a hardware error. */
        { "ERR-304 dataset not found: 'd1'" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR,
          { .label = "d1",
            .status = RBRINSTRUMENTGEN4_UNKNOWN_DATASET_STATUS,
            .scheduleCount = 0,
            .scheduleList = { "" },
            .byteCount = 0LL,
            .dataType = RBRINSTRUMENTGEN4_UNKNOWN_DATATYPE } },
        { NULL, 0, { .label = "" } }
    };

    RBRInstrumentGen4Error err;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        RBRInstrumentGen4Dataset actual = { .label = "d1" };
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_getDataset(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ("dataset d1" COMMAND_TERMINATOR,
                           buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.status,
                            actual.status,
                            RBRInstrumentGen4DatasetStatus);
        TEST_ASSERT_EQ(tests[i].expected.scheduleCount,
                       actual.scheduleCount,
                       "%" PRId32);
        for (int32_t schedule = 0;
             schedule < tests[i].expected.scheduleCount;
             schedule++)
        {
            TEST_ASSERT_STR_EQ(tests[i].expected.scheduleList[schedule],
                               actual.scheduleList[schedule]);
        }
        TEST_ASSERT_EQ(tests[i].expected.byteCount,
                       actual.byteCount,
                       "%" PRId64);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.dataType,
                            actual.dataType,
                            RBRInstrumentGen4DataType);
    }

    /* An empty label is refused before the command. */
    RBRInstrumentGen4Dataset unlabelled = { .label = "" };
    TestIOBuffers_init(buffers, "", 0);
    err = RBRInstrumentGen4_getDataset(instrument, &unlabelled);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE,
                        err,
                        RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

typedef struct GetBlockTest
{
    const char *scheduleLabel;
    const char *command;
    const char *response;
    RBRInstrumentGen4Error expectedError;
    int64_t expectedByteCount;
    int64_t expectedOtherCount;
} GetBlockTest;

TEST_LOGGER4(datasetGetEventsBlock)
{
    GetBlockTest tests[] = {
        { NULL,
          "dataset d1/events" COMMAND_TERMINATOR,
          "dataset d1/events bytecount=120 eventcount=5" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS, 120, 5 },
        /* A dataset with no events reports zero counts. */
        { NULL,
          "dataset d1/events" COMMAND_TERMINATOR,
          "dataset d1/events bytecount=0 eventcount=0" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS, 0, 0 },
        { NULL,
          "dataset d1/events" COMMAND_TERMINATOR,
          "ERR-304 dataset not found: 'd1'" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR, 0, 0 },
        { NULL, NULL, NULL, 0, 0, 0 }
    };

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Dataset dataset = { .label = "d1" };
    RBRInstrumentGen4DatasetEventsBlock block;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4Dataset_getEventsBlock(instrument,
                                                      &dataset,
                                                      &block);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRInstrumentGen4Error);
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
          RBRINSTRUMENTGEN4_SUCCESS, 4836, 0 },
        { NULL,
          "dataset d1/meta" COMMAND_TERMINATOR,
          "ERR-304 dataset not found: 'd1'" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR, 0, 0 },
        { NULL, NULL, NULL, 0, 0, 0 }
    };

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Dataset dataset = { .label = "d1" };
    RBRInstrumentGen4DatasetMetaBlock block;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4Dataset_getMetaBlock(instrument,
                                                    &dataset,
                                                    &block);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRInstrumentGen4Error);
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
          RBRINSTRUMENTGEN4_SUCCESS, 768, 0 },
        /*
         * A schedule the dataset does not know reports the dataset as not
         * found, naming the dataset rather than the schedule.
         */
        { "nosuch",
          "dataset d1/nosuch" COMMAND_TERMINATOR,
          "ERR-304 dataset not found: 'd1'" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR, 0, 0 },
        { NULL, NULL, NULL, 0, 0, 0 }
    };

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Dataset dataset = { .label = "d1" };
    RBRInstrumentGen4DatasetScheduleBlock block;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4Dataset_getScheduleBlock(instrument,
                                                        &dataset,
                                                        tests[i].scheduleLabel,
                                                        &block);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
        TEST_ASSERT_EQ(tests[i].expectedByteCount,
                       block.byteCount,
                       "%" PRId64);
    }

    /* An empty schedule label is refused before the command. */
    TestIOBuffers_init(buffers, "", 0);
    err = RBRInstrumentGen4Dataset_getScheduleBlock(instrument,
                                                    &dataset,
                                                    "",
                                                    &block);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE,
                        err,
                        RBRInstrumentGen4Error);
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
          RBRINSTRUMENTGEN4_SUCCESS, 120, 5 },
        /*
         * A schedule the dataset does not know reports the dataset as not
         * found, naming the dataset rather than the schedule.
         */
        { "nosuch",
          "dataset d1/nosuch/events" COMMAND_TERMINATOR,
          "ERR-304 dataset not found: 'd1'" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR, 0, 0 },
        { NULL, NULL, NULL, 0, 0, 0 }
    };

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Dataset dataset = { .label = "d1" };
    RBRInstrumentGen4DatasetEventsBlock block;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4Dataset_getScheduleEventsBlock(
            instrument,
            &dataset,
            tests[i].scheduleLabel,
            &block);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRInstrumentGen4Error);
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
          RBRINSTRUMENTGEN4_SUCCESS, 648, 27 },
        /* A schedule which never sampled reports zero counts. */
        { "s_cont",
          "dataset d1/s_cont/data" COMMAND_TERMINATOR,
          "dataset d1/s_cont/data bytecount=0 samplecount=0"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS, 0, 0 },
        { "nosuch",
          "dataset d1/nosuch/data" COMMAND_TERMINATOR,
          "ERR-304 dataset not found: 'd1'" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR, 0, 0 },
        { NULL, NULL, NULL, 0, 0, 0 }
    };

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Dataset dataset = { .label = "d1" };
    RBRInstrumentGen4DatasetDataBlock block;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4Dataset_getScheduleDataBlock(
            instrument,
            &dataset,
            tests[i].scheduleLabel,
            &block);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRInstrumentGen4Error);
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
