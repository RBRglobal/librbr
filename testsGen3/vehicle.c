/**
 * \file vehicle.c
 *
 * \brief Tests for instrument vehicle commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include "tests.h"

typedef struct RegimesTest
{
    const char *response;
    RBRInstrumentGen3Regimes expected;
} RegimesTest;

TEST_LOGGER3(regimes)
{
    RegimesTest tests[] = {
        {
            "regimes direction = ascending, count = 1, reference = absolute"
            RESPONSE_TERMINATOR,
            {
                .direction = RBRINSTRUMENTGEN3_DIRECTION_ASCENDING,
                .count = 1,
                .reference = RBRINSTRUMENTGEN3_REFERENCE_ABSOLUTE
            }
        },
        {
            "regimes direction = descending, count = 3, "
            "reference = seapressure" RESPONSE_TERMINATOR,
            {
                .direction = RBRINSTRUMENTGEN3_DIRECTION_DESCENDING,
                .count = 3,
                .reference = RBRINSTRUMENTGEN3_REFERENCE_SEAPRESSURE
            }
        },
        {0}
    };

    RBRInstrumentGen3Error err;
    RBRInstrumentGen3Regimes actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen3_getRegimes(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN3_SUCCESS, err, RBRInstrumentGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.direction,
                            actual.direction,
                            RBRInstrumentGen3Direction);
        TEST_ASSERT_EQ(tests[i].expected.count,
                       actual.count,
                       "%" PRIi32);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.reference,
                            actual.reference,
                            RBRInstrumentGen3RegimesReference);
    }

    return true;
}

TEST_LOGGER3(regimes_set)
{
    RegimesTest tests[] = {
        {
            "regimes direction = ascending, count = 1, reference = absolute",
            {
                .direction = RBRINSTRUMENTGEN3_DIRECTION_ASCENDING,
                .count = 1,
                .reference = RBRINSTRUMENTGEN3_REFERENCE_ABSOLUTE
            }
        },
        {
            "regimes direction = descending, count = 3, "
            "reference = seapressure",
            {
                .direction = RBRINSTRUMENTGEN3_DIRECTION_DESCENDING,
                .count = 3,
                .reference = RBRINSTRUMENTGEN3_REFERENCE_SEAPRESSURE
            }
        },
        {0}
    };

    RBRInstrumentGen3Error err;

    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    for (int i = 0; tests[i].response != NULL; i++)
    {
        rbr_prepareCommandResponse(tests[i].response, expectedCommand, response);
        TestIOBuffers_init(buffers, response, 0);
        err = RBRInstrumentGen3_setRegimes(instrument, &tests[i].expected);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN3_SUCCESS, err, RBRInstrumentGen3Error);
        TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);
    }

    return true;
}

typedef struct RegimeTest
{
    const char *response;
    RBRInstrumentGen3Regime expected;
} RegimeTest;

TEST_LOGGER3(regime)
{
    RegimeTest tests[] = {
        {
            "regime 1 boundary = 50, binsize = 0.1, samplingperiod = 63"
            RESPONSE_TERMINATOR,
            {
                .index = 1,
                .boundary = 50.0,
                .binSize = 0.1,
                .samplingPeriod = 63
            }
        },
        {
            "regime 2 boundary = 100, binsize = 1.0, samplingperiod = 125"
            RESPONSE_TERMINATOR,
            {
                .index = 2,
                .boundary = 100.0,
                .binSize = 1.0,
                .samplingPeriod = 125
            }
        },
        {0}
    };

    RBRInstrumentGen3Error err;
    RBRInstrumentGen3Regime actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        actual.index = tests[i].expected.index;
        err = RBRInstrumentGen3_getRegime(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN3_SUCCESS, err, RBRInstrumentGen3Error);
        TEST_ASSERT_EQ(tests[i].expected.index, actual.index, "%" PRIi8);
        TEST_ASSERT_FLOAT_EQ(tests[i].expected.boundary,
                             actual.boundary,
                             0.0f);
        TEST_ASSERT_FLOAT_EQ(tests[i].expected.binSize,
                             actual.binSize,
                             0.0f);
        TEST_ASSERT_EQ(tests[i].expected.samplingPeriod,
                       actual.samplingPeriod,
                       "%" PRIi32);
    }

    return true;
}

TEST_LOGGER3(regime_set)
{
    RegimeTest tests[] = {
        {
            "regime 1 boundary = 50, binsize = 0.1, samplingperiod = 63",
            {
                .index = 1,
                .boundary = 50.0,
                .binSize = 0.1,
                .samplingPeriod = 63
            }
        },
        {
            "regime 2 boundary = 100, binsize = 1.4, samplingperiod = 125",
            {
                .index = 2,
                .boundary = 100.123,
                .binSize = 1.38,
                .samplingPeriod = 125
            }
        },
        {0}
    };

    RBRInstrumentGen3Error err;
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    for (int i = 0; tests[i].response != NULL; i++)
    {
        rbr_prepareCommandResponse(tests[i].response, expectedCommand, response);
        TestIOBuffers_init(buffers, response, 0);
        err = RBRInstrumentGen3_setRegime(instrument, &tests[i].expected);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN3_SUCCESS, err, RBRInstrumentGen3Error);
        TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);
    }

    return true;
}

typedef struct DirectionDependentSamplingTest
{
    const char *response;
    RBRInstrumentGen3DirectionDependentSampling expected;
} DirectionDependentSamplingTest;

TEST_LOGGER3(ddsampling)
{
    DirectionDependentSamplingTest tests[] = {
        {
            "ddsampling direction = ascending, fastperiod = 63, "
            "slowperiod = 1000, fastthreshold = 3.0, slowthreshold = 3.0"
            RESPONSE_TERMINATOR,
            {
                .direction = RBRINSTRUMENTGEN3_DIRECTION_ASCENDING,
                .fastPeriod = 63,
                .slowPeriod = 1000,
                .fastThreshold = 3.0,
                .slowThreshold = 3.0
            }
        },
        {0}
    };

    RBRInstrumentGen3Error err;
    RBRInstrumentGen3DirectionDependentSampling actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen3_getDirectionDependentSampling(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN3_SUCCESS, err, RBRInstrumentGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.direction,
                            actual.direction,
                            RBRInstrumentGen3Direction);
        TEST_ASSERT_EQ(tests[i].expected.fastPeriod,
                       actual.fastPeriod,
                       "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.slowPeriod,
                       actual.slowPeriod,
                       "%" PRIi32);
        TEST_ASSERT_FLOAT_EQ(tests[i].expected.fastThreshold,
                             actual.fastThreshold,
                             0.0f);
        TEST_ASSERT_FLOAT_EQ(tests[i].expected.slowThreshold,
                             actual.slowThreshold,
                             0.0f);
    }

    return true;
}

TEST_LOGGER3(ddsampling_set)
{
    DirectionDependentSamplingTest tests[] = {
        {
            "ddsampling direction = ascending, fastperiod = 63, "
            "slowperiod = 1000, fastthreshold = 3.0, slowthreshold = 3.0",
            {
                .direction = RBRINSTRUMENTGEN3_DIRECTION_ASCENDING,
                .fastPeriod = 63,
                .slowPeriod = 1000,
                .fastThreshold = 3.0,
                .slowThreshold = 3.0
            }
        },
        {0}
    };

    RBRInstrumentGen3Error err;

    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    for (int i = 0; tests[i].response != NULL; i++)
    {
        rbr_prepareCommandResponse(tests[i].response, expectedCommand, response);
        TestIOBuffers_init(buffers, response, 0);
        err = RBRInstrumentGen3_setDirectionDependentSampling(instrument,
                                                          &tests[i].expected);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN3_SUCCESS, err, RBRInstrumentGen3Error);
        TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);
    }

    return true;
}
