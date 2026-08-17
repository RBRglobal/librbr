/**
 * \file configuration.c
 *
 * \brief Tests for instrument configuration commands.
 *
 * \copyright
 * Copyright (c) 2024 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include <math.h>
#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Configuration.h"
#include "tests.h"

static bool test_node(RBRInstrumentGen4Node *expected,
                      RBRInstrumentGen4Node *actual)
{
    TEST_ASSERT_STR_EQ(expected->label, actual->label);
    TEST_ASSERT_STR_EQ(expected->pcba, actual->pcba);
    TEST_ASSERT_EQ(expected->portCount, actual->portCount, "%" PRIi32);
    for (int32_t port = 0; port < expected->portCount; ++port)
    {
        TEST_ASSERT_STR_EQ(expected->portList[port], actual->portList[port]);
    }
    TEST_ASSERT_STR_EQ(expected->fwVersion, actual->fwVersion);
    TEST_ASSERT_STR_EQ(expected->semver, actual->semver);
    TEST_ASSERT_EQ(expected->fwType, actual->fwType, "%" PRIi32);
    TEST_ASSERT_EQ(expected->powerUpTime, actual->powerUpTime, "%" PRIi32);
    TEST_ASSERT_EQ(expected->inrushOffsetTime,
                   actual->inrushOffsetTime,
                   "%" PRIi32);

    return true;
}

TEST_LOGGER4(nodelist)
{
    /* The pool getter reports only labels; the remaining fields come from
     * RBRInstrumentGen4_getNode(). */
    RBRInstrumentGen4NodePool expected = {
        .count = 2,
        .pool = { { .label = "self" },
                  { .label = "fe4_minimal_00" } }
    };
    RBRInstrumentGen4NodePool actual;

    TestIOBuffers_init(buffers,
                       "node count=2 list=self|fe4_minimal_00"
                       RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_getNodePool(instrument,
                                                               &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("node" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_EQ(expected.count, actual.count, "%" PRIi32);
    for (int32_t node = 0; node < actual.count; ++node)
    {
        if (!test_node(&expected.pool[node], &actual.pool[node]))
        {
            return false;
        }
    }

    return true;
}

TEST_LOGGER4(node)
{
    RBRInstrumentGen4Node expected = {
        .label = "self",
        .pcba = "self",
        .portCount = 6,
        .portList = { "thermistor_00", "pres_serial_00", "internal_adc_00",
                      "serial_00", "serial_01", "serial_02" },
        .fwVersion = "2.0.0",
        .semver = "2.0.0-rc2-67-gdc557ad33",
        .fwType = 150,
        .powerUpTime = 0,
        .inrushOffsetTime = 0
    };

    RBRInstrumentGen4Node actual = {
        .label = "self"
    };

    TestIOBuffers_init(buffers,
                       "node self pcba=self portlist=thermistor_00|"
                       "pres_serial_00|internal_adc_00|serial_00|serial_01|"
                       "serial_02 fwversion=2.0.0 semver=2.0.0-rc2-67-"
                       "gdc557ad33 fwtype=150 poweruptime=0 inrushoffsettime=0"
                       RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_getNode(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("node self" COMMAND_TERMINATOR, buffers->writeBuffer);

    return test_node(&expected, &actual);
}

TEST_LOGGER4(nodeWithoutPorts)
{
    /*
     * A front-end node running no firmware of its own reports `na` for its
     * firmware type, and `none` in place of a port list.
     */
    RBRInstrumentGen4Node expected = {
        .label = "fe4_minimal_00",
        .pcba = "fe4_minimal_00",
        .portCount = 0,
        .fwVersion = "00000001",
        .semver = "00000001",
        .fwType = 0,
        .powerUpTime = 0,
        .inrushOffsetTime = 0
    };

    RBRInstrumentGen4Node actual = {
        .label = "fe4_minimal_00"
    };

    TestIOBuffers_init(buffers,
                       "node fe4_minimal_00 pcba=fe4_minimal_00 portlist=none "
                       "fwversion=00000001 semver=00000001 fwtype=na "
                       "poweruptime=0 inrushoffsettime=0"
                       RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_getNode(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("node fe4_minimal_00" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return test_node(&expected, &actual);
}

static bool test_port(RBRInstrumentGen4Port *expected,
                      RBRInstrumentGen4Port *actual)
{
    TEST_ASSERT_STR_EQ(expected->label, actual->label);
    TEST_ASSERT_STR_EQ(expected->node, actual->node);
    TEST_ASSERT_ENUM_EQ(expected->portClass,
                        actual->portClass,
                        RBRInstrumentGen4PortClass);
    TEST_ASSERT_EQ(expected->protocol, actual->protocol, "%d");
    TEST_ASSERT_EQ(expected->availableProtocols,
                   actual->availableProtocols,
                   "%d");
    TEST_ASSERT_EQ(expected->baudRate, actual->baudRate, "%" PRIi32);
    TEST_ASSERT_EQ(expected->deviceCount, actual->deviceCount, "%" PRIi32);
    for (int32_t device = 0; device < expected->deviceCount; ++device)
    {
        TEST_ASSERT_STR_EQ(expected->deviceList[device],
                           actual->deviceList[device]);
    }
    TEST_ASSERT_EQ(expected->powerUpTime, actual->powerUpTime, "%" PRIi32);

    return true;
}

TEST_LOGGER4(portlist)
{
    /* The pool getter reports only labels; the remaining fields come from
     * RBRInstrumentGen4_getPort(). */
    RBRInstrumentGen4PortPool expected = {
        .count = 6,
        .pool = {
            { .label = "thermistor_00",
              .portClass = RBRINSTRUMENTGEN4_UNKNOWN_PORT_CLASS },
            { .label = "pres_serial_00",
              .portClass = RBRINSTRUMENTGEN4_UNKNOWN_PORT_CLASS },
            { .label = "internal_adc_00",
              .portClass = RBRINSTRUMENTGEN4_UNKNOWN_PORT_CLASS },
            { .label = "serial_00",
              .portClass = RBRINSTRUMENTGEN4_UNKNOWN_PORT_CLASS },
            { .label = "serial_01",
              .portClass = RBRINSTRUMENTGEN4_UNKNOWN_PORT_CLASS },
            { .label = "serial_02",
              .portClass = RBRINSTRUMENTGEN4_UNKNOWN_PORT_CLASS }
        }
    };
    RBRInstrumentGen4PortPool actual;

    TestIOBuffers_init(buffers,
                       "port count=6 list=thermistor_00|pres_serial_00|"
                       "internal_adc_00|serial_00|serial_01|serial_02"
                       RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_getPortPool(instrument,
                                                               &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("port" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_EQ(expected.count, actual.count, "%" PRIi32);
    for (int32_t port = 0; port < actual.count; ++port)
    {
        if (!test_port(&expected.pool[port], &actual.pool[port]))
        {
            return false;
        }
    }

    return true;
}

TEST_LOGGER4(port)
{
    RBRInstrumentGen4Port expected = {
        .label = "thermistor_00",
        .node = "self",
        .portClass = RBRINSTRUMENTGEN4_PORT_CLASS_VIRTUAL,
        .protocol = RBRINSTRUMENTGEN4_PORT_PROTOCOL_NONE,
        .availableProtocols = RBRINSTRUMENTGEN4_PORT_PROTOCOL_NONE,
        .baudRate = 0,
        .deviceCount = 1,
        .deviceList = { "thermistor_00" },
        .powerUpTime = 0
    };

    RBRInstrumentGen4Port actual = {
        .label = "thermistor_00"
    };

    TestIOBuffers_init(buffers,
                       "port thermistor_00 class=virtual protocol=none "
                       "availableprotocols=none baudrate=0 node=self "
                       "devicelist=thermistor_00 poweruptime=0"
                       RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_getPort(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("port thermistor_00" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return test_port(&expected, &actual);
}

TEST_LOGGER4(portWithoutDevices)
{
    /* A port nothing has been discovered on reports `devicelist=none`. */
    RBRInstrumentGen4Port expected = {
        .label = "serial_00",
        .node = "self",
        .portClass = RBRINSTRUMENTGEN4_PORT_CLASS_VIRTUAL,
        .protocol = RBRINSTRUMENTGEN4_PORT_PROTOCOL_NONE,
        .availableProtocols = RBRINSTRUMENTGEN4_PORT_PROTOCOL_NONE,
        .baudRate = 0,
        .deviceCount = 0,
        .powerUpTime = 0
    };

    RBRInstrumentGen4Port actual = {
        .label = "serial_00"
    };

    TestIOBuffers_init(buffers,
                       "port serial_00 class=virtual protocol=none "
                       "availableprotocols=none baudrate=0 node=self "
                       "devicelist=none poweruptime=0"
                       RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_getPort(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("port serial_00" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return test_port(&expected, &actual);
}

TEST_LOGGER4(portSerial)
{
    /*
     * No port on the development instrument is bus-attached, so unlike the
     * other port tests this response is constructed from the command
     * reference rather than captured. It covers the paths the captured
     * responses cannot reach: a non-`virtual` class, a selected protocol, a
     * multi-valued `availableprotocols` bit field, a non-zero baud rate, and
     * a multidrop bus reporting more than one device.
     */
    RBRInstrumentGen4Port expected = {
        .label = "serial_01",
        .node = "self",
        .portClass = RBRINSTRUMENTGEN4_PORT_CLASS_SERIAL,
        .protocol = RBRINSTRUMENTGEN4_PORT_PROTOCOL_RBRMULTIDROP,
        .availableProtocols = RBRINSTRUMENTGEN4_PORT_PROTOCOL_RBRSERIAL
                              | RBRINSTRUMENTGEN4_PORT_PROTOCOL_RBRMODEM
                              | RBRINSTRUMENTGEN4_PORT_PROTOCOL_RBRMULTIDROP,
        .baudRate = 9600,
        .deviceCount = 2,
        .deviceList = { "cond_cell_00", "pres_sensor_01" },
        .powerUpTime = 50
    };

    RBRInstrumentGen4Port actual = {
        .label = "serial_01"
    };

    TestIOBuffers_init(buffers,
                       "port serial_01 class=serial protocol=rbrmultidrop "
                       "availableprotocols=rbrserial|rbrmodem|rbrmultidrop "
                       "baudrate=9600 node=self "
                       "devicelist=cond_cell_00|pres_sensor_01 poweruptime=50"
                       RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_getPort(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("port serial_01" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return test_port(&expected, &actual);
}

bool test_calibration(RBRInstrumentGen4Calibration *expected,
                         RBRInstrumentGen4Calibration *actual)
{
    TEST_ASSERT_EQ(expected->dateTime, actual->dateTime, "%" PRIi64);
    TEST_ASSERT_FLOAT_EQ(expected->userOffset, actual->userOffset, 1e-6f);
    TEST_ASSERT_FLOAT_EQ(expected->userSlope, actual->userSlope, 1e-6f);
    for (uint32_t c = 0; c < RBRINSTRUMENTGEN4_CALIBRATION_C_COEFFICIENT_MAX; c++)
    {
        if (isnan(expected->c[c]))
        {
            TEST_ASSERT(isnan(actual->c[c]));
            break;
        }
        TEST_ASSERT_FLOAT_EQ(expected->c[c], actual->c[c], 1e-6f);
    }
    for (uint32_t x = 0; x < RBRINSTRUMENTGEN4_CALIBRATION_X_COEFFICIENT_MAX; x++)
    {
        if (isnan(expected->x[x]))
        {
            TEST_ASSERT(isnan(actual->x[x]));
            break;
        }
        TEST_ASSERT_FLOAT_EQ(expected->x[x], actual->x[x], 1e-6f);
    }
    /*
     * RBRInstrumentGen4Calibration.n holds pointers to the input channels, not
     * coefficient strings, so compare them as pointers. Nothing populates them
     * yet, which is why the string comparison this replaces dereferenced NULL.
     */
    for (uint32_t n = 0; n < RBRINSTRUMENTGEN4_CALIBRATION_N_COEFFICIENT_MAX; n++)
    {
        TEST_ASSERT_EQ(expected->n[n], actual->n[n], "%p");
    }
    return true;
}

TEST_LOGGER4(calibration)
{
    RBRInstrumentGen4Calibration expected = {
        .dateTime = 20171218175005,
        .userOffset = 0.0000000e+000,
        .userSlope = 1.0000000e+000,
        .c = {9.9876543e+000, 7.5642301e+000, NAN},
        .x = {NAN},
        .n = {NULL}
    };

    RBRInstrumentGen4Channel channel = {
        .label = "voltage_00"
    };
    RBRInstrumentGen4Calibration actual = {
        .parent = &channel
    };

    RBRInstrumentGen4Error err;

    TestIOBuffers_init(buffers,
                       "calibration voltage_00 equation=lin datetime=20171218175005 offset=0.0000000e+000 slope=1.0000000e+000 c0=9.9876543e+000 c1=7.5642301e+000" RESPONSE_TERMINATOR,
                       0);

    err = RBRInstrumentGen4_getCalibration(instrument,
                                           &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);

    return test_calibration(&expected, &actual);
}
