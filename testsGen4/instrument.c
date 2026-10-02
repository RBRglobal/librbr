/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file instrument.c
 *
 * \brief Tests for instrument commands.
 */

/* Required for NAN. */
#include <math.h>

#include "tests.h"
#include "RBRGen4Instrument.h"

typedef struct IdTest {
    const char *response;
    RBRGen4Error expectedError;
    RBRGen4Id4 expected;
} IdTest;

TEST_LOGGER4(id4)
{
    IdTest tests[] = {
        {
            .response = "id4 model=L4 "
                        "sn=999999 "
                        "fwversion=2.0.0 "
                        "semver=2.0.0-rc3-14-g5e07a2c91 "
                        "fwtype=150 "
                        "apiversion=2.1" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .model = "L4",
                    .fwVersion = "2.0.0",
                    .semver = "2.0.0-rc3-14-g5e07a2c91",
                    .apiVersion = "2.1",
                    .sn = 999999,
                    .fwType = 150,
                },
        },
        /* A response without the parameter leaves it empty. */
        {
            .response = "id4 model=L4 "
                        "sn=999999 "
                        "fwversion=2.0.0 "
                        "semver=2.0.0-rc3-14-g5e07a2c91 "
                        "fwtype=150" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .model = "L4",
                    .fwVersion = "2.0.0",
                    .semver = "2.0.0-rc3-14-g5e07a2c91",
                    .apiVersion = "",
                    .sn = 999999,
                    .fwType = 150,
                },
        },
        /* A response carrying no parameters leaves the struct zeroed. */
        {
            .response = "id4" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .model = "",
                    .fwVersion = "",
                    .semver = "",
                    .apiVersion = "",
                    .sn = 0,
                    .fwType = 0,
                },
        },
        {0},
    };

    RBRGen4Error err;
    RBRGen4Id4 actual;

    for (int i = 0; tests[i].response != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4_getId4(conn, &actual);
        TEST_ASSERT_STR_EQ("id4" COMMAND_TERMINATOR, buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].expected.model, actual.model);
        TEST_ASSERT_STR_EQ(tests[i].expected.fwVersion, actual.fwVersion);
        TEST_ASSERT_STR_EQ(tests[i].expected.semver, actual.semver);
        TEST_ASSERT_STR_EQ(tests[i].expected.apiVersion, actual.apiVersion);
        TEST_ASSERT_EQ(tests[i].expected.sn, actual.sn, "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.fwType, actual.fwType, "%" PRIi32);
    }
    return true;
}

typedef struct PowerTest {
    const char *response;
    RBRGen4Error expectedError;
    RBRGen4PowerSource expected;
} PowerTest;

TEST_LOGGER4(power)
{
    PowerTest tests[] = {
        {
            .response = "instrument power source=usb" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .expected = RBRGEN4_POWER_SOURCE_USB,
        },
        {
            .response = "instrument power source=ext" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .expected = RBRGEN4_POWER_SOURCE_EXTERNAL,
        },
        {
            .response = "instrument power source=int" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .expected = RBRGEN4_POWER_SOURCE_INTERNAL,
        },
        {0},
    };

    RBRGen4Error err;
    RBRGen4PowerSource actual;

    for (int i = 0; tests[i].response != NULL; i++) {

        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4_getPowerSource(conn, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen4Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected, actual, RBRGen4PowerSource);
    }

    return true;
}

TEST_LOGGER4(powerinternal)
{
    RBRGen4PowerInternal expected = {
        .voltage = 14.21,
        .batteryType = RBRGEN4_INTERNAL_BATTERY_NIMH,
        .used = 100100,
    };
    RBRGen4PowerInternal actual;

    TestIOBuffers_init(buffers,
                       "instrument power internal voltage=14.21 batterytype=nimh "
                       "used=100.100e+003" RESPONSE_TERMINATOR,
                       0);
    RBRGen4Error err = RBRGen4_getPowerInternal(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_FLOAT_EQ(expected.voltage, actual.voltage, 0.001f);
    TEST_ASSERT_ENUM_EQ(expected.batteryType, actual.batteryType, RBRGen4InternalBatteryType);
    TEST_ASSERT_FLOAT_EQ(expected.used, actual.used, 0.01f);

    return true;
}

typedef struct PowerInternalBatteryTypeTest {
    const char *command;
    const char *response;
    RBRGen4Error expectedError;
    RBRGen4InternalBatteryType batteryType;
} PowerInternalBatteryTypeTest;

TEST_LOGGER4(setPowerInternalBatteryType)
{
    PowerInternalBatteryTypeTest tests[] = {
        {
            .command = "instrument power internal batterytype=lisocl2" COMMAND_TERMINATOR,
            .response = "instrument power internal batterytype=lisocl2" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .batteryType = RBRGEN4_INTERNAL_BATTERY_LISOCL2,
        },
        {
            .command = "instrument power internal batterytype=lifes2" COMMAND_TERMINATOR,
            .response = "instrument power internal batterytype=lifes2" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .batteryType = RBRGEN4_INTERNAL_BATTERY_LIFES2,
        },
        {
            .command = "instrument power internal batterytype=znmno2" COMMAND_TERMINATOR,
            .response = "instrument power internal batterytype=znmno2" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .batteryType = RBRGEN4_INTERNAL_BATTERY_ZNMNO2,
        },
        {
            .command = "instrument power internal batterytype=linimnco" COMMAND_TERMINATOR,
            .response = "instrument power internal batterytype=linimnco" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .batteryType = RBRGEN4_INTERNAL_BATTERY_LINIMNCO,
        },
        {
            .command = "instrument power internal batterytype=nimh" COMMAND_TERMINATOR,
            .response = "instrument power internal batterytype=nimh" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .batteryType = RBRGEN4_INTERNAL_BATTERY_NIMH,
        },
        {
            .command = "instrument power internal batterytype=none" COMMAND_TERMINATOR,
            .response = "instrument power internal batterytype=none" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .batteryType = RBRGEN4_INTERNAL_BATTERY_NONE,
        },
        {0},
    };

    RBRGen4Error err;
    for (int i = 0; tests[i].command != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4_setPowerInternalBatteryType(conn, tests[i].batteryType);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }
    return true;
}

TEST_LOGGER4(resetPowerInternalUsed)
{
    TestIOBuffers_init(buffers, "instrument power internal used=0.000e+000" RESPONSE_TERMINATOR, 0);
    RBRGen4Error err = RBRGen4_resetPowerInternalUsed(conn);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("instrument power internal used=0" COMMAND_TERMINATOR, buffers->writeBuffer);
    return true;
}

TEST_LOGGER4(powerexternal)
{
    RBRGen4PowerExternal expected = {
        .voltage = 14.21,
        .batteryType = RBRGEN4_EXTERNAL_BATTERY_FERMATA_LISOCL2,
        .used = 100100,
    };
    RBRGen4PowerExternal actual;

    TestIOBuffers_init(buffers,
                       "instrument power external voltage=14.21 "
                       "batterytype=fermata_lisocl2 used=100.100e+003" RESPONSE_TERMINATOR,
                       0);
    RBRGen4Error err = RBRGen4_getPowerExternal(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_FLOAT_EQ(expected.voltage, actual.voltage, 0.001f);
    TEST_ASSERT_ENUM_EQ(expected.batteryType, actual.batteryType, RBRGen4ExternalBatteryType);
    TEST_ASSERT_FLOAT_EQ(expected.used, actual.used, 0.01f);

    return true;
}

typedef struct PowerExternalBatteryTypeTest {
    const char *command;
    const char *response;
    RBRGen4Error expectedError;
    RBRGen4ExternalBatteryType batteryType;
} PowerExternalBatteryTypeTest;

TEST_LOGGER4(setPowerExternalBatteryType)
{
    PowerExternalBatteryTypeTest tests[] = {
        {
            .command = "instrument power external batterytype=fermata_lisocl2" COMMAND_TERMINATOR,
            .response = "instrument power external batterytype=fermata_lisocl2" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .batteryType = RBRGEN4_EXTERNAL_BATTERY_FERMATA_LISOCL2,
        },
        {
            .command = "instrument power external batterytype=fermata_znmno2" COMMAND_TERMINATOR,
            .response = "instrument power external batterytype=fermata_znmno2" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .batteryType = RBRGEN4_EXTERNAL_BATTERY_FERMATA_ZNMNO2,
        },
        {
            .command = "instrument power external batterytype=fermette_limno2" COMMAND_TERMINATOR,
            .response = "instrument power external batterytype=fermette_limno2" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .batteryType = RBRGEN4_EXTERNAL_BATTERY_FERMETTE_LIMNO2,
        },
        {
            .command = "instrument power external batterytype=fermette3_lisocl2" COMMAND_TERMINATOR,
            .response =
                "instrument power external batterytype=fermette3_lisocl2" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .batteryType = RBRGEN4_EXTERNAL_BATTERY_FERMETTE3_LISOCL2,
        },
        {
            .command = "instrument power external batterytype=fermette3_lifes2" COMMAND_TERMINATOR,
            .response =
                "instrument power external batterytype=fermette3_lifes2" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .batteryType = RBRGEN4_EXTERNAL_BATTERY_FERMETTE3_LIFES2,
        },
        {
            .command = "instrument power external batterytype=fermette3_znmno2" COMMAND_TERMINATOR,
            .response =
                "instrument power external batterytype=fermette3_znmno2" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .batteryType = RBRGEN4_EXTERNAL_BATTERY_FERMETTE3_ZNMNO2,
        },
        {
            .command =
                "instrument power external batterytype=fermette3_linimnco" COMMAND_TERMINATOR,
            .response =
                "instrument power external batterytype=fermette3_linimnco" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .batteryType = RBRGEN4_EXTERNAL_BATTERY_FERMETTE3_LINIMNCO,
        },
        {
            .command = "instrument power external batterytype=fermette3_nimh" COMMAND_TERMINATOR,
            .response = "instrument power external batterytype=fermette3_nimh" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .batteryType = RBRGEN4_EXTERNAL_BATTERY_FERMETTE3_NIMH,
        },
        {
            .command = "instrument power external batterytype=fermata_nimh" COMMAND_TERMINATOR,
            .response = "instrument power external batterytype=fermata_nimh" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .batteryType = RBRGEN4_EXTERNAL_BATTERY_FERMATA_NIMH,
        },
        {
            .command = "instrument power external batterytype=other" COMMAND_TERMINATOR,
            .response = "instrument power external batterytype=other" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .batteryType = RBRGEN4_EXTERNAL_BATTERY_OTHER,
        },
        {
            .command = "instrument power external batterytype=none" COMMAND_TERMINATOR,
            .response = "instrument power external batterytype=none" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .batteryType = RBRGEN4_EXTERNAL_BATTERY_NONE,
        },
        {0},
    };

    RBRGen4Error err;
    for (int i = 0; tests[i].command != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4_setPowerExternalBatteryType(conn, tests[i].batteryType);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }
    return true;
}

TEST_LOGGER4(resetPowerExternalUsed)
{
    TestIOBuffers_init(buffers, "instrument power external used=0.000e+000" RESPONSE_TERMINATOR, 0);
    RBRGen4Error err = RBRGen4_resetPowerExternalUsed(conn);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("instrument power external used=0" COMMAND_TERMINATOR, buffers->writeBuffer);
    return true;
}

typedef struct InstrumentCommandTest {
    const char *response;
    RBRGen4Error expectedError;
    RBRGen4Instrument expected;
} InstrumentCommandTest;

TEST_LOGGER4(conn)
{
    InstrumentCommandTest tests[] = {
        {
            .response = "instrument state=disabled sn=999999 model=L4 pn=9999999revA "
                        "fwversion=2.0.0 semver=2.0.0-rc3-14-g5e07a2c91 fwtype=150 "
                        "fwlock=off datatype=float64 name=L4 apiversion=2.1" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .state = RBRGEN4_INSTRUMENT_STATE_DISABLED,
                    .sn = 999999,
                    .model = "L4",
                    .pn = "9999999revA",
                    .fwVersion = "2.0.0",
                    .semver = "2.0.0-rc3-14-g5e07a2c91",
                    .fwType = 150,
                    .fwLock = false,
                    .dataType = RBRGEN4_DATA_TYPE_FLOAT64,
                    .name = "L4",
                    .apiVersion = "2.1",
                },
        },
        /* An enabled instrument with the firmware locked, and the extended
         * name and part number populated. */
        {
            .response = "instrument state=enabled sn=210000 model=RBRsolo4 "
                        "pn=L3-M11-BEC11-SC11-ST11-SP11 fwversion=1.0.0 "
                        "semver=1.0.0-rc2-8-gb36d0f4 fwtype=130 fwlock=on "
                        "datatype=float32 name=RBRsolo^4_T.D!fast32" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .state = RBRGEN4_INSTRUMENT_STATE_ENABLED,
                    .sn = 210000,
                    .model = "RBRsolo4",
                    .pn = "L3-M11-BEC11-SC11-ST11-SP11",
                    .fwVersion = "1.0.0",
                    .semver = "1.0.0-rc2-8-gb36d0f4",
                    .fwType = 130,
                    .fwLock = true,
                    .dataType = RBRGEN4_DATA_TYPE_FLOAT32,
                    .name = "RBRsolo^4_T.D!fast32",
                },
        },
        /* calfloat64 is reported only during a calibration-mode deployment. */
        {
            .response = "instrument state=enabled sn=210000 model=RBRsolo4 pn=012345revA "
                        "fwversion=1.0.0 semver=1.0.0 fwtype=130 fwlock=off "
                        "datatype=calfloat64 name=RBRsolo4" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .state = RBRGEN4_INSTRUMENT_STATE_ENABLED,
                    .sn = 210000,
                    .model = "RBRsolo4",
                    .pn = "012345revA",
                    .fwVersion = "1.0.0",
                    .semver = "1.0.0",
                    .fwType = 130,
                    .fwLock = false,
                    .dataType = RBRGEN4_DATA_TYPE_CALFLOAT64,
                    .name = "RBRsolo4",
                },
        },
        /* An unrecognized data type must not be reported as float32, which is
         * the zero value of the enum. */
        {
            .response = "instrument state=disabled sn=999999 model=L4 pn=9999999revA "
                        "fwversion=2.0.0 semver=2.0.0 fwtype=150 fwlock=off "
                        "datatype=float128 name=L4" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .expected =
                {
                    .state = RBRGEN4_INSTRUMENT_STATE_DISABLED,
                    .sn = 999999,
                    .model = "L4",
                    .pn = "9999999revA",
                    .fwVersion = "2.0.0",
                    .semver = "2.0.0",
                    .fwType = 150,
                    .fwLock = false,
                    .dataType = RBRGEN4_UNKNOWN_DATA_TYPE,
                    .name = "L4",
                },
        },
        {0},
    };
    RBRGen4Error err;
    RBRGen4Instrument actual;

    for (int i = 0; tests[i].response != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4_getInstrument(conn, &actual);
        TEST_ASSERT_STR_EQ("instrument" COMMAND_TERMINATOR, buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen4Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.state, actual.state, RBRGen4InstrumentState);
        TEST_ASSERT_EQ(tests[i].expected.sn, actual.sn, "%" PRIi32);
        TEST_ASSERT_STR_EQ(tests[i].expected.model, actual.model);
        TEST_ASSERT_STR_EQ(tests[i].expected.pn, actual.pn);
        TEST_ASSERT_STR_EQ(tests[i].expected.fwVersion, actual.fwVersion);
        TEST_ASSERT_STR_EQ(tests[i].expected.semver, actual.semver);
        TEST_ASSERT_EQ(tests[i].expected.fwType, actual.fwType, "%" PRIi32);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.fwLock, actual.fwLock, bool);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.dataType, actual.dataType, RBRGen4DataType);
        TEST_ASSERT_STR_EQ(tests[i].expected.name, actual.name);
        TEST_ASSERT_STR_EQ(tests[i].expected.apiVersion, actual.apiVersion);
    }
    return true;
}

/* The connection's buffers are supplied by the caller, so a missing or empty
 * buffer has to be refused before any instrument communication happens, both
 * when the connection is opened and when the buffers are replaced later. */
TEST_LOGGER4(openRejectsInvalidBuffers)
{
    uint8_t commandBuffer[RBRGEN4_COMMAND_BUFFER_DEFAULT];
    uint8_t responseBuffer[RBRGEN4_RESPONSE_BUFFER_DEFAULT];

    struct {
        uint8_t *command;
        int32_t commandCapacity;
        uint8_t *response;
        int32_t responseCapacity;
    } tests[] = {
        {
            .command = NULL,
            .commandCapacity = sizeof(commandBuffer),
            .response = responseBuffer,
            .responseCapacity = sizeof(responseBuffer),
        },
        {
            .command = commandBuffer,
            .commandCapacity = 0,
            .response = responseBuffer,
            .responseCapacity = sizeof(responseBuffer),
        },
        {
            .command = commandBuffer,
            .commandCapacity = sizeof(commandBuffer),
            .response = NULL,
            .responseCapacity = sizeof(responseBuffer),
        },
        {
            .command = commandBuffer,
            .commandCapacity = sizeof(commandBuffer),
            .response = responseBuffer,
            .responseCapacity = 0,
        },
        {
            .command = commandBuffer,
            .commandCapacity = sizeof(commandBuffer),
            .response = responseBuffer,
            .responseCapacity = -1,
        },
        /* Room for a line terminator and nothing else can never hold a
         * response. */
        {
            .command = commandBuffer,
            .commandCapacity = sizeof(commandBuffer),
            .response = responseBuffer,
            .responseCapacity = 2,
        },
    };

    RBRGen4 unopened;
    RBRGen4Error err;

    TestIOBuffers_init(buffers, "", 0);
    err = RBRGen4_open(&unopened, NULL, 0, NULL);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_MISSING_CALLBACK, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    const RBRGen4Environment before = conn->environment;

    /* A sample callback whose sample has no readings storage is refused too. */
    RBRGen4Sample noStorage = {.size = 4, .readings = NULL};
    RBRGen4Environment noStorageEnvironment = conn->environment;
    noStorageEnvironment.sampleBuffer = &noStorage;
    TestIOBuffers_init(buffers, "", 0);
    err = RBRGen4_open(&unopened, &noStorageEnvironment, 0, NULL);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
        RBRGen4Environment environment = conn->environment;
        environment.command = tests[i].command;
        environment.commandCapacity = tests[i].commandCapacity;
        environment.response = tests[i].response;
        environment.responseCapacity = tests[i].responseCapacity;

        TestIOBuffers_init(buffers, "", 0);
        err = RBRGen4_open(&unopened, &environment, 0, NULL);
        TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);
        TEST_ASSERT_STR_EQ("", buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(
            RBRCOMMON_UNKNOWN_GENERATION, RBRGen4_getGeneration(&unopened), RBRCommonGeneration);

        /* A refused replacement leaves the connection on its current
         * buffer. Exactly one of the two buffers is bad in each case, and
         * only its setter is tried, so the good one never displaces the
         * shared connection's own. */
        if (tests[i].command == NULL || tests[i].commandCapacity <= 0) {
            err = RBRGen4_setCommandBuffer(conn, tests[i].command, tests[i].commandCapacity);
        } else {
            err = RBRGen4_setResponseBuffer(conn, tests[i].response, tests[i].responseCapacity);
        }
        TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);
        TEST_ASSERT(conn->environment.command == before.command);
        TEST_ASSERT(conn->environment.commandCapacity == before.commandCapacity);
        TEST_ASSERT(conn->environment.response == before.response);
        TEST_ASSERT(conn->environment.responseCapacity == before.responseCapacity);
    }

    return true;
}

/* An instrument of another generation answers `id4` with an error line, and a
 * device which is not an instrument answers with nothing useful; both are
 * reported as an unsupported instrument. A failing callback is the caller's
 * problem and is reported as itself. */
TEST_LOGGER4(openUnsupportedVersusCallbackError)
{
    RBRGen4 unopened;
    RBRGen4Error err;
    uint8_t commandBuffer[RBRGEN4_COMMAND_BUFFER_DEFAULT];
    uint8_t responseBuffer[RBRGEN4_RESPONSE_BUFFER_DEFAULT];
    RBRGen4Environment environment = conn->environment;
    environment.command = commandBuffer;
    environment.commandCapacity = sizeof(commandBuffer);
    environment.response = responseBuffer;
    environment.responseCapacity = sizeof(responseBuffer);

    TestIOBuffers_init(buffers, "ERR-102 invalid command 'id4'" RESPONSE_TERMINATOR, 0);
    err = RBRGen4_open(&unopened, &environment, 0, buffers);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_UNSUPPORTED, err, RBRGen4Error);
    TEST_ASSERT_ENUM_EQ(
        RBRCOMMON_UNKNOWN_GENERATION, RBRGen4_getGeneration(&unopened), RBRCommonGeneration);

    /* The harness reports a read past its scripted data as a callback
     * error. */
    TestIOBuffers_init(buffers, "", 0);
    err = RBRGen4_open(&unopened, &environment, 0, buffers);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_CALLBACK_ERROR, err, RBRGen4Error);

    return true;
}

/* Replacing the buffers carries nothing over from the old ones: the response
 * state is reset as by RBRGen4_resetResponseBuffer() when the response buffer
 * changes, and the next command goes out through the new command buffer. */
TEST_LOGGER4(setCommandAndResponseBuffers)
{
    RBRGen4Error err;
    RBRGen4PowerSource actual;
    const RBRGen4Environment before = conn->environment;
    uint8_t commandBuffer[RBRGEN4_COMMAND_BUFFER_DEFAULT];
    uint8_t responseBuffer[RBRGEN4_RESPONSE_BUFFER_DEFAULT];

    /* Leave a second response buffered behind a hardware error. */
    TestIOBuffers_init(buffers,
                       "ERR-108 invalid argument to command: 'bogus'" RESPONSE_TERMINATOR
                       "instrument power source=ext" RESPONSE_TERMINATOR,
                       0);
    err = RBRGen4_getPowerSource(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_HARDWARE_ERROR, err, RBRGen4Error);
    TEST_ASSERT(RBRGen4_getLastHardwareErrorMessage(conn) != NULL);

    /* Replacing the command buffer leaves the response state alone. */
    err = RBRGen4_setCommandBuffer(conn, commandBuffer, sizeof(commandBuffer));
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT(conn->environment.command == commandBuffer);
    TEST_ASSERT(conn->environment.commandCapacity == sizeof(commandBuffer));
    TEST_ASSERT_EQ(0, conn->commandBufferLength, "%" PRIi32);
    TEST_ASSERT(RBRGen4_getLastHardwareErrorMessage(conn) != NULL);

    err = RBRGen4_setResponseBuffer(conn, responseBuffer, sizeof(responseBuffer));
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT(conn->environment.response == responseBuffer);
    TEST_ASSERT(conn->environment.responseCapacity == sizeof(responseBuffer));
    TEST_ASSERT_EQ(0, conn->responseBufferLength, "%" PRIi32);
    TEST_ASSERT(RBRGen4_getLastHardwareErrorMessage(conn) == NULL);

    /* The buffered second response is gone with the old buffer, so the reply
     * the instrument sends now answers this command; it was built in the new
     * command buffer. */
    TestIOBuffers_init(buffers, "instrument power source=int" RESPONSE_TERMINATOR, 0);
    err = RBRGen4_getPowerSource(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_POWER_SOURCE_INTERNAL, actual, RBRGen4PowerSource);
    TEST_ASSERT(memcmp(commandBuffer, "instrument power", sizeof("instrument power") - 1) == 0);

    /* Put the shared connection back on its own buffers. */
    err = RBRGen4_setCommandBuffer(conn, before.command, before.commandCapacity);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    err = RBRGen4_setResponseBuffer(conn, before.response, before.responseCapacity);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);

    return true;
}

/* A response buffer handed between connections carries whatever the previous
 * one left in it, so the reset has to discard the leftovers rather than let
 * the next command parse them as its own. */
TEST_LOGGER4(resetResponseBuffer)
{
    RBRGen4Error err;
    RBRGen4PowerSource actual;

    /* Two responses arrive together: the error satisfies the command and
     * leaves a message pointing into the buffer; the second response stays
     * buffered behind it. */
    TestIOBuffers_init(buffers,
                       "ERR-108 invalid argument to command: 'bogus'" RESPONSE_TERMINATOR
                       "instrument power source=ext" RESPONSE_TERMINATOR,
                       0);
    err = RBRGen4_getPowerSource(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_HARDWARE_ERROR, err, RBRGen4Error);
    TEST_ASSERT(RBRGen4_getLastHardwareErrorMessage(conn) != NULL);

    RBRGen4_resetResponseBuffer(conn);

    /* The reset drops the message view along with the buffered data. */
    TEST_ASSERT_ENUM_EQ(
        RBRGEN4_HARDWARE_ERROR_NONE, RBRGen4_getLastHardwareError(conn), RBRGen4HardwareError);
    TEST_ASSERT(RBRGen4_getLastHardwareErrorMessage(conn) == NULL);

    /* Without the reset, the buffered second response would answer this
     * command instead of the one the instrument sends now. */
    TestIOBuffers_init(buffers, "instrument power source=int" RESPONSE_TERMINATOR, 0);
    err = RBRGen4_getPowerSource(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_POWER_SOURCE_INTERNAL, actual, RBRGen4PowerSource);

    return true;
}

/* A line longer than the buffer met while waiting for a reply is drained and
 * skipped; the reply behind it still answers the command. */
TEST_LOGGER4(responseBufferOverflow)
{
    RBRGen4Error err;
    RBRGen4PowerSource actual;

    /* One line longer than the buffer, followed by the reply. */
    static char response[RBRGEN4_RESPONSE_BUFFER_DEFAULT + 64];
    const char *rest = "instrument power source=int" RESPONSE_TERMINATOR;
    size_t overlong = sizeof(response) - strlen(RESPONSE_TERMINATOR) - strlen(rest) - 1;
    memset(response, 'a', overlong);
    strcpy(response + overlong, RESPONSE_TERMINATOR);
    strcat(response, rest);

    TestIOBuffers_init(buffers, response, 0);
    err = RBRGen4_getPowerSource(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_POWER_SOURCE_INTERNAL, actual, RBRGen4PowerSource);

    return true;
}

/** \brief A clock which stands still until the fixture has been read, then
 * steps past the command timeout on every call. */
static RBRGen4Error overflowReplyTime(const RBRGen4 *conn, RBRGen4DateTime *time)
{
    TestIOBuffers *buffers = (TestIOBuffers *) RBRGen4_getUserData(conn);
    static RBRGen4DateTime clock;
    if (buffers->readBufferPos < buffers->readBufferSize) {
        clock = 0;
    } else {
        clock += conn->commandTimeout + 1;
    }
    *time = clock;
    return RBRGEN4_SUCCESS;
}

/* When the oversized line was the reply itself, nothing else arrives; the
 * wait ends as RESPONSE_TOO_LONG rather than a bare timeout. */
TEST_LOGGER4(responseBufferOverflowReply)
{
    RBRGen4Error err;
    RBRGen4PowerSource actual;
    RBRGen4TimeCallback savedTime = conn->environment.time;

    static char response[RBRGEN4_RESPONSE_BUFFER_DEFAULT + 64];
    memset(response, 'a', sizeof(response) - strlen(RESPONSE_TERMINATOR) - 1);
    strcpy(response + sizeof(response) - strlen(RESPONSE_TERMINATOR) - 1, RESPONSE_TERMINATOR);

    TestIOBuffers_init(buffers, response, 0);
    conn->environment.time = overflowReplyTime;
    err = RBRGen4_getPowerSource(conn, &actual);
    conn->environment.time = savedTime;
    TEST_ASSERT_ENUM_EQ(RBRGEN4_RESPONSE_TOO_LONG, err, RBRGen4Error);

    /* Nothing of the oversized line lingers. */
    TestIOBuffers_init(buffers, "instrument power source=int" RESPONSE_TERMINATOR, 0);
    err = RBRGen4_getPowerSource(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_POWER_SOURCE_INTERNAL, actual, RBRGen4PowerSource);

    return true;
}

/** \brief A clock which advances by a second on every call, whatever has been
 * read. */
static RBRGen4DateTime tickingClock;
static RBRGen4Error tickingTime(const RBRGen4 *conn, RBRGen4DateTime *time)
{
    (void) conn;
    tickingClock += 1000;
    *time = tickingClock;
    return RBRGEN4_SUCCESS;
}

/* The command timeout bounds the whole wait for a reply. An instrument
 * streaming lines too long for the buffer must not keep the wait alive one
 * line at a time: once the timeout has elapsed the command fails, and as an
 * oversized line was met, it fails with RESPONSE_TOO_LONG. */
TEST_LOGGER4(responseBufferOverflowStream)
{
    RBRGen4Error err;
    RBRGen4PowerSource actual;
    RBRGen4TimeCallback savedTime = conn->environment.time;

    /* Eight oversized lines, each needing several reads and clock checks, so
     * the timeout elapses long before the fixture runs dry. Relies on the
     * harness handing over at least a hundred or so bytes per read; smaller
     * reads spend the timeout before the first line overflows and the test
     * fails with TIMEOUT. */
    static char line[RBRGEN4_RESPONSE_BUFFER_DEFAULT + 16];
    memset(line, 'a', sizeof(line) - strlen(RESPONSE_TERMINATOR) - 1);
    strcpy(line + sizeof(line) - strlen(RESPONSE_TERMINATOR) - 1, RESPONSE_TERMINATOR);
    static char stream[8 * sizeof(line)];
    stream[0] = '\0';
    for (int i = 0; i < 8; i++) {
        strcat(stream, line);
    }

    /* The harness opens its connections with no command timeout; give this
     * one a few ticks' worth. */
    RBRGen4DateTime savedTimeout = RBRGen4_getCommandTimeout(conn);
    RBRGen4_setCommandTimeout(conn, 8000);
    TestIOBuffers_init(buffers, stream, 0);
    tickingClock = 0;
    conn->environment.time = tickingTime;
    err = RBRGen4_getPowerSource(conn, &actual);
    conn->environment.time = savedTime;
    RBRGen4_setCommandTimeout(conn, savedTimeout);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_RESPONSE_TOO_LONG, err, RBRGen4Error);

    return true;
}

/* An oversized line met before a retry belongs to the first attempt. When
 * the resent command times out, that is a timeout, not a buffer problem. */
TEST_LOGGER4(responseBufferOverflowRetry)
{
    RBRGen4Error err;
    RBRGen4PowerSource actual;
    RBRGen4TimeCallback savedTime = conn->environment.time;

    static char response[RBRGEN4_RESPONSE_BUFFER_DEFAULT + 64];
    memset(response, 'a', sizeof(response) - strlen(RESPONSE_TERMINATOR) - 1);
    strcpy(response + sizeof(response) - strlen(RESPONSE_TERMINATOR) - 1, RESPONSE_TERMINATOR);
    static char fixture[sizeof(response) + 64];
    /* The oversized line, then an invalid-command error which ends with our
     * command: garbage was ahead of it on the link, so it is resent. */
    snprintf(fixture,
             sizeof(fixture),
             "%sERR-102 invalid command 'xinstrument'" RESPONSE_TERMINATOR,
             response);

    TestIOBuffers_init(buffers, fixture, 0);
    conn->environment.time = overflowReplyTime;
    err = RBRGen4_getPowerSource(conn, &actual);
    conn->environment.time = savedTime;
    TEST_ASSERT_ENUM_EQ(RBRGEN4_TIMEOUT, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("instrument power" COMMAND_TERMINATOR "instrument power" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

/* An oversized line whose body exactly fills the buffer puts the first byte
 * of its terminator in the last slot. The discard must still stop at that
 * terminator rather than run on into the reply behind it. */
TEST_LOGGER4(responseBufferOverflowAtBoundary)
{
    RBRGen4Error err;
    RBRGen4PowerSource actual;

    static char response[RBRGEN4_RESPONSE_BUFFER_DEFAULT + 64];
    const char *rest = "instrument power source=int" RESPONSE_TERMINATOR;
    memset(response, 'a', RBRGEN4_RESPONSE_BUFFER_DEFAULT - 1);
    strcpy(response + RBRGEN4_RESPONSE_BUFFER_DEFAULT - 1, RESPONSE_TERMINATOR);
    strcat(response, rest);

    TestIOBuffers_init(buffers, response, 0);
    err = RBRGen4_getPowerSource(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_POWER_SOURCE_INTERNAL, actual, RBRGen4PowerSource);

    return true;
}

/* Losing the link part-way through an oversized line forgets the fragment
 * read so far, so a reply arriving once the link comes back is not glued to
 * it and lost. */
TEST_LOGGER4(responseBufferOverflowInterrupted)
{
    RBRGen4Error err;
    RBRGen4PowerSource actual;

    /* More than a buffer's worth of the line arrives, then the link goes
     * quiet: the read callback running dry stands in for a lost link. */
    static char head[RBRGEN4_RESPONSE_BUFFER_DEFAULT + 16];
    memset(head, 'a', sizeof(head) - 1);
    head[sizeof(head) - 1] = '\0';
    TestIOBuffers_init(buffers, head, 0);
    err = RBRGen4_getPowerSource(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_CALLBACK_ERROR, err, RBRGen4Error);

    /* Nothing of the oversized line ever follows; the next reply is whole. */
    TestIOBuffers_init(buffers, "instrument power source=int" RESPONSE_TERMINATOR, 0);
    err = RBRGen4_getPowerSource(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_POWER_SOURCE_INTERNAL, actual, RBRGen4PowerSource);

    return true;
}

/** \brief A clock which jumps past any command timeout as soon as anything
 * has been read, so the deadline lands right after the read which fills the
 * response buffer. */
static RBRGen4Error overflowTimeoutTime(const RBRGen4 *conn, RBRGen4DateTime *time)
{
    TestIOBuffers *buffers = (TestIOBuffers *) RBRGen4_getUserData(conn);
    *time = buffers->readBufferPos > 0 ? INT64_MAX / 2 : 0;
    return RBRGEN4_SUCCESS;
}

/* The read which fills the buffer may also be the one which crosses the
 * deadline. The buffer was too small regardless, and that is what is
 * reported, without waiting out a second timeout. Should the tail of the
 * line turn up later, it is skipped like any other unrelated line. */
TEST_LOGGER4(responseBufferOverflowTimeout)
{
    RBRGen4Error err;
    RBRGen4PowerSource actual;
    RBRGen4TimeCallback savedTime = conn->environment.time;

    /* More than a buffer's worth, so the first read fills the buffer without
     * running the fixture dry. */
    static char head[RBRGEN4_RESPONSE_BUFFER_DEFAULT + 16];
    memset(head, 'a', sizeof(head) - 1);
    head[sizeof(head) - 1] = '\0';
    TestIOBuffers_init(buffers, head, 0);
    conn->environment.time = overflowTimeoutTime;
    err = RBRGen4_getPowerSource(conn, &actual);
    conn->environment.time = savedTime;
    TEST_ASSERT_ENUM_EQ(RBRGEN4_RESPONSE_TOO_LONG, err, RBRGen4Error);

    TestIOBuffers_init(buffers,
                       "aaaaaaaa" RESPONSE_TERMINATOR
                       "instrument power source=int" RESPONSE_TERMINATOR,
                       0);
    err = RBRGen4_getPowerSource(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_POWER_SOURCE_INTERNAL, actual, RBRGen4PowerSource);

    return true;
}

TEST_LOGGER4(reboot)
{
    TestIOBuffers_init(buffers, "", 0);
    RBRGen4Error err = RBRGen4_reboot(conn, 10000);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("instrument reboot delay=10000" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT(conn->lastActivityTime < 0);

    /* A delay of zero omits the parameter entirely; the command defines no
     * default delay of its own. */
    TestIOBuffers_init(buffers, "", 0);
    err = RBRGen4_reboot(conn, 0);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ(RESPONSE_TERMINATOR RESPONSE_TERMINATOR
                       "instrument reboot" COMMAND_TERMINATOR,
                       buffers->writeBuffer);
    TEST_ASSERT(conn->lastActivityTime < 0);

    return true;
}

TEST_LOGGER4(factoryReset)
{
    TestIOBuffers_init(buffers, "instrument factory reset" RESPONSE_TERMINATOR, 0);
    RBRGen4Error err = RBRGen4_factoryReset(conn);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("instrument factory reset" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

typedef struct OutputFormatTest {
    const char *response;
    RBRGen4OutputFormat expected;
} OutputFormatTest;

static bool test_outputformat(RBRGen4OutputFormat *expected, RBRGen4OutputFormat *actual)
{
    TEST_ASSERT_EQ(expected->sn, actual->sn, "%d");
    TEST_ASSERT_EQ(expected->scheduleLabel, actual->scheduleLabel, "%d");
    TEST_ASSERT_EQ(expected->dateTime, actual->dateTime, "%d");
    TEST_ASSERT_EQ(expected->crc, actual->crc, "%d");
    TEST_ASSERT_ENUM_EQ(expected->dataType, actual->dataType, RBRGen4DataType);

    return true;
}

TEST_LOGGER4(outputformat)
{
    OutputFormatTest tests[] = {
        /* The format an L4 reports out of the box. */
        {
            .response = "instrument outputformat sn=off schedulelabel=on datetime=on "
                        "crc=off datatype=float32" RESPONSE_TERMINATOR,
            .expected =
                {
                    .sn = false,
                    .scheduleLabel = true,
                    .dateTime = true,
                    .crc = false,
                    .dataType = RBRGEN4_DATA_TYPE_FLOAT32,
                },
        },
        {
            .response = "instrument outputformat sn=on schedulelabel=on datetime=off "
                        "crc=on datatype=float64" RESPONSE_TERMINATOR,
            .expected =
                {
                    .sn = true,
                    .scheduleLabel = true,
                    .dateTime = false,
                    .crc = true,
                    .dataType = RBRGEN4_DATA_TYPE_FLOAT64,
                },
        },
        {
            .response = "instrument outputformat sn=off schedulelabel=off datetime=off "
                        "crc=off datatype=calfloat64" RESPONSE_TERMINATOR,
            .expected =
                {
                    .sn = false,
                    .scheduleLabel = false,
                    .dateTime = false,
                    .crc = false,
                    .dataType = RBRGEN4_DATA_TYPE_CALFLOAT64,
                },
        },
        {0},
    };

    RBRGen4Error err;
    RBRGen4OutputFormat actual;

    for (int i = 0; tests[i].response != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4_getOutputFormat(conn, &actual);
        TEST_ASSERT_STR_EQ("instrument outputformat" COMMAND_TERMINATOR, buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
        if (!test_outputformat(&tests[i].expected, &actual)) {
            return false;
        }
        /* The format is cached for the sample parser. */
        if (!test_outputformat(&tests[i].expected, &conn->outputFormat)) {
            return false;
        }
    }
    return true;
}

TEST_LOGGER4(outputformat_set)
{
    RBRGen4OutputFormat outputformat = {
        .sn = false,
        .scheduleLabel = true,
        .dateTime = true,
        .crc = false,
        .dataType = RBRGEN4_DATA_TYPE_FLOAT32,
    };

    /* Every parameter of the command is sent. */
    TestIOBuffers_init(buffers,
                       "instrument outputformat sn=off schedulelabel=on "
                       "datetime=on crc=off datatype=float32" RESPONSE_TERMINATOR,
                       0);
    RBRGen4Error err = RBRGen4_setOutputFormat(conn, &outputformat);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("instrument outputformat sn=off schedulelabel=on "
                       "datetime=on crc=off datatype=float32" COMMAND_TERMINATOR,
                       buffers->writeBuffer);
    if (!test_outputformat(&outputformat, &conn->outputFormat)) {
        return false;
    }

    /* Changing one parameter still sends them all. */
    RBRGen4OutputFormat modified = outputformat;
    modified.crc = true;
    TestIOBuffers_init(buffers,
                       "instrument outputformat sn=off schedulelabel=on "
                       "datetime=on crc=on datatype=float32" RESPONSE_TERMINATOR,
                       0);
    err = RBRGen4_setOutputFormat(conn, &modified);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("instrument outputformat sn=off schedulelabel=on "
                       "datetime=on crc=on datatype=float32" COMMAND_TERMINATOR,
                       buffers->writeBuffer);
    if (!test_outputformat(&modified, &conn->outputFormat)) {
        return false;
    }

    /* A datatype which is not a real value is not sent. */
    RBRGen4OutputFormat unreported = modified;
    unreported.dataType = RBRGEN4_UNKNOWN_DATA_TYPE;
    TestIOBuffers_init(buffers, "", 0);
    err = RBRGen4_setOutputFormat(conn, &unreported);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    /* A value the instrument rejects must not update the cached format. */
    RBRGen4OutputFormat rejected = modified;
    rejected.crc = false;
    TestIOBuffers_init(
        buffers, "ERR-108 invalid argument to command: 'bogus'" RESPONSE_TERMINATOR, 0);
    err = RBRGen4_setOutputFormat(conn, &rejected);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_HARDWARE_ERROR, err, RBRGen4Error);
    if (!test_outputformat(&modified, &conn->outputFormat)) {
        return false;
    }

    return true;
}
