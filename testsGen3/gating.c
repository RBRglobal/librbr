/**
 * \file gating.c
 *
 * \brief Tests for instrument gating commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include "tests.h"

typedef struct ThresholdingTest {
    const char *response;
    RBRGen3Thresholding expected;
} ThresholdingTest;

static bool test_thresholding(RBRGen3 *conn, TestIOBuffers *buffers, ThresholdingTest *tests)
{
    RBRGen3Error err;
    RBRGen3Thresholding actual;

    for (int i = 0; tests[i].response != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_getThresholding(conn, &actual);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.enabled, actual.enabled, bool);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.state, actual.state, RBRGen3GatingState);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.channelSelection,
                            actual.channelSelection,
                            RBRGen3ThresholdingChannelSelection);
        TEST_ASSERT_EQ(tests[i].expected.channelIndex, actual.channelIndex, "%" PRIi32);
        TEST_ASSERT_STR_EQ(tests[i].expected.channelLabel, actual.channelLabel);
        TEST_ASSERT_ENUM_EQ(
            tests[i].expected.condition, actual.condition, RBRGen3ThresholdingCondition);
        TEST_ASSERT_FLOAT_EQ(tests[i].expected.value, actual.value, 0.0f);
        TEST_ASSERT_EQ(tests[i].expected.interval, actual.interval, "%" PRIi32);
    }

    return true;
}

TEST_LOGGER2(thresholding)
{
    ThresholdingTest tests[] = {
        {
            "thresholding state = off, channel = 1, condition = above, "
            "value = 0.0000, interval = 60000" RESPONSE_TERMINATOR,
            {
                .enabled = false,
                .state = RBRGEN3_UNKNOWN_GATING,
                .channelSelection = RBRGEN3_THRESHOLD_CHANNEL_BY_INDEX,
                .channelIndex = 1,
                .channelLabel = "",
                .condition = RBRGEN3_THRESHOLDING_ABOVE,
                .value = 0.0,
                .interval = 60000,
            },
        },
        {0},
    };

    return test_thresholding(conn, buffers, tests);
}

TEST_LOGGER3(thresholding)
{
    ThresholdingTest tests[] = {
        {
            "thresholding enabled = false, state = n/a, channelindex = 1, "
            "channellabel = temperature_00, condition = above, value = 0.0000, "
            "interval = 60000" RESPONSE_TERMINATOR,
            {
                .enabled = false,
                .state = RBRGEN3_GATING_NA,
                .channelSelection = RBRGEN3_THRESHOLD_CHANNEL_BY_INDEX,
                .channelIndex = 1,
                .channelLabel = "temperature_00",
                .condition = RBRGEN3_THRESHOLDING_ABOVE,
                .value = 0.0,
                .interval = 60000,
            },
        },
        {
            "thresholding enabled = true, state = paused, channelindex = 2, "
            "channellabel = pressure_00, condition = below, value = 600.0000, "
            "interval = 10000" RESPONSE_TERMINATOR,
            {
                .enabled = true,
                .state = RBRGEN3_GATING_PAUSED,
                .channelSelection = RBRGEN3_THRESHOLD_CHANNEL_BY_INDEX,
                .channelIndex = 2,
                .channelLabel = "pressure_00",
                .condition = RBRGEN3_THRESHOLDING_BELOW,
                .value = 600.0,
                .interval = 10000,
            },
        },
        {
            "thresholding enabled = true, state = paused, channelindex = 2, "
            "channellabel = thispressurelabelislongerthanthe31characterlimit, "
            "condition = below, value = 600.0000, interval = 10000" RESPONSE_TERMINATOR,
            {
                .enabled = true,
                .state = RBRGEN3_GATING_PAUSED,
                .channelSelection = RBRGEN3_THRESHOLD_CHANNEL_BY_INDEX,
                .channelIndex = 2,
                .channelLabel = "thispressurelabelislongerthanth",
                .condition = RBRGEN3_THRESHOLDING_BELOW,
                .value = 600.0,
                .interval = 10000,
            },
        },
        {0},
    };

    return test_thresholding(conn, buffers, tests);
}

TEST_LOGGER2(thresholding_set)
{
    RBRGen3Thresholding threshold = {
        .enabled = true,
        .channelSelection = RBRGEN3_THRESHOLD_CHANNEL_BY_INDEX,
        .channelIndex = 1,
        .condition = RBRGEN3_THRESHOLDING_ABOVE,
        .value = 0.0,
        .interval = 60000,
    };

    RBRGen3Error err;

    const char *text = "thresholding state = on, channel = 1, "
                       "condition = above, value = 0.0000, "
                       "interval = 60000";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    err = RBRGen3_setThresholding(conn, &threshold);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);
    return true;
}

TEST_LOGGER3(thresholding_set_channel_by_index)
{
    RBRGen3Thresholding threshold = {
        .enabled = true,
        .channelSelection = RBRGEN3_THRESHOLD_CHANNEL_BY_INDEX,
        .channelIndex = 1,
        .condition = RBRGEN3_THRESHOLDING_ABOVE,
        .value = 0.0,
        .interval = 60000,
    };

    RBRGen3Error err;

    const char *text = "thresholding enabled = true, channelindex = 1, "
                       "condition = above, value = 0.0000, "
                       "interval = 60000";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    err = RBRGen3_setThresholding(conn, &threshold);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(thresholding_set_channel_by_label)
{
    RBRGen3Thresholding threshold = {
        .enabled = false,
        .channelSelection = RBRGEN3_THRESHOLD_CHANNEL_BY_LABEL,
        .channelLabel = "pressure_00",
        .condition = RBRGEN3_THRESHOLDING_BELOW,
        .value = 30.0,
        .interval = 30000,
    };

    RBRGen3Error err;

    const char *text = "thresholding enabled = false, "
                       "channellabel = pressure_00, condition = below, "
                       "value = 30.0000, interval = 30000";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    err = RBRGen3_setThresholding(conn, &threshold);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

typedef struct TwistActivationTest {
    const char *response;
    RBRGen3TwistActivation expected;
} TwistActivationTest;

static bool test_twistactivation(RBRGen3 *conn, TestIOBuffers *buffers, TwistActivationTest *tests)
{
    RBRGen3Error err;
    RBRGen3TwistActivation actual;

    for (int i = 0; tests[i].response != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_getTwistActivation(conn, &actual);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.enabled, actual.enabled, bool);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.state, actual.state, RBRGen3GatingState);
    }

    return true;
}

TEST_LOGGER2(twistactivation)
{
    TwistActivationTest tests[] = {
        {
            "twistactivation state = off, location = off" RESPONSE_TERMINATOR,
            {
                .enabled = false,
                .state = RBRGEN3_UNKNOWN_GATING,
            },
        },
        {
            "twistactivation state = on, location = who cares" RESPONSE_TERMINATOR,
            {
                .enabled = true,
                .state = RBRGEN3_UNKNOWN_GATING,
            },
        },
        {0},
    };

    return test_twistactivation(conn, buffers, tests);
}

TEST_LOGGER3(twistactivation)
{
    TwistActivationTest tests[] = {
        {
            "twistactivation enabled = false, state = n/a" RESPONSE_TERMINATOR,
            {
                .enabled = false,
                .state = RBRGEN3_GATING_NA,
            },
        },
        {
            "twistactivation enabled = true, state = paused" RESPONSE_TERMINATOR,
            {
                .enabled = true,
                .state = RBRGEN3_GATING_PAUSED,
            },
        },
        {
            "twistactivation enabled = true, state = running" RESPONSE_TERMINATOR,
            {
                .enabled = true,
                .state = RBRGEN3_GATING_RUNNING,
            },
        },
        {0},
    };

    return test_twistactivation(conn, buffers, tests);
}

TEST_LOGGER2(twistactivation_set)
{
    RBRGen3TwistActivation twistActivation = {
        .enabled = true,
    };

    RBRGen3Error err;

    const char *text = "twistactivation state = on";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    err = RBRGen3_setTwistActivation(conn, &twistActivation);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(twistactivation_set)
{
    RBRGen3TwistActivation twistActivation = {
        .enabled = true,
    };

    RBRGen3Error err;

    const char *text = "twistactivation enabled = true";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    err = RBRGen3_setTwistActivation(conn, &twistActivation);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}
