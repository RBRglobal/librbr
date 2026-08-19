/**
 * \file security.c
 *
 * \brief Tests for instrument security commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include "RBRInstrumentGen4Security.h"
#include "tests.h"

void test_print(const char *_buf)
{
    int32_t _bufLen = strlen(_buf);
    int32_t _bufEscapedLen = _bufLen * 4 + 1;
    char *_bufEscaped = (char *) malloc(_bufEscapedLen);
    rbr_strnesccntrl(_bufEscaped,
                        _buf,
                        _bufEscapedLen);
    printf("\"%s\"", _bufEscaped);
    free(_bufEscaped);
}

void test_print_buffers(TestIOBuffers *buffers)
{
    printf("read buffer: ");
    test_print(buffers->readBuffer);
    printf("\n");
    printf("write buffer: ");
    test_print(buffers->writeBuffer);
    printf("\n");
}

TEST_LOGGER4(permit)
{
    const char *text = "permit command=foo";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    /*
    printf("Before TestIOBuffers_init:\n");
    test_print_buffers(buffers);
    */
    TestIOBuffers_init(buffers, response, 0);
    /*
    printf("After TestIOBuffers_init:\n");
    test_print_buffers(buffers);
    */
    RBRInstrumentGen4Error err = RBRInstrumentGen4_permit(instrument, "foo");
    /*
    printf("After RBRInstrumentGen4_permit:\n");
    test_print_buffers(buffers);
    */
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(prompt)
{
    bool expected = true;
    bool actual = false;

    TestIOBuffers_init(buffers, "prompt state=on" RESPONSE_TERMINATOR, 0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_getPrompt(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_ENUM_EQ(expected, actual, bool);
    TEST_ASSERT_STR_EQ("prompt state" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(prompt_set_on)
{
    const char *text = "prompt state=on";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_setPrompt(instrument, true);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(prompt_set_off)
{
    const char *text = "prompt state=off";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_setPrompt(instrument, false);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(confirmation)
{
    bool expected = true;
    bool actual = false;

    TestIOBuffers_init(buffers,
                       "confirmation state=on" RESPONSE_TERMINATOR,
                       0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_getConfirmation(instrument,
                                                           &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_ENUM_EQ(expected, actual, bool);
    TEST_ASSERT_STR_EQ("confirmation state" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(confirmation_set_on)
{
    const char *text = "confirmation state=on";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_setConfirmation(instrument, true);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER4(confirmation_set_off)
{
    TestIOBuffers_init(buffers, "", 0);
    RBRInstrumentGen4Error err = RBRInstrumentGen4_setConfirmation(instrument, false);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);
    TEST_ASSERT_STR_EQ("confirmation state=off" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}
