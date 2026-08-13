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
#include "RBRInstrumentGen4Communication.h"

typedef struct LinkTest
{
    const char *response;
    RBRInstrumentGen4LinkType expected;
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
        TEST_ASSERT_STR_EQ("link" COMMAND_TERMINATOR, buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected,
                            actual.type,
                            RBRInstrumentGen4LinkType);
    }

    return true;
}

TEST_LOGGER4(link)
{
    LinkTest tests[] = {
        { "link type=usb" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_LINK_TYPE_USB },
        { "link type=serial" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_LINK_TYPE_SERIAL },
        /* { "link type=wifi" RESPONSE_TERMINATOR,
             RBRINSTRUMENTGEN4_LINK_TYPE_WIFI }, */
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
        { "link serial baudrate=19200 mode=rs232 availablebaudrates="
          "115200|19200|9600|4800|2400|1200|230400|460800 availablemodes="
          "rs232|rs485f|uart|uart_idlelow" RESPONSE_TERMINATOR,
          { RBRINSTRUMENTGEN4_SERIAL_BAUD_19200,
            RBRINSTRUMENTGEN4_SERIAL_MODE_RS232,
            RBRINSTRUMENTGEN4_SERIAL_BAUD_1200 | RBRINSTRUMENTGEN4_SERIAL_BAUD_2400 | RBRINSTRUMENTGEN4_SERIAL_BAUD_4800 | RBRINSTRUMENTGEN4_SERIAL_BAUD_9600 | RBRINSTRUMENTGEN4_SERIAL_BAUD_19200 | RBRINSTRUMENTGEN4_SERIAL_BAUD_115200 | RBRINSTRUMENTGEN4_SERIAL_BAUD_230400 | RBRINSTRUMENTGEN4_SERIAL_BAUD_460800,
            RBRINSTRUMENTGEN4_SERIAL_MODE_RS232 | RBRINSTRUMENTGEN4_SERIAL_MODE_RS485F | RBRINSTRUMENTGEN4_SERIAL_MODE_UART | RBRINSTRUMENTGEN4_SERIAL_MODE_UART_IDLE_LOW },
          RBRINSTRUMENTGEN4_SUCCESS },
        { "link serial baudrate=115200 mode=rs485f availablebaudrates="
          "115200|19200|9600|4800|2400|1200|230400|460800 availablemodes="
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
        TEST_ASSERT_STR_EQ(
            "link serial baudrate mode availablebaudrates availablemodes" COMMAND_TERMINATOR,
            buffers->writeBuffer);
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
        { "link serial baudrate=19200 mode=rs232" COMMAND_TERMINATOR,
          "link serial baudrate=19200 mode=rs232" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { RBRINSTRUMENTGEN4_SERIAL_BAUD_19200,
            RBRINSTRUMENTGEN4_SERIAL_MODE_RS232,
            RBRINSTRUMENTGEN4_SERIAL_BAUD_NONE,
            RBRINSTRUMENTGEN4_SERIAL_MODE_NONE } },
        { "link serial baudrate=9600 mode=uart" COMMAND_TERMINATOR,
          "link serial baudrate=9600 mode=uart" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { RBRINSTRUMENTGEN4_SERIAL_BAUD_9600,
            RBRINSTRUMENTGEN4_SERIAL_MODE_UART,
            RBRINSTRUMENTGEN4_SERIAL_BAUD_NONE,
            RBRINSTRUMENTGEN4_SERIAL_MODE_NONE } },
        { "link serial baudrate=115200 mode=rs485f" COMMAND_TERMINATOR,
          "link serial baudrate=115200 mode=rs485f" RESPONSE_TERMINATOR,
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

TEST_LOGGER4(sleep)
{
    TestIOBuffers_init(buffers, "", 0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_sleep(instrument);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("sleep" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT(instrument->lastActivityTime < 0);

    /* Wake up the simulated instrument, expecting the two wakeup sequences. */
    RBRInstrumentGen4Link actual;
    TestIOBuffers_init(buffers, "link type=usb" RESPONSE_TERMINATOR, 0);
    err = RBRInstrumentGen4_getLink(instrument, &actual);
    TEST_ASSERT_STR_EQ(
        RESPONSE_TERMINATOR RESPONSE_TERMINATOR "link" COMMAND_TERMINATOR,
        buffers->writeBuffer);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_LINK_TYPE_USB,
                        actual.type,
                        RBRInstrumentGen4LinkType);

    return true;
}
