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

/* Required for isnan, NAN. */
#include <math.h>

#include "tests.h"
#include "RBRGen4Deployment.h"
#include "RBRGen4Configuration.h"
#include "RBRGen4Instrument.h"

typedef struct GetClockTest {
    const char *response;
    RBRGen4Error expectedError;
    RBRGen4Clock expected;
} GetClockTest;

TEST_LOGGER4(getClock)
{
    GetClockTest tests[] = {
        /* A bare query reports both parameters. */
        {
            "clock datetime=20260824120000 offsetfromutc=0.00" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            {
                .dateTime = 1787572800000LL,
                .offsetFromUtc = 0.0f,
            },
        },
        /* The instrument never signs a positive offset. */
        {
            "clock datetime=20260824120130 offsetfromutc=5.50" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            {
                .dateTime = 1787572890000LL,
                .offsetFromUtc = 5.5f,
            },
        },
        {
            "clock datetime=20260824120000 offsetfromutc=-4.50" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            {
                .dateTime = 1787572800000LL,
                .offsetFromUtc = -4.5f,
            },
        },
        /*
         * The response echoes the parameters in the order they were asked
         * for, so the getter must not depend on their position.
         */
        {
            "clock offsetfromutc=14.00 datetime=20260824120000" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            {
                .dateTime = 1787572800000LL,
                .offsetFromUtc = 14.0f,
            },
        },
        /* An unreported parameter keeps its unset value. */
        {
            "clock datetime=20260824120000" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            {
                .dateTime = 1787572800000LL,
                .offsetFromUtc = NAN,
            },
        },
        {
            "clock" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            {
                .dateTime = 0,
                .offsetFromUtc = NAN,
            },
        },
        /* Keys the library does not model are ignored. */
        {
            "clock datetime=20260824120000 offsetfromutc=0.00 bogus=1" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            {
                .dateTime = 1787572800000LL,
                .offsetFromUtc = 0.0f,
            },
        },
        {
            "ERR-105 command prohibited while logging" RESPONSE_TERMINATOR,
            RBRGEN4_HARDWARE_ERROR,
            {
                .dateTime = 0,
                .offsetFromUtc = NAN,
            },
        },
        {0},
    };

    RBRGen4Error err;
    RBRGen4Clock actual;

    for (int i = 0; tests[i].response != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4_getClock(conn, &actual);
        TEST_ASSERT_STR_EQ("clock" COMMAND_TERMINATOR, buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen4Error);
        if (err != RBRGEN4_SUCCESS) {
            continue;
        }
        TEST_ASSERT_EQ(tests[i].expected.dateTime, actual.dateTime, "%" PRIi64);
        if (isnan(tests[i].expected.offsetFromUtc)) {
            TEST_ASSERT(isnan(actual.offsetFromUtc));
        } else {
            TEST_ASSERT_FLOAT_EQ(tests[i].expected.offsetFromUtc, actual.offsetFromUtc, 0.005f);
        }
    }

    return true;
}

typedef struct SetClockTest {
    RBRGen4Clock clock;
    const char *command;
    const char *response;
    RBRGen4Error expectedError;
} SetClockTest;

TEST_LOGGER4(setClock)
{
    SetClockTest tests[] = {
        /* Both parameters are always sent. */
        {
            {
                .dateTime = 1787572800000LL,
                .offsetFromUtc = 0.0f,
            },
            "clock datetime=20260824120000 offsetfromutc=0.00" COMMAND_TERMINATOR,
            "clock datetime=20260824120000 offsetfromutc=0.00" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
        },
        {
            {
                .dateTime = 1787572890000LL,
                .offsetFromUtc = 5.5f,
            },
            "clock datetime=20260824120130 offsetfromutc=5.50" COMMAND_TERMINATOR,
            "clock datetime=20260824120130 offsetfromutc=5.50" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
        },
        {
            {
                .dateTime = 1787572800000LL,
                .offsetFromUtc = -12.0f,
            },
            "clock datetime=20260824120000 offsetfromutc=-12.00" COMMAND_TERMINATOR,
            "clock datetime=20260824120000 offsetfromutc=-12.00" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
        },
        /*
         * The offset is not range-checked: an offset the instrument will not
         * accept is sent and refused there.
         */
        {
            {
                .dateTime = 1787572800000LL,
                .offsetFromUtc = 99.0f,
            },
            "clock datetime=20260824120000 offsetfromutc=99.00" COMMAND_TERMINATOR,
            "ERR-108 invalid argument to command: '99.00'" RESPONSE_TERMINATOR,
            RBRGEN4_HARDWARE_ERROR,
        },
        /* `NAN` would emit "nan", so it is refused before the command. */
        {
            {
                .dateTime = 1787572800000LL,
                .offsetFromUtc = NAN,
            },
            "",
            "",
            RBRGEN4_INVALID_PARAMETER_VALUE,
        },
        {
            {
                .dateTime = RBRGEN4_DATETIME_MIN - 1,
                .offsetFromUtc = 0.0f,
            },
            "",
            "",
            RBRGEN4_INVALID_PARAMETER_VALUE,
        },
        {
            {
                .dateTime = RBRGEN4_DATETIME_MAX + 1,
                .offsetFromUtc = 0.0f,
            },
            "",
            "",
            RBRGEN4_INVALID_PARAMETER_VALUE,
        },
        {
            {
                .dateTime = 1787572800000LL,
                .offsetFromUtc = 0.0f,
            },
            "clock datetime=20260824120000 offsetfromutc=0.00" COMMAND_TERMINATOR,
            "ERR-105 command prohibited while logging" RESPONSE_TERMINATOR,
            RBRGEN4_HARDWARE_ERROR,
        },
        {
            {0},
            NULL,
            NULL,
            0,
        },
    };

    RBRGen4Error err;

    for (int i = 0; tests[i].command != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4_setClock(conn, &tests[i].clock);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }

    return true;
}

typedef struct GetDeploymentTest {
    const char *response;
    RBRGen4Error expectedError;
    RBRGen4Deployment expected;
} GetDeploymentTest;

TEST_LOGGER4(getDeployment)
{
    GetDeploymentTest tests[] = {
        /* No gating condition, so no start time is reported. */
        {
            "deployment status=inactive gate=none simulation=off" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            {
                .startTime = 0,
                .status = RBRGEN4_DEPLOYMENT_STATUS_INACTIVE,
                .gate = RBRGEN4_DEPLOYMENT_GATE_NONE,
                .simulation = false,
            },
        },
        /* Time gating adds the start time, ahead of the other parameters. */
        {
            "deployment starttime=20270101000000 status=inactive gate=time "
            "simulation=off" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            {
                .startTime = 1798761600000LL,
                .status = RBRGEN4_DEPLOYMENT_STATUS_INACTIVE,
                .gate = RBRGEN4_DEPLOYMENT_GATE_TIME,
                .simulation = false,
            },
        },
        {
            "deployment status=sampling gate=none simulation=off" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            {
                .startTime = 0,
                .status = RBRGEN4_DEPLOYMENT_STATUS_SAMPLING,
                .gate = RBRGEN4_DEPLOYMENT_GATE_NONE,
                .simulation = false,
            },
        },
        {
            "deployment status=paused gate=none simulation=off" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            {
                .startTime = 0,
                .status = RBRGEN4_DEPLOYMENT_STATUS_PAUSED,
                .gate = RBRGEN4_DEPLOYMENT_GATE_NONE,
                .simulation = false,
            },
        },
        {
            "deployment starttime=20270101000000 status=gated gate=time "
            "simulation=off" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            {
                .startTime = 1798761600000LL,
                .status = RBRGEN4_DEPLOYMENT_STATUS_GATED,
                .gate = RBRGEN4_DEPLOYMENT_GATE_TIME,
                .simulation = false,
            },
        },
        {
            "deployment status=inactive gate=twistactivation simulation=on" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            {
                .startTime = 0,
                .status = RBRGEN4_DEPLOYMENT_STATUS_INACTIVE,
                .gate = RBRGEN4_DEPLOYMENT_GATE_TWISTACTIVATION,
                .simulation = true,
            },
        },
        {
            "deployment status=inactive gate=wetswitch simulation=off" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            {
                .startTime = 0,
                .status = RBRGEN4_DEPLOYMENT_STATUS_INACTIVE,
                .gate = RBRGEN4_DEPLOYMENT_GATE_WETSWITCH,
                .simulation = false,
            },
        },
        /*
         * A value the library does not model reads as unknown rather than as
         * the first member.
         */
        {
            "deployment status=bogus gate=bogus simulation=off" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            {
                .startTime = 0,
                .status = RBRGEN4_UNKNOWN_DEPLOYMENT_STATUS,
                .gate = RBRGEN4_UNKNOWN_DEPLOYMENT_GATE,
                .simulation = false,
            },
        },
        /* An unreported parameter is left unknown, not zero. */
        {
            "deployment" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            {
                .startTime = 0,
                .status = RBRGEN4_UNKNOWN_DEPLOYMENT_STATUS,
                .gate = RBRGEN4_UNKNOWN_DEPLOYMENT_GATE,
                .simulation = false,
            },
        },
        /* Keys the library does not model are ignored. */
        {
            "deployment status=inactive gate=none simulation=off bogus=1" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            {
                .startTime = 0,
                .status = RBRGEN4_DEPLOYMENT_STATUS_INACTIVE,
                .gate = RBRGEN4_DEPLOYMENT_GATE_NONE,
                .simulation = false,
            },
        },
        {0},
    };

    RBRGen4Error err;
    RBRGen4Deployment actual;

    for (int i = 0; tests[i].response != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4_getDeployment(conn, &actual);
        TEST_ASSERT_STR_EQ("deployment" COMMAND_TERMINATOR, buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen4Error);
        TEST_ASSERT_EQ(tests[i].expected.startTime, actual.startTime, "%" PRIi64);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.status, actual.status, RBRGen4DeploymentStatus);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.gate, actual.gate, RBRGen4DeploymentGate);
        TEST_ASSERT_EQ(tests[i].expected.simulation, actual.simulation, "%d");
    }

    return true;
}

typedef struct SetDeploymentTest {
    RBRGen4Deployment deployment;
    const char *command;
    const char *response;
    RBRGen4Error expectedError;
} SetDeploymentTest;

TEST_LOGGER4(setDeployment)
{
    SetDeploymentTest tests[] = {
        /*
         * Without time gating only `gate` is sent: the instrument answers
         * `ERR-108` for `starttime` under any other condition.
         */
        {
            {
                .startTime = 0,
                .status = RBRGEN4_DEPLOYMENT_STATUS_INACTIVE,
                .gate = RBRGEN4_DEPLOYMENT_GATE_NONE,
                .simulation = false,
            },
            "deployment gate=none" COMMAND_TERMINATOR,
            "deployment gate=none" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
        },
        /* A start time left over from a previous read is not sent either. */
        {
            {
                .startTime = 1798761600000LL,
                .status = RBRGEN4_DEPLOYMENT_STATUS_INACTIVE,
                .gate = RBRGEN4_DEPLOYMENT_GATE_WETSWITCH,
                .simulation = false,
            },
            "deployment gate=wetswitch" COMMAND_TERMINATOR,
            "deployment gate=wetswitch" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
        },
        {
            {
                .startTime = 0,
                .status = RBRGEN4_DEPLOYMENT_STATUS_INACTIVE,
                .gate = RBRGEN4_DEPLOYMENT_GATE_TWISTACTIVATION,
                .simulation = false,
            },
            "deployment gate=twistactivation" COMMAND_TERMINATOR,
            "deployment gate=twistactivation" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
        },
        /* Time gating takes both parameters in one command. */
        {
            {
                .startTime = 1798761600000LL,
                .status = RBRGEN4_DEPLOYMENT_STATUS_INACTIVE,
                .gate = RBRGEN4_DEPLOYMENT_GATE_TIME,
                .simulation = false,
            },
            "deployment gate=time starttime=20270101000000" COMMAND_TERMINATOR,
            "deployment gate=time starttime=20270101000000" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
        },
        /*
         * A start time is range-checked only when it is going to be sent, so
         * an out-of-range value under time gating fails and the same value
         * under any other condition does not.
         */
        {
            {
                .startTime = RBRGEN4_DATETIME_MIN - 1,
                .status = RBRGEN4_DEPLOYMENT_STATUS_INACTIVE,
                .gate = RBRGEN4_DEPLOYMENT_GATE_TIME,
                .simulation = false,
            },
            "",
            "",
            RBRGEN4_INVALID_PARAMETER_VALUE,
        },
        {
            {
                .startTime = RBRGEN4_DATETIME_MAX + 1,
                .status = RBRGEN4_DEPLOYMENT_STATUS_INACTIVE,
                .gate = RBRGEN4_DEPLOYMENT_GATE_TIME,
                .simulation = false,
            },
            "",
            "",
            RBRGEN4_INVALID_PARAMETER_VALUE,
        },
        {
            {
                .startTime = RBRGEN4_DATETIME_MAX + 1,
                .status = RBRGEN4_DEPLOYMENT_STATUS_INACTIVE,
                .gate = RBRGEN4_DEPLOYMENT_GATE_NONE,
                .simulation = false,
            },
            "deployment gate=none" COMMAND_TERMINATOR,
            "deployment gate=none" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
        },
        /* The sentinels a getter can leave behind never reach the command. */
        {
            {
                .startTime = 0,
                .status = RBRGEN4_DEPLOYMENT_STATUS_INACTIVE,
                .gate = RBRGEN4_UNKNOWN_DEPLOYMENT_GATE,
                .simulation = false,
            },
            "",
            "",
            RBRGEN4_INVALID_PARAMETER_VALUE,
        },
        {
            {
                .startTime = 0,
                .status = RBRGEN4_DEPLOYMENT_STATUS_INACTIVE,
                .gate = RBRGEN4_DEPLOYMENT_GATE_COUNT,
                .simulation = false,
            },
            "",
            "",
            RBRGEN4_INVALID_PARAMETER_VALUE,
        },
        /* Both writable parameters are unavailable while logging. */
        {
            {
                .startTime = 0,
                .status = RBRGEN4_DEPLOYMENT_STATUS_SAMPLING,
                .gate = RBRGEN4_DEPLOYMENT_GATE_NONE,
                .simulation = false,
            },
            "deployment gate=none" COMMAND_TERMINATOR,
            "ERR-105 command prohibited while logging" RESPONSE_TERMINATOR,
            RBRGEN4_HARDWARE_ERROR,
        },
        /* A gating condition the instrument does not offer is refused. */
        {
            {
                .startTime = 0,
                .status = RBRGEN4_DEPLOYMENT_STATUS_INACTIVE,
                .gate = RBRGEN4_DEPLOYMENT_GATE_WETSWITCH,
                .simulation = false,
            },
            "deployment gate=wetswitch" COMMAND_TERMINATOR,
            "ERR-108 invalid argument to command: 'wetswitch'" RESPONSE_TERMINATOR,
            RBRGEN4_HARDWARE_ERROR,
        },
        {
            {0},
            NULL,
            NULL,
            0,
        },
    };

    RBRGen4Error err;

    for (int i = 0; tests[i].command != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4_setDeployment(conn, &tests[i].deployment);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }

    return true;
}

typedef struct VerifyTest {
    const RBRGen4Config config;
    const char *datasetLabel;
    RBRGen4DeploymentStorageMode storageMode;
    const char *command;
    const char *response;
    RBRGen4Error expectedError;
    RBRGen4InstrumentState expectedState;
} VerifyTest;

TEST_LOGGER4(verify)
{
    VerifyTest tests[] = {
        {
            {
                .label = "c_test",
            },
            "d1",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "verify config=c_test dataset=d1 storagemode=normal" COMMAND_TERMINATOR,
            "verify config=c_test dataset=d1 storagemode=normal state=enabled" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            RBRGEN4_INSTRUMENT_STATE_ENABLED,
        },
        {
            {
                .label = "pH_cal",
            },
            "d_pHcal_20260824",
            RBRGEN4_STORAGE_MODE_CALIBRATION,
            "verify config=pH_cal dataset=d_pHcal_20260824 "
            "storagemode=calibration" COMMAND_TERMINATOR,
            "verify config=pH_cal dataset=d_pHcal_20260824 "
            "storagemode=calibration state=enabled" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            RBRGEN4_INSTRUMENT_STATE_ENABLED,
        },
        /* A `NULL` dataset label leaves the parameter out. */
        {
            {
                .label = "c_test",
            },
            NULL,
            RBRGEN4_STORAGE_MODE_NORMAL,
            "verify config=c_test storagemode=normal" COMMAND_TERMINATOR,
            "verify config=c_test storagemode=normal state=enabled" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            RBRGEN4_INSTRUMENT_STATE_ENABLED,
        },
        /*
         * The response echoes the parameters in whatever order they were
         * sent, and `state` need not come last.
         */
        {
            {
                .label = "c_test",
            },
            "d1",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "verify config=c_test dataset=d1 storagemode=normal" COMMAND_TERMINATOR,
            "verify state=disabled dataset=d1 config=c_test storagemode=normal" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            RBRGEN4_INSTRUMENT_STATE_DISABLED,
        },
        /* A state the library does not model reads as unknown. */
        {
            {
                .label = "c_test",
            },
            "d1",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "verify config=c_test dataset=d1 storagemode=normal" COMMAND_TERMINATOR,
            "verify config=c_test dataset=d1 state=bogus" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        /* A response with no state at all leaves it unknown. */
        {
            {
                .label = "c_test",
            },
            "d1",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "verify config=c_test dataset=d1 storagemode=normal" COMMAND_TERMINATOR,
            "verify config=c_test dataset=d1" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        /* Out-of-range parameters never reach the instrument. */
        {
            {
                .label = "",
            },
            "d1",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "",
            "",
            RBRGEN4_INVALID_PARAMETER_VALUE,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        {
            {
                .label = "c_test",
            },
            "",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "",
            "",
            RBRGEN4_INVALID_PARAMETER_VALUE,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        /* The longest label the field holds is sent. */
        {
            {
                .label = "c_test",
            },
            "0123456789012345678901234567890",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "verify config=c_test dataset=0123456789012345678901234567890 "
            "storagemode=normal" COMMAND_TERMINATOR,
            "verify config=c_test dataset=0123456789012345678901234567890 storagemode=normal "
            "state=enabled" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            RBRGEN4_INSTRUMENT_STATE_ENABLED,
        },
        {
            {
                .label = "c_test",
            },
            "0123456789012345678901234567890123",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "",
            "",
            RBRGEN4_INVALID_PARAMETER_VALUE,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        {
            {
                .label = "c_test",
            },
            "d1",
            RBRGEN4_UNKNOWN_STORAGE_MODE,
            "",
            "",
            RBRGEN4_INVALID_PARAMETER_VALUE,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        {
            {
                .label = "c_test",
            },
            "d1",
            RBRGEN4_STORAGE_MODE_COUNT,
            "",
            "",
            RBRGEN4_INVALID_PARAMETER_VALUE,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        /* Every failing check the instrument makes. */
        {
            {
                .label = "nope",
            },
            "d1",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "verify config=nope dataset=d1 storagemode=normal" COMMAND_TERMINATOR,
            "ERR-108 invalid argument to command: 'nope'" RESPONSE_TERMINATOR,
            RBRGEN4_HARDWARE_ERROR,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        {
            {
                .label = "c_test",
            },
            "d.1",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "verify config=c_test dataset=d.1 storagemode=normal" COMMAND_TERMINATOR,
            "ERR-131 illegal character in label 'd.1'" RESPONSE_TERMINATOR,
            RBRGEN4_HARDWARE_ERROR,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        {
            {
                .label = "c_test",
            },
            "d1",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "verify config=c_test dataset=d1 storagemode=normal" COMMAND_TERMINATOR,
            "ERR-120 'd1' is already in use" RESPONSE_TERMINATOR,
            RBRGEN4_HARDWARE_ERROR,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        {
            {
                .label = "c_test",
            },
            "d5",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "verify config=c_test dataset=d5 storagemode=normal" COMMAND_TERMINATOR,
            "ERR-431 dataset limit of '4' reached, delete dataset(s) to make "
            "space" RESPONSE_TERMINATOR,
            RBRGEN4_HARDWARE_ERROR,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        {
            {
                .label = "c_test",
            },
            "d2",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "verify config=c_test dataset=d2 storagemode=normal" COMMAND_TERMINATOR,
            "ERR-436 instrument was already enabled with different settings" RESPONSE_TERMINATOR,
            RBRGEN4_HARDWARE_ERROR,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        {
            {
                .label = "",
            },
            NULL,
            0,
            NULL,
            NULL,
            0,
            0,
        },
    };

    RBRGen4Error err;
    RBRGen4InstrumentState actual;

    for (int i = 0; tests[i].command != NULL; i++) {
        actual = RBRGEN4_UNKNOWN_INSTRUMENT_STATE;
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4_verify(
            conn, &tests[i].config, tests[i].datasetLabel, tests[i].storageMode, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedState, actual, RBRGen4InstrumentState);
    }

    return true;
}

typedef struct EnableTest {
    const RBRGen4Config config;
    const char *datasetLabel;
    RBRGen4DeploymentStorageMode storageMode;
    const char *command;
    const char *response;
    RBRGen4Error expectedError;
    RBRGen4InstrumentState expectedState;
} EnableTest;

TEST_LOGGER4(enable)
{
    EnableTest tests[] = {
        {
            {
                .label = "c_test",
            },
            "d1",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "enable config=c_test dataset=d1 storagemode=normal" COMMAND_TERMINATOR,
            "enable config=c_test dataset=d1 storagemode=normal state=enabled" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            RBRGEN4_INSTRUMENT_STATE_ENABLED,
        },
        {
            {
                .label = "pH_cal",
            },
            "d_pHcal_20260824",
            RBRGEN4_STORAGE_MODE_CALIBRATION,
            "enable config=pH_cal dataset=d_pHcal_20260824 "
            "storagemode=calibration" COMMAND_TERMINATOR,
            "enable config=pH_cal dataset=d_pHcal_20260824 "
            "storagemode=calibration state=enabled" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            RBRGEN4_INSTRUMENT_STATE_ENABLED,
        },
        /* A `NULL` dataset label leaves the parameter out. */
        {
            {
                .label = "c_test",
            },
            NULL,
            RBRGEN4_STORAGE_MODE_NORMAL,
            "enable config=c_test storagemode=normal" COMMAND_TERMINATOR,
            "enable config=c_test storagemode=normal state=enabled" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            RBRGEN4_INSTRUMENT_STATE_ENABLED,
        },
        /* Out-of-range parameters never reach the instrument. */
        {
            {
                .label = "",
            },
            "d1",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "",
            "",
            RBRGEN4_INVALID_PARAMETER_VALUE,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        {
            {
                .label = "c_test",
            },
            "",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "",
            "",
            RBRGEN4_INVALID_PARAMETER_VALUE,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        /* The longest label the field holds is sent. */
        {
            {
                .label = "c_test",
            },
            "0123456789012345678901234567890",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "enable config=c_test dataset=0123456789012345678901234567890 "
            "storagemode=normal" COMMAND_TERMINATOR,
            "enable config=c_test dataset=0123456789012345678901234567890 storagemode=normal "
            "state=enabled" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            RBRGEN4_INSTRUMENT_STATE_ENABLED,
        },
        /*
         * A label one character past the field is refused rather than
         * truncated into the instrument's 32-byte field.
         */
        {
            {
                .label = "c_test",
            },
            "0123456789012345678901234567890123",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "",
            "",
            RBRGEN4_INVALID_PARAMETER_VALUE,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        {
            {
                .label = "c_test",
            },
            "d1",
            RBRGEN4_UNKNOWN_STORAGE_MODE,
            "",
            "",
            RBRGEN4_INVALID_PARAMETER_VALUE,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        /* Every failing check the instrument makes. */
        {
            {
                .label = "c_test",
            },
            "d1",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "enable config=c_test dataset=d1 storagemode=normal" COMMAND_TERMINATOR,
            "ERR-408 instrument was already enabled" RESPONSE_TERMINATOR,
            RBRGEN4_HARDWARE_ERROR,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        {
            {
                .label = "c_test",
            },
            "d2",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "enable config=c_test dataset=d2 storagemode=normal" COMMAND_TERMINATOR,
            "ERR-436 instrument was already enabled with different settings" RESPONSE_TERMINATOR,
            RBRGEN4_HARDWARE_ERROR,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        {
            {
                .label = "c_test",
            },
            "d1",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "enable config=c_test dataset=d1 storagemode=normal" COMMAND_TERMINATOR,
            "ERR-120 'd1' is already in use" RESPONSE_TERMINATOR,
            RBRGEN4_HARDWARE_ERROR,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        {
            {
                .label = "c_test",
            },
            "d5",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "enable config=c_test dataset=d5 storagemode=normal" COMMAND_TERMINATOR,
            "ERR-431 dataset limit of '4' reached, delete dataset(s) to make "
            "space" RESPONSE_TERMINATOR,
            RBRGEN4_HARDWARE_ERROR,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        {
            {
                .label = "c_empty",
            },
            "d1",
            RBRGEN4_STORAGE_MODE_NORMAL,
            "enable config=c_empty dataset=d1 storagemode=normal" COMMAND_TERMINATOR,
            "ERR-430 empty schedule list in configuration 'c_empty'" RESPONSE_TERMINATOR,
            RBRGEN4_HARDWARE_ERROR,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        {
            {
                .label = "",
            },
            NULL,
            0,
            NULL,
            NULL,
            0,
            0,
        },
    };

    RBRGen4Error err;
    RBRGen4InstrumentState actual;

    for (int i = 0; tests[i].command != NULL; i++) {
        actual = RBRGEN4_UNKNOWN_INSTRUMENT_STATE;
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4_enable(
            conn, &tests[i].config, tests[i].datasetLabel, tests[i].storageMode, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedState, actual, RBRGen4InstrumentState);
    }

    return true;
}

typedef struct DisableTest {
    const char *response;
    RBRGen4Error expectedError;
    RBRGen4ResponseType expectedType;
    RBRGen4HardwareError expectedHardwareError;
    RBRGen4InstrumentState expectedState;
} DisableTest;

TEST_LOGGER4(disable)
{
    DisableTest tests[] = {
        /* The command reports an instrument state, not a deployment status. */
        {
            "disable state=disabled" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            RBRGEN4_RESPONSE_INFO,
            RBRGEN4_HARDWARE_ERROR_NONE,
            RBRGEN4_INSTRUMENT_STATE_DISABLED,
        },
        /*
         * Disabling an instrument that is already disabled is a warning, which
         * the library surfaces as a hardware error with the response type
         * distinguishing it.
         */
        {
            "WRN-435 instrument state is already disabled" RESPONSE_TERMINATOR,
            RBRGEN4_HARDWARE_ERROR,
            RBRGEN4_RESPONSE_WARNING,
            RBRGEN4_HARDWARE_ERROR_INSTRUMENT_STATE_IS_ALREADY_DISABLED,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        /* A state the library does not model reads as unknown. */
        {
            "disable state=bogus" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            RBRGEN4_RESPONSE_INFO,
            RBRGEN4_HARDWARE_ERROR_NONE,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        {
            "disable" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            RBRGEN4_RESPONSE_INFO,
            RBRGEN4_HARDWARE_ERROR_NONE,
            RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
        },
        {0},
    };

    RBRGen4Error err;
    RBRGen4InstrumentState actual;

    for (int i = 0; tests[i].response != NULL; i++) {
        actual = RBRGEN4_UNKNOWN_INSTRUMENT_STATE;
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4_disable(conn, &actual);
        TEST_ASSERT_STR_EQ("disable" COMMAND_TERMINATOR, buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen4Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedType, conn->response.type, RBRGen4ResponseType);
        TEST_ASSERT_ENUM_EQ(
            tests[i].expectedHardwareError, conn->response.error, RBRGen4HardwareError);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedState, actual, RBRGen4InstrumentState);
    }

    return true;
}

typedef struct PauseResumeTest {
    const char *response;
    RBRGen4Error expectedError;
    RBRGen4DeploymentStatus expectedStatus;
} PauseResumeTest;

static bool test_pauseResume(RBRGen4 *conn, TestIOBuffers *buffers, const char *command,
                             RBRGen4Error (*call)(RBRGen4 *, RBRGen4DeploymentStatus *),
                             PauseResumeTest *tests)
{
    RBRGen4Error err;
    RBRGen4DeploymentStatus actual;

    for (int i = 0; tests[i].response != NULL; i++) {
        actual = RBRGEN4_UNKNOWN_DEPLOYMENT_STATUS;
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = call(conn, &actual);
        TEST_ASSERT_STR_EQ(command, buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen4Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedStatus, actual, RBRGen4DeploymentStatus);
    }

    return true;
}

TEST_LOGGER4(pause)
{
    PauseResumeTest tests[] = {
        {
            "pause status=paused" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            RBRGEN4_DEPLOYMENT_STATUS_PAUSED,
        },
        /*
         * Pausing a deployment that is waiting on its gating condition
         * succeeds and leaves it gated, not paused.
         */
        {
            "pause status=gated" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            RBRGEN4_DEPLOYMENT_STATUS_GATED,
        },
        {
            "ERR-406 cannot pause while disabled" RESPONSE_TERMINATOR,
            RBRGEN4_HARDWARE_ERROR,
            RBRGEN4_UNKNOWN_DEPLOYMENT_STATUS,
        },
        {
            "pause status=bogus" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            RBRGEN4_UNKNOWN_DEPLOYMENT_STATUS,
        },
        {"pause" RESPONSE_TERMINATOR, RBRGEN4_SUCCESS, RBRGEN4_UNKNOWN_DEPLOYMENT_STATUS},
        {0},
    };

    return test_pauseResume(conn, buffers, "pause" COMMAND_TERMINATOR, RBRGen4_pause, tests);
}

TEST_LOGGER4(resume)
{
    PauseResumeTest tests[] = {
        {
            "resume status=sampling" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            RBRGEN4_DEPLOYMENT_STATUS_SAMPLING,
        },
        {
            "resume status=gated" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            RBRGEN4_DEPLOYMENT_STATUS_GATED,
        },
        {
            "ERR-407 cannot resume while disabled" RESPONSE_TERMINATOR,
            RBRGEN4_HARDWARE_ERROR,
            RBRGEN4_UNKNOWN_DEPLOYMENT_STATUS,
        },
        {
            "resume status=bogus" RESPONSE_TERMINATOR,
            RBRGEN4_SUCCESS,
            RBRGEN4_UNKNOWN_DEPLOYMENT_STATUS,
        },
        {"resume" RESPONSE_TERMINATOR, RBRGEN4_SUCCESS, RBRGEN4_UNKNOWN_DEPLOYMENT_STATUS},
        {0},
    };

    return test_pauseResume(conn, buffers, "resume" COMMAND_TERMINATOR, RBRGen4_resume, tests);
}
