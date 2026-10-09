/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file memory.c
 *
 * \brief Tests for instrument memory commands.
 */

#include "tests.h"
#include "RBRGen3Memory.h"

TEST_LOGGER3(meminfo)
{
    RBRGen3MemoryInfo expected = {
        .dataset = RBRGEN3_DATASET_STANDARD,
        .used = 1528,
        .remaining = 134216192,
        .size = 134217728,
    };
    RBRGen3MemoryInfo actual = {
        .dataset = RBRGEN3_DATASET_STANDARD,
    };

    TestIOBuffers_init(buffers,
                       "meminfo dataset = 1, used = 1528, "
                       "remaining = 134216192, size = 134217728" RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRGen3_getMemoryInfo(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(expected.dataset, actual.dataset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.used, actual.used, "%" PRIi32);
    TEST_ASSERT_EQ(expected.remaining, actual.remaining, "%" PRIi32);
    TEST_ASSERT_EQ(expected.size, actual.size, "%" PRIi32);
    TEST_ASSERT_STR_EQ("meminfo dataset = 1" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(meminfo_invalid_dataset)
{
    RBRGen3MemoryInfo test = {
        .dataset = RBRGEN3_UNKNOWN_DATASET,
    };

    TestIOBuffers_init(buffers, "", 0);
    RBRGen3Error err = RBRGen3_getMemoryInfo(conn, &test);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE, err, RBRGen3Error);

    return true;
}

TEST_LOGGER2(read)
{
    uint8_t buf[1400];
    RBRGen3Data expected = {
        .dataset = RBRGEN3_DATASET_STANDARD,
        .size = 8,
        .offset = 2800,
        .data = buf,
    };
    RBRGen3Data actual = {
        .dataset = RBRGEN3_DATASET_STANDARD,
        .size = 1400,
        .offset = 2800,
        .data = buf,
    };

    TestIOBuffers_init(
        buffers, "data 1 8 2800" RESPONSE_TERMINATOR "AAAAAAAA\045\224" RESPONSE_TERMINATOR, 0);
    RBRGen3Error err = RBRGen3_readData(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(expected.dataset, actual.dataset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.size, actual.size, "%" PRIi32);
    TEST_ASSERT_EQ(expected.offset, actual.offset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.data, actual.data, "%p");
    TEST_ASSERT_STR_EQ("read data 1 1400 2800" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

TEST_LOGGER2(read_offset_mismatch)
{
    uint8_t buf[1400];
    RBRGen3Data actual = {
        .dataset = RBRGEN3_DATASET_STANDARD,
        .size = 1400,
        .offset = 2800,
        .data = buf,
    };

    TestIOBuffers_init(
        buffers, "data 1 8 1000" RESPONSE_TERMINATOR "AAAAAAAA\045\224" RESPONSE_TERMINATOR, 0);
    RBRGen3Error err = RBRGen3_readData(conn, &actual);
    TEST_ASSERT_STR_EQ("read data 1 1400 2800" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_COMMUNICATION_ERROR, err, RBRGen3Error);
    return true;
}

TEST_LOGGER3(readdata)
{
    uint8_t buf[1400];
    RBRGen3Data expected = {
        .dataset = RBRGEN3_DATASET_STANDARD,
        .size = 8,
        .offset = 2800,
        .data = buf,
    };
    RBRGen3Data actual = {
        .dataset = RBRGEN3_DATASET_STANDARD,
        .size = 1400,
        .offset = 2800,
        .data = buf,
    };

    TestIOBuffers_init(buffers,
                       "readdata dataset = 1, size = 8, offset = 2800" RESPONSE_TERMINATOR
                       "AAAAAAAA\045\224" RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRGen3_readData(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(expected.dataset, actual.dataset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.size, actual.size, "%" PRIi32);
    TEST_ASSERT_EQ(expected.offset, actual.offset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.data, actual.data, "%p");
    TEST_ASSERT_STR_EQ("readdata dataset = 1, size = 1400, offset = 2800" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(readdata_offset_mismatch)
{
    uint8_t buf[1400];
    RBRGen3Data actual = {
        .dataset = RBRGEN3_DATASET_STANDARD,
        .size = 1400,
        .offset = 2800,
        .data = buf,
    };

    TestIOBuffers_init(buffers,
                       "readdata dataset = 1, size = 8, offset = 1000" RESPONSE_TERMINATOR
                       "AAAAAAAA\045\024" RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRGen3_readData(conn, &actual);
    TEST_ASSERT_STR_EQ("readdata dataset = 1, size = 1400, offset = 2800" COMMAND_TERMINATOR,
                       buffers->writeBuffer);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_COMMUNICATION_ERROR, err, RBRGen3Error);
    return true;
}

TEST_LOGGER3(readdata_invalid_dataset)
{
    RBRGen3Data test = {
        .dataset = RBRGEN3_UNKNOWN_DATASET,
        .size = 0,
        .offset = 0,
        .data = NULL,
    };

    TestIOBuffers_init(buffers, "", 0);
    RBRGen3Error err = RBRGen3_readData(conn, &test);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE, err, RBRGen3Error);

    return true;
}

TEST_LOGGER3(readdata_crc_failure)
{
    uint8_t buf[1400];
    RBRGen3Data expected = {
        .dataset = RBRGEN3_DATASET_STANDARD,
        .size = 0,
        .offset = 2800,
        .data = buf,
    };
    RBRGen3Data actual = {
        .dataset = RBRGEN3_DATASET_STANDARD,
        .size = 1400,
        .offset = 2800,
        .data = buf,
    };

    TestIOBuffers_init(buffers,
                       "readdata dataset = 1, size = 8, offset = 2800" RESPONSE_TERMINATOR
                       "AAAAAAAA00" RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRGen3_readData(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_CHECKSUM_ERROR, err, RBRGen3Error);
    TEST_ASSERT_EQ(expected.dataset, actual.dataset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.size, actual.size, "%" PRIi32);
    TEST_ASSERT_EQ(expected.offset, actual.offset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.data, actual.data, "%p");
    TEST_ASSERT_STR_EQ("readdata dataset = 1, size = 1400, offset = 2800" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER2(memformat_support)
{
    RBRGen3MemoryFormat expected = RBRGEN3_MEMFORMAT_RAWBIN00 | RBRGEN3_MEMFORMAT_CALBIN00;
    RBRGen3MemoryFormat actual = RBRGEN3_MEMFORMAT_NONE;

    TestIOBuffers_init(buffers, "memformat support = rawbin00, calbin00" RESPONSE_TERMINATOR, 0);
    RBRGen3Error err = RBRGen3_getAvailableMemoryFormats(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(expected, actual, "0x%04X");
    TEST_ASSERT_STR_EQ("memformat support" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(memformat_availabletypes)
{
    RBRGen3MemoryFormat expected = RBRGEN3_MEMFORMAT_RAWBIN00 | RBRGEN3_MEMFORMAT_CALBIN00;
    RBRGen3MemoryFormat actual = RBRGEN3_MEMFORMAT_NONE;

    TestIOBuffers_init(
        buffers, "memformat availabletypes = rawbin00|calbin00" RESPONSE_TERMINATOR, 0);
    RBRGen3Error err = RBRGen3_getAvailableMemoryFormats(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(expected, actual, "0x%04X");
    TEST_ASSERT_STR_EQ("memformat availabletypes" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(memformat_type)
{
    RBRGen3MemoryFormat expected = RBRGEN3_MEMFORMAT_RAWBIN00;
    RBRGen3MemoryFormat actual = RBRGEN3_MEMFORMAT_NONE;

    TestIOBuffers_init(buffers, "memformat type = rawbin00" RESPONSE_TERMINATOR, 0);
    RBRGen3Error err = RBRGen3_getCurrentMemoryFormat(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(expected, actual, RBRGen3MemoryFormat);
    TEST_ASSERT_STR_EQ("memformat type" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(memformat_newtype)
{
    RBRGen3MemoryFormat expected = RBRGEN3_MEMFORMAT_CALBIN00;
    RBRGen3MemoryFormat actual = RBRGEN3_MEMFORMAT_NONE;

    TestIOBuffers_init(buffers, "memformat newtype = calbin00" RESPONSE_TERMINATOR, 0);
    RBRGen3Error err = RBRGen3_getNewMemoryFormat(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(expected, actual, RBRGen3MemoryFormat);
    TEST_ASSERT_STR_EQ("memformat newtype" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(memformat_newtype_set)
{
    const char *text = "memformat newtype = calbin00";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRGen3_setNewMemoryFormat(conn, RBRGEN3_MEMFORMAT_CALBIN00);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(postprocessing)
{
    RBRGen3Postprocessing expected = {
        .status = RBRGEN3_POSTPROCESSING_STATUS_IDLE,
        .channels =
            {
                .len = 5,
                .channels =
                    {
                        {
                            .function = RBRGEN3_POSTPROCESSING_AGGREGATE_MEAN,
                            .label = "pressure_01",
                        },
                        {
                            .function = RBRGEN3_POSTPROCESSING_AGGREGATE_SAMPLE_COUNT,
                            .label = "pressure_01",
                        },
                        {
                            .function = RBRGEN3_POSTPROCESSING_AGGREGATE_MEAN,
                            .label = "temperature_01",
                        },
                        {
                            .function = RBRGEN3_POSTPROCESSING_AGGREGATE_STD,
                            .label = "temperature_01",
                        },
                        {
                            .function = RBRGEN3_POSTPROCESSING_AGGREGATE_MEAN,
                            .label = "conductivity_01",
                        },
                    },
            },
        .binReference = "pressure_01",
        .binFilter = RBRGEN3_POSTPROCESSING_BINFILTER_NONE,
        .binSize = 50.0,
        .tstampMin = RBRGEN3_DATETIME_MIN,
        .tstampMax = RBRGEN3_DATETIME_MAX,
        .depthMin = 10.0,
        .depthMax = 1000.0,
        .dcAlpha = 0.08,
        .dcTau = 8.0,
        .dcTdelay = 0.35,
        .dcCtCoeff = 2.4e-4,
    };
    RBRGen3Postprocessing actual;

    const char *response = "postprocessing status = idle, channels = "
                           "mean(pressure_01)|count(pressure_01)"
                           "|mean(temperature_01)|std(temperature_01)"
                           "|mean(conductivity_01), "
                           "tstamp_min = 20000101000000, "
                           "tstamp_max = 20991231235959, "
                           "binsize = 50.0, binreference = pressure_01, "
                           "depth_min = 10.0, depth_max = 1000.0, "
                           "binfilter = none, "
                           "dc_alpha = 0.08, dc_tau = 8.000, dc_tdelay = 0.35, dc_ctcoeff = "
                           "2.4e-4" RESPONSE_TERMINATOR;
    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRGen3_getPostprocessing(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(expected.status, actual.status, RBRGen3PostprocessingStatus);
    TEST_ASSERT_EQ(expected.channels.len, actual.channels.len, "%" PRIi32);
    for (int i = 0; i < expected.channels.len; i++) {
        TEST_ASSERT_ENUM_EQ(expected.channels.channels[i].function,
                            actual.channels.channels[i].function,
                            RBRGen3PostprocessingAggregate);
        TEST_ASSERT_STR_EQ(expected.channels.channels[i].label, actual.channels.channels[i].label);
    }
    TEST_ASSERT_STR_EQ(expected.binReference, actual.binReference);
    TEST_ASSERT_ENUM_EQ(expected.binFilter, actual.binFilter, RBRGen3PostprocessingBinFilter);
    TEST_ASSERT_FLOAT_EQ(expected.binSize, actual.binSize, 0.0f);
    TEST_ASSERT_EQ(expected.tstampMin, actual.tstampMin, "%" PRIi64);
    TEST_ASSERT_EQ(expected.tstampMax, actual.tstampMax, "%" PRIi64);
    TEST_ASSERT_FLOAT_EQ(expected.depthMin, actual.depthMin, 0.0f);
    TEST_ASSERT_FLOAT_EQ(expected.depthMax, actual.depthMax, 0.0f);
    TEST_ASSERT_FLOAT_EQ(expected.dcAlpha, actual.dcAlpha, 0.0f);
    TEST_ASSERT_FLOAT_EQ(expected.dcTau, actual.dcTau, 0.0f);
    TEST_ASSERT_FLOAT_EQ(expected.dcTdelay, actual.dcTdelay, 0.0f);
    TEST_ASSERT_FLOAT_EQ(expected.dcCtCoeff, actual.dcCtCoeff, 0.0f);
    TEST_ASSERT_STR_EQ("postprocessing all" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

/* The channel list is flushed in pieces when it does not fit the command
 * buffer, and a piece which exactly fills the buffer goes into the next write
 * rather than being truncated. */
TEST_LOGGER3(postprocessingSetSplitsChannels)
{
    /* Large enough for the fixed-format lines; "postprocessing channels =
     * mean(pressure_01)|std(temperature_01)|mean(conductivity_a1234567890)"
     * is 94 characters, exactly filling the buffer with no room for
     * snprintf()'s null. */
    uint8_t commandBuffer[94];
    uint8_t responseBuffer[RBRGEN3_RESPONSE_BUFFER_DEFAULT];
    RBRGen3Environment small = conn->environment;
    small.command = commandBuffer;
    small.commandCapacity = sizeof(commandBuffer);
    small.response = responseBuffer;
    small.responseCapacity = sizeof(responseBuffer);
    RBRGen3 tiny;
    RBRGen3Error err;

    TestIOBuffers_init(
        buffers,
        "RBR RBRduo3 1.090 999999" RESPONSE_TERMINATOR
        "id model = RBRoem3, version = 1.134, serial = 999999, fwtype = 104" RESPONSE_TERMINATOR,
        0);
    err = RBRGen3_open(&tiny, &small, 0, buffers);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);

    RBRGen3Postprocessing postprocessing = {
        .status = RBRGEN3_UNKNOWN_POSTPROCESSING_STATUS,
        .channels =
            {
                .len = 3,
                .channels =
                    {
                        {
                            .function = RBRGEN3_POSTPROCESSING_AGGREGATE_MEAN,
                            .label = "pressure_01",
                        },
                        {
                            .function = RBRGEN3_POSTPROCESSING_AGGREGATE_STD,
                            .label = "temperature_01",
                        },
                        {
                            .function = RBRGEN3_POSTPROCESSING_AGGREGATE_MEAN,
                            .label = "conductivity_a1234567890",
                        },
                    },
            },
        .binReference = "pressure_01",
        .binFilter = RBRGEN3_POSTPROCESSING_BINFILTER_NONE,
        .binSize = 50.0,
        .tstampMin = RBRGEN3_DATETIME_MIN,
        .tstampMax = RBRGEN3_DATETIME_MAX,
        .depthMin = 10.0,
        .depthMax = 1000.0,
    };
    const char *expectedCommand =
        "postprocessing binreference = pressure_01, "
        "binfilter = none, binsize = 50.0" COMMAND_TERMINATOR
        "postprocessing tstamp_min = 20000101000000" COMMAND_TERMINATOR
        "postprocessing tstamp_max = 20991231235959" COMMAND_TERMINATOR
        "postprocessing depth_min = 10.0, depth_max = 1000.0" COMMAND_TERMINATOR
        "postprocessing dc_alpha = 0.000, dc_tau = 0.000, dc_tdelay = 0.000, dc_ctcoeff = "
        "0.0000e+00" COMMAND_TERMINATOR
        "postprocessing channels = mean(pressure_01)|std(temperature_01)"
        "|mean(conductivity_a1234567890)" COMMAND_TERMINATOR;
    const char *response =
        "postprocessing binreference = pressure_01, "
        "binfilter = none, binsize = 50.0" RESPONSE_TERMINATOR
        "postprocessing tstamp_min = 20000101000000" RESPONSE_TERMINATOR
        "postprocessing tstamp_max = 20991231235959" RESPONSE_TERMINATOR
        "postprocessing depth_min = 10.0, depth_max = 1000.0" RESPONSE_TERMINATOR
        "postprocessing dc_alpha = 0.000, dc_tau = 0.000, dc_tdelay = 0.000, dc_ctcoeff = "
        "0.0000e+00" RESPONSE_TERMINATOR
        "postprocessing channels = mean(pressure_01)|std(temperature_01)"
        "|mean(conductivity_a1234567890)" RESPONSE_TERMINATOR;

    TestIOBuffers_init(buffers, response, 0);
    err = RBRGen3_setPostprocessing(&tiny, &postprocessing);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(postprocessing_set)
{
    RBRGen3Postprocessing postprocessing = {
        .status = RBRGEN3_UNKNOWN_POSTPROCESSING_STATUS,
        .channels =
            {
                .len = 5,
                .channels =
                    {
                        {
                            .function = RBRGEN3_POSTPROCESSING_AGGREGATE_MEAN,
                            .label = "pressure_01",
                        },
                        {
                            .function = RBRGEN3_POSTPROCESSING_AGGREGATE_SAMPLE_COUNT,
                            .label = "pressure_01",
                        },
                        {
                            .function = RBRGEN3_POSTPROCESSING_AGGREGATE_MEAN,
                            .label = "temperature_01",
                        },
                        {
                            .function = RBRGEN3_POSTPROCESSING_AGGREGATE_STD,
                            .label = "temperature_01",
                        },
                        {
                            .function = RBRGEN3_POSTPROCESSING_AGGREGATE_MEAN,
                            .label = "conductivity_01",
                        },
                    },
            },
        .binReference = "pressure_01",
        .binFilter = RBRGEN3_POSTPROCESSING_BINFILTER_NONE,
        .binSize = 50.0,
        .tstampMin = RBRGEN3_DATETIME_MIN,
        .tstampMax = RBRGEN3_DATETIME_MAX,
        .depthMin = 10.0,
        .depthMax = 1000.0,
        .dcAlpha = 0.08,
        .dcTau = 8.0,
        .dcTdelay = 0.35,
        .dcCtCoeff = 2.4e-4,
    };

    const char *expectedCommand =
        "postprocessing binreference = pressure_01, "
        "binfilter = none, binsize = 50.0" COMMAND_TERMINATOR
        "postprocessing tstamp_min = 20000101000000" COMMAND_TERMINATOR
        "postprocessing tstamp_max = 20991231235959" COMMAND_TERMINATOR
        "postprocessing depth_min = 10.0, depth_max = 1000.0" COMMAND_TERMINATOR
        "postprocessing dc_alpha = 0.080, dc_tau = 8.000, dc_tdelay = 0.350, dc_ctcoeff = "
        "2.4000e-04" COMMAND_TERMINATOR "postprocessing channels = mean(pressure_01)"
        "|count(pressure_01)|mean(temperature_01)"
        "|std(temperature_01)|mean(conductivity_01)" COMMAND_TERMINATOR;

    const char *response =
        "postprocessing binreference = pressure_01, "
        "binfilter = none, binsize = 50.0" RESPONSE_TERMINATOR
        "postprocessing tstamp_min = 20000101000000" RESPONSE_TERMINATOR
        "postprocessing tstamp_max = 20991231235959" RESPONSE_TERMINATOR
        "postprocessing depth_min = 10.0, depth_max = 1000.0" RESPONSE_TERMINATOR
        "postprocessing dc_alpha = 0.080, dc_tau = 8.000, dc_tdelay = 0.350, dc_ctcoeff = "
        "2.4000e-04" RESPONSE_TERMINATOR "postprocessing channels = mean(pressure_01)"
        "|count(pressure_01)|mean(temperature_01)"
        "|std(temperature_01)|mean(conductivity_01)" RESPONSE_TERMINATOR;

    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRGen3_setPostprocessing(conn, &postprocessing);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

/* A channel list of exactly RBRGEN3_POSTPROCESSING_CHANNEL_MAX entries is
 * sent; one more is refused before anything is sent. */
TEST_LOGGER3(postprocessingSetChannelsMax)
{
    RBRGen3Postprocessing postprocessing = {
        .status = RBRGEN3_UNKNOWN_POSTPROCESSING_STATUS,
        .binReference = "pressure_01",
        .binFilter = RBRGEN3_POSTPROCESSING_BINFILTER_NONE,
        .binSize = 50.0,
        .tstampMin = RBRGEN3_DATETIME_MIN,
        .tstampMax = RBRGEN3_DATETIME_MAX,
        .depthMin = 10.0,
        .depthMax = 1000.0,
    };
    const char *fixedLines = "postprocessing binreference = pressure_01, "
                             "binfilter = none, binsize = 50.0\n"
                             "postprocessing tstamp_min = 20000101000000\n"
                             "postprocessing tstamp_max = 20991231235959\n"
                             "postprocessing depth_min = 10.0, depth_max = 1000.0\n"
                             "postprocessing dc_alpha = 0.000, dc_tau = 0.000, "
                             "dc_tdelay = 0.000, dc_ctcoeff = 0.0000e+00\n";
    char channels[512] = "postprocessing channels =";
    for (int32_t i = 0; i < RBRGEN3_POSTPROCESSING_CHANNEL_MAX; i++) {
        postprocessing.channels.channels[i].function = RBRGEN3_POSTPROCESSING_AGGREGATE_MEAN;
        snprintf(postprocessing.channels.channels[i].label,
                 sizeof(postprocessing.channels.channels[i].label),
                 "c%02" PRIi32,
                 i);
        snprintf(channels + strlen(channels),
                 sizeof(channels) - strlen(channels),
                 "%cmean(c%02" PRIi32 ")",
                 i == 0 ? ' ' : '|',
                 i);
    }
    postprocessing.channels.len = RBRGEN3_POSTPROCESSING_CHANNEL_MAX;

    /* The same lines, terminated for the command and for the response. */
    char expectedCommand[1024] = "";
    char response[1024] = "";
    char lines[1024];
    snprintf(lines, sizeof(lines), "%s%s\n", fixedLines, channels);
    for (char *line = strtok(lines, "\n"); line != NULL; line = strtok(NULL, "\n")) {
        snprintf(expectedCommand + strlen(expectedCommand),
                 sizeof(expectedCommand) - strlen(expectedCommand),
                 "%s" COMMAND_TERMINATOR,
                 line);
        snprintf(response + strlen(response),
                 sizeof(response) - strlen(response),
                 "%s" RESPONSE_TERMINATOR,
                 line);
    }

    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRGen3_setPostprocessing(conn, &postprocessing);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    postprocessing.channels.len = RBRGEN3_POSTPROCESSING_CHANNEL_MAX + 1;
    TestIOBuffers_init(buffers, "", 0);
    err = RBRGen3_setPostprocessing(conn, &postprocessing);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

/* Channels past RBRGEN3_POSTPROCESSING_CHANNEL_MAX are dropped, not written
 * past the list over the fields which follow it, and parameters after the
 * channel list are still read. */
TEST_LOGGER3(postprocessingChannelsTruncated)
{
    char response[1024] = "postprocessing status = idle, binreference = pressure_01, channels =";
    for (int32_t i = 0; i <= RBRGEN3_POSTPROCESSING_CHANNEL_MAX; i++) {
        snprintf(response + strlen(response),
                 sizeof(response) - strlen(response),
                 "%cmean(c%02" PRIi32 ")",
                 i == 0 ? ' ' : '|',
                 i);
    }
    snprintf(response + strlen(response),
             sizeof(response) - strlen(response),
             ", binsize = 50.0" RESPONSE_TERMINATOR);
    RBRGen3Postprocessing actual;

    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRGen3_getPostprocessing(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_TRUNCATED, err, RBRGen3Error);
    TEST_ASSERT_EQ(RBRGEN3_POSTPROCESSING_CHANNEL_MAX, actual.channels.len, "%" PRIi32);
    TEST_ASSERT_STR_EQ("c00", actual.channels.channels[0].label);
    TEST_ASSERT_STR_EQ("c23",
                       actual.channels.channels[RBRGEN3_POSTPROCESSING_CHANNEL_MAX - 1].label);
    TEST_ASSERT_STR_EQ("pressure_01", actual.binReference);
    TEST_ASSERT_FLOAT_EQ(50.0f, actual.binSize, 0.0f);

    return true;
}

TEST_LOGGER3(postprocessing_command)
{
    const char *response = "postprocessing status = processing" RESPONSE_TERMINATOR;
    TestIOBuffers_init(buffers, response, 0);

    RBRGen3PostprocessingStatus result;
    RBRGen3Error err =
        RBRGen3_setPostprocessingCommand(conn, RBRGEN3_POSTPROCESSING_COMMAND_START, &result);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(
        RBRGEN3_POSTPROCESSING_STATUS_PROCESSING, result, RBRGen3PostprocessingStatus);
    TEST_ASSERT_STR_EQ("postprocessing command = start" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}
