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

typedef struct RegimesTest {
    const char *response;
    RBRInstrumentRegimes expected;
} RegimesTest;

TEST_LOGGER3(regimes)
{
    RegimesTest tests[] = {
        {"regimes direction = ascending, count = 1, reference = absolute" RESPONSE_TERMINATOR,
         {.direction = RBRINSTRUMENT_DIRECTION_ASCENDING,
          .count = 1,
          .reference = RBRINSTRUMENT_REFERENCE_ABSOLUTE}},
        {"regimes direction = descending, count = 3, "
         "reference = seapressure" RESPONSE_TERMINATOR,
         {.direction = RBRINSTRUMENT_DIRECTION_DESCENDING,
          .count = 3,
          .reference = RBRINSTRUMENT_REFERENCE_SEAPRESSURE}},
        {0}};

    RBRInstrumentError err;
    RBRInstrumentRegimes actual;

    for (int i = 0; tests[i].response != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrument_getRegimes(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.direction, actual.direction, RBRInstrumentDirection);
        TEST_ASSERT_EQ(tests[i].expected.count, actual.count, "%" PRIi32);
        TEST_ASSERT_ENUM_EQ(
            tests[i].expected.reference, actual.reference, RBRInstrumentRegimesReference);
    }

    return true;
}

TEST_LOGGER3(regimes_set)
{
    RegimesTest tests[] = {{"regimes direction = ascending, count = 1, reference = absolute",
                            {.direction = RBRINSTRUMENT_DIRECTION_ASCENDING,
                             .count = 1,
                             .reference = RBRINSTRUMENT_REFERENCE_ABSOLUTE}},
                           {"regimes direction = descending, count = 3, "
                            "reference = seapressure",
                            {.direction = RBRINSTRUMENT_DIRECTION_DESCENDING,
                             .count = 3,
                             .reference = RBRINSTRUMENT_REFERENCE_SEAPRESSURE}},
                           {0}};

    RBRInstrumentError err;

    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    for (int i = 0; tests[i].response != NULL; i++) {
        rbr_prepareCommandResponse(tests[i].response, expectedCommand, response);
        TestIOBuffers_init(buffers, response, 0);
        err = RBRInstrument_setRegimes(instrument, &tests[i].expected);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);
    }

    return true;
}

typedef struct RegimeTest {
    const char *response;
    RBRInstrumentRegime expected;
} RegimeTest;

TEST_LOGGER3(regime)
{
    RegimeTest tests[] = {
        {"regime 1 boundary = 50, binsize = 0.1, samplingperiod = 63" RESPONSE_TERMINATOR,
         {.index = 1, .boundary = 50.0, .binSize = 0.1, .samplingPeriod = 63}},
        {"regime 2 boundary = 100, binsize = 1.0, samplingperiod = 125" RESPONSE_TERMINATOR,
         {.index = 2, .boundary = 100.0, .binSize = 1.0, .samplingPeriod = 125}},
        {0}};

    RBRInstrumentError err;
    RBRInstrumentRegime actual;

    for (int i = 0; tests[i].response != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        actual.index = tests[i].expected.index;
        err = RBRInstrument_getRegime(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_EQ(tests[i].expected.index, actual.index, "%" PRIi8);
        TEST_ASSERT_EQ(tests[i].expected.boundary, actual.boundary, "%f");
        TEST_ASSERT_EQ(tests[i].expected.binSize, actual.binSize, "%f");
        TEST_ASSERT_EQ(tests[i].expected.samplingPeriod, actual.samplingPeriod, "%" PRIi32);
    }

    return true;
}

TEST_LOGGER3(regime_set)
{
    RegimeTest tests[] = {
        {"regime 1 boundary = 50, binsize = 0.1, samplingperiod = 63",
         {.index = 1, .boundary = 50.0, .binSize = 0.1, .samplingPeriod = 63}},
        {"regime 2 boundary = 100, binsize = 1.4, samplingperiod = 125",
         {.index = 2, .boundary = 100.123, .binSize = 1.38, .samplingPeriod = 125}},
        {0}};

    RBRInstrumentError err;
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    for (int i = 0; tests[i].response != NULL; i++) {
        rbr_prepareCommandResponse(tests[i].response, expectedCommand, response);
        TestIOBuffers_init(buffers, response, 0);
        err = RBRInstrument_setRegime(instrument, &tests[i].expected);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);
    }

    return true;
}

typedef struct DirectionDependentSamplingTest {
    const char *response;
    RBRInstrumentDirectionDependentSampling expected;
} DirectionDependentSamplingTest;

TEST_LOGGER3(ddsampling)
{
    DirectionDependentSamplingTest tests[] = {
        {"ddsampling direction = ascending, fastperiod = 63, "
         "slowperiod = 1000, fastthreshold = 3.0, slowthreshold = 3.0" RESPONSE_TERMINATOR,
         {.direction = RBRINSTRUMENT_DIRECTION_ASCENDING,
          .fastPeriod = 63,
          .slowPeriod = 1000,
          .fastThreshold = 3.0,
          .slowThreshold = 3.0}},
        {0}};

    RBRInstrumentError err;
    RBRInstrumentDirectionDependentSampling actual;

    for (int i = 0; tests[i].response != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrument_getDirectionDependentSampling(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.direction, actual.direction, RBRInstrumentDirection);
        TEST_ASSERT_EQ(tests[i].expected.fastPeriod, actual.fastPeriod, "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.slowPeriod, actual.slowPeriod, "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.fastThreshold, actual.fastThreshold, "%f");
        TEST_ASSERT_EQ(tests[i].expected.slowThreshold, actual.slowThreshold, "%f");
    }

    return true;
}

TEST_LOGGER3(ddsampling_set)
{
    DirectionDependentSamplingTest tests[] = {
        {"ddsampling direction = ascending, fastperiod = 63, "
         "slowperiod = 1000, fastthreshold = 3.0, slowthreshold = 3.0",
         {.direction = RBRINSTRUMENT_DIRECTION_ASCENDING,
          .fastPeriod = 63,
          .slowPeriod = 1000,
          .fastThreshold = 3.0,
          .slowThreshold = 3.0}},
        {0}};

    RBRInstrumentError err;

    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    for (int i = 0; tests[i].response != NULL; i++) {
        rbr_prepareCommandResponse(tests[i].response, expectedCommand, response);
        TestIOBuffers_init(buffers, response, 0);
        err = RBRInstrument_setDirectionDependentSampling(instrument, &tests[i].expected);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);
    }

    return true;
}
