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
#include "RBRGen4Communication.h"

typedef struct LinkTest {
    const char *response;
    RBRGen4LinkType expected;
} LinkTest;

static bool test_link(RBRGen4 *conn, TestIOBuffers *buffers, LinkTest *tests)
{
    RBRGen4Error err;
    RBRGen4Link actual;

    for (int i = 0; tests[i].response != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4_getLink(conn, &actual);
        TEST_ASSERT_STR_EQ("link" COMMAND_TERMINATOR, buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected, actual.type, RBRGen4LinkType);
    }

    return true;
}

TEST_LOGGER4(link)
{
    LinkTest tests[] = {
        {
            .response = "link type=usb" RESPONSE_TERMINATOR,
            .expected = RBRGEN4_LINK_TYPE_USB,
        },
        {
            .response = "link type=serial" RESPONSE_TERMINATOR,
            .expected = RBRGEN4_LINK_TYPE_SERIAL,
        },
        {0},
    };

    return test_link(conn, buffers, tests);
}

typedef struct LinkSerialTest {
    const char *response;
    RBRGen4LinkSerial expected;
    RBRGen4Error expectedError;
} LinkSerialTest;

TEST_LOGGER4(linkSerial)
{
    LinkSerialTest tests[] = {
        {
            .response = "link serial baudrate=230400 mode=rs232" RESPONSE_TERMINATOR,
            .expected =
                {
                    .baudRate = RBRGEN4_LINK_SERIAL_BAUD_230400,
                    .mode = RBRGEN4_LINK_SERIAL_MODE_RS232,
                },
            .expectedError = RBRGEN4_SUCCESS,
        },
        {
            .response = "link serial baudrate=115200 mode=uart_idlelow" RESPONSE_TERMINATOR,
            .expected =
                {
                    .baudRate = RBRGEN4_LINK_SERIAL_BAUD_115200,
                    .mode = RBRGEN4_LINK_SERIAL_MODE_UART_IDLE_LOW,
                },
            .expectedError = RBRGEN4_SUCCESS,
        },
        /* Values the library does not know are reported as unknown. */
        {
            .response = "link serial baudrate=921600 mode=rs485h" RESPONSE_TERMINATOR,
            .expected =
                {
                    .baudRate = RBRGEN4_LINK_SERIAL_BAUD_NONE,
                    .mode = RBRGEN4_LINK_SERIAL_MODE_NONE,
                },
            .expectedError = RBRGEN4_SUCCESS,
        },
        {0},
    };

    RBRGen4Error err;
    RBRGen4LinkSerial actual;

    for (int i = 0; tests[i].response != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4_getLinkSerial(conn, &actual);
        TEST_ASSERT_STR_EQ("link serial" COMMAND_TERMINATOR, buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen4Error);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.baudRate, actual.baudRate, RBRGen4LinkSerialBaudRate);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.mode, actual.mode, RBRGen4LinkSerialMode);
    }

    return true;
}

typedef struct SetLinkSerialTest {
    const char *command;
    const char *response;
    RBRGen4Error expectedError;
    RBRGen4LinkSerial serial;
} SetLinkSerialTest;

TEST_LOGGER4(setLinkSerial)
{
    SetLinkSerialTest tests[] = {
        {
            .command = "link serial baudrate=19200 mode=rs232" COMMAND_TERMINATOR,
            .response = "link serial baudrate=19200 mode=rs232" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .serial =
                {
                    .baudRate = RBRGEN4_LINK_SERIAL_BAUD_19200,
                    .mode = RBRGEN4_LINK_SERIAL_MODE_RS232,
                },
        },
        {
            .command = "link serial baudrate=9600 mode=uart" COMMAND_TERMINATOR,
            .response = "link serial baudrate=9600 mode=uart" RESPONSE_TERMINATOR,
            .expectedError = RBRGEN4_SUCCESS,
            .serial =
                {
                    .baudRate = RBRGEN4_LINK_SERIAL_BAUD_9600,
                    .mode = RBRGEN4_LINK_SERIAL_MODE_UART,
                },
        },
        /* A value the getter could not read reaches the setter as `none`, a
         * value out of range is a mistake, and the command takes one rate and
         * one mode so a combination of flags is a mistake too: none of the
         * three is sent. */
        {
            .command = "",
            .response = "",
            .expectedError = RBRGEN4_INVALID_PARAMETER_VALUE,
            .serial =
                {
                    .baudRate = RBRGEN4_LINK_SERIAL_BAUD_NONE,
                    .mode = RBRGEN4_LINK_SERIAL_MODE_RS232,
                },
        },
        {
            .command = "",
            .response = "",
            .expectedError = RBRGEN4_INVALID_PARAMETER_VALUE,
            .serial =
                {
                    .baudRate = RBRGEN4_LINK_SERIAL_BAUD_115200,
                    .mode = RBRGEN4_LINK_SERIAL_MODE_NONE,
                },
        },
        {
            .command = "",
            .response = "",
            .expectedError = RBRGEN4_INVALID_PARAMETER_VALUE,
            .serial =
                {
                    .baudRate = RBRGEN4_LINK_SERIAL_BAUD_MAX << 1,
                    .mode = RBRGEN4_LINK_SERIAL_MODE_RS232,
                },
        },
        {
            .command = "",
            .response = "",
            .expectedError = RBRGEN4_INVALID_PARAMETER_VALUE,
            .serial =
                {
                    .baudRate = RBRGEN4_LINK_SERIAL_BAUD_9600 | RBRGEN4_LINK_SERIAL_BAUD_19200,
                    .mode = RBRGEN4_LINK_SERIAL_MODE_RS232,
                },
        },
        {0},
    };

    RBRGen4Error err;
    for (int i = 0; tests[i].command != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen4_setLinkSerial(conn, &(tests[i].serial));
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }

    return true;
}

TEST_LOGGER4(sleep)
{
    TestIOBuffers_init(buffers, "", 0);
    RBRGen4Error err = RBRGen4_sleep(conn);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_STR_EQ("sleep" COMMAND_TERMINATOR, buffers->writeBuffer);
    TEST_ASSERT(conn->lastActivityTime < 0);

    /* Wake up the simulated instrument, expecting the two wakeup sequences.
     * The instrument acknowledges the sleep command when confirmation is on,
     * but the command is not one we wait for a response to, so that
     * acknowledgement is still unread: it must be skipped in favour of the
     * response to the next command. */
    RBRGen4Link actual;
    TestIOBuffers_init(buffers, "sleep" RESPONSE_TERMINATOR "link type=usb" RESPONSE_TERMINATOR, 0);
    err = RBRGen4_getLink(conn, &actual);
    TEST_ASSERT_STR_EQ(RESPONSE_TERMINATOR RESPONSE_TERMINATOR "link" COMMAND_TERMINATOR,
                       buffers->writeBuffer);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_SUCCESS, err, RBRGen4Error);
    TEST_ASSERT_ENUM_EQ(RBRGEN4_LINK_TYPE_USB, actual.type, RBRGen4LinkType);

    return true;
}
