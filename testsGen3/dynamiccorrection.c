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
#include <stdlib.h>
#include <math.h>


#define TEST_DYNCORR_DATASET_SIZE    7

/* declaration of private functions */
float RBRDynamicCorrection_PSS78(float C, float T, float P);

float RBRDynamicCorrection_calcAscentRate(RBRDynamicCorrectionParams *params, int64_t timestamp, float pressure);

void RBRDynamicCorrection_updateVariables(RBRDynamicCorrectionParams *params, float Vp);


typedef struct Pss78Test
{
    float C_value;
    float T_value;
    float P_value;
    float S_expected;
} Pss78Test;

typedef struct DCorrCoeffTest
{
    float Vp;
    float alpha_expected;
    float tau_expected;
    float ctcoeff_expected;
} DCorrCoeffTest;



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

static bool test_verify_ascent_rate(void)
{
    RBRDynamicCorrectionParams params;
    float pressure;
    int64_t t;
    float Vp;
    float noise;

    /* initialiaze only the field needed for the test */
    params.Fs = 1.0f;
    params.Vp_fc = 1/100.0f;
    params._lastPressure = -1.0f;
    params._lastPressureTime = -1.0f;
    params._ascentRate = (float)(0.0f/0.0f); // NAN macro may be not exist

    int64_t step = (int64_t)llroundf(params.Fs *1000);
    
    /* seed for random number, allow repeatable test*/
    srand(1234);

    for ( t = 0; t < 600000; t += step )
    {
        /* the C rand() isn't really good random source,
         * (but for a unit test, it will be okay) */
        noise = 0.1f*(rand() / (float)RAND_MAX) - 0.1f;
        if ( t < 60000 )
        {
            pressure = 3000.0f - 0.05f*(t/1000.f) + noise;
        }
        else if ( t > 120000 && t < 133000 )
        {
            pressure = (float)(0.0f/0.0f);  // force a sequence of NAN
        }
        else if ( t == 700000 )
        {
            pressure = (float)(0.0f/0.0f);  // force a single NAN
        }
        else
        {
            pressure = (3000.0f - 3.0f) - 0.1f*((t-60000)/1000.0f) + noise;
        }
        
        Vp = RBRDynamicCorrection_calcAscentRate(&params, t + 9000000, pressure);

        /* since the filter is dynamic, it is difficult to make
        * a condition for each time step.  In this case, just wait until
        * the filter is stable (600s, filter bandwidth is 100s) */
        if ( t >= 300000 )
        {
            TEST_ASSERT_FLOAT_EQ(Vp, 0.1f, 1e-2f);
        }

        // print for manual check
        //printf("%.3f, %.4f, %.4f\n", t, pressure, Vp);
    }

    return true;
}

static bool test_verify_coeff_alpha_tau_ctcoeff(DCorrCoeffTest *tests)
{
    RBRDynamicCorrectionParams params;
    RBRDynamicCorrectionError status;
    float Vp;

    /* using Fs = 1.0f.  This parameter is not affecting the test result */
    status = RBRDynamicCorrection_init(&params, 1.0f, DCORR_T_DELAY, 
                                    DCORR_ALPHA_A, DCORR_ALPHA_E,
                                    DCORR_TAU_A, DCORR_TAU_E,
                                    DCORR_CT_COEFF_A, DCORR_CT_COEFF_E,
                                    DCORR_VP_MIN, DCORR_VP_MAX, DCORR_VP_FC);
    
    TEST_ASSERT(status == RBR_DCORR_SUCCESS);

    for (int i = 0; tests[i].Vp > 0.0f; i++)
    {
        Vp = tests[i].Vp;

        RBRDynamicCorrection_updateVariables(&params, Vp);

        // the data is fitted.  Check we are within 5% of value
        TEST_ASSERT_FLOAT_EQ(tests[i].alpha_expected, params.alpha, params.alpha * 5e-2f);
        TEST_ASSERT_FLOAT_EQ(tests[i].tau_expected, params.tau, params.tau * 5e-2f);
        TEST_ASSERT_FLOAT_EQ(tests[i].ctcoeff_expected, params.CT_coeff, params.CT_coeff * 5e-2f);
    }

    return true;
}

/* run the dynamic correction test */
static bool test_dynamic_correction(float *dataset, float Fs)
{
    RBRDynamicCorrectionError status;
    RBRDynamicCorrectionParams params;
    RBRDynamicCorrectionMeasurement measIn;
    RBRDynamicCorrectionResult corrResult;
    float *datasetPtr = dataset;
    float *resultPtr = dataset;
    float target_Tcor, target_Scor;

    status = RBRDynamicCorrection_init(&params, Fs, DCORR_T_DELAY, 
                                    DCORR_ALPHA_A, DCORR_ALPHA_E,
                                    DCORR_TAU_A, DCORR_TAU_E,
                                    DCORR_CT_COEFF_A, DCORR_CT_COEFF_E,
                                    DCORR_VP_MIN, DCORR_VP_MAX, DCORR_VP_FC);

    TEST_ASSERT_EQ(RBR_DCORR_SUCCESS, status, "%d");

    while ( datasetPtr[0] >= 0.0f )
    {
        measIn.timestamp = (int64_t)llroundf(datasetPtr[0]*1000.0f); //time in millisecond
        measIn.conductivity = datasetPtr[1];
        measIn.marineTemperature = datasetPtr[2];
        measIn.pressure = datasetPtr[3];
        measIn.condTemperature = datasetPtr[4];
        
        status = RBRDynamicCorrection_addMeasurement(&params, &measIn, &corrResult);

        if ( status != RBR_DCORR_NOT_VALID_YET )
        {
            /* check the result */
            target_Tcor = resultPtr[5];
            target_Scor = resultPtr[6];
            resultPtr += TEST_DYNCORR_DATASET_SIZE;

            TEST_ASSERT_FLOAT_EQ(corrResult.corrTemperature, target_Tcor, 0.001f);
            TEST_ASSERT_FLOAT_EQ(corrResult.corrSalinity, target_Scor, 0.01f);
        }

        datasetPtr += TEST_DYNCORR_DATASET_SIZE;
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

TEST_LOGGER3(verify_ascent_rate)
{
    return test_verify_ascent_rate();
}

TEST_LOGGER3(verify_coeff_alpha_tau_ctcoeff)
{
    // https://wiki.rbr-global.com/display/RAD/Dynamic+processing+on+floats
    DCorrCoeffTest tests[] = {
            { 0.02f, 0.120f, 12.26f, 0.046f },
            { 0.03f, 0.120f, 12.26f, 0.046f },
            { 0.05f, 0.071f, 10.54f, 0.0280f },
            { 0.06f, 0.058f, 10.05f, 0.0233f },
            { 0.07f, 0.050f, 9.66f, 0.0200f },
            { 0.08f, 0.043f, 9.33f, 0.0175f },
            { 0.09f, 0.038f, 9.05f, 0.0156f },
            { 0.10f, 0.035f, 8.80f, 0.0140f },
            { 0.11f, 0.031f, 8.59f, 0.0127f },
            { 0.12f, 0.029f, 8.40f, 0.0117f },
            { 0.13f, 0.026f, 8.22f, 0.0108f },
            { 0.14f, 0.024f, 8.07f, 0.0100f },
            { 0.15f, 0.023f, 7.92f, 0.0093f },
            { 0.20f, 0.0169f, 7.492f, 0.0069f },
            { 0.45f, 0.0074f, 6.07f, 0.0031f },
            { 0.50f, 0.0074f, 6.07f, 0.0031f },
            { 0.0f, 0.0f, 0.f, 0.0f}    // end-of-test
    };

    return test_verify_coeff_alpha_tau_ctcoeff(tests);
}

TEST_LOGGER3(verify_dynamic_correction)
{
    /* Expect timestamp,C,T,P,Tcond,Tcor,Scor */
    float  dataset[][TEST_DYNCORR_DATASET_SIZE] = {
        { 0.00f, 37.43682000f, 8.64478000f, 1004.25549000f, 8.58359000f, 8.64499349994183f, 35.1910764589035f },
        { 0.25f, 37.43714750f, 8.64493250f, 1004.22552250f, 8.54564000f, 8.64514599994183f, 35.1901894819513f },
        { 0.50f, 37.43747500f, 8.64508500f, 1004.19555500f, 8.50769000f, 8.64529849994183f, 35.1906750470648f },
        { 0.75f, 37.43780250f, 8.64523750f, 1004.16558750f, 8.46974000f, 8.64545099994183f, 35.1912996516834f },
        { 1.00f, 37.43813000f, 8.64539000f, 1004.13562000f, 8.43179000f, 8.64560349994183f, 35.1919608017530f },
        { 1.25f, 37.43807750f, 8.64554250f, 1004.10696500f, 8.43179000f, 8.64575599994183f, 35.1917654658364f },
        { 1.50f, 37.43802500f, 8.64569500f, 1004.07831000f, 8.43179000f, 8.64590849994183f, 35.1915613188093f },
        { 1.75f, 37.43797250f, 8.64584750f, 1004.04965500f, 8.43179000f, 8.64609099991322f, 35.1913040114158f },
        { 2.00f, 37.43792000f, 8.64600000f, 1004.02100000f, 8.43179000f, 8.64631849991322f, 35.1910111691001f },
        { 2.25f, 37.43836500f, 8.64622750f, 1003.99469250f, 8.44129000f, 8.64654599991322f, 35.1911192731392f },
        { 2.50f, 37.43881000f, 8.64645500f, 1003.96838500f, 8.45079000f, 8.64677349991322f, 35.1912413015959f },
        { 2.75f, 37.43925500f, 8.64668250f, 1003.94207750f, 8.46029000f, 8.64701999989510f, 35.1913624108901f },
        { 3.00f, 37.43970000f, 8.64691000f, 1003.91577000f, 8.46979000f, 8.64729499989510f, 35.1914659279734f },
        { 3.25f, 37.44009250f, 8.64718500f, 1003.88671750f, 8.45079250f, 8.64756999989510f, 35.1919024340491f },
        { 3.50f, 37.44048500f, 8.64746000f, 1003.85766500f, 8.43179500f, 8.64784499989510f, 35.1923255693562f },
        { 3.75f, 37.44087750f, 8.64773500f, 1003.82861250f, 8.41279750f, 8.64815699985981f, 35.1926670326019f },
        { -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }
    };

    return test_dynamic_correction(dataset[0], 4.0f);
}
