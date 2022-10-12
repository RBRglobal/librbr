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

typedef struct ThresholdingTest
{
    const char *response;
    RBRInstrumentThresholding expected;
} ThresholdingTest;

static bool test_thresholding(RBRInstrument *instrument,
                              TestIOBuffers *buffers,
                              ThresholdingTest *tests)
{
    RBRInstrumentError err;
    RBRInstrumentThresholding actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrument_getThresholding(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.enabled, actual.enabled, bool);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.state,
                            actual.state,
                            RBRInstrumentGatingState);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.channelSelection,
                            actual.channelSelection,
                            RBRInstrumentThresholdingChannelSelection);
        TEST_ASSERT_EQ(tests[i].expected.channelIndex,
                       actual.channelIndex,
                       "%" PRIi32);
        TEST_ASSERT_STR_EQ(tests[i].expected.channelLabel,
                           actual.channelLabel);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.condition,
                            actual.condition,
                            RBRInstrumentThresholdingCondition);
        TEST_ASSERT_EQ(tests[i].expected.value, actual.value, "%f");
        TEST_ASSERT_EQ(tests[i].expected.interval,
                       actual.interval,
                       "%" PRIi32);
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
                .state = RBRINSTRUMENT_UNKNOWN_GATING,
                .channelSelection = RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_INDEX,
                .channelIndex = 1,
                .channelLabel = "",
                .condition = RBRINSTRUMENT_THRESHOLDING_ABOVE,
                .value = 0.0,
                .interval = 60000
            }
        },
        {0}
    };

    return test_thresholding(instrument, buffers, tests);
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
                .state = RBRINSTRUMENT_GATING_NA,
                .channelSelection = RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_INDEX,
                .channelIndex = 1,
                .channelLabel = "temperature_00",
                .condition = RBRINSTRUMENT_THRESHOLDING_ABOVE,
                .value = 0.0,
                .interval = 60000
            }
        },
        {
            "thresholding enabled = true, state = paused, channelindex = 2, "
            "channellabel = pressure_00, condition = below, value = 600.0000, "
            "interval = 10000" RESPONSE_TERMINATOR,
            {
                .enabled = true,
                .state = RBRINSTRUMENT_GATING_PAUSED,
                .channelSelection = RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_INDEX,
                .channelIndex = 2,
                .channelLabel = "pressure_00",
                .condition = RBRINSTRUMENT_THRESHOLDING_BELOW,
                .value = 600.0,
                .interval = 10000
            }
        },
        {
            "thresholding enabled = true, state = paused, channelindex = 2, "
            "channellabel = thispressurelabelislongerthanthe31characterlimit, "
            "condition = below, value = 600.0000, interval = 10000"
            RESPONSE_TERMINATOR,
            {
                .enabled = true,
                .state = RBRINSTRUMENT_GATING_PAUSED,
                .channelSelection = RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_INDEX,
                .channelIndex = 2,
                .channelLabel = "thispressurelabelislongerthanth",
                .condition = RBRINSTRUMENT_THRESHOLDING_BELOW,
                .value = 600.0,
                .interval = 10000
            }
        },
        {0}
    };

    return test_thresholding(instrument, buffers, tests);
}

TEST_LOGGER2(thresholding_set)
{
    RBRInstrumentThresholding threshold = {
        .enabled = true,
        .channelSelection = RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_INDEX,
        .channelIndex = 1,
        .condition = RBRINSTRUMENT_THRESHOLDING_ABOVE,
        .value = 0.0,
        .interval = 60000
    };

    RBRInstrumentError err;

    const char *text = "thresholding state = on, channel = 1, "
                           "condition = above, value = 0.0000, "
                           "interval = 60000";
    char expectedCommand[1024];
    char response[1024];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    err = RBRInstrument_setThresholding(instrument, &threshold);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);
    return true;
}

TEST_LOGGER3(thresholding_set_channel_by_index)
{
    RBRInstrumentThresholding threshold = {
        .enabled = true,
        .channelSelection = RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_INDEX,
        .channelIndex = 1,
        .condition = RBRINSTRUMENT_THRESHOLDING_ABOVE,
        .value = 0.0,
        .interval = 60000
    };

    RBRInstrumentError err;

    const char *text = "thresholding enabled = true, channelindex = 1, "
                           "condition = above, value = 0.0000, "
                           "interval = 60000";
    char expectedCommand[1024];
    char response[1024];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    // const char *command = "thresholding enabled = true, channelindex = 1, "
    //                        "condition = above, value = 0.0000, "
    //                        "interval = 60000"
    //                         COMMAND_TERMINATOR;

    TestIOBuffers_init(buffers, response, 0);
    err = RBRInstrument_setThresholding(instrument, &threshold);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(thresholding_set_channel_by_label)
{
    RBRInstrumentThresholding threshold = {
        .enabled = false,
        .channelSelection = RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_LABEL,
        .channelLabel = "pressure_00",
        .condition = RBRINSTRUMENT_THRESHOLDING_BELOW,
        .value = 30.0,
        .interval = 30000
    };

    RBRInstrumentError err;

    const char *text = "thresholding enabled = false, "
                           "channellabel = pressure_00, condition = below, "
                           "value = 30.0000, interval = 30000";
    char expectedCommand[1024];
    char response[1024];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    err = RBRInstrument_setThresholding(instrument, &threshold);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

typedef struct TwistActivationTest
{
    const char *response;
    RBRInstrumentTwistActivation expected;
} TwistActivationTest;

static bool test_twistactivation(RBRInstrument *instrument,
                                 TestIOBuffers *buffers,
                                 TwistActivationTest *tests)
{
    RBRInstrumentError err;
    RBRInstrumentTwistActivation actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrument_getTwistActivation(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.enabled, actual.enabled, bool);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.state,
                            actual.state,
                            RBRInstrumentGatingState);
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
                .state = RBRINSTRUMENT_UNKNOWN_GATING
            }
        },
        {
            "twistactivation state = on, location = who cares" RESPONSE_TERMINATOR,
            {
                .enabled = true,
                .state = RBRINSTRUMENT_UNKNOWN_GATING
            }
        },
        {0}
    };

    return test_twistactivation(instrument, buffers, tests);
}

TEST_LOGGER3(twistactivation)
{
    TwistActivationTest tests[] = {
        {
            "twistactivation enabled = false, state = n/a" RESPONSE_TERMINATOR,
            {
                .enabled = false,
                .state = RBRINSTRUMENT_GATING_NA
            }
        },
        {
            "twistactivation enabled = true, state = paused"
            RESPONSE_TERMINATOR,
            {
                .enabled = true,
                .state = RBRINSTRUMENT_GATING_PAUSED
            }
        },
        {
            "twistactivation enabled = true, state = running"
            RESPONSE_TERMINATOR,
            {
                .enabled = true,
                .state = RBRINSTRUMENT_GATING_RUNNING
            }
        },
        {0}
    };

    return test_twistactivation(instrument, buffers, tests);
}

TEST_LOGGER2(twistactivation_set)
{
    RBRInstrumentTwistActivation twistActivation = {
        .enabled = true
    };

    RBRInstrumentError err;

    const char *text = "twistactivation state = on";
    char expectedCommand[1024];
    char response[1024];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    err = RBRInstrument_setTwistActivation(instrument, &twistActivation);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(twistactivation_set)
{
    RBRInstrumentTwistActivation twistActivation = {
        .enabled = true
    };

    RBRInstrumentError err;

    const char *text = "twistactivation enabled = true";
    char expectedCommand[1024];
    char response[1024];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    err = RBRInstrument_setTwistActivation(instrument, &twistActivation);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}
