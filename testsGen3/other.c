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
    TEST_ASSERT(RBRGen3Version_compare("1.000", "1.000") == 0);
    TEST_ASSERT(RBRGen3Version_compare("1.000", "1X000") > 0);
    TEST_ASSERT(RBRGen3Version_compare("1X000", "1.000") < 0);
    TEST_ASSERT(RBRGen3Version_compare("2.000", "1.000") > 0);
    TEST_ASSERT(RBRGen3Version_compare("1.000", "2.000") < 0);
    TEST_ASSERT(RBRGen3Version_compare("1.200", "1.000") > 0);
    TEST_ASSERT(RBRGen3Version_compare("1.000", "1.200") < 0);
    TEST_ASSERT(RBRGen3Version_compare("1.200", "1X000") > 0);
    TEST_ASSERT(RBRGen3Version_compare("1.200", "1X200") > 0);
    TEST_ASSERT(RBRGen3Version_compare("10.000", "1.000") > 0);
    TEST_ASSERT(RBRGen3Version_compare("1.000", "10.000") < 0);

    /* Invalid versions. */
    TEST_ASSERT(RBRGen3Version_compare(".", ".") == 0);
    TEST_ASSERT(RBRGen3Version_compare(".000", "000.") == 0);
    TEST_ASSERT(RBRGen3Version_compare("0.", "0.000") < 0);
    TEST_ASSERT(RBRGen3Version_compare("000.", "0.000") < 0);
    TEST_ASSERT(RBRGen3Version_compare(".000", "0.000") < 0);

    return true;
}

TEST_LOGGER2(id)
{
    RBRGen3Id expected = {
        .model = "RBRduo",
        .version = "1.440",
        .serial = 912345,
        .fwtype = 103,
        .mode = ""
    };
    RBRGen3Id actual;

    TestIOBuffers_init(buffers,
                       "id model = RBRduo, version = 1.440, "
                       "serial = 912345, fwtype = 103" RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRGen3_getId(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expected.model, actual.model);
    TEST_ASSERT_STR_EQ(expected.version, actual.version);
    TEST_ASSERT_EQ(expected.serial, actual.serial, "%" PRIi32);
    TEST_ASSERT_EQ(expected.fwtype, actual.fwtype, "%" PRIi32);
    TEST_ASSERT_STR_EQ(expected.mode, actual.mode);

    return true;
}

TEST_LOGGER3(id)
{
    RBRGen3Id expected = {
        .model = "RBRduo3",
        .version = "1.092",
        .serial = 923456,
        .fwtype = 104,
        .mode = ""
    };
    RBRGen3Id actual;

    TestIOBuffers_init(buffers,
                       "id model = RBRduo3, version = 1.092, "
                       "serial = 923456, fwtype = 104" RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRGen3_getId(conn, &actual);
    TEST_ASSERT_STR_EQ("id" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expected.model, actual.model);
    TEST_ASSERT_STR_EQ(expected.version, actual.version);
    TEST_ASSERT_EQ(expected.serial, actual.serial, "%" PRIi32);
    TEST_ASSERT_EQ(expected.fwtype, actual.fwtype, "%" PRIi32);
    TEST_ASSERT_STR_EQ(expected.mode, actual.mode);

    return true;
}

TEST_LOGGER3(id_simulated)
{
    RBRGen3Id expected = {
        .model = "RBRduo3",
        .version = "1.092",
        .serial = 923456,
        .fwtype = 104,
        .mode = "SIMULATED"
    };
    RBRGen3Id actual;

    TestIOBuffers_init(buffers,
                       "id mode = SIMULATED, model = RBRduo3, "
                       "version = 1.092, serial = 923456, fwtype = 104"
                       RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRGen3_getId(conn, &actual);
    TEST_ASSERT_STR_EQ("id" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expected.model, actual.model);
    TEST_ASSERT_STR_EQ(expected.version, actual.version);
    TEST_ASSERT_EQ(expected.serial, actual.serial, "%" PRIi32);
    TEST_ASSERT_EQ(expected.fwtype, actual.fwtype, "%" PRIi32);
    TEST_ASSERT_STR_EQ(expected.mode, actual.mode);

    return true;
}

TEST_LOGGER3(id_short)
{
    RBRGen3Id expected = {
        .model = "",
        .version = "",
        .serial = 0,
        .fwtype = 0,
        .mode = ""
    };
    RBRGen3Id actual;

    TestIOBuffers_init(buffers, "id" RESPONSE_TERMINATOR, 0);
    RBRGen3Error err = RBRGen3_getId(conn, &actual);
    TEST_ASSERT_STR_EQ("id" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expected.model, actual.model);
    TEST_ASSERT_STR_EQ(expected.version, actual.version);
    TEST_ASSERT_EQ(expected.serial, actual.serial, "%" PRIi32);
    TEST_ASSERT_EQ(expected.fwtype, actual.fwtype, "%" PRIi32);

    return true;
}

TEST_LOGGER2(hwrev)
{
    RBRGen3HardwareRevision expected = {
        .pcb = 'G',
        .cpu = "5659A",
        .bsl = 'A'
    };
    RBRGen3HardwareRevision actual;

    TestIOBuffers_init(buffers,
                       "hwrev pcb = G, cpu = 5659A, bsl = A"
                       RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRGen3_getHardwareRevision(conn,
                                                               &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(expected.pcb, actual.pcb, "%c");
    TEST_ASSERT_STR_EQ(expected.cpu, actual.cpu);
    TEST_ASSERT_EQ(expected.bsl, actual.bsl, "%c");

    return true;
}

TEST_LOGGER3(hwrev)
{
    RBRGen3HardwareRevision expected = {
        .pcb = 'J',
        .cpu = "5659A",
        .bsl = 'A'
    };
    RBRGen3HardwareRevision actual;

    TestIOBuffers_init(buffers,
                       "hwrev pcb = J, cpu = 5659A, bsl = A"
                       RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRGen3_getHardwareRevision(conn,
                                                               &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(expected.pcb, actual.pcb, "%c");
    TEST_ASSERT_STR_EQ(expected.cpu, actual.cpu);
    TEST_ASSERT_EQ(expected.bsl, actual.bsl, "%c");

    return true;
}

TEST_LOGGER2(powerstatus)
{
    RBRGen3Power expected = {
        .source = RBRGEN3_POWER_SOURCE_USB,
        .internal = 12.4,
        .external = 0,
        .regulator = NAN
    };
    RBRGen3Power actual;

    TestIOBuffers_init(buffers,
                       "powerstatus source = usb, int = 12.40, ext = 0.00, "
                       "capacity = 24.000" RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRGen3_getPower(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(expected.source,
                        actual.source,
                        RBRGen3PowerSource);
    TEST_ASSERT_FLOAT_EQ(expected.internal, actual.internal, 0.0f);
    TEST_ASSERT_FLOAT_EQ(expected.external, actual.external, 0.0f);
    TEST_ASSERT(isnan(actual.regulator));

    return true;
}

TEST_LOGGER3(power)
{
    RBRGen3Power expected = {
        .source = RBRGEN3_POWER_SOURCE_EXTERNAL,
        .internal = 0,
        .external = 11.59,
        .regulator = NAN
    };
    RBRGen3Power actual;

    TestIOBuffers_init(buffers,
                       "power source = ext, int =  0.00, ext = 11.59, "
                       "reg = n/a" RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRGen3_getPower(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(expected.source,
                        actual.source,
                        RBRGen3PowerSource);
    TEST_ASSERT_FLOAT_EQ(expected.internal, actual.internal, 0.0f);
    TEST_ASSERT_FLOAT_EQ(expected.external, actual.external, 0.0f);
    TEST_ASSERT(isnan(actual.regulator));

    return true;
}

TEST_LOGGER2(powerinternal)
{
    RBRGen3PowerInternal actual;
    RBRGen3Error err = RBRGen3_getPowerInternal(conn,
                                                            &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_UNSUPPORTED, err, RBRGen3Error);

    return true;
}

TEST_LOGGER3(powerinternal)
{
    RBRGen3PowerInternal expected = {
        .batteryType = RBRGEN3_INTERNAL_BATTERY_NIMH,
        .capacity = 138000,
        .used = 100100
    };
    RBRGen3PowerInternal actual;

    TestIOBuffers_init(buffers,
                       "powerinternal batterytype = nimh, "
                       "capacity = 138.000e+003, used = 100.100e+003"
                       RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRGen3_getPowerInternal(conn,
                                                            &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(expected.batteryType,
                        actual.batteryType,
                        RBRGen3InternalBatteryType);
    TEST_ASSERT_FLOAT_EQ(expected.capacity, actual.capacity, 0.0f);
    TEST_ASSERT_FLOAT_EQ(expected.used, actual.used, 0.0f);

    return true;
}

TEST_LOGGER2(powerexternal)
{
    RBRGen3PowerExternal actual;
    RBRGen3Error err = RBRGen3_getPowerExternal(conn,
                                                            &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_UNSUPPORTED, err, RBRGen3Error);

    return true;
}

TEST_LOGGER3(powerexternal)
{
    RBRGen3PowerExternal expected = {
        .batteryType = RBRGEN3_EXTERNAL_BATTERY_FERMATA_LISOCL2,
        .capacity = 22000000,
        .used = 100100
    };
    RBRGen3PowerExternal actual;

    TestIOBuffers_init(buffers,
                       " powerexternal batterytype = fermata_lisocl2, "
                       "capacity = 22.000e+006, used = 100.100e+003"
                       RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRGen3_getPowerExternal(conn,
                                                            &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(expected.batteryType,
                        actual.batteryType,
                        RBRGen3ExternalBatteryType);
    TEST_ASSERT_FLOAT_EQ(expected.capacity, actual.capacity, 0.0f);
    TEST_ASSERT_FLOAT_EQ(expected.used, actual.used, 0.0f);

    return true;
}

TEST_LOGGER2(info)
{
    RBRGen3Info actual;

    TestIOBuffers_init(buffers,
                       "E0102 invalid command 'info'" RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRGen3_getInfo(conn,
                                                   &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_UNSUPPORTED, err, RBRGen3Error);

    return true;
}

TEST_LOGGER3(info)
{
    RBRGen3Info expected = {
        .partNumber = "L3-M11-BEC11-SC11-ST11-SP11",
        .fwLock = false
    };
    RBRGen3Info actual;

    TestIOBuffers_init(buffers,
                       "info pn = L3-M11-BEC11-SC11-ST11-SP11"
                       RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRGen3_getInfo(conn,
                                                   &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expected.partNumber, actual.partNumber);
    TEST_ASSERT_ENUM_EQ(expected.fwLock, actual.fwLock, bool);

    return true;
}

/* `info fwlock` was added in fwtype 104, v1.094. */
TEST_LOGGER3(info_fwlock)
{
    RBRGen3Info expected = {
        .partNumber = "L3-M11-F14-BEC11-G1-SCT12-SP11",
        .fwLock = true
    };
    RBRGen3Info actual;

    TestIOBuffers_init(buffers,
                       "info pn = L3-M11-F14-BEC11-G1-SCT12-SP11, fwlock = on"
                       RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRGen3_getInfo(conn,
                                                   &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expected.partNumber, actual.partNumber);
    TEST_ASSERT_ENUM_EQ(expected.fwLock, actual.fwLock, bool);

    return true;
}
