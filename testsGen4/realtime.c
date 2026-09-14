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
    const char *expectedCommand;
    const char *response;
    RBRInstrumentGen4OutputFormat outputFormat;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4Sample expected;
} PollTest;

#define OUTPUTFORMAT_DEFAULT \
    { .sn = false, .scheduleLabel = false, .dateTime = true, .crc = false }

TEST_LOGGER4(poll)
{
    PollTest tests[] = {
        /* A bare poll samples every channel. */
        { NULL,
          NULL,
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
              RBRInstrumentGen4Reading_setError(9) } } },
        { "temperature_00",
          NULL,
          "poll channellist=temperature_00" COMMAND_TERMINATOR,
          "2024-03-10 07:01:07.000 Error-01" RESPONSE_TERMINATOR,
          OUTPUTFORMAT_DEFAULT,
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 1710054067000LL,
            .channelCount = 1,
            .readings = { RBRInstrumentGen4Reading_setError(1) } } },
        /* A sample's timestamp may be a bare count of milliseconds. */
        { NULL,
          NULL,
          "poll" COMMAND_TERMINATOR,
          "12345 12.5364470e+000 9.91695000e+000" RESPONSE_TERMINATOR,
          OUTPUTFORMAT_DEFAULT,
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 12345LL,
            .channelCount = 2,
            .readings = { 12.5364470, 9.91695000 } } },
        /* A reading is never taken for a millisecond timestamp, so a line
         * missing its timestamp is refused as a sample. The library then
         * keeps waiting for one; here the read buffer runs dry first. */
        { NULL,
          NULL,
          "poll" COMMAND_TERMINATOR,
          "12.5364470e+000 9.91695000e+000" RESPONSE_TERMINATOR,
          OUTPUTFORMAT_DEFAULT,
          RBRINSTRUMENTGEN4_CALLBACK_ERROR,
          { 0 } },
        /* A repeated channel is reported at every requested position. */
        { "pressure_00|temperature_00|pressure_00",
          NULL,
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
              RBRInstrumentGen4Reading_setError(9) } } },
        { NULL,
          "g",
          "poll grouplist=g" COMMAND_TERMINATOR,
          "2024-03-10 07:01:10.000 Error-09" RESPONSE_TERMINATOR,
          OUTPUTFORMAT_DEFAULT,
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 1710054070000LL,
            .channelCount = 1,
            .readings = { RBRInstrumentGen4Reading_setError(9) } } },
        /* Successful readings parse as their values. */
        { NULL,
          NULL,
          "poll" COMMAND_TERMINATOR,
          "2024-10-21 11:50:49.000 18.1745130 12.7052970 2.69308210"
          RESPONSE_TERMINATOR,
          OUTPUTFORMAT_DEFAULT,
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 1729511449000LL,
            .channelCount = 3,
            .readings = { 18.1745130, 12.7052970, 2.69308210 } } },
        /* Polled samples carry the “polling” schedule label when enabled. */
        { "temperature_00",
          NULL,
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
            .readings = { RBRInstrumentGen4Reading_setError(1) } } },
        { "temperature_00",
          NULL,
          "poll channellist=temperature_00" COMMAND_TERMINATOR,
          "RBR 999999 2024-03-10 07:01:33.000 Error-01" RESPONSE_TERMINATOR,
          { .sn = true,
            .scheduleLabel = false,
            .dateTime = true,
            .crc = false },
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 1710054093000LL,
            .channelCount = 1,
            .readings = { RBRInstrumentGen4Reading_setError(1) } } },
        { "temperature_00",
          NULL,
          "poll channellist=temperature_00" COMMAND_TERMINATOR,
          "2024-03-10 07:01:35.000 Error-01 0x3C7A" RESPONSE_TERMINATOR,
          { .sn = false,
            .scheduleLabel = false,
            .dateTime = true,
            .crc = true },
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 1710054095000LL,
            .channelCount = 1,
            .readings = { RBRInstrumentGen4Reading_setError(1) } } },
        /* A single reading with no other fields is a one-token sample. */
        { "pressure_00",
          NULL,
          "poll channellist=pressure_00" COMMAND_TERMINATOR,
          "9.85289000e+000" RESPONSE_TERMINATOR,
          { .sn = false,
            .scheduleLabel = false,
            .dateTime = false,
            .crc = false },
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 0,
            .channelCount = 1,
            .readings = { 9.85289000 } } },
        { "pressure_00",
          NULL,
          "poll channellist=pressure_00" COMMAND_TERMINATOR,
          "9.80449000e+000 0xF80B" RESPONSE_TERMINATOR,
          { .sn = false,
            .scheduleLabel = false,
            .dateTime = false,
            .crc = true },
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 0,
            .channelCount = 1,
            .readings = { 9.80449000 } } },
        /* The first reading is not dropped when the timestamp,
         * schedule label, and serial number are all omitted. */
        { NULL,
          NULL,
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
                          -0.304020648, -0.302152465 } } },
        { NULL,
          NULL,
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
                          -0.312160648, -0.310242445 } } },
        { NULL,
          NULL,
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
                          -0.319890648, -0.317924945 } } },
        { NULL,
          NULL,
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
                          -0.337070648, -0.334999375 } } },
        { NULL,
          NULL,
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
                          -0.324300648, -0.322307846 } } },
        { NULL,
          NULL,
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
                          -0.314250648, -0.312319602 } } },
        { NULL,
          NULL,
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
                          -0.321410648, -0.319435605 } } },
        { NULL,
          NULL,
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
                          -0.322910648, -0.320926387 } } },
        /* A float64-encoded sample with every optional prefix still
         * retains its first reading. */
        { NULL,
          NULL,
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
                          -0.331880648498535, -0.329841267502290 } } },
        /* An unknown channel or group label is a hardware error. */
        { "nosuchchannel_00",
          NULL,
          "poll channellist=nosuchchannel_00" COMMAND_TERMINATOR,
          "ERR-117 'nosuchchannel_00' is not a known qualifier"
          RESPONSE_TERMINATOR,
          OUTPUTFORMAT_DEFAULT,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR,
          { 0 } },
        { NULL,
          "nosuchgroup",
          "poll grouplist=nosuchgroup" COMMAND_TERMINATOR,
          "ERR-117 'nosuchgroup' is not a known qualifier"
          RESPONSE_TERMINATOR,
          OUTPUTFORMAT_DEFAULT,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR,
          { 0 } },
        /* An empty list is refused by the instrument, not the library. */
        { "",
          NULL,
          "poll channellist=" COMMAND_TERMINATOR,
          "ERR-115 syntax error ''" RESPONSE_TERMINATOR,
          OUTPUTFORMAT_DEFAULT,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR,
          { 0 } },
        { NULL, NULL, NULL, NULL, { 0 }, 0, { 0 } }
    };

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Sample actual;

    for (int i = 0; tests[i].expectedCommand != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        instrument->outputFormat = tests[i].outputFormat;
        if (tests[i].channelList != NULL)
        {
            err = RBRInstrumentGen4_pollChannels(instrument,
                                                 tests[i].channelList,
                                                 &actual);
        }
        else if (tests[i].groupList != NULL)
        {
            err = RBRInstrumentGen4_pollGroups(instrument,
                                               tests[i].groupList,
                                               &actual);
        }
        else
        {
            err = RBRInstrumentGen4_poll(instrument, &actual);
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
    }

    /* A label list too long to send is refused before the command. */
    char longList[RBRINSTRUMENTGEN4_COMMAND_BUFFER_MAX];
    memset(longList, 'a', sizeof(longList) - 1);
    longList[sizeof(longList) - 1] = '\0';
    TestIOBuffers_init(buffers, "", 0);
    err = RBRInstrumentGen4_pollChannels(instrument, longList, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE,
                        err,
                        RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}
