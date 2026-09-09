/**
 * \file deployment.c
 *
 * \brief Tests for instrument deployment commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include "tests.h"

typedef struct StatusTest
{
    const char *response;
    RBRGen3Error expectedError;
    RBRGen3Response expectedResponse;
    RBRGen3DeploymentStatus expected;
} StatusTest;

static bool test_verify(RBRGen3 *conn,
                        TestIOBuffers *buffers,
                        StatusTest *tests)
{
    RBRGen3Error err;
    RBRGen3DeploymentStatus actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_verify(conn, false, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedResponse.type,
                            conn->response.type,
                            RBRGen3ResponseType);
        TEST_ASSERT_EQ(tests[i].expectedResponse.error,
                       conn->response.error,
                       "%" PRIi32);
        TEST_ASSERT_ENUM_EQ(tests[i].expected,
                            actual,
                            RBRGen3DeploymentStatus);
    }

    return true;
}

TEST_LOGGER2(verify)
{
    StatusTest tests[] = {
        {
            "verify = pending" RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS,
            {
                .type = RBRGEN3_RESPONSE_INFO,
                .error = RBRGEN3_HARDWARE_ERROR_NONE
            },
            RBRGEN3_STATUS_PENDING
        },
        {
            "verify = logging" RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS,
            {
                .type = RBRGEN3_RESPONSE_INFO,
                .error = RBRGEN3_HARDWARE_ERROR_NONE
            },
            RBRGEN3_STATUS_LOGGING
        },
        {
            "E0402 memory not empty, erase first, verify = stopped"
            RESPONSE_TERMINATOR,
            RBRGEN3_HARDWARE_ERROR,
            {
                .type = RBRGEN3_RESPONSE_ERROR,
                .error = RBRGEN3_HARDWARE_ERROR_MEMORY_NOT_EMPTY_ERASE_FIRST
            },
            RBRGEN3_UNKNOWN_STATUS
        },
        {
            "E0401 estimated memory usage exceeds capacity, verify = logging"
            RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS,
            {
                .type = RBRGEN3_RESPONSE_WARNING,
                .error = RBRGEN3_HARDWARE_ERROR_ESTIMATED_MEMORY_USAGE_EXCEEDS_CAPACITY
            },
            RBRGEN3_STATUS_LOGGING
        },
        {0}
    };

    return test_verify(conn, buffers, tests);
}

TEST_LOGGER3(verify)
{
    StatusTest tests[] = {
        {
            "verify status = pending, warning = none" RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS,
            {
                .type = RBRGEN3_RESPONSE_INFO,
                .error = RBRGEN3_HARDWARE_ERROR_NONE
            },
            RBRGEN3_STATUS_PENDING
        },
        {
            "verify status = logging, warning = none" RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS,
            {
                .type = RBRGEN3_RESPONSE_INFO,
                .error = RBRGEN3_HARDWARE_ERROR_NONE
            },
            RBRGEN3_STATUS_LOGGING
        },
        {
            "E0402 memory not empty, erase first" RESPONSE_TERMINATOR,
            RBRGEN3_HARDWARE_ERROR,
            {
                .type = RBRGEN3_RESPONSE_ERROR,
                .error = RBRGEN3_HARDWARE_ERROR_MEMORY_NOT_EMPTY_ERASE_FIRST
            },
            RBRGEN3_UNKNOWN_STATUS
        },
        {
            "verify status = logging, warning = W0401" RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS,
            {
                .type = RBRGEN3_RESPONSE_WARNING,
                .error = RBRGEN3_HARDWARE_ERROR_ESTIMATED_MEMORY_USAGE_EXCEEDS_CAPACITY
            },
            RBRGEN3_STATUS_LOGGING
        },
        {0}
    };

    return test_verify(conn, buffers, tests);
}

static bool test_enable(RBRGen3 *conn,
                        TestIOBuffers *buffers,
                        StatusTest *tests)
{
    RBRGen3Error err;
    RBRGen3DeploymentStatus actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_enable(conn, false, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedResponse.type,
                            conn->response.type,
                            RBRGen3ResponseType);
        TEST_ASSERT_EQ(tests[i].expectedResponse.error,
                       conn->response.error,
                       "%" PRIi32);
        TEST_ASSERT_ENUM_EQ(tests[i].expected,
                            actual,
                            RBRGen3DeploymentStatus);
    }

    return true;
}

TEST_LOGGER2(enable)
{
    StatusTest tests[] = {
        {
            "enable = pending" RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS,
            {
                .type = RBRGEN3_RESPONSE_INFO,
                .error = RBRGEN3_HARDWARE_ERROR_NONE
            },
            RBRGEN3_STATUS_PENDING
        },
        {
            "enable = logging" RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS,
            {
                .type = RBRGEN3_RESPONSE_INFO,
                .error = RBRGEN3_HARDWARE_ERROR_NONE
            },
            RBRGEN3_STATUS_LOGGING
        },
        {
            "E0402 memory not empty, erase first"
            RESPONSE_TERMINATOR,
            RBRGEN3_HARDWARE_ERROR,
            {
                .type = RBRGEN3_RESPONSE_ERROR,
                .error = RBRGEN3_HARDWARE_ERROR_MEMORY_NOT_EMPTY_ERASE_FIRST
            },
            RBRGEN3_UNKNOWN_STATUS
        },
        {
            "E0401 estimated memory usage exceeds capacity, enable = logging"
            RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS,
            {
                .type = RBRGEN3_RESPONSE_WARNING,
                .error = RBRGEN3_HARDWARE_ERROR_ESTIMATED_MEMORY_USAGE_EXCEEDS_CAPACITY
            },
            RBRGEN3_STATUS_LOGGING
        },
        {0}
    };

    return test_enable(conn, buffers, tests);
}

TEST_LOGGER3(enable)
{
    StatusTest tests[] = {
        {
            "enable status = pending, warning = none" RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS,
            {
                .type = RBRGEN3_RESPONSE_INFO,
                .error = RBRGEN3_HARDWARE_ERROR_NONE
            },
            RBRGEN3_STATUS_PENDING
        },
        {
            "enable status = logging, warning = none" RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS,
            {
                .type = RBRGEN3_RESPONSE_INFO,
                .error = RBRGEN3_HARDWARE_ERROR_NONE
            },
            RBRGEN3_STATUS_LOGGING
        },
        {
            "E0402 memory not empty, erase first" RESPONSE_TERMINATOR,
            RBRGEN3_HARDWARE_ERROR,
            {
                .type = RBRGEN3_RESPONSE_ERROR,
                .error = RBRGEN3_HARDWARE_ERROR_MEMORY_NOT_EMPTY_ERASE_FIRST
            },
            RBRGEN3_UNKNOWN_STATUS
        },
        {
            "enable status = logging, warning = W0401" RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS,
            {
                .type = RBRGEN3_RESPONSE_WARNING,
                .error = RBRGEN3_HARDWARE_ERROR_ESTIMATED_MEMORY_USAGE_EXCEEDS_CAPACITY
            },
            RBRGEN3_STATUS_LOGGING
        },
        {0}
    };

    return test_enable(conn, buffers, tests);
}

static bool test_disable(RBRGen3 *conn,
                         TestIOBuffers *buffers,
                         StatusTest *tests)
{
    RBRGen3Error err;
    RBRGen3DeploymentStatus actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_disable(conn, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedResponse.type,
                            conn->response.type,
                            RBRGen3ResponseType);
        TEST_ASSERT_EQ(tests[i].expectedResponse.error,
                       conn->response.error,
                       "%" PRIi32);
        TEST_ASSERT_ENUM_EQ(tests[i].expected,
                            actual,
                            RBRGen3DeploymentStatus);
    }

    return true;
}

TEST_LOGGER2(stop)
{
    StatusTest tests[] = {
        {
            "stop = stopped" RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS,
            {
                .type = RBRGEN3_RESPONSE_INFO,
                .error = RBRGEN3_HARDWARE_ERROR_NONE
            },
            RBRGEN3_STATUS_STOPPED
        },
        {
            "E0406 not logging, stop = stopped" RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS,
            {
                .type = RBRGEN3_RESPONSE_WARNING,
                .error = RBRGEN3_HARDWARE_ERROR_NOT_LOGGING
            },
            RBRGEN3_STATUS_STOPPED
        },
        {
            "E0406 not logging, stop = fullandstopped" RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS,
            {
                .type = RBRGEN3_RESPONSE_WARNING,
                .error = RBRGEN3_HARDWARE_ERROR_NOT_LOGGING
            },
            RBRGEN3_STATUS_FULLANDSTOPPED
        },
        {
            "E0406 not logging, stop = disabled" RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS,
            {
                .type = RBRGEN3_RESPONSE_WARNING,
                .error = RBRGEN3_HARDWARE_ERROR_NOT_LOGGING
            },
            RBRGEN3_STATUS_DISABLED
        },
        {0}
    };

    return test_disable(conn, buffers, tests);
}

TEST_LOGGER3(disable)
{
    StatusTest tests[] = {
        {
            "disable status = stopped" RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS,
            {
                .type = RBRGEN3_RESPONSE_INFO,
                .error = RBRGEN3_HARDWARE_ERROR_NONE
            },
            RBRGEN3_STATUS_STOPPED
        },
        {
            "disable status = fullandstopped" RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS,
            {
                .type = RBRGEN3_RESPONSE_INFO,
                .error = RBRGEN3_HARDWARE_ERROR_NONE
            },
            RBRGEN3_STATUS_FULLANDSTOPPED
        },
        {
            "disable status = disabled" RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS,
            {
                .type = RBRGEN3_RESPONSE_INFO,
                .error = RBRGEN3_HARDWARE_ERROR_NONE
            },
            RBRGEN3_STATUS_DISABLED
        },
        {0}
    };

    return test_disable(conn, buffers, tests);
}

typedef struct SimulationTest
{
    const char *response;
    RBRGen3Simulation expected;
} SimulationTest;

TEST_LOGGER3(simulation)
{
    SimulationTest tests[] = {
        {
            "simulation state = off, period = 3600000" RESPONSE_TERMINATOR,
            {
                .state = false,
                .period = 3600000
            }
        },
        {
            "simulation state = on, period = 3600000" RESPONSE_TERMINATOR,
            {
                .state = true,
                .period = 3600000
            }
        },
        {0}
    };

    RBRGen3Error err;
    RBRGen3Simulation actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_getSimulation(conn, &actual);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.state, actual.state, bool);
        TEST_ASSERT_EQ(tests[i].expected.period, actual.period, "%" PRIi32);
    }

    return true;
}

typedef struct SimulationSetTest
{
    RBRGen3Simulation simulation;
    const char *command;
    const char *response;
    RBRGen3Error expectedError;
} SimulationSetTest;

TEST_LOGGER3(simulation_set)
{
    SimulationSetTest tests[] = {
        {
            {
                .state = false,
                .period = 3600000
            },
            "permit command = simulation" COMMAND_TERMINATOR
            "simulation state = off, period = 3600000" COMMAND_TERMINATOR,
            "permit command = simulation" RESPONSE_TERMINATOR
            "simulation state = off, period = 3600000" RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS
        },
        {
            {
                .state = true,
                .period = 3600000
            },
            "permit command = simulation" COMMAND_TERMINATOR
            "simulation state = on, period = 3600000" COMMAND_TERMINATOR,
            "permit command = simulation" RESPONSE_TERMINATOR
            "simulation state = on, period = 3600000" RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS
        },
        {
            {
                .state = true,
                .period = 123
            },
            "permit command = simulation" COMMAND_TERMINATOR
            "simulation state = on, period = 123" COMMAND_TERMINATOR,
            "permit command = simulation" RESPONSE_TERMINATOR
            "E0108 invalid argument to command: '123'"
            RESPONSE_TERMINATOR,
            RBRGEN3_HARDWARE_ERROR
        },
        {
            {
                .state = false,
                .period = 0
            },
            "",
            "",
            RBRGEN3_INVALID_PARAMETER_VALUE
        },
        {{0}, 0, 0, 0}
    };

    RBRGen3Error err;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_setSimulation(conn, &tests[i].simulation);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen3Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }

    return true;
}
