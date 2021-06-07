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

TEST_LOGGER3(meminfo)
{
    RBRInstrumentMemoryInfo expected = {
        .dataset = RBRINSTRUMENT_DATASET_STANDARD,
        .used = 1528,
        .remaining = 134216192,
        .size = 134217728
    };
    RBRInstrumentMemoryInfo actual = {
        .dataset = RBRINSTRUMENT_DATASET_STANDARD
    };

    TestIOBuffers_init(buffers,
                       "meminfo dataset = 1, used = 1528, "
                       "remaining = 134216192, size = 134217728"
                       COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getMemoryInfo(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_EQ(expected.dataset, actual.dataset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.used, actual.used, "%" PRIi32);
    TEST_ASSERT_EQ(expected.remaining, actual.remaining, "%" PRIi32);
    TEST_ASSERT_EQ(expected.size, actual.size, "%" PRIi32);
    TEST_ASSERT_STR_EQ("meminfo dataset = 1" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(meminfo_invalid_dataset)
{
    RBRInstrumentMemoryInfo test = {
        .dataset = RBRINSTRUMENT_UNKNOWN_DATASET
    };

    TestIOBuffers_init(buffers, "", 0);
    RBRInstrumentError err = RBRInstrument_getMemoryInfo(instrument, &test);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_INVALID_PARAMETER_VALUE,
                        err,
                        RBRInstrumentError);

    return true;
}

TEST_LOGGER2(read)
{
    uint8_t buf[1400];
    RBRInstrumentData expected = {
        .dataset = RBRINSTRUMENT_DATASET_STANDARD,
        .size    = 8,
        .offset  = 2800,
        .data    = buf
    };
    RBRInstrumentData actual = {
        .dataset = RBRINSTRUMENT_DATASET_STANDARD,
        .size    = 1400,
        .offset  = 2800,
        .data    = buf
    };

    TestIOBuffers_init(buffers,
                       "data 1 8 2800"
                       COMMAND_TERMINATOR
                       "AAAAAAAA\045\224"
                       COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_readData(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_EQ(expected.dataset, actual.dataset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.size, actual.size, "%" PRIi32);
    TEST_ASSERT_EQ(expected.offset, actual.offset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.data, actual.data, "%p");
    TEST_ASSERT_STR_EQ("read data 1 1400 2800"
                       COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(readdata)
{
    uint8_t buf[1400];
    RBRInstrumentData expected = {
        .dataset = RBRINSTRUMENT_DATASET_STANDARD,
        .size    = 8,
        .offset  = 2800,
        .data    = buf
    };
    RBRInstrumentData actual = {
        .dataset = RBRINSTRUMENT_DATASET_STANDARD,
        .size    = 1400,
        .offset  = 2800,
        .data    = buf
    };

    TestIOBuffers_init(buffers,
                       "readdata dataset = 1, size = 8, offset = 2800"
                       COMMAND_TERMINATOR
                       "AAAAAAAA\045\224"
                       COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_readData(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_EQ(expected.dataset, actual.dataset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.size, actual.size, "%" PRIi32);
    TEST_ASSERT_EQ(expected.offset, actual.offset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.data, actual.data, "%p");
    TEST_ASSERT_STR_EQ("readdata dataset = 1, size = 1400, offset = 2800"
                       COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(readdata_invalid_dataset)
{
    RBRInstrumentData test = {
        .dataset = RBRINSTRUMENT_UNKNOWN_DATASET,
        .size    = 0,
        .offset  = 0,
        .data    = NULL
    };

    TestIOBuffers_init(buffers, "", 0);
    RBRInstrumentError err = RBRInstrument_readData(instrument, &test);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_INVALID_PARAMETER_VALUE,
                        err,
                        RBRInstrumentError);

    return true;
}

TEST_LOGGER3(readdata_crc_failure)
{
    uint8_t buf[1400];
    RBRInstrumentData expected = {
        .dataset = RBRINSTRUMENT_DATASET_STANDARD,
        .size    = 0,
        .offset  = 2800,
        .data    = buf
    };
    RBRInstrumentData actual = {
        .dataset = RBRINSTRUMENT_DATASET_STANDARD,
        .size    = 1400,
        .offset  = 2800,
        .data    = buf
    };

    TestIOBuffers_init(buffers,
                       "readdata dataset = 1, size = 8, offset = 2800"
                       COMMAND_TERMINATOR
                       "AAAAAAAA00"
                       COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_readData(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_CHECKSUM_ERROR, err, RBRInstrumentError);
    TEST_ASSERT_EQ(expected.dataset, actual.dataset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.size, actual.size, "%" PRIi32);
    TEST_ASSERT_EQ(expected.offset, actual.offset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.data, actual.data, "%p");
    TEST_ASSERT_STR_EQ("readdata dataset = 1, size = 1400, offset = 2800"
                       COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER2(memformat_support)
{
    RBRInstrumentMemoryFormat expected = RBRINSTRUMENT_MEMFORMAT_RAWBIN00
                                         | RBRINSTRUMENT_MEMFORMAT_CALBIN00;
    RBRInstrumentMemoryFormat actual = RBRINSTRUMENT_MEMFORMAT_NONE;

    TestIOBuffers_init(buffers,
                       "memformat support = rawbin00, calbin00"
                       COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getAvailableMemoryFormats(
        instrument,
        &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_EQ(expected, actual, "0x%04X");
    TEST_ASSERT_STR_EQ("memformat support" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(memformat_availabletypes)
{
    RBRInstrumentMemoryFormat expected = RBRINSTRUMENT_MEMFORMAT_RAWBIN00
                                         | RBRINSTRUMENT_MEMFORMAT_CALBIN00;
    RBRInstrumentMemoryFormat actual = RBRINSTRUMENT_MEMFORMAT_NONE;

    TestIOBuffers_init(buffers,
                       "memformat availabletypes = rawbin00|calbin00"
                       COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getAvailableMemoryFormats(
        instrument,
        &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_EQ(expected, actual, "0x%04X");
    TEST_ASSERT_STR_EQ("memformat availabletypes" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(memformat_type)
{
    RBRInstrumentMemoryFormat expected = RBRINSTRUMENT_MEMFORMAT_RAWBIN00;
    RBRInstrumentMemoryFormat actual = RBRINSTRUMENT_MEMFORMAT_NONE;

    TestIOBuffers_init(buffers,
                       "memformat type = rawbin00" COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getCurrentMemoryFormat(
        instrument,
        &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_ENUM_EQ(expected, actual, RBRInstrumentMemoryFormat);
    TEST_ASSERT_STR_EQ("memformat type" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(memformat_newtype)
{
    RBRInstrumentMemoryFormat expected = RBRINSTRUMENT_MEMFORMAT_CALBIN00;
    RBRInstrumentMemoryFormat actual = RBRINSTRUMENT_MEMFORMAT_NONE;

    TestIOBuffers_init(buffers,
                       "memformat newtype = calbin00" COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getNewMemoryFormat(
        instrument,
        &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_ENUM_EQ(expected, actual, RBRInstrumentMemoryFormat);
    TEST_ASSERT_STR_EQ("memformat newtype" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(memformat_newtype_set)
{
    const char *command = "memformat newtype = calbin00" COMMAND_TERMINATOR;
    TestIOBuffers_init(buffers, command, 0);
    RBRInstrumentError err = RBRInstrument_setNewMemoryFormat(
        instrument,
        RBRINSTRUMENT_MEMFORMAT_CALBIN00);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(command, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(postprocessing)
{
    RBRInstrumentPostprocessing expected = {
        .status = RBRINSTRUMENT_POSTPROCESSING_STATUS_IDLE,
        .channels = {
            .count = 5,
            .channels = {
                {
                    .function = RBRINSTRUMENT_POSTPROCESSING_AGGREGATE_MEAN,
                    .label = "pressure_01"
                },
                {
                    .function =
                        RBRINSTRUMENT_POSTPROCESSING_AGGREGATE_SAMPLE_COUNT,
                    .label = "pressure_01"
                },
                {
                    .function = RBRINSTRUMENT_POSTPROCESSING_AGGREGATE_MEAN,
                    .label = "temperature_01"
                },
                {
                    .function = RBRINSTRUMENT_POSTPROCESSING_AGGREGATE_STD,
                    .label = "temperature_01"
                },
                {
                    .function = RBRINSTRUMENT_POSTPROCESSING_AGGREGATE_MEAN,
                    .label = "conductivity_01"
                }
            }
        },
        .binReference = "pressure_01",
        .binFilter = RBRINSTRUMENT_POSTPROCESSING_BINFILTER_NONE,
        .binSize = 50.0,
        .tstampMin = RBRINSTRUMENT_DATETIME_MIN,
        .tstampMax = RBRINSTRUMENT_DATETIME_MAX,
        .depthMin = 10.0,
        .depthMax = 1000.0,
        .dcAlpha = 0.08,
        .dcTau = 8.0,
        .dcTdelay = 0.35,
        .dcCtCoeff = 2.4e-4
    };
    RBRInstrumentPostprocessing actual;

    const char *command = "postprocessing status = idle, channels = "
                          "mean(pressure_01)|count(pressure_01)"
                          "|mean(temperature_01)|std(temperature_01)"
                          "|mean(conductivity_01), "
                          "tstamp_min = 20000101000000, "
                          "tstamp_max = 20991231235959, "
                          "binsize = 50.0, binreference = pressure_01, "
                          "depth_min = 10.0, depth_max = 1000.0, "
                          "binfilter = none, "
                          "dc_alpha = 0.08, dc_tau = 8.000, dc_tdelay = 0.35, dc_ctcoeff = 2.4e-4" COMMAND_TERMINATOR;
    TestIOBuffers_init(buffers, command, 0);
    RBRInstrumentError err = RBRInstrument_getPostprocessing(instrument,
                                                             &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_ENUM_EQ(expected.status,
                        actual.status,
                        RBRInstrumentPostprocessingStatus);
    TEST_ASSERT_EQ(expected.channels.count, actual.channels.count, "%" PRIi32);
    for (int i = 0; i < expected.channels.count; i++)
    {
        TEST_ASSERT_ENUM_EQ(expected.channels.channels[i].function,
                            actual.channels.channels[i].function,
                            RBRInstrumentPostprocessingAggregate);
        TEST_ASSERT_STR_EQ(expected.channels.channels[i].label,
                           actual.channels.channels[i].label);
    }
    TEST_ASSERT_STR_EQ(expected.binReference, actual.binReference);
    TEST_ASSERT_ENUM_EQ(expected.binFilter,
                        actual.binFilter,
                        RBRInstrumentPostprocessingBinFilter);
    TEST_ASSERT_EQ(expected.binSize, actual.binSize, "%f");
    TEST_ASSERT_EQ(expected.tstampMin, actual.tstampMin, "%" PRIi64);
    TEST_ASSERT_EQ(expected.tstampMax, actual.tstampMax, "%" PRIi64);
    TEST_ASSERT_EQ(expected.depthMin, actual.depthMin, "%f");
    TEST_ASSERT_EQ(expected.depthMax, actual.depthMax, "%f");
    TEST_ASSERT_EQ(expected.dcAlpha, actual.dcAlpha, "%f");
    TEST_ASSERT_EQ(expected.dcTau, actual.dcTau, "%f");
    TEST_ASSERT_EQ(expected.dcTdelay, actual.dcTdelay, "%f");
    TEST_ASSERT_EQ(expected.dcCtCoeff, actual.dcCtCoeff, "%f");
    TEST_ASSERT_STR_EQ("postprocessing all" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(postprocessing_set)
{
    RBRInstrumentPostprocessing postprocessing = {
        .status = RBRINSTRUMENT_UNKNOWN_POSTPROCESSING_STATUS,
        .channels = {
            .count = 5,
            .channels = {
                {
                    .function = RBRINSTRUMENT_POSTPROCESSING_AGGREGATE_MEAN,
                    .label = "pressure_01"
                },
                {
                    .function =
                        RBRINSTRUMENT_POSTPROCESSING_AGGREGATE_SAMPLE_COUNT,
                    .label = "pressure_01"
                },
                {
                    .function = RBRINSTRUMENT_POSTPROCESSING_AGGREGATE_MEAN,
                    .label = "temperature_01"
                },
                {
                    .function = RBRINSTRUMENT_POSTPROCESSING_AGGREGATE_STD,
                    .label = "temperature_01"
                },
                {
                    .function = RBRINSTRUMENT_POSTPROCESSING_AGGREGATE_MEAN,
                    .label = "conductivity_01"
                }
            }
        },
        .binReference = "pressure_01",
        .binFilter = RBRINSTRUMENT_POSTPROCESSING_BINFILTER_NONE,
        .binSize = 50.0,
        .tstampMin = RBRINSTRUMENT_DATETIME_MIN,
        .tstampMax = RBRINSTRUMENT_DATETIME_MAX,
        .depthMin = 10.0,
        .depthMax = 1000.0,
        .dcAlpha = 0.08,
        .dcTau = 8.0,
        .dcTdelay = 0.35,
        .dcCtCoeff = 2.4e-4
    };

    const char *command = "postprocessing binreference = pressure_01, "
                          "binfilter = none, binsize = 50.0"
                          COMMAND_TERMINATOR
                          "postprocessing tstamp_min = 20000101000000"
                          COMMAND_TERMINATOR
                          "postprocessing tstamp_max = 20991231235959"
                          COMMAND_TERMINATOR
                          "postprocessing depth_min = 10.0, depth_max = 1000.0"
                          COMMAND_TERMINATOR
                          "postprocessing dc_alpha = 0.080, dc_tau = 8.000, dc_tdelay = 0.350, dc_ctcoeff = 2.4000e-04"
                          COMMAND_TERMINATOR
                          "postprocessing channels = mean(pressure_01)"
                          "|count(pressure_01)|mean(temperature_01)"
                          "|std(temperature_01)|mean(conductivity_01)"
                          COMMAND_TERMINATOR;

    TestIOBuffers_init(buffers, command, 0);
    RBRInstrumentError err = RBRInstrument_setPostprocessing(instrument,
                                                             &postprocessing);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(command, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(postprocessing_command)
{
    const char *command = "postprocessing status = processing"
                          COMMAND_TERMINATOR;
    TestIOBuffers_init(buffers, command, 0);

    RBRInstrumentPostprocessingStatus result;
    RBRInstrumentError err = RBRInstrument_setPostprocessingCommand(
        instrument,
        RBRINSTRUMENT_POSTPROCESSING_COMMAND_START,
        &result);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_POSTPROCESSING_STATUS_PROCESSING,
                        result,
                        RBRInstrumentPostprocessingStatus);
    TEST_ASSERT_STR_EQ("postprocessing command = start" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}
