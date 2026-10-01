/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file schedule.c
 *
 * \brief Tests for instrument schedule commands.
 */

/* Required for isnan, NAN. */
#include <math.h>
#include "tests.h"
#include "RBRGen3Schedule.h"

typedef struct ClockTest {
    const char *command;
    const char *response;
    RBRGen3Clock expected;
} ClockTest;

static bool test_clock(RBRGen3 *conn, TestIOBuffers *buffers, ClockTest *tests)
{
    RBRGen3Clock actual;

    for (int i = 0; tests[i].command != NULL; ++i) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        RBRGen3Error err = RBRGen3_getClock(conn, &actual);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_EQ(tests[i].expected.dateTime, actual.dateTime, "%" PRIi64);
        if (isnan(tests[i].expected.offsetFromUtc)) {
            TEST_ASSERT(isnan(actual.offsetFromUtc));
        } else {
            TEST_ASSERT_FLOAT_EQ(tests[i].expected.offsetFromUtc, actual.offsetFromUtc, 0.0f);
        }
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }

    return true;
}

TEST_LOGGER2(now)
{
    ClockTest tests[] = {
        {
            .command = "settings offsetfromutc" COMMAND_TERMINATOR "now" COMMAND_TERMINATOR,
            .response = "settings offsetfromutc = unknown" RESPONSE_TERMINATOR
                        "now = 20180920214914" RESPONSE_TERMINATOR,
            .expected =
                {
                    .dateTime = 1537480154000LL,
                    .offsetFromUtc = NAN,
                },
        },
        {
            .command = "settings offsetfromutc" COMMAND_TERMINATOR "now" COMMAND_TERMINATOR,
            .response = "settings offsetfromutc = unknown" RESPONSE_TERMINATOR
                        "now = 20000101000000" RESPONSE_TERMINATOR,
            .expected =
                {
                    .dateTime = RBRGEN3_DATETIME_MIN,
                    .offsetFromUtc = NAN,
                },
        },
        {
            .command = "settings offsetfromutc" COMMAND_TERMINATOR "now" COMMAND_TERMINATOR,
            .response = "settings offsetfromutc = unknown" RESPONSE_TERMINATOR
                        "now = 20991231235959" RESPONSE_TERMINATOR,
            .expected =
                {
                    .dateTime = RBRGEN3_DATETIME_MAX,
                    .offsetFromUtc = NAN,
                },
        },
        {
            .command = "settings offsetfromutc" COMMAND_TERMINATOR "now" COMMAND_TERMINATOR,
            .response = "settings offsetfromutc = +7.50" RESPONSE_TERMINATOR
                        "now = 20180920214914" RESPONSE_TERMINATOR,
            .expected =
                {
                    .dateTime = 1537480154000LL,
                    .offsetFromUtc = 7.5,
                },
        },
        {
            .command = "settings offsetfromutc" COMMAND_TERMINATOR "now" COMMAND_TERMINATOR,
            .response = "settings offsetfromutc = -4.00" RESPONSE_TERMINATOR
                        "now = 20180920214914" RESPONSE_TERMINATOR,
            .expected =
                {
                    .dateTime = 1537480154000LL,
                    .offsetFromUtc = -4,
                },
        },
        {0},
    };

    return test_clock(conn, buffers, tests);
}

TEST_LOGGER2(now_set)
{
    RBRGen3Clock now = {
        .dateTime = 1550264758524LL,
        .offsetFromUtc = 0,
    };

    const char *expectedCommand =
        "now = 20190215210558" COMMAND_TERMINATOR "permit = settings" COMMAND_TERMINATOR
        "settings offsetfromutc = 0.000000" COMMAND_TERMINATOR;

    const char *response =
        "now = 20190215210558" RESPONSE_TERMINATOR "permit = settings" RESPONSE_TERMINATOR
        "settings offsetfromutc = 0.000000" RESPONSE_TERMINATOR;

    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRGen3_setClock(conn, &now);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(clock)
{
    ClockTest tests[] = {
        {
            .command = "clock" COMMAND_TERMINATOR,
            .response = "clock datetime = 20180920214914, "
                        "offsetfromutc = unknown" RESPONSE_TERMINATOR,
            .expected =
                {
                    .dateTime = 1537480154000LL,
                    .offsetFromUtc = NAN,
                },
        },
        {
            .command = "clock" COMMAND_TERMINATOR,
            .response = "clock datetime = 20000101000000, "
                        "offsetfromutc = unknown" RESPONSE_TERMINATOR,
            .expected =
                {
                    .dateTime = RBRGEN3_DATETIME_MIN,
                    .offsetFromUtc = NAN,
                },
        },
        {
            .command = "clock" COMMAND_TERMINATOR,
            .response = "clock datetime = 20991231235959, "
                        "offsetfromutc = unknown" RESPONSE_TERMINATOR,
            .expected =
                {
                    .dateTime = RBRGEN3_DATETIME_MAX,
                    .offsetFromUtc = NAN,
                },
        },
        {
            .command = "clock" COMMAND_TERMINATOR,
            .response = "clock datetime = 20180920214914, "
                        "offsetfromutc = +7.50" RESPONSE_TERMINATOR,
            .expected =
                {
                    .dateTime = 1537480154000LL,
                    .offsetFromUtc = 7.5,
                },
        },
        {
            .command = "clock" COMMAND_TERMINATOR,
            .response = "clock datetime = 20180920214914, "
                        "offsetfromutc = -4.00" RESPONSE_TERMINATOR,
            .expected =
                {
                    .dateTime = 1537480154000LL,
                    .offsetFromUtc = -4,
                },
        },
        {0},
    };

    return test_clock(conn, buffers, tests);
}

TEST_LOGGER3(clock_set)
{
    RBRGen3Clock now = {
        .dateTime = 1550264758524LL,
        .offsetFromUtc = 0,
    };

    const char *text = "clock datetime = 20190215210558, "
                       "offsetfromutc = 0.000000";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRGen3_setClock(conn, &now);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

typedef struct SamplingTest {
    const char *command;
    const char *response;
    RBRGen3Sampling expected;
} SamplingTest;

static bool test_sampling(RBRGen3 *conn, TestIOBuffers *buffers, SamplingTest *tests)
{
    RBRGen3Sampling actual;

    for (int i = 0; tests[i].command != NULL; ++i) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        RBRGen3Error err = RBRGen3_getSampling(conn, &actual);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.mode, actual.mode, RBRGen3SamplingMode);
        TEST_ASSERT_EQ(tests[i].expected.period, actual.period, "%" PRIi32);

        for (int period = 0;
             period < RBRGEN3_AVAILABLE_FAST_PERIODS_MAX &&
             (period == 0 || tests[i].expected.availableFastPeriods[period - 1] != 0);
             ++period) {
            TEST_ASSERT_EQ(tests[i].expected.availableFastPeriods[period],
                           actual.availableFastPeriods[period],
                           "%" PRIi32);
        }

        TEST_ASSERT_EQ(tests[i].expected.userPeriodLimit, actual.userPeriodLimit, "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.burstLength, actual.burstLength, "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.burstInterval, actual.burstInterval, "%" PRIi32);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.gate, actual.gate, RBRGen3Gate);

        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }

    return true;
}

TEST_LOGGER2(sampling)
{
    SamplingTest tests[] = {
        {
            .command = "sampling" COMMAND_TERMINATOR,
            .response = "sampling schedule = 1, mode = continuous, period = 167, "
                        "burstlength = 10, burstinterval = 10000, gate = none, "
                        "userperiodlimit = 167" RESPONSE_TERMINATOR,
            .expected =
                {
                    .mode = RBRGEN3_SAMPLING_CONTINUOUS,
                    .period = 167,
                    .availableFastPeriods = {167, 250, 500, 0},
                    .userPeriodLimit = 167,
                    .burstLength = 10,
                    .burstInterval = 10000,
                    .gate = RBRGEN3_GATE_NONE,
                },
        },
        {
            .command = "sampling" COMMAND_TERMINATOR,
            .response = "sampling schedule = 1, mode = ddsampling, period = 83, "
                        "burstlength = 10, burstinterval = 10000, gate = thresholding, "
                        "userperiodlimit = 83" RESPONSE_TERMINATOR,
            .expected =
                {
                    .mode = RBRGEN3_SAMPLING_DDSAMPLING,
                    .period = 83,
                    .availableFastPeriods = {83, 125, 167, 250, 500, 0},
                    .userPeriodLimit = 83,
                    .burstLength = 10,
                    .burstInterval = 10000,
                    .gate = RBRGEN3_GATE_THRESHOLDING,
                },
        },
        {0},
    };

    return test_sampling(conn, buffers, tests);
}

TEST_LOGGER3(sampling)
{
    SamplingTest tests[] = {
        {
            .command = "sampling all" COMMAND_TERMINATOR,
            .response = "sampling mode = continuous, period = 1000, burstlength = 240, "
                        "burstinterval = 300000, gate = none, userperiodlimit = 32, "
                        "availablefastperiods = 500|250|125|63|32" RESPONSE_TERMINATOR,
            .expected =
                {
                    .mode = RBRGEN3_SAMPLING_CONTINUOUS,
                    .period = 1000,
                    .availableFastPeriods = {500, 250, 125, 63, 32, 0},
                    .userPeriodLimit = 32,
                    .burstLength = 240,
                    .burstInterval = 300000,
                    .gate = RBRGEN3_GATE_NONE,
                },
        },
        {
            .command = "sampling all" COMMAND_TERMINATOR,
            .response = "sampling mode = continuous, period = 1000, burstlength = 10, "
                        "burstinterval = 10000, gate = thresholding, "
                        "userperiodlimit = 1000, availablefastperiods = none" RESPONSE_TERMINATOR,
            .expected =
                {
                    .mode = RBRGEN3_SAMPLING_CONTINUOUS,
                    .period = 1000,
                    .availableFastPeriods = {0},
                    .userPeriodLimit = 1000,
                    .burstLength = 10,
                    .burstInterval = 10000,
                    .gate = RBRGEN3_GATE_THRESHOLDING,
                },
        },
        {0},
    };

    return test_sampling(conn, buffers, tests);
}

typedef struct SamplingSetTest {
    RBRGen3Sampling sampling;
    const char *response;
    const char *burstResponse;
    RBRGen3Error expectedError;
    RBRGen3Error expectedBurstError;
} SamplingSetTest;

TEST_LOGGER3(sampling_set)
{
    SamplingSetTest tests[] = {
        {
            .sampling =
                {
                    .mode = RBRGEN3_SAMPLING_CONTINUOUS,
                    .period = 1000,
                    .availableFastPeriods = {500, 250, 125, 63, 32, 0},
                    .userPeriodLimit = 32,
                    .burstLength = 240,
                    .burstInterval = 300000,
                    .gate = RBRGEN3_GATE_NONE,
                },
            .response = "sampling mode = continuous, period = 1000",
            .burstResponse = "sampling burstlength = 240, burstinterval = 300000",
            .expectedError = RBRGEN3_SUCCESS,
            .expectedBurstError = RBRGEN3_SUCCESS,
        },
        {
            .sampling =
                {
                    .mode = RBRGEN3_SAMPLING_CONTINUOUS,
                    .period = 100,
                    .availableFastPeriods = {500, 250, 125, 63, 0},
                    .userPeriodLimit = 63,
                    .burstLength = 10,
                    .burstInterval = 10000,
                    .gate = RBRGEN3_GATE_THRESHOLDING,
                },
            .response = "",
            .burstResponse = "",
            /* Failure because the period isn't in availableFastPeriods. */
            .expectedError = RBRGEN3_INVALID_PARAMETER_VALUE,
            .expectedBurstError = RBRGEN3_INVALID_PARAMETER_VALUE,
        },
        {
            .sampling =
                {
                    .mode = RBRGEN3_SAMPLING_CONTINUOUS,
                    .period = 63,
                    .availableFastPeriods = {0},
                    .userPeriodLimit = 125,
                    .burstLength = 10,
                    .burstInterval = 10000,
                    .gate = RBRGEN3_GATE_THRESHOLDING,
                },
            .response = "",
            .burstResponse = "",
            /* Failure because the period is less than userPeriodLimit. */
            .expectedError = RBRGEN3_INVALID_PARAMETER_VALUE,
            .expectedBurstError = RBRGEN3_INVALID_PARAMETER_VALUE,
        },
        {
            .sampling =
                {
                    .mode = RBRGEN3_SAMPLING_CONTINUOUS,
                    .period = 1000,
                    .availableFastPeriods = {500, 250, 125, 63, 32, 0},
                    .userPeriodLimit = 32,
                    .burstLength = 240,
                    .burstInterval = 1000 * 240,
                    .gate = RBRGEN3_GATE_NONE,
                },
            .response = "sampling mode = continuous, period = 1000",
            .burstResponse = "",
            .expectedError = RBRGEN3_SUCCESS,
            /* Failure because the burst interval is inconsistent. */
            .expectedBurstError = RBRGEN3_INVALID_PARAMETER_VALUE,
        },
        {.sampling = {0}},
    };

    RBRGen3Error err;

    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    for (int i = 0; tests[i].response != NULL; i++) {
        rbr_prepareCommandResponse(tests[i].response, expectedCommand, response);
        TestIOBuffers_init(buffers, response, 0);
        err = RBRGen3_setSampling(conn, &tests[i].sampling);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen3Error);
        TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

        rbr_prepareCommandResponse(tests[i].burstResponse, expectedCommand, response);
        TestIOBuffers_init(buffers, response, 0);
        err = RBRGen3_setBurstSampling(conn, &tests[i].sampling);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedBurstError, err, RBRGen3Error);
        TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);
    }

    return true;
}

typedef struct DeploymentTest {
    const char *command;
    const char *response;
    RBRGen3Deployment expected;
} DeploymentTest;

static bool test_deployment(RBRGen3 *conn, TestIOBuffers *buffers, DeploymentTest *tests)
{
    RBRGen3Deployment actual;

    for (int i = 0; tests[i].command != NULL; ++i) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        RBRGen3Error err = RBRGen3_getDeployment(conn, &actual);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_EQ(tests[i].expected.startTime, actual.startTime, "%" PRIi64);
        TEST_ASSERT_EQ(tests[i].expected.endTime, actual.endTime, "%" PRIi64);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.status, actual.status, RBRGen3DeploymentStatus);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }

    return true;
}

TEST_LOGGER2(deployment)
{
    DeploymentTest tests[] = {
        {
            .command = "starttime" COMMAND_TERMINATOR "endtime" COMMAND_TERMINATOR
                       "status" COMMAND_TERMINATOR,
            .response = "starttime = 20000101000000" RESPONSE_TERMINATOR
                        "endtime = 2099123123595959" RESPONSE_TERMINATOR
                        "status = disabled" RESPONSE_TERMINATOR,
            .expected =
                {
                    .startTime = RBRGEN3_DATETIME_MIN,
                    .endTime = RBRGEN3_DATETIME_MAX,
                    .status = RBRGEN3_STATUS_DISABLED,
                },
        },
        {0},
    };

    return test_deployment(conn, buffers, tests);
}

TEST_LOGGER3(deployment)
{
    DeploymentTest tests[] = {
        {
            .command = "deployment" COMMAND_TERMINATOR,
            .response = "deployment starttime = 20000101000000, "
                        "endtime = 2099123123595959, status = disabled" RESPONSE_TERMINATOR,
            .expected =
                {
                    .startTime = RBRGEN3_DATETIME_MIN,
                    .endTime = RBRGEN3_DATETIME_MAX,
                    .status = RBRGEN3_STATUS_DISABLED,
                },
        },
        {0},
    };

    return test_deployment(conn, buffers, tests);
}

typedef struct DeploymentSetTest {
    RBRGen3Deployment deployment;
    const char *response;
    RBRGen3Error expectedError;
} DeploymentSetTest;

TEST_LOGGER3(deployment_set)
{
    DeploymentSetTest tests[] = {
        {
            .deployment =
                {
                    .startTime = RBRGEN3_DATETIME_MIN,
                    .endTime = RBRGEN3_DATETIME_MAX,
                    .status = RBRGEN3_STATUS_DISABLED,
                },
            .response = "deployment starttime = 20000101000000, "
                        "endtime = 20991231235959",
            .expectedError = RBRGEN3_SUCCESS,
        },
        {
            .deployment =
                {
                    .startTime = 1537556712000LL,
                    .endTime = 1537556699000LL,
                    .status = RBRGEN3_STATUS_DISABLED,
                },
            .response = "",
            /* Failure because the end time is before the start time. */
            .expectedError = RBRGEN3_INVALID_PARAMETER_VALUE,
        },
        {
            .deployment =
                {
                    .startTime = 1537556699000LL,
                    .endTime = 1537556699000LL,
                    .status = RBRGEN3_STATUS_DISABLED,
                },
            .response = "",
            /* Failure because the end time equals the start time. */
            .expectedError = RBRGEN3_INVALID_PARAMETER_VALUE,
        },
        {
            .deployment =
                {
                    .startTime = 915148800000LL,
                    .endTime = 1537556699000LL,
                    .status = RBRGEN3_STATUS_DISABLED,
                },
            .response = "",
            /* Failure because the start time is before the epoch. */
            .expectedError = RBRGEN3_INVALID_PARAMETER_VALUE,
        },
        {
            .deployment =
                {
                    .startTime = 915148800000LL,
                    .endTime = 4102444800000LL,
                    .status = RBRGEN3_STATUS_DISABLED,
                },
            .response = "",
            /* Failure because the end time is after the limit. */
            .expectedError = RBRGEN3_INVALID_PARAMETER_VALUE,
        },
        {.deployment = {0}},
    };

    RBRGen3Error err;

    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    for (int i = 0; tests[i].response != NULL; i++) {
        rbr_prepareCommandResponse(tests[i].response, expectedCommand, response);
        TestIOBuffers_init(buffers, response, 0);
        err = RBRGen3_setDeployment(conn, &tests[i].deployment);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen3Error);
        TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);
    }

    return true;
}
