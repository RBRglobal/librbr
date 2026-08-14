/**
 * \file other.c
 *
 * \brief Tests for other instrument commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for NAN. */
#include <math.h>

#include "tests.h"
#include "RBRInstrumentGen4Instrument.h"

// only used in RBRInstrumentGen4_setPostprocessing() in RBRInstrumentGen4Memory.c
TEST_LOGGER4(version_comparison)
{
    /* Valid versions. */
    TEST_ASSERT(RBRInstrumentGen4Version_compare("1.14.5+202310150927", "1.14.5+202310150927") == RBRINSTRUMENTGEN4_FW_EQUAL);
    TEST_ASSERT(RBRInstrumentGen4Version_compare("1.14.6+202310150927", "1.14.5+202310150927") == RBRINSTRUMENTGEN4_FW_GREATER_THAN);
    TEST_ASSERT(RBRInstrumentGen4Version_compare("1.14.4+202310150927", "1.14.5+202310150927") == RBRINSTRUMENTGEN4_FW_LESS_THAN);

    TEST_ASSERT(RBRInstrumentGen4Version_compare("0.7.0-dev.dbg+202312051738", "0.7.0-dev.dbg+202312051738") == RBRINSTRUMENTGEN4_FW_EQUAL);
    TEST_ASSERT(RBRInstrumentGen4Version_compare("0.8.0-dev.dbg+202312051738", "0.7.0-dev.dbg+202312051738") == RBRINSTRUMENTGEN4_FW_GREATER_THAN);
    TEST_ASSERT(RBRInstrumentGen4Version_compare("0.6.0-dev.dbg+202312051738", "0.7.0-dev.dbg+202312051738") == RBRINSTRUMENTGEN4_FW_LESS_THAN);

    TEST_ASSERT(RBRInstrumentGen4Version_compare("1.0.0-dev.dbg+202312051738", "1.0.0+202312051738") == RBRINSTRUMENTGEN4_FW_EQUAL);
    TEST_ASSERT(RBRInstrumentGen4Version_compare("1.0.0+202312051738", "0.7.0-dev.dbg+202312051738") == RBRINSTRUMENTGEN4_FW_GREATER_THAN);
    TEST_ASSERT(RBRInstrumentGen4Version_compare("0.7.0-dev.dbg+202312051738", "1.0.0+202312051738") == RBRINSTRUMENTGEN4_FW_LESS_THAN);

    TEST_ASSERT(RBRInstrumentGen4Version_compare("0.0.000+202310150927", "000.0.0+202310150927") == RBRINSTRUMENTGEN4_FW_EQUAL);
    /* Invalid versions. */
    TEST_ASSERT(RBRInstrumentGen4Version_compare(".", ".") == RBRINSTRUMENTGEN4_FW_INVALID);
    TEST_ASSERT(RBRInstrumentGen4Version_compare(".000.0+202310150927", "0.000.0+202310150927") == RBRINSTRUMENTGEN4_FW_INVALID);
    TEST_ASSERT(RBRInstrumentGen4Version_compare("..0+202310150927", "0.000.0+202310150927") == RBRINSTRUMENTGEN4_FW_INVALID);
    TEST_ASSERT(RBRInstrumentGen4Version_compare("0.0.+202310150927", "0.0.000+202310150927") == RBRINSTRUMENTGEN4_FW_INVALID);
    TEST_ASSERT(RBRInstrumentGen4Version_compare("0.000.+202310150927", "0.0.000+202310150927") == RBRINSTRUMENTGEN4_FW_INVALID);
    return true;
}

typedef struct LegacyIdTest
{
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4Id expected;
} LegacyIdTest;

typedef struct IdTest
{
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4Id4 expected;
} IdTest;

TEST_LOGGER4(id)
{
    LegacyIdTest tests[] = {
        { "id model = L4, "
          "version = 2.0.0, "
          "serial = 999999, "
          "fwtype = 150"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { "L4",
            "2.0.0",
            999999,
            150 } },
        /* A response carrying no parameters leaves the struct zeroed. */
        { "id" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { "",
            "",
            0,
            0 } },
        /*
         * A name too long for the key buffer must not truncate onto one of
         * the names we look for: `versionfoo` would become `version` in a
         * buffer sized to the longest name exactly. Only `model` is read
         * here.
         */
        { "id model = L4, "
          "versionfoo = 9.9.9"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { "L4",
            "",
            0,
            0 } },
        { 0 }
    };

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Id actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_getId(instrument, &actual);
        TEST_ASSERT_STR_EQ("id" COMMAND_TERMINATOR, buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].expected.model, actual.model);
        TEST_ASSERT_STR_EQ(tests[i].expected.fwversion, actual.fwversion);
        TEST_ASSERT_EQ(tests[i].expected.sn, actual.sn, "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.fwtype, actual.fwtype, "%" PRIi32);
    }
    return true;
}

TEST_LOGGER4(id4)
{
    IdTest tests[] = {
        { "id4 model=L4 "
          "sn=999999 "
          "fwversion=2.0.0 "
          "semver=2.0.0-rc1-10-g148bc5eb1 "
          "fwtype=150"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { "L4",
            "2.0.0",
            "2.0.0-rc1-10-g148bc5eb1",
            999999,
            150 } },
        /* A response carrying no parameters leaves the struct zeroed. */
        { "id4" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { "",
            "",
            "",
            0,
            0 } },
        { 0 }
    };

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Id4 actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_getId4(instrument, &actual);
        TEST_ASSERT_STR_EQ("id4" COMMAND_TERMINATOR, buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].expected.model, actual.model);
        TEST_ASSERT_STR_EQ(tests[i].expected.fwversion, actual.fwversion);
        TEST_ASSERT_STR_EQ(tests[i].expected.semver, actual.semver);
        TEST_ASSERT_EQ(tests[i].expected.sn, actual.sn, "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.fwtype, actual.fwtype, "%" PRIi32);
    }
    return true;
}

static bool test_pcba(RBRInstrumentGen4Pcba *expected, RBRInstrumentGen4Pcba *actual)
{
    TEST_ASSERT_STR_EQ(expected->label, actual->label);
    TEST_ASSERT_EQ(expected->sn, actual->sn, "%" PRIi32);
    TEST_ASSERT_STR_EQ(expected->pn, actual->pn);
    TEST_ASSERT_STR_EQ(expected->node, actual->node);

    return true;
}

TEST_LOGGER4(pcbalist)
{
    /* The pool getter reports only labels; the remaining fields come from
     * RBRInstrumentGen4_getPcba(). */
    RBRInstrumentGen4PcbaPool expected = {
        .count = 2,
        .pool = { { "self", 0, "", "" },
                  { "fe4_cond_00", 0, "", "" } }
    };
    RBRInstrumentGen4PcbaPool actual;

    TestIOBuffers_init(buffers,
                       "pcba count=2 list=self|fe4_cond_00" RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_getPcbaPool(instrument,
                                                               &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_EQ(expected.count, actual.count, "%" PRIi32);
    for (int32_t pcba = 0; pcba < actual.count; ++pcba)
    {
        if (!test_pcba(&expected.pool[pcba], &actual.pool[pcba]))
        {
            return false;
        }
    }

    return true;
}

TEST_LOGGER4(pcba)
{
    RBRInstrumentGen4Pcba expected = {
        .label = "fe4_cond_00",
        .sn = 0,
        .pn = "na",
        .node = "fe4_cond_00"
    };

    RBRInstrumentGen4Pcba actual = {
        .label = "fe4_cond_00"
    };

    TestIOBuffers_init(buffers,
                       "pcba fe4_cond_00 sn=na pn=na node=fe4_cond_00"
                       RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_getPcba(instrument,
                                                           &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("pcba fe4_cond_00" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return test_pcba(&expected, &actual);
}

TEST_LOGGER4(pcbaSerialNumber)
{
    /* Zero stands for the `na` of the test above, so prove a real serial
     * number is read rather than left at that zero. */
    RBRInstrumentGen4Pcba expected = {
        .label = "self",
        .sn = 850032,
        .pn = "na",
        .node = "self"
    };

    RBRInstrumentGen4Pcba actual = {
        .label = "self"
    };

    TestIOBuffers_init(buffers,
                       "pcba self sn=850032 pn=na node=self"
                       RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_getPcba(instrument,
                                                           &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("pcba self" COMMAND_TERMINATOR, buffers->writeBuffer);

    return test_pcba(&expected, &actual);
}

typedef struct PowerTest
{
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4PowerSource expected;
} PowerTest;

TEST_LOGGER4(power)
{
    PowerTest tests[] = {
        { "instrument power source=usb" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_POWER_SOURCE_USB },
        { "instrument power source=ext" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_POWER_SOURCE_EXTERNAL },
        { "instrument power source=int" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_POWER_SOURCE_INTERNAL },
        { 0 }
    };

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4PowerSource actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {

        TestIOBuffers_init(buffers,
                           tests[i].response,
                           0);
        err = RBRInstrumentGen4_getPowerSource(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentGen4Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected,
                            actual,
                            RBRInstrumentGen4PowerSource);
    }

    return true;
}

TEST_LOGGER4(powerinternal)
{
    RBRInstrumentGen4PowerInternal expected = {
        .voltage = 14.21,
        .batteryType = RBRINSTRUMENTGEN4_INTERNAL_BATTERY_NIMH,
        .used = 100100
    };
    RBRInstrumentGen4PowerInternal actual;

    TestIOBuffers_init(buffers,
                       "instrument power internal voltage=14.21 batterytype=nimh "
                       "used=100.100e+003" RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_getPowerInternal(instrument,
                                                                    &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_FLOAT_EQ(expected.voltage, actual.voltage, 0.001f);
    TEST_ASSERT_ENUM_EQ(expected.batteryType,
                        actual.batteryType,
                        RBRInstrumentGen4InternalBatteryType);
    TEST_ASSERT_FLOAT_EQ(expected.used, actual.used, 0.01f);

    return true;
}

typedef struct PowerInternalBatteryTypeTest
{
    const char *command;
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4InternalBatteryType batteryType;
} PowerInternalBatteryTypeTest;

TEST_LOGGER4(setPowerInternalBatteryType)
{
    PowerInternalBatteryTypeTest tests[] = {
        { "instrument power internal batterytype=lisocl2" COMMAND_TERMINATOR,
          "instrument power internal batterytype=lisocl2" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_INTERNAL_BATTERY_LISOCL2 },
        { "instrument power internal batterytype=lifes2" COMMAND_TERMINATOR,
          "instrument power internal batterytype=lifes2" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_INTERNAL_BATTERY_LIFES2 },
        { "instrument power internal batterytype=znmno2" COMMAND_TERMINATOR,
          "instrument power internal batterytype=znmno2" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_INTERNAL_BATTERY_ZNMNO2 },
        { "instrument power internal batterytype=linimnco" COMMAND_TERMINATOR,
          "instrument power internal batterytype=linimnco" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_INTERNAL_BATTERY_LINIMNCO },
        { "instrument power internal batterytype=nimh" COMMAND_TERMINATOR,
          "instrument power internal batterytype=nimh" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_INTERNAL_BATTERY_NIMH },
        { "instrument power internal batterytype=none" COMMAND_TERMINATOR,
          "instrument power internal batterytype=none" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_INTERNAL_BATTERY_NONE },
        { 0 }
    };

    RBRInstrumentGen4Error err;
    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers,
                           tests[i].response,
                           0);
        err = RBRInstrumentGen4_setPowerInternalBatteryType(instrument, tests[i].batteryType);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command,
                           buffers->writeBuffer);
    }
    return true;
}

TEST_LOGGER4(resetPowerInternalUsed)
{
    TestIOBuffers_init(buffers, "instrument power internal used=0.000e+000" RESPONSE_TERMINATOR, 0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_resetPowerInternalUsed(instrument);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("instrument power internal used=0" COMMAND_TERMINATOR, buffers->writeBuffer);
    return true;
}

TEST_LOGGER4(powerexternal)
{
    RBRInstrumentGen4PowerExternal expected = {
        .voltage = 14.21,
        .batteryType = RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMATA_LISOCL2,
        .used = 100100
    };
    RBRInstrumentGen4PowerExternal actual;

    TestIOBuffers_init(buffers,
                       "instrument power external voltage=14.21 "
                       "batterytype=fermata_lisocl2 used=100.100e+003"
                       RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_getPowerExternal(instrument,
                                                                    &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_FLOAT_EQ(expected.voltage, actual.voltage, 0.001f);
    TEST_ASSERT_ENUM_EQ(expected.batteryType,
                        actual.batteryType,
                        RBRInstrumentGen4ExternalBatteryType);
    TEST_ASSERT_FLOAT_EQ(expected.used, actual.used, 0.01f);

    return true;
}

typedef struct PowerExternalBatteryTypeTest
{
    const char *command;
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4ExternalBatteryType batteryType;
} PowerExternalBatteryTypeTest;

TEST_LOGGER4(setPowerExternalBatteryType)
{
    PowerExternalBatteryTypeTest tests[] = {
        { "instrument power external batterytype=fermata_lisocl2" COMMAND_TERMINATOR,
          "instrument power external batterytype=fermata_lisocl2" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMATA_LISOCL2, },
        { "instrument power external batterytype=fermata_znmno2" COMMAND_TERMINATOR,
          "instrument power external batterytype=fermata_znmno2" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMATA_ZNMNO2 },
        { "instrument power external batterytype=fermette_limno2" COMMAND_TERMINATOR,
          "instrument power external batterytype=fermette_limno2" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE_LIMNO2 },
        { "instrument power external batterytype=fermette3_lisocl2" COMMAND_TERMINATOR,
          "instrument power external batterytype=fermette3_lisocl2" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_LISOCL2 },
        { "instrument power external batterytype=fermette3_lifes2" COMMAND_TERMINATOR,
          "instrument power external batterytype=fermette3_lifes2" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_LIFES2 },
        { "instrument power external batterytype=fermette3_znmno2" COMMAND_TERMINATOR,
          "instrument power external batterytype=fermette3_znmno2" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_ZNMNO2 },
        { "instrument power external batterytype=fermette3_linimnco" COMMAND_TERMINATOR,
          "instrument power external batterytype=fermette3_linimnco" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_LINIMNCO },
        { "instrument power external batterytype=fermette3_nimh" COMMAND_TERMINATOR,
          "instrument power external batterytype=fermette3_nimh" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_NIMH },
        { "instrument power external batterytype=fermata_nimh" COMMAND_TERMINATOR,
          "instrument power external batterytype=fermata_nimh" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMATA_NIMH },
        { "instrument power external batterytype=other" COMMAND_TERMINATOR,
          "instrument power external batterytype=other" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_OTHER },
        { "instrument power external batterytype=none" COMMAND_TERMINATOR,
          "instrument power external batterytype=none" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_NONE },
        { 0 }
    };

    RBRInstrumentGen4Error err;
    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers,
                           tests[i].response,
                           0);
        err = RBRInstrumentGen4_setPowerExternalBatteryType(instrument, tests[i].batteryType);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command,
                           buffers->writeBuffer);
    }
    return true;
}

TEST_LOGGER4(resetPowerExternalUsed)
{
    TestIOBuffers_init(buffers, "instrument power external used=0.000e+000" RESPONSE_TERMINATOR, 0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_resetPowerExternalUsed(instrument);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("instrument power external used=0" COMMAND_TERMINATOR, buffers->writeBuffer);
    return true;
}

typedef struct InstrumentCommandTest
{
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4Instrument expected;
} InstrumentCommandTest;

TEST_LOGGER4(instrument)
{
    InstrumentCommandTest tests[] = {
        { "instrument state=disabled sn=999999 model=L4 pn=9999999revA "
          "fwversion=2.0.0 semver=2.0.0-rc1-10-g148bc5eb1 fwtype=150 "
          "fwlock=off datatype=float64 name=L4"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { RBRINSTRUMENTGEN4_LOGGING_STATE_DISABLED,
            999999,
            "L4",
            "9999999revA",
            "2.0.0",
            "2.0.0-rc1-10-g148bc5eb1",
            150,
            false,
            RBRINSTRUMENTGEN4_DATATYPE_FLOAT64,
            "L4" } },
        /* An enabled instrument with the firmware locked, and the extended
         * name and part number populated. */
        { "instrument state=enabled sn=210000 model=RBRsolo4 "
          "pn=L3-M11-BEC11-SC11-ST11-SP11 fwversion=1.0.0 "
          "semver=1.0.0-rc4-11-g941ae64 fwtype=130 fwlock=on "
          "datatype=float32 name=RBRsolo^4_T.D!fast32"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { RBRINSTRUMENTGEN4_LOGGING_STATE_ENABLED,
            210000,
            "RBRsolo4",
            "L3-M11-BEC11-SC11-ST11-SP11",
            "1.0.0",
            "1.0.0-rc4-11-g941ae64",
            130,
            true,
            RBRINSTRUMENTGEN4_DATATYPE_FLOAT32,
            "RBRsolo^4_T.D!fast32" } },
        /* calfloat64 is reported only during a calibration-mode deployment. */
        { "instrument state=enabled sn=210000 model=RBRsolo4 pn=012345revA "
          "fwversion=1.0.0 semver=1.0.0 fwtype=130 fwlock=off "
          "datatype=calfloat64 name=RBRsolo4"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { RBRINSTRUMENTGEN4_LOGGING_STATE_ENABLED,
            210000,
            "RBRsolo4",
            "012345revA",
            "1.0.0",
            "1.0.0",
            130,
            false,
            RBRINSTRUMENTGEN4_DATATYPE_CALFLOAT64,
            "RBRsolo4" } },
        /* An unrecognized data type must not be reported as float32, which is
         * the zero value of the enum. */
        { "instrument state=disabled sn=999999 model=L4 pn=9999999revA "
          "fwversion=2.0.0 semver=2.0.0 fwtype=150 fwlock=off "
          "datatype=float128 name=L4"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { RBRINSTRUMENTGEN4_LOGGING_STATE_DISABLED,
            999999,
            "L4",
            "9999999revA",
            "2.0.0",
            "2.0.0",
            150,
            false,
            RBRINSTRUMENTGEN4_UNKNOWN_DATATYPE,
            "L4" } },
        { 0 }
    };
    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Instrument actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers,
                           tests[i].response,
                           0);
        err = RBRInstrumentGen4_getInstrument(instrument,
                                              &actual);
        TEST_ASSERT_STR_EQ("instrument" COMMAND_TERMINATOR,
                           buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentGen4Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.state,
                            actual.state,
                            RBRInstrumentGen4LoggingState);
        TEST_ASSERT_EQ(tests[i].expected.sn, actual.sn, "%" PRIi32);
        TEST_ASSERT_STR_EQ(tests[i].expected.model, actual.model);
        TEST_ASSERT_STR_EQ(tests[i].expected.pn, actual.pn);
        TEST_ASSERT_STR_EQ(tests[i].expected.fwversion, actual.fwversion);
        TEST_ASSERT_STR_EQ(tests[i].expected.semver, actual.semver);
        TEST_ASSERT_EQ(tests[i].expected.fwtype, actual.fwtype, "%" PRIi32);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.fwLock, actual.fwLock, bool);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.dataType, actual.dataType, RBRInstrumentGen4DataType);
        TEST_ASSERT_STR_EQ(tests[i].expected.name, actual.name);
    }
    return true;
}

TEST_LOGGER4(reboot)
{
    TestIOBuffers_init(buffers, "", 0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_reboot(instrument, 10000);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("instrument reboot delay=10000" COMMAND_TERMINATOR,
                       buffers->writeBuffer);
    TEST_ASSERT(instrument->lastActivityTime < 0);

    /* A delay of zero omits the parameter entirely; the command defines no
     * default delay of its own. */
    TestIOBuffers_init(buffers, "", 0);
    err = RBRInstrumentGen4_reboot(instrument, 0);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ(RESPONSE_TERMINATOR RESPONSE_TERMINATOR
                       "instrument reboot" COMMAND_TERMINATOR,
                       buffers->writeBuffer);
    TEST_ASSERT(instrument->lastActivityTime < 0);

    return true;
}

TEST_LOGGER4(factoryReset)
{
    TestIOBuffers_init(buffers,
                       "instrument factory reset" RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_factoryReset(instrument);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("instrument factory reset" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

typedef struct OutputFormatTest
{
    const char *response;
    RBRInstrumentGen4OutputFormat expected;
} OutputFormatTest;

static bool test_outputformat(RBRInstrumentGen4OutputFormat *expected,
                              RBRInstrumentGen4OutputFormat *actual)
{
    TEST_ASSERT_EQ(expected->sn, actual->sn, "%d");
    TEST_ASSERT_EQ(expected->scheduleLabel, actual->scheduleLabel, "%d");
    TEST_ASSERT_EQ(expected->dateTime, actual->dateTime, "%d");
    TEST_ASSERT_EQ(expected->crc, actual->crc, "%d");
    TEST_ASSERT_ENUM_EQ(expected->encoding,
                        actual->encoding,
                        RBRInstrumentGen4Encoding);
    TEST_ASSERT_ENUM_EQ(expected->dataType,
                        actual->dataType,
                        RBRInstrumentGen4DataType);

    return true;
}

TEST_LOGGER4(outputformat)
{
    OutputFormatTest tests[] = {
        /* The format an L4 reports out of the box. */
        { "instrument outputformat sn=off schedulelabel=on datetime=on "
          "crc=off encoding=ascii datatype=float32" RESPONSE_TERMINATOR,
          { false, true, true, false,
            RBRINSTRUMENTGEN4_ENCODING_ASCII,
            RBRINSTRUMENTGEN4_DATATYPE_FLOAT32 } },
        { "instrument outputformat sn=on schedulelabel=on datetime=off "
          "crc=on encoding=binary datatype=float64" RESPONSE_TERMINATOR,
          { true, true, false, true,
            RBRINSTRUMENTGEN4_ENCODING_BINARY,
            RBRINSTRUMENTGEN4_DATATYPE_FLOAT64 } },
        { "instrument outputformat sn=off schedulelabel=off datetime=off "
          "crc=off encoding=ascii datatype=calfloat64" RESPONSE_TERMINATOR,
          { false, false, false, false,
            RBRINSTRUMENTGEN4_ENCODING_ASCII,
            RBRINSTRUMENTGEN4_DATATYPE_CALFLOAT64 } },
        { 0 }
    };

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4OutputFormat actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_getOutputFormat(instrument, &actual);
        TEST_ASSERT_STR_EQ("instrument outputformat" COMMAND_TERMINATOR,
                           buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
        if (!test_outputformat(&tests[i].expected, &actual))
        {
            return false;
        }
        /* The format is cached for the sample parser. */
        if (!test_outputformat(&tests[i].expected, &instrument->outputFormat))
        {
            return false;
        }
    }
    return true;
}

TEST_LOGGER4(outputformat_set)
{
    RBRInstrumentGen4OutputFormat outputformat = {
        .sn = false,
        .scheduleLabel = true,
        .dateTime = true,
        .crc = false,
        .encoding = RBRINSTRUMENTGEN4_ENCODING_ASCII,
        .dataType = RBRINSTRUMENTGEN4_DATATYPE_FLOAT32
    };

    /* Every parameter of the command is sent. */
    TestIOBuffers_init(buffers,
                       "instrument outputformat sn=off schedulelabel=on "
                       "datetime=on crc=off encoding=ascii datatype=float32"
                       RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_setOutputFormat(
        instrument,
        &outputformat);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("instrument outputformat sn=off schedulelabel=on "
                       "datetime=on crc=off encoding=ascii datatype=float32"
                       COMMAND_TERMINATOR,
                       buffers->writeBuffer);
    if (!test_outputformat(&outputformat, &instrument->outputFormat))
    {
        return false;
    }

    /* Changing one parameter still sends them all. */
    RBRInstrumentGen4OutputFormat modified = outputformat;
    modified.crc = true;
    TestIOBuffers_init(buffers,
                       "instrument outputformat sn=off schedulelabel=on "
                       "datetime=on crc=on encoding=ascii datatype=float32"
                       RESPONSE_TERMINATOR,
                       0);
    err = RBRInstrumentGen4_setOutputFormat(instrument, &modified);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("instrument outputformat sn=off schedulelabel=on "
                       "datetime=on crc=on encoding=ascii datatype=float32"
                       COMMAND_TERMINATOR,
                       buffers->writeBuffer);
    if (!test_outputformat(&modified, &instrument->outputFormat))
    {
        return false;
    }

    /* An encoding or datatype which is not a real value is not sent. */
    RBRInstrumentGen4OutputFormat unreported = modified;
    unreported.encoding = RBRINSTRUMENTGEN4_UNKNOWN_ENCODING;
    TestIOBuffers_init(buffers, "", 0);
    err = RBRInstrumentGen4_setOutputFormat(instrument, &unreported);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE,
                        err,
                        RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    /* A value the instrument rejects must not update the cached format. */
    RBRInstrumentGen4OutputFormat rejected = modified;
    rejected.crc = false;
    TestIOBuffers_init(buffers,
                       "ERR-108 invalid argument to command: 'bogus'"
                       RESPONSE_TERMINATOR,
                       0);
    err = RBRInstrumentGen4_setOutputFormat(instrument, &rejected);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_HARDWARE_ERROR,
                        err,
                        RBRInstrumentGen4Error);
    if (!test_outputformat(&modified, &instrument->outputFormat))
    {
        return false;
    }

    return true;
}
