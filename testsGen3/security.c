/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file security.c
 *
 * \brief Tests for instrument security commands.
 */

#include "tests.h"
#include "RBRGen3Security.h"

TEST_LOGGER2(permit)
{
    const char *text = "permit = foo";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRGen3_permit(conn, "foo");
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(permit)
{
    const char *text = "permit command = foo";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRGen3_permit(conn, "foo");
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(prompt)
{
    bool expected = true;
    bool actual = false;

    TestIOBuffers_init(buffers, "prompt state = on" RESPONSE_TERMINATOR, 0);
    RBRGen3Error err = RBRGen3_getPrompt(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(expected, actual, bool);
    TEST_ASSERT_STR_EQ("prompt state" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(prompt_set)
{
    const char *text = "prompt state = on";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRGen3_setPrompt(conn, true);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(confirmation)
{
    bool expected = true;
    bool actual = false;

    TestIOBuffers_init(buffers, "confirmation state = on" RESPONSE_TERMINATOR, 0);
    RBRGen3Error err = RBRGen3_getConfirmation(conn, &actual);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(expected, actual, bool);
    TEST_ASSERT_STR_EQ("confirmation state" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(confirmation_set_on)
{
    const char *text = "confirmation state = on";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRGen3_setConfirmation(conn, true);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(confirmation_set_off)
{
    TestIOBuffers_init(buffers, "", 0);
    RBRGen3Error err = RBRGen3_setConfirmation(conn, false);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("confirmation state = off" COMMAND_TERMINATOR, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(reboot)
{
    TestIOBuffers_init(buffers, "permit command = reboot" RESPONSE_TERMINATOR, 0);
    RBRGen3Error err = RBRGen3_reboot(conn, 123);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("permit command = reboot" COMMAND_TERMINATOR "reboot 123" COMMAND_TERMINATOR,
                       buffers->writeBuffer);
    TEST_ASSERT(conn->lastActivityTime < 0);

    return true;
}
