/**
 * \file streaming.c
 *
 * \brief Tests for instrument streaming commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include "tests.h"


typedef struct OutputformatTest
{
    const char *response;
    RBRInstrumentGen4Outputformat expected;
}OutputformatTest;

static bool test_outputformat(RBRInstrumentGen4 *instrument,
                            TestIOBuffers *buffers,
                            OutputformatTest *tests)
{
    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Outputformat actual;

    for(int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_getOutputformat(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
        TEST_ASSERT_EQ(tests[i].expected, instrument->outputFormat, "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected, actual, "%" PRIi32);
    }
    return true;
}

TEST_LOGGER4(outputformat)
{
    OutputformatTest tests[] = {
        {
            "instrument outputformat sn=on schedulelabel=on datetime=off crc=on "
            "encoding=ascii datatype=float32" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_SERIAL
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_SCHEDULELABEL
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_CRC
        },
        {
            "instrument outputformat sn=off schedulelabel=off datetime=off crc=off "
            "encoding=ascii datatype=float32" RESPONSE_TERMINATOR,
            0

        },
        {
            "instrument outputformat sn=on schedulelabel=off datetime=off crc=on "
            "encoding=ascii datatype=float32" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_SERIAL
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_CRC
        },
        {0}
    };

    return test_outputformat(instrument, buffers, tests);
}

typedef struct SetOutputformatTest
{
    const char *command;
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4Outputformat outputformat;
}SetOutputformatTest;

static bool test_setoutputformat(RBRInstrumentGen4 *instrument,
                            TestIOBuffers *buffers,
                            SetOutputformatTest *tests)
{
    RBRInstrumentGen4Error err;

    for(int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        RBRInstrumentGen4Outputformat priorformat = instrument->outputFormat;
        err = RBRInstrumentGen4_setOutputformat(instrument, tests[i].outputformat);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentGen4Error);
        if (err == RBRINSTRUMENTGEN4_SUCCESS)
        {
            TEST_ASSERT_EQ(tests[i].outputformat, instrument->outputFormat, "%" PRIi32);
        }
        else
        {
            TEST_ASSERT_EQ(priorformat, instrument->outputFormat, "%" PRIi32);
        }
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }
    return true;
}

TEST_LOGGER4(setOutputformat)
{
    SetOutputformatTest tests[] = {
        {
            "instrument outputformat sn=on schedulelabel=on datetime=on crc=on" COMMAND_TERMINATOR,

            "instrument outputformat sn=on schedulelabel=on datetime=on crc=on "
            "encoding=ascii datatype=float32" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_SUCCESS,
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_SERIAL
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_SCHEDULELABEL
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_CRC
        },
        {
            "instrument outputformat sn=off schedulelabel=off datetime=off crc=off" COMMAND_TERMINATOR,

            "instrument outputformat sn=off schedulelabel=off datetime=off crc=off "
            "encoding=ascii datatype=float32" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_SUCCESS,
            0
        },
        {
            "instrument outputformat sn=off schedulelabel=on datetime=off crc=off" COMMAND_TERMINATOR,

            "instrument outputformat sn=off schedulelabel=on datetime=off crc=off "
            "encoding=ascii datatype=float32" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_SUCCESS,
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_SCHEDULELABEL
        },
        {0}
    };

    return test_setoutputformat(instrument, buffers, tests);
}


typedef struct ReadSampleTest
{
    const char *response;
    RBRInstrumentGen4Outputformat outputFormat;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4Sample expected;
}ReadSampleTest;

static bool test_readSample(RBRInstrumentGen4 *instrument,
                          TestIOBuffers *buffers,
                          ReadSampleTest *tests)
{
    RBRInstrumentGen4Error err;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        instrument->outputFormat=tests[i].outputFormat;
        err = RBRInstrumentGen4_readSample(instrument);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
        TEST_ASSERT_EQ(tests[i].expected.timestamp,
                       buffers->streamSample.timestamp,
                       "%" PRIi64);
        TEST_ASSERT_EQ(tests[i].expected.channels,
                       buffers->streamSample.channels,
                       "%" PRIi32);
        for (int32_t channel = 0; channel < buffers->streamSample.channels; ++channel)
        {
            TEST_ASSERT_ENUM_EQ(
                RBRInstrumentGen4Reading_getFlag(
                    tests[i].expected.readings[channel]),
                RBRInstrumentGen4Reading_getFlag(buffers->streamSample.readings[channel]),
                RBRInstrumentGen4ReadingFlag);

            switch (RBRInstrumentGen4Reading_getFlag(buffers->streamSample.readings[channel]))
            {
            case RBRINSTRUMENTGEN4_READING_FLAG_UNCALIBRATED:
            case RBRINSTRUMENTGEN4_READING_FLAG_ERROR:
                TEST_ASSERT_EQ(
                    RBRInstrumentGen4Reading_getError(
                        tests[i].expected.readings[channel]),
                    RBRInstrumentGen4Reading_getError(buffers->streamSample.readings[channel]),
                    "%" PRIi8);
                break;
            case RBRINSTRUMENTGEN4_READING_FLAG_NONE:
            default:
                TEST_ASSERT_EQ(tests[i].expected.readings[channel],
                               buffers->streamSample.readings[channel],
                               "%lf");
            }
        }
    }

    return true;
}


TEST_LOGGER4(stream_sample_parse)
{
    ReadSampleTest tests[]={
        {
            "2018-07-26 14:56:24.000 10.1325" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP,
            RBRINSTRUMENTGEN4_SUCCESS,
            {
                .timestamp = 1532616984000LL,
                .channels = 1,
                .readings = {
                    10.1325
                }
            }
        },
        {
            "2023-09-10 11:24:14.125 38.6671142e+000 22.0217124e+000 1.95962418e+003" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP,
            RBRINSTRUMENTGEN4_SUCCESS,
            {
                .timestamp = 1694345054125LL,
                .channels = 3,
                .readings = {
                    38.6671142,
                    22.0217124,
                    1959.62418
                }
            }
        },
        { 
            "RBR 142152 sch_fast_CTD 2018-09-10 11:24:14.125 35.6671142e+000 27.0217124e+000 5.95962418e+002" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_SERIAL
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_SCHEDULELABEL
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP,
            RBRINSTRUMENTGEN4_SUCCESS,
            {
                .timestamp = 1536578654125LL,
                .channels = 3,
                .readings = {
                    35.6671142,
                    27.0217124,
                    595.962418
                }
            }
        },
        {
            "sch_fast_CTD 2023-09-01 15:25:10.000 50.3724311e+000 15.2014375e+000 3.50074385e+002 0xD607" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_SCHEDULELABEL
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP
             | RBRINSTRUMENTGEN4_OUTPUTFORMAT_CRC,
            RBRINSTRUMENTGEN4_SUCCESS,
            {
                .timestamp = 1693581910000LL,
                .channels = 3,
                .readings = {
                    50.3724311,
                    15.2014375,
                    350.074385,
                }
            }
        },
        {
            "2000-01-01 20:09:36.000 -129.805680e+000 Error-09 Error-14" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP,
            RBRINSTRUMENTGEN4_SUCCESS,
            {
                .timestamp = 946757376000LL,
                .channels = 3,
                .readings = {
                    -129.80568,
                    RBRInstrumentGen4Reading_setError(
                        RBRINSTRUMENTGEN4_READING_FLAG_ERROR,
                        9),
                    RBRInstrumentGen4Reading_setError(
                        RBRINSTRUMENTGEN4_READING_FLAG_ERROR,
                        14)
                }
            }
        },
        {0}
    };

   return test_readSample(instrument, buffers, tests);
}


