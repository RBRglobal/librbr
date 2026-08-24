/**
 * \file deployment.c
 *
 * \brief Tests for instrument deployment commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include "tests.h"
#include "RBRInstrumentGen4Deployment.h"

/* Required for isnan, NAN. */
#include <math.h>

typedef struct GetClockTest
{
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4Clock expected;
} GetClockTest;

TEST_LOGGER4(getClock)
{
    GetClockTest tests[] = {
        /* A bare query reports both parameters. */
        { "clock datetime=20260824120000 offsetfromutc=0.00"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 1787572800000LL, 0.0f } },
        /* The instrument never signs a positive offset. */
        { "clock datetime=20260824120130 offsetfromutc=5.50"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 1787572890000LL, 5.5f } },
        { "clock datetime=20260824120000 offsetfromutc=-4.50"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 1787572800000LL, -4.5f } },
        /*
         * The response echoes the parameters in the order they were asked
         * for, so the getter must not depend on their position.
         */
        { "clock offsetfromutc=14.00 datetime=20260824120000"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 1787572800000LL, 14.0f } },
        /* An unreported parameter keeps its unset value. */
        { "clock datetime=20260824120000" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 1787572800000LL, NAN } },
        { "clock" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 0, NAN } },
        /* Keys the library does not model are ignored. */
        { "clock datetime=20260824120000 offsetfromutc=0.00 bogus=1"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 1787572800000LL, 0.0f } },
        { "ERR-105 command prohibited while logging" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR,
          { 0, NAN } },
        { 0 }
    };

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Clock actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_getClock(instrument, &actual);
        TEST_ASSERT_STR_EQ("clock" COMMAND_TERMINATOR, buffers->writeBuffer);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRInstrumentGen4Error);
        if (err != RBRINSTRUMENTGEN4_SUCCESS)
        {
            continue;
        }
        TEST_ASSERT_EQ(tests[i].expected.dateTime,
                       actual.dateTime,
                       "%" PRIi64);
        if (isnan(tests[i].expected.offsetFromUtc))
        {
            TEST_ASSERT(isnan(actual.offsetFromUtc));
        }
        else
        {
            TEST_ASSERT_FLOAT_EQ(tests[i].expected.offsetFromUtc,
                                 actual.offsetFromUtc,
                                 0.005f);
        }
    }

    return true;
}

typedef struct SetClockTest
{
    RBRInstrumentGen4Clock clock;
    const char *command;
    const char *response;
    RBRInstrumentGen4Error expectedError;
} SetClockTest;

TEST_LOGGER4(setClock)
{
    SetClockTest tests[] = {
        /* Both parameters are always sent. */
        { { 1787572800000LL, 0.0f },
          "clock datetime=20260824120000 offsetfromutc=0.00"
          COMMAND_TERMINATOR,
          "clock datetime=20260824120000 offsetfromutc=0.00"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS },
        { { 1787572890000LL, 5.5f },
          "clock datetime=20260824120130 offsetfromutc=5.50"
          COMMAND_TERMINATOR,
          "clock datetime=20260824120130 offsetfromutc=5.50"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS },
        { { 1787572800000LL, -12.0f },
          "clock datetime=20260824120000 offsetfromutc=-12.00"
          COMMAND_TERMINATOR,
          "clock datetime=20260824120000 offsetfromutc=-12.00"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS },
        /*
         * The offset is not range-checked: an offset the instrument will not
         * accept is sent and refused there.
         */
        { { 1787572800000LL, 99.0f },
          "clock datetime=20260824120000 offsetfromutc=99.00"
          COMMAND_TERMINATOR,
          "ERR-108 invalid argument to command: '99.00'" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR },
        /* `NAN` would emit "nan", so it is refused before the command. */
        { { 1787572800000LL, NAN },
          "",
          "",
          RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE },
        { { RBRINSTRUMENTGEN4_DATETIME_MIN - 1, 0.0f },
          "",
          "",
          RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE },
        { { RBRINSTRUMENTGEN4_DATETIME_MAX + 1, 0.0f },
          "",
          "",
          RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE },
        { { 1787572800000LL, 0.0f },
          "clock datetime=20260824120000 offsetfromutc=0.00"
          COMMAND_TERMINATOR,
          "ERR-105 command prohibited while logging" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR },
        { { 0 }, NULL, NULL, 0 }
    };

    RBRInstrumentGen4Error err;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_setClock(instrument, &tests[i].clock);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }

    return true;
}
