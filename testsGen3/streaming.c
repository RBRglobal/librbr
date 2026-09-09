/**
 * \file streaming.c
 *
 * \brief Tests for instrument streaming commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include "tests.h"

TEST_LOGGER2(outputformat_channelslist)
{
    RBRGen3Error err;
    RBRInstrumentGen3ChannelsList channelsList;

    err = RBRInstrumentGen3_getChannelsList(instrument, &channelsList);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_UNSUPPORTED, err, RBRGen3Error);

    return true;
}

TEST_LOGGER3(outputformat_channelslist)
{
    RBRInstrumentGen3ChannelsList expected = {
        .count = 5,
        .channels = {
            {.name = "temperature", .unit = "C"},
            {.name = "pressure", .unit = "dbar"},
            {.name = "pressure", .unit = "dbar"},
            {.name = "depth", .unit = "m"},
            {.name = "measurement_count", .unit = "counts"}
        }
    };

    RBRGen3Error err;
    RBRInstrumentGen3ChannelsList actual;

    TestIOBuffers_init(
        buffers,
        "outputformat channelslist = temperature(C)|pressure(dbar)"
        "|pressure(dbar)|depth(m)|measurement_count(counts)"
        RESPONSE_TERMINATOR,
        0);
    err = RBRInstrumentGen3_getChannelsList(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(expected.count, actual.count, "%" PRIi32);

    for (int32_t channel = 0; channel < RBRGEN3_CHANNEL_MAX; channel++)
    {
        TEST_ASSERT_STR_EQ(expected.channels[channel].name,
                           actual.channels[channel].name);
        TEST_ASSERT_STR_EQ(expected.channels[channel].unit,
                           actual.channels[channel].unit);
    }

    return true;
}

TEST_LOGGER2(outputformat_labelslist)
{
    RBRGen3Error err;
    RBRInstrumentGen3LabelsList labelsList;

    err = RBRInstrumentGen3_getLabelsList(instrument, &labelsList);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_UNSUPPORTED, err, RBRGen3Error);

    return true;
}

TEST_LOGGER3(outputformat_labelslist)
{
    RBRInstrumentGen3LabelsList expected = {
        .count = 5,
        .labels = {
            "temperature_00",
            "pressure_00",
            "seapressure_00",
            "depth_00",
            "count_00"
        }
    };

    RBRGen3Error err;
    RBRInstrumentGen3LabelsList actual;

    TestIOBuffers_init(
        buffers,
        "outputformat labelslist = temperature_00|pressure_00|seapressure_00"
        "|depth_00|count_00"
        RESPONSE_TERMINATOR,
        0);
    err = RBRInstrumentGen3_getLabelsList(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(expected.count, actual.count, "%" PRIi32);

    for (int32_t label = 0; label < RBRGEN3_CHANNEL_MAX; label++)
    {
        TEST_ASSERT_STR_EQ(expected.labels[label], actual.labels[label]);
    }

    return true;
}

bool test_outputformat_support(RBRGen3 *instrument)
{
    RBRGen3Error err;
    RBRInstrumentGen3OutputFormat formats;

    err = RBRInstrumentGen3_getAvailableOutputFormats(instrument, &formats);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(
        RBRINSTRUMENTGEN3_OUTFORMAT_CALTEXT01
        | RBRINSTRUMENTGEN3_OUTFORMAT_CALTEXT02
        | RBRINSTRUMENTGEN3_OUTFORMAT_CALTEXT03
        | RBRINSTRUMENTGEN3_OUTFORMAT_CALTEXT04,
        formats,
        "0x%04X");
    return true;
}

/* test outputformat caltext07 for LOGGER3 with firmware >= 1.109. */
bool test_outputformat_support_caltext07(RBRGen3 *instrument)
{
    RBRGen3Error err;
    RBRInstrumentGen3OutputFormat formats;

    err = RBRInstrumentGen3_getAvailableOutputFormats(instrument, &formats);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    /* assuming instrument->id.fwtype==104, and id.version>=1.109. */
    TEST_ASSERT_EQ(
        RBRINSTRUMENTGEN3_OUTFORMAT_CALTEXT01
        | RBRINSTRUMENTGEN3_OUTFORMAT_CALTEXT02
        | RBRINSTRUMENTGEN3_OUTFORMAT_CALTEXT03
        | RBRINSTRUMENTGEN3_OUTFORMAT_CALTEXT04
        | RBRINSTRUMENTGEN3_OUTFORMAT_CALTEXT07,
        formats,
        "0x%04X");
    return true;
}

TEST_LOGGER2(outputformat_support)
{
    TestIOBuffers_init(
        buffers,
        "outputformat support = caltext01, caltext02, caltext03, caltext04"
        RESPONSE_TERMINATOR,
        0);
    return test_outputformat_support(instrument);
}

TEST_LOGGER3(outputformat_availabletypes)
{
    TestIOBuffers_init(
        buffers,
        "outputformat availabletypes = caltext01|caltext02|caltext03|caltext04"
        RESPONSE_TERMINATOR,
        0);
    return test_outputformat_support(instrument);
}

/* test outputformat caltext07 for LOGGER3 with firmware >= 1.109. */
TEST_LOGGER3(outputformat_availabletypes_caltext07)
{
    TestIOBuffers_init(
        buffers,
        "outputformat availabletypes = caltext01|caltext02|caltext03|caltext04|caltext07"
        RESPONSE_TERMINATOR,
        0);
    return test_outputformat_support_caltext07(instrument);
}

TEST_LOGGER3(outputformat_type)
{
    RBRGen3Error err;
    RBRInstrumentGen3OutputFormat format;

    TestIOBuffers_init(buffers,
                       "outputformat type = caltext01" RESPONSE_TERMINATOR,
                       0);
    err = RBRInstrumentGen3_getOutputFormat(instrument, &format);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN3_OUTFORMAT_CALTEXT01,
                        format,
                        RBRInstrumentGen3OutputFormat);

    return true;
}

typedef struct ToggleTest
{
    const char *response;
    bool expected;
} ToggleTest;

TEST_LOGGER3(streamusb)
{
    RBRGen3Error err;
    bool actual;

    ToggleTest tests[] = {
        {"streamusb state = on" RESPONSE_TERMINATOR, true},
        {"streamusb state = off" RESPONSE_TERMINATOR, false},
        {NULL, 0}
    };

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen3_getUSBStreamingState(instrument, &actual);
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
        {"streamserial state = on" RESPONSE_TERMINATOR, true},
        {"streamserial state = off" RESPONSE_TERMINATOR, false},
        {NULL, 0}
    };

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen3_getSerialStreamingState(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected, actual, bool);
    }

    return true;
}

TEST_LOGGER2(streamserial_aux)
{
    RBRInstrumentGen3AuxOutput expected = {
        .aux = 1,
        .enabled = false,
        .setup = 1000,
        .hold = 1000,
        .active = RBRINSTRUMENTGEN3_ACTIVE_HIGH,
        .sleep = RBRINSTRUMENTGEN3_SLEEP_TRISTATE
    };

    RBRGen3Error err;
    RBRInstrumentGen3AuxOutput actual;
    actual.aux = 1;

    TestIOBuffers_init(
        buffers,
        "streamserial aux1_state = off, aux1_setup = 1000, aux1_hold = 1000, "
        "aux1_active = high, aux1_sleep = tristate" RESPONSE_TERMINATOR,
        0);
    err = RBRInstrumentGen3_getAuxOutput(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(expected.aux, actual.aux, "%" PRIi8);
    TEST_ASSERT_ENUM_EQ(expected.enabled, actual.enabled, bool);
    TEST_ASSERT_EQ(expected.setup, actual.setup, "%" PRIi32);
    TEST_ASSERT_EQ(expected.hold, actual.hold, "%" PRIi32);
    TEST_ASSERT_ENUM_EQ(expected.active,
                        actual.active,
                        RBRInstrumentGen3AuxOutputActiveLevel);
    TEST_ASSERT_ENUM_EQ(expected.sleep,
                        actual.sleep,
                        RBRInstrumentGen3AuxOutputSleepLevel);

    return true;
}

TEST_LOGGER2(streamserial_set_aux)
{
    RBRInstrumentGen3AuxOutput auxOutput = {
        .aux = 1,
        .enabled = true,
        .setup = 500,
        .hold = 750,
        .active = RBRINSTRUMENTGEN3_ACTIVE_LOW,
        .sleep = RBRINSTRUMENTGEN3_SLEEP_HIGH
    };

    RBRGen3Error err;

    const char *text = "streamserial aux1_state = on, "
                           "aux1_setup = 500, aux1_hold = 750, "
                           "aux1_active = low, aux1_sleep = high";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    err = RBRInstrumentGen3_setAuxOutput(instrument, &auxOutput);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(streamserial_aux)
{
    RBRInstrumentGen3AuxOutput expected = {
        .aux = 1,
        .enabled = false,
        .setup = 1000,
        .hold = 1000,
        .active = RBRINSTRUMENTGEN3_ACTIVE_HIGH,
        .sleep = RBRINSTRUMENTGEN3_SLEEP_TRISTATE
    };

    RBRGen3Error err;
    RBRInstrumentGen3AuxOutput actual;
    actual.aux = 1;

    TestIOBuffers_init(
        buffers,
        "streamserial aux1_enabled = false, aux1_setup = 1000, "
        "aux1_hold = 1000, aux1_active = high, aux1_sleep = tristate"
        RESPONSE_TERMINATOR,
        0);
    err = RBRInstrumentGen3_getAuxOutput(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(expected.aux, actual.aux, "%" PRIi8);
    TEST_ASSERT_ENUM_EQ(expected.enabled, actual.enabled, bool);
    TEST_ASSERT_EQ(expected.setup, actual.setup, "%" PRIi32);
    TEST_ASSERT_EQ(expected.hold, actual.hold, "%" PRIi32);
    TEST_ASSERT_ENUM_EQ(expected.active,
                        actual.active,
                        RBRInstrumentGen3AuxOutputActiveLevel);
    TEST_ASSERT_ENUM_EQ(expected.sleep,
                        actual.sleep,
                        RBRInstrumentGen3AuxOutputSleepLevel);

    return true;
}

TEST_LOGGER3(streamserial_aux_invalid)
{
    RBRGen3Error err;
    RBRInstrumentGen3AuxOutput auxOutput;
    auxOutput.aux = 0;

    err = RBRInstrumentGen3_getAuxOutput(instrument, &auxOutput);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE,
                        err,
                        RBRGen3Error);

    return true;
}

TEST_LOGGER3(streamserial_set_aux)
{
    RBRInstrumentGen3AuxOutput auxOutput = {
        .aux = 1,
        .enabled = true,
        .setup = 500,
        .hold = 750,
        .active = RBRINSTRUMENTGEN3_ACTIVE_LOW,
        .sleep = RBRINSTRUMENTGEN3_SLEEP_HIGH
    };

    RBRGen3Error err;

    const char *text = "streamserial aux1_enabled = true, "
                           "aux1_setup = 500, aux1_hold = 750, "
                           "aux1_active = low, aux1_sleep = high";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    err = RBRInstrumentGen3_setAuxOutput(instrument, &auxOutput);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(stream_sample_parse)
{
    RBRGen3Error err;

    TestIOBuffers_init(
        buffers,
        "2018-07-26 14:56:24.000, 10.1325" RESPONSE_TERMINATOR,
        0);
    err = RBRInstrumentGen3_readSample(instrument);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(1, buffers->streamSample.channels, "%" PRIi32);
    TEST_ASSERT_EQ(10.1325, buffers->streamSample.readings[0], "%lf");

    return true;
}
