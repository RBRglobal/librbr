/**
 * \file realtime.c
 *
 * \brief Tests for instrument realtime data commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for memset. */
#include <string.h>

#include "RBRInstrumentGen4Realtime.h"
#include "tests.h"

typedef struct PollTest
{
    const char *channelList;
    const char *groupList;
    bool requireLabel;
    const char *expectedCommand;
    const char *response;
    RBRInstrumentGen4OutputFormat outputFormat;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4Sample expected;
    /** \brief The streamed sample expected to reach the sample callback;
     * asserted only when its channelCount is non-zero. */
    RBRInstrumentGen4Sample expectedStreamed;
} PollTest;

#define OUTPUTFORMAT_DEFAULT \
    { .sn = false, .scheduleLabel = false, .dateTime = true, .crc = false }

TEST_LOGGER4(poll)
{
    PollTest tests[] = {
        /* A bare poll samples every channel. */
        { NULL,
          NULL,
          false,
          "poll" COMMAND_TERMINATOR,
          "2024-03-10 07:01:06.000 Error-01 Error-09 Error-09 Error-09"
          RESPONSE_TERMINATOR,
          OUTPUTFORMAT_DEFAULT,
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 1710054066000LL,
            .channelCount = 4,
            .readings = {
              RBRInstrumentGen4Reading_setError(1),
              RBRInstrumentGen4Reading_setError(9),
              RBRInstrumentGen4Reading_setError(9),
              RBRInstrumentGen4Reading_setError(9) } },
          { 0 } },
        { "temperature_00",
          NULL,
          false,
          "poll channellist=temperature_00" COMMAND_TERMINATOR,
          "2024-03-10 07:01:07.000 Error-01" RESPONSE_TERMINATOR,
          OUTPUTFORMAT_DEFAULT,
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 1710054067000LL,
            .channelCount = 1,
            .readings = { RBRInstrumentGen4Reading_setError(1) } },
          { 0 } },
        /* A sample's timestamp may be a bare count of milliseconds. */
        { NULL,
          NULL,
          false,
          "poll" COMMAND_TERMINATOR,
          "12345 12.5364470e+000 9.91695000e+000" RESPONSE_TERMINATOR,
          OUTPUTFORMAT_DEFAULT,
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 12345LL,
            .channelCount = 2,
            .readings = { 12.5364470, 9.91695000 } },
          { 0 } },
        /* A reading is never taken for a millisecond timestamp, so a line
         * missing its timestamp is refused as a sample. The library then
         * keeps waiting for one; here the read buffer runs dry first. */
        { NULL,
          NULL,
          false,
          "poll" COMMAND_TERMINATOR,
          "12.5364470e+000 9.91695000e+000" RESPONSE_TERMINATOR,
          OUTPUTFORMAT_DEFAULT,
          RBRINSTRUMENTGEN4_CALLBACK_ERROR,
          { 0 },
          { 0 } },
        /* A repeated channel is reported at every requested position. */
        { "pressure_00|temperature_00|pressure_00",
          NULL,
          false,
          "poll channellist=pressure_00|temperature_00|pressure_00"
          COMMAND_TERMINATOR,
          "2024-03-10 07:01:09.000 Error-09 Error-01 Error-09"
          RESPONSE_TERMINATOR,
          OUTPUTFORMAT_DEFAULT,
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 1710054069000LL,
            .channelCount = 3,
            .readings = {
              RBRInstrumentGen4Reading_setError(9),
              RBRInstrumentGen4Reading_setError(1),
              RBRInstrumentGen4Reading_setError(9) } },
          { 0 } },
        { NULL,
          "g",
          false,
          "poll grouplist=g" COMMAND_TERMINATOR,
          "2024-03-10 07:01:10.000 Error-09" RESPONSE_TERMINATOR,
          OUTPUTFORMAT_DEFAULT,
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 1710054070000LL,
            .channelCount = 1,
            .readings = { RBRInstrumentGen4Reading_setError(9) } },
          { 0 } },
        /* Successful readings parse as their values. */
        { NULL,
          NULL,
          false,
          "poll" COMMAND_TERMINATOR,
          "2024-10-21 11:50:49.000 18.1745130 12.7052970 2.69308210"
          RESPONSE_TERMINATOR,
          OUTPUTFORMAT_DEFAULT,
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 1729511449000LL,
            .channelCount = 3,
            .readings = { 18.1745130, 12.7052970, 2.69308210 } },
          { 0 } },
        /* Polled samples carry the “polling” schedule label when enabled. */
        { "temperature_00",
          NULL,
          false,
          "poll channellist=temperature_00" COMMAND_TERMINATOR,
          "polling 2024-03-10 07:01:31.000 Error-01" RESPONSE_TERMINATOR,
          { .sn = false,
            .scheduleLabel = true,
            .dateTime = true,
            .crc = false },
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 1710054091000LL,
            .scheduleLabel = "polling",
            .channelCount = 1,
            .readings = { RBRInstrumentGen4Reading_setError(1) } },
          { 0 } },
        { "temperature_00",
          NULL,
          false,
          "poll channellist=temperature_00" COMMAND_TERMINATOR,
          "RBR 999999 2024-03-10 07:01:33.000 Error-01" RESPONSE_TERMINATOR,
          { .sn = true,
            .scheduleLabel = false,
            .dateTime = true,
            .crc = false },
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 1710054093000LL,
            .channelCount = 1,
            .readings = { RBRInstrumentGen4Reading_setError(1) } },
          { 0 } },
        { "temperature_00",
          NULL,
          false,
          "poll channellist=temperature_00" COMMAND_TERMINATOR,
          "2024-03-10 07:01:35.000 Error-01 0x3C7A" RESPONSE_TERMINATOR,
          { .sn = false,
            .scheduleLabel = false,
            .dateTime = true,
            .crc = true },
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 1710054095000LL,
            .channelCount = 1,
            .readings = { RBRInstrumentGen4Reading_setError(1) } },
          { 0 } },
        /* A single reading with no other fields is a one-token sample. */
        { "pressure_00",
          NULL,
          false,
          "poll channellist=pressure_00" COMMAND_TERMINATOR,
          "9.85289000e+000" RESPONSE_TERMINATOR,
          { .sn = false,
            .scheduleLabel = false,
            .dateTime = false,
            .crc = false },
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 0,
            .channelCount = 1,
            .readings = { 9.85289000 } },
          { 0 } },
        { "pressure_00",
          NULL,
          false,
          "poll channellist=pressure_00" COMMAND_TERMINATOR,
          "9.80449000e+000 0xF80B" RESPONSE_TERMINATOR,
          { .sn = false,
            .scheduleLabel = false,
            .dateTime = false,
            .crc = true },
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 0,
            .channelCount = 1,
            .readings = { 9.80449000 } },
          { 0 } },
        /* The first reading is not dropped when the timestamp,
         * schedule label, and serial number are all omitted. */
        { NULL,
          NULL,
          false,
          "poll" COMMAND_TERMINATOR,
          "12.5363015e+000 9.82848000e+000 -304.020648e-003"
          " -302.152465e-003" RESPONSE_TERMINATOR,
          { .sn = false,
            .scheduleLabel = false,
            .dateTime = false,
            .crc = false },
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 0,
            .channelCount = 4,
            .readings = { 12.5363015, 9.82848000,
                          -0.304020648, -0.302152465 } },
          { 0 } },
        { NULL,
          NULL,
          false,
          "poll" COMMAND_TERMINATOR,
          "12.5361167e+000 9.82034000e+000 -312.160648e-003"
          " -310.242445e-003 0x1E75" RESPONSE_TERMINATOR,
          { .sn = false,
            .scheduleLabel = false,
            .dateTime = false,
            .crc = true },
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 0,
            .channelCount = 4,
            .readings = { 12.5361167, 9.82034000,
                          -0.312160648, -0.310242445 } },
          { 0 } },
        { NULL,
          NULL,
          false,
          "poll" COMMAND_TERMINATOR,
          "polling 12.5356691e+000 9.81261000e+000 -319.890648e-003"
          " -317.924945e-003" RESPONSE_TERMINATOR,
          { .sn = false,
            .scheduleLabel = true,
            .dateTime = false,
            .crc = false },
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 0,
            .scheduleLabel = "polling",
            .channelCount = 4,
            .readings = { 12.5356691, 9.81261000,
                          -0.319890648, -0.317924945 } },
          { 0 } },
        { NULL,
          NULL,
          false,
          "poll" COMMAND_TERMINATOR,
          "polling 12.5356399e+000 9.79543000e+000 -337.070648e-003"
          " -334.999375e-003 0x271D" RESPONSE_TERMINATOR,
          { .sn = false,
            .scheduleLabel = true,
            .dateTime = false,
            .crc = true },
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 0,
            .scheduleLabel = "polling",
            .channelCount = 4,
            .readings = { 12.5356399, 9.79543000,
                          -0.337070648, -0.334999375 } },
          { 0 } },
        { NULL,
          NULL,
          false,
          "poll" COMMAND_TERMINATOR,
          "RBR 999999 12.5360777e+000 9.80820000e+000 -324.300648e-003"
          " -322.307846e-003" RESPONSE_TERMINATOR,
          { .sn = true,
            .scheduleLabel = false,
            .dateTime = false,
            .crc = false },
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 0,
            .channelCount = 4,
            .readings = { 12.5360777, 9.80820000,
                          -0.324300648, -0.322307846 } },
          { 0 } },
        { NULL,
          NULL,
          false,
          "poll" COMMAND_TERMINATOR,
          "RBR 999999 12.5362042e+000 9.81825000e+000 -314.250648e-003"
          " -312.319602e-003 0x3BE3" RESPONSE_TERMINATOR,
          { .sn = true,
            .scheduleLabel = false,
            .dateTime = false,
            .crc = true },
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 0,
            .channelCount = 4,
            .readings = { 12.5362042, 9.81825000,
                          -0.314250648, -0.312319602 } },
          { 0 } },
        { NULL,
          NULL,
          false,
          "poll" COMMAND_TERMINATOR,
          "RBR 999999 polling 12.5369048e+000 9.81109000e+000"
          " -321.410648e-003 -319.435605e-003" RESPONSE_TERMINATOR,
          { .sn = true,
            .scheduleLabel = true,
            .dateTime = false,
            .crc = false },
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 0,
            .scheduleLabel = "polling",
            .channelCount = 4,
            .readings = { 12.5369048, 9.81109000,
                          -0.321410648, -0.319435605 } },
          { 0 } },
        { NULL,
          NULL,
          false,
          "poll" COMMAND_TERMINATOR,
          "RBR 999999 polling 12.5369632e+000 9.80959000e+000"
          " -322.910648e-003 -320.926387e-003 0xF4F3" RESPONSE_TERMINATOR,
          { .sn = true,
            .scheduleLabel = true,
            .dateTime = false,
            .crc = true },
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 0,
            .scheduleLabel = "polling",
            .channelCount = 4,
            .readings = { 12.5369632, 9.80959000,
                          -0.322910648, -0.320926387 } },
          { 0 } },
        /* A float64-encoded sample with every optional prefix still
         * retains its first reading. */
        { NULL,
          NULL,
          false,
          "poll" COMMAND_TERMINATOR,
          "RBR 999999 polling 2024-03-13 12:37:36.000"
          " 12.5369242316864e+000 9.80062000000000e+000"
          " -331.880648498535e-003 -329.841267502290e-003 0xE4EE"
          RESPONSE_TERMINATOR,
          { .sn = true,
            .scheduleLabel = true,
            .dateTime = true,
            .crc = true,
            .dataType = RBRINSTRUMENTGEN4_DATATYPE_FLOAT64 },
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 1710333456000LL,
            .scheduleLabel = "polling",
            .channelCount = 4,
            .readings = { 12.5369242316864, 9.80062000000000,
                          -0.331880648498535, -0.329841267502290 } },
          { 0 } },
        /* requireLabel is checked before anything is sent. */
        { "pressure_00",
          NULL,
          true,
          "",
          "",
          { .sn = false,
            .scheduleLabel = false,
            .dateTime = false,
            .crc = false },
          RBRINSTRUMENTGEN4_UNSUPPORTED,
          { 0 },
          { 0 } },
        /* With requireLabel set, streamed samples read while waiting for
         * the polled one are forwarded to the sample callback and skipped. */
        { "pressure_00",
          NULL,
          true,
          "poll channellist=pressure_00" COMMAND_TERMINATOR,
          "RBR 999999 sch_asc_pts 9.84050000e+000 12.5362918e+000"
          RESPONSE_TERMINATOR
          "RBR 999999 polling 9.83602000e+000" RESPONSE_TERMINATOR,
          { .sn = true,
            .scheduleLabel = true,
            .dateTime = false,
            .crc = false },
          RBRINSTRUMENTGEN4_SUCCESS,
          { .scheduleLabel = "polling",
            .channelCount = 1,
            .readings = { 9.83602000 } },
          { .scheduleLabel = "sch_asc_pts",
            .channelCount = 2,
            .readings = { 9.84050000, 12.5362918 } } },
        /* Without requireLabel, the first sample read wins, even if it's a
         * streamed sample. */
        { "pressure_00",
          NULL,
          false,
          "poll channellist=pressure_00" COMMAND_TERMINATOR,
          "RBR 999999 sch_asc_pts 9.84050000e+000 12.5362918e+000"
          RESPONSE_TERMINATOR
          "RBR 999999 polling 9.83602000e+000" RESPONSE_TERMINATOR,
          { .sn = true,
            .scheduleLabel = true,
            .dateTime = false,
            .crc = false },
          RBRINSTRUMENTGEN4_SUCCESS,
          { .scheduleLabel = "sch_asc_pts",
            .channelCount = 2,
            .readings = { 9.84050000, 12.5362918 } },
          { 0 } },
        /* An unknown channel or group label is a hardware error. */
        { "nosuchchannel_00",
          NULL,
          false,
          "poll channellist=nosuchchannel_00" COMMAND_TERMINATOR,
          "ERR-117 'nosuchchannel_00' is not a known qualifier"
          RESPONSE_TERMINATOR,
          OUTPUTFORMAT_DEFAULT,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR,
          { 0 },
          { 0 } },
        { NULL,
          "nosuchgroup",
          false,
          "poll grouplist=nosuchgroup" COMMAND_TERMINATOR,
          "ERR-117 'nosuchgroup' is not a known qualifier"
          RESPONSE_TERMINATOR,
          OUTPUTFORMAT_DEFAULT,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR,
          { 0 },
          { 0 } },
        /* An empty list is refused by the instrument, not the library. */
        { "",
          NULL,
          false,
          "poll channellist=" COMMAND_TERMINATOR,
          "ERR-115 syntax error ''" RESPONSE_TERMINATOR,
          OUTPUTFORMAT_DEFAULT,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR,
          { 0 },
          { 0 } },
        { NULL, NULL, false, NULL, NULL, { 0 }, 0, { 0 },
          { 0 } }
    };

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Sample actual;

    for (int i = 0; tests[i].expectedCommand != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        memset(&buffers->streamSample, 0, sizeof(buffers->streamSample));
        instrument->outputFormat = tests[i].outputFormat;
        if (tests[i].channelList != NULL)
        {
            err = RBRInstrumentGen4_pollChannels(instrument,
                                                 tests[i].requireLabel,
                                                 tests[i].channelList,
                                                 &actual);
        }
        else if (tests[i].groupList != NULL)
        {
            err = RBRInstrumentGen4_pollGroups(instrument,
                                               tests[i].requireLabel,
                                               tests[i].groupList,
                                               &actual);
        }
        else
        {
            err = RBRInstrumentGen4_poll(instrument,
                                         tests[i].requireLabel,
                                         &actual);
        }
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].expectedCommand, buffers->writeBuffer);

        if (tests[i].expectedError != RBRINSTRUMENTGEN4_SUCCESS)
        {
            continue;
        }

        TEST_ASSERT_EQ(tests[i].expected.timestamp,
                       actual.timestamp,
                       "%" PRIi64);
        TEST_ASSERT_EQ(tests[i].expected.channelCount,
                       actual.channelCount,
                       "%" PRIi32);
        TEST_ASSERT_STR_EQ(tests[i].expected.scheduleLabel,
                           actual.scheduleLabel);
        for (int32_t channel = 0; channel < actual.channelCount; ++channel)
        {
            TEST_ASSERT_ENUM_EQ(
                RBRInstrumentGen4Reading_isError(
                    tests[i].expected.readings[channel]),
                RBRInstrumentGen4Reading_isError(actual.readings[channel]),
                bool);

            if (RBRInstrumentGen4Reading_isError(actual.readings[channel]))
            {
                TEST_ASSERT_EQ(
                    RBRInstrumentGen4Reading_getError(
                        tests[i].expected.readings[channel]),
                    RBRInstrumentGen4Reading_getError(
                        actual.readings[channel]),
                    "%" PRIi8);
            }
            else
            {
                TEST_ASSERT_EQ(tests[i].expected.readings[channel],
                               actual.readings[channel],
                               "%lf");
            }
        }

        if (tests[i].expectedStreamed.channelCount != 0)
        {
            TEST_ASSERT_EQ(tests[i].expectedStreamed.channelCount,
                           buffers->streamSample.channelCount,
                           "%" PRIi32);
            TEST_ASSERT_STR_EQ(tests[i].expectedStreamed.scheduleLabel,
                               buffers->streamSample.scheduleLabel);
            for (int32_t channel = 0;
                 channel < tests[i].expectedStreamed.channelCount;
                 ++channel)
            {
                TEST_ASSERT_EQ(tests[i].expectedStreamed.readings[channel],
                               buffers->streamSample.readings[channel],
                               "%lf");
            }
        }
    }

    /* A label list too long to send is refused before the command. */
    char longList[RBRINSTRUMENTGEN4_COMMAND_BUFFER_MAX];
    memset(longList, 'a', sizeof(longList) - 1);
    longList[sizeof(longList) - 1] = '\0';
    TestIOBuffers_init(buffers, "", 0);
    err = RBRInstrumentGen4_pollChannels(instrument, false, longList, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE,
                        err,
                        RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

/** \brief A fake clock, advanced by pollTimeoutTime() on every call. */
static RBRInstrumentGen4DateTime pollTimeoutClock;

/** \brief A time callback that advances #pollTimeoutClock by less than a
 * third of RBRInstrumentGen4.pollTimeout on every call, so a poll's
 * overall timeout can be exercised without waiting in real time. */
static RBRInstrumentGen4Error pollTimeoutTime(
    const struct RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4DateTime *time)
{
    (void) instrument;
    *time = pollTimeoutClock;
    pollTimeoutClock += 3000;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

TEST_LOGGER4(pollTimeout)
{
    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Sample actual;
    RBRInstrumentGen4TimeCallback savedTime = instrument->callbacks.time;
    RBRInstrumentGen4DateTime savedPollTimeout = instrument->pollTimeout;

    pollTimeoutClock = 0;
    instrument->callbacks.time = pollTimeoutTime;
    instrument->pollTimeout = 10000;

    instrument->outputFormat = (RBRInstrumentGen4OutputFormat) {
        .sn = false,
        .scheduleLabel = true,
        .dateTime = false,
        .crc = false
    };
    /* Six streamed samples, and no polled sample at all: if the overall
     * poll timeout didn't bound the wait, the read buffer would exhaust
     * before the poll gave up. */
    TestIOBuffers_init(
        buffers,
        "sch_asc_pts 9.84050000e+000 12.5362918e+000" RESPONSE_TERMINATOR
        "sch_asc_pts 9.84050000e+000 12.5362918e+000" RESPONSE_TERMINATOR
        "sch_asc_pts 9.84050000e+000 12.5362918e+000" RESPONSE_TERMINATOR
        "sch_asc_pts 9.84050000e+000 12.5362918e+000" RESPONSE_TERMINATOR
        "sch_asc_pts 9.84050000e+000 12.5362918e+000" RESPONSE_TERMINATOR
        "sch_asc_pts 9.84050000e+000 12.5362918e+000" RESPONSE_TERMINATOR,
        0);
    memset(&buffers->streamSample, 0, sizeof(buffers->streamSample));

    err = RBRInstrumentGen4_pollChannels(instrument,
                                         true,
                                         "pressure_00",
                                         &actual);

    instrument->callbacks.time = savedTime;
    instrument->pollTimeout = savedPollTimeout;

    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_TIMEOUT,
                        err,
                        RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("poll channellist=pressure_00" COMMAND_TERMINATOR,
                        buffers->writeBuffer);
    /* At least one streamed sample must have been forwarded before the
     * timeout fired. */
    TEST_ASSERT_EQ(2, buffers->streamSample.channelCount, "%" PRIi32);

    return true;
}

/** \brief A fake clock, advanced by pollSlowResponseTime() on every call. */
static RBRInstrumentGen4DateTime pollSlowResponseClock;

/** \brief A time callback that advances #pollSlowResponseClock by more than
 * a small commandTimeout on every call, so that a wait spanning several
 * such ticks can be exercised without waiting in real time. */
static RBRInstrumentGen4Error pollSlowResponseTime(
    const struct RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4DateTime *time)
{
    (void) instrument;
    *time = pollSlowResponseClock;
    pollSlowResponseClock += 500;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

TEST_LOGGER4(pollSlowResponse)
{
    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Sample actual;
    RBRInstrumentGen4TimeCallback savedTime = instrument->callbacks.time;
    RBRInstrumentGen4DateTime savedCommandTimeout
        = instrument->commandTimeout;
    RBRInstrumentGen4DateTime savedPollTimeout = instrument->pollTimeout;

    pollSlowResponseClock = 0;
    instrument->callbacks.time = pollSlowResponseTime;
    /* commandTimeout is small enough that it would trip on its own between
     * clock ticks; only pollTimeout is large enough to bound the wait. */
    instrument->commandTimeout = 100;
    instrument->pollTimeout = 10000;

    instrument->outputFormat = (RBRInstrumentGen4OutputFormat) {
        .sn = false,
        .scheduleLabel = true,
        .dateTime = false,
        .crc = false
    };
    /* A single polled sample, arriving only after silence long enough to
     * exceed commandTimeout. */
    TestIOBuffers_init(
        buffers,
        "polling 12.5364470e+000 9.91695000e+000" RESPONSE_TERMINATOR,
        0);

    err = RBRInstrumentGen4_poll(instrument, true, &actual);

    instrument->callbacks.time = savedTime;
    instrument->commandTimeout = savedCommandTimeout;
    instrument->pollTimeout = savedPollTimeout;

    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS,
                        err,
                        RBRInstrumentGen4Error);
    TEST_ASSERT_EQ(2, actual.channelCount, "%" PRIi32);

    return true;
}
