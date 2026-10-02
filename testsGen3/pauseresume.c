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
#include "RBRGen3PauseResume.h"

typedef struct PauseResumeTest {
    const char *command;
    const char *response;
    RBRGen3PauseResumeState state;
} PauseResumeTest;

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

static bool test_pauseResume_error(RBRGen3 *conn, TestIOBuffers *buffers, PauseResumeTest *tests)
{
    RBRGen3Error err;
    RBRGen3PauseResumeState state;
    state = 3;
    for (int i = 0; tests[i].command != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_getPauseResume(conn, &state);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_HARDWARE_ERROR, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].state, state, RBRGen3PauseResumeState);
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
    PauseResumeTest tests[] = {
        {
            .command = "pauseresume" COMMAND_TERMINATOR,
            .response = "E0102 invalid command" RESPONSE_TERMINATOR,
            .state = 3,
        },
        {0},
    };
    return test_pauseResume_error(conn, buffers, tests);
}

TEST_LOGGER2(pause_error)
{
    PauseTest tests[] = {
        {
            .command = "pause" COMMAND_TERMINATOR,
            .response = "E0102 invalid command" RESPONSE_TERMINATOR,
            .status = 1,
        },
        {0},
    };
    return test_pause_error(conn, buffers, tests);
}

TEST_LOGGER2(resume_error)
{
    ResumeTest tests[] = {
        {
            .command = "resume" COMMAND_TERMINATOR,
            .response = "E0102 invalid command" RESPONSE_TERMINATOR,
            .status = 2,
        },
        {0},
    };
    return test_resume_error(conn, buffers, tests);
}

TEST_LOGGER3(pauseresume_error)
{
    PauseResumeTest tests[] = {
        {
            .command = "pauseresume" COMMAND_TERMINATOR,
            .response = "E0102 invalid command" RESPONSE_TERMINATOR,
            .state = 3,
        },
        {
            .command = "pauseresume" COMMAND_TERMINATOR,
            .response = "E0109 feature not available" RESPONSE_TERMINATOR,
            .state = 3,
        },
        {0},
    };
    return test_pauseResume_error(conn, buffers, tests);
}

TEST_LOGGER3(pause_error)
{
    PauseTest tests[] = {
        {
            .command = "pause" COMMAND_TERMINATOR,
            .response = "E0102 invalid command" RESPONSE_TERMINATOR,
            .status = 1,
        },
        {
            .command = "pause" COMMAND_TERMINATOR,
            .response = "E0109 feature not available" RESPONSE_TERMINATOR,
            .status = 1,
        },
        {
            .command = "pause" COMMAND_TERMINATOR,
            .response = "E0406 not logging" RESPONSE_TERMINATOR,
            .status = 1,
        },
        {
            .command = "pause" COMMAND_TERMINATOR,
            .response = "E0415 more than one gating condition is enabled" RESPONSE_TERMINATOR,
            .status = 1,
        },
        {
            .command = "pause" COMMAND_TERMINATOR,
            .response = "E0417 no gating allowed with regimes mode" RESPONSE_TERMINATOR,
            .status = 1,
        },
        {0},
    };
    return test_pause_error(conn, buffers, tests);
}

TEST_LOGGER3(resume_error)
{
    ResumeTest tests[] = {
        {
            .command = "resume" COMMAND_TERMINATOR,
            .response = "E0102 invalid command" RESPONSE_TERMINATOR,
            .status = 2,
        },
        {
            .command = "resume" COMMAND_TERMINATOR,
            .response = "E0109 feature not available" RESPONSE_TERMINATOR,
            .status = 2,
        },
        {
            .command = "resume" COMMAND_TERMINATOR,
            .response = "E0406 not logging" RESPONSE_TERMINATOR,
            .status = 2,
        },
        {
            .command = "resume" COMMAND_TERMINATOR,
            .response = "E0415 more than one gating condition is enabled" RESPONSE_TERMINATOR,
            .status = 2,
        },
        {
            .command = "resume" COMMAND_TERMINATOR,
            .response = "E0417 no gating allowed with regimes mode" RESPONSE_TERMINATOR,
            .status = 2,
        },
        {0},
    };
    return test_resume_error(conn, buffers, tests);
}

static bool test_pauseResume(RBRGen3 *conn, TestIOBuffers *buffers, PauseResumeTest *tests)
{
    RBRGen3Error err;
    RBRGen3PauseResumeState state;
    state = 3;

    for (int i = 0; tests[i].command != NULL; i++) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_getPauseResume(conn, &state);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_ENUM_EQ(tests[i].state, state, RBRGen3PauseResumeState);
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
    PauseResumeTest tests[] = {
        {
            .command = "pauseresume" COMMAND_TERMINATOR,
            .response = "pauseresume state = n/a" RESPONSE_TERMINATOR,
            .state = 0,
        },
        {
            .command = "pauseresume" COMMAND_TERMINATOR,
            .response = "pauseresume state = paused" RESPONSE_TERMINATOR,
            .state = 1,
        },
        {
            .command = "pauseresume" COMMAND_TERMINATOR,
            .response = "pauseresume state = running" RESPONSE_TERMINATOR,
            .state = 2,
        },
        {0},
    };

    return test_pauseResume(conn, buffers, tests);
}

TEST_LOGGER3(pause)
{
    PauseTest tests[] = {
        {
            .command = "pause" COMMAND_TERMINATOR,
            .response = "pause status = paused" RESPONSE_TERMINATOR,
            .status = 0,
        },
        {0},
    };

    return test_pause(conn, buffers, tests);
}

TEST_LOGGER3(resume)
{
    ResumeTest tests[] = {
        {
            .command = "resume" COMMAND_TERMINATOR,
            .response = "resume status = pending" RESPONSE_TERMINATOR,
            .status = 0,
        },
        {
            .command = "resume" COMMAND_TERMINATOR,
            .response = "resume status = logging" RESPONSE_TERMINATOR,
            .status = 1,
        },
        {0},
    };

    return test_resume(conn, buffers, tests);
}
