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
#include "RBRGen3Other.h"

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
        .mode = "",
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
        .mode = "",
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
        .mode = "SIMULATED",
    };
    RBRGen3Id actual;

    TestIOBuffers_init(buffers,
                       "id mode = SIMULATED, model = RBRduo3, "
                       "version = 1.092, serial = 923456, fwtype = 104" RESPONSE_TERMINATOR,
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
        .mode = "",
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
        .bsl = 'A',
    };
    RBRGen3HardwareRevision actual;

    TestIOBuffers_init(buffers, "hwrev pcb = G, cpu = 5659A, bsl = A" RESPONSE_TERMINATOR, 0);
    RBRGen3Error err = RBRGen3_getHardwareRevision(conn, &actual);
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
        .bsl = 'A',
    };
    RBRGen3HardwareRevision actual;

    TestIOBuffers_init(buffers, "hwrev pcb = J, cpu = 5659A, bsl = A" RESPONSE_TERMINATOR, 0);
    RBRGen3Error err = RBRGen3_getHardwareRevision(conn, &actual);
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
        .regulator = NAN,
    };
    RBRGen3Power actual;

    TestIOBuffers_init(buffers,
                       "powerstatus source = usb, int = 12.40, ext = 0.00, "
                       "capacity = 24.000" RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRGen3_getPower(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(expected.source, actual.source, RBRGen3PowerSource);
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
        .regulator = NAN,
    };
    RBRGen3Power actual;

    TestIOBuffers_init(buffers,
                       "power source = ext, int =  0.00, ext = 11.59, "
                       "reg = n/a" RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRGen3_getPower(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(expected.source, actual.source, RBRGen3PowerSource);
    TEST_ASSERT_FLOAT_EQ(expected.internal, actual.internal, 0.0f);
    TEST_ASSERT_FLOAT_EQ(expected.external, actual.external, 0.0f);
    TEST_ASSERT(isnan(actual.regulator));

    return true;
}

/* The connection's buffers are supplied by the caller, so a missing or empty
 * buffer has to be refused before any instrument communication happens, both
 * when the connection is opened and when the buffers are replaced later. */
TEST_LOGGER3(openRejectsInvalidBuffers)
{
    uint8_t commandBuffer[RBRGEN3_COMMAND_BUFFER_DEFAULT];
    uint8_t responseBuffer[RBRGEN3_RESPONSE_BUFFER_DEFAULT];

    struct {
        uint8_t *command;
        int32_t commandCapacity;
        uint8_t *response;
        int32_t responseCapacity;
    } tests[] = {
        {NULL, sizeof(commandBuffer), responseBuffer, sizeof(responseBuffer)},
        {commandBuffer, 0, responseBuffer, sizeof(responseBuffer)},
        {commandBuffer, sizeof(commandBuffer), NULL, sizeof(responseBuffer)},
        {commandBuffer, sizeof(commandBuffer), responseBuffer, 0},
        {commandBuffer, sizeof(commandBuffer), responseBuffer, -1},
        /* Room for a line terminator and nothing else can never hold a
         * response. */
        {commandBuffer, sizeof(commandBuffer), responseBuffer, 2},
    };

    RBRGen3 unopened;
    RBRGen3Error err;

    TestIOBuffers_init(buffers, "", 0);
    err = RBRGen3_open(&unopened, NULL, 0, NULL);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_MISSING_CALLBACK, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    const RBRGen3Environment before = conn->environment;
    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
        RBRGen3Environment environment = conn->environment;
        environment.command = tests[i].command;
        environment.commandCapacity = tests[i].commandCapacity;
        environment.response = tests[i].response;
        environment.responseCapacity = tests[i].responseCapacity;

        TestIOBuffers_init(buffers, "", 0);
        err = RBRGen3_open(&unopened, &environment, 0, NULL);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE, err, RBRGen3Error);
        TEST_ASSERT_STR_EQ("", buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(
            RBRCOMMON_UNKNOWN_GENERATION, RBRGen3_getGeneration(&unopened), RBRCommonGeneration);

        /* A refused replacement leaves the connection on its current
         * buffer. Exactly one of the two buffers is bad in each case, and
         * only its setter is tried, so the good one never displaces the
         * shared connection's own. */
        if (tests[i].command == NULL || tests[i].commandCapacity <= 0) {
            err = RBRGen3_setCommandBuffer(conn, tests[i].command, tests[i].commandCapacity);
        } else {
            err = RBRGen3_setResponseBuffer(conn, tests[i].response, tests[i].responseCapacity);
        }
        TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE, err, RBRGen3Error);
        TEST_ASSERT(conn->environment.command == before.command);
        TEST_ASSERT(conn->environment.commandCapacity == before.commandCapacity);
        TEST_ASSERT(conn->environment.response == before.response);
        TEST_ASSERT(conn->environment.responseCapacity == before.responseCapacity);
    }

    return true;
}

/* An instrument of another generation answers `id` with an error line, and a
 * device which is not an instrument answers with nothing useful; both are
 * reported as an unsupported instrument. A failing callback is the caller's
 * problem and is reported as itself. */
TEST_LOGGER3(openUnsupportedVersusCallbackError)
{
    RBRGen3 unopened;
    RBRGen3Error err;
    uint8_t commandBuffer[RBRGEN3_COMMAND_BUFFER_DEFAULT];
    uint8_t responseBuffer[RBRGEN3_RESPONSE_BUFFER_DEFAULT];
    RBRGen3Environment environment = conn->environment;
    environment.command = commandBuffer;
    environment.commandCapacity = sizeof(commandBuffer);
    environment.response = responseBuffer;
    environment.responseCapacity = sizeof(responseBuffer);

    TestIOBuffers_init(buffers, "E0102 invalid command 'id'" RESPONSE_TERMINATOR, 0);
    err = RBRGen3_open(&unopened, &environment, 0, buffers);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_UNSUPPORTED, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(
        RBRCOMMON_UNKNOWN_GENERATION, RBRGen3_getGeneration(&unopened), RBRCommonGeneration);

    /* The harness reports a read past its scripted data as a callback
     * error. */
    TestIOBuffers_init(buffers, "", 0);
    err = RBRGen3_open(&unopened, &environment, 0, buffers);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_CALLBACK_ERROR, err, RBRGen3Error);

    return true;
}

/* Replacing the buffers carries nothing over from the old ones: the response
 * state is reset as by RBRGen3_resetResponseBuffer() when the response buffer
 * changes, and the next command goes out through the new command buffer. */
TEST_LOGGER3(setCommandAndResponseBuffers)
{
    RBRGen3Error err;
    RBRGen3Power actual;
    const RBRGen3Environment before = conn->environment;
    uint8_t commandBuffer[RBRGEN3_COMMAND_BUFFER_DEFAULT];
    uint8_t responseBuffer[RBRGEN3_RESPONSE_BUFFER_DEFAULT];

    /* Leave a second response buffered behind a hardware error. */
    TestIOBuffers_init(buffers,
                       "E0108 invalid argument to command: 'bogus'" RESPONSE_TERMINATOR
                       "power source = usb, int =  0.00, ext =  0.00, "
                       "reg = n/a" RESPONSE_TERMINATOR,
                       0);
    err = RBRGen3_getPower(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_HARDWARE_ERROR, err, RBRGen3Error);
    TEST_ASSERT(RBRGen3_getLastHardwareErrorMessage(conn) != NULL);

    /* Replacing the command buffer leaves the response state alone. */
    err = RBRGen3_setCommandBuffer(conn, commandBuffer, sizeof(commandBuffer));
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT(conn->environment.command == commandBuffer);
    TEST_ASSERT(conn->environment.commandCapacity == sizeof(commandBuffer));
    TEST_ASSERT_EQ(0, conn->commandBufferLength, "%" PRIi32);
    TEST_ASSERT(RBRGen3_getLastHardwareErrorMessage(conn) != NULL);

    err = RBRGen3_setResponseBuffer(conn, responseBuffer, sizeof(responseBuffer));
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT(conn->environment.response == responseBuffer);
    TEST_ASSERT(conn->environment.responseCapacity == sizeof(responseBuffer));
    TEST_ASSERT_EQ(0, conn->responseBufferLength, "%" PRIi32);
    TEST_ASSERT(RBRGen3_getLastHardwareErrorMessage(conn) == NULL);

    /* The buffered second response is gone with the old buffer, so the reply
     * the instrument sends now answers this command; it was built in the new
     * command buffer. */
    TestIOBuffers_init(buffers,
                       "power source = int, int = 11.59, ext =  0.00, "
                       "reg = n/a" RESPONSE_TERMINATOR,
                       0);
    err = RBRGen3_getPower(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_POWER_SOURCE_INTERNAL, actual.source, RBRGen3PowerSource);
    TEST_ASSERT(memcmp(commandBuffer, "power", sizeof("power") - 1) == 0);

    /* Put the shared connection back on its own buffers. */
    err = RBRGen3_setCommandBuffer(conn, before.command, before.commandCapacity);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    err = RBRGen3_setResponseBuffer(conn, before.response, before.responseCapacity);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);

    return true;
}

/* A response buffer handed between connections carries whatever the previous
 * one left in it, so the reset has to discard the leftovers rather than let
 * the next command parse them as its own. */
TEST_LOGGER3(resetResponseBuffer)
{
    RBRGen3Error err;
    RBRGen3Power actual;

    /* Two responses arrive together: the error satisfies the command and
     * leaves a message pointing into the buffer; the second response stays
     * buffered behind it. */
    TestIOBuffers_init(buffers,
                       "E0108 invalid argument to command: 'bogus'" RESPONSE_TERMINATOR
                       "power source = usb, int =  0.00, ext =  0.00, "
                       "reg = n/a" RESPONSE_TERMINATOR,
                       0);
    err = RBRGen3_getPower(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_HARDWARE_ERROR, err, RBRGen3Error);
    TEST_ASSERT(RBRGen3_getLastHardwareErrorMessage(conn) != NULL);

    RBRGen3_resetResponseBuffer(conn);

    /* The reset drops the message view along with the buffered data. */
    TEST_ASSERT_ENUM_EQ(
        RBRGEN3_HARDWARE_ERROR_NONE, RBRGen3_getLastHardwareError(conn), RBRGen3HardwareError);
    TEST_ASSERT(RBRGen3_getLastHardwareErrorMessage(conn) == NULL);

    /* Without the reset, the buffered second response would answer this
     * command instead of the one the instrument sends now. */
    TestIOBuffers_init(buffers,
                       "power source = int, int = 11.59, ext =  0.00, "
                       "reg = n/a" RESPONSE_TERMINATOR,
                       0);
    err = RBRGen3_getPower(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_POWER_SOURCE_INTERNAL, actual.source, RBRGen3PowerSource);

    return true;
}

TEST_LOGGER2(powerinternal)
{
    RBRGen3PowerInternal actual;
    RBRGen3Error err = RBRGen3_getPowerInternal(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_UNSUPPORTED, err, RBRGen3Error);

    return true;
}

TEST_LOGGER3(powerinternal)
{
    RBRGen3PowerInternal expected = {
        .batteryType = RBRGEN3_INTERNAL_BATTERY_NIMH,
        .capacity = 138000,
        .used = 100100,
    };
    RBRGen3PowerInternal actual;

    TestIOBuffers_init(buffers,
                       "powerinternal batterytype = nimh, "
                       "capacity = 138.000e+003, used = 100.100e+003" RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRGen3_getPowerInternal(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(expected.batteryType, actual.batteryType, RBRGen3InternalBatteryType);
    TEST_ASSERT_FLOAT_EQ(expected.capacity, actual.capacity, 0.0f);
    TEST_ASSERT_FLOAT_EQ(expected.used, actual.used, 0.0f);

    return true;
}

TEST_LOGGER2(powerexternal)
{
    RBRGen3PowerExternal actual;
    RBRGen3Error err = RBRGen3_getPowerExternal(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_UNSUPPORTED, err, RBRGen3Error);

    return true;
}

TEST_LOGGER3(powerexternal)
{
    RBRGen3PowerExternal expected = {
        .batteryType = RBRGEN3_EXTERNAL_BATTERY_FERMATA_LISOCL2,
        .capacity = 22000000,
        .used = 100100,
    };
    RBRGen3PowerExternal actual;

    TestIOBuffers_init(buffers,
                       " powerexternal batterytype = fermata_lisocl2, "
                       "capacity = 22.000e+006, used = 100.100e+003" RESPONSE_TERMINATOR,
                       0);
    RBRGen3Error err = RBRGen3_getPowerExternal(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(expected.batteryType, actual.batteryType, RBRGen3ExternalBatteryType);
    TEST_ASSERT_FLOAT_EQ(expected.capacity, actual.capacity, 0.0f);
    TEST_ASSERT_FLOAT_EQ(expected.used, actual.used, 0.0f);

    return true;
}

TEST_LOGGER2(info)
{
    RBRGen3Info actual;

    TestIOBuffers_init(buffers, "E0102 invalid command 'info'" RESPONSE_TERMINATOR, 0);
    RBRGen3Error err = RBRGen3_getInfo(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_UNSUPPORTED, err, RBRGen3Error);

    return true;
}

TEST_LOGGER3(info)
{
    RBRGen3Info expected = {
        .partNumber = "L3-M11-BEC11-SC11-ST11-SP11",
        .fwLock = false,
    };
    RBRGen3Info actual;

    TestIOBuffers_init(buffers, "info pn = L3-M11-BEC11-SC11-ST11-SP11" RESPONSE_TERMINATOR, 0);
    RBRGen3Error err = RBRGen3_getInfo(conn, &actual);
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
        .fwLock = true,
    };
    RBRGen3Info actual;

    TestIOBuffers_init(
        buffers, "info pn = L3-M11-F14-BEC11-G1-SCT12-SP11, fwlock = on" RESPONSE_TERMINATOR, 0);
    RBRGen3Error err = RBRGen3_getInfo(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expected.partNumber, actual.partNumber);
    TEST_ASSERT_ENUM_EQ(expected.fwLock, actual.fwLock, bool);

    return true;
}
