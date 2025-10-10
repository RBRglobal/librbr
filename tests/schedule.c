/**
 * \file memory.c
 *
 * \brief Tests for instrument schedule commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for isnan, NAN. */
#include <math.h>
#include "tests.h"

typedef struct ClockTest
{
    const char *command;
    const char *response;
    RBRInstrumentClock expected;
} ClockTest;

static bool test_clock(RBRInstrument *instrument,
                       TestIOBuffers *buffers,
                       ClockTest *tests)
{
    RBRInstrumentClock actual;

    for (int i = 0; tests[i].command != NULL; ++i)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        RBRInstrumentError err = RBRInstrument_getClock(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_EQ(tests[i].expected.dateTime,
                       actual.dateTime,
                       "%" PRIi64);
        if (isnan(tests[i].expected.offsetFromUtc))
        {
            TEST_ASSERT(isnan(actual.offsetFromUtc));
        }
        else
        {
            TEST_ASSERT_FLOAT_EQ(tests[i].expected.offsetFromUtc,
                                 actual.offsetFromUtc,
                                 0.0f);
        }
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }

    return true;
}

TEST_LOGGER2(now)
{
    ClockTest tests[] = {
        {
            "settings offsetfromutc" COMMAND_TERMINATOR
            "now" COMMAND_TERMINATOR,
            "settings offsetfromutc = unknown" RESPONSE_TERMINATOR
            "now = 20180920214914" RESPONSE_TERMINATOR,
            {
                .dateTime = 1537480154000LL,
                .offsetFromUtc = NAN
            }
        },
        {
            "settings offsetfromutc" COMMAND_TERMINATOR
            "now" COMMAND_TERMINATOR,
            "settings offsetfromutc = unknown" RESPONSE_TERMINATOR
            "now = 20000101000000" RESPONSE_TERMINATOR,
            {
                .dateTime = RBRINSTRUMENT_DATETIME_MIN,
                .offsetFromUtc = NAN
            }
        },
        {
            "settings offsetfromutc" COMMAND_TERMINATOR
            "now" COMMAND_TERMINATOR,
            "settings offsetfromutc = unknown" RESPONSE_TERMINATOR
            "now = 20991231235959" RESPONSE_TERMINATOR,
            {
                .dateTime = RBRINSTRUMENT_DATETIME_MAX,
                .offsetFromUtc = NAN
            }
        },
        {
            "settings offsetfromutc" COMMAND_TERMINATOR
            "now" COMMAND_TERMINATOR,
            "settings offsetfromutc = +7.50" RESPONSE_TERMINATOR
            "now = 20180920214914" RESPONSE_TERMINATOR,
            {
                .dateTime = 1537480154000LL,
                .offsetFromUtc = 7.5
            }
        },
        {
            "settings offsetfromutc" COMMAND_TERMINATOR
            "now" COMMAND_TERMINATOR,
            "settings offsetfromutc = -4.00" RESPONSE_TERMINATOR
            "now = 20180920214914" RESPONSE_TERMINATOR,
            {
                .dateTime = 1537480154000LL,
                .offsetFromUtc = -4
            }
        },
        {0}
    };

    return test_clock(instrument, buffers, tests);
}

TEST_LOGGER2(now_set)
{
    RBRInstrumentClock now = {
        .dateTime = 1550264758524LL,
        .offsetFromUtc = 0
    };

    const char *expectedCommand = "now = 20190215210558" COMMAND_TERMINATOR
                          "permit = settings" COMMAND_TERMINATOR
                          "settings offsetfromutc = 0.000000"
                          COMMAND_TERMINATOR;

    const char *response = "now = 20190215210558" RESPONSE_TERMINATOR
                          "permit = settings" RESPONSE_TERMINATOR
                          "settings offsetfromutc = 0.000000"
                          RESPONSE_TERMINATOR;

    TestIOBuffers_init(buffers, response, 0);
    RBRInstrumentError err = RBRInstrument_setClock(instrument, &now);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(clock)
{
    ClockTest tests[] = {
        {
            "clock" COMMAND_TERMINATOR,
            "clock datetime = 20180920214914, "
            "offsetfromutc = unknown" RESPONSE_TERMINATOR,
            {
                .dateTime = 1537480154000LL,
                .offsetFromUtc = NAN
            }
        },
        {
            "clock" COMMAND_TERMINATOR,
            "clock datetime = 20000101000000, "
            "offsetfromutc = unknown" RESPONSE_TERMINATOR,
            {
                .dateTime = RBRINSTRUMENT_DATETIME_MIN,
                .offsetFromUtc = NAN
            }
        },
        {
            "clock" COMMAND_TERMINATOR,
            "clock datetime = 20991231235959, "
            "offsetfromutc = unknown" RESPONSE_TERMINATOR,
            {
                .dateTime = RBRINSTRUMENT_DATETIME_MAX,
                .offsetFromUtc = NAN
            }
        },
        {
            "clock" COMMAND_TERMINATOR,
            "clock datetime = 20180920214914, "
            "offsetfromutc = +7.50" RESPONSE_TERMINATOR,
            {
                .dateTime = 1537480154000LL,
                .offsetFromUtc = 7.5
            }
        },
        {
            "clock" COMMAND_TERMINATOR,
            "clock datetime = 20180920214914, "
            "offsetfromutc = -4.00" RESPONSE_TERMINATOR,
            {
                .dateTime = 1537480154000LL,
                .offsetFromUtc = -4
            }
        },
        {0}
    };

    return test_clock(instrument, buffers, tests);
}

TEST_LOGGER3(clock_set)
{
    RBRInstrumentClock now = {
        .dateTime = 1550264758524LL,
        .offsetFromUtc = 0
    };

    const char *text = "clock datetime = 20190215210558, "
                          "offsetfromutc = 0.000000";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    RBRInstrumentError err = RBRInstrument_setClock(instrument, &now);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

typedef struct SamplingTest
{
    const char *command;
    const char *response;
    RBRInstrumentSampling expected;
} SamplingTest;

static bool test_sampling(RBRInstrument *instrument,
                          TestIOBuffers *buffers,
                          SamplingTest *tests)
{
    RBRInstrumentSampling actual;

    for (int i = 0; tests[i].command != NULL; ++i)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        RBRInstrumentError err = RBRInstrument_getSampling(instrument,
                                                           &actual);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.mode,
                            actual.mode,
                            RBRInstrumentSamplingMode);
        TEST_ASSERT_EQ(tests[i].expected.period, actual.period, "%" PRIi32);

        for (int period = 0;
             period < RBRINSTRUMENT_AVAILABLE_FAST_PERIODS_MAX
             && (period == 0
                 || tests[i].expected.availableFastPeriods[period - 1] != 0);
             ++period)
        {
            TEST_ASSERT_EQ(tests[i].expected.availableFastPeriods[period],
                           actual.availableFastPeriods[period],
                           "%" PRIi32);
        }

        TEST_ASSERT_EQ(tests[i].expected.userPeriodLimit,
                       actual.userPeriodLimit,
                       "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.burstLength,
                       actual.burstLength,
                       "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.burstInterval,
                       actual.burstInterval,
                       "%" PRIi32);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.gate,
                            actual.gate,
                            RBRInstrumentGate);

        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }

    return true;
}

TEST_LOGGER2(sampling)
{
    SamplingTest tests[] = {
        {
            "sampling" COMMAND_TERMINATOR,
            "sampling schedule = 1, mode = continuous, period = 167, "
            "burstlength = 10, burstinterval = 10000, gate = none, "
            "userperiodlimit = 167" RESPONSE_TERMINATOR,
            {
                .mode = RBRINSTRUMENT_SAMPLING_CONTINUOUS,
                .period = 167,
                .availableFastPeriods = {167, 250, 500, 0},
                .userPeriodLimit = 167,
                .burstLength = 10,
                .burstInterval = 10000,
                .gate = RBRINSTRUMENT_GATE_NONE
            }
        },
        {
            "sampling" COMMAND_TERMINATOR,
            "sampling schedule = 1, mode = ddsampling, period = 83, "
            "burstlength = 10, burstinterval = 10000, gate = thresholding, "
            "userperiodlimit = 83" RESPONSE_TERMINATOR,
            {
                .mode = RBRINSTRUMENT_SAMPLING_DDSAMPLING,
                .period = 83,
                .availableFastPeriods = {83, 125, 167, 250, 500, 0},
                .userPeriodLimit = 83,
                .burstLength = 10,
                .burstInterval = 10000,
                .gate = RBRINSTRUMENT_GATE_THRESHOLDING
            }
        },
        {0}
    };

    return test_sampling(instrument, buffers, tests);
}

TEST_LOGGER3(sampling)
{
    SamplingTest tests[] = {
        {
            "sampling all" COMMAND_TERMINATOR,
            "sampling mode = continuous, period = 1000, burstlength = 240, "
            "burstinterval = 300000, gate = none, userperiodlimit = 32, "
            "availablefastperiods = 500|250|125|63|32" RESPONSE_TERMINATOR,
            {
                .mode = RBRINSTRUMENT_SAMPLING_CONTINUOUS,
                .period = 1000,
                .availableFastPeriods = {500, 250, 125, 63, 32, 0},
                .userPeriodLimit = 32,
                .burstLength = 240,
                .burstInterval = 300000,
                .gate = RBRINSTRUMENT_GATE_NONE
            }
        },
        {
            "sampling all" COMMAND_TERMINATOR,
            "sampling mode = continuous, period = 1000, burstlength = 10, "
            "burstinterval = 10000, gate = thresholding, "
            "userperiodlimit = 1000, availablefastperiods = none"
            RESPONSE_TERMINATOR,
            {
                .mode = RBRINSTRUMENT_SAMPLING_CONTINUOUS,
                .period = 1000,
                .availableFastPeriods = {0},
                .userPeriodLimit = 1000,
                .burstLength = 10,
                .burstInterval = 10000,
                .gate = RBRINSTRUMENT_GATE_THRESHOLDING
            }
        },
        {0}
    };

    return test_sampling(instrument, buffers, tests);
}

typedef struct SamplingSetTest
{
    RBRInstrumentSampling sampling;
    const char *response;
    const char *burstResponse;
    RBRInstrumentError expectedError;
    RBRInstrumentError expectedBurstError;
} SamplingSetTest;

TEST_LOGGER3(sampling_set)
{
    SamplingSetTest tests[] = {
        {
            {
                .mode = RBRINSTRUMENT_SAMPLING_CONTINUOUS,
                .period = 1000,
                .availableFastPeriods = {500, 250, 125, 63, 32, 0},
                .userPeriodLimit = 32,
                .burstLength = 240,
                .burstInterval = 300000,
                .gate = RBRINSTRUMENT_GATE_NONE
            },
            "sampling mode = continuous, period = 1000",
            "sampling burstlength = 240, burstinterval = 300000",
            RBRINSTRUMENT_SUCCESS,
            RBRINSTRUMENT_SUCCESS
        },
        {
            {
                .mode = RBRINSTRUMENT_SAMPLING_CONTINUOUS,
                .period = 100,
                .availableFastPeriods = {500, 250, 125, 63, 0},
                .userPeriodLimit = 63,
                .burstLength = 10,
                .burstInterval = 10000,
                .gate = RBRINSTRUMENT_GATE_THRESHOLDING
            },
            "",
            "",
            /* Failure because the period isn't in availableFastPeriods. */
            RBRINSTRUMENT_INVALID_PARAMETER_VALUE,
            RBRINSTRUMENT_INVALID_PARAMETER_VALUE
        },
        {
            {
                .mode = RBRINSTRUMENT_SAMPLING_CONTINUOUS,
                .period = 63,
                .availableFastPeriods = {0},
                .userPeriodLimit = 125,
                .burstLength = 10,
                .burstInterval = 10000,
                .gate = RBRINSTRUMENT_GATE_THRESHOLDING
            },
            "",
            "",
            /* Failure because the period is less than userPeriodLimit. */
            RBRINSTRUMENT_INVALID_PARAMETER_VALUE,
            RBRINSTRUMENT_INVALID_PARAMETER_VALUE
        },
        {
            {
                .mode = RBRINSTRUMENT_SAMPLING_CONTINUOUS,
                .period = 1000,
                .availableFastPeriods = {500, 250, 125, 63, 32, 0},
                .userPeriodLimit = 32,
                .burstLength = 240,
                .burstInterval = 1000 * 240,
                .gate = RBRINSTRUMENT_GATE_NONE
            },
            "sampling mode = continuous, period = 1000",
            "",
            RBRINSTRUMENT_SUCCESS,
            /* Failure because the burst interval is inconsistent. */
            RBRINSTRUMENT_INVALID_PARAMETER_VALUE
        },
        {{0}, 0, 0, 0, 0}
    };

    RBRInstrumentError err;

    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    for (int i = 0; tests[i].response != NULL; i++)
    {
        rbr_prepareCommandResponse(tests[i].response, expectedCommand, response);
        TestIOBuffers_init(buffers, response, 0);
        err = RBRInstrument_setSampling(instrument, &tests[i].sampling);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentError);
        TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

        rbr_prepareCommandResponse(tests[i].burstResponse, expectedCommand, response);
        TestIOBuffers_init(buffers, response, 0);
        err = RBRInstrument_setBurstSampling(instrument, &tests[i].sampling);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedBurstError,
                            err,
                            RBRInstrumentError);
        TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);
    }

    return true;
}

typedef struct DeploymentTest
{
    const char *command;
    const char *response;
    RBRInstrumentDeployment expected;
} DeploymentTest;

static bool test_deployment(RBRInstrument *instrument,
                            TestIOBuffers *buffers,
                            DeploymentTest *tests)
{
    RBRInstrumentDeployment actual;

    for (int i = 0; tests[i].command != NULL; ++i)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        RBRInstrumentError err = RBRInstrument_getDeployment(instrument,
                                                             &actual);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_EQ(tests[i].expected.startTime,
                       actual.startTime,
                       "%" PRIi64);
        TEST_ASSERT_EQ(tests[i].expected.endTime,
                       actual.endTime,
                       "%" PRIi64);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.status,
                            actual.status,
                            RBRInstrumentDeploymentStatus);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }

    return true;
}

TEST_LOGGER2(deployment)
{
    DeploymentTest tests[] = {
        {
            "starttime" COMMAND_TERMINATOR
            "endtime" COMMAND_TERMINATOR
            "status" COMMAND_TERMINATOR,
            "starttime = 20000101000000" RESPONSE_TERMINATOR
            "endtime = 2099123123595959" RESPONSE_TERMINATOR
            "status = disabled" RESPONSE_TERMINATOR,
            {
                .startTime = RBRINSTRUMENT_DATETIME_MIN,
                .endTime = RBRINSTRUMENT_DATETIME_MAX,
                .status = RBRINSTRUMENT_STATUS_DISABLED
            }
        },
        {0}
    };

    return test_deployment(instrument, buffers, tests);
}

TEST_LOGGER3(deployment)
{
    DeploymentTest tests[] = {
        {
            "deployment" COMMAND_TERMINATOR,
            "deployment starttime = 20000101000000, "
            "endtime = 2099123123595959, status = disabled" RESPONSE_TERMINATOR,
            {
                .startTime = RBRINSTRUMENT_DATETIME_MIN,
                .endTime = RBRINSTRUMENT_DATETIME_MAX,
                .status = RBRINSTRUMENT_STATUS_DISABLED
            }
        },
        {0}
    };

    return test_deployment(instrument, buffers, tests);
}

typedef struct DeploymentSetTest
{
    RBRInstrumentDeployment deployment;
    const char *response;
    RBRInstrumentError expectedError;
} DeploymentSetTest;

TEST_LOGGER3(deployment_set)
{
    DeploymentSetTest tests[] = {
        {
            {
                .startTime = RBRINSTRUMENT_DATETIME_MIN,
                .endTime = RBRINSTRUMENT_DATETIME_MAX,
                .status = RBRINSTRUMENT_STATUS_DISABLED
            },
            "deployment starttime = 20000101000000, "
            "endtime = 20991231235959",
            RBRINSTRUMENT_SUCCESS
        },
        {
            {
                .startTime = 1537556712000LL,
                .endTime = 1537556699000LL,
                .status = RBRINSTRUMENT_STATUS_DISABLED
            },
            "",
            /* Failure because the end time is before the start time. */
            RBRINSTRUMENT_INVALID_PARAMETER_VALUE
        },
        {
            {
                .startTime = 1537556699000LL,
                .endTime = 1537556699000LL,
                .status = RBRINSTRUMENT_STATUS_DISABLED
            },
            "",
            /* Failure because the end time equals the start time. */
            RBRINSTRUMENT_INVALID_PARAMETER_VALUE
        },
        {
            {
                .startTime = 915148800000LL,
                .endTime = 1537556699000LL,
                .status = RBRINSTRUMENT_STATUS_DISABLED
            },
            "",
            /* Failure because the start time is before the epoch. */
            RBRINSTRUMENT_INVALID_PARAMETER_VALUE
        },
        {
            {
                .startTime = 915148800000LL,
                .endTime = 4102444800000LL,
                .status = RBRINSTRUMENT_STATUS_DISABLED
            },
            "",
            /* Failure because the end time is after the limit. */
            RBRINSTRUMENT_INVALID_PARAMETER_VALUE
        },
        {{0}, 0, 0}
    };

    RBRInstrumentError err;

    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    for (int i = 0; tests[i].response != NULL; i++)
    {
        rbr_prepareCommandResponse(tests[i].response, expectedCommand, response);
        TestIOBuffers_init(buffers, response, 0);
        err = RBRInstrument_setDeployment(instrument, &tests[i].deployment);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentError);  
        TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);
    }

    return true;
}
