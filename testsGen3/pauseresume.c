/*
 * Copyright (c) 2022 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file pauseresume.c
 *
 * \brief Tests for instrument pauseresume commands.
 */

#include "tests.h"
#include "RBRGen3Pauseresume.h"

typedef struct PauseresumeTest {
    const char *command;
    const char *response;
    RBRGen3PauseresumeState state;
} PauseresumeTest;

typedef struct PauseTest {
    const char *command;
    const char *response;
    RBRGen3PauseStatus status;
} PauseTest;

typedef struct ResumeTest {
    const char *command;
    const char *response;
    RBRGen3ResumeStatus status;
} ResumeTest;

static bool test_pauseresume_error(RBRGen3 *conn, TestIOBuffers *buffers, PauseresumeTest *tests)
{
    RBRGen3Error err;
    RBRGen3PauseresumeState state;
    state = 3;
    for (int i = 0; tests[i].command != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_getPauseresume(conn, &state);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_HARDWARE_ERROR, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].state, state, RBRGen3PauseresumeState);
    }
    return true;
}

static bool test_pause_error(RBRGen3 *conn, TestIOBuffers *buffers, PauseTest *tests)
{
    RBRGen3Error err;
    RBRGen3PauseStatus status;
    status = 1;
    for (int i = 0; tests[i].command != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_pause(conn, &status);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_HARDWARE_ERROR, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].status, status, RBRGen3PauseStatus);
    }
    return true;
}

static bool test_resume_error(RBRGen3 *conn, TestIOBuffers *buffers, ResumeTest *tests)
{
    RBRGen3Error err;
    RBRGen3ResumeStatus status;
    status = 2;
    for (int i = 0; tests[i].command != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_resume(conn, &status);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_HARDWARE_ERROR, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].status, status, RBRGen3ResumeStatus);
    }
    return true;
}

TEST_LOGGER2(pauseresume_error)
{
    PauseresumeTest tests[] = {
        {"pauseresume" COMMAND_TERMINATOR, "E0102 invalid command" RESPONSE_TERMINATOR, 3},
        {0},
    };
    return test_pauseresume_error(conn, buffers, tests);
}

TEST_LOGGER2(pause_error)
{
    PauseTest tests[] = {
        {"pause" COMMAND_TERMINATOR, "E0102 invalid command" RESPONSE_TERMINATOR, 1},
        {0},
    };
    return test_pause_error(conn, buffers, tests);
}

TEST_LOGGER2(resume_error)
{
    ResumeTest tests[] = {
        {"resume" COMMAND_TERMINATOR, "E0102 invalid command" RESPONSE_TERMINATOR, 2},
        {0},
    };
    return test_resume_error(conn, buffers, tests);
}

TEST_LOGGER3(pauseresume_error)
{
    PauseresumeTest tests[] = {
        {"pauseresume" COMMAND_TERMINATOR, "E0102 invalid command" RESPONSE_TERMINATOR, 3},
        {"pauseresume" COMMAND_TERMINATOR, "E0109 feature not available" RESPONSE_TERMINATOR, 3},
        {0},
    };
    return test_pauseresume_error(conn, buffers, tests);
}

TEST_LOGGER3(pause_error)
{
    PauseTest tests[] = {
        {"pause" COMMAND_TERMINATOR, "E0102 invalid command" RESPONSE_TERMINATOR, 1},
        {"pause" COMMAND_TERMINATOR, "E0109 feature not available" RESPONSE_TERMINATOR, 1},
        {"pause" COMMAND_TERMINATOR, "E0406 not logging" RESPONSE_TERMINATOR, 1},
        {"pause" COMMAND_TERMINATOR,
         "E0415 more than one gating condition is enabled" RESPONSE_TERMINATOR,
         1},
        {"pause" COMMAND_TERMINATOR,
         "E0417 no gating allowed with regimes mode" RESPONSE_TERMINATOR,
         1},
        {0},
    };
    return test_pause_error(conn, buffers, tests);
}

TEST_LOGGER3(resume_error)
{
    ResumeTest tests[] = {
        {"resume" COMMAND_TERMINATOR, "E0102 invalid command" RESPONSE_TERMINATOR, 2},
        {"resume" COMMAND_TERMINATOR, "E0109 feature not available" RESPONSE_TERMINATOR, 2},
        {"resume" COMMAND_TERMINATOR, "E0406 not logging" RESPONSE_TERMINATOR, 2},
        {"resume" COMMAND_TERMINATOR,
         "E0415 more than one gating condition is enabled" RESPONSE_TERMINATOR,
         2},
        {"resume" COMMAND_TERMINATOR,
         "E0417 no gating allowed with regimes mode" RESPONSE_TERMINATOR,
         2},
        {0},
    };
    return test_resume_error(conn, buffers, tests);
}

static bool test_pauseresume(RBRGen3 *conn, TestIOBuffers *buffers, PauseresumeTest *tests)
{
    RBRGen3Error err;
    RBRGen3PauseresumeState state;
    state = 3;

    for (int i = 0; tests[i].command != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_getPauseresume(conn, &state);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].state, state, RBRGen3PauseresumeState);
    }
    return true;
}

static bool test_pause(RBRGen3 *conn, TestIOBuffers *buffers, PauseTest *tests)
{
    RBRGen3Error err;
    RBRGen3PauseStatus status;
    status = 1;
    for (int i = 0; tests[i].command != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_pause(conn, &status);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].status, status, RBRGen3PauseStatus);
    }
    return true;
}

static bool test_resume(RBRGen3 *conn, TestIOBuffers *buffers, ResumeTest *tests)
{
    RBRGen3Error err;
    RBRGen3ResumeStatus status;
    status = 2;
    for (int i = 0; tests[i].command != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_resume(conn, &status);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].status, status, RBRGen3ResumeStatus);
    }
    return true;
}

TEST_LOGGER3(pauseresume)
{
    PauseresumeTest tests[] = {
        {"pauseresume" COMMAND_TERMINATOR, "pauseresume state = n/a" RESPONSE_TERMINATOR, 0},
        {"pauseresume" COMMAND_TERMINATOR, "pauseresume state = paused" RESPONSE_TERMINATOR, 1},
        {"pauseresume" COMMAND_TERMINATOR, "pauseresume state = running" RESPONSE_TERMINATOR, 2},
        {0},
    };

    return test_pauseresume(conn, buffers, tests);
}

TEST_LOGGER3(pause)
{
    PauseTest tests[] = {
        {"pause" COMMAND_TERMINATOR, "pause status = paused" RESPONSE_TERMINATOR, 0},
        {0},
    };

    return test_pause(conn, buffers, tests);
}

TEST_LOGGER3(resume)
{
    ResumeTest tests[] = {
        {"resume" COMMAND_TERMINATOR, "resume status = pending" RESPONSE_TERMINATOR, 0},
        {"resume" COMMAND_TERMINATOR, "resume status = logging" RESPONSE_TERMINATOR, 1},
        {0},
    };

    return test_resume(conn, buffers, tests);
}
