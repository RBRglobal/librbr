/**
 * \file memory.c
 *
 * \brief Tests for instrument memory commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include "tests.h"
#include "RBRInstrumentGen4Memory.h"

/* Required for PRId64. */
#include <inttypes.h>

typedef struct GetStorageTest
{
    const char *response;
    RBRInstrumentGen4Error expectedError;
    RBRInstrumentGen4Storage expected;
} GetStorageTest;

TEST_LOGGER4(getStorage)
{
    GetStorageTest tests[] = {
        /* A bare query reports all four parameters. */
        { "storage used=15372 remaining=61016097780 size=61016113152"
          " access=instrument" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 15372LL,
            61016097780LL,
            61016113152LL,
            RBRINSTRUMENTGEN4_STORAGE_ACCESS_INSTRUMENT } },
        { "storage used=1528 remaining=134216192 size=134217728"
          " access=usbhost" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 1528LL,
            134216192LL,
            134217728LL,
            RBRINSTRUMENTGEN4_STORAGE_ACCESS_USBHOST } },
        /* An unreported parameter keeps its unset value. */
        { "storage used=15372" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 15372LL, 0LL, 0LL, RBRINSTRUMENTGEN4_UNKNOWN_STORAGE_ACCESS } },
        /* An unrecognized access location parses to the unknown member. */
        { "storage used=15372 remaining=61016097780 size=61016113152"
          " access=cloud" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 15372LL,
            61016097780LL,
            61016113152LL,
            RBRINSTRUMENTGEN4_UNKNOWN_STORAGE_ACCESS } },
        /* Keys the library does not model are ignored. */
        { "storage used=15372 remaining=61016097780 size=61016113152"
          " access=instrument bogus=1" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS,
          { 15372LL,
            61016097780LL,
            61016113152LL,
            RBRINSTRUMENTGEN4_STORAGE_ACCESS_INSTRUMENT } },
        { NULL, 0, { 0, 0, 0, 0 } }
    };

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4Storage actual;

    for (int i = 0; tests[i].response != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_getStorage(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ("storage" COMMAND_TERMINATOR,
                           buffers->writeBuffer);
        TEST_ASSERT_EQ(tests[i].expected.used, actual.used, "%" PRId64);
        TEST_ASSERT_EQ(tests[i].expected.remaining,
                       actual.remaining,
                       "%" PRId64);
        TEST_ASSERT_EQ(tests[i].expected.size, actual.size, "%" PRId64);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.access,
                            actual.access,
                            RBRInstrumentGen4StorageAccess);
    }

    return true;
}

typedef struct SetStorageTest
{
    RBRInstrumentGen4StorageAccess access;
    const char *command;
    const char *response;
    RBRInstrumentGen4Error expectedError;
} SetStorageTest;

TEST_LOGGER4(setStorage)
{
    SetStorageTest tests[] = {
        { RBRINSTRUMENTGEN4_STORAGE_ACCESS_INSTRUMENT,
          "storage access=instrument" COMMAND_TERMINATOR,
          "storage access=instrument" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS },
        { RBRINSTRUMENTGEN4_STORAGE_ACCESS_USBHOST,
          "storage access=usbhost" COMMAND_TERMINATOR,
          "storage access=usbhost" RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_SUCCESS },
        /*
         * Selecting the location already in use is a warning, which the
         * library surfaces as a hardware error with the response type
         * distinguishing it.
         */
        { RBRINSTRUMENTGEN4_STORAGE_ACCESS_INSTRUMENT,
          "storage access=instrument" COMMAND_TERMINATOR,
          "WRN-305 storage access already at selected location"
          RESPONSE_TERMINATOR,
          RBRINSTRUMENTGEN4_HARDWARE_ERROR },
        /* Sentinel members are refused before the command. */
        { RBRINSTRUMENTGEN4_STORAGE_ACCESS_COUNT,
          "",
          "",
          RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE },
        { RBRINSTRUMENTGEN4_UNKNOWN_STORAGE_ACCESS,
          "",
          "",
          RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE },
        { 0, NULL, NULL, 0 }
    };

    RBRInstrumentGen4Error err;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        RBRInstrumentGen4Storage storage = {
            .access = tests[i].access
        };
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrumentGen4_setStorage(instrument, &storage);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError,
                            err,
                            RBRInstrumentGen4Error);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }

    return true;
}
