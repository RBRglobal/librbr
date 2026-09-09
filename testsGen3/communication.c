/**
 * \file communication.c
 *
 * \brief Tests for instrument communication commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include "tests.h"

typedef struct LinkTest
{
    const char *response;
    RBRGen3Link expected;
} LinkTest;

static bool test_link(RBRGen3 *instrument,
                      TestIOBuffers *buffers,
                      LinkTest *tests)
{
    RBRGen3Error err;
    RBRGen3Link actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_getLink(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected, actual, RBRGen3Link);
    }

    return true;
}

TEST_LOGGER2(link)
{
    LinkTest tests[] = {
        {"link = usb" RESPONSE_TERMINATOR, RBRGEN3_LINK_USB},
        {"link = serial" RESPONSE_TERMINATOR, RBRGEN3_LINK_SERIAL},
        {"link = wifi" RESPONSE_TERMINATOR, RBRGEN3_LINK_WIFI},
        {0}
    };

    return test_link(instrument, buffers, tests);
}

TEST_LOGGER3(link)
{
    LinkTest tests[] = {
        {"link type = usb" RESPONSE_TERMINATOR, RBRGEN3_LINK_USB},
        {"link type = serial" RESPONSE_TERMINATOR, RBRGEN3_LINK_SERIAL},
        {"link type = wifi" RESPONSE_TERMINATOR, RBRGEN3_LINK_WIFI},
        {0}
    };

    return test_link(instrument, buffers, tests);
}

typedef struct SerialTest
{
    const char *response;
    RBRGen3Serial expected;
} SerialTest;

TEST_LOGGER2(serial)
{
    SerialTest tests[] = {
        {
            "serial baudrate = 19200, mode = rs232" RESPONSE_TERMINATOR,
            {
                RBRGEN3_SERIAL_BAUD_19200,
                RBRGEN3_SERIAL_MODE_RS232,
                RBRGEN3_SERIAL_BAUD_1200
                | RBRGEN3_SERIAL_BAUD_2400
                | RBRGEN3_SERIAL_BAUD_4800
                | RBRGEN3_SERIAL_BAUD_9600
                | RBRGEN3_SERIAL_BAUD_19200
                | RBRGEN3_SERIAL_BAUD_115200,
                RBRGEN3_SERIAL_MODE_RS232
                | RBRGEN3_SERIAL_MODE_RS485F
                | RBRGEN3_SERIAL_MODE_UART
                | RBRGEN3_SERIAL_MODE_UART_IDLE_LOW
            }
        },
        {
            "serial baudrate = 115200, mode = rs485f" RESPONSE_TERMINATOR,
            {
                RBRGEN3_SERIAL_BAUD_115200,
                RBRGEN3_SERIAL_MODE_RS485F,
                RBRGEN3_SERIAL_BAUD_1200
                | RBRGEN3_SERIAL_BAUD_2400
                | RBRGEN3_SERIAL_BAUD_4800
                | RBRGEN3_SERIAL_BAUD_9600
                | RBRGEN3_SERIAL_BAUD_19200
                | RBRGEN3_SERIAL_BAUD_115200,
                RBRGEN3_SERIAL_MODE_RS232
                | RBRGEN3_SERIAL_MODE_RS485F
                | RBRGEN3_SERIAL_MODE_UART
                | RBRGEN3_SERIAL_MODE_UART_IDLE_LOW
            }
        },
        {0}
    };

    RBRGen3Error err;
    RBRGen3Serial actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_getSerial(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.baudRate,
                            actual.baudRate,
                            RBRGen3SerialBaudRate);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.mode,
                            actual.mode,
                            RBRGen3SerialMode);
        TEST_ASSERT_EQ(tests[i].expected.availableBaudRates,
                       actual.availableBaudRates,
                       "0x%04X");
        TEST_ASSERT_EQ(tests[i].expected.availableModes,
                       actual.availableModes,
                       "0x%04X");
    }

    return true;
}

TEST_LOGGER3(serial)
{
    SerialTest tests[] = {
        {
            "serial baudrate = 19200, mode = rs232, availablebaudrates = "
            "115200|19200|9600|4800|2400|1200|230400|460800, availablemodes = "
            "rs232|rs485f|uart|uart_idlelow" RESPONSE_TERMINATOR,
            {
                RBRGEN3_SERIAL_BAUD_19200,
                RBRGEN3_SERIAL_MODE_RS232,
                RBRGEN3_SERIAL_BAUD_1200
                | RBRGEN3_SERIAL_BAUD_2400
                | RBRGEN3_SERIAL_BAUD_4800
                | RBRGEN3_SERIAL_BAUD_9600
                | RBRGEN3_SERIAL_BAUD_19200
                | RBRGEN3_SERIAL_BAUD_115200
                | RBRGEN3_SERIAL_BAUD_230400
                | RBRGEN3_SERIAL_BAUD_460800,
                RBRGEN3_SERIAL_MODE_RS232
                | RBRGEN3_SERIAL_MODE_RS485F
                | RBRGEN3_SERIAL_MODE_UART
                | RBRGEN3_SERIAL_MODE_UART_IDLE_LOW
            }
        },
        {
            "serial baudrate = 115200, mode = rs485f, availablebaudrates = "
            "115200|19200|9600|4800|2400|1200|230400|460800, availablemodes = "
            "rs232|rs485f|uart|uart_idlelow" RESPONSE_TERMINATOR,
            {
                RBRGEN3_SERIAL_BAUD_115200,
                RBRGEN3_SERIAL_MODE_RS485F,
                RBRGEN3_SERIAL_BAUD_1200
                | RBRGEN3_SERIAL_BAUD_2400
                | RBRGEN3_SERIAL_BAUD_4800
                | RBRGEN3_SERIAL_BAUD_9600
                | RBRGEN3_SERIAL_BAUD_19200
                | RBRGEN3_SERIAL_BAUD_115200
                | RBRGEN3_SERIAL_BAUD_230400
                | RBRGEN3_SERIAL_BAUD_460800,
                RBRGEN3_SERIAL_MODE_RS232
                | RBRGEN3_SERIAL_MODE_RS485F
                | RBRGEN3_SERIAL_MODE_UART
                | RBRGEN3_SERIAL_MODE_UART_IDLE_LOW
            }
        },
        {0}
    };

    RBRGen3Error err;
    RBRGen3Serial actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_getSerial(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.baudRate,
                            actual.baudRate,
                            RBRGen3SerialBaudRate);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.mode,
                            actual.mode,
                            RBRGen3SerialMode);
        TEST_ASSERT_EQ(tests[i].expected.availableBaudRates,
                       actual.availableBaudRates,
                       "0x%04X");
        TEST_ASSERT_EQ(tests[i].expected.availableModes,
                       actual.availableModes,
                       "0x%04X");
    }

    return true;
}

TEST_LOGGER3(sleep)
{
    TestIOBuffers_init(buffers, "", 0);
    RBRGen3Error err = RBRGen3_sleep(instrument);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("sleep" COMMAND_TERMINATOR,buffers->writeBuffer);
    TEST_ASSERT(instrument->lastActivityTime < 0);

    return true;
}

typedef struct WiFiTest
{
    const char *response;
    RBRGen3Error expectedError;
    RBRGen3WiFi expected;
} WiFiTest;

static bool test_wifi(RBRGen3 *instrument,
                      TestIOBuffers *buffers,
                      WiFiTest *tests)
{
    RBRGen3Error err;
    RBRGen3WiFi actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_getWiFi(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.enabled, actual.enabled, bool);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.state,
                            actual.state,
                            RBRGen3WiFiState);
        TEST_ASSERT_EQ(tests[i].expected.powerTimeout,
                       actual.powerTimeout,
                       "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.commandTimeout,
                       actual.commandTimeout,
                       "%" PRIi32);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.baudRate,
                            actual.baudRate,
                            RBRGen3SerialBaudRate);
    }

    return true;
}

TEST_LOGGER2(wifi)
{
    WiFiTest tests[] = {
        {
            "wifi timeout = 60, commandtimeout = 90" RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS,
            {
                false,
                RBRGEN3_UNKNOWN_WIFI,
                60000,
                90000,
                RBRGEN3_SERIAL_BAUD_NONE
            }
        },
        {0}
    };

    return test_wifi(instrument, buffers, tests);
}

TEST_LOGGER3(wifi)
{
    WiFiTest tests[] = {
        {
            "wifi enabled = false, state = n/a, timeout = 60, "
            "commandtimeout = 60, baudrate = 921600" RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS,
            {
                false,
                RBRGEN3_WIFI_NA,
                60000,
                60000,
                RBRGEN3_SERIAL_BAUD_921600
            }
        },
        {
            "wifi enabled = true, state = off, timeout = 90, "
            "commandtimeout = 30, baudrate = 921600" RESPONSE_TERMINATOR,
            RBRGEN3_SUCCESS,
            {
                true,
                RBRGEN3_WIFI_OFF,
                90000,
                30000,
                RBRGEN3_SERIAL_BAUD_921600
            }
        },
        {
            "E0109 feature not available" RESPONSE_TERMINATOR,
            RBRGEN3_HARDWARE_ERROR,
            {
                false,
                RBRGEN3_UNKNOWN_WIFI,
                0,
                0,
                RBRGEN3_SERIAL_BAUD_NONE
            }
        },
        {0}
    };

    return test_wifi(instrument, buffers, tests);
}
