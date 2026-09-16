/**
 * \file RBRGen4DynamicCorrection.c
 *
 * \brief Library for salinity dynamic correction
 *
 * \copyright
 * Copyright (c) 2021 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "../include/RBRGen4DynamicCorrection.h"


/* need to using C99 standard to get NAN and isnan().
 * (some alternative definition otherwise) */
#if __STDC_VERSION__ >= 199901L
#define ISNAN(x) isnan(x)
#else
#define NAN      (float)(0.0f/0.0f)
#define ISNAN(x) (x != x)
#endif

#ifndef M_PI
#define M_PI 3.14159265f
#endif


/* parameters for PSS-78 conversion */
#define	PSS78_A0 0.0080f
#define	PSS78_A1 -0.1692f
#define	PSS78_A2 25.3851f
#define	PSS78_A3 14.0941f
#define	PSS78_A4 -7.0261f
#define	PSS78_A5 2.7081f
#define	PSS78_B0 0.0005f
#define	PSS78_B1 -0.0056f
#define	PSS78_B2 -0.0066f
#define	PSS78_B3 -0.0375f
#define	PSS78_B4 0.0636f
#define	PSS78_B5 -0.0144f
#define	PSS78_K  0.0162f
#define	PSS78_C0 0.6766097f
#define	PSS78_C1 2.00564e-2f
#define	PSS78_C2 1.104259e-4f
#define	PSS78_C3 -6.9698e-7f
#define	PSS78_C4 1.0031e-9f
#define	PSS78_D1 3.426e-2f
#define	PSS78_D2 4.464e-4f
#define	PSS78_D3 0.4215f
#define	PSS78_D4 -3.107e-3f
#define	PSS78_E1 2.070e-4f
#define	PSS78_E2 -6.370e-8f
#define	PSS78_E3 3.989e-12f
#define PSS78_C_REF 42.914f
#define PSS78_T_REF 15.0f


/**
 * \brief Internal: Apply the practical salinity 1978 (PSS-78) equation from C,T,P input.
 *
 * \param [in] C Conductivity
 * \param [in] T Temperature
 * \param [in] P Sea Pressure
 * \return Salinity
 */
float RBRGen4DynamicCorrection_PSS78(float C, float T, float P)
{
    float pressure;
    float T_its68;
    float Rp_num, Rp_den;
    float R, Rp, rT;
    float RT, RT_sqrt;
    float S_1, S_2;
    float S;

    // hydrostatic pressure(i.e. sea Pressure) in bars
    pressure=P*0.1f;

    // temperature in ITS68...
    T_its68=T*1.00024f;

    // convert conductivity to a ratio
    R= C/PSS78_C_REF;

    // rT & Rp
    //rT = PSS78_C0 + PSS78_C1*T_its68 + PSS78_C2*(T_its68*T_its68) + PSS78_C3*T_its68*(T_its68*T_its68) + 
    //                        PSS78_C4*(T_its68*T_its68)*(T_its68*T_its68);
    rT = PSS78_C3 + PSS78_C4*T_its68;
    rT = PSS78_C2 + rT*T_its68;
    rT = PSS78_C1 + rT*T_its68;
    rT = PSS78_C0 + rT*T_its68;

    //Rp_num = PSS78_E1*seaPressure + PSS78_E2*(seaPressure*seaPressure) + PSS78_E3*seaPressure*(seaPressure*seaPressure);
    Rp_num = PSS78_E2 + PSS78_E3*pressure;
    Rp_num = (PSS78_E1 + Rp_num*pressure)*pressure;

    Rp_den = 1.0f + PSS78_D1*T_its68 + PSS78_D2*(T_its68*T_its68) + PSS78_D3*R + PSS78_D4*(T_its68*R);
    Rp = 1.0f + Rp_num/Rp_den;

    // R_T
    RT = R/(Rp*rT);

    // sqrt(RT)
    RT_sqrt = sqrtf(RT);
    if ( ISNAN(RT_sqrt) )
    {
        /* comply with RBR logger conversion
         * when salinity value is undefined */
        return 0.0f;
    }

    S_1 = PSS78_A4 + PSS78_A5*RT_sqrt;
    S_1 = PSS78_A3 + S_1*RT_sqrt;
    S_1 = PSS78_A2 + S_1*RT_sqrt;
    S_1 = PSS78_A1 + S_1*RT_sqrt;
    S_1 = PSS78_A0 + S_1*RT_sqrt;

    S_2 = PSS78_B4 + PSS78_B5*RT_sqrt;
    S_2 = PSS78_B3 + S_2*RT_sqrt;
    S_2 = PSS78_B2 + S_2*RT_sqrt;
    S_2 = PSS78_B1 + S_2*RT_sqrt;
    S_2 = PSS78_B0 + S_2*RT_sqrt;

    S = S_1 + ((T_its68 - PSS78_T_REF)/(1.0f + PSS78_K*(T_its68 - PSS78_T_REF)))*S_2;

    return S;
}


/* Precalculate some factors/index used for temperature interpolation */
int RBRGen4DynamicCorrection_initCorrectionCoeff(RBRGen4DynamicCorrectionParams *params, float Fs)
{
    params->_lagIndex = (int)(Fs * params->t_delay);
    params->_phi = (params->t_delay - params->_lagIndex/Fs)*Fs;

    /* sanity check (parameter for Fs should have been checked before call) */
    if ( params->_lagIndex < 0 || params->_lagIndex >= DCORR_MAX_LAG_ARRAY )
    {
        return -1;
    }
    if ( params->_phi < 0.0f || params->_phi > 1.0f )
    {
        return -1;
    }

    return 0;
}

/* initial arrays */
void RBRGen4DynamicCorrection_initLagArray(RBRGen4DynamicCorrectionParams *params)
{
    int k;

    /* fill with invalid entries (to catch errors) */
    for (k = 0; k < DCORR_MAX_LAG_ARRAY; k++)
    {
        params->_isValid_lagArray[k] = 0;
        params->_timestamp_lagArray[k] = -999.9f;
        params->_C_meas_lagArray[k] = -999.9f;
        params->_P_meas_lagArray[k] = -999.9f;
        params->_T_cond_lagArray[k] = -999.9f;
    }
}

/* apply temperature interpolation */
float RBRGen4DynamicCorrection_applyTempCorr(RBRGen4DynamicCorrectionParams *params, float T_meas)
{
    float T_cor;

    /* (first evaluation will be incorrect, but won't be used) */
    T_cor = (float) ((1.0 - (double) params->_phi)*(double) params->_T_meas_lag
                     + ((double) params->_phi)*(double) T_meas);

    return T_cor;
}

/* sanity check on data */
int32_t RBRGen4DynamicCorrection_checkData(RBRGen4DynamicCorrectionMeasurement * measIn)
{
    int32_t isError = 0;

    if ( ISNAN(measIn->conductivity) )
    {
        isError = -1;
    }
    if ( ISNAN(measIn->pressure) )
    {
        isError = -1;
    }
    if ( ISNAN(measIn->condTemperature) )
    {
        isError = -1;
    }

    return isError;
}

/* resample all lagged variables using new sampling rate */
void RBRGen4DynamicCorrection_resampleLag(RBRGen4DynamicCorrectionParams *params, float timestamp, float Fs)
{
    float timestamp_array[DCORR_MAX_LAG_ARRAY];
    float C_meas_array[DCORR_MAX_LAG_ARRAY];
    float P_meas_array[DCORR_MAX_LAG_ARRAY];
    float T_cond_array[DCORR_MAX_LAG_ARRAY];
    float factor;
    float t1, t2;
    float dt = 1.0f/Fs;
    int k;
    int j;
    int timeout = 0;

    /* check entries (current/next got timestamp, interp) */
    j = 0;

    for (k = 0; k < DCORR_MAX_LAG_ARRAY-1; k++)
    {
        timeout = 0;
        while (params->_timestamp_lagArray[k] < timestamp)
        {
            timestamp -= dt;
            timeout++;
            if ( timeout > 30 )
            {
                return;
            }
        }

        t1 = params->_timestamp_lagArray[k+1];
        t2 = params->_timestamp_lagArray[k];
        
        if ( timestamp > t1 && timestamp <= t2 )
        {
            factor = (timestamp - t2) / (t2 - t1);

            timestamp_array[j] = timestamp;
            C_meas_array[j] = params->_C_meas_lagArray[k] + factor*(params->_C_meas_lagArray[k+1] - params->_C_meas_lagArray[k]);
            P_meas_array[j] = params->_P_meas_lagArray[k] + factor*(params->_P_meas_lagArray[k+1] - params->_P_meas_lagArray[k]);
            T_cond_array[j] = params->_T_cond_lagArray[k] + factor*(params->_T_cond_lagArray[k+1] - params->_T_cond_lagArray[k]);   
            timestamp -= dt;
            j++;
        }
    }

    memcpy(params->_timestamp_lagArray, timestamp_array, DCORR_MAX_LAG_ARRAY*sizeof(float));
    memcpy(params->_C_meas_lagArray, C_meas_array, DCORR_MAX_LAG_ARRAY*sizeof(float));
    memcpy(params->_P_meas_lagArray, P_meas_array, DCORR_MAX_LAG_ARRAY*sizeof(float));
    memcpy(params->_T_cond_lagArray, T_cond_array, DCORR_MAX_LAG_ARRAY*sizeof(float));
}

/* update all lagged variables */
int32_t RBRGen4DynamicCorrection_updateLag(RBRGen4DynamicCorrectionParams *params, const RBRGen4DynamicCorrectionMeasurement * measIn, RBRGen4DynamicCorrectionMeasurement * meas_out)
{
    int32_t lagIndex;
    int32_t isValid;
    float T_meas;
    int k;

    /* a simple shortcut */
    lagIndex = params->_lagIndex;

    T_meas = measIn->marineTemperature;
    params->_T_meas_lag = T_meas;

    isValid = params->_isValid_lagArray[lagIndex];
    meas_out->timestamp = params->_timestamp_lagArray[lagIndex];
    meas_out->conductivity = params->_C_meas_lagArray[lagIndex];
    meas_out->pressure = params->_P_meas_lagArray[lagIndex];
    meas_out->condTemperature = params->_T_cond_lagArray[lagIndex];
    
    /* Move all the values (even above lagIndex) */
    for (k = DCORR_MAX_LAG_ARRAY-1; k > 0; k--)
    {
        params->_isValid_lagArray[k] = params->_isValid_lagArray[k-1];
        params->_timestamp_lagArray[k] = params->_timestamp_lagArray[k-1];
        params->_C_meas_lagArray[k] = params->_C_meas_lagArray[k-1];
        params->_P_meas_lagArray[k] = params->_P_meas_lagArray[k-1];
        params->_T_cond_lagArray[k] = params->_T_cond_lagArray[k-1];
    }

    params->_isValid_lagArray[0] = 1;
    params->_timestamp_lagArray[0] = measIn->timestamp;
    params->_C_meas_lagArray[0] = measIn->conductivity;
    params->_P_meas_lagArray[0] = measIn->pressure;
    params->_T_cond_lagArray[0] = measIn->condTemperature;

    return isValid;
}

RBRGen4DynamicCorrectionError RBRGen4DynamicCorrection_update_Fs(const float Fs, RBRGen4DynamicCorrectionParams *params)
{
    /* sanity check */
    if ( DCORR_MAX_LAG_ARRAY/Fs < DCORR_T_DELAY )
    {
        return RBRGEN4DYNAMICCORRECTION_INVALID_SAMPLING_RATE;
    }

    // parameters _cte_a and _cte_b no longer valid.
    // But they will be updated on next call to _addMeasurement().
    params->Fs = Fs;

    // not enough info to update, keep unchanged
    params->_T_short_lag = params->_T_short_lag;

    RBRGen4DynamicCorrection_initCorrectionCoeff(params, Fs);
    RBRGen4DynamicCorrection_resampleLag(params, params->_timestamp_lagArray[0], Fs);

    return RBRGEN4DYNAMICCORRECTION_SUCCESS;
}

/* calculate the ascent rate (in our case, using the pressure as unit).
 * return Vp (positive for ascent, negative for descent)*/
float RBRGen4DynamicCorrection_calcAscentRate(RBRGen4DynamicCorrectionParams *params, float timestamp, float pressure)
{
    float Vp;
    float a;
    float deltaT;

    a = 1.0f - expf(-2.0f*M_PI*params->Vp_fc / params->Fs);

    if ( !ISNAN(params->_ascentRate) )
    {
        Vp = params->_ascentRate;
    }
    else
    {
        Vp = 0.0f;  
    }

    if ( !ISNAN(pressure) )
    {
        /* NOTE: _lastPressureTime is initialiazed to negative value.
         * We cannot calculate ascent rate until we got two samples */
        if ( params->_lastPressureTime > 0 )
        {
            deltaT = timestamp - params->_lastPressureTime;
            if ( deltaT > 1.0f / params->Fs )
            {
                a = 1.0f - expf(-2.0f*M_PI*params->Vp_fc * deltaT);
                Vp = (1.0f - a)*Vp + a*((params->_lastPressure - pressure)/deltaT);
            }
            else
            {
                Vp = (1.0f - a)*Vp + a*((params->_lastPressure - pressure)*params->Fs);
            }
        }

        // update the pressure for next step
        params->_lastPressure = pressure;
        params->_lastPressureTime = timestamp;
        params->_ascentRate = Vp;
    }

    /* we want ascent rate (positive for ascent) */
    return Vp;
}

void RBRGen4DynamicCorrection_updateVariables(RBRGen4DynamicCorrectionParams *params, float Vp)
{
    float F_nyquist;
    float alpha, tau;

    /* for evaluation of 'alpha', 'tau' and 'CT_coeff',
     * the value for 'Vp' need to clamp between 'min' and 'max' */
    if ( Vp < params->Vp_min )
    {
        Vp = params->Vp_min;
    }
    if ( Vp > params->Vp_max )
    {
        Vp = params->Vp_max;
    }

    /* new evaluation */
    alpha = params->alpha_a * powf(Vp, params->alpha_e);
    tau = params->tau_a * powf(Vp, params->tau_e);
    params->alpha = alpha;
    params->tau = tau;
    params->CT_coeff = params->ctcoeff_a * powf(Vp, params->ctcoeff_e);

    F_nyquist = params->Fs / 2.0f;
    params->_cte_a = (float) ((double) (4.0f*F_nyquist * alpha * tau)
                              / (1.0 + (double) (4.0f*F_nyquist * tau)));
    params->_cte_b = 1.0f -  2.0f * params->_cte_a / alpha;
}

void RBRGen4DynamicCorrection_updatePressure(RBRGen4DynamicCorrectionParams *params, float timestamp, float pressure)
{
    float Vp;

    Vp = RBRGen4DynamicCorrection_calcAscentRate(params, timestamp, pressure);

    RBRGen4DynamicCorrection_updateVariables(params, Vp);
}

RBRGen4DynamicCorrectionError RBRGen4DynamicCorrection_init(const float Fs,
                                            const float t_delay, const float alpha_a, const float alpha_e, 
                                            const float tau_a, const float tau_e, const float ctcoeff_a, const float ctcoeff_e, 
                                            const float Vp_min, const float Vp_max, const float Vp_fc,
                                            RBRGen4DynamicCorrectionParams *params)
{
    /* sanity check */
    if ( DCORR_MAX_LAG_ARRAY/Fs < DCORR_T_DELAY )
    {
        return RBRGEN4DYNAMICCORRECTION_BAD_PARAMS;
    }
    if ( Vp_min < 0.0f || Vp_max < 0.0f )
    {
        return RBRGEN4DYNAMICCORRECTION_BAD_PARAMS;
    }
    if ( Vp_max < Vp_min )
    {
        return RBRGEN4DYNAMICCORRECTION_BAD_PARAMS;
    }

    params->Fs = Fs;
    params->t_delay = t_delay;
    params->alpha_a = alpha_a;
    params->alpha_e = alpha_e;
    params->tau_a = tau_a;
    params->tau_e = tau_e;
    params->ctcoeff_a = ctcoeff_a;
    params->ctcoeff_e = ctcoeff_e;
    params->Vp_min = Vp_min;
    params->Vp_max = Vp_max;
    params->Vp_fc = Vp_fc;

    params->_firstCall = 1;
    params->_T_meas_lag = -999.0f;
    params->_T_cor_lag = 0.0f;
    params->_T_short_lag = 0.0f;

    params->_ascentRate = -1.0f;
    params->_lastPressure = -1.0f;
    params->_lastPressureTime = -1.0f;

    /* we don't want uninitialiazed values, but we don't know
     * the ascent rate yet.  Let just use the mid-point for now,
     * this will get re-evaluated in _addMeasurement() anyway */
    RBRGen4DynamicCorrection_updateVariables(params, 0.5f*(Vp_min + Vp_max));
    
    RBRGen4DynamicCorrection_initCorrectionCoeff(params, Fs);
    RBRGen4DynamicCorrection_initLagArray(params);

    return RBRGEN4DYNAMICCORRECTION_SUCCESS;
}


RBRGen4DynamicCorrectionError RBRGen4DynamicCorrection_addMeasurement(const RBRGen4DynamicCorrectionMeasurement * measIn, RBRGen4DynamicCorrectionParams *params, RBRGen4DynamicCorrectionResult * corrMeasOut)
{
    RBRGen4DynamicCorrectionMeasurement measLagged;
    float T_cor, T_cell;
    float T_short, T_long;
    float timestamp;
    float C_meas, T_meas, P_meas, T_cond;
    float S_cor;
    int32_t isFasterSampling;
    int32_t isValid;
    int32_t isDataError = 0;
    RBRGen4DynamicCorrectionError statusCode = RBRGEN4DYNAMICCORRECTION_UNKNOWN_ERROR;

    /* flag to indicate 'fast sampling (>= 1Hz)'.
     * In this case, the data won't go through the 0.35s lag and
     * it will bypass the short-term thermal correction */
    isFasterSampling = (params->Fs >= 1.0f) ? 1 : 0;

    T_meas = measIn->marineTemperature;

    /* initial call */
    if ( params->_firstCall && ( ISNAN(measIn->marineTemperature) == 0) )
    {
        params->_T_meas_lag = measIn->marineTemperature;
        params->_T_cor_lag = measIn->marineTemperature;
        params->_firstCall = 0;
    }

    // Interpolate the temperature with time offset 't_delay'.
    // (only for >= 1Hz data rate)
    if ( isFasterSampling )
    {
        T_cor = RBRGen4DynamicCorrection_applyTempCorr(params, T_meas);
    }
    else
    {
        T_cor = T_meas;
    }

    /* update coefficient alpha, tau and ctcoeff based on ascent rate 
     * (use instantaneous value, the filter is several second and data only
     * got a lag delay of 0.35s.  Since the first pass of the filter need to
     * be initialiazed anyway, this will do the trick since the data will need to feed
     * multiple time before reaching the 0.35s mark */
    RBRGen4DynamicCorrection_updatePressure(params, measIn->timestamp, measIn->pressure);

    /* the variables need to be delayed until all samples are available for calculation
     * (only when rate >= 1Hz) */
    if ( isFasterSampling )
    {
        isValid = RBRGen4DynamicCorrection_updateLag(params, measIn, &measLagged);
    }
    else
    {
        /* we still need to update the array (in case sampling rate change) */
        RBRGen4DynamicCorrection_updateLag(params, measIn, &measLagged);
        
        /* ignore the lag, just copy the data to the variable */
        isValid = 1;
        memcpy(&measLagged, measIn, sizeof(RBRGen4DynamicCorrectionMeasurement));
    }

    if ( !isValid )
    {
        statusCode = RBRGEN4DYNAMICCORRECTION_NOT_VALID_YET;
    }

    if ( isValid )
    {
        /* check input */
        isDataError = RBRGen4DynamicCorrection_checkData(&measLagged);

        /* if T_cor ever become NAN, it will not recover
         * (all other variables will eventually cleared once correct data is available) */
        if ( ISNAN(T_cor) )
        {
            isDataError = -1;
            T_cor = params->_T_cor_lag;
        }

        /* shortcut */
        timestamp = measLagged.timestamp;
        C_meas = measLagged.conductivity;
        P_meas = measLagged.pressure;
        T_cond = measLagged.condTemperature;

        /* long-term thermal mass adjustment */
        T_long = params->CT_coeff * (T_cond - T_cor);

        /* apply the short-term thermal mass adjustment (but only when rate >= 1Hz) */
        if ( isFasterSampling )
        {
            T_short = -params->_cte_b*params->_T_short_lag + params->_cte_a*(T_cor - params->_T_cor_lag);
        }
        else
        {
            /* do a no-op */
            T_short = 0;
        }

        T_cell = T_cor + T_long - T_short;

        /* calculate salinity based on corrected values */
        S_cor = RBRGen4DynamicCorrection_PSS78(C_meas, T_cell, P_meas);

        /* update lagged variables */
        params->_T_short_lag = T_short;
        params->_T_cor_lag = T_cor;

        /* assign output */
        corrMeasOut->timestamp = timestamp;
        corrMeasOut->conductivity = C_meas;
        corrMeasOut->corrTemperature = T_cor;
        corrMeasOut->pressure = P_meas;
        corrMeasOut->corrSalinity = S_cor;

        statusCode = RBRGEN4DYNAMICCORRECTION_SUCCESS;
    }

    /* flag the data with potential issue */
    if ( isDataError )
    {
        statusCode = RBRGEN4DYNAMICCORRECTION_CORRUPTED;
    }

    return statusCode;
}

