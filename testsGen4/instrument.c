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

typedef struct IdTest
{
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4Id4 expected;
} IdTest;

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
    TEST_ASSERT_STR_EQ(expected->fw, actual->fw);
    TEST_ASSERT_STR_EQ(expected->hw, actual->hw);
    TEST_ASSERT_EQ(expected->address, actual->address, "%" PRIi32);

    return true;
}

TEST_LOGGER4(pcbalist)
{
    RBRInstrumentGen4PcbaPool expected = {
        .count = 3,
        .pool = { {"L3-CPU", 0, "", "", "", 0},
                  {"FE-cond3", 0, "", "", "", 0},
                  {"FE-v2", 0, "", "", "", 0} }
    };
    RBRInstrumentGen4PcbaPool actual;

    TestIOBuffers_init(buffers,
                       "pcba count=3 list=L3-CPU|FE-cond3|FE-v2" RESPONSE_TERMINATOR,
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
        .label = "FE-cond3",
        .sn = 123456,
        .pn = "0123456revA",
        .fw = "1.1.1",
        .hw = "A01",
        .address = 128
    };

    RBRInstrumentGen4Pcba actual = {
        .label = "FE-cond3"
    };

    TestIOBuffers_init(buffers,
                       "pcba FE-cond3 sn=123456 pn=0123456revA fw=1.1.1 hw=A01 address=128" RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_getPcba(instrument,
                                                           &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);

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
        .capacity = 138000,
        .used = 100100
    };
    RBRInstrumentGen4PowerInternal actual;

    TestIOBuffers_init(buffers,
                       "instrument power internal voltage=14.21 batterytype=nimh "
                       "capacity=138.000e+003 used=100.100e+003" RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_getPowerInternal(instrument,
                                                                    &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_FLOAT_EQ(expected.voltage, actual.voltage, 0.001f);
    TEST_ASSERT_ENUM_EQ(expected.batteryType,
                        actual.batteryType,
                        RBRInstrumentGen4InternalBatteryType);
    TEST_ASSERT_FLOAT_EQ(expected.capacity, actual.capacity, 0.01f);
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
        .capacity = 22000000,
        .used = 100100
    };
    RBRInstrumentGen4PowerExternal actual;

    TestIOBuffers_init(buffers,
                       "instrument power external voltage=14.21 batterytype=fermata_lisocl2 "
                       "capacity=22.000e+006 used=100.100e+003" RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_getPowerExternal(instrument,
                                                                    &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_FLOAT_EQ(expected.voltage, actual.voltage, 0.001f);
    TEST_ASSERT_ENUM_EQ(expected.batteryType,
                        actual.batteryType,
                        RBRInstrumentGen4ExternalBatteryType);
    TEST_ASSERT_FLOAT_EQ(expected.capacity, actual.capacity, 1.0f);
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

typedef struct InfoTest
{
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4Info expected;
} InfoTest;

TEST_LOGGER4(info)
{
    InfoTest tests[] = {
        { "instrument pn=L3-M11-BEC11-SC11-ST11-SP11 fwlock=off datatype=float64" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { "L3-M11-BEC11-SC11-ST11-SP11",
            false,
            RBRINSTRUMENTGEN4_DATATYPE_FLOAT64 } },
        { "instrument pn=012345revA fwlock=on datatype=float32" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { "012345revA",
            true,
            RBRINSTRUMENTGEN4_DATATYPE_FLOAT32 } },
        { 0 }
    };
    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Info actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers,
                           tests[i].response,
                           0);
        err = RBRInstrumentGen4_getInfo(instrument,
                                        &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].expected.pn, actual.pn);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.fwLock, actual.fwLock, bool);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.dataType, actual.dataType, RBRInstrumentGen4DataType);
    }
    return true;
}
