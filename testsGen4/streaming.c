/**
 * \file streaming.c
 *
 * \brief Tests for instrument streaming data commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for memset. */
#include <string.h>

#include "RBRInstrumentGen4Streaming.h"
#include "tests.h"

typedef struct ReadSampleTest
{
    RBRInstrumentGen4OutputFormat outputFormat;
    const char *response;
    RBRInstrumentGen4Sample expected;
} ReadSampleTest;

TEST_LOGGER4(readSample)
{
    ReadSampleTest tests[] = {
        /* sn, scheduleLabel, dateTime, and crc all on. */
        { { .sn = true,
            .scheduleLabel = true,
            .dateTime = true,
            .crc = true },
          "RBR 999999 sch_asc_pts 2024-03-13 12:45:53.000 9.84033000e+000"
          " 12.5358150e+000 0x25C8" RESPONSE_TERMINATOR,
          { .timestamp = 1710333953000LL,
            .scheduleLabel = "sch_asc_pts",
            .channelCount = 2,
            .readings = { 9.84033000, 12.5358150 } } },
        /* sn, scheduleLabel, dateTime, and crc all off. */
        { { .sn = false,
            .scheduleLabel = false,
            .dateTime = false,
            .crc = false },
          "9.85054000e+000 12.5359318e+000" RESPONSE_TERMINATOR,
          { .timestamp = 0,
            .scheduleLabel = "",
            .channelCount = 2,
            .readings = { 9.85054000, 12.5359318 } } },
        /* Only scheduleLabel and crc on. */
        { { .sn = false,
            .scheduleLabel = true,
            .dateTime = false,
            .crc = true },
          "sch_asc_pts 9.83676000e+000 12.5361556e+000 0xC4EC"
          RESPONSE_TERMINATOR,
          { .timestamp = 0,
            .scheduleLabel = "sch_asc_pts",
            .channelCount = 2,
            .readings = { 9.83676000, 12.5361556 } } },
        /* Only sn on, with a float64-encoded sample. */
        { { .sn = true,
            .scheduleLabel = false,
            .dateTime = false,
            .crc = false,
            .dataType = RBRINSTRUMENTGEN4_DATATYPE_FLOAT64 },
          "RBR 999999 9.84050000000000e+000 12.5362917963016e+000"
          RESPONSE_TERMINATOR,
          { .timestamp = 0,
            .scheduleLabel = "",
            .channelCount = 2,
            .readings = { 9.84050000000000, 12.5362917963016 } } },
        /* The same as the first row, but preceded by the command prompt,
         * which must be trimmed before the sample is parsed. */
        { { .sn = true,
            .scheduleLabel = true,
            .dateTime = true,
            .crc = true },
          "ready: RBR 999999 sch_asc_pts 2024-03-13 12:45:49.000"
          " 9.83297000e+000 12.5356886e+000 0x29C1" RESPONSE_TERMINATOR,
          { .timestamp = 1710333949000LL,
            .scheduleLabel = "sch_asc_pts",
            .channelCount = 2,
            .readings = { 9.83297000, 12.5356886 } } },
    };

    RBRInstrumentGen4Error err;

    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        memset(&buffers->streamSample, 0, sizeof(buffers->streamSample));
        instrument->outputFormat = tests[i].outputFormat;

        err = RBRInstrumentGen4_readSample(instrument);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS,
                            err,
                            RBRInstrumentGen4Error);

        TEST_ASSERT_EQ(tests[i].expected.timestamp,
                       buffers->streamSample.timestamp,
                       "%" PRIi64);
        TEST_ASSERT_STR_EQ(tests[i].expected.scheduleLabel,
                           buffers->streamSample.scheduleLabel);
        TEST_ASSERT_EQ(tests[i].expected.channelCount,
                       buffers->streamSample.channelCount,
                       "%" PRIi32);
        for (int32_t channel = 0;
             channel < buffers->streamSample.channelCount;
             ++channel)
        {
            TEST_ASSERT_ENUM_EQ(
                RBRInstrumentGen4Reading_isError(
                    tests[i].expected.readings[channel]),
                RBRInstrumentGen4Reading_isError(buffers->streamSample.readings[channel]),
                bool);

            if (RBRInstrumentGen4Reading_isError(buffers->streamSample.readings[channel]))
            {
                TEST_ASSERT_EQ(
                    RBRInstrumentGen4Reading_getError(
                        tests[i].expected.readings[channel]),
                    RBRInstrumentGen4Reading_getError(
                        buffers->streamSample.readings[channel]),
                    "%" PRIi8);
            }
            else
            {
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
    RBRInstrumentGen4Error err;

    instrument->outputFormat = (RBRInstrumentGen4OutputFormat) {
        .sn = false,
        .scheduleLabel = true,
        .dateTime = false,
        .crc = true
    };
    /* The last CRC digit is altered (...C4EC becomes ...C4ED), so the
     * checksum won't match. */
    TestIOBuffers_init(
        buffers,
        "sch_asc_pts 9.83676000e+000 12.5361556e+000 0xC4ED"
        RESPONSE_TERMINATOR,
        0);
    memset(&buffers->streamSample, 0, sizeof(buffers->streamSample));

    err = RBRInstrumentGen4_readSample(instrument);
    /* The checksum failure makes the parser reject the line as a sample;
     * with no error/warning prefix, errorCheckResponse() then treats it as
     * an ordinary command response, so readSample() loops back for another
     * response and, finding no more data, fails with a callback error
     * rather than a checksum or timeout error. */
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_CALLBACK_ERROR,
                        err,
                        RBRInstrumentGen4Error);

    return true;
}

TEST_LOGGER4(readSampleWithoutCallback)
{
    RBRInstrumentGen4Error err;
    RBRInstrumentGen4SampleCallback savedCallback
        = instrument->callbacks.sample;

    instrument->callbacks.sample = NULL;

    instrument->outputFormat = (RBRInstrumentGen4OutputFormat) {
        .sn = false,
        .scheduleLabel = false,
        .dateTime = false,
        .crc = false
    };
    TestIOBuffers_init(buffers,
                       "9.85054000e+000 12.5359318e+000" RESPONSE_TERMINATOR,
                       0);

    err = RBRInstrumentGen4_readSample(instrument);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_MISSING_CALLBACK,
                        err,
                        RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    instrument->callbacks.sample = savedCallback;

    return true;
}
