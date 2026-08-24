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
#include "RBRInstrumentGen4Deployment.h"

/* Required for isnan, NAN. */
#include <math.h>

typedef struct GetClockTest
{
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4Clock expected;
} GetClockTest;

TEST_LOGGER4(getClock)
{
    GetClockTest tests[] = {
        /* A bare query reports both parameters. */
        { "clock datetime=20260824120000 offsetfromutc=0.00"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 1787572800000LL, 0.0f } },
        /* The instrument never signs a positive offset. */
        { "clock datetime=20260824120130 offsetfromutc=5.50"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 1787572890000LL, 5.5f } },
        { "clock datetime=20260824120000 offsetfromutc=-4.50"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 1787572800000LL, -4.5f } },
        /*
         * The response echoes the parameters in the order they were asked
         * for, so the getter must not depend on their position.
         */
        { "clock offsetfromutc=14.00 datetime=20260824120000"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 1787572800000LL, 14.0f } },
        /* An unreported parameter keeps its unset value. */
        { "clock datetime=20260824120000" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 1787572800000LL, NAN } },
        { "clock" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 0, NAN } },
        /* Keys the library does not model are ignored. */
        { "clock datetime=20260824120000 offsetfromutc=0.00 bogus=1"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 1787572800000LL, 0.0f } },
        { "ERR-105 command prohibited while logging" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR,
          { 0, NAN } },
        { 0 }
    };

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Clock actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_getClock(instrument, &actual);
        TEST_ASSERT_STR_EQ("clock" COMMAND_TERMINATOR, buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRInstrumentGen4Error);
        if (err != RBRINSTRUMENTGEN4_SUCCESS)
        {
            continue;
        }
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
                                 0.005f);
        }
    }

    return true;
}

typedef struct SetClockTest
{
    RBRInstrumentGen4Clock clock;
    const char *command;
    const char *response;
    RBRInstrumentGen4Error expectedError;
} SetClockTest;

TEST_LOGGER4(setClock)
{
    SetClockTest tests[] = {
        /* Both parameters are always sent. */
        { { 1787572800000LL, 0.0f },
          "clock datetime=20260824120000 offsetfromutc=0.00"
          COMMAND_TERMINATOR,
          "clock datetime=20260824120000 offsetfromutc=0.00"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS },
        { { 1787572890000LL, 5.5f },
          "clock datetime=20260824120130 offsetfromutc=5.50"
          COMMAND_TERMINATOR,
          "clock datetime=20260824120130 offsetfromutc=5.50"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS },
        { { 1787572800000LL, -12.0f },
          "clock datetime=20260824120000 offsetfromutc=-12.00"
          COMMAND_TERMINATOR,
          "clock datetime=20260824120000 offsetfromutc=-12.00"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS },
        /*
         * The offset is not range-checked: an offset the instrument will not
         * accept is sent and refused there.
         */
        { { 1787572800000LL, 99.0f },
          "clock datetime=20260824120000 offsetfromutc=99.00"
          COMMAND_TERMINATOR,
          "ERR-108 invalid argument to command: '99.00'" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR },
        /* `NAN` would emit "nan", so it is refused before the command. */
        { { 1787572800000LL, NAN },
          "",
          "",
          RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE },
        { { RBRINSTRUMENTGEN4_DATETIME_MIN - 1, 0.0f },
          "",
          "",
          RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE },
        { { RBRINSTRUMENTGEN4_DATETIME_MAX + 1, 0.0f },
          "",
          "",
          RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE },
        { { 1787572800000LL, 0.0f },
          "clock datetime=20260824120000 offsetfromutc=0.00"
          COMMAND_TERMINATOR,
          "ERR-105 command prohibited while logging" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR },
        { { 0 }, NULL, NULL, 0 }
    };

    RBRInstrumentGen4Error err;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_setClock(instrument, &tests[i].clock);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }

    return true;
}

typedef struct GetDeploymentTest
{
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4Deployment expected;
} GetDeploymentTest;

TEST_LOGGER4(getDeployment)
{
    GetDeploymentTest tests[] = {
        /* No gating condition, so no start time is reported. */
        { "deployment status=inactive gate=none simulation=off"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 0,
            RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_INACTIVE,
            RBRINSTRUMENTGEN4_GATE_NONE,
            false } },
        /* Time gating adds the start time, ahead of the other parameters. */
        { "deployment starttime=20270101000000 status=inactive gate=time "
          "simulation=off" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 1798761600000LL,
            RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_INACTIVE,
            RBRINSTRUMENTGEN4_GATE_TIME,
            false } },
        { "deployment status=sampling gate=none simulation=off"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 0,
            RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_SAMPLING,
            RBRINSTRUMENTGEN4_GATE_NONE,
            false } },
        { "deployment status=paused gate=none simulation=off"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 0,
            RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_PAUSED,
            RBRINSTRUMENTGEN4_GATE_NONE,
            false } },
        { "deployment starttime=20270101000000 status=gated gate=time "
          "simulation=off" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 1798761600000LL,
            RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_GATED,
            RBRINSTRUMENTGEN4_GATE_TIME,
            false } },
        { "deployment status=inactive gate=twistactivation simulation=on"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 0,
            RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_INACTIVE,
            RBRINSTRUMENTGEN4_GATE_TWISTACTIVATION,
            true } },
        { "deployment status=inactive gate=wetswitch simulation=off"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 0,
            RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_INACTIVE,
            RBRINSTRUMENTGEN4_GATE_WETSWITCH,
            false } },
        /*
         * A value the library does not model reads as unknown rather than as
         * the first member.
         */
        { "deployment status=bogus gate=bogus simulation=off"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 0,
            RBRINSTRUMENTGEN4_UNKNOWN_DEPLOYMENT_STATUS,
            RBRINSTRUMENTGEN4_UNKNOWN_GATE,
            false } },
        /* An unreported parameter is left unknown, not zero. */
        { "deployment" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 0,
            RBRINSTRUMENTGEN4_UNKNOWN_DEPLOYMENT_STATUS,
            RBRINSTRUMENTGEN4_UNKNOWN_GATE,
            false } },
        /* Keys the library does not model are ignored. */
        { "deployment status=inactive gate=none simulation=off bogus=1"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 0,
            RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_INACTIVE,
            RBRINSTRUMENTGEN4_GATE_NONE,
            false } },
        { 0 }
    };

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Deployment actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_getDeployment(instrument, &actual);
        TEST_ASSERT_STR_EQ("deployment" COMMAND_TERMINATOR,
                           buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRInstrumentGen4Error);
        TEST_ASSERT_EQ(tests[i].expected.startTime,
                       actual.startTime,
                       "%" PRIi64);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.status,
                            actual.status,
                            RBRInstrumentGen4DeploymentStatus);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.gate,
                            actual.gate,
                            RBRInstrumentGen4Gate);
        TEST_ASSERT_EQ(tests[i].expected.simulation,
                       actual.simulation,
                       "%d");
    }

    return true;
}

typedef struct SetDeploymentTest
{
    RBRInstrumentGen4Deployment deployment;
    const char *command;
    const char *response;
    RBRInstrumentGen4Error expectedError;
} SetDeploymentTest;

TEST_LOGGER4(setDeployment)
{
    SetDeploymentTest tests[] = {
        /*
         * Without time gating only `gate` is sent: the instrument answers
         * `ERR-108` for `starttime` under any other condition.
         */
        { { 0, RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_INACTIVE,
            RBRINSTRUMENTGEN4_GATE_NONE, false },
          "deployment gate=none" COMMAND_TERMINATOR,
          "deployment gate=none" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS },
        /* A start time left over from a previous read is not sent either. */
        { { 1798761600000LL, RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_INACTIVE,
            RBRINSTRUMENTGEN4_GATE_WETSWITCH, false },
          "deployment gate=wetswitch" COMMAND_TERMINATOR,
          "deployment gate=wetswitch" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS },
        { { 0, RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_INACTIVE,
            RBRINSTRUMENTGEN4_GATE_TWISTACTIVATION, false },
          "deployment gate=twistactivation" COMMAND_TERMINATOR,
          "deployment gate=twistactivation" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS },
        /* Time gating takes both parameters in one command. */
        { { 1798761600000LL, RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_INACTIVE,
            RBRINSTRUMENTGEN4_GATE_TIME, false },
          "deployment gate=time starttime=20270101000000"
          COMMAND_TERMINATOR,
          "deployment gate=time starttime=20270101000000"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS },
        /*
         * A start time is range-checked only when it is going to be sent, so
         * an out-of-range value under time gating fails and the same value
         * under any other condition does not.
         */
        { { RBRINSTRUMENTGEN4_DATETIME_MIN - 1,
            RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_INACTIVE,
            RBRINSTRUMENTGEN4_GATE_TIME, false },
          "",
          "",
          RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE },
        { { RBRINSTRUMENTGEN4_DATETIME_MAX + 1,
            RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_INACTIVE,
            RBRINSTRUMENTGEN4_GATE_TIME, false },
          "",
          "",
          RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE },
        { { RBRINSTRUMENTGEN4_DATETIME_MAX + 1,
            RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_INACTIVE,
            RBRINSTRUMENTGEN4_GATE_NONE, false },
          "deployment gate=none" COMMAND_TERMINATOR,
          "deployment gate=none" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS },
        /* The sentinels a getter can leave behind never reach the command. */
        { { 0, RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_INACTIVE,
            RBRINSTRUMENTGEN4_UNKNOWN_GATE, false },
          "",
          "",
          RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE },
        { { 0, RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_INACTIVE,
            RBRINSTRUMENTGEN4_GATE_COUNT, false },
          "",
          "",
          RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE },
        /* Both writable parameters are unavailable while logging. */
        { { 0, RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_SAMPLING,
            RBRINSTRUMENTGEN4_GATE_NONE, false },
          "deployment gate=none" COMMAND_TERMINATOR,
          "ERR-105 command prohibited while logging" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR },
        /* A gating condition the instrument does not offer is refused. */
        { { 0, RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_INACTIVE,
            RBRINSTRUMENTGEN4_GATE_WETSWITCH, false },
          "deployment gate=wetswitch" COMMAND_TERMINATOR,
          "ERR-108 invalid argument to command: 'wetswitch'"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR },
        { { 0 }, NULL, NULL, 0 }
    };

    RBRInstrumentGen4Error err;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_setDeployment(instrument,
                                              &tests[i].deployment);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }

    return true;
}

typedef struct VerifyTest
{
    const RBRInstrumentGen4Config config;
    const char *datasetLabel;
    RBRInstrumentGen4DeploymentStoragemode storageMode;
    const char *command;
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4InstrumentState expectedState;
} VerifyTest;

TEST_LOGGER4(verify)
{
    VerifyTest tests[] = {
        /* All three parameters are always sent. */
        { { .label = "c_test" },
          "d1",
          RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL,
          "verify config=c_test dataset=d1 storagemode=normal"
          COMMAND_TERMINATOR,
          "verify config=c_test dataset=d1 storagemode=normal state=enabled"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_INSTRUMENT_STATE_ENABLED },
        { { .label = "pH_cal" },
          "d_pHcal_20260824",
          RBRINSTRUMENTGEN4_STORAGEMODE_CALIBRATION,
          "verify config=pH_cal dataset=d_pHcal_20260824 "
          "storagemode=calibration" COMMAND_TERMINATOR,
          "verify config=pH_cal dataset=d_pHcal_20260824 "
          "storagemode=calibration state=enabled" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_INSTRUMENT_STATE_ENABLED },
        /*
         * The response echoes the parameters in whatever order they were
         * sent, and `state` need not come last.
         */
        { { .label = "c_test" },
          "d1",
          RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL,
          "verify config=c_test dataset=d1 storagemode=normal"
          COMMAND_TERMINATOR,
          "verify state=disabled dataset=d1 config=c_test storagemode=normal"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_INSTRUMENT_STATE_DISABLED },
        /* A state the library does not model reads as unknown. */
        { { .label = "c_test" },
          "d1",
          RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL,
          "verify config=c_test dataset=d1 storagemode=normal"
          COMMAND_TERMINATOR,
          "verify config=c_test dataset=d1 state=bogus" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_UNKNOWN_INSTRUMENT_STATE },
        /* A response with no state at all leaves it unknown. */
        { { .label = "c_test" },
          "d1",
          RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL,
          "verify config=c_test dataset=d1 storagemode=normal"
          COMMAND_TERMINATOR,
          "verify config=c_test dataset=d1" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_UNKNOWN_INSTRUMENT_STATE },
        /* Out-of-range parameters never reach the instrument. */
        { { .label = "" },
          "d1",
          RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL,
          "",
          "",
          RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE,
          RBRINSTRUMENTGEN4_UNKNOWN_INSTRUMENT_STATE },
        { { .label = "c_test" },
          "",
          RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL,
          "",
          "",
          RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE,
          RBRINSTRUMENTGEN4_UNKNOWN_INSTRUMENT_STATE },
        { { .label = "c_test" },
          "0123456789012345678901234567890123",
          RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL,
          "",
          "",
          RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE,
          RBRINSTRUMENTGEN4_UNKNOWN_INSTRUMENT_STATE },
        { { .label = "c_test" },
          "d1",
          RBRINSTRUMENTGEN4_UNKNOWN_STORAGEMODE,
          "",
          "",
          RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE,
          RBRINSTRUMENTGEN4_UNKNOWN_INSTRUMENT_STATE },
        { { .label = "c_test" },
          "d1",
          RBRINSTRUMENTGEN4_STORAGEMODE_COUNT,
          "",
          "",
          RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE,
          RBRINSTRUMENTGEN4_UNKNOWN_INSTRUMENT_STATE },
        /* Every failing check the instrument makes. */
        { { .label = "nope" },
          "d1",
          RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL,
          "verify config=nope dataset=d1 storagemode=normal"
          COMMAND_TERMINATOR,
          "ERR-108 invalid argument to command: 'nope'" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR,
          RBRINSTRUMENTGEN4_UNKNOWN_INSTRUMENT_STATE },
        { { .label = "c_test" },
          "d.1",
          RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL,
          "verify config=c_test dataset=d.1 storagemode=normal"
          COMMAND_TERMINATOR,
          "ERR-131 illegal character in label 'd.1'" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR,
          RBRINSTRUMENTGEN4_UNKNOWN_INSTRUMENT_STATE },
        { { .label = "c_test" },
          "d1",
          RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL,
          "verify config=c_test dataset=d1 storagemode=normal"
          COMMAND_TERMINATOR,
          "ERR-120 'd1' is already in use" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR,
          RBRINSTRUMENTGEN4_UNKNOWN_INSTRUMENT_STATE },
        { { .label = "c_test" },
          "d5",
          RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL,
          "verify config=c_test dataset=d5 storagemode=normal"
          COMMAND_TERMINATOR,
          "ERR-431 dataset limit of '4' reached, delete dataset(s) to make "
          "space" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR,
          RBRINSTRUMENTGEN4_UNKNOWN_INSTRUMENT_STATE },
        { { .label = "c_test" },
          "d2",
          RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL,
          "verify config=c_test dataset=d2 storagemode=normal"
          COMMAND_TERMINATOR,
          "ERR-436 instrument was already enabled with different settings"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR,
          RBRINSTRUMENTGEN4_UNKNOWN_INSTRUMENT_STATE },
        { { .label = "" }, NULL, 0, NULL, NULL, 0, 0 }
    };

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4InstrumentState actual;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        actual = RBRINSTRUMENTGEN4_UNKNOWN_INSTRUMENT_STATE;
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_verify(instrument,
                                       &tests[i].config,
                                       tests[i].datasetLabel,
                                       tests[i].storageMode,
                                       &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedState,
                            actual,
                            RBRInstrumentGen4InstrumentState);
    }

    return true;
}
