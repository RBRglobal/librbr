/**
 * \file gating.c
 *
 * \brief Tests for instrument fetching commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include <math.h>
#include "RBRInstrumentGen4Polling.h"
#include "tests.h"

typedef struct PollAllChannelsTest
{
    const char *expectedCommand;
    const char *response;
    RBRInstrumentGen4OutputFormat outputFormat;
    RBRInstrumentGen4Sample expected;
} PollAllChannelsTest;

static bool test_pollAllChannels(RBRInstrumentGen4 *instrument,
                          TestIOBuffers *buffers,
                          PollAllChannelsTest *tests)
{
    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Sample actual;

    for (int i = 0; tests[i].expectedCommand != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        if (i == 0)
        {
          instrument->lastActivityTime = 0;
        }
        instrument->outputFormat=tests[i].outputFormat;
        err = RBRInstrumentGen4_pollAllChannels(instrument,
                                  &actual);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].expectedCommand, buffers->writeBuffer);
        TEST_ASSERT_EQ(tests[i].expected.timestamp,
                       actual.timestamp,
                       "%" PRIi64);
        TEST_ASSERT_EQ(tests[i].expected.channels,
                       actual.channels,
                       "%" PRIi32);
        for (int32_t channel = 0; channel < actual.channels; ++channel)
        {
            TEST_ASSERT_ENUM_EQ(
                RBRInstrumentGen4Reading_getFlag(
                    tests[i].expected.readings[channel]),
                RBRInstrumentGen4Reading_getFlag(actual.readings[channel]),
                RBRInstrumentGen4ReadingFlag);

            switch (RBRInstrumentGen4Reading_getFlag(actual.readings[channel]))
            {
            case RBRINSTRUMENTGEN4_READING_FLAG_UNCALIBRATED:
            case RBRINSTRUMENTGEN4_READING_FLAG_ERROR:
                TEST_ASSERT_EQ(
                    RBRInstrumentGen4Reading_getError(
                        tests[i].expected.readings[channel]),
                    RBRInstrumentGen4Reading_getError(actual.readings[channel]),
                    "%" PRIi8);
                break;
            case RBRINSTRUMENTGEN4_READING_FLAG_NONE:
            default:
                TEST_ASSERT_EQ(tests[i].expected.readings[channel],
                               actual.readings[channel],
                               "%lf");
            }
        }
    }

    return true;
}

TEST_LOGGER4(poll_all_channels)
{
    PollAllChannelsTest tests[] = {
        {
            "poll channellist=all" COMMAND_TERMINATOR,
            "polling 2000-01-01 03:22:42.000 -129.993424e+000 349.649536e-003 "
            "500.022304e-003" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_SCHEDULELABEL
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP,
            {
                .timestamp = 946696962000LL,
                .channels = 3,
                .readings = {
                    -129.993424,
                    0.349649536,
                    0.500022304
                }
            }
        },
        {
            "poll channellist=all" COMMAND_TERMINATOR,
            "RBR 999999 polling 2000-01-01 03:22:42.000 -129.993424e+000 349.649536e-003 "
            "500.022304e-003" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_SCHEDULELABEL
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_SERIAL,
            {
                .timestamp = 946696962000LL,
                .channels = 3,
                .readings = {
                    -129.993424,
                    0.349649536,
                    0.500022304
                }
            }
        },
        {
            "poll channellist=all" COMMAND_TERMINATOR,
            "polling 2000-01-01 03:22:42.000 -129.993424e+000 349.649536e-003 "
            "500.022304e-003" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_SCHEDULELABEL
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP,
            {
                .timestamp = 946696962000LL,
                .channels = 3,
                .readings = {
                    -129.993424,
                    0.349649536,
                    0.500022304
                }
            }
        },
        {
            "poll channellist=all" COMMAND_TERMINATOR,
            "polling 2000-01-01 20:09:36.000 -129.805680e+000 Error-14 Error-14 "
            "Error-14 1.00000000e+000" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_SCHEDULELABEL
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP,
            {
                .timestamp = 946757376000LL,
                .channels = 5,
                .readings = {
                    -129.805680,
                    RBRInstrumentGen4Reading_setError(
                        RBRINSTRUMENTGEN4_READING_FLAG_ERROR,
                        14),
                    RBRInstrumentGen4Reading_setError(
                        RBRINSTRUMENTGEN4_READING_FLAG_ERROR,
                        14),
                    RBRInstrumentGen4Reading_setError(
                        RBRINSTRUMENTGEN4_READING_FLAG_ERROR,
                        14),
                    1.0
                }
            }
        },
        {
            "poll channellist=all" COMMAND_TERMINATOR,
            "RBR 999999 polling 2000-01-01 03:22:42.000 -129.993424e+000 349.649536e-003 "
            "500.022304e-003 0x802A" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_SCHEDULELABEL
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_SERIAL
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_CRC,
            {
                .timestamp = 946696962000LL,
                .channels = 3,
                .readings = {
                    -129.993424,
                    0.349649536,
                    0.500022304
                }
            }
        },
        {0}
    };
    return test_pollAllChannels(instrument, buffers, tests);
}

typedef struct PollOneChannelTest
{
    const char *expectedCommand;
    const char *response;
    const char *channelLabel;
    RBRInstrumentGen4OutputFormat outputFormat;
    RBRInstrumentGen4Sample expected;
} PollOneChannelTest;

static bool test_pollOneChannel(RBRInstrumentGen4 *instrument,
                          TestIOBuffers *buffers,
                          PollOneChannelTest *tests)
{
    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Sample actual;

    for (int i=0; tests[i].expectedCommand != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        instrument->outputFormat=tests[i].outputFormat;
        err = RBRInstrumentGen4_pollOneChannel(instrument,
                                  tests[i].channelLabel,
                                  &actual);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].expectedCommand, buffers->writeBuffer);
        TEST_ASSERT_EQ(tests[i].expected.timestamp,
                       actual.timestamp,
                       "%" PRIi64);
        TEST_ASSERT_EQ(tests[i].expected.channels,
                       actual.channels,
                       "%" PRIi32);
        for (int32_t channel = 0; channel < actual.channels; ++channel)
        {
            TEST_ASSERT_ENUM_EQ(
                RBRInstrumentGen4Reading_getFlag(
                    tests[i].expected.readings[channel]),
                RBRInstrumentGen4Reading_getFlag(actual.readings[channel]),
                RBRInstrumentGen4ReadingFlag);

            switch (RBRInstrumentGen4Reading_getFlag(actual.readings[channel]))
            {
            case RBRINSTRUMENTGEN4_READING_FLAG_UNCALIBRATED:
            case RBRINSTRUMENTGEN4_READING_FLAG_ERROR:
                TEST_ASSERT_EQ(
                    RBRInstrumentGen4Reading_getError(
                        tests[i].expected.readings[channel]),
                    RBRInstrumentGen4Reading_getError(actual.readings[channel]),
                    "%" PRIi8);
                break;
            case RBRINSTRUMENTGEN4_READING_FLAG_NONE:
            default:
                TEST_ASSERT_EQ(tests[i].expected.readings[channel],
                               actual.readings[channel],
                               "%lf");
            }
        }
    }

    return true;
}

TEST_LOGGER4(poll_one_channel)
{
    PollOneChannelTest tests[] = {
        {
            "poll channellist=conductivity_00" COMMAND_TERMINATOR,
            "polling 2018-07-26 14:56:24.000 39.993424e+000" RESPONSE_TERMINATOR,
            "conductivity_00",
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_SCHEDULELABEL
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP,
            {
                .timestamp = 1532616984000LL,
                .channels = 1,
                .readings = {
                    39.993424
                }
            }
        },
        {
            "poll channellist=temperature_00" COMMAND_TERMINATOR,
            "2000-01-01 03:22:42.000 39.993424e+000" RESPONSE_TERMINATOR,
            "temperature_00",
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP,
            {
                .timestamp = 946696962000LL,
                .channels = 1,
                .readings = {
                    39.993424
                }
            }
        },
        {
            "poll channellist=temperature_00" COMMAND_TERMINATOR,
            "2000-01-01 03:22:42.000 Error-07" RESPONSE_TERMINATOR,
            "temperature_00",
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP,
            {
                .timestamp = 946696962000LL,
                .channels = 1,
                .readings = {
                    RBRInstrumentGen4Reading_setError(
                        RBRINSTRUMENTGEN4_READING_FLAG_ERROR,
                        7)
                }
            }
        },
        {
            "poll channellist=temperature_00" COMMAND_TERMINATOR,
            "RBR 999999 2000-01-01 03:22:42.000 Error-07" RESPONSE_TERMINATOR,
            "temperature_00",
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_SERIAL 
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP,
            {
                .timestamp = 946696962000LL,
                .channels = 1,
                .readings = {
                    RBRInstrumentGen4Reading_setError(
                        RBRINSTRUMENTGEN4_READING_FLAG_ERROR,
                        7)
                }
            }
        },
        {
            "poll channellist=temperature_00" COMMAND_TERMINATOR,
            "RBR 999999 polling 2000-01-01 03:22:42.000 349.649536e-003 0xAD85" RESPONSE_TERMINATOR,
            "temperature_00",
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_SCHEDULELABEL
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_SERIAL
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_CRC,
            {
                .timestamp = 946696962000LL,
                .channels = 1,
                .readings = {
                    349.649536e-003
                }
            }
        },
        {0}
    };
    return test_pollOneChannel(instrument, buffers, tests);
}


typedef struct PollOneGroupTest
{
    const char *expectedCommand;
    const char *response;
    const char *groupLabel;
    RBRInstrumentGen4OutputFormat outputFormat;
    RBRInstrumentGen4Sample expected;
} PollOneGroupTest;

static bool test_pollOneGroup(RBRInstrumentGen4 *instrument,
                          TestIOBuffers *buffers,
                          PollOneGroupTest *tests)
{
    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Sample actual;

    for (int i = 0; tests[i].expectedCommand != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        instrument->outputFormat=tests[i].outputFormat;
        err = RBRInstrumentGen4_pollOneGroup(instrument,
                                  tests[i].groupLabel,
                                  &actual);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].expectedCommand, buffers->writeBuffer);
        TEST_ASSERT_EQ(tests[i].expected.timestamp,
                       actual.timestamp,
                       "%" PRIi64);
        TEST_ASSERT_EQ(tests[i].expected.channels,
                       actual.channels,
                       "%" PRIi32);
        for (int32_t channel = 0; channel < actual.channels; ++channel)
        {
            TEST_ASSERT_ENUM_EQ(
                RBRInstrumentGen4Reading_getFlag(
                    tests[i].expected.readings[channel]),
                RBRInstrumentGen4Reading_getFlag(actual.readings[channel]),
                RBRInstrumentGen4ReadingFlag);

            switch (RBRInstrumentGen4Reading_getFlag(actual.readings[channel]))
            {
            case RBRINSTRUMENTGEN4_READING_FLAG_UNCALIBRATED:
            case RBRINSTRUMENTGEN4_READING_FLAG_ERROR:
                TEST_ASSERT_EQ(
                    RBRInstrumentGen4Reading_getError(
                        tests[i].expected.readings[channel]),
                    RBRInstrumentGen4Reading_getError(actual.readings[channel]),
                    "%" PRIi8);
                break;
            case RBRINSTRUMENTGEN4_READING_FLAG_NONE:
            default:
                TEST_ASSERT_EQ(tests[i].expected.readings[channel],
                               actual.readings[channel],
                               "%lf");
            }
        }
    }

    return true;
}

TEST_LOGGER4(poll_one_group)
{
    PollOneGroupTest tests[] = {
        {
            "poll grouplist=g_depth" COMMAND_TERMINATOR,
            "polling 2000-01-01 03:22:42.000 39.993424e+000" RESPONSE_TERMINATOR,
            "g_depth",
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_SCHEDULELABEL
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP,
            {
                .timestamp = 946696962000LL,
                .channels = 1,
                .readings = {
                    39.993424
                }
            }
        },
        {
            "poll grouplist=gr_odo" COMMAND_TERMINATOR,
            "2000-01-01 03:22:42.000 39.993424e+000 -129.993424e+000 349.649536e-003 "
            "500.022304e-003" RESPONSE_TERMINATOR,
            "gr_odo",
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP,
            {
                .timestamp = 946696962000LL,
                .channels = 4,
                .readings = {
                    39.993424,
                    -129.993424,
                    0.349649536,
                    0.500022304
                }
            }
        },
        {
            "poll grouplist=gr_ph" COMMAND_TERMINATOR,
            "2000-01-01 03:22:42.000 39.993424e+000 Error-07" RESPONSE_TERMINATOR,
            "gr_ph",
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP,
            {
                .timestamp = 946696962000LL,
                .channels = 2,
                .readings = {
                    39.993424,
                    RBRInstrumentGen4Reading_setError(
                        RBRINSTRUMENTGEN4_READING_FLAG_ERROR,
                        7)
                }
            }
        },
        {
            "poll grouplist=gr_bbpfl" COMMAND_TERMINATOR,
            "RBR 999999 2000-01-01 03:22:42.000 39.993424e+000 -129.993424e+000 "
            "349.649536e-003 0xD863" RESPONSE_TERMINATOR,
            "gr_bbpfl",
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_SERIAL
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_CRC,
            {
                .timestamp = 946696962000LL,
                .channels = 3,
                .readings = {
                    39.993424,
                    -129.993424,
                    0.349649536
                }
            }
        },
        {
            "poll grouplist=gr_radiometry" COMMAND_TERMINATOR,
            "RBR 999999 polling 2000-01-01 03:22:42.000 349.649536e-003 39.993424e+000 0x89AB" RESPONSE_TERMINATOR,
            "gr_radiometry",
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_SCHEDULELABEL
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_SERIAL
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_CRC,
            {
                .timestamp = 946696962000LL,
                .channels = 2,
                .readings = {
                    0.349649536,
                    39.993424
                }
            }
        },
        {0}
    };
    return test_pollOneGroup(instrument, buffers, tests);
}
