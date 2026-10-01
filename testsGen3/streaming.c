/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file streaming.c
 *
 * \brief Tests for instrument streaming commands.
 */

#include "tests.h"
#include "RBRGen3Streaming.h"

TEST_LOGGER2(outputformat_channelslist)
{
    RBRGen3Error err;
    RBRGEN3_CHANNELS_LIST_DECL(channelsList, TESTS_CHANNEL_MAX);

    err = RBRGen3_getChannelsList(conn, &channelsList);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_UNSUPPORTED, err, RBRGen3Error);

    return true;
}

TEST_LOGGER3(outputformat_channelslist)
{
    RBRGen3ChannelsList expected = {
        .len = 5,
        .channels =
            (RBRGen3ChannelsListEntry[]) {
                {
                    .name = "temperature",
                    .unit = "C",
                },
                {
                    .name = "pressure",
                    .unit = "dbar",
                },
                {
                    .name = "pressure",
                    .unit = "dbar",
                },
                {
                    .name = "depth",
                    .unit = "m",
                },
                {
                    .name = "measurement_count",
                    .unit = "counts",
                },
            },
    };

    RBRGen3Error err;
    RBRGEN3_CHANNELS_LIST_DECL(actual, TESTS_CHANNEL_MAX);

    TestIOBuffers_init(buffers,
                       "outputformat channelslist = temperature(C)|pressure(dbar)"
                       "|pressure(dbar)|depth(m)|measurement_count(counts)" RESPONSE_TERMINATOR,
                       0);
    err = RBRGen3_getChannelsList(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(expected.len, actual.len, "%" PRIi32);

    for (int32_t channel = 0; channel < expected.len; channel++) {
        TEST_ASSERT_STR_EQ(expected.channels[channel].name, actual.channels[channel].name);
        TEST_ASSERT_STR_EQ(expected.channels[channel].unit, actual.channels[channel].unit);
    }
    /* Entries past the length are cleared. */
    for (int32_t channel = expected.len; channel < actual.size; channel++) {
        TEST_ASSERT_STR_EQ("", actual.channels[channel].name);
        TEST_ASSERT_STR_EQ("", actual.channels[channel].unit);
    }

    return true;
}

TEST_LOGGER2(outputformat_labelslist)
{
    RBRGen3Error err;
    RBRGEN3_LABELS_LIST_DECL(labelsList, TESTS_CHANNEL_MAX);

    err = RBRGen3_getLabelsList(conn, &labelsList);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_UNSUPPORTED, err, RBRGen3Error);

    return true;
}

TEST_LOGGER3(outputformat_labelslist)
{
    RBRGen3LabelsList expected = {
        .len = 5,
        .labels =
            (RBRGen3Label[]) {
                "temperature_00",
                "pressure_00",
                "seapressure_00",
                "depth_00",
                "count_00",
            },
    };

    RBRGen3Error err;
    RBRGEN3_LABELS_LIST_DECL(actual, TESTS_CHANNEL_MAX);

    TestIOBuffers_init(buffers,
                       "outputformat labelslist = temperature_00|pressure_00|seapressure_00"
                       "|depth_00|count_00" RESPONSE_TERMINATOR,
                       0);
    err = RBRGen3_getLabelsList(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(expected.len, actual.len, "%" PRIi32);

    for (int32_t label = 0; label < expected.len; label++) {
        TEST_ASSERT_STR_EQ(expected.labels[label], actual.labels[label]);
    }
    /* Entries past the length are cleared. */
    for (int32_t label = expected.len; label < actual.size; label++) {
        TEST_ASSERT_STR_EQ("", actual.labels[label]);
    }

    return true;
}

/* Lists shorter than the instrument's channel count keep what fits, report
 * the true count, and are flagged. Nothing is written past the storage. */
TEST_LOGGER3(outputformat_channelslist_truncated)
{
    RBRGen3Error err;
    /* Two entries of storage with a guard entry behind them. */
    RBRGen3ChannelsListEntry storage[3] = {{
                                               .name = "",
                                               .unit = "",
                                           },
                                           {
                                               .name = "",
                                               .unit = "",
                                           },
                                           {
                                               .name = "guard",
                                               .unit = "g",
                                           }};
    RBRGen3ChannelsList actual = {.size = 2, .channels = storage};

    TestIOBuffers_init(buffers,
                       "outputformat channelslist = temperature(C)|pressure(dbar)"
                       "|depth(m)" RESPONSE_TERMINATOR,
                       0);
    err = RBRGen3_getChannelsList(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_TRUNCATED, err, RBRGen3Error);
    TEST_ASSERT_EQ(2, actual.len, "%" PRIi32);
    TEST_ASSERT_STR_EQ("temperature", actual.channels[0].name);
    TEST_ASSERT_STR_EQ("pressure", actual.channels[1].name);
    TEST_ASSERT_STR_EQ("dbar", actual.channels[1].unit);
    TEST_ASSERT_STR_EQ("guard", storage[2].name);

    return true;
}

TEST_LOGGER3(outputformat_labelslist_truncated)
{
    RBRGen3Error err;
    /* Two labels of storage with a guard label behind them. */
    RBRGen3Label storage[3] = {"", "", "guard"};
    RBRGen3LabelsList actual = {.size = 2, .labels = storage};

    TestIOBuffers_init(buffers,
                       "outputformat labelslist = temperature_00|pressure_00"
                       "|depth_00" RESPONSE_TERMINATOR,
                       0);
    err = RBRGen3_getLabelsList(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_TRUNCATED, err, RBRGen3Error);
    TEST_ASSERT_EQ(2, actual.len, "%" PRIi32);
    TEST_ASSERT_STR_EQ("temperature_00", actual.labels[0]);
    TEST_ASSERT_STR_EQ("pressure_00", actual.labels[1]);
    TEST_ASSERT_STR_EQ("guard", storage[2]);

    return true;
}

/* A list without storage is refused before anything is sent. */
TEST_LOGGER3(outputformat_lists_reject_missing_storage)
{
    RBRGen3Error err;
    RBRGen3ChannelsListEntry entry;
    RBRGen3ChannelsList noChannels = {.size = 4, .channels = NULL};
    RBRGen3ChannelsList noChannelsSize = {.size = 0, .channels = &entry};
    RBRGen3LabelsList noLabels = {.size = 4, .labels = NULL};
    RBRGen3LabelsList noLabelsSize = {.size = 0, .labels = (RBRGen3Label[]) {"x"}};

    TestIOBuffers_init(buffers, "", 0);
    err = RBRGen3_getChannelsList(conn, &noChannels);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE, err, RBRGen3Error);
    err = RBRGen3_getChannelsList(conn, &noChannelsSize);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE, err, RBRGen3Error);
    err = RBRGen3_getLabelsList(conn, &noLabels);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE, err, RBRGen3Error);
    err = RBRGen3_getLabelsList(conn, &noLabelsSize);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

bool test_outputformat_support(RBRGen3 *conn)
{
    RBRGen3Error err;
    RBRGen3OutputFormat formats;

    err = RBRGen3_getAvailableOutputFormats(conn, &formats);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(RBRGEN3_OUTFORMAT_CALTEXT01 | RBRGEN3_OUTFORMAT_CALTEXT02 |
                       RBRGEN3_OUTFORMAT_CALTEXT03 | RBRGEN3_OUTFORMAT_CALTEXT04,
                   formats,
                   "0x%04X");
    return true;
}

/* test outputformat caltext07 for LOGGER3 with firmware >= 1.109. */
bool test_outputformat_support_caltext07(RBRGen3 *conn)
{
    RBRGen3Error err;
    RBRGen3OutputFormat formats;

    err = RBRGen3_getAvailableOutputFormats(conn, &formats);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    /* assuming instrument->id.fwType==104, and id.version>=1.109. */
    TEST_ASSERT_EQ(RBRGEN3_OUTFORMAT_CALTEXT01 | RBRGEN3_OUTFORMAT_CALTEXT02 |
                       RBRGEN3_OUTFORMAT_CALTEXT03 | RBRGEN3_OUTFORMAT_CALTEXT04 |
                       RBRGEN3_OUTFORMAT_CALTEXT07,
                   formats,
                   "0x%04X");
    return true;
}

TEST_LOGGER2(outputformat_support)
{
    TestIOBuffers_init(
        buffers,
        "outputformat support = caltext01, caltext02, caltext03, caltext04" RESPONSE_TERMINATOR,
        0);
    return test_outputformat_support(conn);
}

TEST_LOGGER3(outputformat_availabletypes)
{
    TestIOBuffers_init(
        buffers,
        "outputformat availabletypes = caltext01|caltext02|caltext03|caltext04" RESPONSE_TERMINATOR,
        0);
    return test_outputformat_support(conn);
}

/* test outputformat caltext07 for LOGGER3 with firmware >= 1.109. */
TEST_LOGGER3(outputformat_availabletypes_caltext07)
{
    TestIOBuffers_init(buffers,
                       "outputformat availabletypes = "
                       "caltext01|caltext02|caltext03|caltext04|caltext07" RESPONSE_TERMINATOR,
                       0);
    return test_outputformat_support_caltext07(conn);
}

TEST_LOGGER3(outputformat_type)
{
    RBRGen3Error err;
    RBRGen3OutputFormat format;

    TestIOBuffers_init(buffers, "outputformat type = caltext01" RESPONSE_TERMINATOR, 0);
    err = RBRGen3_getOutputFormat(conn, &format);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_OUTFORMAT_CALTEXT01, format, RBRGen3OutputFormat);

    return true;
}

typedef struct ToggleTest {
    const char *response;
    bool expected;
} ToggleTest;

TEST_LOGGER3(streamusb)
{
    RBRGen3Error err;
    bool actual;

    ToggleTest tests[] = {
        {
            .response = "streamusb state = on" RESPONSE_TERMINATOR,
            .expected = true,
        },
        {
            .response = "streamusb state = off" RESPONSE_TERMINATOR,
            .expected = false,
        },
        {0},
    };

    for (int i = 0; tests[i].response != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_getUSBStreamingState(conn, &actual);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected, actual, bool);
    }

    return true;
}

TEST_LOGGER3(streamserial)
{
    RBRGen3Error err;
    bool actual;

    ToggleTest tests[] = {
        {
            .response = "streamserial state = on" RESPONSE_TERMINATOR,
            .expected = true,
        },
        {
            .response = "streamserial state = off" RESPONSE_TERMINATOR,
            .expected = false,
        },
        {0},
    };

    for (int i = 0; tests[i].response != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_getSerialStreamingState(conn, &actual);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected, actual, bool);
    }

    return true;
}

TEST_LOGGER2(streamserial_aux)
{
    RBRGen3AuxOutput expected = {
        .aux = 1,
        .enabled = false,
        .setup = 1000,
        .hold = 1000,
        .active = RBRGEN3_ACTIVE_HIGH,
        .sleep = RBRGEN3_SLEEP_TRISTATE,
    };

    RBRGen3Error err;
    RBRGen3AuxOutput actual;
    actual.aux = 1;

    TestIOBuffers_init(buffers,
                       "streamserial aux1_state = off, aux1_setup = 1000, aux1_hold = 1000, "
                       "aux1_active = high, aux1_sleep = tristate" RESPONSE_TERMINATOR,
                       0);
    err = RBRGen3_getAuxOutput(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(expected.aux, actual.aux, "%" PRIi8);
    TEST_ASSERT_ENUM_EQ(expected.enabled, actual.enabled, bool);
    TEST_ASSERT_EQ(expected.setup, actual.setup, "%" PRIi32);
    TEST_ASSERT_EQ(expected.hold, actual.hold, "%" PRIi32);
    TEST_ASSERT_ENUM_EQ(expected.active, actual.active, RBRGen3AuxOutputActiveLevel);
    TEST_ASSERT_ENUM_EQ(expected.sleep, actual.sleep, RBRGen3AuxOutputSleepLevel);

    return true;
}

TEST_LOGGER2(streamserial_set_aux)
{
    RBRGen3AuxOutput auxOutput = {
        .aux = 1,
        .enabled = true,
        .setup = 500,
        .hold = 750,
        .active = RBRGEN3_ACTIVE_LOW,
        .sleep = RBRGEN3_SLEEP_HIGH,
    };

    RBRGen3Error err;

    const char *text = "streamserial aux1_state = on, "
                       "aux1_setup = 500, aux1_hold = 750, "
                       "aux1_active = low, aux1_sleep = high";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    err = RBRGen3_setAuxOutput(conn, &auxOutput);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(streamserial_aux)
{
    RBRGen3AuxOutput expected = {
        .aux = 1,
        .enabled = false,
        .setup = 1000,
        .hold = 1000,
        .active = RBRGEN3_ACTIVE_HIGH,
        .sleep = RBRGEN3_SLEEP_TRISTATE,
    };

    RBRGen3Error err;
    RBRGen3AuxOutput actual;
    actual.aux = 1;

    TestIOBuffers_init(
        buffers,
        "streamserial aux1_enabled = false, aux1_setup = 1000, "
        "aux1_hold = 1000, aux1_active = high, aux1_sleep = tristate" RESPONSE_TERMINATOR,
        0);
    err = RBRGen3_getAuxOutput(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(expected.aux, actual.aux, "%" PRIi8);
    TEST_ASSERT_ENUM_EQ(expected.enabled, actual.enabled, bool);
    TEST_ASSERT_EQ(expected.setup, actual.setup, "%" PRIi32);
    TEST_ASSERT_EQ(expected.hold, actual.hold, "%" PRIi32);
    TEST_ASSERT_ENUM_EQ(expected.active, actual.active, RBRGen3AuxOutputActiveLevel);
    TEST_ASSERT_ENUM_EQ(expected.sleep, actual.sleep, RBRGen3AuxOutputSleepLevel);

    return true;
}

TEST_LOGGER3(streamserial_aux_invalid)
{
    RBRGen3Error err;
    RBRGen3AuxOutput auxOutput;
    auxOutput.aux = 0;

    err = RBRGen3_getAuxOutput(conn, &auxOutput);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE, err, RBRGen3Error);

    return true;
}

TEST_LOGGER3(streamserial_set_aux)
{
    RBRGen3AuxOutput auxOutput = {
        .aux = 1,
        .enabled = true,
        .setup = 500,
        .hold = 750,
        .active = RBRGEN3_ACTIVE_LOW,
        .sleep = RBRGEN3_SLEEP_HIGH,
    };

    RBRGen3Error err;

    const char *text = "streamserial aux1_enabled = true, "
                       "aux1_setup = 500, aux1_hold = 750, "
                       "aux1_active = low, aux1_sleep = high";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    err = RBRGen3_setAuxOutput(conn, &auxOutput);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(stream_sample_parse)
{
    RBRGen3Error err;

    TestIOBuffers_init(buffers, "2018-07-26 14:56:24.000, 10.1325" RESPONSE_TERMINATOR, 0);
    err = RBRGen3_readSample(conn);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(1, buffers->streamSample.channelCount, "%" PRIi32);
    TEST_ASSERT_EQ(10.1325, buffers->streamSample.readings[0], "%lf");

    return true;
}
