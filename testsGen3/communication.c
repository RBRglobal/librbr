/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file communication.c
 *
 * \brief Tests for instrument communication commands.
 */

#include "tests.h"
#include "RBRGen3Communication.h"

typedef struct LinkTest {
    const char *response;
    RBRGen3Link expected;
} LinkTest;

static bool test_link(RBRGen3 *conn, TestIOBuffers *buffers, LinkTest *tests)
{
    RBRGen3Error err;
    RBRGen3Link actual;

    for (int i = 0; tests[i].response != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_getLink(conn, &actual);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected, actual, RBRGen3Link);
    }

    return true;
}

TEST_LOGGER2(link)
{
    LinkTest tests[] = {
        {
            .response = "link = usb" RESPONSE_TERMINATOR,
            .expected = RBRGEN3_LINK_USB,
        },
        {
            .response = "link = serial" RESPONSE_TERMINATOR,
            .expected = RBRGEN3_LINK_SERIAL,
        },
        {
            .response = "link = wifi" RESPONSE_TERMINATOR,
            .expected = RBRGEN3_LINK_WIFI,
        },
        {0},
    };

    return test_link(conn, buffers, tests);
}

TEST_LOGGER3(link)
{
    LinkTest tests[] = {
        {
            .response = "link type = usb" RESPONSE_TERMINATOR,
            .expected = RBRGEN3_LINK_USB,
        },
        {
            .response = "link type = serial" RESPONSE_TERMINATOR,
            .expected = RBRGEN3_LINK_SERIAL,
        },
        {
            .response = "link type = wifi" RESPONSE_TERMINATOR,
            .expected = RBRGEN3_LINK_WIFI,
        },
        {0},
    };

    return test_link(conn, buffers, tests);
}

typedef struct SerialTest {
    const char *response;
    RBRGen3Serial expected;
} SerialTest;

TEST_LOGGER2(serial)
{
    SerialTest tests[] = {
        {
            .response = "serial baudrate = 19200, mode = rs232" RESPONSE_TERMINATOR,
            .expected =
                {
                    .baudRate = RBRGEN3_SERIAL_BAUD_19200,
                    .mode = RBRGEN3_SERIAL_MODE_RS232,
                    .availableBaudRates = RBRGEN3_SERIAL_BAUD_1200 | RBRGEN3_SERIAL_BAUD_2400 |
                                          RBRGEN3_SERIAL_BAUD_4800 | RBRGEN3_SERIAL_BAUD_9600 |
                                          RBRGEN3_SERIAL_BAUD_19200 | RBRGEN3_SERIAL_BAUD_115200,
                    .availableModes = RBRGEN3_SERIAL_MODE_RS232 | RBRGEN3_SERIAL_MODE_RS485F |
                                      RBRGEN3_SERIAL_MODE_UART | RBRGEN3_SERIAL_MODE_UART_IDLE_LOW,
                },
        },
        {
            .response = "serial baudrate = 115200, mode = rs485f" RESPONSE_TERMINATOR,
            .expected =
                {
                    .baudRate = RBRGEN3_SERIAL_BAUD_115200,
                    .mode = RBRGEN3_SERIAL_MODE_RS485F,
                    .availableBaudRates = RBRGEN3_SERIAL_BAUD_1200 | RBRGEN3_SERIAL_BAUD_2400 |
                                          RBRGEN3_SERIAL_BAUD_4800 | RBRGEN3_SERIAL_BAUD_9600 |
                                          RBRGEN3_SERIAL_BAUD_19200 | RBRGEN3_SERIAL_BAUD_115200,
                    .availableModes = RBRGEN3_SERIAL_MODE_RS232 | RBRGEN3_SERIAL_MODE_RS485F |
                                      RBRGEN3_SERIAL_MODE_UART | RBRGEN3_SERIAL_MODE_UART_IDLE_LOW,
                },
        },
        {0},
    };

    RBRGen3Error err;
    RBRGen3Serial actual;

    for (int i = 0; tests[i].response != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_getSerial(conn, &actual);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.baudRate, actual.baudRate, RBRGen3SerialBaudRate);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.mode, actual.mode, RBRGen3SerialMode);
        TEST_ASSERT_EQ(tests[i].expected.availableBaudRates, actual.availableBaudRates, "0x%04X");
        TEST_ASSERT_EQ(tests[i].expected.availableModes, actual.availableModes, "0x%04X");
    }

    return true;
}

TEST_LOGGER3(serial)
{
    SerialTest tests[] = {
        {
            .response = "serial baudrate = 19200, mode = rs232, availablebaudrates = "
                        "115200|19200|9600|4800|2400|1200|230400|460800, availablemodes = "
                        "rs232|rs485f|uart|uart_idlelow" RESPONSE_TERMINATOR,
            .expected =
                {
                    .baudRate = RBRGEN3_SERIAL_BAUD_19200,
                    .mode = RBRGEN3_SERIAL_MODE_RS232,
                    .availableBaudRates = RBRGEN3_SERIAL_BAUD_1200 | RBRGEN3_SERIAL_BAUD_2400 |
                                          RBRGEN3_SERIAL_BAUD_4800 | RBRGEN3_SERIAL_BAUD_9600 |
                                          RBRGEN3_SERIAL_BAUD_19200 | RBRGEN3_SERIAL_BAUD_115200 |
                                          RBRGEN3_SERIAL_BAUD_230400 | RBRGEN3_SERIAL_BAUD_460800,
                    .availableModes = RBRGEN3_SERIAL_MODE_RS232 | RBRGEN3_SERIAL_MODE_RS485F |
                                      RBRGEN3_SERIAL_MODE_UART | RBRGEN3_SERIAL_MODE_UART_IDLE_LOW,
                },
        },
        {
            .response = "serial baudrate = 115200, mode = rs485f, availablebaudrates = "
                        "115200|19200|9600|4800|2400|1200|230400|460800, availablemodes = "
                        "rs232|rs485f|uart|uart_idlelow" RESPONSE_TERMINATOR,
            .expected =
                {
                    .baudRate = RBRGEN3_SERIAL_BAUD_115200,
                    .mode = RBRGEN3_SERIAL_MODE_RS485F,
                    .availableBaudRates = RBRGEN3_SERIAL_BAUD_1200 | RBRGEN3_SERIAL_BAUD_2400 |
                                          RBRGEN3_SERIAL_BAUD_4800 | RBRGEN3_SERIAL_BAUD_9600 |
                                          RBRGEN3_SERIAL_BAUD_19200 | RBRGEN3_SERIAL_BAUD_115200 |
                                          RBRGEN3_SERIAL_BAUD_230400 | RBRGEN3_SERIAL_BAUD_460800,
                    .availableModes = RBRGEN3_SERIAL_MODE_RS232 | RBRGEN3_SERIAL_MODE_RS485F |
                                      RBRGEN3_SERIAL_MODE_UART | RBRGEN3_SERIAL_MODE_UART_IDLE_LOW,
                },
        },
        {0},
    };

    RBRGen3Error err;
    RBRGen3Serial actual;

    for (int i = 0; tests[i].response != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_getSerial(conn, &actual);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.baudRate, actual.baudRate, RBRGen3SerialBaudRate);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.mode, actual.mode, RBRGen3SerialMode);
        TEST_ASSERT_EQ(tests[i].expected.availableBaudRates, actual.availableBaudRates, "0x%04X");
        TEST_ASSERT_EQ(tests[i].expected.availableModes, actual.availableModes, "0x%04X");
    }

    return true;
}

TEST_LOGGER3(sleep)
{
    TestIOBuffers_init(buffers, "", 0);
    RBRGen3Error err = RBRGen3_sleep(conn);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("sleep" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT(conn->lastActivityTime < 0);

    return true;
}

typedef struct WiFiTest {
    const char *response;
    RBRGen3Error expectedError;
    RBRGen3WiFi expected;
} WiFiTest;

static bool test_wifi(RBRGen3 *conn, TestIOBuffers *buffers, WiFiTest *tests)
{
    RBRGen3Error err;
    RBRGen3WiFi actual;

    for (int i = 0; tests[i].response != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_getWiFi(conn, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.enabled, actual.enabled, bool);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.state, actual.state, RBRGen3WiFiState);
        TEST_ASSERT_EQ(tests[i].expected.powerTimeout, actual.powerTimeout, "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.commandTimeout, actual.commandTimeout, "%" PRIi32);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.baudRate, actual.baudRate, RBRGen3SerialBaudRate);
    }

    return true;
}

TEST_LOGGER2(wifi)
{
    WiFiTest tests[] = {
        {
            .response = "wifi timeout = 60, commandtimeout = 90" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
            .expected =
                {
                    .enabled = false,
                    .state = RBRGEN3_UNKNOWN_WIFI,
                    .powerTimeout = 60000,
                    .commandTimeout = 90000,
                    .baudRate = RBRGEN3_SERIAL_BAUD_NONE,
                },
        },
        {0},
    };

    return test_wifi(conn, buffers, tests);
}

TEST_LOGGER3(wifi)
{
    WiFiTest tests[] = {
        {
            .response = "wifi enabled = false, state = n/a, timeout = 60, "
                        "commandtimeout = 60, baudrate = 921600" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
            .expected =
                {
                    .enabled = false,
                    .state = RBRGEN3_WIFI_NA,
                    .powerTimeout = 60000,
                    .commandTimeout = 60000,
                    .baudRate = RBRGEN3_SERIAL_BAUD_921600,
                },
        },
        {
            .response = "wifi enabled = true, state = off, timeout = 90, "
                        "commandtimeout = 30, baudrate = 921600" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_SUCCESS,
            .expected =
                {
                    .enabled = true,
                    .state = RBRGEN3_WIFI_OFF,
                    .powerTimeout = 90000,
                    .commandTimeout = 30000,
                    .baudRate = RBRGEN3_SERIAL_BAUD_921600,
                },
        },
        {
            .response = "E0109 feature not available" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN3_HARDWARE_ERROR,
            .expected =
                {
                    .enabled = false,
                    .state = RBRGEN3_UNKNOWN_WIFI,
                    .powerTimeout = 0,
                    .commandTimeout = 0,
                    .baudRate = RBRGEN3_SERIAL_BAUD_NONE,
                },
        },
        {0},
    };

    return test_wifi(conn, buffers, tests);
}
