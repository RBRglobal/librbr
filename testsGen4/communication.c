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

static bool test_link(RBRGen4 *instrument,
                      TestIOBuffers *buffers,
                      LinkTest *tests)
{
    RBRGen4Error err;
    RBRInstrumentGen4Link actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_getLink(instrument, &actual);
        TEST_ASSERT_STR_EQ("link" COMMAND_TERMINATOR, buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
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

typedef struct LinkSerialTest
{
    const char *response;
    RBRInstrumentGen4LinkSerial expected;
    RBRGen4Error expectedError;
} LinkSerialTest;

/* The baud rates and modes an L4 reports as available. */
#define L4_AVAILABLE_BAUD_RATES \
    "4800|9600|19200|38400|57600|115200|230400"
#define L4_AVAILABLE_MODES "rs232|rs485f|uart|uart_idlelow"

#define L4_AVAILABLE_BAUD_RATE_MASK \
    (RBRINSTRUMENTGEN4_LINK_SERIAL_BAUD_4800 \
     | RBRINSTRUMENTGEN4_LINK_SERIAL_BAUD_9600 \
     | RBRINSTRUMENTGEN4_LINK_SERIAL_BAUD_19200 \
     | RBRINSTRUMENTGEN4_LINK_SERIAL_BAUD_38400 \
     | RBRINSTRUMENTGEN4_LINK_SERIAL_BAUD_57600 \
     | RBRINSTRUMENTGEN4_LINK_SERIAL_BAUD_115200 \
     | RBRINSTRUMENTGEN4_LINK_SERIAL_BAUD_230400)
#define L4_AVAILABLE_MODE_MASK \
    (RBRINSTRUMENTGEN4_LINK_SERIAL_MODE_RS232 \
     | RBRINSTRUMENTGEN4_LINK_SERIAL_MODE_RS485F \
     | RBRINSTRUMENTGEN4_LINK_SERIAL_MODE_UART \
     | RBRINSTRUMENTGEN4_LINK_SERIAL_MODE_UART_IDLE_LOW)

TEST_LOGGER4(linkSerial)
{
    LinkSerialTest tests[] = {
        { "link serial baudrate=230400 mode=rs232 availablebaudrates="
          L4_AVAILABLE_BAUD_RATES " availablemodes="
          L4_AVAILABLE_MODES RESPONSE_TERMINATOR,
          { .baudRate = RBRINSTRUMENTGEN4_LINK_SERIAL_BAUD_230400,
            .mode = RBRINSTRUMENTGEN4_LINK_SERIAL_MODE_RS232,
            .availableBaudRates = L4_AVAILABLE_BAUD_RATE_MASK,
            .availableModes = L4_AVAILABLE_MODE_MASK },
          RBRGEN4_SUCCESS },
        { "link serial baudrate=115200 mode=uart_idlelow availablebaudrates="
          L4_AVAILABLE_BAUD_RATES " availablemodes="
          L4_AVAILABLE_MODES RESPONSE_TERMINATOR,
          { .baudRate = RBRINSTRUMENTGEN4_LINK_SERIAL_BAUD_115200,
            .mode = RBRINSTRUMENTGEN4_LINK_SERIAL_MODE_UART_IDLE_LOW,
            .availableBaudRates = L4_AVAILABLE_BAUD_RATE_MASK,
            .availableModes = L4_AVAILABLE_MODE_MASK },
          RBRGEN4_SUCCESS },
        /* An instrument which offers only one mode. */
        { "link serial baudrate=19200 mode=rs232 availablebaudrates="
          "9600|19200 availablemodes=rs232" RESPONSE_TERMINATOR,
          { .baudRate = RBRINSTRUMENTGEN4_LINK_SERIAL_BAUD_19200,
            .mode = RBRINSTRUMENTGEN4_LINK_SERIAL_MODE_RS232,
            .availableBaudRates =
                RBRINSTRUMENTGEN4_LINK_SERIAL_BAUD_9600
                | RBRINSTRUMENTGEN4_LINK_SERIAL_BAUD_19200,
            .availableModes = RBRINSTRUMENTGEN4_LINK_SERIAL_MODE_RS232 },
          RBRGEN4_SUCCESS },
        /* Values the library does not know are reported as unknown, and are
         * left out of the available lists. */
        { "link serial baudrate=921600 mode=rs485h availablebaudrates="
          "9600|921600 availablemodes=rs232|rs485h" RESPONSE_TERMINATOR,
          { .baudRate = RBRINSTRUMENTGEN4_LINK_SERIAL_BAUD_NONE,
            .mode = RBRINSTRUMENTGEN4_LINK_SERIAL_MODE_NONE,
            .availableBaudRates = RBRINSTRUMENTGEN4_LINK_SERIAL_BAUD_9600,
            .availableModes = RBRINSTRUMENTGEN4_LINK_SERIAL_MODE_RS232 },
          RBRGEN4_SUCCESS },
        { 0 }
    };

    RBRGen4Error err;
    RBRInstrumentGen4LinkSerial actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_getLinkSerial(instrument, &actual);
        TEST_ASSERT_STR_EQ(
            "link serial baudrate mode availablebaudrates availablemodes" COMMAND_TERMINATOR,
            buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen4Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.baudRate,
                            actual.baudRate,
                            RBRInstrumentGen4LinkSerialBaudRate);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.mode,
                            actual.mode,
                            RBRInstrumentGen4LinkSerialMode);

        TEST_ASSERT_EQ(tests[i].expected.availableBaudRates,
                       actual.availableBaudRates,
                       "%d");
        TEST_ASSERT_EQ(tests[i].expected.availableModes,
                       actual.availableModes,
                       "%d");
    }

    return true;
}

typedef struct SetLinkSerialTest
{
    const char *command;
    const char *response;
    RBRGen4Error expectedError;
    RBRInstrumentGen4LinkSerial serial;
} SetLinkSerialTest;

TEST_LOGGER4(setLinkSerial)
{
    SetLinkSerialTest tests[] = {
        { "link serial baudrate=19200 mode=rs232" COMMAND_TERMINATOR,
          "link serial baudrate=19200 mode=rs232" RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS,
          { .baudRate = RBRINSTRUMENTGEN4_LINK_SERIAL_BAUD_19200,
            .mode = RBRINSTRUMENTGEN4_LINK_SERIAL_MODE_RS232 } },
        { "link serial baudrate=9600 mode=uart" COMMAND_TERMINATOR,
          "link serial baudrate=9600 mode=uart" RESPONSE_TERMINATOR,
          RBRGEN4_SUCCESS,
          { .baudRate = RBRINSTRUMENTGEN4_LINK_SERIAL_BAUD_9600,
            .mode = RBRINSTRUMENTGEN4_LINK_SERIAL_MODE_UART } },
        /* A value the getter could not read reaches the setter as `none`, a
         * value out of range is a mistake, and the command takes one rate and
         * one mode so a combination of flags is a mistake too: none of the
         * three is sent. */
        { "",
          "",
          RBRGEN4_INVALID_PARAMETER_VALUE,
          { .baudRate = RBRINSTRUMENTGEN4_LINK_SERIAL_BAUD_NONE,
            .mode = RBRINSTRUMENTGEN4_LINK_SERIAL_MODE_RS232 } },
        { "",
          "",
          RBRGEN4_INVALID_PARAMETER_VALUE,
          { .baudRate = RBRINSTRUMENTGEN4_LINK_SERIAL_BAUD_115200,
            .mode = RBRINSTRUMENTGEN4_LINK_SERIAL_MODE_NONE } },
        { "",
          "",
          RBRGEN4_INVALID_PARAMETER_VALUE,
          { .baudRate = RBRINSTRUMENTGEN4_LINK_SERIAL_BAUD_MAX << 1,
            .mode = RBRINSTRUMENTGEN4_LINK_SERIAL_MODE_RS232 } },
        { "",
          "",
          RBRGEN4_INVALID_PARAMETER_VALUE,
          { .baudRate = RBRINSTRUMENTGEN4_LINK_SERIAL_BAUD_9600
                        | RBRINSTRUMENTGEN4_LINK_SERIAL_BAUD_19200,
            .mode = RBRINSTRUMENTGEN4_LINK_SERIAL_MODE_RS232 } },
        { 0 }
    };

    RBRGen4Error err;
    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_setLinkSerial(instrument, &(tests[i].serial));
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command,
                           buffers->writeBuffer);
    }

    return true;
}

TEST_LOGGER4(sleep)
{
    TestIOBuffers_init(buffers, "", 0);
    RBRGen4Error err = RBRInstrumentGen4_sleep(instrument);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("sleep" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT(instrument->lastActivityTime < 0);

    /* Wake up the simulated instrument, expecting the two wakeup sequences.
     * The instrument acknowledges the sleep command when confirmation is on,
     * but the command is not one we wait for a response to, so that
     * acknowledgement is still unread: it must be skipped in favour of the
     * response to the next command. */
    RBRInstrumentGen4Link actual;
    TestIOBuffers_init(buffers,
                       "sleep" RESPONSE_TERMINATOR
                       "link type=usb" RESPONSE_TERMINATOR,
                       0);
    err = RBRInstrumentGen4_getLink(instrument, &actual);
    TEST_ASSERT_STR_EQ(
        RESPONSE_TERMINATOR RESPONSE_TERMINATOR "link" COMMAND_TERMINATOR,
        buffers->writeBuffer);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_LINK_TYPE_USB,
                        actual.type,
                        RBRInstrumentGen4LinkType);

    return true;
}
