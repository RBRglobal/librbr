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

typedef struct VerifyTest
{
    const char *expectedCommand;
    const char *response;
    const RBRInstrumentGen4Config config;
    const char *datasetLabel;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4Response expectedResponse;
    RBRInstrumentGen4DeploymentStatus expectedStatus;
} VerifyTest;

static bool test_verify(RBRInstrumentGen4 *instrument,
                        TestIOBuffers *buffers,
                        VerifyTest *tests)
{
    RBRInstrumentGen4Error err;
    RBRInstrumentGen4DeploymentStatus actualStatus;

    for (int i = 0; tests[i].expectedCommand != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_verify(instrument, &tests[i].config, tests[i].datasetLabel, &actualStatus);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentGen4Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedResponse.type,
                            instrument->response.type,
                            RBRInstrumentGen4ResponseType);
        TEST_ASSERT_EQ(tests[i].expectedResponse.error,
                       instrument->response.error,
                       "%" PRIi32);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedStatus,
                            actualStatus,
                            RBRInstrumentGen4DeploymentStatus);
    }

    return true;
}

TEST_LOGGER4(verify)
{
    VerifyTest tests[] = {
        {
            "verify config = profiling, dataset = test" COMMAND_TERMINATOR,
            "verify config = profiling, dataset = test, simulation = off, "
            "storagemode = normal, status = pending, warning = none" RESPONSE_TERMINATOR,
            { .label= "profiling"},
            "test",
            RBRINSTRUMENTGEN4_SUCCESS,
            {
                .type = RBRINSTRUMENTGEN4_RESPONSE_INFO,
                .error = RBRINSTRUMENTGEN4_HARDWARE_ERROR_NONE
            },
            RBRINSTRUMENTGEN4_STATUS_PENDING
        },
        {
            "verify config = profiling, dataset = test" COMMAND_TERMINATOR,
            "verify config = profiling, dataset = test, simulation = off, "
            "storagemode = normal, status = logging, warning = W0401" RESPONSE_TERMINATOR,
           { .label= "profiling"},
            "test",
            RBRINSTRUMENTGEN4_SUCCESS,
            {
                .type = RBRINSTRUMENTGEN4_RESPONSE_WARNING,
                .error = RBRINSTRUMENTGEN4_HARDWARE_ERROR_ESTIMATED_MEMORY_USAGE_EXCEEDS_CAPACITY
            },
            RBRINSTRUMENTGEN4_STATUS_LOGGING
        },
        {0}
    };

    return test_verify(instrument, buffers, tests);
}


typedef struct EnableTest
{
    const char *expectedCommand;
    const char *response;
    const RBRInstrumentGen4Config config;
    const char *datasetLabel;
    const bool simulation;
    const RBRInstrumentGen4DeploymentStoragemode storagemode;
    RBRInstrumentGen4Datasets datasets;
    int expectedDatasetCount;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4Response expectedResponse;
    RBRInstrumentGen4DeploymentStatus expectedStatus;
} EnableTest;

static bool test_enable(RBRInstrumentGen4 *instrument,
                        TestIOBuffers *buffers,
                        EnableTest *tests)
{
    RBRInstrumentGen4Error err;
    RBRInstrumentGen4DeploymentStatus actualStatus;

    for (int i = 0; tests[i].expectedCommand != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_enable(instrument, &tests[i].config, tests[i].datasetLabel, 
                                        false, tests[i].storagemode, 
                                        &tests[i].datasets, 
                                        &actualStatus);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentGen4Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedResponse.type,
                            instrument->response.type,
                            RBRInstrumentGen4ResponseType);
        TEST_ASSERT_EQ(tests[i].expectedResponse.error,
                       instrument->response.error,
                       "%" PRIi32);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedStatus,
                            actualStatus,
                            RBRInstrumentGen4DeploymentStatus);
        if(err == RBRINSTRUMENTGEN4_SUCCESS)
        {
            TEST_ASSERT_EQ(tests[i].expectedDatasetCount, tests[i].datasets.datasetlist.count, "%" PRIi32);
            TEST_ASSERT_STR_EQ(tests[i].datasetLabel, tests[i].datasets.datasetlist.datasets[tests[i].expectedDatasetCount-1]->label);
        }
}
    return true;
}

TEST_LOGGER4(enable)
{
    RBRInstrumentGen4Datasets datasets_empty={0};

    RBRInstrumentGen4Dataset existedDataset1 = {
        .label = "label1"
    };
    RBRInstrumentGen4Dataset existedDataset2 = {
        .label = "label2"
    };
    RBRInstrumentGen4Datasets datasets_some={
        .datasetlist = {
            .count = 2,
            .datasets = { &existedDataset1, &existedDataset2}
        }
    };
    EnableTest tests[] = {
        {
            "enable config = profiling, dataset = test, simulation = off, storagemode = normal" COMMAND_TERMINATOR,
            "enable config = profiling, dataset = test, simulation = off, storagemode = normal, "
            "status = pending, warning = none" RESPONSE_TERMINATOR,
            { .label= "profiling"},
            "test",
            false,
            RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL,
            datasets_empty,
            1,
            RBRINSTRUMENTGEN4_SUCCESS,
            {
                .type = RBRINSTRUMENTGEN4_RESPONSE_INFO,
                .error = RBRINSTRUMENTGEN4_HARDWARE_ERROR_NONE
            },
            RBRINSTRUMENTGEN4_STATUS_PENDING
        },
        {
            "enable config = profiling, dataset = test, simulation = off, storagemode = normal" COMMAND_TERMINATOR,
            "enable config = profiling, dataset = test, simulation = off, storagemode = normal, "
            "status = stopped, warning = none" RESPONSE_TERMINATOR,
            { .label= "profiling"},
            "test",
            false,
            RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL,
            datasets_some,
            3,
            RBRINSTRUMENTGEN4_SUCCESS,
            {
                .type = RBRINSTRUMENTGEN4_RESPONSE_INFO,
                .error = RBRINSTRUMENTGEN4_HARDWARE_ERROR_NONE
            },
            RBRINSTRUMENTGEN4_STATUS_STOPPED
        },
        {
            "enable config = profiling, dataset = label1, simulation = off, storagemode = normal" COMMAND_TERMINATOR,
            "E0170 'label1' qualifier used by another object" RESPONSE_TERMINATOR,
            { .label= "profiling"},
            "label1",
            false,
            RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL,
            datasets_some,
            2,
            RBRINSTRUMENTGEN4_HARDWARE_ERROR,
            {
                .type = RBRINSTRUMENTGEN4_RESPONSE_ERROR,
                .error = RBRINSTRUMENTGEN4_HARDWARE_ERROR_QUALIFIER_USED_BY_ANOTHER_OBJECT
            },
            RBRINSTRUMENTGEN4_STATUS_STOPPED
        },
        {
            "enable config = profiling, dataset = label1, simulation = off, storagemode = normal" COMMAND_TERMINATOR,
            "E0431 dataset limit reached, delete dataset(s) to make space" RESPONSE_TERMINATOR,
            { .label= "profiling"},
            "label1",
            false,
            RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL,
            datasets_some,
            2,
            RBRINSTRUMENTGEN4_HARDWARE_ERROR,
            {
                .type = RBRINSTRUMENTGEN4_RESPONSE_ERROR,
                .error = RBRINSTRUMENTGEN4_HARDWARE_ERROR_DATASET_LIMIT_REACHED
            },
            RBRINSTRUMENTGEN4_STATUS_STOPPED
        },
        {0}
    };

    return test_enable(instrument, buffers, tests);
}

typedef struct DisableTest
{
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4DeploymentStatus expectedStatus;
}DisableTest;

static bool test_disable(RBRInstrumentGen4 *instrument,
                         TestIOBuffers *buffers,
                         DisableTest *tests)
{
    RBRInstrumentGen4Error err;
    RBRInstrumentGen4DeploymentStatus actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_disable(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentGen4Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedStatus,
                            actual,
                            RBRInstrumentGen4DeploymentStatus);
    }

    return true;
}
TEST_LOGGER4(disable){
    DisableTest tests[]={
        {
            "disable status = disabled" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_SUCCESS,
            RBRINSTRUMENTGEN4_STATUS_DISABLED
        },
        {
            "disable status = stopped" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_SUCCESS,
            RBRINSTRUMENTGEN4_STATUS_STOPPED
        },
        {0}
    };
    return test_disable(instrument, buffers, tests);
}

typedef struct GetSimulationTest
{
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4Simulation expectedSimulation;
}GetSimulationTest;

static bool test_getSimulation(RBRInstrumentGen4 *instrument,
                         TestIOBuffers *buffers,
                         GetSimulationTest *tests)
{
    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Simulation actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_getSimulation(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentGen4Error);
        if(err == RBRINSTRUMENTGEN4_SUCCESS)
        {
            TEST_ASSERT_ENUM_EQ(tests[i].expectedSimulation.state,actual.state, bool);
            TEST_ASSERT_EQ(tests[i].expectedSimulation.period, actual.period, "%" PRIi32);
            TEST_ASSERT_STR_EQ(tests[i].expectedSimulation.channellabellist, actual.channellabellist);
        }
    }

    return true;
}
TEST_LOGGER4(getSimulation){
    GetSimulationTest tests[]={
        {
            "simulation state = off, period = 600000, channellist = none" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_SUCCESS,
            {
                .state = false,
                .period = 600000,
                .channellabellist = "none"
            }
        },
        {
            "simulation state = on, period = 600000, channellist = conductivity_00" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_SUCCESS,
            {
                .state = true,
                .period = 600000,
                .channellabellist = "conductivity_00"
            }
        },
        {
            "simulation state = off, period = 600000, channellist = conductivity_00|temperature_00|pressure_00" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_SUCCESS,
            {
                .state = false,
                .period = 600000,
                .channellabellist = "conductivity_00|temperature_00|pressure_00"
            }
        },
        {
            "E0105 command prohibited while logging" RESPONSE_TERMINATOR,
             RBRINSTRUMENTGEN4_HARDWARE_ERROR,
            {0}
        },
        {0}
    };
    return test_getSimulation(instrument, buffers, tests);
}

typedef struct SetSimulationTest
{
    RBRInstrumentGen4Simulation simulation;
    const char *command;
    const char *response;
    RBRInstrumentGen4Error expectedError;
}SetSimulationTest;

static bool test_setSimulation(RBRInstrumentGen4 *instrument,
                         TestIOBuffers *buffers,
                         SetSimulationTest *tests)
{
    RBRInstrumentGen4Error err;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_setSimulation(instrument, &tests[i].simulation);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }

    return true;
}

TEST_LOGGER4(setSimulation){
    SetSimulationTest tests[]={
        {
            {
                .state = true,
                .period = 3600000,
                .channellabellist = "conductivity_00"
            },
            "permit command = simulation" COMMAND_TERMINATOR
            "simulation state = on, period = 3600000, channellist = conductivity_00" COMMAND_TERMINATOR,
            "permit command = simulation" RESPONSE_TERMINATOR
            "simulation state = on, period = 3600000, channellist = conductivity_00" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_SUCCESS
        },
        {
            {
                .state = true,
                .period = 3600000,
                .channellabellist = "conductivity_00|temperature_00|pressure_00|chlorophyll_00"
            },
            "permit command = simulation" COMMAND_TERMINATOR
            "simulation state = on, period = 3600000, channellist = conductivity_00|temperature_00|pressure_00|chlorophyll_00" COMMAND_TERMINATOR,
            "permit command = simulation" RESPONSE_TERMINATOR
            "simulation state = on, period = 3600000, channellist = conductivity_00|temperature_00|pressure_00|chlorophyll_00" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_SUCCESS
        },
        {
            {
                .state = true,
                .period = 3600000,
                .channellabellist = "conductivity_00|temperature_00|pressure_00|chlorophyll_00"
            },
            "permit command = simulation" COMMAND_TERMINATOR
            "simulation state = on, period = 3600000, channellist = conductivity_00|temperature_00|pressure_00|chlorophyll_00" COMMAND_TERMINATOR,
            "permit command = simulation" RESPONSE_TERMINATOR
            "E0105 command prohibited while logging" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_HARDWARE_ERROR
        },
        {
            {0},0,0,0
        }
    };
    return test_setSimulation(instrument, buffers, tests);
}