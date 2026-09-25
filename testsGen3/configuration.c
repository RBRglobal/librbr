/**
 * \file configuration.c
 *
 * \brief Tests for instrument configuration commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include <math.h>
#include "tests.h"
#include "RBRGen3Configuration.h"

typedef struct ChannelsTest {
    const char *response;
    RBRGen3Channels expected;
} ChannelsTest;

static bool test_channels(RBRGen3 *conn, TestIOBuffers *buffers, ChannelsTest *tests)
{
    RBRGen3Error err;
    RBRGen3Channels actual;

    for (int i = 0; tests[i].response != NULL; ++i) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_getChannels(conn, &actual);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_EQ(tests[i].expected.count, actual.count, "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.on, actual.on, "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.settlingTime, actual.settlingTime, "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.readTime, actual.readTime, "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.minimumPeriod, actual.minimumPeriod, "%" PRIi32);

        for (int channel = 0; channel < actual.count; ++channel) {
            RBRGen3Channel *expectedChannel = &tests[i].expected.channels[channel];
            RBRGen3Channel *actualChannel = &actual.channels[channel];

            TEST_ASSERT_STR_EQ(expectedChannel->type, actualChannel->type);
            TEST_ASSERT_EQ(expectedChannel->module, actualChannel->module, "%" PRIi32);
            TEST_ASSERT_ENUM_EQ(expectedChannel->status, actualChannel->status, bool);
            TEST_ASSERT_EQ(expectedChannel->settlingTime, actualChannel->settlingTime, "%" PRIi32);
            TEST_ASSERT_EQ(expectedChannel->readTime, actualChannel->readTime, "%" PRIi32);
            TEST_ASSERT_STR_EQ(expectedChannel->equation, actualChannel->equation);
            TEST_ASSERT_STR_EQ(expectedChannel->userUnits, actualChannel->userUnits);
            TEST_ASSERT_ENUM_EQ(expectedChannel->derived, actualChannel->derived, bool);

            TEST_ASSERT_ENUM_EQ(expectedChannel->gain.rangingMode,
                                actualChannel->gain.rangingMode,
                                RBRGen3ChannelRangingMode);
            if (isnan(expectedChannel->gain.currentGain)) {
                TEST_ASSERT(isnan(actualChannel->gain.currentGain));
            } else {
                TEST_ASSERT_FLOAT_EQ(
                    expectedChannel->gain.currentGain, actualChannel->gain.currentGain, 0.0f);
            }
            int i = 0;
            while (true) {
                if (isnan(expectedChannel->gain.availableGains[i])) {
                    TEST_ASSERT(isnan(actualChannel->gain.availableGains[i]));
                    break;
                } else {
                    TEST_ASSERT_FLOAT_EQ(expectedChannel->gain.availableGains[i],
                                         actualChannel->gain.availableGains[i],
                                         0.0f);
                }

                ++i;
            }

            TEST_ASSERT_STR_EQ(expectedChannel->label, actualChannel->label);

            TEST_ASSERT_EQ(expectedChannel->calibration.dateTime,
                           actualChannel->calibration.dateTime,
                           "%" PRIi64);

            TEST_ASSERT_EQ(
                expectedChannel->calibration.cCount, actualChannel->calibration.cCount, "%" PRIi32);
            for (int32_t i = 0; i < expectedChannel->calibration.cCount; ++i) {
                TEST_ASSERT_FLOAT_EQ(
                    expectedChannel->calibration.c[i], actualChannel->calibration.c[i], 0.0f);
            }

            TEST_ASSERT_EQ(
                expectedChannel->calibration.xCount, actualChannel->calibration.xCount, "%" PRIi32);
            for (int32_t i = 0; i < expectedChannel->calibration.xCount; ++i) {
                TEST_ASSERT_FLOAT_EQ(
                    expectedChannel->calibration.x[i], actualChannel->calibration.x[i], 0.0f);
            }

            TEST_ASSERT_EQ(
                expectedChannel->calibration.nCount, actualChannel->calibration.nCount, "%" PRIi32);
            for (int32_t i = 0; i < expectedChannel->calibration.nCount; ++i) {
                TEST_ASSERT_EQ(
                    expectedChannel->calibration.n[i], actualChannel->calibration.n[i], "%" PRIi8);
            }
        }
    }

    return true;
}

TEST_LOGGER2(channels)
{
    ChannelsTest tests[] = {
        {
            "channels count = 3, on = 3, latency = 300, readtime = 350, "
            "minperiod = 480" RESPONSE_TERMINATOR

            "channel 1 type = temp09, module = 1, status = on, latency = 50, "
            "readtime = 260, equation = tmp, userunits = C, gain = none, "
            "gainsavailable = none, derived = off" RESPONSE_TERMINATOR

            "calibration 1 type = temp09, datetime = 20000401000000, "
            "c0 = 3.5000000e-003, c1 = -250.00002e-006, c2 = 2.7000000e-006, "
            "c3 = 23.000000e-009" RESPONSE_TERMINATOR

            "channel 2 type = pres19, module = 2, status = on, latency = 50, "
            "readtime = 260, equation = corr_pres2, userunits = dbar, "
            "gain = none, gainsavailable = none, derived = off" RESPONSE_TERMINATOR

            "calibration 2 type = pres19, datetime = 20000401000000, "
            "c0 = 0.0000000e+000, c1 = 1.0000000e+000, c2 = 0.0000000e+000, "
            "c3 = 0.0000000e+000, x0 = 0.0000000e+000, x1 = 0.0000000e+000, "
            "x2 = 0.0000000e+000, x3 = 0.0000000e+000, x4 = 0.0000000e+000, "
            "x5 = 0.0000000e+000, n0 = value" RESPONSE_TERMINATOR

            "channel 3 type = volt00, module = 40, status = on, "
            "latency = 300, readtime = 350, equation = lin, userunits = V, "
            "gain = none, gainsavailable = none, derived = off" RESPONSE_TERMINATOR

            "calibration 3 type = volt00, datetime = 20000401000000, "
            "c0 = 0.0000000e+000, c1 = 1.0000000e+000" RESPONSE_TERMINATOR,

            {
                .count = 3,
                .on = 3,
                .settlingTime = 300,
                .readTime = 350,
                .minimumPeriod = 480,
                .channels =
                    {
                        {
                            .type = "temp09",
                            .module = 1,
                            .status = true,
                            .settlingTime = 50,
                            .readTime = 260,
                            .equation = "tmp",
                            .userUnits = "C",
                            .gain =
                                {
                                    .rangingMode = RBRGEN3_RANGING_NONE,
                                    .currentGain = NAN,
                                    .availableGains = {NAN},
                                },
                            .derived = false,
                            .label = "none",
                            .calibration =
                                {
                                    .dateTime = 954547200000LL,
                                    .cCount = 4,
                                    .c =
                                        {
                                            3.5000000e-003,
                                            -250.00002e-006,
                                            2.7000000e-006,
                                            23.000000e-009,
                                        },
                                },
                        },
                        {
                            .type = "pres19",
                            .module = 2,
                            .status = true,
                            .settlingTime = 50,
                            .readTime = 260,
                            .equation = "corr_pres2",
                            .userUnits = "dbar",
                            .gain =
                                {
                                    .rangingMode = RBRGEN3_RANGING_NONE,
                                    .currentGain = NAN,
                                    .availableGains = {NAN},
                                },
                            .derived = false,
                            .label = "none",
                            .calibration =
                                {
                                    .dateTime = 954547200000LL,
                                    .cCount = 4,
                                    .c =
                                        {
                                            0.0000000e+000,
                                            1.0000000e+000,
                                            0.0000000e+000,
                                            0.0000000e+000,
                                        },
                                    .xCount = 6,
                                    .x =
                                        {
                                            0.0000000e+000,
                                            0.0000000e+000,
                                            0.0000000e+000,
                                            0.0000000e+000,
                                            0.0000000e+000,
                                            0.0000000e+000,
                                        },
                                    .nCount = 1,
                                    .n = {RBRGEN3_VALUE_COEFFICIENT},
                                },
                        },
                        {
                            .type = "volt00",
                            .module = 40,
                            .status = true,
                            .settlingTime = 300,
                            .readTime = 350,
                            .equation = "lin",
                            .userUnits = "V",
                            .gain =
                                {
                                    .rangingMode = RBRGEN3_RANGING_NONE,
                                    .currentGain = NAN,
                                    .availableGains = {NAN},
                                },
                            .derived = false,
                            .label = "none",
                            .calibration =
                                {
                                    .dateTime = 954547200000LL,
                                    .cCount = 2,
                                    .c =
                                        {
                                            0.0000000e+000,
                                            1.0000000e+000,
                                        },
                                },
                        },
                    },
            },
        },
        {
            "channels count = 1, on = 1, latency = 600, readtime = 1700, "
            "minperiod = 1910" RESPONSE_TERMINATOR
            "channel 1 type = fluo01, module = 40, status = on, "
            "latency = 600, readtime = 1700, equation = lin, "
            "userunits = ug/L, gain = auto, "
            "gainsavailable = 1.0|3.0|10.0|30.0, derived = off" RESPONSE_TERMINATOR
            "calibration 1 type = fluo01, datetime = 20000401000000, "
            "c0 = 203.47984e+000, c1 = -277.72070e+000" RESPONSE_TERMINATOR,
            {
                .count = 1,
                .on = 1,
                .settlingTime = 600,
                .readTime = 1700,
                .minimumPeriod = 1910,
                .channels =
                    {
                        {
                            .type = "fluo01",
                            .module = 40,
                            .status = true,
                            .settlingTime = 600,
                            .readTime = 1700,
                            .equation = "lin",
                            .userUnits = "ug/L",
                            .gain =
                                {
                                    .rangingMode = RBRGEN3_RANGING_AUTO,
                                    .currentGain = NAN,
                                    .availableGains = {1.0, 3.0, 10.0, 30.0, NAN},
                                },
                            .derived = false,
                            .label = "none",
                            .calibration =
                                {
                                    .dateTime = 954547200000LL,
                                    .cCount = 2,
                                    .c =
                                        {
                                            203.47984e+000,
                                            -277.72070e+000,
                                        },
                                },
                        },
                    },
            },
        },
        {0},
    };

    return test_channels(conn, buffers, tests);
}

TEST_LOGGER3(channels)
{
    ChannelsTest tests[] = {
        {
            "channels count = 5, on = 5, settlingtime = 50, readtime = 290, "
            "minperiod = 450" RESPONSE_TERMINATOR

            "channel 1 type = temp09, module = 1, status = on, "
            "settlingtime = 50, readtime = 260, equation = tmp, "
            "userunits = C, gain = none, availablegains = none, "
            "derived = off, label = temperature_00" RESPONSE_TERMINATOR

            "calibration 1 label = temperature_00, datetime = 20000401000000, "
            "c0 = 3.5000000e-003, c1 = -250.00002e-006, c2 = 2.7000000e-006, "
            "c3 = 23.000000e-009" RESPONSE_TERMINATOR

            "channel 2 type = pres24, module = 2, status = on, "
            "settlingtime = 50, readtime = 290, equation = corr_pres2, "
            "userunits = dbar, gain = none, availablegains = none, "
            "derived = off, label = pressure_00" RESPONSE_TERMINATOR

            "calibration 2 label = pressure_00, "
            "datetime = 20000401000000, c0 = 0.0000000e+000, "
            "c1 = 1.0000000e+000, c2 = 0.0000000e+000, c3 = 0.0000000e+000, "
            "x0 = 0.0000000e+000, x1 = 0.0000000e+000, x2 = 0.0000000e+000, "
            "x3 = 0.0000000e+000, x4 = 0.0000000e+000, x5 = 0.0000000e+000, "
            "n0 = 6" RESPONSE_TERMINATOR

            "channel 3 type = pres08, module = 240, status = on, "
            "settlingtime = 0, readtime = 0, equation = deri_seapres, "
            "userunits = dbar, gain = none, availablegains = none, "
            "derived = on, label = seapressure_00" RESPONSE_TERMINATOR

            "calibration 3 label = seapressure_00, datetime = 20000401000000, "
            "n0 = 2, n1 = value" RESPONSE_TERMINATOR

            "channel 4 type = dpth01, module = 241, status = on, "
            "settlingtime = 0, readtime = 0, equation = deri_depth, "
            "userunits = m, gain = none, availablegains = none, derived = on, "
            "label = depth_00" RESPONSE_TERMINATOR

            "calibration 4 label = depth_00, datetime = 20000401000000, "
            "n0 = 2, n1 = value" RESPONSE_TERMINATOR

            "channel 5 type = cnt_00, module = 242, status = on, "
            "settlingtime = 0, readtime = 0, equation = none, "
            "userunits = counts, gain = none, availablegains = none, "
            "derived = on, label = count_00" RESPONSE_TERMINATOR

            "calibration 5 label = count_00, datetime = 20000401000000, "
            "n0 = value" RESPONSE_TERMINATOR,

            {
                .count = 5,
                .on = 5,
                .settlingTime = 50,
                .readTime = 290,
                .minimumPeriod = 450,
                .channels =
                    {
                        {
                            .type = "temp09",
                            .module = 1,
                            .status = true,
                            .settlingTime = 50,
                            .readTime = 260,
                            .equation = "tmp",
                            .userUnits = "C",
                            .gain =
                                {
                                    .rangingMode = RBRGEN3_RANGING_NONE,
                                    .currentGain = NAN,
                                    .availableGains = {NAN},
                                },
                            .derived = false,
                            .label = "temperature_00",
                            .calibration =
                                {
                                    .dateTime = 954547200000LL,
                                    .cCount = 4,
                                    .c =
                                        {
                                            3.5000000e-003,
                                            -250.00002e-006,
                                            2.7000000e-006,
                                            23.000000e-009,
                                        },
                                },
                        },
                        {
                            .type = "pres24",
                            .module = 2,
                            .status = true,
                            .settlingTime = 50,
                            .readTime = 290,
                            .equation = "corr_pres2",
                            .userUnits = "dbar",
                            .gain =
                                {
                                    .rangingMode = RBRGEN3_RANGING_NONE,
                                    .currentGain = NAN,
                                    .availableGains = {NAN},
                                },
                            .derived = false,
                            .label = "pressure_00",
                            .calibration =
                                {
                                    .dateTime = 954547200000LL,
                                    .cCount = 4,
                                    .c =
                                        {
                                            0.0000000e+000,
                                            1.0000000e+000,
                                            0.0000000e+000,
                                            0.0000000e+000,
                                        },
                                    .xCount = 6,
                                    .x =
                                        {
                                            0.0000000e+000,
                                            0.0000000e+000,
                                            0.0000000e+000,
                                            0.0000000e+000,
                                            0.0000000e+000,
                                            0.0000000e+000,
                                        },
                                    .nCount = 1,
                                    .n = {6},
                                },
                        },
                        {
                            .type = "pres08",
                            .module = 240,
                            .status = true,
                            .settlingTime = 0,
                            .readTime = 0,
                            .equation = "deri_seapres",
                            .userUnits = "dbar",
                            .gain =
                                {
                                    .rangingMode = RBRGEN3_RANGING_NONE,
                                    .currentGain = NAN,
                                    .availableGains = {NAN},
                                },
                            .derived = true,
                            .label = "seapressure_00",
                            .calibration =
                                {
                                    .dateTime = 954547200000LL,
                                    .nCount = 2,
                                    .n = {2, RBRGEN3_VALUE_COEFFICIENT},
                                },
                        },
                        {
                            .type = "dpth01",
                            .module = 241,
                            .status = true,
                            .settlingTime = 0,
                            .readTime = 0,
                            .equation = "deri_depth",
                            .userUnits = "m",
                            .gain =
                                {
                                    .rangingMode = RBRGEN3_RANGING_NONE,
                                    .currentGain = NAN,
                                    .availableGains = {NAN},
                                },
                            .derived = true,
                            .label = "depth_00",
                            .calibration =
                                {
                                    .dateTime = 954547200000LL,
                                    .nCount = 2,
                                    .n = {2, RBRGEN3_VALUE_COEFFICIENT},
                                },
                        },
                        {
                            .type = "cnt_00",
                            .module = 242,
                            .status = true,
                            .settlingTime = 0,
                            .readTime = 0,
                            .equation = "none",
                            .userUnits = "counts",
                            .gain =
                                {
                                    .rangingMode = RBRGEN3_RANGING_NONE,
                                    .currentGain = NAN,
                                    .availableGains = {NAN},
                                },
                            .derived = true,
                            .label = "count_00",
                            .calibration =
                                {
                                    .dateTime = 954547200000LL,
                                    .nCount = 1,
                                    .n = {RBRGEN3_VALUE_COEFFICIENT},
                                },
                        },
                    },
            },
        },
        {
            "channels count = 1, on = 1, settlingtime = 5000, "
            "readtime = 10500, minperiod = 10670" RESPONSE_TERMINATOR
            "channel 1 type = fluo10, module = 40, status = on, "
            "settlingtime = 5000, readtime = 10500, equation = lin, "
            "userunits = ug/L, gain = auto, availablegains = 1.0|10.0|100.0, "
            "derived = off, label = chlorophyll_00" RESPONSE_TERMINATOR
            "calibration 1 label = chlorophyll_00, datetime = 20000401000000, "
            "c0 = 678.26611e+000, c1 = -925.73568e+000" RESPONSE_TERMINATOR,
            {
                .count = 1,
                .on = 1,
                .settlingTime = 5000,
                .readTime = 10500,
                .minimumPeriod = 10670,
                .channels =
                    {
                        {
                            .type = "fluo10",
                            .module = 40,
                            .status = true,
                            .settlingTime = 5000,
                            .readTime = 10500,
                            .equation = "lin",
                            .userUnits = "ug/L",
                            .gain =
                                {
                                    .rangingMode = RBRGEN3_RANGING_AUTO,
                                    .currentGain = NAN,
                                    .availableGains = {1.0, 10.0, 100.0, NAN},
                                },
                            .derived = false,
                            .label = "chlorophyll_00",
                            .calibration =
                                {
                                    .dateTime = 954547200000LL,
                                    .cCount = 2,
                                    .c =
                                        {
                                            678.26611e+000,
                                            -925.73568e+000,
                                        },
                                },
                        },
                    },
            },
        },
        {
            "channels count = 1, on = 1, settlingtime = 5000, "
            "readtime = 10500, minperiod = 10670" RESPONSE_TERMINATOR
            "channel 1 type = turb00, module = 40, status = on, "
            "settlingtime = 1000, readtime = 350, equation = lin, "
            "userunits = NTU, gain = 20.0, "
            "availablegains = 1.0|5.0|20.0|100.0, derived = off, "
            "label = turbidity_00" RESPONSE_TERMINATOR
            "calibration 1 label = turbidity_00, datetime = 20000401000000, "
            "c0 = 3.3910000e+003, c1 = -4.6280000e+003" RESPONSE_TERMINATOR,
            {
                .count = 1,
                .on = 1,
                .settlingTime = 5000,
                .readTime = 10500,
                .minimumPeriod = 10670,
                .channels =
                    {
                        {
                            .type = "turb00",
                            .module = 40,
                            .status = true,
                            .settlingTime = 1000,
                            .readTime = 350,
                            .equation = "lin",
                            .userUnits = "NTU",
                            .gain =
                                {
                                    .rangingMode = RBRGEN3_RANGING_MANUAL,
                                    .currentGain = 20.0,
                                    .availableGains = {1.0, 5.0, 20.0, 100.0, NAN},
                                },
                            .derived = false,
                            .label = "turbidity_00",
                            .calibration =
                                {
                                    .dateTime = 954547200000LL,
                                    .cCount = 2,
                                    .c =
                                        {
                                            3.3910000e+003,
                                            -4.6280000e+003,
                                        },
                                },
                        },
                    },
            },
        },
        {0},
    };

    return test_channels(conn, buffers, tests);
}

TEST_LOGGER3(channels_calibration_indices)
{
    ChannelsTest tests[] = {
        {
            "channels count = 1, on = 1, settlingtime = 5000, "
            "readtime = 10500, minperiod = 10670" RESPONSE_TERMINATOR
            "channel 1 type = fluo10, module = 40, status = on, "
            "settlingtime = 5000, readtime = 10500, equation = lin, "
            "userunits = ug/L, gain = none, availablegains = none, "
            "derived = off, label = chlorophyll_00" RESPONSE_TERMINATOR
            "calibration 1 label = chlorophyll_00, datetime = 20000401000000, "
            "c1 = 2.0000000e+000, c0 = 1.0000000e+000, x2 = 3.0000000e+000, "
            "cfoo = 9.0000000e+000, x = 8.0000000e+000, n1 = 4" RESPONSE_TERMINATOR,
            {
                .count = 1,
                .on = 1,
                .settlingTime = 5000,
                .readTime = 10500,
                .minimumPeriod = 10670,
                .channels =
                    {
                        {
                            .type = "fluo10",
                            .module = 40,
                            .status = true,
                            .settlingTime = 5000,
                            .readTime = 10500,
                            .equation = "lin",
                            .userUnits = "ug/L",
                            .gain =
                                {
                                    .rangingMode = RBRGEN3_RANGING_NONE,
                                    .currentGain = NAN,
                                    .availableGains = {NAN},
                                },
                            .derived = false,
                            .label = "chlorophyll_00",
                            .calibration =
                                {
                                    .dateTime = 954547200000LL,
                                    .cCount = 2,
                                    .c = {1.0000000e+000, 2.0000000e+000},
                                    .xCount = 3,
                                    .x = {0.0f, 0.0f, 3.0000000e+000},
                                    .nCount = 2,
                                    .n = {0, 4},
                                },
                        },
                    },
            },
        },
        {0},
    };

    return test_channels(conn, buffers, tests);
}

TEST_LOGGER3(channel_gain_set_auto)
{
    RBRGen3ChannelGain gain = {
        .rangingMode = RBRGEN3_RANGING_AUTO,
    };

    const char *text = "channel 1 gain = auto";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRGen3_setChannelGain(conn, 1, &gain);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(channel_gain_set_manual)
{
    RBRGen3ChannelGain gain = {
        .rangingMode = RBRGEN3_RANGING_MANUAL,
        .currentGain = 5.0,
        .availableGains = {1.0, 5.0, 10.0, NAN},
    };

    const char *text = "channel 1 gain = 5.0";
    char expectedCommand[COMMAND_RESPONSE_SIZE];
    char response[COMMAND_RESPONSE_SIZE];
    rbr_prepareCommandResponse(text, expectedCommand, response);

    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRGen3_setChannelGain(conn, 1, &gain);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(calibration_set)
{
    RBRGen3Calibration calibration = {
        .dateTime = 1537380975000LL,
        .cCount = 4,
        .c =
            {
                3.5000000e-003,
                -250.00002e-006,
                2.7000000e-006,
                23.000000e-009,
            },
    };
    const char *expectedCommand =
        "calibration 1 datetime = 20180919181615, "
        "c0 = 0.0035" COMMAND_TERMINATOR "calibration 1 datetime = 20180919181615, "
        "c1 = -0.00025" COMMAND_TERMINATOR "calibration 1 datetime = 20180919181615, "
        "c2 = 2.7e-06" COMMAND_TERMINATOR "calibration 1 datetime = 20180919181615, "
        "c3 = 2.3e-08" COMMAND_TERMINATOR;

    const char *response =
        "calibration 1 datetime = 20180919181615, "
        "c0 = 0.0035" RESPONSE_TERMINATOR "calibration 1 datetime = 20180919181615, "
        "c1 = -0.00025" RESPONSE_TERMINATOR "calibration 1 datetime = 20180919181615, "
        "c2 = 2.7e-06" RESPONSE_TERMINATOR "calibration 1 datetime = 20180919181615, "
        "c3 = 2.3e-08" RESPONSE_TERMINATOR;

    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRGen3_setCalibration(conn, 1, &calibration);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(calibration_set_x)
{
    RBRGen3Calibration calibration = {
        .dateTime = 1537380975000LL,
        .xCount = 2,
        .x = {1.0000000e+000, -1.0000000e+000},
    };
    const char *expectedCommand =
        "calibration 1 datetime = 20180919181615, "
        "x0 = 1" COMMAND_TERMINATOR "calibration 1 datetime = 20180919181615, "
        "x1 = -1" COMMAND_TERMINATOR;

    const char *response = "calibration 1 datetime = 20180919181615, "
                           "x0 = 1" RESPONSE_TERMINATOR "calibration 1 datetime = 20180919181615, "
                           "x1 = -1" RESPONSE_TERMINATOR;

    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRGen3_setCalibration(conn, 1, &calibration);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(calibration_set_invalid_c_count)
{
    RBRGen3Calibration calibration = {
        .dateTime = 1537380975000LL,
        .cCount = RBRGEN3_CALIBRATION_C_COEFFICIENT_MAX + 1,
    };

    TestIOBuffers_init(buffers, "", 0);
    RBRGen3Error err = RBRGen3_setCalibration(conn, 1, &calibration);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(calibration_set_negative_c_count)
{
    RBRGen3Calibration calibration = {
        .dateTime = 1537380975000LL,
        .cCount = -1,
    };

    TestIOBuffers_init(buffers, "", 0);
    RBRGen3Error err = RBRGen3_setCalibration(conn, 1, &calibration);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(calibration_set_invalid_x_count)
{
    RBRGen3Calibration calibration = {
        .dateTime = 1537380975000LL,
        .xCount = RBRGEN3_CALIBRATION_X_COEFFICIENT_MAX + 1,
    };

    TestIOBuffers_init(buffers, "", 0);
    RBRGen3Error err = RBRGen3_setCalibration(conn, 1, &calibration);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(calibration_set_negative_x_count)
{
    RBRGen3Calibration calibration = {
        .dateTime = 1537380975000LL,
        .xCount = -1,
    };

    TestIOBuffers_init(buffers, "", 0);
    RBRGen3Error err = RBRGen3_setCalibration(conn, 1, &calibration);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(calibration_set_empty)
{
    RBRGen3Calibration calibration = {
        .dateTime = 1537380975000LL,
    };

    TestIOBuffers_init(buffers, "", 0);
    RBRGen3Error err = RBRGen3_setCalibration(conn, 1, &calibration);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(calibration_set_n_only)
{
    RBRGen3Calibration calibration = {
        .dateTime = 1537380975000LL,
        .nCount = 2,
        .n = {3, RBRGEN3_VALUE_COEFFICIENT},
    };

    TestIOBuffers_init(buffers, "", 0);
    RBRGen3Error err = RBRGen3_setCalibration(conn, 1, &calibration);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_INVALID_PARAMETER_VALUE, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ("", buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(settings_fetchpoweroffdelay)
{
    RBRGen3Period fetchPowerOffDelay = 0;

    TestIOBuffers_init(buffers, "settings fetchpoweroffdelay = 8000" RESPONSE_TERMINATOR, 0);
    RBRGen3Error err = RBRGen3_getFetchPowerOffDelay(conn, &fetchPowerOffDelay);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(8000, fetchPowerOffDelay, "%" PRIi32);

    return true;
}

TEST_LOGGER3(settings_fetchpoweroffdelay_set)
{
    RBRGen3Period fetchPowerOffDelay = 8000;
    const char *expectedCommand = "permit command = settings" COMMAND_TERMINATOR
                                  "settings fetchpoweroffdelay = 8000" COMMAND_TERMINATOR;

    const char *response = "permit command = settings" RESPONSE_TERMINATOR
                           "settings fetchpoweroffdelay = 8000" RESPONSE_TERMINATOR;

    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRGen3_setFetchPowerOffDelay(conn, fetchPowerOffDelay);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(settings_sensorpoweralwayson)
{
    bool sensorPowerAlwaysOn = false;

    TestIOBuffers_init(buffers, "settings sensorpoweralwayson = on" RESPONSE_TERMINATOR, 0);
    RBRGen3Error err = RBRGen3_isSensorPowerAlwaysOn(conn, &sensorPowerAlwaysOn);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(true, sensorPowerAlwaysOn, bool);

    return true;
}

TEST_LOGGER3(settings_sensorpoweralwayson_set)
{
    RBRGen3Period sensorPowerAlwaysOn = true;
    const char *expectedCommand = "permit command = settings" COMMAND_TERMINATOR
                                  "settings sensorpoweralwayson = on" COMMAND_TERMINATOR;
    const char *response = "permit command = settings" RESPONSE_TERMINATOR
                           "settings sensorpoweralwayson = on" RESPONSE_TERMINATOR;

    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRGen3_setSensorPowerAlwaysOn(conn, sensorPowerAlwaysOn);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(settings_castdetection)
{
    bool castDetection = false;

    TestIOBuffers_init(buffers, "settings castdetection = on" RESPONSE_TERMINATOR, 0);
    RBRGen3Error err = RBRGen3_getCastDetection(conn, &castDetection);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_ENUM_EQ(true, castDetection, bool);

    return true;
}

TEST_LOGGER3(settings_castdetection_set)
{
    RBRGen3Period castDetection = true;
    const char *expectedCommand = "permit command = settings" COMMAND_TERMINATOR
                                  "settings castdetection = on" COMMAND_TERMINATOR;

    const char *response = "permit command = settings" RESPONSE_TERMINATOR
                           "settings castdetection = on" RESPONSE_TERMINATOR;

    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRGen3_setCastDetection(conn, castDetection);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(settings_inputtimeout)
{
    RBRGen3Period inputTimeout = 0;

    TestIOBuffers_init(buffers, "settings inputtimeout = 10000" RESPONSE_TERMINATOR, 0);
    RBRGen3Error err = RBRGen3_getInputTimeout(conn, &inputTimeout);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_EQ(10000, inputTimeout, "%" PRIi32);

    return true;
}

TEST_LOGGER3(settings_inputtimeout_set)
{
    RBRGen3Period inputTimeout = 15000;
    const char *expectedCommand = "permit command = settings" COMMAND_TERMINATOR
                                  "settings inputtimeout = 15000" COMMAND_TERMINATOR;

    const char *response = "permit command = settings" RESPONSE_TERMINATOR
                           "settings inputtimeout = 15000" RESPONSE_TERMINATOR;

    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRGen3_setInputTimeout(conn, inputTimeout);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(settings_atmosphere)
{
    float atmosphere = 0;

    TestIOBuffers_init(buffers, "settings atmosphere = 10.1325010" RESPONSE_TERMINATOR, 0);
    RBRGen3Error err = RBRGen3_getValueSetting(conn, RBRGEN3_SETTING_ATMOSPHERE, &atmosphere);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_FLOAT_EQ(10.132501f, atmosphere, 0.000001f);

    return true;
}

TEST_LOGGER3(settings_atmosphere_set)
{
    float atmosphere = 10.132501;
    const char *expectedCommand = "permit command = settings" COMMAND_TERMINATOR
                                  "settings atmosphere = 10.132501" COMMAND_TERMINATOR;

    const char *response = "permit command = settings" RESPONSE_TERMINATOR
                           "settings atmosphere = 10.132501" RESPONSE_TERMINATOR;

    TestIOBuffers_init(buffers, response, 0);
    RBRGen3Error err = RBRGen3_setValueSetting(conn, RBRGEN3_SETTING_ATMOSPHERE, atmosphere);
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    TEST_ASSERT_STR_EQ(expectedCommand, buffers->writeBuffer);

    return true;
}

#define TEST_SENSOR_PARAMETER_MAX 3

typedef struct SensorTest {
    const char *response;
    RBRGen3SensorParameter expected[TEST_SENSOR_PARAMETER_MAX];
    int32_t size;
} SensorTest;

static bool test_sensor(RBRGen3 *conn, TestIOBuffers *buffers, SensorTest *tests)
{
    RBRGen3Error err;
    RBRGen3SensorParameter actual;

    for (int i = 0; tests[i].response != NULL; ++i) {
        snprintf(actual.key, sizeof(actual.key), "%s", tests[i].expected[0].key);
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRGen3_getSensorParameter(conn, 1, &actual);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_STR_EQ(tests[i].expected[0].key, actual.key);
        TEST_ASSERT_STR_EQ(tests[i].expected[0].value, actual.value);
    }

    return true;
}

static bool test_sensors(RBRGen3 *conn, TestIOBuffers *buffers, SensorTest *tests)
{
    RBRGen3Error err;
    RBRGen3SensorParameter actual[TEST_SENSOR_PARAMETER_MAX];

    for (int i = 0; tests[i].response != NULL; ++i) {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        int32_t size = TEST_SENSOR_PARAMETER_MAX;
        err = RBRGen3_getSensorParameters(conn, 1, &actual[0], &size);
        TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
        TEST_ASSERT_EQ(tests[i].size, size, "%" PRIi32);

        for (int parameter = 0; parameter < size; ++parameter) {
            TEST_ASSERT_STR_EQ(tests[i].expected[parameter].key, actual[parameter].key);
            TEST_ASSERT_STR_EQ(tests[i].expected[parameter].value, actual[parameter].value);
        }
    }

    return true;
}

TEST_LOGGER2(sensor)
{
    SensorTest tests[] = {
        {
            .response = "sensor 1 serial = 12345" RESPONSE_TERMINATOR,
            .expected =
                {
                    {
                        .key = "serial",
                        .value = "12345",
                    },
                },
            .size = 0,
        },
        {
            .response = "E0501 item is not configured" RESPONSE_TERMINATOR,
            .expected =
                {
                    {
                        .key = "serial",
                        .value = "n/a",
                    },
                },
            .size = 0,
        },
        {0},
    };

    return test_sensor(conn, buffers, tests);
}

TEST_LOGGER2(sensor_all)
{
    SensorTest tests[] = {
        {
            .response = "sensor 1 serial = 12345" RESPONSE_TERMINATOR,
            .expected =
                {
                    {
                        .key = "serial",
                        .value = "12345",
                    },
                },
            .size = 1,
        },
        {
            .response = "sensor 1 serial = 12345, manufacturer = Whoever, "
                        "foo = bar" RESPONSE_TERMINATOR,
            .expected =
                {
                    {
                        .key = "serial",
                        .value = "12345",
                    },
                    {
                        .key = "manufacturer",
                        .value = "Whoever",
                    },
                    {
                        .key = "foo",
                        .value = "bar",
                    },
                },
            .size = 3,
        },
        {
            .response = "E0109 feature not available" RESPONSE_TERMINATOR,
            .size = 0,
        },
        {0},
    };

    return test_sensors(conn, buffers, tests);
}

TEST_LOGGER3(sensor)
{
    SensorTest tests[] = {
        {
            .response = "sensor 1 serial = 12345" RESPONSE_TERMINATOR,
            .expected =
                {
                    {
                        .key = "serial",
                        .value = "12345",
                    },
                },
            .size = 0,
        },
        {
            .response = "sensor 1 serial = n/a" RESPONSE_TERMINATOR,
            .expected =
                {
                    {
                        .key = "serial",
                        .value = "n/a",
                    },
                },
            .size = 0,
        },
        {0},
    };

    return test_sensor(conn, buffers, tests);
}

TEST_LOGGER3(sensor_all)
{
    SensorTest tests[] = {
        {
            .response = "sensor 1 serial = 12345" RESPONSE_TERMINATOR,
            .expected =
                {
                    {
                        .key = "serial",
                        .value = "12345",
                    },
                },
            .size = 1,
        },
        {
            .response = "sensor 1 serial = 12345, manufacturer = Whoever, "
                        "foo = bar" RESPONSE_TERMINATOR,
            .expected =
                {
                    {
                        .key = "serial",
                        .value = "12345",
                    },
                    {
                        .key = "manufacturer",
                        .value = "Whoever",
                    },
                    {
                        .key = "foo",
                        .value = "bar",
                    },
                },
            .size = 3,
        },
        {
            .response = "sensor 1 serial = 12345, manufacturer = Whoever, "
                        "foo = bar, baz = lem" RESPONSE_TERMINATOR,
            .expected =
                {
                    {
                        .key = "serial",
                        .value = "12345",
                    },
                    {
                        .key = "manufacturer",
                        .value = "Whoever",
                    },
                    {
                        .key = "foo",
                        .value = "bar",
                    },
                },
            .size = 3,
        },
        {
            .response = "sensor 1" RESPONSE_TERMINATOR,
            .size = 0,
        },
        {0},
    };

    return test_sensors(conn, buffers, tests);
}
