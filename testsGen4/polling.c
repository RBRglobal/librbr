/**
 * \file polling.c
 *
 * \brief Tests for instrument polling commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for memset. */
#include <string.h>

#include "RBRInstrumentGen4Polling.h"
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
              RBRInstrumentGen4Reading_setError(
                RBRINSTRUMENTGEN4_READING_FLAG_ERROR, 1),
              RBRInstrumentGen4Reading_setError(
                RBRINSTRUMENTGEN4_READING_FLAG_ERROR, 9),
              RBRInstrumentGen4Reading_setError(
                RBRINSTRUMENTGEN4_READING_FLAG_ERROR, 9),
              RBRInstrumentGen4Reading_setError(
                RBRINSTRUMENTGEN4_READING_FLAG_ERROR, 9) } } },
        { "temperature_00",
          NULL,
          "poll channellist=temperature_00" COMMAND_TERMINATOR,
          "2024-03-10 07:01:07.000 Error-01" RESPONSE_TERMINATOR,
          OUTPUTFORMAT_DEFAULT,
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 1710054067000LL,
            .channelCount = 1,
            .readings = { RBRInstrumentGen4Reading_setError(
                RBRINSTRUMENTGEN4_READING_FLAG_ERROR, 1) } } },
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
              RBRInstrumentGen4Reading_setError(
                RBRINSTRUMENTGEN4_READING_FLAG_ERROR, 9),
              RBRInstrumentGen4Reading_setError(
                RBRINSTRUMENTGEN4_READING_FLAG_ERROR, 1),
              RBRInstrumentGen4Reading_setError(
                RBRINSTRUMENTGEN4_READING_FLAG_ERROR, 9) } } },
        { NULL,
          "g",
          "poll grouplist=g" COMMAND_TERMINATOR,
          "2024-03-10 07:01:10.000 Error-09" RESPONSE_TERMINATOR,
          OUTPUTFORMAT_DEFAULT,
          RBRINSTRUMENTGEN4_SUCCESS,
          { .timestamp = 1710054070000LL,
            .channelCount = 1,
            .readings = { RBRInstrumentGen4Reading_setError(
                RBRINSTRUMENTGEN4_READING_FLAG_ERROR, 9) } } },
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
            .channelCount = 1,
            .readings = { RBRInstrumentGen4Reading_setError(
                RBRINSTRUMENTGEN4_READING_FLAG_ERROR, 1) } } },
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
            .readings = { RBRInstrumentGen4Reading_setError(
                RBRINSTRUMENTGEN4_READING_FLAG_ERROR, 1) } } },
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
            .readings = { RBRInstrumentGen4Reading_setError(
                RBRINSTRUMENTGEN4_READING_FLAG_ERROR, 1) } } },
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
        for (int32_t channel = 0; channel < actual.channelCount; ++channel)
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
                    RBRInstrumentGen4Reading_getError(
                        actual.readings[channel]),
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
