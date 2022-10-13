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

TEST_LOGGER3(version_comparison)
{
    /* Valid versions. */
    TEST_ASSERT(RBRInstrumentVersion_compare("1.000", "1.000") == 0);
    TEST_ASSERT(RBRInstrumentVersion_compare("1.000", "1X000") > 0);
    TEST_ASSERT(RBRInstrumentVersion_compare("1X000", "1.000") < 0);
    TEST_ASSERT(RBRInstrumentVersion_compare("2.000", "1.000") > 0);
    TEST_ASSERT(RBRInstrumentVersion_compare("1.000", "2.000") < 0);
    TEST_ASSERT(RBRInstrumentVersion_compare("1.200", "1.000") > 0);
    TEST_ASSERT(RBRInstrumentVersion_compare("1.000", "1.200") < 0);
    TEST_ASSERT(RBRInstrumentVersion_compare("1.200", "1X000") > 0);
    TEST_ASSERT(RBRInstrumentVersion_compare("1.200", "1X200") > 0);
    TEST_ASSERT(RBRInstrumentVersion_compare("10.000", "1.000") > 0);
    TEST_ASSERT(RBRInstrumentVersion_compare("1.000", "10.000") < 0);

    /* Invalid versions. */
    TEST_ASSERT(RBRInstrumentVersion_compare(".", ".") == 0);
    TEST_ASSERT(RBRInstrumentVersion_compare(".000", "000.") == 0);
    TEST_ASSERT(RBRInstrumentVersion_compare("0.", "0.000") < 0);
    TEST_ASSERT(RBRInstrumentVersion_compare("000.", "0.000") < 0);
    TEST_ASSERT(RBRInstrumentVersion_compare(".000", "0.000") < 0);

    return true;
}

TEST_LOGGER2(id)
{
    RBRInstrumentId expected = {
        .model = "RBRduo",
        .version = "1.440",
        .serial = 912345,
        .fwtype = 103,
        .mode = ""
    };
    RBRInstrumentId actual;

    TestIOBuffers_init(buffers,
                       "id model = RBRduo, version = 1.440, "
                       "serial = 912345, fwtype = 103" RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getId(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(expected.model, actual.model);
    TEST_ASSERT_STR_EQ(expected.version, actual.version);
    TEST_ASSERT_EQ(expected.serial, actual.serial, "%" PRIi32);
    TEST_ASSERT_EQ(expected.fwtype, actual.fwtype, "%" PRIi32);
    TEST_ASSERT_STR_EQ(expected.mode, actual.mode);

    return true;
}

TEST_LOGGER3(id)
{
    RBRInstrumentId expected = {
        .model = "RBRduo3",
        .version = "1.092",
        .serial = 923456,
        .fwtype = 104,
        .mode = ""
    };
    RBRInstrumentId actual;

    TestIOBuffers_init(buffers,
                       "id model = RBRduo3, version = 1.092, "
                       "serial = 923456, fwtype = 104" RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getId(instrument, &actual);
    TEST_ASSERT_STR_EQ("id" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(expected.model, actual.model);
    TEST_ASSERT_STR_EQ(expected.version, actual.version);
    TEST_ASSERT_EQ(expected.serial, actual.serial, "%" PRIi32);
    TEST_ASSERT_EQ(expected.fwtype, actual.fwtype, "%" PRIi32);
    TEST_ASSERT_STR_EQ(expected.mode, actual.mode);

    return true;
}

TEST_LOGGER3(id_simulated)
{
    RBRInstrumentId expected = {
        .model = "RBRduo3",
        .version = "1.092",
        .serial = 923456,
        .fwtype = 104,
        .mode = "SIMULATED"
    };
    RBRInstrumentId actual;

    TestIOBuffers_init(buffers,
                       "id mode = SIMULATED, model = RBRduo3, "
                       "version = 1.092, serial = 923456, fwtype = 104"
                       RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getId(instrument, &actual);
    TEST_ASSERT_STR_EQ("id" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(expected.model, actual.model);
    TEST_ASSERT_STR_EQ(expected.version, actual.version);
    TEST_ASSERT_EQ(expected.serial, actual.serial, "%" PRIi32);
    TEST_ASSERT_EQ(expected.fwtype, actual.fwtype, "%" PRIi32);
    TEST_ASSERT_STR_EQ(expected.mode, actual.mode);

    return true;
}

TEST_LOGGER3(id_short)
{
    RBRInstrumentId expected = {
        .model = "",
        .version = "",
        .serial = 0,
        .fwtype = 0,
        .mode = ""
    };
    RBRInstrumentId actual;

    TestIOBuffers_init(buffers, "id" RESPONSE_TERMINATOR, 0);
    RBRInstrumentError err = RBRInstrument_getId(instrument, &actual);
    TEST_ASSERT_STR_EQ("id" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(expected.model, actual.model);
    TEST_ASSERT_STR_EQ(expected.version, actual.version);
    TEST_ASSERT_EQ(expected.serial, actual.serial, "%" PRIi32);
    TEST_ASSERT_EQ(expected.fwtype, actual.fwtype, "%" PRIi32);

    return true;
}

TEST_LOGGER2(hwrev)
{
    RBRInstrumentHardwareRevision expected = {
        .pcb = 'G',
        .cpu = "5659A",
        .bsl = 'A'
    };
    RBRInstrumentHardwareRevision actual;

    TestIOBuffers_init(buffers,
                       "hwrev pcb = G, cpu = 5659A, bsl = A"
                       RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getHardwareRevision(instrument,
                                                               &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_EQ(expected.pcb, actual.pcb, "%c");
    TEST_ASSERT_STR_EQ(expected.cpu, actual.cpu);
    TEST_ASSERT_EQ(expected.bsl, actual.bsl, "%c");

    return true;
}

TEST_LOGGER3(hwrev)
{
    RBRInstrumentHardwareRevision expected = {
        .pcb = 'J',
        .cpu = "5659A",
        .bsl = 'A'
    };
    RBRInstrumentHardwareRevision actual;

    TestIOBuffers_init(buffers,
                       "hwrev pcb = J, cpu = 5659A, bsl = A"
                       RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getHardwareRevision(instrument,
                                                               &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_EQ(expected.pcb, actual.pcb, "%c");
    TEST_ASSERT_STR_EQ(expected.cpu, actual.cpu);
    TEST_ASSERT_EQ(expected.bsl, actual.bsl, "%c");

    return true;
}

TEST_LOGGER2(powerstatus)
{
    RBRInstrumentPower expected = {
        .source = RBRINSTRUMENT_POWER_SOURCE_USB,
        .internal = 12.4,
        .external = 0,
        .regulator = NAN
    };
    RBRInstrumentPower actual;

    TestIOBuffers_init(buffers,
                       "powerstatus source = usb, int = 12.40, ext = 0.00, "
                       "capacity = 24.000" RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getPower(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_ENUM_EQ(expected.source,
                        actual.source,
                        RBRInstrumentPowerSource);
    TEST_ASSERT_EQ(expected.internal, actual.internal, "%f");
    TEST_ASSERT_EQ(expected.external, actual.external, "%f");
    TEST_ASSERT(isnan(actual.regulator));

    return true;
}

TEST_LOGGER3(power)
{
    RBRInstrumentPower expected = {
        .source = RBRINSTRUMENT_POWER_SOURCE_EXTERNAL,
        .internal = 0,
        .external = 11.59,
        .regulator = NAN
    };
    RBRInstrumentPower actual;

    TestIOBuffers_init(buffers,
                       "power source = ext, int =  0.00, ext = 11.59, "
                       "reg = n/a" RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getPower(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_ENUM_EQ(expected.source,
                        actual.source,
                        RBRInstrumentPowerSource);
    TEST_ASSERT_EQ(expected.internal, actual.internal, "%f");
    TEST_ASSERT_EQ(expected.external, actual.external, "%f");
    TEST_ASSERT(isnan(actual.regulator));

    return true;
}

TEST_LOGGER2(powerinternal)
{
    RBRInstrumentPowerInternal actual;
    RBRInstrumentError err = RBRInstrument_getPowerInternal(instrument,
                                                            &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_UNSUPPORTED, err, RBRInstrumentError);

    return true;
}

TEST_LOGGER3(powerinternal)
{
    RBRInstrumentPowerInternal expected = {
        .batteryType = RBRINSTRUMENT_INTERNAL_BATTERY_NIMH,
        .capacity = 138000,
        .used = 100100
    };
    RBRInstrumentPowerInternal actual;

    TestIOBuffers_init(buffers,
                       "powerinternal batterytype = nimh, "
                       "capacity = 138.000e+003, used = 100.100e+003"
                       RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getPowerInternal(instrument,
                                                            &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_ENUM_EQ(expected.batteryType,
                        actual.batteryType,
                        RBRInstrumentInternalBatteryType);
    TEST_ASSERT_EQ(expected.capacity, actual.capacity, "%f");
    TEST_ASSERT_EQ(expected.used, actual.used, "%f");

    return true;
}

TEST_LOGGER2(powerexternal)
{
    RBRInstrumentPowerExternal actual;
    RBRInstrumentError err = RBRInstrument_getPowerExternal(instrument,
                                                            &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_UNSUPPORTED, err, RBRInstrumentError);

    return true;
}

TEST_LOGGER3(powerexternal)
{
    RBRInstrumentPowerExternal expected = {
        .batteryType = RBRINSTRUMENT_EXTERNAL_BATTERY_FERMATA_LISOCL2,
        .capacity = 22000000,
        .used = 100100
    };
    RBRInstrumentPowerExternal actual;

    TestIOBuffers_init(buffers,
                       " powerexternal batterytype = fermata_lisocl2, "
                       "capacity = 22.000e+006, used = 100.100e+003"
                       RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getPowerExternal(instrument,
                                                            &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_ENUM_EQ(expected.batteryType,
                        actual.batteryType,
                        RBRInstrumentExternalBatteryType);
    TEST_ASSERT_EQ(expected.capacity, actual.capacity, "%f");
    TEST_ASSERT_EQ(expected.used, actual.used, "%f");

    return true;
}

TEST_LOGGER2(info)
{
    RBRInstrumentInfo actual;

    TestIOBuffers_init(buffers,
                       "E0102 invalid command 'info'" RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getInfo(instrument,
                                                   &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_UNSUPPORTED, err, RBRInstrumentError);

    return true;
}

TEST_LOGGER3(info)
{
    RBRInstrumentInfo expected = {
        .partNumber = "L3-M11-BEC11-SC11-ST11-SP11",
        .fwLock = false
    };
    RBRInstrumentInfo actual;

    TestIOBuffers_init(buffers,
                       "info pn = L3-M11-BEC11-SC11-ST11-SP11"
                       RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getInfo(instrument,
                                                   &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(expected.partNumber, actual.partNumber);
    TEST_ASSERT_ENUM_EQ(expected.fwLock, actual.fwLock, bool);

    return true;
}

/* `info fwlock` was added in fwtype 104, v1.094. */
TEST_LOGGER3(info_fwlock)
{
    RBRInstrumentInfo expected = {
        .partNumber = "L3-M11-F14-BEC11-G1-SCT12-SP11",
        .fwLock = true
    };
    RBRInstrumentInfo actual;

    TestIOBuffers_init(buffers,
                       "info pn = L3-M11-F14-BEC11-G1-SCT12-SP11, fwlock = on"
                       RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getInfo(instrument,
                                                   &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(expected.partNumber, actual.partNumber);
    TEST_ASSERT_ENUM_EQ(expected.fwLock, actual.fwLock, bool);

    return true;
}
