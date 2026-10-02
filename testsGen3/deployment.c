/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file deployment.c
 *
 * \brief Tests for instrument deployment commands.
 */

#include "tests.h"
#include "RBRGen3Deployment.h"
#include "RBRGen3Schedule.h"

typedef struct StatusTest {
    const char *response;
    RBRGen3Error expectedError;
    RBRGen3Response expectedResponse;
    RBRGen3DeploymentStatus expected;
} StatusTest;

static bool test_verify(RBRGen3 *conn, TestIOBuffers *buffers, StatusTest *tests)
{
    RBRGen3Error err;
    RBRGen3DeploymentStatus actual;

    for (int i = 0; tests[i].response != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_verify(conn, false, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(
            tests[i].expectedResponse.type, conn->response.type, RBRGen3ResponseType);
        TEST_ASSERT_EQ(tests[i].expectedResponse.error, conn->response.error, "%" PRIi32);
        TEST_ASSERT_ENUM_EQ(tests[i].expected, actual, RBRGen3DeploymentStatus);
    }

    return true;
}

TEST_LOGGER2(verify)
{
    StatusTest tests[] = {
        {
            .response = "verify = pending" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_INFO,
                    .error = RBRGEN3_HARDWARE_ERROR_NONE,
                },
            .expected = RBRGEN3_STATUS_PENDING,
        },
        {
            .response = "verify = logging" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_INFO,
                    .error = RBRGEN3_HARDWARE_ERROR_NONE,
                },
            .expected = RBRGEN3_STATUS_LOGGING,
        },
        {
            .response = "E0402 memory not empty, erase first, verify = stopped" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_HARDWARE_ERROR,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_ERROR,
                    .error = RBRGEN3_HARDWARE_ERROR_MEMORY_NOT_EMPTY_ERASE_FIRST,
                },
            .expected = RBRGEN3_UNKNOWN_STATUS,
        },
        {
            .response = "E0401 estimated memory usage exceeds capacity, verify = "
                        "logging" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_WARNING,
                    .error = RBRGEN3_HARDWARE_ERROR_ESTIMATED_MEMORY_USAGE_EXCEEDS_CAPACITY,
                },
            .expected = RBRGEN3_STATUS_LOGGING,
        },
        {0},
    };

    return test_verify(conn, buffers, tests);
}

TEST_LOGGER3(verify)
{
    StatusTest tests[] = {
        {
            .response = "verify status = pending, warning = none" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_INFO,
                    .error = RBRGEN3_HARDWARE_ERROR_NONE,
                },
            .expected = RBRGEN3_STATUS_PENDING,
        },
        {
            .response = "verify status = logging, warning = none" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_INFO,
                    .error = RBRGEN3_HARDWARE_ERROR_NONE,
                },
            .expected = RBRGEN3_STATUS_LOGGING,
        },
        {
            .response = "E0402 memory not empty, erase first" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_HARDWARE_ERROR,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_ERROR,
                    .error = RBRGEN3_HARDWARE_ERROR_MEMORY_NOT_EMPTY_ERASE_FIRST,
                },
            .expected = RBRGEN3_UNKNOWN_STATUS,
        },
        {
            .response = "verify status = logging, warning = W0401" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_WARNING,
                    .error = RBRGEN3_HARDWARE_ERROR_ESTIMATED_MEMORY_USAGE_EXCEEDS_CAPACITY,
                },
            .expected = RBRGEN3_STATUS_LOGGING,
        },
        {0},
    };

    return test_verify(conn, buffers, tests);
}

static bool test_enable(RBRGen3 *conn, TestIOBuffers *buffers, StatusTest *tests)
{
    RBRGen3Error err;
    RBRGen3DeploymentStatus actual;

    for (int i = 0; tests[i].response != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_enable(conn, false, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(
            tests[i].expectedResponse.type, conn->response.type, RBRGen3ResponseType);
        TEST_ASSERT_EQ(tests[i].expectedResponse.error, conn->response.error, "%" PRIi32);
        TEST_ASSERT_ENUM_EQ(tests[i].expected, actual, RBRGen3DeploymentStatus);
    }

    return true;
}

TEST_LOGGER2(enable)
{
    StatusTest tests[] = {
        {
            .response = "enable = pending" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_INFO,
                    .error = RBRGEN3_HARDWARE_ERROR_NONE,
                },
            .expected = RBRGEN3_STATUS_PENDING,
        },
        {
            .response = "enable = logging" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_INFO,
                    .error = RBRGEN3_HARDWARE_ERROR_NONE,
                },
            .expected = RBRGEN3_STATUS_LOGGING,
        },
        {
            .response = "E0402 memory not empty, erase first" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_HARDWARE_ERROR,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_ERROR,
                    .error = RBRGEN3_HARDWARE_ERROR_MEMORY_NOT_EMPTY_ERASE_FIRST,
                },
            .expected = RBRGEN3_UNKNOWN_STATUS,
        },
        {
            .response = "E0401 estimated memory usage exceeds capacity, enable = "
                        "logging" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_WARNING,
                    .error = RBRGEN3_HARDWARE_ERROR_ESTIMATED_MEMORY_USAGE_EXCEEDS_CAPACITY,
                },
            .expected = RBRGEN3_STATUS_LOGGING,
        },
        {0},
    };

    return test_enable(conn, buffers, tests);
}

TEST_LOGGER3(enable)
{
    StatusTest tests[] = {
        {
            .response = "enable status = pending, warning = none" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_INFO,
                    .error = RBRGEN3_HARDWARE_ERROR_NONE,
                },
            .expected = RBRGEN3_STATUS_PENDING,
        },
        {
            .response = "enable status = logging, warning = none" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_INFO,
                    .error = RBRGEN3_HARDWARE_ERROR_NONE,
                },
            .expected = RBRGEN3_STATUS_LOGGING,
        },
        {
            .response = "E0402 memory not empty, erase first" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_HARDWARE_ERROR,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_ERROR,
                    .error = RBRGEN3_HARDWARE_ERROR_MEMORY_NOT_EMPTY_ERASE_FIRST,
                },
            .expected = RBRGEN3_UNKNOWN_STATUS,
        },
        {
            .response = "enable status = logging, warning = W0401" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_WARNING,
                    .error = RBRGEN3_HARDWARE_ERROR_ESTIMATED_MEMORY_USAGE_EXCEEDS_CAPACITY,
                },
            .expected = RBRGEN3_STATUS_LOGGING,
        },
        {0},
    };

    return test_enable(conn, buffers, tests);
}

static bool test_disable(RBRGen3 *conn, TestIOBuffers *buffers, StatusTest *tests)
{
    RBRGen3Error err;
    RBRGen3DeploymentStatus actual;

    for (int i = 0; tests[i].response != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_disable(conn, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(
            tests[i].expectedResponse.type, conn->response.type, RBRGen3ResponseType);
        TEST_ASSERT_EQ(tests[i].expectedResponse.error, conn->response.error, "%" PRIi32);
        TEST_ASSERT_ENUM_EQ(tests[i].expected, actual, RBRGen3DeploymentStatus);
    }

    return true;
}

TEST_LOGGER2(stop)
{
    StatusTest tests[] = {
        {
            .response = "stop = stopped" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_INFO,
                    .error = RBRGEN3_HARDWARE_ERROR_NONE,
                },
            .expected = RBRGEN3_STATUS_STOPPED,
        },
        {
            .response = "E0406 not logging, stop = stopped" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_WARNING,
                    .error = RBRGEN3_HARDWARE_ERROR_NOT_LOGGING,
                },
            .expected = RBRGEN3_STATUS_STOPPED,
        },
        {
            .response = "E0406 not logging, stop = fullandstopped" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_WARNING,
                    .error = RBRGEN3_HARDWARE_ERROR_NOT_LOGGING,
                },
            .expected = RBRGEN3_STATUS_FULLANDSTOPPED,
        },
        {
            .response = "E0406 not logging, stop = disabled" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_WARNING,
                    .error = RBRGEN3_HARDWARE_ERROR_NOT_LOGGING,
                },
            .expected = RBRGEN3_STATUS_DISABLED,
        },
        {0},
    };

    return test_disable(conn, buffers, tests);
}

TEST_LOGGER3(disable)
{
    StatusTest tests[] = {
        {
            .response = "disable status = stopped" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_INFO,
                    .error = RBRGEN3_HARDWARE_ERROR_NONE,
                },
            .expected = RBRGEN3_STATUS_STOPPED,
        },
        {
            .response = "disable status = fullandstopped" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_INFO,
                    .error = RBRGEN3_HARDWARE_ERROR_NONE,
                },
            .expected = RBRGEN3_STATUS_FULLANDSTOPPED,
        },
        {
            .response = "disable status = disabled" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
            .expectedResponse =
                {
                    .type = RBRGEN3_RESPONSE_INFO,
                    .error = RBRGEN3_HARDWARE_ERROR_NONE,
                },
            .expected = RBRGEN3_STATUS_DISABLED,
        },
        {0},
    };

    return test_disable(conn, buffers, tests);
}

typedef struct SimulationTest {
    const char *response;
    RBRGen3Simulation expected;
} SimulationTest;

TEST_LOGGER3(simulation)
{
    SimulationTest tests[] = {
        {
            .response = "simulation state = off, period = 3600000" RESPONSE_TERMINATOR,
            .expected =
                {
                    .state = false,
                    .period = 3600000,
                },
        },
        {
            .response = "simulation state = on, period = 3600000" RESPONSE_TERMINATOR,
            .expected =
                {
                    .state = true,
                    .period = 3600000,
                },
        },
        {0},
    };

    RBRGen3Error err;
    RBRGen3Simulation actual;

    for (int i = 0; tests[i].response != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_getSimulation(conn, &actual);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.state, actual.state, bool);
        TEST_ASSERT_EQ(tests[i].expected.period, actual.period, "%" PRIi32);
    }

    return true;
}

typedef struct SimulationSetTest {
    RBRGen3Simulation simulation;
    const char *command;
    const char *response;
    RBRGen3Error expectedError;
} SimulationSetTest;

TEST_LOGGER3(simulation_set)
{
    SimulationSetTest tests[] = {
        {
            .simulation =
                {
                    .state = false,
                    .period = 3600000,
                },
            .command = "permit command = simulation" COMMAND_TERMINATOR
                       "simulation state = off, period = 3600000" COMMAND_TERMINATOR,
            .response = "permit command = simulation" RESPONSE_TERMINATOR
                        "simulation state = off, period = 3600000" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
        },
        {
            .simulation =
                {
                    .state = true,
                    .period = 3600000,
                },
            .command = "permit command = simulation" COMMAND_TERMINATOR
                       "simulation state = on, period = 3600000" COMMAND_TERMINATOR,
            .response = "permit command = simulation" RESPONSE_TERMINATOR
                        "simulation state = on, period = 3600000" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
        },
        {
            .simulation =
                {
                    .state = true,
                    .period = 123,
                },
            .command = "permit command = simulation" COMMAND_TERMINATOR
                       "simulation state = on, period = 123" COMMAND_TERMINATOR,
            .response = "permit command = simulation" RESPONSE_TERMINATOR
                        "E0108 invalid argument to command: '123'" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_HARDWARE_ERROR,
        },
        {
            .simulation =
                {
                    .state = false,
                    .period = 0,
                },
            .command = "",
            .response = "",
            .expectedError = RBRGEN3_INVALID_PARAMETER_VALUE,
        },
        {.simulation = {0}},
    };

    RBRGen3Error err;

    for (int i = 0; tests[i].command != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_setSimulation(conn, &tests[i].simulation);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen3Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }

    return true;
}
