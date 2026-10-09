/*
 * Copyright (c) 2026 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file tests.h
 *
 * \brief Testing definitions for the dynamic correction test suite.
 */

#ifndef LIBRBR_TESTSDYNAMICCORRECTION_TESTS_H
#define LIBRBR_TESTSDYNAMICCORRECTION_TESTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
/* Required for printf. */
#include <stdio.h>

#include "RBRDynamicCorrection.h"

/**
 * \brief Assert that a condition is true.
 *
 * If the assertion fails, an error message will be printed containing the file
 * name and line number on which the macro invocation occurs, and the
 * surrounding function will `return false;`.
 *
 * \param [in] _condition the condition to test
 */
#define TEST_ASSERT(_condition)                                        \
    do {                                                               \
        if (!(_condition)) {                                           \
            printf(" assertion failure at %s:%d", __FILE__, __LINE__); \
            return false;                                              \
        }                                                              \
    } while (0)

/**
 * \brief Assert that two variables are equal.
 *
 * If the assertion fails, an error message will be printed containing the file
 * name and line number on which the macro invocation occurs and the expected
 * and actual values, and the surrounding function will `return false;`.
 *
 * Because \a _expected, \a _actual, and \a _type will be evaluated multiple
 * times by the macro, do not pass expressions having side effects.
 *
 * \param [in] _expected the expected value
 * \param [in] _actual the actual value
 * \param [in] _type a printf format string suitable for the values
 */
#define TEST_ASSERT_EQ(_expected, _actual, _type)        \
    do {                                                 \
        if ((_expected) != (_actual)) {                  \
            printf(" assertion failure at %s:%d:"        \
                   " expected " _type "; actual " _type, \
                   __FILE__,                             \
                   __LINE__,                             \
                   _expected,                            \
                   _actual);                             \
            return false;                                \
        }                                                \
    } while (0)

/**
 * \brief Assert that two float variables are equal.
 *
 * If the assertion fails, an error message will be printed containing the file
 * name and line number on which the macro invocation occurs and the expected
 * and actual values, and the surrounding function will `return false;`.
 *
 * Because \a _expected, \a _actual, and \a _eps will be evaluated multiple
 * times by the macro, do not pass expressions having side effects.
 *
 * \param [in] _expected the expected value
 * \param [in] _actual the actual value
 * \param [in] _eps the precision range for comparison
 */
#define TEST_ASSERT_FLOAT_EQ(_expected, _actual, _eps)                                      \
    do {                                                                                    \
        if (((_expected) < ((_actual) - (_eps))) || ((_expected) > ((_actual) + (_eps)))) { \
            printf(" assertion failure at %s:%d:"                                           \
                   " expected %f ; actual %f",                                              \
                   __FILE__,                                                                \
                   __LINE__,                                                                \
                   (double) (_expected),                                                    \
                   (double) (_actual));                                                     \
            return false;                                                                   \
        }                                                                                   \
    } while (0)

/**
 * \brief Declare a test function.
 *
 * \param [in] fn the name of the test function
 */
#define TEST(fn) static bool fn(void)

/**
 * \brief Declaration of a dynamic correction test.
 */
typedef struct DynamicCorrectionTest {
    /** \brief The name of the test. */
    const char *name;
    /** \brief The test to be run. */
    bool (*function)(void);
} DynamicCorrectionTest;

/**
 * \brief All the dynamic correction tests to run, terminated by an entry
 * with a `NULL` function.
 */
extern const DynamicCorrectionTest dynamicCorrectionTests[];

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_TESTSDYNAMICCORRECTION_TESTS_H */
