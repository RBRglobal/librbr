/**
 * \file configuration.c
 *
 * \brief Tests for instrument configuration commands.
 *
 * \copyright
 * Copyright (c) 2024 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include <math.h>
#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Configuration.h"
#include "tests.h"

bool test_calibration(RBRInstrumentGen4Calibration *expected,
                         RBRInstrumentGen4Calibration *actual)
{
    TEST_ASSERT_EQ(expected->dateTime, actual->dateTime, "%" PRIi64);
    TEST_ASSERT_FLOAT_EQ(expected->userOffset, actual->userOffset, 1e-6f);
    TEST_ASSERT_FLOAT_EQ(expected->userSlope, actual->userSlope, 1e-6f);
    for (uint32_t c = 0; c < RBRINSTRUMENTGEN4_CALIBRATION_C_COEFFICIENT_MAX; c++)
    {
        if (isnan(expected->c[c]))
        {
            TEST_ASSERT(isnan(actual->c[c]));
            break;
        }
        TEST_ASSERT_FLOAT_EQ(expected->c[c], actual->c[c], 1e-6f);
    }
    for (uint32_t x = 0; x < RBRINSTRUMENTGEN4_CALIBRATION_X_COEFFICIENT_MAX; x++)
    {
        if (isnan(expected->x[x]))
        {
            TEST_ASSERT(isnan(actual->x[x]));
            break;
        }
        TEST_ASSERT_FLOAT_EQ(expected->x[x], actual->x[x], 1e-6f);
    }
    /*
     * RBRInstrumentGen4Calibration.n holds pointers to the input channels, not
     * coefficient strings, so compare them as pointers. Nothing populates them
     * yet, which is why the string comparison this replaces dereferenced NULL.
     */
    for (uint32_t n = 0; n < RBRINSTRUMENTGEN4_CALIBRATION_N_COEFFICIENT_MAX; n++)
    {
        TEST_ASSERT_EQ(expected->n[n], actual->n[n], "%p");
    }
    return true;
}

TEST_LOGGER4(calibration)
{
    RBRInstrumentGen4Calibration expected = {
        .dateTime = 20171218175005,
        .userOffset = 0.0000000e+000,
        .userSlope = 1.0000000e+000,
        .c = {9.9876543e+000, 7.5642301e+000, NAN},
        .x = {NAN},
        .n = {NULL}
    };

    RBRInstrumentGen4Channel channel = {
        .label = "voltage_00"
    };
    RBRInstrumentGen4Calibration actual = {
        .parent = &channel
    };

    RBRInstrumentGen4Error err;

    TestIOBuffers_init(buffers,
                       "calibration voltage_00 equation=lin datetime=20171218175005 offset=0.0000000e+000 slope=1.0000000e+000 c0=9.9876543e+000 c1=7.5642301e+000" RESPONSE_TERMINATOR,
                       0);

    err = RBRInstrumentGen4_getCalibration(instrument,
                                           &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);

    return test_calibration(&expected, &actual);
}
