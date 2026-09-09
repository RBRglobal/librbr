/**
 * \file pauseresume.c
 *
 * \brief Tests for instrument pauseresume commands.
 *
 * \copyright
 * Copyright (c) 2022 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include "tests.h"

typedef struct PauseresumeTest
{
    const char *command;
    const char *response;
    RBRInstrumentPauseresumeState state;
} PauseresumeTest;

typedef struct PauseTest
{
    const char *command;
    const char *response;
    RBRInstrumentPauseStatus status;
} PauseTest;

typedef struct ResumeTest
{
    const char *command;
    const char *response;
    RBRInstrumentResumeStatus status;
} ResumeTest;

static bool test_pauseresume_error(RBRInstrument *instrument,
                                   TestIOBuffers *buffers,
                                   PauseresumeTest *tests)
{
    RBRInstrumentError err;
    RBRInstrumentPauseresumeState state;
    state = 3;
    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrument_getPauseresume(instrument, &state);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_HARDWARE_ERROR, err, RBRInstrumentError);
        TEST_ASSERT_ENUM_EQ(tests[i].state, state, RBRInstrumentPauseresumeState);
    }
    return true;
}

static bool test_pause_error(RBRInstrument *instrument,
                             TestIOBuffers *buffers,
                             PauseTest *tests)
{
    RBRInstrumentError err;
    RBRInstrumentPauseStatus status;
    status = 1;
    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrument_pause(instrument, &status);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_HARDWARE_ERROR, err, RBRInstrumentError);
        TEST_ASSERT_ENUM_EQ(tests[i].status, status, RBRInstrumentPauseStatus);
    }
    return true;
}

static bool test_resume_error(RBRInstrument *instrument,
                              TestIOBuffers *buffers,
                              ResumeTest *tests)
{
    RBRInstrumentError err;
    RBRInstrumentResumeStatus status;
    status = 2;
    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrument_resume(instrument, &status);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_HARDWARE_ERROR, err, RBRInstrumentError);
        TEST_ASSERT_ENUM_EQ(tests[i].status, status, RBRInstrumentResumeStatus);
    }
    return true;
}

TEST_LOGGER2(pauseresume_error)
{
    PauseresumeTest tests[] = {
        {"pauseresume" COMMAND_TERMINATOR,
         "E0102 invalid command" RESPONSE_TERMINATOR,
         3},
        {0}};
    return test_pauseresume_error(instrument, buffers, tests);
}

TEST_LOGGER2(pause_error)
{
    PauseTest tests[] = {
        {"pause" COMMAND_TERMINATOR,
         "E0102 invalid command" RESPONSE_TERMINATOR,
         1},
        {0}};
    return test_pause_error(instrument, buffers, tests);
}

TEST_LOGGER2(resume_error)
{
    ResumeTest tests[] = {
        {"resume" COMMAND_TERMINATOR,
         "E0102 invalid command" RESPONSE_TERMINATOR,
         2},
        {0}};
    return test_resume_error(instrument, buffers, tests);
}

TEST_LOGGER3(pauseresume_error)
{
    PauseresumeTest tests[] = {
        {"pauseresume" COMMAND_TERMINATOR,
         "E0102 invalid command" RESPONSE_TERMINATOR,
         3},
        {"pauseresume" COMMAND_TERMINATOR,
         "E0109 feature not available" RESPONSE_TERMINATOR,
         3},
        {0}};
    return test_pauseresume_error(instrument, buffers, tests);
}

TEST_LOGGER3(pause_error)
{
    PauseTest tests[] = {
        {"pause" COMMAND_TERMINATOR,
         "E0102 invalid command" RESPONSE_TERMINATOR,
         1},
        {"pause" COMMAND_TERMINATOR,
         "E0109 feature not available" RESPONSE_TERMINATOR,
         1},
        {"pause" COMMAND_TERMINATOR,
         "E0406 not logging" RESPONSE_TERMINATOR,
         1},
        {"pause" COMMAND_TERMINATOR,
         "E0415 more than one gating condition is enabled" RESPONSE_TERMINATOR,
         1},
        {"pause" COMMAND_TERMINATOR,
         "E0417 no gating allowed with regimes mode" RESPONSE_TERMINATOR,
         1},
        {0}};
    return test_pause_error(instrument, buffers, tests);
}

TEST_LOGGER3(resume_error)
{
    ResumeTest tests[] = {
        {"resume" COMMAND_TERMINATOR,
         "E0102 invalid command" RESPONSE_TERMINATOR,
         2},
        {"resume" COMMAND_TERMINATOR,
         "E0109 feature not available" RESPONSE_TERMINATOR,
         2},
        {"resume" COMMAND_TERMINATOR,
         "E0406 not logging" RESPONSE_TERMINATOR,
         2},
        {"resume" COMMAND_TERMINATOR,
         "E0415 more than one gating condition is enabled" RESPONSE_TERMINATOR,
         2},
        {"resume" COMMAND_TERMINATOR,
         "E0417 no gating allowed with regimes mode" RESPONSE_TERMINATOR,
         2},
        {0}};
    return test_resume_error(instrument, buffers, tests);
}

static bool test_pauseresume(RBRInstrument *instrument,
                             TestIOBuffers *buffers,
                             PauseresumeTest *tests)
{
    RBRInstrumentError err;
    RBRInstrumentPauseresumeState state;
    state = 3;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrument_getPauseresume(instrument, &state);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_ENUM_EQ(tests[i].state, state, RBRInstrumentPauseresumeState);
    }
    return true;
}

static bool test_pause(RBRInstrument *instrument,
                       TestIOBuffers *buffers,
                       PauseTest *tests)
{
    RBRInstrumentError err;
    RBRInstrumentPauseStatus status;
    status = 1;
    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrument_pause(instrument, &status);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_ENUM_EQ(tests[i].status, status, RBRInstrumentPauseStatus);
    }
    return true;
}

static bool test_resume(RBRInstrument *instrument,
                              TestIOBuffers *buffers,
                              ResumeTest *tests)
{
    RBRInstrumentError err;
    RBRInstrumentResumeStatus status;
    status = 2;
    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrument_resume(instrument, &status);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_ENUM_EQ(tests[i].status, status, RBRInstrumentResumeStatus);
    }
    return true;
}

TEST_LOGGER3(pauseresume)
{
    PauseresumeTest tests[] = {
        {"pauseresume" COMMAND_TERMINATOR,
         "pauseresume state = n/a" RESPONSE_TERMINATOR,
         0},
        {"pauseresume" COMMAND_TERMINATOR,
         "pauseresume state = paused" RESPONSE_TERMINATOR,
         1},
        {"pauseresume" COMMAND_TERMINATOR,
         "pauseresume state = running" RESPONSE_TERMINATOR,
         2},
        {0}};

    return test_pauseresume(instrument, buffers, tests);
}

TEST_LOGGER3(pause)
{
    PauseTest tests[] = {
        {"pause" COMMAND_TERMINATOR,
         "pause status = paused" RESPONSE_TERMINATOR,
         0},
        {0}};

    return test_pause(instrument, buffers, tests);
}

TEST_LOGGER3(resume)
{
    ResumeTest tests[] = {
        {"resume" COMMAND_TERMINATOR,
         "resume status = pending" RESPONSE_TERMINATOR,
         0},
        {"resume" COMMAND_TERMINATOR,
         "resume status = logging" RESPONSE_TERMINATOR,
         1},
        {0}};

    return test_resume(instrument, buffers, tests);
}
