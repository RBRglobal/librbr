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
    RBRInstrumentGen3MemoryInfo expected = {
        .dataset = RBRINSTRUMENTGEN3_DATASET_STANDARD,
        .used = 1528,
        .remaining = 134216192,
        .size = 134217728
    };
    RBRInstrumentGen3MemoryInfo actual = {
        .dataset = RBRINSTRUMENTGEN3_DATASET_STANDARD
    };

    TestIOBuffers_init(buffers,
                       "meminfo dataset = 1, used = 1528, "
                       "remaining = 134216192, size = 134217728"
                       RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRInstrumentGen3_getMemoryInfo(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
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
    RBRInstrumentGen3MemoryInfo test = {
        .dataset = RBRINSTRUMENTGEN3_UNKNOWN_DATASET
    };

    TestIOBuffers_init(buffers, "", 0);
    RBRGen3Error err = RBRInstrumentGen3_getMemoryInfo(instrument, &test);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE,
                        err,
                        RBRGen3Error);

    return true;
}

TEST_LOGGER2(read)
{
    uint8_t buf[1400];
    RBRInstrumentGen3Data expected = {
        .dataset = RBRINSTRUMENTGEN3_DATASET_STANDARD,
        .size    = 8,
        .offset  = 2800,
        .data    = buf
    };
    RBRInstrumentGen3Data actual = {
        .dataset = RBRINSTRUMENTGEN3_DATASET_STANDARD,
        .size    = 1400,
        .offset  = 2800,
        .data    = buf
    };

    TestIOBuffers_init(buffers,
                       "data 1 8 2800"
                       RESPONSE_TERMINATOR
                       "AAAAAAAA\045\224"
                       RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRInstrumentGen3_readData(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(expected.dataset, actual.dataset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.size, actual.size, "%" PRIi32);
    TEST_ASSERT_EQ(expected.offset, actual.offset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.data, actual.data, "%p");
    TEST_ASSERT_STR_EQ("read data 1 1400 2800"
                       COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER2(read_offset_mismatch)
{
    uint8_t buf[1400];
    RBRInstrumentGen3Data actual = {
        .dataset = RBRINSTRUMENTGEN3_DATASET_STANDARD,
        .size    = 1400,
        .offset  = 2800,
        .data    = buf
    };


    TestIOBuffers_init(buffers,
                       "data 1 8 1000"
                       RESPONSE_TERMINATOR
                       "AAAAAAAA\045\224"
                       RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRInstrumentGen3_readData(instrument, &actual);
    TEST_ASSERT_STR_EQ("read data 1 1400 2800"
                       COMMAND_TERMINATOR,
                       buffers->writeBuffer);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_COMMUNICATION_ERROR, err, RBRGen3Error);
    return true;
}

TEST_LOGGER3(readdata)
{
    uint8_t buf[1400];
    RBRInstrumentGen3Data expected = {
        .dataset = RBRINSTRUMENTGEN3_DATASET_STANDARD,
        .size    = 8,
        .offset  = 2800,
        .data    = buf
    };
    RBRInstrumentGen3Data actual = {
        .dataset = RBRINSTRUMENTGEN3_DATASET_STANDARD,
        .size    = 1400,
        .offset  = 2800,
        .data    = buf
    };

    TestIOBuffers_init(buffers,
                       "readdata dataset = 1, size = 8, offset = 2800"
                       RESPONSE_TERMINATOR
                       "AAAAAAAA\045\224"
                       RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRInstrumentGen3_readData(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(expected.dataset, actual.dataset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.size, actual.size, "%" PRIi32);
    TEST_ASSERT_EQ(expected.offset, actual.offset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.data, actual.data, "%p");
    TEST_ASSERT_STR_EQ("readdata dataset = 1, size = 1400, offset = 2800"
                       COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(readdata_offset_mismatch)
{
    uint8_t buf[1400];
    RBRInstrumentGen3Data actual = {
        .dataset = RBRINSTRUMENTGEN3_DATASET_STANDARD,
        .size    = 1400,
        .offset  = 2800,
        .data    = buf
    };

    TestIOBuffers_init(buffers,
                       "readdata dataset = 1, size = 8, offset = 1000"
                       RESPONSE_TERMINATOR
                       "AAAAAAAA\045\024"
                       RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRInstrumentGen3_readData(instrument, &actual);
    TEST_ASSERT_STR_EQ("readdata dataset = 1, size = 1400, offset = 2800"
                       COMMAND_TERMINATOR,
                       buffers->writeBuffer);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_COMMUNICATION_ERROR, err, RBRGen3Error);
    return true;
}

TEST_LOGGER3(readdata_invalid_dataset)
{
    RBRInstrumentGen3Data test = {
        .dataset = RBRINSTRUMENTGEN3_UNKNOWN_DATASET,
        .size    = 0,
        .offset  = 0,
        .data    = NULL
    };

    TestIOBuffers_init(buffers, "", 0);
    RBRGen3Error err = RBRInstrumentGen3_readData(instrument, &test);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE,
                        err,
                        RBRGen3Error);

    return true;
}

TEST_LOGGER3(readdata_crc_failure)
{
    uint8_t buf[1400];
    RBRInstrumentGen3Data expected = {
        .dataset = RBRINSTRUMENTGEN3_DATASET_STANDARD,
        .size    = 0,
        .offset  = 2800,
        .data    = buf
    };
    RBRInstrumentGen3Data actual = {
        .dataset = RBRINSTRUMENTGEN3_DATASET_STANDARD,
        .size    = 1400,
        .offset  = 2800,
        .data    = buf
    };

    TestIOBuffers_init(buffers,
                       "readdata dataset = 1, size = 8, offset = 2800"
                       RESPONSE_TERMINATOR
                       "AAAAAAAA00"
                       RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRInstrumentGen3_readData(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_CHECKSUM_ERROR, err, RBRGen3Error);
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
    RBRInstrumentGen3MemoryFormat expected = RBRINSTRUMENTGEN3_MEMFORMAT_RAWBIN00
                                         | RBRINSTRUMENTGEN3_MEMFORMAT_CALBIN00;
    RBRInstrumentGen3MemoryFormat actual = RBRINSTRUMENTGEN3_MEMFORMAT_NONE;

    TestIOBuffers_init(buffers,
                       "memformat support = rawbin00, calbin00"
                       RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRInstrumentGen3_getAvailableMemoryFormats(
        instrument,
        &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(expected, actual, "0x%04X");
    TEST_ASSERT_STR_EQ("memformat support" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(memformat_availabletypes)
{
    RBRInstrumentGen3MemoryFormat expected = RBRINSTRUMENTGEN3_MEMFORMAT_RAWBIN00
                                         | RBRINSTRUMENTGEN3_MEMFORMAT_CALBIN00;
    RBRInstrumentGen3MemoryFormat actual = RBRINSTRUMENTGEN3_MEMFORMAT_NONE;

    TestIOBuffers_init(buffers,
                       "memformat availabletypes = rawbin00|calbin00"
                       RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRInstrumentGen3_getAvailableMemoryFormats(
        instrument,
        &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(expected, actual, "0x%04X");
    TEST_ASSERT_STR_EQ("memformat availabletypes" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(memformat_type)
{
    RBRInstrumentGen3MemoryFormat expected = RBRINSTRUMENTGEN3_MEMFORMAT_RAWBIN00;
    RBRInstrumentGen3MemoryFormat actual = RBRINSTRUMENTGEN3_MEMFORMAT_NONE;

    TestIOBuffers_init(buffers,
                       "memformat type = rawbin00" RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRInstrumentGen3_getCurrentMemoryFormat(
        instrument,
        &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(expected, actual, RBRInstrumentGen3MemoryFormat);
    TEST_ASSERT_STR_EQ("memformat type" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(memformat_newtype)
{
    RBRInstrumentGen3MemoryFormat expected = RBRINSTRUMENTGEN3_MEMFORMAT_CALBIN00;
    RBRInstrumentGen3MemoryFormat actual = RBRINSTRUMENTGEN3_MEMFORMAT_NONE;

    TestIOBuffers_init(buffers,
                       "memformat newtype = calbin00" RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRInstrumentGen3_getNewMemoryFormat(
        instrument,
        &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(expected, actual, RBRInstrumentGen3MemoryFormat);
    TEST_ASSERT_STR_EQ("memformat newtype" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(memformat_newtype_set)
{
    const char *text = "memformat newtype = calbin00";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRInstrumentGen3_setNewMemoryFormat(
        instrument,
        RBRINSTRUMENTGEN3_MEMFORMAT_CALBIN00);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(postprocessing)
{
    RBRInstrumentGen3Postprocessing expected = {
        .status = RBRINSTRUMENTGEN3_POSTPROCESSING_STATUS_IDLE,
        .channels = {
            .count = 5,
            .channels = {
                {
                    .function = RBRINSTRUMENTGEN3_POSTPROCESSING_AGGREGATE_MEAN,
                    .label = "pressure_01"
                },
                {
                    .function =
                        RBRINSTRUMENTGEN3_POSTPROCESSING_AGGREGATE_SAMPLE_COUNT,
                    .label = "pressure_01"
                },
                {
                    .function = RBRINSTRUMENTGEN3_POSTPROCESSING_AGGREGATE_MEAN,
                    .label = "temperature_01"
                },
                {
                    .function = RBRINSTRUMENTGEN3_POSTPROCESSING_AGGREGATE_STD,
                    .label = "temperature_01"
                },
                {
                    .function = RBRINSTRUMENTGEN3_POSTPROCESSING_AGGREGATE_MEAN,
                    .label = "conductivity_01"
                }
            }
        },
        .binReference = "pressure_01",
        .binFilter = RBRINSTRUMENTGEN3_POSTPROCESSING_BINFILTER_NONE,
        .binSize = 50.0,
        .tstampMin = RBRGEN3_DATETIME_MIN,
        .tstampMax = RBRGEN3_DATETIME_MAX,
        .depthMin = 10.0,
        .depthMax = 1000.0,
        .dcAlpha = 0.08,
        .dcTau = 8.0,
        .dcTdelay = 0.35,
        .dcCtCoeff = 2.4e-4
    };
    RBRInstrumentGen3Postprocessing actual;

    const char *response = "postprocessing status = idle, channels = "
                          "mean(pressure_01)|count(pressure_01)"
                          "|mean(temperature_01)|std(temperature_01)"
                          "|mean(conductivity_01), "
                          "tstamp_min = 20000101000000, "
                          "tstamp_max = 20991231235959, "
                          "binsize = 50.0, binreference = pressure_01, "
                          "depth_min = 10.0, depth_max = 1000.0, "
                          "binfilter = none, "
                          "dc_alpha = 0.08, dc_tau = 8.000, dc_tdelay = 0.35, dc_ctcoeff = 2.4e-4" RESPONSE_TERMINATOR;
    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRInstrumentGen3_getPostprocessing(instrument,
                                                             &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(expected.status,
                        actual.status,
                        RBRInstrumentGen3PostprocessingStatus);
    TEST_ASSERT_EQ(expected.channels.count, actual.channels.count, "%" PRIi32);
    for (int i = 0; i < expected.channels.count; i++)
    {
        TEST_ASSERT_ENUM_EQ(expected.channels.channels[i].function,
                            actual.channels.channels[i].function,
                            RBRInstrumentGen3PostprocessingAggregate);
        TEST_ASSERT_STR_EQ(expected.channels.channels[i].label,
                           actual.channels.channels[i].label);
    }
    TEST_ASSERT_STR_EQ(expected.binReference, actual.binReference);
    TEST_ASSERT_ENUM_EQ(expected.binFilter,
                        actual.binFilter,
                        RBRInstrumentGen3PostprocessingBinFilter);
    TEST_ASSERT_FLOAT_EQ(expected.binSize, actual.binSize, 0.0f);
    TEST_ASSERT_EQ(expected.tstampMin, actual.tstampMin, "%" PRIi64);
    TEST_ASSERT_EQ(expected.tstampMax, actual.tstampMax, "%" PRIi64);
    TEST_ASSERT_FLOAT_EQ(expected.depthMin, actual.depthMin, 0.0f);
    TEST_ASSERT_FLOAT_EQ(expected.depthMax, actual.depthMax, 0.0f);
    TEST_ASSERT_FLOAT_EQ(expected.dcAlpha, actual.dcAlpha, 0.0f);
    TEST_ASSERT_FLOAT_EQ(expected.dcTau, actual.dcTau, 0.0f);
    TEST_ASSERT_FLOAT_EQ(expected.dcTdelay, actual.dcTdelay, 0.0f);
    TEST_ASSERT_FLOAT_EQ(expected.dcCtCoeff, actual.dcCtCoeff, 0.0f);
    TEST_ASSERT_STR_EQ("postprocessing all" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(postprocessing_set)
{
    RBRInstrumentGen3Postprocessing postprocessing = {
        .status = RBRINSTRUMENTGEN3_UNKNOWN_POSTPROCESSING_STATUS,
        .channels = {
            .count = 5,
            .channels = {
                {
                    .function = RBRINSTRUMENTGEN3_POSTPROCESSING_AGGREGATE_MEAN,
                    .label = "pressure_01"
                },
                {
                    .function =
                        RBRINSTRUMENTGEN3_POSTPROCESSING_AGGREGATE_SAMPLE_COUNT,
                    .label = "pressure_01"
                },
                {
                    .function = RBRINSTRUMENTGEN3_POSTPROCESSING_AGGREGATE_MEAN,
                    .label = "temperature_01"
                },
                {
                    .function = RBRINSTRUMENTGEN3_POSTPROCESSING_AGGREGATE_STD,
                    .label = "temperature_01"
                },
                {
                    .function = RBRINSTRUMENTGEN3_POSTPROCESSING_AGGREGATE_MEAN,
                    .label = "conductivity_01"
                }
            }
        },
        .binReference = "pressure_01",
        .binFilter = RBRINSTRUMENTGEN3_POSTPROCESSING_BINFILTER_NONE,
        .binSize = 50.0,
        .tstampMin = RBRGEN3_DATETIME_MIN,
        .tstampMax = RBRGEN3_DATETIME_MAX,
        .depthMin = 10.0,
        .depthMax = 1000.0,
        .dcAlpha = 0.08,
        .dcTau = 8.0,
        .dcTdelay = 0.35,
        .dcCtCoeff = 2.4e-4
    };

    const char *expectedCommand = "postprocessing binreference = pressure_01, "
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

    const char *response = "postprocessing binreference = pressure_01, "
                          "binfilter = none, binsize = 50.0"
                          RESPONSE_TERMINATOR
                          "postprocessing tstamp_min = 20000101000000"
                          RESPONSE_TERMINATOR
                          "postprocessing tstamp_max = 20991231235959"
                          RESPONSE_TERMINATOR
                          "postprocessing depth_min = 10.0, depth_max = 1000.0"
                          RESPONSE_TERMINATOR
                          "postprocessing dc_alpha = 0.080, dc_tau = 8.000, dc_tdelay = 0.350, dc_ctcoeff = 2.4000e-04"
                          RESPONSE_TERMINATOR
                          "postprocessing channels = mean(pressure_01)"
                          "|count(pressure_01)|mean(temperature_01)"
                          "|std(temperature_01)|mean(conductivity_01)"
                          RESPONSE_TERMINATOR;

    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRInstrumentGen3_setPostprocessing(instrument,
                                                             &postprocessing);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(postprocessing_command)
{
    const char *response = "postprocessing status = processing"
                          RESPONSE_TERMINATOR;
    TestIOBuffers_init(buffers, response, 0);

    RBRInstrumentGen3PostprocessingStatus result;
    RBRGen3Error err = RBRInstrumentGen3_setPostprocessingCommand(
        instrument,
        RBRINSTRUMENTGEN3_POSTPROCESSING_COMMAND_START,
        &result);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN3_POSTPROCESSING_STATUS_PROCESSING,
                        result,
                        RBRInstrumentGen3PostprocessingStatus);
    TEST_ASSERT_STR_EQ("postprocessing command = start" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}
