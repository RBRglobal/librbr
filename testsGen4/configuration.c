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
#include "tests.h"

/** \brief Room for the pools and lists the fixtures report. */
#define POOL_SIZE 16

static bool test_node(RBRGen4Node *expected, RBRGen4Node *actual)
{
    TEST_ASSERT_STR_EQ(expected->label, actual->label);
    TEST_ASSERT_STR_EQ(expected->pcba, actual->pcba);
    TEST_ASSERT_EQ(expected->portCount, actual->portCount, "%" PRIi32);
    for (int32_t port = 0; port < expected->portCount; ++port) {
        TEST_ASSERT_STR_EQ(expected->portList[port], actual->portList[port]);
    }
    TEST_ASSERT_STR_EQ(expected->fwversion, actual->fwversion);
    TEST_ASSERT_STR_EQ(expected->semver, actual->semver);
    TEST_ASSERT_EQ(expected->fwtype, actual->fwtype, "%" PRIi32);
    TEST_ASSERT_EQ(expected->powerUpTime, actual->powerUpTime, "%" PRIi32);
    TEST_ASSERT_EQ(expected->inrushOffsetTime, actual->inrushOffsetTime, "%" PRIi32);

    return true;
}

TEST_LOGGER4(nodelist)
{
    /* The pool getter reports only labels; the remaining fields come from
     * RBRGen4_getNode(). */
    RBRGen4NodePool expected = {
        .count = 2,
        .pool =
            {
                {
                    .label = "self",
                },
                {
                    .label = "fe4_minimal_00",
                },
            },
    };
    RBRGen4NodePool actual;

    TestIOBuffers_init(buffers, "node count=2 list=self|fe4_minimal_00" RESPONSE_TERMINATOR, 0);
    RBRGen4Error err = RBRGen4_getNodePool(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("node" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_EQ(expected.count, actual.count, "%" PRIi32);
    for (int32_t node = 0; node < actual.count; ++node) {
        if (!test_node(&expected.pool[node], &actual.pool[node])) {
            return false;
        }
    }

    return true;
}

TEST_LOGGER4(node)
{
    RBRGen4Node expected = {
        .label = "self",
        .pcba = "self",
        .portCount = 6,
        .portList = {"thermistor_00",
                     "pres_serial_00",
                     "internal_adc_00",
                     "serial_00",
                     "serial_01",
                     "serial_02"},
        .fwversion = "2.0.0",
        .semver = "2.0.0-rc2-67-gdc557ad33",
        .fwtype = 150,
        .powerUpTime = 0,
        .inrushOffsetTime = 0,
    };

    RBRGen4Node actual = {
        .label = "self",
    };

    TestIOBuffers_init(buffers,
                       "node self pcba=self portlist=thermistor_00|"
                       "pres_serial_00|internal_adc_00|serial_00|serial_01|"
                       "serial_02 fwversion=2.0.0 semver=2.0.0-rc2-67-"
                       "gdc557ad33 fwtype=150 poweruptime=0 inrushoffsettime=0" RESPONSE_TERMINATOR,
                       0);
    RBRGen4Error err = RBRGen4_getNode(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("node self" COMMAND_TERMINATOR, buffers->writeBuffer);

    return test_node(&expected, &actual);
}

TEST_LOGGER4(nodeWithoutPorts)
{
    /*
     * A front-end node running no firmware of its own reports `na` for its
     * firmware type, and `none` in place of a port list.
     */
    RBRGen4Node expected = {
        .label = "fe4_minimal_00",
        .pcba = "fe4_minimal_00",
        .portCount = 0,
        .fwversion = "00000001",
        .semver = "00000001",
        .fwtype = 0,
        .powerUpTime = 0,
        .inrushOffsetTime = 0,
    };

    RBRGen4Node actual = {
        .label = "fe4_minimal_00",
    };

    TestIOBuffers_init(buffers,
                       "node fe4_minimal_00 pcba=fe4_minimal_00 portlist=none "
                       "fwversion=00000001 semver=00000001 fwtype=na "
                       "poweruptime=0 inrushoffsettime=0" RESPONSE_TERMINATOR,
                       0);
    RBRGen4Error err = RBRGen4_getNode(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("node fe4_minimal_00" COMMAND_TERMINATOR, buffers->writeBuffer);

    return test_node(&expected, &actual);
}

static bool test_port(RBRGen4Port *expected, RBRGen4Port *actual)
{
    TEST_ASSERT_STR_EQ(expected->label, actual->label);
    TEST_ASSERT_STR_EQ(expected->node, actual->node);
    TEST_ASSERT_ENUM_EQ(expected->portClass, actual->portClass, RBRGen4PortClass);
    TEST_ASSERT_EQ(expected->protocol, actual->protocol, "%d");
    TEST_ASSERT_EQ(expected->availableProtocols, actual->availableProtocols, "%d");
    TEST_ASSERT_EQ(expected->baudRate, actual->baudRate, "%" PRIi32);
    TEST_ASSERT_EQ(expected->deviceCount, actual->deviceCount, "%" PRIi32);
    for (int32_t device = 0; device < expected->deviceCount; ++device) {
        TEST_ASSERT_STR_EQ(expected->deviceList[device], actual->deviceList[device]);
    }
    TEST_ASSERT_EQ(expected->powerUpTime, actual->powerUpTime, "%" PRIi32);

    return true;
}

TEST_LOGGER4(portlist)
{
    /* The pool getter reports only labels; the remaining fields come from
     * RBRGen4_getPort(). */
    RBRGen4PortPool expected = {
        .count = 6,
        .pool =
            {
                {
                    .label = "thermistor_00",
                    .portClass = RBRGEN4_UNKNOWN_PORT_CLASS,
                },
                {
                    .label = "pres_serial_00",
                    .portClass = RBRGEN4_UNKNOWN_PORT_CLASS,
                },
                {
                    .label = "internal_adc_00",
                    .portClass = RBRGEN4_UNKNOWN_PORT_CLASS,
                },
                {
                    .label = "serial_00",
                    .portClass = RBRGEN4_UNKNOWN_PORT_CLASS,
                },
                {
                    .label = "serial_01",
                    .portClass = RBRGEN4_UNKNOWN_PORT_CLASS,
                },
                {
                    .label = "serial_02",
                    .portClass = RBRGEN4_UNKNOWN_PORT_CLASS,
                },
            },
    };
    RBRGen4PortPool actual;

    TestIOBuffers_init(buffers,
                       "port count=6 list=thermistor_00|pres_serial_00|"
                       "internal_adc_00|serial_00|serial_01|serial_02" RESPONSE_TERMINATOR,
                       0);
    RBRGen4Error err = RBRGen4_getPortPool(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("port" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_EQ(expected.count, actual.count, "%" PRIi32);
    for (int32_t port = 0; port < actual.count; ++port) {
        if (!test_port(&expected.pool[port], &actual.pool[port])) {
            return false;
        }
    }

    return true;
}

TEST_LOGGER4(port)
{
    RBRGen4Port expected = {
        .label = "thermistor_00",
        .node = "self",
        .portClass = RBRGEN4_PORT_CLASS_VIRTUAL,
        .protocol = RBRGEN4_PORT_PROTOCOL_NONE,
        .availableProtocols = RBRGEN4_PORT_PROTOCOL_NONE,
        .baudRate = 0,
        .deviceCount = 1,
        .deviceList = {"thermistor_00"},
        .powerUpTime = 0,
    };

    RBRGen4Port actual = {
        .label = "thermistor_00",
    };

    TestIOBuffers_init(buffers,
                       "port thermistor_00 class=virtual protocol=none "
                       "availableprotocols=none baudrate=0 node=self "
                       "devicelist=thermistor_00 poweruptime=0" RESPONSE_TERMINATOR,
                       0);
    RBRGen4Error err = RBRGen4_getPort(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("port thermistor_00" COMMAND_TERMINATOR, buffers->writeBuffer);

    return test_port(&expected, &actual);
}

TEST_LOGGER4(portWithoutDevices)
{
    /* A port nothing has been discovered on reports `devicelist=none`. */
    RBRGen4Port expected = {
        .label = "serial_00",
        .node = "self",
        .portClass = RBRGEN4_PORT_CLASS_VIRTUAL,
        .protocol = RBRGEN4_PORT_PROTOCOL_NONE,
        .availableProtocols = RBRGEN4_PORT_PROTOCOL_NONE,
        .baudRate = 0,
        .deviceCount = 0,
        .powerUpTime = 0,
    };

    RBRGen4Port actual = {
        .label = "serial_00",
    };

    TestIOBuffers_init(buffers,
                       "port serial_00 class=virtual protocol=none "
                       "availableprotocols=none baudrate=0 node=self "
                       "devicelist=none poweruptime=0" RESPONSE_TERMINATOR,
                       0);
    RBRGen4Error err = RBRGen4_getPort(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("port serial_00" COMMAND_TERMINATOR, buffers->writeBuffer);

    return test_port(&expected, &actual);
}

TEST_LOGGER4(portSerial)
{
    /*
     * No port on the captured instrument is bus-attached, so unlike the
     * other port tests this response is constructed from the command
     * reference rather than captured. It covers the paths the captured
     * responses cannot reach: a non-`virtual` class, a selected protocol, a
     * multi-valued `availableprotocols` bit field, a non-zero baud rate, and
     * a multidrop bus reporting more than one device.
     */
    RBRGen4Port expected = {
        .label = "serial_01",
        .node = "self",
        .portClass = RBRGEN4_PORT_CLASS_SERIAL,
        .protocol = RBRGEN4_PORT_PROTOCOL_RBRMULTIDROP,
        .availableProtocols = RBRGEN4_PORT_PROTOCOL_RBRSERIAL | RBRGEN4_PORT_PROTOCOL_RBRMODEM |
                              RBRGEN4_PORT_PROTOCOL_RBRMULTIDROP,
        .baudRate = 9600,
        .deviceCount = 2,
        .deviceList = {"cond_cell_00", "pres_sensor_01"},
        .powerUpTime = 50,
    };

    RBRGen4Port actual = {
        .label = "serial_01",
    };

    TestIOBuffers_init(buffers,
                       "port serial_01 class=serial protocol=rbrmultidrop "
                       "availableprotocols=rbrserial|rbrmodem|rbrmultidrop "
                       "baudrate=9600 node=self "
                       "devicelist=cond_cell_00|pres_sensor_01 poweruptime=50" RESPONSE_TERMINATOR,
                       0);
    RBRGen4Error err = RBRGen4_getPort(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("port serial_01" COMMAND_TERMINATOR, buffers->writeBuffer);

    return test_port(&expected, &actual);
}

#define DEVICE_KEYS                                                        \
    " port class sn pn fwversion fwtype name channellist lock poweruptime" \
    " cooldowntime powerdowntime inrushoffsettime"

static bool test_device(RBRGen4Device *expected, RBRGen4Device *actual)
{
    TEST_ASSERT_STR_EQ(expected->label, actual->label);
    TEST_ASSERT_STR_EQ(expected->port, actual->port);
    TEST_ASSERT_ENUM_EQ(expected->deviceClass, actual->deviceClass, RBRGen4DeviceClass);
    TEST_ASSERT_EQ(expected->sn, actual->sn, "%" PRIi32);
    TEST_ASSERT_STR_EQ(expected->pn, actual->pn);
    TEST_ASSERT_STR_EQ(expected->fwversion, actual->fwversion);
    TEST_ASSERT_EQ(expected->fwtype, actual->fwtype, "%" PRIi32);
    TEST_ASSERT_STR_EQ(expected->name, actual->name);
    TEST_ASSERT_EQ(expected->channelCount, actual->channelCount, "%" PRIi32);
    for (int32_t channel = 0; channel < expected->channelCount; ++channel) {
        TEST_ASSERT_STR_EQ(expected->channelList[channel], actual->channelList[channel]);
    }
    TEST_ASSERT_ENUM_EQ(expected->lock, actual->lock, bool);
    TEST_ASSERT_EQ(expected->powerUpTime, actual->powerUpTime, "%" PRIi32);
    TEST_ASSERT_EQ(expected->coolDownTime, actual->coolDownTime, "%" PRIi32);
    TEST_ASSERT_EQ(expected->powerDownTime, actual->powerDownTime, "%" PRIi32);
    TEST_ASSERT_EQ(expected->inrushOffsetTime, actual->inrushOffsetTime, "%" PRIi32);

    return true;
}

TEST_LOGGER4(devicelist)
{
    /* The pool getter reports only labels; the remaining fields come from
     * RBRGen4_getDevice(). */
    RBRGen4DevicePool expected = {
        .count = 3,
        .pool =
            {
                {
                    .label = "thermistor_00",
                    .deviceClass = RBRGEN4_UNKNOWN_DEVICE_CLASS,
                },
                {
                    .label = "pres_sensor_00",
                    .deviceClass = RBRGEN4_UNKNOWN_DEVICE_CLASS,
                },
                {
                    .label = "internal_adc_00",
                    .deviceClass = RBRGEN4_UNKNOWN_DEVICE_CLASS,
                },
            },
    };
    RBRGen4DevicePool actual;

    TestIOBuffers_init(buffers,
                       "device count=3 list=thermistor_00|pres_sensor_00|"
                       "internal_adc_00" RESPONSE_TERMINATOR,
                       0);
    RBRGen4Error err = RBRGen4_getDevicePool(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("device" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_EQ(expected.count, actual.count, "%" PRIi32);
    for (int32_t device = 0; device < actual.count; ++device) {
        if (!test_device(&expected.pool[device], &actual.pool[device])) {
            return false;
        }
    }

    return true;
}

TEST_LOGGER4(discoverDevices)
{
    /* Discovery reports every device present after the sweep, not only the
     * ones it has just added, so the result has the same shape as the pool
     * getter's and carries labels only. */
    const char *expected[] = {
        "thermistor_00",
        "pres_sensor_00",
        "internal_adc_00",
    };
    RBRGen4DevicePool actual;

    TestIOBuffers_init(buffers,
                       "device discover found=thermistor_00|pres_sensor_00|"
                       "internal_adc_00" RESPONSE_TERMINATOR,
                       0);
    RBRGen4Error err = RBRGen4_discoverDevices(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("device discover" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_EQ(3, actual.count, "%" PRIi32);
    for (int32_t device = 0; device < actual.count; ++device) {
        TEST_ASSERT_STR_EQ(expected[device], actual.pool[device].label);
    }

    return true;
}

TEST_LOGGER4(discoverDevicesFindingNothing)
{
    /* An instrument with nothing attached reports `found=none`, which is no
     * devices rather than one called `none`. */
    RBRGen4DevicePool actual;

    TestIOBuffers_init(buffers, "device discover found=none" RESPONSE_TERMINATOR, 0);
    RBRGen4Error err = RBRGen4_discoverDevices(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("device discover" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_EQ(0, actual.count, "%" PRIi32);
    TEST_ASSERT_STR_EQ("", actual.pool[0].label);

    return true;
}

TEST_LOGGER4(device)
{
    RBRGen4Device expected = {
        .label = "thermistor_00",
        .port = "thermistor_00",
        .deviceClass = RBRGEN4_DEVICE_CLASS_SENSOR,
        .sn = 0,
        .pn = "na",
        .fwversion = "0.0.0",
        .fwtype = 0,
        .name = "thermistor",
        .channelCount = 1,
        .channelList = {"temperature_00"},
        .lock = false,
        .powerUpTime = 12,
        .coolDownTime = 0,
        .powerDownTime = 0,
        .inrushOffsetTime = 10,
    };

    RBRGen4Device actual = {
        .label = "thermistor_00",
    };

    TestIOBuffers_init(buffers,
                       "device thermistor_00 port=thermistor_00 class=sensor "
                       "sn=na pn=na fwversion=0.0.0 fwtype=na "
                       "name=thermistor channellist=temperature_00 lock=off "
                       "poweruptime=12 cooldowntime=0 powerdowntime=0 "
                       "inrushoffsettime=10" RESPONSE_TERMINATOR,
                       0);
    RBRGen4Error err = RBRGen4_getDevice(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("device thermistor_00" DEVICE_KEYS COMMAND_TERMINATOR, buffers->writeBuffer);

    return test_device(&expected, &actual);
}

TEST_LOGGER4(deviceIdentity)
{
    /*
     * Zero stands for the `na` every device on the captured instrument
     * reports, so prove a real serial number and firmware type are read
     * rather than left at that zero. The serial number is the one the
     * command reference uses in its worked example; no device to hand
     * reports either.
     */
    RBRGen4Device expected = {
        .label = "internal_adc_00",
        .port = "internal_adc_00",
        .deviceClass = RBRGEN4_DEVICE_CLASS_SENSOR,
        .sn = 850032,
        .pn = "na",
        .fwversion = "1.0.0",
        .fwtype = 170,
        .name = "internal_adc",
        .channelCount = 1,
        .channelList = {"vmon_bat_input_00"},
        .lock = true,
        .powerUpTime = 0,
        .coolDownTime = 0,
        .powerDownTime = 0,
        .inrushOffsetTime = 0,
    };

    RBRGen4Device actual = {
        .label = "internal_adc_00",
    };

    TestIOBuffers_init(buffers,
                       "device internal_adc_00 port=internal_adc_00 "
                       "class=sensor sn=850032 pn=na fwversion=1.0.0 "
                       "fwtype=170 name=internal_adc "
                       "channellist=vmon_bat_input_00 lock=on poweruptime=0 "
                       "cooldowntime=0 powerdowntime=0 inrushoffsettime=0" RESPONSE_TERMINATOR,
                       0);
    RBRGen4Error err = RBRGen4_getDevice(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("device internal_adc_00" DEVICE_KEYS COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return test_device(&expected, &actual);
}

TEST_LOGGER4(deviceWithUnlistedChannels)
{
    /*
     * `channellist` can name channels the `channel` command does not
     * enumerate and will not accept: on the captured instrument this
     * device reports `temperature_01`, which `channel temperature_01`
     * answers with `ERR-117`.
     */
    RBRGen4Device expected = {
        .label = "pres_sensor_00",
        .port = "pres_serial_00",
        .deviceClass = RBRGEN4_DEVICE_CLASS_SENSOR,
        .sn = 0,
        .pn = "na",
        .fwversion = "0.0.0",
        .fwtype = 0,
        .name = "pres_sensor",
        .channelCount = 2,
        .channelList = {"pressure_00", "temperature_01"},
        .lock = false,
        .powerUpTime = 0,
        .coolDownTime = 0,
        .powerDownTime = 0,
        .inrushOffsetTime = 75,
    };

    RBRGen4Device actual = {
        .label = "pres_sensor_00",
    };

    TestIOBuffers_init(buffers,
                       "device pres_sensor_00 port=pres_serial_00 "
                       "class=sensor sn=na pn=na fwversion=0.0.0 fwtype=na "
                       "name=pres_sensor "
                       "channellist=pressure_00|temperature_01 lock=off "
                       "poweruptime=0 cooldowntime=0 powerdowntime=0 "
                       "inrushoffsettime=75" RESPONSE_TERMINATOR,
                       0);
    RBRGen4Error err = RBRGen4_getDevice(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("device pres_sensor_00" DEVICE_KEYS COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return test_device(&expected, &actual);
}

static bool test_calibration(RBRGen4Calibration *expected, RBRGen4Calibration *actual)
{
    TEST_ASSERT_STR_EQ(expected->label, actual->label);
    TEST_ASSERT_STR_EQ(expected->equation, actual->equation);
    TEST_ASSERT_EQ(expected->dateTime, actual->dateTime, "%" PRIi64);
    TEST_ASSERT_FLOAT_EQ(expected->userOffset, actual->userOffset, 1e-6f);
    TEST_ASSERT_FLOAT_EQ(expected->userSlope, actual->userSlope, 1e-6f);
    TEST_ASSERT_EQ(expected->aCount, actual->aCount, "%" PRIi32);
    for (int32_t a = 0; a < expected->aCount; ++a) {
        TEST_ASSERT_FLOAT_EQ(expected->a[a], actual->a[a], 1e-12f);
    }
    TEST_ASSERT_EQ(expected->bCount, actual->bCount, "%" PRIi32);
    for (int32_t b = 0; b < expected->bCount; ++b) {
        TEST_ASSERT_FLOAT_EQ(expected->b[b], actual->b[b], 1e-12f);
    }
    TEST_ASSERT_EQ(expected->mCount, actual->mCount, "%" PRIi32);
    for (int32_t m = 0; m < expected->mCount; ++m) {
        TEST_ASSERT_STR_EQ(expected->m[m], actual->m[m]);
    }

    return true;
}

TEST_LOGGER4(calibration)
{
    RBRGen4Calibration expected = {
        .label = "temperature_00",
        .equation = "temperature",
        .dateTime = 20000101000000,
        .userOffset = 0.0f,
        .userSlope = 1.0f,
        .aCount = 4,
        .a =
            {
                3.50000011e-003f,
                -250.000012e-006f,
                2.70000010e-006f,
                23.0000001e-009f,
            },
        .bCount = 0,
        .mCount = 0,
    };
    RBRGen4Calibration actual = {
        .label = "temperature_00",
    };

    TestIOBuffers_init(
        buffers,
        "calibration temperature_00 equation=temperature "
        "datetime=20000101000000 offset=0 slope=1 a0=3.50000011e-003 "
        "a1=-250.000012e-006 a2=2.70000010e-006 a3=23.0000001e-009" RESPONSE_TERMINATOR,
        0);

    RBRGen4Error err = RBRGen4_getCalibration(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("calibration temperature_00" COMMAND_TERMINATOR, buffers->writeBuffer);

    return test_calibration(&expected, &actual);
}

TEST_LOGGER4(calibrationCrossChannel)
{
    RBRGen4Calibration expected = {
        .label = "depth_00",
        .equation = "deri_depth",
        .dateTime = 20000101000000,
        .userOffset = 0.0f,
        .userSlope = 1.0f,
        .aCount = 0,
        .bCount = 0,
        .mCount = 2,
        .m = {"pressure_00", "param_atmosphere"},
    };
    RBRGen4Calibration actual = {
        .label = "depth_00",
    };

    TestIOBuffers_init(buffers,
                       "calibration depth_00 equation=deri_depth datetime=20000101000000 "
                       "offset=0 slope=1 m0=pressure_00 m1=param_atmosphere" RESPONSE_TERMINATOR,
                       0);

    RBRGen4Error err = RBRGen4_getCalibration(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("calibration depth_00" COMMAND_TERMINATOR, buffers->writeBuffer);

    return test_calibration(&expected, &actual);
}

TEST_LOGGER4(calibrationUnusedReference)
{
    RBRGen4Calibration actual = {
        .label = "temperature_00",
    };

    TestIOBuffers_init(buffers, "calibration temperature_00 m0=none" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_getCalibration(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_EQ(0, actual.mCount, "%" PRIi32);

    return true;
}

TEST_LOGGER4(calibrationEmptyGroup)
{
    RBRGen4Calibration actual = {
        .label = "temperature_00",
    };

    TestIOBuffers_init(buffers, "calibration temperature_00" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_getCalibration(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_EQ(0, actual.aCount, "%" PRIi32);
    TEST_ASSERT_EQ(0, actual.bCount, "%" PRIi32);
    TEST_ASSERT_EQ(0, actual.mCount, "%" PRIi32);

    return true;
}

TEST_LOGGER4(calibrationSet)
{
    RBRGen4Calibration calibration = {
        .label = "temperature_00",
        .dateTime = 20000101000000,
        .userOffset = 0.0f,
        .userSlope = 1.0f,
        .aCount = 4,
        .a =
            {
                3.50000011e-003f,
                -250.000012e-006f,
                2.70000010e-006f,
                23.0000001e-009f,
            },
        .bCount = 0,
    };

    TestIOBuffers_init(buffers,
                       "calibration temperature_00 datetime=20000101000000 offset=0 slope=1 "
                       "a0=3.50000011e-003 a1=-250.000012e-006 a2=2.70000010e-006 "
                       "a3=23.0000001e-009" RESPONSE_TERMINATOR,
                       0);

    RBRGen4Error err = RBRGen4_setCalibration(conn, &calibration);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("calibration temperature_00 datetime=20000101000000 offset=0 slope=1"
                       " a0=0.00350000011 a1=-0.000250000012 a2=2.7000001e-06"
                       " a3=2.30000001e-08" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(calibrationSetCrossChannel)
{
    RBRGen4Calibration calibration = {
        .label = "depth_00",
        .dateTime = 20240101120000,
        .userOffset = 0.0f,
        .userSlope = 1.0f,
        .aCount = 0,
        .bCount = 0,
        .mCount = 2,
        .m = {"pressure_00", "param_atmosphere"},
    };

    TestIOBuffers_init(buffers,
                       "calibration depth_00 datetime=20240101120000 offset=0 slope=1 "
                       "m0=pressure_00 m1=param_atmosphere" RESPONSE_TERMINATOR,
                       0);

    RBRGen4Error err = RBRGen4_setCalibration(conn, &calibration);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("calibration depth_00 datetime=20240101120000 offset=0 slope=1"
                       " m0=pressure_00 m1=param_atmosphere" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(calibrationSetUnusedReference)
{
    RBRGen4Calibration calibration = {
        .label = "depth_00",
        .dateTime = 20240101120000,
        .userOffset = 0.0f,
        .userSlope = 1.0f,
        .mCount = 2,
        .m = {"", "param_atmosphere"},
    };

    TestIOBuffers_init(buffers,
                       "calibration depth_00 datetime=20240101120000 offset=0 slope=1 "
                       "m0=none m1=param_atmosphere" RESPONSE_TERMINATOR,
                       0);

    RBRGen4Error err = RBRGen4_setCalibration(conn, &calibration);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("calibration depth_00 datetime=20240101120000 offset=0 slope=1"
                       " m0=none m1=param_atmosphere" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(calibrationSetInvalidCount)
{
    RBRGen4Calibration calibration = {
        .label = "temperature_00",
        .aCount = RBRGEN4_CALIBRATION_COEFFICIENT_MAX + 1,
    };

    TestIOBuffers_init(buffers, "", 0);

    RBRGen4Error err = RBRGen4_setCalibration(conn, &calibration);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);

    return true;
}

TEST_LOGGER4(settings)
{
    RBRGen4Settings expected = {
        .prompt = true,
        .confirmation = true,
        .pollPowerOffDelay = 8000,
    };
    RBRGen4Settings actual;

    TestIOBuffers_init(
        buffers,
        "settings prompt=on confirmation=on pollpoweroffdelay=8000" RESPONSE_TERMINATOR,
        0);

    RBRGen4Error err = RBRGen4_getSettings(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("settings" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_ENUM_EQ(expected.prompt, actual.prompt, bool);
    TEST_ASSERT_ENUM_EQ(expected.confirmation, actual.confirmation, bool);
    TEST_ASSERT_EQ(expected.pollPowerOffDelay, actual.pollPowerOffDelay, "%" PRIi32);

    return true;
}

TEST_LOGGER4(settingsSet)
{
    RBRGen4Settings settings = {
        .prompt = true,
        .confirmation = true,
        .pollPowerOffDelay = 9000,
    };

    TestIOBuffers_init(
        buffers,
        "settings prompt=on confirmation=on pollpoweroffdelay=9000" RESPONSE_TERMINATOR,
        0);

    RBRGen4Error err = RBRGen4_setSettings(conn, &settings);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ(
        "settings prompt=on confirmation=on pollpoweroffdelay=9000" COMMAND_TERMINATOR,
        buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(settingsSetConfirmationOff)
{
    RBRGen4Settings settings = {
        .prompt = true,
        .confirmation = false,
        .pollPowerOffDelay = 8000,
    };

    TestIOBuffers_init(buffers, "", 0);

    RBRGen4Error err = RBRGen4_setSettings(conn, &settings);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ(
        "settings prompt=on confirmation=off pollpoweroffdelay=8000" COMMAND_TERMINATOR,
        buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(settingsSetInvalidPollPowerOffDelay)
{
    RBRGen4Settings settings = {
        .prompt = true,
        .confirmation = true,
        .pollPowerOffDelay = -1,
    };

    TestIOBuffers_init(buffers, "", 0);

    RBRGen4Error err = RBRGen4_setSettings(conn, &settings);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);

    return true;
}

TEST_LOGGER4(parameters)
{
    RBRGen4Parameters expected = {
        .specCondTempCo = 0.0191f,
        .altitude = 0.0f,
        .temperature = 15.0f,
        .pressure = 10.1325006f,
        .atmosphere = 10.1325006f,
        .density = 1.0260210f,
        .salinity = 35.0f,
        .avgSoundSpeed = 1506.8f,
    };
    RBRGen4Parameters actual;

    TestIOBuffers_init(
        buffers,
        "parameters altitude=0.0000 atmosphere=10.1325006 "
        "avgsoundspeed=1506.8000 density=1.0260210 pressure=10.1325006 "
        "salinity=35.0000 speccondtempco=0.0191 temperature=15.0000" RESPONSE_TERMINATOR,
        0);

    RBRGen4Error err = RBRGen4_getParameters(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("parameters" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_FLOAT_EQ(expected.altitude, actual.altitude, 1e-9f);
    TEST_ASSERT_FLOAT_EQ(expected.atmosphere, actual.atmosphere, 1e-6f);
    TEST_ASSERT_FLOAT_EQ(expected.avgSoundSpeed, actual.avgSoundSpeed, 1e-3f);
    TEST_ASSERT_FLOAT_EQ(expected.density, actual.density, 1e-7f);
    TEST_ASSERT_FLOAT_EQ(expected.pressure, actual.pressure, 1e-6f);
    TEST_ASSERT_FLOAT_EQ(expected.salinity, actual.salinity, 1e-4f);
    TEST_ASSERT_FLOAT_EQ(expected.specCondTempCo, actual.specCondTempCo, 1e-7f);
    TEST_ASSERT_FLOAT_EQ(expected.temperature, actual.temperature, 1e-4f);

    return true;
}

TEST_LOGGER4(parametersSet)
{
    RBRGen4Parameters parameters = {
        .specCondTempCo = 0.0191f,
        .altitude = 0.0f,
        .temperature = 15.0f,
        .pressure = 10.1325006f,
        .atmosphere = 10.1325006f,
        .density = 1.0260210f,
        .salinity = 35.0f,
        .avgSoundSpeed = 1506.8f,
    };

    TestIOBuffers_init(
        buffers,
        "parameters altitude=0.0000 atmosphere=10.1325006 "
        "avgsoundspeed=1506.8000 density=1.0260210 pressure=10.1325006 "
        "salinity=35.0000 speccondtempco=0.0191 temperature=15.0000" RESPONSE_TERMINATOR,
        0);

    RBRGen4Error err = RBRGen4_setParameters(conn, &parameters);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("parameters altitude=0 atmosphere=10.1325006 "
                       "avgsoundspeed=1506.80005 density=1.026021 pressure=10.1325006 "
                       "salinity=35 speccondtempco=0.0190999992 temperature=15" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

static bool test_channel(RBRGen4Channel *expected, RBRGen4Channel *actual)
{
    TEST_ASSERT_STR_EQ(expected->label, actual->label);
    TEST_ASSERT_STR_EQ(expected->type, actual->type);
    TEST_ASSERT_EQ(expected->settlingTime, actual->settlingTime, "%" PRIi32);
    TEST_ASSERT_EQ(expected->measuringTime, actual->measuringTime, "%" PRIi32);
    TEST_ASSERT_EQ(expected->readOutTime, actual->readOutTime, "%" PRIi32);
    TEST_ASSERT_STR_EQ(expected->userUnits, actual->userUnits);
    TEST_ASSERT_ENUM_EQ(expected->nature, actual->nature, RBRGen4ChannelNature);
    TEST_ASSERT_ENUM_EQ(expected->derived, actual->derived, bool);
    TEST_ASSERT_STR_EQ(expected->node, actual->node);
    TEST_ASSERT_STR_EQ(expected->port, actual->port);
    TEST_ASSERT_STR_EQ(expected->device, actual->device);

    return true;
}

TEST_LOGGER4(channellist)
{
    const char *expected[] = {
        "temperature_00",
        "pressure_00",
        "seapressure_00",
        "depth_00",
    };
    RBRGEN4_CHANNEL_POOL_DECL(actual, RBRGEN4_CHANNEL_MAX);

    TestIOBuffers_init(
        buffers,
        "channel count=4 "
        "list=temperature_00|pressure_00|seapressure_00|depth_00" RESPONSE_TERMINATOR,
        0);

    RBRGen4Error err = RBRGen4_getChannelPool(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("channel" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_EQ(4, actual.count, "%" PRIi32);
    for (int32_t channel = 0; channel < actual.count; ++channel) {
        TEST_ASSERT_STR_EQ(expected[channel], actual.pool[channel].label);
    }

    return true;
}

TEST_LOGGER4(channellistTooSmall)
{
    RBRGEN4_CHANNEL_POOL_DECL(actual, 2);

    TestIOBuffers_init(
        buffers,
        "channel count=4 "
        "list=temperature_00|pressure_00|seapressure_00|depth_00" RESPONSE_TERMINATOR,
        0);

    RBRGen4Error err = RBRGen4_getChannelPool(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_TRUNCATED, err, RBRGen4Error);
    TEST_ASSERT_EQ(4, actual.count, "%" PRIi32);
    TEST_ASSERT_STR_EQ("temperature_00", actual.pool[0].label);
    TEST_ASSERT_STR_EQ("pressure_00", actual.pool[1].label);

    return true;
}

TEST_LOGGER4(channellistScientific)
{
    RBRGEN4_CHANNEL_POOL_DECL(actual, RBRGEN4_CHANNEL_MAX);

    TestIOBuffers_init(
        buffers,
        "channel scientific count=4 "
        "list=temperature_00|pressure_00|seapressure_00|depth_00" RESPONSE_TERMINATOR,
        0);

    RBRGen4Error err =
        RBRGen4_getChannelPoolByNature(conn, RBRGEN4_CHANNEL_NATURE_SCIENTIFIC, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("channel scientific" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_EQ(4, actual.count, "%" PRIi32);
    TEST_ASSERT_STR_EQ("temperature_00", actual.pool[0].label);

    return true;
}

TEST_LOGGER4(channellistWithoutChannels)
{
    RBRGEN4_CHANNEL_POOL_DECL(actual, RBRGEN4_CHANNEL_MAX);

    TestIOBuffers_init(buffers, "channel system count=0 list=none" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_getChannelPoolByNature(conn, RBRGEN4_CHANNEL_NATURE_SYSTEM, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("channel system" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_EQ(0, actual.count, "%" PRIi32);

    return true;
}

TEST_LOGGER4(channellistUnknownNature)
{
    RBRGEN4_CHANNEL_POOL_DECL(actual, RBRGEN4_CHANNEL_MAX);

    TestIOBuffers_init(buffers, "", 0);

    RBRGen4Error err =
        RBRGen4_getChannelPoolByNature(conn, RBRGEN4_UNKNOWN_CHANNEL_NATURE, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);

    return true;
}

TEST_LOGGER4(channel)
{
    RBRGen4Channel expected = {
        .label = "temperature_00",
        .type = "temp006",
        .settlingTime = 100,
        .measuringTime = 13,
        .readOutTime = 1,
        .userUnits = "C",
        .nature = RBRGEN4_CHANNEL_NATURE_SCIENTIFIC,
        .derived = false,
        .node = "self",
        .port = "thermistor_00",
        .device = "thermistor_00",
    };
    RBRGen4Channel actual = {
        .label = "temperature_00",
    };

    TestIOBuffers_init(buffers,
                       "channel temperature_00 type=temp006 settlingtime=100 "
                       "measuringtime=13 readouttime=1 userunits=C grouplist=none "
                       "nature=scientific derived=false node=self port=thermistor_00 "
                       "device=thermistor_00" RESPONSE_TERMINATOR,
                       0);

    RBRGen4Error err = RBRGen4_getChannel(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("channel temperature_00" COMMAND_TERMINATOR, buffers->writeBuffer);

    return test_channel(&expected, &actual);
}

TEST_LOGGER4(channelDerived)
{
    RBRGen4Channel expected = {
        .label = "depth_00",
        .type = "dpth001",
        .settlingTime = 0,
        .measuringTime = 0,
        .readOutTime = 0,
        .userUnits = "m",
        .nature = RBRGEN4_CHANNEL_NATURE_SCIENTIFIC,
        .derived = true,
        .node = "",
        .port = "",
        .device = "",
    };
    RBRGen4Channel actual = {
        .label = "depth_00",
    };

    TestIOBuffers_init(
        buffers,
        "channel depth_00 type=dpth001 userunits=m grouplist=none "
        "nature=scientific derived=true node=na port=na device=na" RESPONSE_TERMINATOR,
        0);

    RBRGen4Error err = RBRGen4_getChannel(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("channel depth_00" COMMAND_TERMINATOR, buffers->writeBuffer);

    return test_channel(&expected, &actual);
}

/* Group membership is reported by the instrument but not modelled here: the
 * parameter is skipped. */
TEST_LOGGER4(channelWithGroups)
{
    RBRGen4Channel actual = {
        .label = "temperature_00",
    };

    TestIOBuffers_init(buffers,
                       "channel temperature_00 type=temp006 settlingtime=100 "
                       "measuringtime=13 readouttime=1 userunits=C "
                       "grouplist=surface|profile nature=scientific derived=false "
                       "node=self port=thermistor_00 device=thermistor_00" RESPONSE_TERMINATOR,
                       0);

    RBRGen4Error err = RBRGen4_getChannel(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("temp006", actual.type);
    TEST_ASSERT_STR_EQ("thermistor_00", actual.device);

    return true;
}

TEST_LOGGER4(channelEmptyLabel)
{
    /* An empty label is refused before the command. */
    RBRGen4Channel channel = {
        .label = "",
    };

    TestIOBuffers_init(buffers, "", 0);

    RBRGen4Error err = RBRGen4_getChannel(conn, &channel);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(channelSet)
{
    RBRGen4Channel channel = {
        .label = "temperature_00",
        .userUnits = "C",
    };

    TestIOBuffers_init(buffers, "channel temperature_00 userunits=C" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_setChannel(conn, &channel);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("channel temperature_00 userunits=C" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(channelSetEmptyUserUnits)
{
    RBRGen4Channel channel = {
        .label = "temperature_00",
        .userUnits = "",
    };

    TestIOBuffers_init(buffers, "", 0);

    RBRGen4Error err = RBRGen4_setChannel(conn, &channel);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);

    return true;
}

TEST_LOGGER4(grouplist)
{
    RBRGEN4_GROUP_POOL_DECL(actual, POOL_SIZE);

    TestIOBuffers_init(buffers, "group count=2 maxcount=16 list=g_a|g_b" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_getGroupPool(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("group" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_EQ(2, actual.count, "%" PRIi32);
    TEST_ASSERT_EQ(16, actual.maxCount, "%" PRIi32);
    TEST_ASSERT_STR_EQ("g_a", actual.pool[0].label);
    TEST_ASSERT_STR_EQ("g_b", actual.pool[1].label);

    return true;
}

TEST_LOGGER4(grouplistTooSmall)
{
    RBRGEN4_GROUP_POOL_DECL(actual, 1);

    TestIOBuffers_init(buffers, "group count=2 maxcount=16 list=g_a|g_b" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_getGroupPool(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_TRUNCATED, err, RBRGen4Error);
    TEST_ASSERT_EQ(2, actual.count, "%" PRIi32);
    TEST_ASSERT_STR_EQ("g_a", actual.pool[0].label);

    return true;
}

TEST_LOGGER4(grouplistWithoutGroups)
{
    RBRGEN4_GROUP_POOL_DECL(actual, POOL_SIZE);

    TestIOBuffers_init(buffers, "group count=0 maxcount=16 list=none" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_getGroupPool(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_EQ(0, actual.count, "%" PRIi32);
    TEST_ASSERT_EQ(16, actual.maxCount, "%" PRIi32);
    TEST_ASSERT_STR_EQ("", actual.pool[0].label);

    return true;
}

/* The group's schedules are reported by the instrument but not modelled:
 * the parameter is skipped. */
TEST_LOGGER4(group)
{
    RBRGen4Group group = {
        .label = "g_a",
    };
    RBRGEN4_LABEL_LIST_DECL(channelList, 4);

    TestIOBuffers_init(
        buffers,
        "group g_a channellist=temperature_00|pressure_00 schedulelist=s_a" RESPONSE_TERMINATOR,
        0);

    RBRGen4Error err = RBRGen4_getGroup(conn, &group, &channelList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("group g_a" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_EQ(2, channelList.count, "%" PRIi32);
    TEST_ASSERT_STR_EQ("temperature_00", channelList.labels[0]);
    TEST_ASSERT_STR_EQ("pressure_00", channelList.labels[1]);

    return true;
}

TEST_LOGGER4(groupWithoutChannels)
{
    RBRGen4Group group = {
        .label = "g_b",
    };
    RBRGEN4_LABEL_LIST_DECL(channelList, 4);

    TestIOBuffers_init(
        buffers, "group g_b channellist=none schedulelist=s_a" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_getGroup(conn, &group, &channelList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_EQ(0, channelList.count, "%" PRIi32);

    return true;
}

TEST_LOGGER4(groupWithoutChannelList)
{
    RBRGen4Group group = {
        .label = "g_a",
    };

    TestIOBuffers_init(
        buffers,
        "group g_a channellist=temperature_00|pressure_00 schedulelist=s_a" RESPONSE_TERMINATOR,
        0);

    RBRGen4Error err = RBRGen4_getGroup(conn, &group, NULL);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("group g_a" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(groupChannelListTooSmall)
{
    RBRGen4Group group = {
        .label = "g_a",
    };
    RBRGEN4_LABEL_LIST_DECL(channelList, 1);

    TestIOBuffers_init(
        buffers,
        "group g_a channellist=temperature_00|pressure_00 schedulelist=s_a" RESPONSE_TERMINATOR,
        0);

    RBRGen4Error err = RBRGen4_getGroup(conn, &group, &channelList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_TRUNCATED, err, RBRGen4Error);
    TEST_ASSERT_EQ(2, channelList.count, "%" PRIi32);
    TEST_ASSERT_STR_EQ("temperature_00", channelList.labels[0]);

    return true;
}

TEST_LOGGER4(groupEmptyLabel)
{
    /* An empty label is refused before the command. */
    RBRGen4Group group = {
        .label = "",
    };

    TestIOBuffers_init(buffers, "", 0);

    RBRGen4Error err = RBRGen4_getGroup(conn, &group, NULL);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(groupSet)
{
    RBRGen4Group group = {
        .label = "g_a",
    };
    RBRGen4Label labelBuf[] = {"temperature_00", "pressure_00"};
    RBRGen4LabelList channelList = {
        .size = 2,
        .count = 2,
        .labels = labelBuf,
    };

    TestIOBuffers_init(
        buffers, "group g_a channellist=temperature_00|pressure_00" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_setGroup(conn, &group, &channelList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("group g_a channellist=temperature_00|pressure_00" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(groupSetWithoutChannelList)
{
    /* The setter has nothing to send without a list. */
    RBRGen4Group group = {
        .label = "g_a",
    };

    TestIOBuffers_init(buffers, "", 0);

    RBRGen4Error err = RBRGen4_setGroup(conn, &group, NULL);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(groupSetClearingChannels)
{
    RBRGen4Group group = {
        .label = "g_a",
    };
    RBRGen4Label labelBuf[1];
    RBRGen4LabelList channelList = {
        .size = 1,
        .count = 0,
        .labels = labelBuf,
    };

    TestIOBuffers_init(buffers, "group g_a channellist=none" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_setGroup(conn, &group, &channelList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("group g_a channellist=none" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(groupSetEmptyLabel)
{
    RBRGen4Group group = {
        .label = "",
    };
    RBRGen4Label labelBuf[] = {"temperature_00"};
    RBRGen4LabelList channelList = {
        .size = 1,
        .count = 1,
        .labels = labelBuf,
    };

    TestIOBuffers_init(buffers, "", 0);

    RBRGen4Error err = RBRGen4_setGroup(conn, &group, &channelList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);

    return true;
}

TEST_LOGGER4(groupSetInvalidChannelCount)
{
    /* The count exceeds the array. */
    RBRGen4Group group = {
        .label = "g_a",
    };
    RBRGen4Label labelBuf[] = {"temperature_00", "pressure_00"};
    RBRGen4LabelList channelList = {
        .size = 2,
        .count = 3,
        .labels = labelBuf,
    };

    TestIOBuffers_init(buffers, "", 0);

    RBRGen4Error err = RBRGen4_setGroup(conn, &group, &channelList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);

    return true;
}

TEST_LOGGER4(groupSetEmptyChannelLabel)
{
    /* An empty label would produce a malformed list. */
    RBRGen4Group group = {
        .label = "g_a",
    };
    RBRGen4Label labelBuf[] = {"temperature_00", ""};
    RBRGen4LabelList channelList = {
        .size = 2,
        .count = 2,
        .labels = labelBuf,
    };

    TestIOBuffers_init(buffers, "", 0);

    RBRGen4Error err = RBRGen4_setGroup(conn, &group, &channelList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);

    return true;
}

TEST_LOGGER4(groupCreate)
{
    TestIOBuffers_init(buffers, "group create g_a" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_createGroup(conn, "g_a");
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("group create g_a" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(groupCreateEmptyLabel)
{
    TestIOBuffers_init(buffers, "", 0);

    RBRGen4Error err = RBRGen4_createGroup(conn, "");
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);

    return true;
}

TEST_LOGGER4(groupDelete)
{
    TestIOBuffers_init(buffers, "group delete g_a" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_deleteGroup(conn, "g_a");
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("group delete g_a" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(groupDeleteAll)
{
    TestIOBuffers_init(buffers, "group delete all" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_deleteGroupAll(conn);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("group delete all" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(configlist)
{
    RBRGEN4_CONFIG_POOL_DECL(actual, POOL_SIZE);

    TestIOBuffers_init(buffers, "config count=1 maxcount=2 list=c_a" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_getConfigPool(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("config" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_EQ(1, actual.count, "%" PRIi32);
    TEST_ASSERT_EQ(2, actual.maxCount, "%" PRIi32);
    TEST_ASSERT_STR_EQ("c_a", actual.pool[0].label);

    return true;
}

TEST_LOGGER4(configlistTooSmall)
{
    RBRGEN4_CONFIG_POOL_DECL(actual, 1);

    TestIOBuffers_init(buffers, "config count=2 maxcount=2 list=c_a|c_b" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_getConfigPool(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_TRUNCATED, err, RBRGen4Error);
    TEST_ASSERT_EQ(2, actual.count, "%" PRIi32);
    TEST_ASSERT_STR_EQ("c_a", actual.pool[0].label);

    return true;
}

TEST_LOGGER4(configlistWithoutConfigs)
{
    RBRGEN4_CONFIG_POOL_DECL(actual, POOL_SIZE);

    TestIOBuffers_init(buffers, "config count=0 maxcount=2 list=none" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_getConfigPool(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_EQ(0, actual.count, "%" PRIi32);
    TEST_ASSERT_EQ(2, actual.maxCount, "%" PRIi32);
    TEST_ASSERT_STR_EQ("", actual.pool[0].label);

    return true;
}

TEST_LOGGER4(config)
{
    RBRGen4Config config = {
        .label = "c_a",
    };
    RBRGEN4_LABEL_LIST_DECL(scheduleList, 4);

    TestIOBuffers_init(buffers, "config c_a schedulelist=s_a" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_getConfig(conn, &config, &scheduleList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("config c_a" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_EQ(1, scheduleList.count, "%" PRIi32);
    TEST_ASSERT_STR_EQ("s_a", scheduleList.labels[0]);

    return true;
}

TEST_LOGGER4(configWithoutSchedules)
{
    RBRGen4Config config = {
        .label = "c_a",
    };
    RBRGEN4_LABEL_LIST_DECL(scheduleList, 4);

    TestIOBuffers_init(buffers, "config c_a schedulelist=none" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_getConfig(conn, &config, &scheduleList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_EQ(0, scheduleList.count, "%" PRIi32);

    return true;
}

TEST_LOGGER4(configWithoutScheduleList)
{
    RBRGen4Config config = {
        .label = "c_a",
    };

    TestIOBuffers_init(buffers, "config c_a schedulelist=s_a" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_getConfig(conn, &config, NULL);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("config c_a" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(configScheduleListTooSmall)
{
    RBRGen4Config config = {
        .label = "c_a",
    };
    RBRGEN4_LABEL_LIST_DECL(scheduleList, 1);

    TestIOBuffers_init(buffers, "config c_a schedulelist=s_a|s_b" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_getConfig(conn, &config, &scheduleList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_TRUNCATED, err, RBRGen4Error);
    TEST_ASSERT_EQ(2, scheduleList.count, "%" PRIi32);
    TEST_ASSERT_STR_EQ("s_a", scheduleList.labels[0]);

    return true;
}

TEST_LOGGER4(configEmptyLabel)
{
    /* An empty label is refused before the command. */
    RBRGen4Config config = {
        .label = "",
    };

    TestIOBuffers_init(buffers, "", 0);

    RBRGen4Error err = RBRGen4_getConfig(conn, &config, NULL);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(configSet)
{
    RBRGen4Config config = {
        .label = "cfgPrimary",
    };
    RBRGen4Label labelBuf[] = {"schedule_fast", "schedule_burst"};
    RBRGen4LabelList scheduleList = {
        .size = 2,
        .count = 2,
        .labels = labelBuf,
    };

    TestIOBuffers_init(
        buffers,
        "config cfgPrimary schedulelist=schedule_fast|schedule_burst" RESPONSE_TERMINATOR,
        0);

    RBRGen4Error err = RBRGen4_setConfig(conn, &config, &scheduleList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("config cfgPrimary "
                       "schedulelist=schedule_fast|schedule_burst" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(configSetClearingSchedules)
{
    RBRGen4Config config = {
        .label = "c_a",
    };
    RBRGen4Label labelBuf[1];
    RBRGen4LabelList scheduleList = {
        .size = 1,
        .count = 0,
        .labels = labelBuf,
    };

    TestIOBuffers_init(buffers, "config c_a schedulelist=none" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_setConfig(conn, &config, &scheduleList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("config c_a schedulelist=none" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(configSetEmptyLabel)
{
    RBRGen4Config config = {
        .label = "",
    };
    RBRGen4Label labelBuf[] = {"s_a"};
    RBRGen4LabelList scheduleList = {
        .size = 1,
        .count = 1,
        .labels = labelBuf,
    };

    TestIOBuffers_init(buffers, "", 0);

    RBRGen4Error err = RBRGen4_setConfig(conn, &config, &scheduleList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);

    return true;
}

TEST_LOGGER4(configSetEmptyScheduleLabel)
{
    RBRGen4Config config = {
        .label = "c_a",
    };
    RBRGen4Label labelBuf[] = {"s_a", ""};
    RBRGen4LabelList scheduleList = {
        .size = 2,
        .count = 2,
        .labels = labelBuf,
    };

    TestIOBuffers_init(buffers, "", 0);

    RBRGen4Error err = RBRGen4_setConfig(conn, &config, &scheduleList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);

    return true;
}

TEST_LOGGER4(configCreate)
{
    TestIOBuffers_init(buffers, "config create c_a" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_createConfig(conn, "c_a");
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("config create c_a" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(configDelete)
{
    TestIOBuffers_init(buffers, "config delete c_a" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_deleteConfig(conn, "c_a");
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("config delete c_a" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(configDeleteAll)
{
    TestIOBuffers_init(buffers, "config delete all" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_deleteConfigAll(conn);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("config delete all" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(configDeleteEmptyLabel)
{
    TestIOBuffers_init(buffers, "", 0);

    RBRGen4Error err = RBRGen4_deleteConfig(conn, "");
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);

    return true;
}

TEST_LOGGER4(schedulelist)
{
    RBRGEN4_SCHEDULE_POOL_DECL(actual, POOL_SIZE);

    TestIOBuffers_init(buffers,
                       "schedule count=1 maxcount=8 list=s "
                       "availablemodes=continuous "
                       "availablefastperiods=none maxregimes=3" RESPONSE_TERMINATOR,
                       0);

    RBRGen4Error err = RBRGen4_getSchedulePool(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("schedule" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_EQ(1, actual.count, "%" PRIi32);
    TEST_ASSERT_EQ(8, actual.maxCount, "%" PRIi32);
    TEST_ASSERT_STR_EQ("s", actual.pool[0].label);
    TEST_ASSERT_ENUM_EQ(
        RBRGEN4_SCHEDULE_MODE_CONTINUOUS, actual.availableModes, RBRGen4ScheduleMode);
    TEST_ASSERT_EQ(0, actual.availableFastPeriodCount, "%" PRIi32);
    TEST_ASSERT_EQ(3, actual.maxRegimes, "%" PRIi32);

    return true;
}

TEST_LOGGER4(schedulelistTooSmall)
{
    RBRGEN4_SCHEDULE_POOL_DECL(actual, 1);

    TestIOBuffers_init(buffers,
                       "schedule count=2 maxcount=8 list=s_a|s_b "
                       "availablemodes=continuous "
                       "availablefastperiods=none maxregimes=3" RESPONSE_TERMINATOR,
                       0);

    RBRGen4Error err = RBRGen4_getSchedulePool(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_TRUNCATED, err, RBRGen4Error);
    TEST_ASSERT_EQ(2, actual.count, "%" PRIi32);
    TEST_ASSERT_STR_EQ("s_a", actual.pool[0].label);
    TEST_ASSERT_EQ(3, actual.maxRegimes, "%" PRIi32);

    return true;
}

TEST_LOGGER4(schedulelistEveryMode)
{
    RBRGEN4_SCHEDULE_POOL_DECL(actual, POOL_SIZE);

    TestIOBuffers_init(buffers,
                       "schedule count=1 maxcount=8 list=s "
                       "availablemodes=continuous|average|burst|tide|wave|ddsampling|regimes "
                       "availablefastperiods=500|250|125|63 maxregimes=3" RESPONSE_TERMINATOR,
                       0);

    RBRGen4Error err = RBRGen4_getSchedulePool(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_EQ(RBRGEN4_SCHEDULE_MODE_CONTINUOUS | RBRGEN4_SCHEDULE_MODE_AVERAGE |
                       RBRGEN4_SCHEDULE_MODE_BURST | RBRGEN4_SCHEDULE_MODE_TIDE |
                       RBRGEN4_SCHEDULE_MODE_WAVE | RBRGEN4_SCHEDULE_MODE_DDSAMPLING |
                       RBRGEN4_SCHEDULE_MODE_REGIMES,
                   actual.availableModes,
                   "%d");
    TEST_ASSERT_EQ(4, actual.availableFastPeriodCount, "%" PRIi32);
    TEST_ASSERT_EQ(500, actual.availableFastPeriods[0], "%" PRIi32);
    TEST_ASSERT_EQ(63, actual.availableFastPeriods[3], "%" PRIi32);

    return true;
}

TEST_LOGGER4(schedulelistUnknownMode)
{
    /* An unrecognized mode drops out of the set. */
    RBRGEN4_SCHEDULE_POOL_DECL(actual, POOL_SIZE);

    TestIOBuffers_init(buffers,
                       "schedule count=0 maxcount=8 list=none "
                       "availablemodes=continuous|somethingnew "
                       "availablefastperiods=none maxregimes=3" RESPONSE_TERMINATOR,
                       0);

    RBRGen4Error err = RBRGen4_getSchedulePool(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_ENUM_EQ(
        RBRGEN4_SCHEDULE_MODE_CONTINUOUS, actual.availableModes, RBRGen4ScheduleMode);

    return true;
}

TEST_LOGGER4(schedule)
{
    RBRGen4Schedule actual = {
        .label = "s",
    };

    TestIOBuffers_init(buffers,
                       "schedule s grouplist=none configlist=none stream=off "
                       "castdetection=off mode=continuous period=1000" RESPONSE_TERMINATOR,
                       0);

    RBRGen4Error err = RBRGen4_getSchedule(conn, &actual, NULL);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("schedule s" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SCHEDULE_STREAM_OFF, actual.stream, RBRGen4ScheduleStream);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_UNKNOWN_SCHEDULE_STORAGE, actual.storage, RBRGen4ScheduleStorage);
    TEST_ASSERT_EQ(false, actual.castDetection, "%d");
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SCHEDULE_MODE_CONTINUOUS, actual.mode, RBRGen4ScheduleMode);
    TEST_ASSERT_EQ(1000, actual.parameters.continuous.period, "%" PRIi32);

    return true;
}

/* The schedule's configurations are reported by the instrument but not
 * modelled: the parameter is skipped. */
TEST_LOGGER4(scheduleWithGroupsAndConfigs)
{
    RBRGen4Schedule actual = {
        .label = "s_a",
    };
    RBRGEN4_LABEL_LIST_DECL(groupList, 4);

    TestIOBuffers_init(buffers,
                       "schedule s_a grouplist=g_a|g_b configlist=c_a "
                       "stream=off castdetection=off mode=continuous "
                       "period=1000" RESPONSE_TERMINATOR,
                       0);

    RBRGen4Error err = RBRGen4_getSchedule(conn, &actual, &groupList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_EQ(2, groupList.count, "%" PRIi32);
    TEST_ASSERT_STR_EQ("g_a", groupList.labels[0]);
    TEST_ASSERT_STR_EQ("g_b", groupList.labels[1]);

    return true;
}

TEST_LOGGER4(scheduleGroupListTooSmall)
{
    RBRGen4Schedule actual = {
        .label = "s_a",
    };
    RBRGEN4_LABEL_LIST_DECL(groupList, 1);

    TestIOBuffers_init(buffers,
                       "schedule s_a grouplist=g_a|g_b configlist=c_a "
                       "stream=off castdetection=off mode=continuous "
                       "period=1000" RESPONSE_TERMINATOR,
                       0);

    RBRGen4Error err = RBRGen4_getSchedule(conn, &actual, &groupList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_TRUNCATED, err, RBRGen4Error);
    TEST_ASSERT_EQ(2, groupList.count, "%" PRIi32);
    TEST_ASSERT_STR_EQ("g_a", groupList.labels[0]);
    TEST_ASSERT_EQ(1000, actual.parameters.continuous.period, "%" PRIi32);

    return true;
}

TEST_LOGGER4(scheduleWithStorage)
{
    RBRGen4Schedule actual = {
        .label = "s",
    };

    TestIOBuffers_init(buffers,
                       "schedule s grouplist=none configlist=none stream=off "
                       "storage=off castdetection=off mode=continuous "
                       "period=1000" RESPONSE_TERMINATOR,
                       0);

    RBRGen4Error err = RBRGen4_getSchedule(conn, &actual, NULL);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SCHEDULE_STORAGE_OFF, actual.storage, RBRGen4ScheduleStorage);

    return true;
}

TEST_LOGGER4(scheduleBursting)
{
    RBRGen4Schedule actual = {
        .label = "s_cap",
    };

    TestIOBuffers_init(buffers,
                       "schedule s_cap grouplist=none configlist=none "
                       "stream=usb storage=on castdetection=on mode=average "
                       "period=10000 measurementcount=8 "
                       "measurementperiod=1000" RESPONSE_TERMINATOR,
                       0);

    RBRGen4Error err = RBRGen4_getSchedule(conn, &actual, NULL);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SCHEDULE_STREAM_USB, actual.stream, RBRGen4ScheduleStream);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SCHEDULE_STORAGE_ON, actual.storage, RBRGen4ScheduleStorage);
    TEST_ASSERT_EQ(true, actual.castDetection, "%d");
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SCHEDULE_MODE_AVERAGE, actual.mode, RBRGen4ScheduleMode);
    TEST_ASSERT_EQ(10000, actual.parameters.bursting.period, "%" PRIi32);
    TEST_ASSERT_EQ(8, actual.parameters.bursting.measurementCount, "%" PRIi32);
    TEST_ASSERT_EQ(1000, actual.parameters.bursting.measurementPeriod, "%" PRIi32);

    return true;
}

TEST_LOGGER4(scheduleDeferredMode)
{
    /* The mode is read; its parameters are not modelled. */
    RBRGen4Schedule actual = {
        .label = "s_cap",
    };

    TestIOBuffers_init(buffers,
                       "schedule s_cap grouplist=none configlist=none "
                       "stream=off storage=off castdetection=off "
                       "mode=regimes reference=none direction=ascending "
                       "boundarylist=none binsizelist=none periodlist=none" RESPONSE_TERMINATOR,
                       0);

    RBRGen4Error err = RBRGen4_getSchedule(conn, &actual, NULL);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SCHEDULE_MODE_REGIMES, actual.mode, RBRGen4ScheduleMode);
    TEST_ASSERT_EQ(0, actual.parameters.continuous.period, "%" PRIi32);

    return true;
}

TEST_LOGGER4(scheduleEmptyLabel)
{
    /* An empty label is refused before the command. */
    RBRGen4Schedule schedule = {
        .label = "",
    };

    TestIOBuffers_init(buffers, "", 0);

    RBRGen4Error err = RBRGen4_getSchedule(conn, &schedule, NULL);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(scheduleSet)
{
    RBRGen4Schedule schedule = {
        .label = "s_cap",
        .stream = RBRGEN4_SCHEDULE_STREAM_OFF,
        .storage = RBRGEN4_UNKNOWN_SCHEDULE_STORAGE,
        .castDetection = true,
        .mode = RBRGEN4_SCHEDULE_MODE_CONTINUOUS,
        .parameters =
            {
                .continuous =
                    {
                        .period = 2000,
                    },
            },
    };

    RBRGen4Label labelBuf[] = {"g_test"};
    RBRGen4LabelList groupList = {
        .size = sizeof(labelBuf) / sizeof(labelBuf[0]),
        .count = 1,
        .labels = labelBuf,
    };

    TestIOBuffers_init(buffers,
                       "schedule s_cap grouplist=g_test stream=off "
                       "castdetection=on mode=continuous period=2000" RESPONSE_TERMINATOR,
                       0);

    RBRGen4Error err = RBRGen4_setSchedule(conn, &schedule, &groupList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("schedule s_cap grouplist=g_test stream=off "
                       "castdetection=on mode=continuous period=2000" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(scheduleSetWithoutGroupList)
{
    RBRGen4Schedule schedule = {
        .label = "s_cap",
        .stream = RBRGEN4_SCHEDULE_STREAM_OFF,
        .storage = RBRGEN4_UNKNOWN_SCHEDULE_STORAGE,
        .castDetection = true,
        .mode = RBRGEN4_SCHEDULE_MODE_CONTINUOUS,
        .parameters =
            {
                .continuous =
                    {
                        .period = 2000,
                    },
            },
    };

    TestIOBuffers_init(buffers,
                       "schedule s_cap stream=off castdetection=on "
                       "mode=continuous period=2000" RESPONSE_TERMINATOR,
                       0);

    RBRGen4Error err = RBRGen4_setSchedule(conn, &schedule, NULL);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("schedule s_cap stream=off castdetection=on "
                       "mode=continuous period=2000" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(scheduleSetWithStorage)
{
    RBRGen4Schedule schedule = {
        .label = "s_cap",
        .stream = RBRGEN4_SCHEDULE_STREAM_USB,
        .storage = RBRGEN4_SCHEDULE_STORAGE_ON,
        .castDetection = true,
        .mode = RBRGEN4_SCHEDULE_MODE_AVERAGE,
        .parameters =
            {
                .bursting =
                    {
                        .period = 10000,
                        .measurementCount = 8,
                        .measurementPeriod = 1000,
                    },
            },
    };

    RBRGen4Label labelBuf[1];
    RBRGen4LabelList groupList = {
        .size = 1,
        .count = 0,
        .labels = labelBuf,
    };

    TestIOBuffers_init(buffers,
                       "schedule s_cap grouplist=none stream=usb storage=on "
                       "castdetection=on mode=average period=10000 "
                       "measurementcount=8 measurementperiod=1000" RESPONSE_TERMINATOR,
                       0);

    RBRGen4Error err = RBRGen4_setSchedule(conn, &schedule, &groupList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("schedule s_cap grouplist=none stream=usb storage=on "
                       "castdetection=on mode=average period=10000 "
                       "measurementcount=8 measurementperiod=1000" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(scheduleSetMultipleModes)
{
    /* A multi-flag value compiles but cannot be sent. */
    RBRGen4Schedule schedule = {
        .label = "s_cap",
        .storage = RBRGEN4_UNKNOWN_SCHEDULE_STORAGE,
        .mode = RBRGEN4_SCHEDULE_MODE_CONTINUOUS | RBRGEN4_SCHEDULE_MODE_AVERAGE,
    };

    RBRGen4Label labelBuf[1];
    RBRGen4LabelList groupList = {
        .size = 1,
        .count = 0,
        .labels = labelBuf,
    };

    TestIOBuffers_init(buffers, "", 0);

    RBRGen4Error err = RBRGen4_setSchedule(conn, &schedule, &groupList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);

    return true;
}

TEST_LOGGER4(scheduleSetNoMode)
{
    RBRGen4Schedule schedule = {
        .label = "s_cap",
        .storage = RBRGEN4_UNKNOWN_SCHEDULE_STORAGE,
        .mode = RBRGEN4_SCHEDULE_MODE_NONE,
    };

    RBRGen4Label labelBuf[1];
    RBRGen4LabelList groupList = {
        .size = 1,
        .count = 0,
        .labels = labelBuf,
    };

    TestIOBuffers_init(buffers, "", 0);

    RBRGen4Error err = RBRGen4_setSchedule(conn, &schedule, &groupList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);

    return true;
}

TEST_LOGGER4(scheduleSetDeferredMode)
{
    RBRGen4Schedule schedule = {
        .label = "s_cap",
        .storage = RBRGEN4_UNKNOWN_SCHEDULE_STORAGE,
        .mode = RBRGEN4_SCHEDULE_MODE_REGIMES,
    };

    RBRGen4Label labelBuf[1];
    RBRGen4LabelList groupList = {
        .size = 1,
        .count = 0,
        .labels = labelBuf,
    };

    TestIOBuffers_init(buffers, "", 0);

    RBRGen4Error err = RBRGen4_setSchedule(conn, &schedule, &groupList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_UNSUPPORTED, err, RBRGen4Error);

    return true;
}

TEST_LOGGER4(scheduleSetEmptyGroupLabel)
{
    RBRGen4Schedule schedule = {
        .label = "s_cap",
        .storage = RBRGEN4_UNKNOWN_SCHEDULE_STORAGE,
        .mode = RBRGEN4_SCHEDULE_MODE_CONTINUOUS,
    };

    RBRGen4Label labelBuf[] = {"g_test", ""};
    RBRGen4LabelList groupList = {
        .size = sizeof(labelBuf) / sizeof(labelBuf[0]),
        .count = 2,
        .labels = labelBuf,
    };

    TestIOBuffers_init(buffers, "", 0);

    RBRGen4Error err = RBRGen4_setSchedule(conn, &schedule, &groupList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_INVALID_PARAMETER_VALUE, err, RBRGen4Error);

    return true;
}

TEST_LOGGER4(scheduleCreate)
{
    TestIOBuffers_init(buffers, "schedule create s_cap" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_createSchedule(conn, "s_cap");
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("schedule create s_cap" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(scheduleDelete)
{
    TestIOBuffers_init(buffers, "schedule delete s_cap" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_deleteSchedule(conn, "s_cap");
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("schedule delete s_cap" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(scheduleDeleteAll)
{
    TestIOBuffers_init(buffers, "schedule delete all" RESPONSE_TERMINATOR, 0);

    RBRGen4Error err = RBRGen4_deleteScheduleAll(conn);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("schedule delete all" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(scheduleEveryBurstingMode)
{
    const struct {
        const char *mode;
        RBRGen4ScheduleMode expected;
    } cases[] = {
        {"average", RBRGEN4_SCHEDULE_MODE_AVERAGE},
        {"burst", RBRGEN4_SCHEDULE_MODE_BURST},
        {"tide", RBRGEN4_SCHEDULE_MODE_TIDE},
        {"wave", RBRGEN4_SCHEDULE_MODE_WAVE},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        char response[256];
        snprintf(response,
                 sizeof(response),
                 "schedule s_cap grouplist=none configlist=none stream=off "
                 "castdetection=off mode=%s period=10000 measurementcount=8 "
                 "measurementperiod=1000" RESPONSE_TERMINATOR,
                 cases[i].mode);

        RBRGen4Schedule actual = {
            .label = "s_cap",
        };
        TestIOBuffers_init(buffers, response, 0);

        RBRGen4Error err = RBRGen4_getSchedule(conn, &actual, NULL);
        TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
        TEST_ASSERT_ENUM_EQ(cases[i].expected, actual.mode, RBRGen4ScheduleMode);
        TEST_ASSERT_EQ(10000, actual.parameters.bursting.period, "%" PRIi32);
        TEST_ASSERT_EQ(1000, actual.parameters.bursting.measurementPeriod, "%" PRIi32);
        TEST_ASSERT_EQ(8, actual.parameters.bursting.measurementCount, "%" PRIi32);
    }

    return true;
}

TEST_LOGGER4(scheduleSetEveryBurstingMode)
{
    RBRGen4Schedule schedule = {
        .label = "s_cap",
        .storage = RBRGEN4_UNKNOWN_SCHEDULE_STORAGE,
        .parameters =
            {
                .bursting =
                    {
                        .period = 10000,
                        .measurementPeriod = 1000,
                        .measurementCount = 8,
                    },
            },
    };

    const RBRGen4ScheduleMode modes[] = {
        RBRGEN4_SCHEDULE_MODE_AVERAGE,
        RBRGEN4_SCHEDULE_MODE_BURST,
        RBRGEN4_SCHEDULE_MODE_TIDE,
        RBRGEN4_SCHEDULE_MODE_WAVE,
    };

    RBRGen4Label labelBuf[1];
    RBRGen4LabelList groupList = {
        .size = 1,
        .count = 0,
        .labels = labelBuf,
    };

    for (size_t i = 0; i < sizeof(modes) / sizeof(modes[0]); ++i) {
        schedule.mode = modes[i];
        const char *name = RBRGen4ScheduleMode_name(schedule.mode);

        char expected[256];
        snprintf(expected,
                 sizeof(expected),
                 "schedule s_cap grouplist=none stream=off castdetection=off "
                 "mode=%s period=10000 measurementcount=8 "
                 "measurementperiod=1000" COMMAND_TERMINATOR,
                 name);

        char response[256];
        snprintf(response,
                 sizeof(response),
                 "schedule s_cap grouplist=none stream=off castdetection=off "
                 "mode=%s period=10000 measurementcount=8 "
                 "measurementperiod=1000" RESPONSE_TERMINATOR,
                 name);

        TestIOBuffers_init(buffers, response, 0);

        RBRGen4Error err = RBRGen4_setSchedule(conn, &schedule, &groupList);
        TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
        TEST_ASSERT_STR_EQ(expected, buffers->writeBuffer);
    }

    return true;
}

TEST_LOGGER4(scheduleSetLongParameters)
{
    RBRGen4Schedule schedule = {
        .label = "s",
        .storage = RBRGEN4_UNKNOWN_SCHEDULE_STORAGE,
        .mode = RBRGEN4_SCHEDULE_MODE_AVERAGE,
        .parameters =
            {
                .bursting =
                    {
                        .period = 86400000,
                        .measurementPeriod = 86400000,
                        .measurementCount = 65535,
                    },
            },
    };

    RBRGen4Label labelBuf[1];
    RBRGen4LabelList groupList = {
        .size = 1,
        .count = 0,
        .labels = labelBuf,
    };

    TestIOBuffers_init(buffers,
                       "schedule s grouplist=none stream=off castdetection=off "
                       "mode=average period=86400000 measurementcount=65535 "
                       "measurementperiod=86400000" RESPONSE_TERMINATOR,
                       0);

    RBRGen4Error err = RBRGen4_setSchedule(conn, &schedule, &groupList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("schedule s grouplist=none stream=off castdetection=off "
                       "mode=average period=86400000 measurementcount=65535 "
                       "measurementperiod=86400000" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(scheduleSetCommandTooLong)
{
    /* A command which cannot fit has to be reported, not truncated. */
    RBRGen4Schedule schedule = {
        .label = "s",
        .storage = RBRGEN4_UNKNOWN_SCHEDULE_STORAGE,
        .mode = RBRGEN4_SCHEDULE_MODE_CONTINUOUS,
        .parameters =
            {
                .continuous =
                    {
                        .period = 1000,
                    },
            },
    };

    RBRGen4Label labelBuf[POOL_SIZE];
    RBRGen4LabelList groupList = {
        .size = POOL_SIZE,
        .count = POOL_SIZE,
        .labels = labelBuf,
    };
    for (int32_t i = 0; i < groupList.count; ++i) {
        memset(labelBuf[i], 'g', RBRGEN4_LABEL_NAME_MAX);
        labelBuf[i][RBRGEN4_LABEL_NAME_MAX] = '\0';
    }

    TestIOBuffers_init(buffers, "", 0);

    RBRGen4Error err = RBRGen4_setSchedule(conn, &schedule, &groupList);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_BUFFER_TOO_SMALL, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}
