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
    RBRInstrumentGen4Link expected;
} LinkTest;

static bool test_link(RBRInstrumentGen4 *instrument,
                      TestIOBuffers *buffers,
                      LinkTest *tests)
{
    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Link actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_getLink(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected, actual, RBRInstrumentGen4Link);
    }

    return true;
}

TEST_LOGGER4(link)
{
    LinkTest tests[] = {
        { "link type = usb" RESPONSE_TERMINATOR, RBRINSTRUMENTGEN4_LINK_USB },
        { "link type = serial" RESPONSE_TERMINATOR, RBRINSTRUMENTGEN4_LINK_SERIAL },
        { "link type = wifi" RESPONSE_TERMINATOR, RBRINSTRUMENTGEN4_LINK_WIFI },
        { 0 }
    };

    return test_link(instrument, buffers, tests);
}

typedef struct SerialTest
{
    const char *response;
    RBRInstrumentGen4Serial expected;
    RBRInstrumentGen4Error expectedError;
} SerialTest;

TEST_LOGGER4(serial)
{
    SerialTest tests[] = {
        { "serial baudrate = 19200, mode = rs232, availablebaudrates = "
          "115200|19200|9600|4800|2400|1200|230400|460800, availablemodes = "
          "rs232|rs485f|uart|uart_idlelow" RESPONSE_TERMINATOR,
          { RBRINSTRUMENTGEN4_SERIAL_BAUD_19200,
            RBRINSTRUMENTGEN4_SERIAL_MODE_RS232,
            RBRINSTRUMENTGEN4_SERIAL_BAUD_1200 | RBRINSTRUMENTGEN4_SERIAL_BAUD_2400 | RBRINSTRUMENTGEN4_SERIAL_BAUD_4800 | RBRINSTRUMENTGEN4_SERIAL_BAUD_9600 | RBRINSTRUMENTGEN4_SERIAL_BAUD_19200 | RBRINSTRUMENTGEN4_SERIAL_BAUD_115200 | RBRINSTRUMENTGEN4_SERIAL_BAUD_230400 | RBRINSTRUMENTGEN4_SERIAL_BAUD_460800,
            RBRINSTRUMENTGEN4_SERIAL_MODE_RS232 | RBRINSTRUMENTGEN4_SERIAL_MODE_RS485F | RBRINSTRUMENTGEN4_SERIAL_MODE_UART | RBRINSTRUMENTGEN4_SERIAL_MODE_UART_IDLE_LOW },
          RBRINSTRUMENTGEN4_SUCCESS },
        { "serial baudrate = 115200, mode = rs485f, availablebaudrates = "
          "115200|19200|9600|4800|2400|1200|230400|460800, availablemodes = "
          "rs232|rs485f|uart|uart_idlelow" RESPONSE_TERMINATOR,
          { RBRINSTRUMENTGEN4_SERIAL_BAUD_115200,
            RBRINSTRUMENTGEN4_SERIAL_MODE_RS485F,
            RBRINSTRUMENTGEN4_SERIAL_BAUD_1200 | RBRINSTRUMENTGEN4_SERIAL_BAUD_2400 | RBRINSTRUMENTGEN4_SERIAL_BAUD_4800 | RBRINSTRUMENTGEN4_SERIAL_BAUD_9600 | RBRINSTRUMENTGEN4_SERIAL_BAUD_19200 | RBRINSTRUMENTGEN4_SERIAL_BAUD_115200 | RBRINSTRUMENTGEN4_SERIAL_BAUD_230400 | RBRINSTRUMENTGEN4_SERIAL_BAUD_460800,
            RBRINSTRUMENTGEN4_SERIAL_MODE_RS232 | RBRINSTRUMENTGEN4_SERIAL_MODE_RS485F | RBRINSTRUMENTGEN4_SERIAL_MODE_UART | RBRINSTRUMENTGEN4_SERIAL_MODE_UART_IDLE_LOW },
          RBRINSTRUMENTGEN4_SUCCESS },
        { 0 }
    };

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Serial actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_getSerial(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentGen4Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.baudRate,
                            actual.baudRate,
                            RBRInstrumentGen4SerialBaudRate);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.mode,
                            actual.mode,
                            RBRInstrumentGen4SerialMode);
        TEST_ASSERT_EQ(tests[i].expected.availableBaudRates,
                       actual.availableBaudRates,
                       "0x%04X");
        TEST_ASSERT_EQ(tests[i].expected.availableModes,
                       actual.availableModes,
                       "0x%04X");
    }

    return true;
}

typedef struct SetSerialTest
{
    const char *command;
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4Serial serial;
} SetSerialTest;

TEST_LOGGER4(setSerial)
{
    SetSerialTest tests[] = {
        { "serial baudrate = 19200, mode = rs232" COMMAND_TERMINATOR,
          "serial baudrate = 19200, mode = rs232" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { RBRINSTRUMENTGEN4_SERIAL_BAUD_19200,
            RBRINSTRUMENTGEN4_SERIAL_MODE_RS232,
            RBRINSTRUMENTGEN4_SERIAL_BAUD_NONE,
            RBRINSTRUMENTGEN4_SERIAL_MODE_NONE } },
        { "serial baudrate = 9600, mode = uart" COMMAND_TERMINATOR,
          "serial baudrate = 9600, mode = uart" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { RBRINSTRUMENTGEN4_SERIAL_BAUD_9600,
            RBRINSTRUMENTGEN4_SERIAL_MODE_UART,
            RBRINSTRUMENTGEN4_SERIAL_BAUD_NONE,
            RBRINSTRUMENTGEN4_SERIAL_MODE_NONE } },
        { "serial baudrate = 115200, mode = rs485f" COMMAND_TERMINATOR,
          "serial baudrate = 115200, mode = rs485f" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { RBRINSTRUMENTGEN4_SERIAL_BAUD_115200,
            RBRINSTRUMENTGEN4_SERIAL_MODE_RS485F,
            RBRINSTRUMENTGEN4_SERIAL_BAUD_NONE,
            RBRINSTRUMENTGEN4_SERIAL_MODE_NONE } },
        { 0 }
    };

    RBRInstrumentGen4Error err;
    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_setSerial(instrument, &(tests[i].serial));
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command,
                           buffers->writeBuffer);
    }

    return true;
}

TEST_LOGGER4(aux1)
{
    RBRInstrumentGen4Aux1 expected = {
        .aux1_state = 1,
        .aux1_setup = 2000,
        .aux1_hold = 3000,
        .aux1_active = RBRINSTRUMENTGEN4_AUX1ACTIVE_HIGH,
        .aux1_sleep = RBRINSTRUMENTGEN4_AUX1SLEEP_TRISTATE
    };
    RBRInstrumentGen4Aux1 actual;
    TestIOBuffers_init(buffers, "serial aux1_state = on, aux1_setup = 2000, "
                                "aux1_hold = 3000, aux1_active = high, aux1_sleep = tristate" RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_getAux1(instrument, &actual);
    TEST_ASSERT_STR_EQ("serial aux1_all" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_EQ(expected.aux1_state, actual.aux1_state, "%" PRIi32);
    TEST_ASSERT_EQ(expected.aux1_setup, actual.aux1_setup, "%" PRIi32);
    TEST_ASSERT_EQ(expected.aux1_active, actual.aux1_active, "%" PRIi32);
    TEST_ASSERT_EQ(expected.aux1_sleep, actual.aux1_sleep, "%" PRIi32);
    return true;
}

typedef struct SetAux1Test
{
    const char *command;
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4Aux1 aux1;
} SetAux1Test;

TEST_LOGGER4(setAux1)
{
    SetAux1Test tests[] = {
        { "serial aux1_state = on, aux1_setup = 2000, "
          "aux1_hold = 3000, aux1_active = high, aux1_sleep = tristate" COMMAND_TERMINATOR,

          "serial aux1_state = on, aux1_setup = 2000, "
          "aux1_hold = 3000, aux1_active = high, aux1_sleep = tristate" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 1,
            2000,
            3000,
            RBRINSTRUMENTGEN4_AUX1ACTIVE_HIGH,
            RBRINSTRUMENTGEN4_AUX1SLEEP_TRISTATE } },
        { "serial aux1_state = off, aux1_setup = 500, "
          "aux1_hold = 1000, aux1_active = low, aux1_sleep = high" COMMAND_TERMINATOR,

          "serial aux1_state = off, aux1_setup = 500, "
          "aux1_hold = 1000, aux1_active = low, aux1_sleep = high" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 0,
            500,
            1000,
            RBRINSTRUMENTGEN4_AUX1ACTIVE_LOW,
            RBRINSTRUMENTGEN4_AUX1SLEEP_HIGH } },
        { 0 }
    };

    RBRInstrumentGen4Error err;
    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_setAux1(instrument, &(tests[i].aux1));
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command,
                           buffers->writeBuffer);
    }
    return true;
}

TEST_LOGGER4(sleep)
{
    TestIOBuffers_init(buffers, "", 0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_sleep(instrument);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("sleep" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT(instrument->lastActivityTime < 0);

    return true;
}

typedef struct WiFiTest
{
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4WiFi expected;
} WiFiTest;

static bool test_wifi(RBRInstrumentGen4 *instrument,
                      TestIOBuffers *buffers,
                      const WiFiTest *tests)
{
    RBRInstrumentGen4Error err;
    RBRInstrumentGen4WiFi actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_getWiFi(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentGen4Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.enabled, actual.enabled, bool);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.state,
                            actual.state,
                            RBRInstrumentGen4WiFiState);
        TEST_ASSERT_EQ(tests[i].expected.timeout,
                       actual.timeout,
                       "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.commandTimeout,
                       actual.commandTimeout,
                       "%" PRIi32);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.baudRate,
                            actual.baudRate,
                            RBRInstrumentGen4SerialBaudRate);
    }

    return true;
}

TEST_LOGGER4(wifi)
{
    WiFiTest tests[] = {
        {
            "wifi enabled = false, state = n/a, timeout = 60, "
            "commandtimeout = 60, baudrate = 921600" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_SUCCESS,
            {
                false,
                RBRINSTRUMENTGEN4_WIFI_NA,
                60000,
                60000,
                RBRINSTRUMENTGEN4_SERIAL_BAUD_921600
            }
        },
        {
            "wifi enabled = true, state = off, timeout = 90, "
            "commandtimeout = 30, baudrate = 921600" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_SUCCESS,
            {
                true,
                RBRINSTRUMENTGEN4_WIFI_OFF,
                90000,
                30000,
                RBRINSTRUMENTGEN4_SERIAL_BAUD_921600
            }
        },
        {
            "E0109 feature not available" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_HARDWARE_ERROR,
            {
                false,
                RBRINSTRUMENTGEN4_UNKNOWN_WIFI,
                0,
                0,
                RBRINSTRUMENTGEN4_SERIAL_BAUD_NONE
            }
        },
        {0}
    };

    return test_wifi(instrument, buffers, tests);
}

typedef struct setWiFiTest
{
    const char *command;
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4WiFi wifi;
} setWiFiTest;

static bool test_setWifi(RBRInstrumentGen4 *instrument,
                      TestIOBuffers *buffers,
                      const setWiFiTest *tests)
{
    RBRInstrumentGen4Error err;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_setWiFi(instrument, &(tests[i].wifi));
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }

    return true;
}

TEST_LOGGER4(setWifi)
{
    setWiFiTest tests[] = {
        {
            "wifi enabled = false, timeout = 60, "
            "commandtimeout = 60" COMMAND_TERMINATOR,

            "wifi enabled = false, timeout = 60, "
            "commandtimeout = 60" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_SUCCESS,
            {
                false,
                RBRINSTRUMENTGEN4_UNKNOWN_WIFI,
                60000,
                60000,
                RBRINSTRUMENTGEN4_SERIAL_BAUD_NONE
            }
        },
        {
            "wifi enabled = true, timeout = 20, "
            "commandtimeout = 60" COMMAND_TERMINATOR,

            "wifi enabled = true, timeout = 20, "
            "commandtimeout = 60" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_SUCCESS,
            {
                true,
                RBRINSTRUMENTGEN4_WIFI_OFF,
                20000,
                60000,
                RBRINSTRUMENTGEN4_SERIAL_BAUD_NONE
            }
        },
        {
            /* invalid argument: won't even write buffer per _setWiFi() function. */
            /* invalid argument and feature not available: logger will respond invalid argument.*/

            // "wifi enabled = false, timeout = 2, "
            // "commandtimeout = 2" COMMAND_TERMINATOR,

            "",

            "E0108 invalid argument to command: '2'" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE,
            {
                false,
                RBRINSTRUMENTGEN4_WIFI_OFF,
                2000,
                2000,
                RBRINSTRUMENTGEN4_SERIAL_BAUD_NONE
            }
        },
        {
            /* feature not available, but parameters are valid: */
            "wifi enabled = false, timeout = 10, "
            "commandtimeout = 10" COMMAND_TERMINATOR,

            "E0109 feature not available" RESPONSE_TERMINATOR,
            RBRINSTRUMENTGEN4_HARDWARE_ERROR,
            {
                false,
                RBRINSTRUMENTGEN4_WIFI_OFF,
                10000,
                10000,
                RBRINSTRUMENTGEN4_SERIAL_BAUD_NONE
            }
        },
        {0}
    };

    return test_setWifi(instrument, buffers, tests);
}