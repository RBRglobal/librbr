/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file gating.c
 *
 * \brief Tests for instrument fetching commands.
 */

#include <math.h>
#include "tests.h"
#include "RBRGen3Fetching.h"
#include "RBRGen3Streaming.h"

typedef struct FetchingTest {
    const char *command;
    const char *response;
    bool passChannels;
    RBRGen3LabelsList channels;
    bool sleepAfter;
    RBRGen3Sample expected;
} FetchingTest;

static bool test_fetching(RBRGen3 *conn, TestIOBuffers *buffers, FetchingTest *tests)
{
    RBRGen3Error err;
    RBRGEN3_SAMPLE_DECL(actual, TESTS_CHANNEL_MAX);

    for (int i = 0; tests[i].command != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_fetch(
            conn, tests[i].passChannels ? &tests[i].channels : NULL, tests[i].sleepAfter, &actual);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
        TEST_ASSERT_EQ(tests[i].expected.timestamp, actual.timestamp, "%" PRIi64);
        TEST_ASSERT_EQ(tests[i].expected.channelCount, actual.channelCount, "%" PRIi32);
        TEST_ASSERT_EQ(false, actual.readingsDropped, "%d");
        /* Readings past the count are zeroed. */
        for (int32_t channel = actual.channelCount; channel < actual.size; ++channel) {
            TEST_ASSERT_EQ(0.0, actual.readings[channel], "%lf");
        }
        for (int32_t channel = 0; channel < actual.channelCount; ++channel) {
            TEST_ASSERT_ENUM_EQ(RBRGen3Reading_getFlag(tests[i].expected.readings[channel]),
                                RBRGen3Reading_getFlag(actual.readings[channel]),
                                RBRGen3ReadingFlag);

            switch (RBRGen3Reading_getFlag(actual.readings[channel])) {
            case RBRGEN3_READING_FLAG_UNCALIBRATED:
            case RBRGEN3_READING_FLAG_ERROR:
                TEST_ASSERT_EQ(RBRGen3Reading_getError(tests[i].expected.readings[channel]),
                               RBRGen3Reading_getError(actual.readings[channel]),
                               "%" PRIi8);
                break;
            case RBRGEN3_READING_FLAG_NONE:
            default:
                TEST_ASSERT_EQ(
                    tests[i].expected.readings[channel], actual.readings[channel], "%lf");
            }
        }
    }

    return true;
}

/* The caller sizes the command buffer, so a fetch whose fixed prefix does not
 * fit has to be refused before anything reaches the instrument rather than
 * sent truncated. */
TEST_LOGGER3(fetchCommandBufferTooSmall)
{
    uint8_t commandBuffer[20];
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

    RBRGen3LabelsList channels = {
        .size = 1,
        .len = 1,
        .labels = (RBRGen3Label[]) {"temperature_00"},
    };
    RBRGEN3_SAMPLE_DECL(sample, TESTS_CHANNEL_MAX);
    TestIOBuffers_init(buffers, "", 0);
    err = RBRGen3_fetch(&tiny, &channels, false, &sample);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_COMMAND_TOO_LONG, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    /* An idle connection would normally be woken first; a refused command
     * must not even do that. */
    tiny.lastActivityTime = -1;
    TestIOBuffers_init(buffers, "", 0);
    err = RBRGen3_fetch(&tiny, &channels, false, &sample);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_COMMAND_TOO_LONG, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

/* A fetch whose channel list exactly fills the buffer leaves no room for
 * snprintf()'s null, so the last label is flushed into a second write rather
 * than truncated or refused. */
TEST_LOGGER3(fetchCommandExactlyFull)
{
    /* "fetch sleepafter = false, channels = temp" is 41 characters; with a
     * capacity of 41 snprintf() has room for 40 of them plus its null. */
    uint8_t commandBuffer[41];
    uint8_t responseBuffer[RBRGEN3_RESPONSE_BUFFER_DEFAULT];
    RBRGen3Environment exact = conn->environment;
    exact.command = commandBuffer;
    exact.commandCapacity = sizeof(commandBuffer);
    exact.response = responseBuffer;
    exact.responseCapacity = sizeof(responseBuffer);
    RBRGen3 tiny;
    RBRGen3Error err;

    TestIOBuffers_init(
        buffers,
        "RBR RBRduo3 1.090 999999" RESPONSE_TERMINATOR
        "id model = RBRoem3, version = 1.134, serial = 999999, fwtype = 104" RESPONSE_TERMINATOR,
        0);
    err = RBRGen3_open(&tiny, &exact, 0, buffers);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);

    RBRGen3LabelsList channels = {.size = 1, .len = 1, .labels = (RBRGen3Label[]) {"temp"}};
    RBRGEN3_SAMPLE_DECL(sample, TESTS_CHANNEL_MAX);
    TestIOBuffers_init(buffers, "2000-01-01 03:22:42.000, -129.993424e+000" RESPONSE_TERMINATOR, 0);
    err = RBRGen3_fetch(&tiny, &channels, false, &sample);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("fetch sleepafter = false, channels = temp" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

/* A label which cannot fit even an empty buffer is refused before any part
 * of the command has been written. */
TEST_LOGGER3(fetchLabelTooLongForBuffer)
{
    uint8_t commandBuffer[30];
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

    /* 31 characters: the longest label the instrument allows. */
    RBRGen3LabelsList channels = {
        .size = 2,
        .len = 2,
        .labels = (RBRGen3Label[]) {"temp", "abcdefghijklmnopqrstuvwxyz01234"},
    };
    RBRGEN3_SAMPLE_DECL(sample, TESTS_CHANNEL_MAX);
    TestIOBuffers_init(buffers, "", 0);
    err = RBRGen3_fetch(&tiny, &channels, false, &sample);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_COMMAND_TOO_LONG, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

/* The caller sizes the readings storage, so a sample with more readings than
 * it holds keeps the first ones and flags the rest as dropped. */
TEST_LOGGER3(fetchReadingsTruncated)
{
    RBRGen3Error err;
    RBRGEN3_SAMPLE_DECL(actual, 1);

    TestIOBuffers_init(buffers,
                       "2000-01-01 03:22:42.000, -129.993424e+000, 349.649536e-003, "
                       "500.022304e-003" RESPONSE_TERMINATOR,
                       0);
    err = RBRGen3_fetch(conn, NULL, false, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(1, actual.channelCount, "%" PRIi32);
    TEST_ASSERT_EQ(true, actual.readingsDropped, "%d");
    TEST_ASSERT_EQ(1, actual.size, "%" PRIi32);
    TEST_ASSERT_EQ(-129.993424, actual.readings[0], "%lf");

    return true;
}

/* A sample without readings storage is refused before anything is sent. */
TEST_LOGGER3(fetchRejectsSampleWithoutReadings)
{
    RBRGen3Error err;
    RBRGen3Sample noStorage = {.size = 4, .readings = NULL};
    RBRGEN3_SAMPLE_DECL(noCapacity, 1);
    noCapacity.size = 0;

    TestIOBuffers_init(buffers, "", 0);
    err = RBRGen3_fetch(conn, NULL, false, &noStorage);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE, err, RBRGen3Error);
    err = RBRGen3_fetch(conn, NULL, false, &noCapacity);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

/* A label list which claims more labels than it has storage for is refused
 * before anything is sent, not even a wake: only its first size labels
 * exist. */
TEST_LOGGER3(fetchRejectsBadLabelList)
{
    RBRGen3Error err;
    RBRGEN3_SAMPLE_DECL(sample, TESTS_CHANNEL_MAX);
    RBRGen3LabelsList overfull = {.size = 1, .len = 2, .labels = (RBRGen3Label[]) {"temp"}};
    RBRGen3LabelsList noStorage = {.size = 2, .len = 1, .labels = NULL};
    RBRGen3LabelsList negative = {.size = 2, .len = -1, .labels = (RBRGen3Label[]) {"temp"}};
    RBRGen3DateTime savedActivity = conn->lastActivityTime;

    /* An idle connection would normally be woken first. */
    conn->lastActivityTime = -1;
    TestIOBuffers_init(buffers, "", 0);
    err = RBRGen3_fetch(conn, &overfull, false, &sample);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE, err, RBRGen3Error);
    err = RBRGen3_fetch(conn, &noStorage, false, &sample);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE, err, RBRGen3Error);
    err = RBRGen3_fetch(conn, &negative, false, &sample);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);
    conn->lastActivityTime = savedActivity;

    return true;
}

/* An empty list, storage or not, means every channel. */
TEST_LOGGER3(fetchEmptyLabelList)
{
    RBRGen3Error err;
    RBRGEN3_SAMPLE_DECL(sample, TESTS_CHANNEL_MAX);
    RBRGen3LabelsList empty = {0};

    TestIOBuffers_init(buffers, "2000-01-01 03:22:42.000, -129.993424e+000" RESPONSE_TERMINATOR, 0);
    err = RBRGen3_fetch(conn, &empty, false, &sample);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("fetch sleepafter = false" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_EQ(1, sample.channelCount, "%" PRIi32);

    return true;
}

/* Logger2 ignores the label list but still holds it to the same contract. */
TEST_LOGGER2(fetchLabelList)
{
    RBRGen3Error err;
    RBRGEN3_SAMPLE_DECL(sample, TESTS_CHANNEL_MAX);
    RBRGen3LabelsList channels = {.size = 1, .len = 1, .labels = (RBRGen3Label[]) {"temp"}};
    RBRGen3LabelsList overfull = {.size = 1, .len = 2, .labels = (RBRGen3Label[]) {"temp"}};

    TestIOBuffers_init(buffers, "2000-01-01 03:22:42.000, -129.993424e+000" RESPONSE_TERMINATOR, 0);
    err = RBRGen3_fetch(conn, &channels, false, &sample);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("fetch sleepafter = false" COMMAND_TERMINATOR, buffers->writeBuffer);

    TestIOBuffers_init(buffers, "", 0);
    err = RBRGen3_fetch(conn, &overfull, false, &sample);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

/* When the fixed prefix fits but the channels parameter does not, the prefix
 * is flushed on its own and the command continues in the next write. */
TEST_LOGGER3(fetchSplitsBeforeChannels)
{
    uint8_t commandBuffer[30];
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

    RBRGen3LabelsList channels = {.size = 1, .len = 1, .labels = (RBRGen3Label[]) {"temp"}};
    RBRGEN3_SAMPLE_DECL(sample, TESTS_CHANNEL_MAX);
    TestIOBuffers_init(buffers, "2000-01-01 03:22:42.000, -129.993424e+000" RESPONSE_TERMINATOR, 0);
    err = RBRGen3_fetch(&tiny, &channels, false, &sample);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("fetch sleepafter = false, channels = temp" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(fetch)
{
    FetchingTest tests[] = {
        {
            .command = "fetch sleepafter = false" COMMAND_TERMINATOR,
            .response = "2000-01-01 03:22:42.000, -129.993424e+000, 349.649536e-003, "
                        "500.022304e-003" RESPONSE_TERMINATOR,
            .passChannels = false,
            .channels = {0},
            .sleepAfter = false,
            .expected =
                {
                    .timestamp = 946696962000LL,
                    .channelCount = 3,
                    .readings =
                        (double[]) {
                            -129.993424,
                            0.349649536,
                            0.500022304,
                        },
                },
        },
        {
            .command = "fetch sleepafter = false" COMMAND_TERMINATOR,
            .response = "2000-01-01 20:09:36.000, -129.805680e+000, Error-14, Error-14, "
                        "Error-14, 1.00000000e+000" RESPONSE_TERMINATOR,
            .passChannels = false,
            .channels = {0},
            .sleepAfter = false,
            .expected =
                {
                    .timestamp = 946757376000LL,
                    .channelCount = 5,
                    .readings =
                        (double[]) {
                            -129.805680,
                            RBRGen3Reading_setError(RBRGEN3_READING_FLAG_ERROR, 14),
                            RBRGen3Reading_setError(RBRGEN3_READING_FLAG_ERROR, 14),
                            RBRGen3Reading_setError(RBRGEN3_READING_FLAG_ERROR, 14),
                            1.0,
                        },
                },
        },
        {
            .command =
                "fetch sleepafter = false, channels = temperature_00"
                "|temperature_01|temperature_02|temperature_03|temperature_04" COMMAND_TERMINATOR,
            .response = "2000-01-01 00:00:00.000, 0.0, 1.0, 2.0, 3.0, 4.0" RESPONSE_TERMINATOR,
            .passChannels = true,
            .channels =
                {
                    .size = 5,
                    .len = 5,
                    .labels =
                        (RBRGen3Label[]) {
                            "temperature_00",
                            "temperature_01",
                            "temperature_02",
                            "temperature_03",
                            "temperature_04",
                        },
                },
            .sleepAfter = false,
            .expected =
                {
                    .timestamp = 946684800000LL,
                    .channelCount = 5,
                    .readings =
                        (double[]) {
                            0.0,
                            1.0,
                            2.0,
                            3.0,
                            4.0,
                        },
                },
        },
        {
            .command = "fetch sleepafter = false, channels ="
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
                       "|6666666666666666666666666666666" COMMAND_TERMINATOR,
            .response = "2000-01-01 00:00:00.000, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, "
                        "8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, "
                        "19.0, 20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, "
                        "29.0, 30.0, 31.0" RESPONSE_TERMINATOR,
            .passChannels = true,
            .channels =
                {
                    .size = 32,
                    .len = 32,
                    .labels =
                        (RBRGen3Label[]) {
                            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
                            "ccccccccccccccccccccccccccccccc", "ddddddddddddddddddddddddddddddd",
                            "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeee", "fffffffffffffffffffffffffffffff",
                            "ggggggggggggggggggggggggggggggg", "hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh",
                            "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", "jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj",
                            "kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk", "lllllllllllllllllllllllllllllll",
                            "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm", "nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn",
                            "ooooooooooooooooooooooooooooooo", "ppppppppppppppppppppppppppppppp",
                            "qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq", "rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr",
                            "sssssssssssssssssssssssssssssss", "ttttttttttttttttttttttttttttttt",
                            "uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu", "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv",
                            "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy",
                            "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz", "0000000000000000000000000000000",
                            "1111111111111111111111111111111", "2222222222222222222222222222222",
                            "3333333333333333333333333333333", "4444444444444444444444444444444",
                            "5555555555555555555555555555555", "6666666666666666666666666666666",
                        },
                },
            .sleepAfter = false,
            .expected =
                {
                    .timestamp = 946684800000LL,
                    .channelCount = 32,
                    .readings =
                        (double[]) {
                            0.0,  1.0,  2.0,  3.0,  4.0,  5.0,  6.0,  7.0,  8.0,  9.0,  10.0,
                            11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0, 21.0,
                            22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0,
                        },
                },
        },
        {0},
    };

    return test_fetching(conn, buffers, tests);
}
