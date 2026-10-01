/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file realtime.c
 *
 * \brief Tests for instrument realtime data commands.
 */

/* Required for memset. */
#include <string.h>

#include "tests.h"
#include "RBRGen4Realtime.h"

typedef struct PollTest {
    const char *channelList;
    const char *groupList;
    bool requireLabel;
    const char *expectedCommand;
    const char *response;
    RBRGen4OutputFormat outputFormat;
    RBRGen4Error expectedError;
    RBRGen4Sample expected;
    /** \brief The streamed sample expected to reach the sample callback;
     * asserted only when its channelCount is non-zero. */
    RBRGen4Sample expectedStreamed;
} PollTest;

#define OUTPUTFORMAT_DEFAULT {.sn = false, .scheduleLabel = false, .dateTime = true, .crc = false}

/**
 * \brief Fill \a list with the pipe-separated labels of \a labels.
 *
 * Lets a PollTest spell its list the way the command does.
 */
static void PollTest_labelList(const char *labels, RBRGen4LabelList *list)
{
    list->len = 0;
    while (*labels != '\0' && list->len < list->size) {
        const char *separator = strchr(labels, '|');
        size_t length = separator != NULL ? (size_t) (separator - labels) : strlen(labels);
        snprintf(
            list->labels[list->len], sizeof(list->labels[list->len]), "%.*s", (int) length, labels);
        list->len++;
        labels += separator != NULL ? length + 1 : length;
    }
}

TEST_LOGGER4(poll)
{
    PollTest tests[] = {
        /* A bare poll samples every channel. */
        {
            .channelList = NULL,
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand = "poll" COMMAND_TERMINATOR,
            .response =
                "2024-03-10 07:01:06.000 Error-01 Error-09 Error-09 Error-09" RESPONSE_TERMINATOR,
            .outputFormat = OUTPUTFORMAT_DEFAULT,
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .timestamp = 1710054066000LL,
                    .channelCount = 4,
                    .readings =
                        (double[]) {
                            RBRGen4Reading_setError(1),
                            RBRGen4Reading_setError(9),
                            RBRGen4Reading_setError(9),
                            RBRGen4Reading_setError(9),
                        },
                },
            .expectedStreamed = {0},
        },
        {
            .channelList = "temperature_00",
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand = "poll channellist=temperature_00" COMMAND_TERMINATOR,
            .response = "2024-03-10 07:01:07.000 Error-01" RESPONSE_TERMINATOR,
            .outputFormat = OUTPUTFORMAT_DEFAULT,
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .timestamp = 1710054067000LL,
                    .channelCount = 1,
                    .readings = (double[]) {RBRGen4Reading_setError(1)},
                },
            .expectedStreamed = {0},
        },
        /* A sample's timestamp may be a bare count of milliseconds. */
        {
            .channelList = NULL,
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand = "poll" COMMAND_TERMINATOR,
            .response = "12345 12.5364470e+000 9.91695000e+000" RESPONSE_TERMINATOR,
            .outputFormat = OUTPUTFORMAT_DEFAULT,
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .timestamp = 12345LL,
                    .channelCount = 2,
                    .readings = (double[]) {12.5364470, 9.91695000},
                },
            .expectedStreamed = {0},
        },
        /* A reading is never taken for a millisecond timestamp, so a line
         * missing its timestamp is refused as a sample. The library then
         * keeps waiting for one; here the read buffer runs dry first. */
        {
            .channelList = NULL,
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand = "poll" COMMAND_TERMINATOR,
            .response = "12.5364470e+000 9.91695000e+000" RESPONSE_TERMINATOR,
            .outputFormat = OUTPUTFORMAT_DEFAULT,
            .expectedError = RBRGEN4_CALLBACK_ERROR,
            .expected = {0},
            .expectedStreamed = {0},
        },
        /* A repeated channel is reported at every requested position. */
        {
            .channelList = "pressure_00|temperature_00|pressure_00",
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand =
                "poll channellist=pressure_00|temperature_00|pressure_00" COMMAND_TERMINATOR,
            .response = "2024-03-10 07:01:09.000 Error-09 Error-01 Error-09" RESPONSE_TERMINATOR,
            .outputFormat = OUTPUTFORMAT_DEFAULT,
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .timestamp = 1710054069000LL,
                    .channelCount = 3,
                    .readings =
                        (double[]) {
                            RBRGen4Reading_setError(9),
                            RBRGen4Reading_setError(1),
                            RBRGen4Reading_setError(9),
                        },
                },
            .expectedStreamed = {0},
        },
        {
            .channelList = NULL,
            .groupList = "g",
            .requireLabel = false,
            .expectedCommand = "poll grouplist=g" COMMAND_TERMINATOR,
            .response = "2024-03-10 07:01:10.000 Error-09" RESPONSE_TERMINATOR,
            .outputFormat = OUTPUTFORMAT_DEFAULT,
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .timestamp = 1710054070000LL,
                    .channelCount = 1,
                    .readings = (double[]) {RBRGen4Reading_setError(9)},
                },
            .expectedStreamed = {0},
        },
        /* Successful readings parse as their values. */
        {
            .channelList = NULL,
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand = "poll" COMMAND_TERMINATOR,
            .response =
                "2024-10-21 11:50:49.000 18.1745130 12.7052970 2.69308210" RESPONSE_TERMINATOR,
            .outputFormat = OUTPUTFORMAT_DEFAULT,
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .timestamp = 1729511449000LL,
                    .channelCount = 3,
                    .readings = (double[]) {18.1745130, 12.7052970, 2.69308210},
                },
            .expectedStreamed = {0},
        },
        /* Polled samples carry the “polling” schedule label when enabled. */
        {
            .channelList = "temperature_00",
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand = "poll channellist=temperature_00" COMMAND_TERMINATOR,
            .response = "polling 2024-03-10 07:01:31.000 Error-01" RESPONSE_TERMINATOR,
            .outputFormat =
                {
                    .sn = false,
                    .scheduleLabel = true,
                    .dateTime = true,
                    .crc = false,
                },
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .timestamp = 1710054091000LL,
                    .scheduleLabel = "polling",
                    .channelCount = 1,
                    .readings = (double[]) {RBRGen4Reading_setError(1)},
                },
            .expectedStreamed = {0},
        },
        {
            .channelList = "temperature_00",
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand = "poll channellist=temperature_00" COMMAND_TERMINATOR,
            .response = "RBR 999999 2024-03-10 07:01:33.000 Error-01" RESPONSE_TERMINATOR,
            .outputFormat =
                {
                    .sn = true,
                    .scheduleLabel = false,
                    .dateTime = true,
                    .crc = false,
                },
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .timestamp = 1710054093000LL,
                    .channelCount = 1,
                    .readings = (double[]) {RBRGen4Reading_setError(1)},
                },
            .expectedStreamed = {0},
        },
        {
            .channelList = "temperature_00",
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand = "poll channellist=temperature_00" COMMAND_TERMINATOR,
            .response = "2024-03-10 07:01:35.000 Error-01 0x3C7A" RESPONSE_TERMINATOR,
            .outputFormat =
                {
                    .sn = false,
                    .scheduleLabel = false,
                    .dateTime = true,
                    .crc = true,
                },
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .timestamp = 1710054095000LL,
                    .channelCount = 1,
                    .readings = (double[]) {RBRGen4Reading_setError(1)},
                },
            .expectedStreamed = {0},
        },
        /* A single reading with no other fields is a one-token sample. */
        {
            .channelList = "pressure_00",
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand = "poll channellist=pressure_00" COMMAND_TERMINATOR,
            .response = "9.85289000e+000" RESPONSE_TERMINATOR,
            .outputFormat =
                {
                    .sn = false,
                    .scheduleLabel = false,
                    .dateTime = false,
                    .crc = false,
                },
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .timestamp = 0,
                    .channelCount = 1,
                    .readings = (double[]) {9.85289000},
                },
            .expectedStreamed = {0},
        },
        {
            .channelList = "pressure_00",
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand = "poll channellist=pressure_00" COMMAND_TERMINATOR,
            .response = "9.80449000e+000 0xF80B" RESPONSE_TERMINATOR,
            .outputFormat =
                {
                    .sn = false,
                    .scheduleLabel = false,
                    .dateTime = false,
                    .crc = true,
                },
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .timestamp = 0,
                    .channelCount = 1,
                    .readings = (double[]) {9.80449000},
                },
            .expectedStreamed = {0},
        },
        /* The first reading is not dropped when the timestamp,
         * schedule label, and serial number are all omitted. */
        {
            .channelList = NULL,
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand = "poll" COMMAND_TERMINATOR,
            .response = "12.5363015e+000 9.82848000e+000 -304.020648e-003"
                        " -302.152465e-003" RESPONSE_TERMINATOR,
            .outputFormat =
                {
                    .sn = false,
                    .scheduleLabel = false,
                    .dateTime = false,
                    .crc = false,
                },
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .timestamp = 0,
                    .channelCount = 4,
                    .readings = (double[]) {12.5363015, 9.82848000, -0.304020648, -0.302152465},
                },
            .expectedStreamed = {0},
        },
        {
            .channelList = NULL,
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand = "poll" COMMAND_TERMINATOR,
            .response = "12.5361167e+000 9.82034000e+000 -312.160648e-003"
                        " -310.242445e-003 0x1E75" RESPONSE_TERMINATOR,
            .outputFormat =
                {
                    .sn = false,
                    .scheduleLabel = false,
                    .dateTime = false,
                    .crc = true,
                },
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .timestamp = 0,
                    .channelCount = 4,
                    .readings = (double[]) {12.5361167, 9.82034000, -0.312160648, -0.310242445},
                },
            .expectedStreamed = {0},
        },
        {
            .channelList = NULL,
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand = "poll" COMMAND_TERMINATOR,
            .response = "polling 12.5356691e+000 9.81261000e+000 -319.890648e-003"
                        " -317.924945e-003" RESPONSE_TERMINATOR,
            .outputFormat =
                {
                    .sn = false,
                    .scheduleLabel = true,
                    .dateTime = false,
                    .crc = false,
                },
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .timestamp = 0,
                    .scheduleLabel = "polling",
                    .channelCount = 4,
                    .readings = (double[]) {12.5356691, 9.81261000, -0.319890648, -0.317924945},
                },
            .expectedStreamed = {0},
        },
        {
            .channelList = NULL,
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand = "poll" COMMAND_TERMINATOR,
            .response = "polling 12.5356399e+000 9.79543000e+000 -337.070648e-003"
                        " -334.999375e-003 0x271D" RESPONSE_TERMINATOR,
            .outputFormat =
                {
                    .sn = false,
                    .scheduleLabel = true,
                    .dateTime = false,
                    .crc = true,
                },
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .timestamp = 0,
                    .scheduleLabel = "polling",
                    .channelCount = 4,
                    .readings = (double[]) {12.5356399, 9.79543000, -0.337070648, -0.334999375},
                },
            .expectedStreamed = {0},
        },
        {
            .channelList = NULL,
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand = "poll" COMMAND_TERMINATOR,
            .response = "RBR 999999 12.5360777e+000 9.80820000e+000 -324.300648e-003"
                        " -322.307846e-003" RESPONSE_TERMINATOR,
            .outputFormat =
                {
                    .sn = true,
                    .scheduleLabel = false,
                    .dateTime = false,
                    .crc = false,
                },
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .timestamp = 0,
                    .channelCount = 4,
                    .readings = (double[]) {12.5360777, 9.80820000, -0.324300648, -0.322307846},
                },
            .expectedStreamed = {0},
        },
        {
            .channelList = NULL,
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand = "poll" COMMAND_TERMINATOR,
            .response = "RBR 999999 12.5362042e+000 9.81825000e+000 -314.250648e-003"
                        " -312.319602e-003 0x3BE3" RESPONSE_TERMINATOR,
            .outputFormat =
                {
                    .sn = true,
                    .scheduleLabel = false,
                    .dateTime = false,
                    .crc = true,
                },
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .timestamp = 0,
                    .channelCount = 4,
                    .readings = (double[]) {12.5362042, 9.81825000, -0.314250648, -0.312319602},
                },
            .expectedStreamed = {0},
        },
        {
            .channelList = NULL,
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand = "poll" COMMAND_TERMINATOR,
            .response = "RBR 999999 polling 12.5369048e+000 9.81109000e+000"
                        " -321.410648e-003 -319.435605e-003" RESPONSE_TERMINATOR,
            .outputFormat =
                {
                    .sn = true,
                    .scheduleLabel = true,
                    .dateTime = false,
                    .crc = false,
                },
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .timestamp = 0,
                    .scheduleLabel = "polling",
                    .channelCount = 4,
                    .readings = (double[]) {12.5369048, 9.81109000, -0.321410648, -0.319435605},
                },
            .expectedStreamed = {0},
        },
        {
            .channelList = NULL,
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand = "poll" COMMAND_TERMINATOR,
            .response = "RBR 999999 polling 12.5369632e+000 9.80959000e+000"
                        " -322.910648e-003 -320.926387e-003 0xF4F3" RESPONSE_TERMINATOR,
            .outputFormat =
                {
                    .sn = true,
                    .scheduleLabel = true,
                    .dateTime = false,
                    .crc = true,
                },
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .timestamp = 0,
                    .scheduleLabel = "polling",
                    .channelCount = 4,
                    .readings = (double[]) {12.5369632, 9.80959000, -0.322910648, -0.320926387},
                },
            .expectedStreamed = {0},
        },
        /* A float64-encoded sample with every optional prefix still
         * retains its first reading. */
        {
            .channelList = NULL,
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand = "poll" COMMAND_TERMINATOR,
            .response = "RBR 999999 polling 2024-03-13 12:37:36.000"
                        " 12.5369242316864e+000 9.80062000000000e+000"
                        " -331.880648498535e-003 -329.841267502290e-003 0xE4EE" RESPONSE_TERMINATOR,
            .outputFormat =
                {
                    .sn = true,
                    .scheduleLabel = true,
                    .dateTime = true,
                    .crc = true,
                    .dataType = RBRGEN4_DATA_TYPE_FLOAT64,
                },
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .timestamp = 1710333456000LL,
                    .scheduleLabel = "polling",
                    .channelCount = 4,
                    .readings =
                        (double[]) {
                            12.5369242316864,
                            9.80062000000000,
                            -0.331880648498535,
                            -0.329841267502290,
                        },
                },
            .expectedStreamed = {0},
        },
        /* requireLabel is checked before anything is sent. */
        {
            .channelList = "pressure_00",
            .groupList = NULL,
            .requireLabel = true,
            .expectedCommand = "",
            .response = "",
            .outputFormat =
                {
                    .sn = false,
                    .scheduleLabel = false,
                    .dateTime = false,
                    .crc = false,
                },
            .expectedError = RBRGEN4_UNSUPPORTED,
            .expected = {0},
            .expectedStreamed = {0},
        },
        /* With requireLabel set, streamed samples read while waiting for
         * the polled one are forwarded to the sample callback and skipped. */
        {
            .channelList = "pressure_00",
            .groupList = NULL,
            .requireLabel = true,
            .expectedCommand = "poll channellist=pressure_00" COMMAND_TERMINATOR,
            .response = "RBR 999999 sch_asc_pts 9.84050000e+000 12.5362918e+000" RESPONSE_TERMINATOR
                        "RBR 999999 polling 9.83602000e+000" RESPONSE_TERMINATOR,
            .outputFormat =
                {
                    .sn = true,
                    .scheduleLabel = true,
                    .dateTime = false,
                    .crc = false,
                },
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .scheduleLabel = "polling",
                    .channelCount = 1,
                    .readings = (double[]) {9.83602000},
                },
            .expectedStreamed =
                {
                    .scheduleLabel = "sch_asc_pts",
                    .channelCount = 2,
                    .readings = (double[]) {9.84050000, 12.5362918},
                },
        },
        /* Without requireLabel, the first sample read wins, even if it's a
         * streamed sample. */
        {
            .channelList = "pressure_00",
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand = "poll channellist=pressure_00" COMMAND_TERMINATOR,
            .response = "RBR 999999 sch_asc_pts 9.84050000e+000 12.5362918e+000" RESPONSE_TERMINATOR
                        "RBR 999999 polling 9.83602000e+000" RESPONSE_TERMINATOR,
            .outputFormat =
                {
                    .sn = true,
                    .scheduleLabel = true,
                    .dateTime = false,
                    .crc = false,
                },
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .scheduleLabel = "sch_asc_pts",
                    .channelCount = 2,
                    .readings = (double[]) {9.84050000, 12.5362918},
                },
            .expectedStreamed = {0},
        },
        /* An unknown channel or group label is a hardware error. */
        {
            .channelList = "nosuchchannel_00",
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand = "poll channellist=nosuchchannel_00" COMMAND_TERMINATOR,
            .response = "ERR-117 'nosuchchannel_00' is not a known qualifier" RESPONSE_TERMINATOR,
            .outputFormat = OUTPUTFORMAT_DEFAULT,
            .expectedError = RBRGEN4_HARDWARE_ERROR,
            .expected = {0},
            .expectedStreamed = {0},
        },
        {
            .channelList = NULL,
            .groupList = "nosuchgroup",
            .requireLabel = false,
            .expectedCommand = "poll grouplist=nosuchgroup" COMMAND_TERMINATOR,
            .response = "ERR-117 'nosuchgroup' is not a known qualifier" RESPONSE_TERMINATOR,
            .outputFormat = OUTPUTFORMAT_DEFAULT,
            .expectedError = RBRGEN4_HARDWARE_ERROR,
            .expected = {0},
            .expectedStreamed = {0},
        },
        /* An empty list is refused before the command: `poll` has no
         * equivalent of the `none` an empty list is otherwise sent as. */
        {
            .channelList = "",
            .groupList = NULL,
            .requireLabel = false,
            .expectedCommand = "",
            .response = "",
            .outputFormat = OUTPUTFORMAT_DEFAULT,
            .expectedError = RBRGEN4_INVALID_PARAMETER_VALUE,
            .expected = {0},
            .expectedStreamed = {0},
        },
        {0},
    };

    RBRGen4Error err;
    RBRGEN4_SAMPLE_DECL(actual, TESTS_CHANNEL_MAX);

    for (int i = 0; tests[i].expectedCommand != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        conn->outputFormat = tests[i].outputFormat;
        RBRGEN4_LABEL_LIST_DECL(list, TESTS_CHANNEL_MAX);
        if (tests[i].channelList != NULL) {
            PollTest_labelList(tests[i].channelList, &list);
            err = RBRGen4_pollChannels(conn, tests[i].requireLabel, &list, &actual);
        } else if (tests[i].groupList != NULL) {
            PollTest_labelList(tests[i].groupList, &list);
            err = RBRGen4_pollGroups(conn, tests[i].requireLabel, &list, &actual);
        } else {
            err = RBRGen4_poll(conn, tests[i].requireLabel, &actual);
        }
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].expectedCommand, buffers->writeBuffer);

        if (tests[i].expectedError != RBRGEN4_SUCCESS) {
            continue;
        }

        TEST_ASSERT_EQ(tests[i].expected.timestamp, actual.timestamp, "%" PRIi64);
        TEST_ASSERT_EQ(tests[i].expected.channelCount, actual.channelCount, "%" PRIi32);
        TEST_ASSERT_STR_EQ(tests[i].expected.scheduleLabel, actual.scheduleLabel);
        for (int32_t channel = 0; channel < actual.channelCount; ++channel) {
            TEST_ASSERT_ENUM_EQ(RBRGen4Reading_isError(tests[i].expected.readings[channel]),
                                RBRGen4Reading_isError(actual.readings[channel]),
                                bool);

            if (RBRGen4Reading_isError(actual.readings[channel])) {
                TEST_ASSERT_EQ(RBRGen4Reading_getError(tests[i].expected.readings[channel]),
                               RBRGen4Reading_getError(actual.readings[channel]),
                               "%" PRIi8);
            } else {
                TEST_ASSERT_EQ(
                    tests[i].expected.readings[channel], actual.readings[channel], "%lf");
            }
        }

        if (tests[i].expectedStreamed.channelCount != 0) {
            TEST_ASSERT_EQ(tests[i].expectedStreamed.channelCount,
                           buffers->streamSample.channelCount,
                           "%" PRIi32);
            TEST_ASSERT_STR_EQ(tests[i].expectedStreamed.scheduleLabel,
                               buffers->streamSample.scheduleLabel);
            for (int32_t channel = 0; channel < tests[i].expectedStreamed.channelCount; ++channel) {
                TEST_ASSERT_EQ(tests[i].expectedStreamed.readings[channel],
                               buffers->streamSample.readings[channel],
                               "%lf");
            }
        }
    }

    /* A label list too long to send is refused before the command. */
    RBRGEN4_LABEL_LIST_DECL(longList, TESTS_CHANNEL_MAX);
    for (longList.len = 0; longList.len < longList.size; longList.len++) {
        memset(longList.labels[longList.len], 'a', RBRGEN4_LABEL_NAME_MAX);
        longList.labels[longList.len][RBRGEN4_LABEL_NAME_MAX] = '\0';
    }
    TestIOBuffers_init(buffers, "", 0);
    err = RBRGen4_pollChannels(conn, false, &longList, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_COMMAND_TOO_LONG, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

/** \brief A fake clock, advanced by pollTimeoutTime() on every call. */
static RBRGen4DateTime pollTimeoutClock;

/** \brief A time callback that advances #pollTimeoutClock by less than a
 * third of RBRGen4.pollTimeout on every call, so a poll's
 * overall timeout can be exercised without waiting in real time. */
static RBRGen4Error pollTimeoutTime(const RBRGen4 *conn, RBRGen4DateTime *time)
{
    (void) conn;
    *time = pollTimeoutClock;
    pollTimeoutClock += 3000;
    return RBRGEN4_SUCCESS;
}

/* The caller sizes the command buffer, so a poll command which does not fit
 * is refused before anything reaches the instrument. */
TEST_LOGGER4(pollCommandBufferTooSmall)
{
    RBRGen4Error err;
    RBRGEN4_SAMPLE_DECL(sample, TESTS_CHANNEL_MAX);
    const RBRGen4Environment before = conn->environment;
    uint8_t commandBuffer[5];

    err = RBRGen4_setCommandBuffer(conn, commandBuffer, sizeof(commandBuffer));
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);

    TestIOBuffers_init(buffers, "", 0);
    err = RBRGen4_poll(conn, false, &sample);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_COMMAND_TOO_LONG, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    err = RBRGen4_setCommandBuffer(conn, before.command, before.commandCapacity);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);

    return true;
}

TEST_LOGGER4(pollTimeout)
{
    RBRGen4Error err;
    RBRGEN4_SAMPLE_DECL(actual, TESTS_CHANNEL_MAX);
    RBRGen4TimeCallback savedTime = conn->environment.time;
    RBRGen4DateTime savedPollTimeout = conn->pollTimeout;

    pollTimeoutClock = 0;
    conn->environment.time = pollTimeoutTime;
    conn->pollTimeout = 10000;

    conn->outputFormat =
        (RBRGen4OutputFormat) {.sn = false, .scheduleLabel = true, .dateTime = false, .crc = false};
    /* Six streamed samples, and no polled sample at all: if the overall
     * poll timeout didn't bound the wait, the read buffer would exhaust
     * before the poll gave up. */
    TestIOBuffers_init(buffers,
                       "sch_asc_pts 9.84050000e+000 12.5362918e+000" RESPONSE_TERMINATOR
                       "sch_asc_pts 9.84050000e+000 12.5362918e+000" RESPONSE_TERMINATOR
                       "sch_asc_pts 9.84050000e+000 12.5362918e+000" RESPONSE_TERMINATOR
                       "sch_asc_pts 9.84050000e+000 12.5362918e+000" RESPONSE_TERMINATOR
                       "sch_asc_pts 9.84050000e+000 12.5362918e+000" RESPONSE_TERMINATOR
                       "sch_asc_pts 9.84050000e+000 12.5362918e+000" RESPONSE_TERMINATOR,
                       0);

    RBRGEN4_LABEL_LIST_DECL(channelList, 1);
    PollTest_labelList("pressure_00", &channelList);
    err = RBRGen4_pollChannels(conn, true, &channelList, &actual);

    conn->environment.time = savedTime;
    conn->pollTimeout = savedPollTimeout;

    TEST_ASSERT_ENUM_EQ(RBRGEN4_TIMEOUT, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("poll channellist=pressure_00" COMMAND_TERMINATOR, buffers->writeBuffer);
    /* At least one streamed sample must have been forwarded before the
     * timeout fired. */
    TEST_ASSERT_EQ(2, buffers->streamSample.channelCount, "%" PRIi32);

    return true;
}

/** \brief A fake clock, advanced by pollSlowResponseTime() on every call. */
static RBRGen4DateTime pollSlowResponseClock;

/** \brief A time callback that advances #pollSlowResponseClock by more than
 * a small commandTimeout on every call, so that a wait spanning several
 * such ticks can be exercised without waiting in real time. */
static RBRGen4Error pollSlowResponseTime(const RBRGen4 *conn, RBRGen4DateTime *time)
{
    (void) conn;
    *time = pollSlowResponseClock;
    pollSlowResponseClock += 500;
    return RBRGEN4_SUCCESS;
}

TEST_LOGGER4(pollSlowResponse)
{
    RBRGen4Error err;
    RBRGEN4_SAMPLE_DECL(actual, TESTS_CHANNEL_MAX);
    RBRGen4TimeCallback savedTime = conn->environment.time;
    RBRGen4DateTime savedCommandTimeout = conn->commandTimeout;
    RBRGen4DateTime savedPollTimeout = conn->pollTimeout;

    pollSlowResponseClock = 0;
    conn->environment.time = pollSlowResponseTime;
    /* commandTimeout is small enough that it would trip on its own between
     * clock ticks; only pollTimeout is large enough to bound the wait. */
    conn->commandTimeout = 100;
    conn->pollTimeout = 10000;

    conn->outputFormat =
        (RBRGen4OutputFormat) {.sn = false, .scheduleLabel = true, .dateTime = false, .crc = false};
    /* A single polled sample, arriving only after silence long enough to
     * exceed commandTimeout. */
    TestIOBuffers_init(buffers, "polling 12.5364470e+000 9.91695000e+000" RESPONSE_TERMINATOR, 0);

    err = RBRGen4_poll(conn, true, &actual);

    conn->environment.time = savedTime;
    conn->commandTimeout = savedCommandTimeout;
    conn->pollTimeout = savedPollTimeout;

    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_EQ(2, actual.channelCount, "%" PRIi32);

    return true;
}

typedef struct ReadSampleTest {
    RBRGen4OutputFormat outputFormat;
    const char *response;
    RBRGen4Sample expected;
} ReadSampleTest;

TEST_LOGGER4(readSample)
{
    ReadSampleTest tests[] = {
        /* sn, scheduleLabel, dateTime, and crc all on. */
        {
            .outputFormat =
                {
                    .sn = true,
                    .scheduleLabel = true,
                    .dateTime = true,
                    .crc = true,
                },
            .response = "RBR 999999 sch_asc_pts 2024-03-13 12:45:53.000 9.84033000e+000"
                        " 12.5358150e+000 0x25C8" RESPONSE_TERMINATOR,
            .expected =
                {
                    .timestamp = 1710333953000LL,
                    .scheduleLabel = "sch_asc_pts",
                    .channelCount = 2,
                    .readings = (double[]) {9.84033000, 12.5358150},
                },
        },
        /* sn, scheduleLabel, dateTime, and crc all off. */
        {
            .outputFormat =
                {
                    .sn = false,
                    .scheduleLabel = false,
                    .dateTime = false,
                    .crc = false,
                },
            .response = "9.85054000e+000 12.5359318e+000" RESPONSE_TERMINATOR,
            .expected =
                {
                    .timestamp = 0,
                    .scheduleLabel = "",
                    .channelCount = 2,
                    .readings = (double[]) {9.85054000, 12.5359318},
                },
        },
        /* Only scheduleLabel and crc on. */
        {
            .outputFormat =
                {
                    .sn = false,
                    .scheduleLabel = true,
                    .dateTime = false,
                    .crc = true,
                },
            .response = "sch_asc_pts 9.83676000e+000 12.5361556e+000 0xC4EC" RESPONSE_TERMINATOR,
            .expected =
                {
                    .timestamp = 0,
                    .scheduleLabel = "sch_asc_pts",
                    .channelCount = 2,
                    .readings = (double[]) {9.83676000, 12.5361556},
                },
        },
        /* Only sn on, with a float64-encoded sample. */
        {
            .outputFormat =
                {
                    .sn = true,
                    .scheduleLabel = false,
                    .dateTime = false,
                    .crc = false,
                    .dataType = RBRGEN4_DATA_TYPE_FLOAT64,
                },
            .response =
                "RBR 999999 9.84050000000000e+000 12.5362917963016e+000" RESPONSE_TERMINATOR,
            .expected =
                {
                    .timestamp = 0,
                    .scheduleLabel = "",
                    .channelCount = 2,
                    .readings = (double[]) {9.84050000000000, 12.5362917963016},
                },
        },
        /* The same as the first row, but preceded by the command prompt,
         * which must be trimmed before the sample is parsed. */
        {
            .outputFormat =
                {
                    .sn = true,
                    .scheduleLabel = true,
                    .dateTime = true,
                    .crc = true,
                },
            .response = "ready: RBR 999999 sch_asc_pts 2024-03-13 12:45:49.000"
                        " 9.83297000e+000 12.5356886e+000 0x29C1" RESPONSE_TERMINATOR,
            .expected =
                {
                    .timestamp = 1710333949000LL,
                    .scheduleLabel = "sch_asc_pts",
                    .channelCount = 2,
                    .readings = (double[]) {9.83297000, 12.5356886},
                },
        },
    };

    RBRGen4Error err;

    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        conn->outputFormat = tests[i].outputFormat;

        err = RBRGen4_readSample(conn);
        TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);

        TEST_ASSERT_EQ(tests[i].expected.timestamp, buffers->streamSample.timestamp, "%" PRIi64);
        TEST_ASSERT_STR_EQ(tests[i].expected.scheduleLabel, buffers->streamSample.scheduleLabel);
        TEST_ASSERT_EQ(
            tests[i].expected.channelCount, buffers->streamSample.channelCount, "%" PRIi32);
        for (int32_t channel = 0; channel < buffers->streamSample.channelCount; ++channel) {
            TEST_ASSERT_ENUM_EQ(RBRGen4Reading_isError(tests[i].expected.readings[channel]),
                                RBRGen4Reading_isError(buffers->streamSample.readings[channel]),
                                bool);

            if (RBRGen4Reading_isError(buffers->streamSample.readings[channel])) {
                TEST_ASSERT_EQ(RBRGen4Reading_getError(tests[i].expected.readings[channel]),
                               RBRGen4Reading_getError(buffers->streamSample.readings[channel]),
                               "%" PRIi8);
            } else {
                TEST_ASSERT_EQ(tests[i].expected.readings[channel],
                               buffers->streamSample.readings[channel],
                               "%lf");
            }
        }
    }

    return true;
}

TEST_LOGGER4(readSampleBadCrc)
{
    RBRGen4Error err;

    conn->outputFormat =
        (RBRGen4OutputFormat) {.sn = false, .scheduleLabel = true, .dateTime = false, .crc = true};
    /* The last CRC digit is altered (...C4EC becomes ...C4ED), so the
     * checksum won't match. */
    TestIOBuffers_init(
        buffers, "sch_asc_pts 9.83676000e+000 12.5361556e+000 0xC4ED" RESPONSE_TERMINATOR, 0);

    err = RBRGen4_readSample(conn);
    /* The checksum failure makes the parser reject the line as a sample;
     * with no error/warning prefix, errorCheckResponse() then treats it as
     * an ordinary command response, so readSample() loops back for another
     * response and, finding no more data, fails with a callback error
     * rather than a checksum or timeout error. */
    TEST_ASSERT_ENUM_EQ(RBRGEN4_CALLBACK_ERROR, err, RBRGen4Error);

    return true;
}

/* The caller sizes the readings storage, so a sample with more readings than
 * it holds keeps the first ones and flags the rest as dropped. */
TEST_LOGGER4(pollReadingsTruncated)
{
    RBRGen4Error err;
    RBRGEN4_SAMPLE_DECL(actual, 1);

    conn->outputFormat = (RBRGen4OutputFormat) OUTPUTFORMAT_DEFAULT;
    TestIOBuffers_init(
        buffers, "2024-03-10 07:01:06.000 12.5364470e+000 9.91695000e+000" RESPONSE_TERMINATOR, 0);
    err = RBRGen4_poll(conn, false, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_EQ(1, actual.channelCount, "%" PRIi32);
    TEST_ASSERT_EQ(true, actual.readingsDropped, "%d");
    TEST_ASSERT_EQ(1, actual.size, "%" PRIi32);
    TEST_ASSERT_EQ(12.5364470, actual.readings[0], "%lf");

    return true;
}

/* A sample without readings storage is refused before anything is sent. */
TEST_LOGGER4(pollRejectsSampleWithoutReadings)
{
    RBRGen4Error err;
    RBRGen4Sample noStorage = {.size = 4, .readings = NULL};
    RBRGEN4_SAMPLE_DECL(noCapacity, 1);
    noCapacity.size = 0;

    TestIOBuffers_init(buffers, "", 0);
    err = RBRGen4_poll(conn, false, &noStorage);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);
    err = RBRGen4_poll(conn, false, &noCapacity);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

/* A streamed sample forwarded to the callback fills the callback sample's
 * own storage even when the polled sample has less. */
TEST_LOGGER4(streamedSampleFillsLargerCallbackStorage)
{
    RBRGen4Error err;
    RBRGEN4_SAMPLE_DECL(actual, 1);

    conn->outputFormat =
        (RBRGen4OutputFormat) {.sn = true, .scheduleLabel = true, .dateTime = false, .crc = false};
    TestIOBuffers_init(
        buffers,
        "ready: RBR 999999 sch_asc_pts 9.84050000e+000 12.5362918e+000" RESPONSE_TERMINATOR
        "RBR 999999 polling 9.83602000e+000" RESPONSE_TERMINATOR,
        0);

    RBRGEN4_LABEL_LIST_DECL(channelList, 1);
    PollTest_labelList("pressure_00", &channelList);
    err = RBRGen4_pollChannels(conn, true, &channelList, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("polling", actual.scheduleLabel);
    TEST_ASSERT_EQ(1, actual.channelCount, "%" PRIi32);
    TEST_ASSERT_EQ(false, actual.readingsDropped, "%d");
    TEST_ASSERT_EQ(9.83602000, actual.readings[0], "%lf");

    TEST_ASSERT_STR_EQ("sch_asc_pts", buffers->deliveredSample.scheduleLabel);
    TEST_ASSERT_EQ(2, buffers->deliveredSample.channelCount, "%" PRIi32);
    TEST_ASSERT_EQ(false, buffers->deliveredSample.readingsDropped, "%d");
    TEST_ASSERT_EQ(9.84050000, buffers->deliveredSample.readings[0], "%lf");
    TEST_ASSERT_EQ(12.5362918, buffers->deliveredSample.readings[1], "%lf");

    return true;
}

/* When the callback sample is the smaller one, it receives what fits and is
 * flagged, while the polled sample keeps everything. */
TEST_LOGGER4(streamedSampleTruncatedToSmallerCallbackStorage)
{
    RBRGen4Error err;
    RBRGEN4_SAMPLE_DECL(actual, TESTS_CHANNEL_MAX);

    conn->outputFormat =
        (RBRGen4OutputFormat) {.sn = true, .scheduleLabel = true, .dateTime = false, .crc = false};
    TestIOBuffers_init(buffers,
                       "RBR 999999 sch_asc_pts 9.84050000e+000 12.5362918e+000" RESPONSE_TERMINATOR
                       "RBR 999999 polling 9.83602000e+000 1.5e+000" RESPONSE_TERMINATOR,
                       0);
    buffers->streamSample.size = 1;
    buffers->streamReadings[1] = -1.0;

    RBRGEN4_LABEL_LIST_DECL(channelList, 1);
    PollTest_labelList("pressure_00", &channelList);
    err = RBRGen4_pollChannels(conn, true, &channelList, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("polling", actual.scheduleLabel);
    TEST_ASSERT_EQ(2, actual.channelCount, "%" PRIi32);
    TEST_ASSERT_EQ(false, actual.readingsDropped, "%d");
    TEST_ASSERT_EQ(9.83602000, actual.readings[0], "%lf");
    TEST_ASSERT_EQ(1.5, actual.readings[1], "%lf");

    TEST_ASSERT_STR_EQ("sch_asc_pts", buffers->deliveredSample.scheduleLabel);
    TEST_ASSERT_EQ(1, buffers->deliveredSample.channelCount, "%" PRIi32);
    TEST_ASSERT_EQ(true, buffers->deliveredSample.readingsDropped, "%d");
    TEST_ASSERT_EQ(9.84050000, buffers->deliveredSample.readings[0], "%lf");
    /* Untouched: the copy stops at the callback sample's own size. */
    TEST_ASSERT_EQ(-1.0, buffers->streamReadings[1], "%lf");

    return true;
}

TEST_LOGGER4(readSampleWithoutCallback)
{
    RBRGen4Error err;
    RBRGen4SampleCallback savedCallback = conn->environment.sample;

    conn->environment.sample = NULL;

    conn->outputFormat = (RBRGen4OutputFormat) {
        .sn = false,
        .scheduleLabel = false,
        .dateTime = false,
        .crc = false,
    };
    TestIOBuffers_init(buffers, "9.85054000e+000 12.5359318e+000" RESPONSE_TERMINATOR, 0);

    err = RBRGen4_readSample(conn);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_MISSING_CALLBACK, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    conn->environment.sample = savedCallback;

    return true;
}
