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
    TEST_ASSERT_EQ(expected->userOffset, actual->userOffset, "%f");
    TEST_ASSERT_EQ(expected->userSlope, actual->userSlope, "%f");
    for (uint32_t c = 0; c < RBRINSTRUMENTGEN4_CALIBRATION_C_COEFFICIENT_MAX; c++)
    {
        if (isnan(expected->c[c]))
        {
            TEST_ASSERT(isnan(actual->c[c]));
            break;
        }
        TEST_ASSERT_EQ(expected->c[c], actual->c[c], "%f");
    }
    for (uint32_t x = 0; x < RBRINSTRUMENTGEN4_CALIBRATION_X_COEFFICIENT_MAX; x++)
    {
        if (isnan(expected->x[x]))
        {
            TEST_ASSERT(isnan(actual->x[x]));
            break;
        }
        TEST_ASSERT_EQ(expected->x[x], actual->x[x], "%f");
    }
    for (uint32_t n = 0; n < RBRINSTRUMENTGEN4_CALIBRATION_N_COEFFICIENT_MAX; n++)
    {
        TEST_ASSERT_STR_EQ(expected->n[n], actual->n[n]);
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

TEST_LOGGER4(channel)
{
    RBRInstrumentGen4Channel expected = {
        .label = "conductivity_00",
        .type = "cond00",
        .address = 32,
        .settlingTime = 50,
        .readTime = 260,
        .guardTime = 20,
        .equation = "lin",
        .userUnits = "mS/cm",
        .derived = false,
        .gain = {
            .currentGain = 1,
            .availableGains = {1}
        },
        .calibration = {
            .dateTime = 0,
            .userOffset = NAN,
            .userSlope = NAN,
            .c = {NAN},
            .x = {NAN},
            .n = {NULL}
        }
    };

    RBRInstrumentGen4Channel actual = {
        .label = "conductivity_00"
    };

    RBRInstrumentGen4Error err;

    TestIOBuffers_init(buffers,
                       "channel conductivity_00 type=cond00 address=32 settlingtime=50 readtime=260 guardtime=20 userunits=mS/cm derived=off grouplist=none sensor=none" RESPONSE_TERMINATOR,
                       0);

    err = RBRInstrumentGen4_getChannel(instrument,
                                       &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENTGEN4_SUCCESS, err, RBRInstrumentGen4Error);

    TEST_ASSERT_STR_EQ(expected.label, actual.label);
    TEST_ASSERT_STR_EQ(expected.type, actual.type);
    TEST_ASSERT_EQ(expected.address, actual.address, "%" PRIi32);
    TEST_ASSERT_EQ(expected.settlingTime, actual.settlingTime, "%" PRIi32);
    TEST_ASSERT_EQ(expected.readTime, actual.readTime, "%" PRIi32);
    TEST_ASSERT_EQ(expected.guardTime, actual.guardTime, "%" PRIi32);
    TEST_ASSERT_STR_EQ(expected.equation, actual.equation);
    TEST_ASSERT_STR_EQ(expected.userUnits, actual.userUnits);
    TEST_ASSERT_EQ(expected.derived, actual.derived, "%" PRIi32);
    TEST_ASSERT_EQ(expected.gain.currentGain, actual.gain.currentGain, "%f");
    for (int32_t i = 0; i < RBRINSTRUMENTGEN4_CHANNEL_GAINS_MAX; i++)
    {
        TEST_ASSERT_EQ(expected.gain.availableGains[i],
                       actual.gain.availableGains[i],
                       "%f");
    }
    return test_calibration(&expected.calibration, &actual.calibration);
}
