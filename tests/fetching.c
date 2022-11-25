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
#include "tests.h"

typedef struct FetchingTest
{
    const char *command;
    const char *response;
    bool passChannels;
    RBRInstrumentLabelsList channels;
    bool sleepAfter;
    RBRInstrumentSample expected;
} FetchingTest;

static bool test_fetching(RBRInstrument *instrument,
                          TestIOBuffers *buffers,
                          FetchingTest *tests)
{
    RBRInstrumentError err;
    RBRInstrumentSample actual;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrument_fetch(instrument,
                                  tests[i].passChannels ? &tests[i].channels : NULL,
                                  tests[i].sleepAfter,
                                  &actual);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
        TEST_ASSERT_EQ(tests[i].expected.timestamp,
                       actual.timestamp,
                       "%" PRIi64);
        TEST_ASSERT_EQ(tests[i].expected.channels,
                       actual.channels,
                       "%" PRIi32);
        for (int32_t channel = 0; channel < actual.channels; ++channel)
        {
            TEST_ASSERT_ENUM_EQ(
                RBRInstrumentReading_getFlag(
                    tests[i].expected.readings[channel]),
                RBRInstrumentReading_getFlag(actual.readings[channel]),
                RBRInstrumentReadingFlag);

            switch (RBRInstrumentReading_getFlag(actual.readings[channel]))
            {
            case RBRINSTRUMENT_READING_FLAG_UNCALIBRATED:
            case RBRINSTRUMENT_READING_FLAG_ERROR:
                TEST_ASSERT_EQ(
                    RBRInstrumentReading_getError(
                        tests[i].expected.readings[channel]),
                    RBRInstrumentReading_getError(actual.readings[channel]),
                    "%" PRIi8);
                break;
            case RBRINSTRUMENT_READING_FLAG_NONE:
            default:
                TEST_ASSERT_EQ(tests[i].expected.readings[channel],
                               actual.readings[channel],
                               "%lf");
            }
        }
    }

    return true;
}

TEST_LOGGER3(fetch)
{
    FetchingTest tests[] = {
        {
            "fetch sleepafter = false" COMMAND_TERMINATOR,
            "2000-01-01 03:22:42.000, -129.993424e+000, 349.649536e-003, "
            "500.022304e-003" RESPONSE_TERMINATOR,
            false,
            {0},
            false,
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
            "fetch sleepafter = false" COMMAND_TERMINATOR,
            "2000-01-01 20:09:36.000, -129.805680e+000, Error-14, Error-14, "
            "Error-14, 1.00000000e+000" RESPONSE_TERMINATOR,
            false,
            {0},
            false,
            {
                .timestamp = 946757376000LL,
                .channels = 5,
                .readings = {
                    -129.805680,
                    RBRInstrumentReading_setError(
                        RBRINSTRUMENT_READING_FLAG_ERROR,
                        14),
                    RBRInstrumentReading_setError(
                        RBRINSTRUMENT_READING_FLAG_ERROR,
                        14),
                    RBRInstrumentReading_setError(
                        RBRINSTRUMENT_READING_FLAG_ERROR,
                        14),
                    1.0
                }
            }
        },
        {
            "fetch sleepafter = false, channels = temperature_00"
            "|temperature_01|temperature_02|temperature_03|temperature_04"
            COMMAND_TERMINATOR,
            "2000-01-01 00:00:00.000, 0.0, 1.0, 2.0, 3.0, 4.0"
            RESPONSE_TERMINATOR,
            true,
            {
                .count = 5,
                .labels = {
                    "temperature_00",
                    "temperature_01",
                    "temperature_02",
                    "temperature_03",
                    "temperature_04"
                }
            },
            false,
            {
                .timestamp = 946684800000LL,
                .channels = 5,
                .readings = {
                    0.0,
                    1.0,
                    2.0,
                    3.0,
                    4.0
                }
            }
        },
        {
            "fetch sleepafter = false, channels ="
            " aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "|bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
            "|ccccccccccccccccccccccccccccccc"
            "|ddddddddddddddddddddddddddddddd"
            "|eeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
            "|fffffffffffffffffffffffffffffff"
            "|ggggggggggggggggggggggggggggggg"
            "|hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh"
            "|iiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
            "|jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj"
            "|kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk"
            "|lllllllllllllllllllllllllllllll"
            "|mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"
            "|nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn"
            "|ooooooooooooooooooooooooooooooo"
            "|ppppppppppppppppppppppppppppppp"
            "|qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq"
            "|rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"
            "|sssssssssssssssssssssssssssssss"
            "|ttttttttttttttttttttttttttttttt"
            "|uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu"
            "|vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv"
            "|xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
            "|yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy"
            "|zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
            "|0000000000000000000000000000000"
            "|1111111111111111111111111111111"
            "|2222222222222222222222222222222"
            "|3333333333333333333333333333333"
            "|4444444444444444444444444444444"
            "|5555555555555555555555555555555"
            "|6666666666666666666666666666666"
            COMMAND_TERMINATOR,
            "2000-01-01 00:00:00.000, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, "
            "8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, "
            "19.0, 20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, "
            "29.0, 30.0, 31.0"
            RESPONSE_TERMINATOR,
            true,
            {
                .count = 32,
                .labels = {
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
                    "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
                    "ccccccccccccccccccccccccccccccc",
                    "ddddddddddddddddddddddddddddddd",
                    "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeee",
                    "fffffffffffffffffffffffffffffff",
                    "ggggggggggggggggggggggggggggggg",
                    "hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh",
                    "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiii",
                    "jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj",
                    "kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk",
                    "lllllllllllllllllllllllllllllll",
                    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm",
                    "nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn",
                    "ooooooooooooooooooooooooooooooo",
                    "ppppppppppppppppppppppppppppppp",
                    "qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq",
                    "rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr",
                    "sssssssssssssssssssssssssssssss",
                    "ttttttttttttttttttttttttttttttt",
                    "uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu",
                    "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv",
                    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
                    "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy",
                    "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz",
                    "0000000000000000000000000000000",
                    "1111111111111111111111111111111",
                    "2222222222222222222222222222222",
                    "3333333333333333333333333333333",
                    "4444444444444444444444444444444",
                    "5555555555555555555555555555555",
                    "6666666666666666666666666666666"
                }
            },
            false,
            {
                .timestamp = 946684800000LL,
                .channels = 32,
                .readings = {
                    0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0,
                    8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
                    16.0, 17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0,
                    24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0
                }
            }
        },
        {0}
    };

    return test_fetching(instrument, buffers, tests);
}
