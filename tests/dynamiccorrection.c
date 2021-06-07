/**
 * \file dynamiccorrection.c
 *
 * \brief Tests for dynamic correction
 *
 * \copyright
 * Copyright (c) 2021 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include "tests.h"



/* declaration of private functions */
float RBRDynamicCorrection_PSS78(float C, float T, float P);


typedef struct Pss78Test
{
    float C_value;
    float T_value;
    float P_value;
    float S_expected;
} Pss78Test;

static bool test_verify_pss78(Pss78Test *tests)
{
    float S_result;

    for (int i = 0; tests[i].C_value > 0.0f; i++)
    {
        S_result = RBRDynamicCorrection_PSS78(tests[i].C_value, tests[i].T_value, tests[i].P_value);

        TEST_ASSERT_FLOAT_EQ(tests[i].S_expected, S_result, 1e-3f);
    }

    return true;
}

TEST_LOGGER3(verify_pss78)
{
    Pss78Test tests[] = {
        { 110.0f, 5.0f, 2500.0f, 138.626f },
        { 55.0f, 5.0f, 2500.0f, 59.4009f },
        { 55.0f, 21.0f, 2500.0f, 39.0323f },
        { 55.0f, 21.0f, 100.0f, 39.8831f },
        { 0.0f, 0.0f, 0.0f, 0.0f }
    };

    return test_verify_pss78(tests);
}

