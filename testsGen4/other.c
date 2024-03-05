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
    RBRInstrumentGen4Id expected;
} IdTest;

TEST_LOGGER4(id)
{
    IdTest tests[] = {
        { "id model = RBRconcerto4, version = 1.14.5+202310150927, "
          "serial = 092431, fwtype = 130" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { "RBRconcerto4",
            "1.14.5+202310150927",
            92431,
            130 } },
        { // replacing previous "id_short" test.
          "id" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { "",
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
        TEST_ASSERT_STR_EQ(tests[i].expected.version, actual.version);
        TEST_ASSERT_EQ(tests[i].expected.serial, actual.serial, "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.fwtype, actual.fwtype, "%" PRIi32);
    }
    return true;
}

TEST_LOGGER4(hwrev)
{
    RBRInstrumentGen4HardwareRevision expected = {
        .pcb = 'J',
        .cpu = "5659A",
        .bsl = 'A'
    };
    RBRInstrumentGen4HardwareRevision actual;

    TestIOBuffers_init(buffers,
                       "hwrev pcb = J, cpu = 5659A, bsl = A" RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_getHardwareRevision(instrument,
                                                                       &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_EQ(expected.pcb, actual.pcb, "%c");
    TEST_ASSERT_STR_EQ(expected.cpu, actual.cpu);
    TEST_ASSERT_EQ(expected.bsl, actual.bsl, "%c");

    return true;
}

typedef struct PowerTest
{
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4Power expected;
} PowerTest;

TEST_LOGGER4(power)
{
    PowerTest tests[] = {
        { "power source = usb, int = 12.40, ext = 0.00, "
          "reg = n/a" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { RBRINSTRUMENTGEN4_POWER_SOURCE_USB,
            12.4,
            0.0,
            NAN } },
        { "power source = ext, int =  0.00, ext = 4.53, "
          "reg = n/a" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { RBRINSTRUMENTGEN4_POWER_SOURCE_EXTERNAL,
            0.0,
            4.53,
            NAN } },
        { "power source = int, int =  0.00, ext = 12.4, "
          "reg = n/a" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { RBRINSTRUMENTGEN4_POWER_SOURCE_INTERNAL,
            0.0,
            12.4,
            NAN } },
        { 0 }
    };

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Power actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {

        TestIOBuffers_init(buffers,
                           tests[i].response,
                           0);
        err = RBRInstrumentGen4_getPower(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentGen4Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.source,
                            actual.source,
                            RBRInstrumentGen4PowerSource);
        TEST_ASSERT_EQ(tests[i].expected.internal, actual.internal, "%f");
        TEST_ASSERT_EQ(tests[i].expected.external, actual.external, "%f");
        TEST_ASSERT(isnan(actual.regulator));
    }

    return true;
}

TEST_LOGGER4(powerinternal)
{
    RBRInstrumentGen4PowerInternal expected = {
        .batteryType = RBRINSTRUMENTGEN4_INTERNAL_BATTERY_NIMH,
        .capacity = 138000,
        .used = 100100
    };
    RBRInstrumentGen4PowerInternal actual;

    TestIOBuffers_init(buffers,
                       "powerinternal batterytype = nimh, "
                       "capacity = 138.000e+003, used = 100.100e+003" RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_getPowerInternal(instrument,
                                                                    &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_ENUM_EQ(expected.batteryType,
                        actual.batteryType,
                        RBRInstrumentGen4InternalBatteryType);
    TEST_ASSERT_EQ(expected.capacity, actual.capacity, "%f");
    TEST_ASSERT_EQ(expected.used, actual.used, "%f");

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
        { "powerinternal batterytype = lisocl2" COMMAND_TERMINATOR,
          "powerinternal batterytype = lisocl2" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_INTERNAL_BATTERY_LISOCL2 },
        { "powerinternal batterytype = lifes2" COMMAND_TERMINATOR,
          "powerinternal batterytype = lifes2" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_INTERNAL_BATTERY_LIFES2 },
        { "powerinternal batterytype = znmno2" COMMAND_TERMINATOR,
          "powerinternal batterytype = znmno2" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_INTERNAL_BATTERY_ZNMNO2 },
        { "powerinternal batterytype = linimnco" COMMAND_TERMINATOR,
          "powerinternal batterytype = linimnco" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_INTERNAL_BATTERY_LINIMNCO },
        { "powerinternal batterytype = nimh" COMMAND_TERMINATOR,
          "powerinternal batterytype = nimh" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_INTERNAL_BATTERY_NIMH },
        { "powerinternal batterytype = none" COMMAND_TERMINATOR,
          "powerinternal batterytype = none" RESPONSE_TERMINATOR,
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
    TestIOBuffers_init(buffers, "powerinternal used = 0.000e+000" RESPONSE_TERMINATOR, 0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_resetPowerInternalUsed(instrument);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("powerinternal used = 0" COMMAND_TERMINATOR, buffers->writeBuffer);
    return true;
}

TEST_LOGGER4(powerexternal)
{
    RBRInstrumentGen4PowerExternal expected = {
        .batteryType = RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMATA_LISOCL2,
        .capacity = 22000000,
        .used = 100100
    };
    RBRInstrumentGen4PowerExternal actual;

    TestIOBuffers_init(buffers,
                       " powerexternal batterytype = fermata_lisocl2, "
                       "capacity = 22.000e+006, used = 100.100e+003" RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_getPowerExternal(instrument,
                                                                    &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_ENUM_EQ(expected.batteryType,
                        actual.batteryType,
                        RBRInstrumentGen4ExternalBatteryType);
    TEST_ASSERT_EQ(expected.capacity, actual.capacity, "%f");
    TEST_ASSERT_EQ(expected.used, actual.used, "%f");

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
        {
            "powerexternal batterytype = fermata_lisocl2" COMMAND_TERMINATOR,
            "powerexternal batterytype = fermata_lisocl2" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_SUCCESS,
            RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMATA_LISOCL2,
        },
        { "powerexternal batterytype = fermata_znmno2" COMMAND_TERMINATOR,
          "powerexternal batterytype = fermata_znmno2" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMATA_ZNMNO2 },
        { "powerexternal batterytype = fermette_limno2" COMMAND_TERMINATOR,
          "powerexternal batterytype = fermette_limno2" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE_LIMNO2 },
        { "powerexternal batterytype = fermette3_lisocl2" COMMAND_TERMINATOR,
          "powerexternal batterytype = fermette3_lisocl2" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_LISOCL2 },
        {
            "powerexternal batterytype = fermette3_lifes2" COMMAND_TERMINATOR,
            "powerexternal batterytype = fermette3_lifes2" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_SUCCESS,
            RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_LIFES2,
        },
        { "powerexternal batterytype = fermette3_znmno2" COMMAND_TERMINATOR,
          "powerexternal batterytype = fermette3_znmno2" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_ZNMNO2 },
        { "powerexternal batterytype = fermette3_linimnco" COMMAND_TERMINATOR,
          "powerexternal batterytype = fermette3_linimnco" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_LINIMNCO },
        { "powerexternal batterytype = fermette3_nimh" COMMAND_TERMINATOR,
          "powerexternal batterytype = fermette3_nimh" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_NIMH },
        { "powerexternal batterytype = fermata_nimh" COMMAND_TERMINATOR,
          "powerexternal batterytype = fermata_nimh" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMATA_NIMH },
        { "powerexternal batterytype = other" COMMAND_TERMINATOR,
          "powerexternal batterytype = other" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_OTHER },
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
    TestIOBuffers_init(buffers, "powerexternal used = 0.000e+000" RESPONSE_TERMINATOR, 0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_resetPowerExternalUsed(instrument);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("powerexternal used = 0" COMMAND_TERMINATOR, buffers->writeBuffer);
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
        { "info pn = L3-M11-BEC11-SC11-ST11-SP11, fwlock = off" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { "L3-M11-BEC11-SC11-ST11-SP11",
            false } },
        { "info pn = 012345revA, fwlock = on" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { "012345revA",
            true } },
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
        TEST_ASSERT_STR_EQ(tests[i].expected.partNumber, actual.partNumber);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.fwLock, actual.fwLock, bool);
    }
    return true;
}
