/**
 * \file RBRDynamicCorrection.c
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

#include "RBRDynCorrection.h"
#include "RBRDynCorrectionPSS78.h"


/* need to using C99 standard to get NAN and isnan().
 * (some alternative definition otherwise) */
#if __STDC_VERSION__ >= 199901L
#define ISNAN(x) isnan(x)
#else
#define NAN      (float)(0.0f/0.0f)
#define ISNAN(x) (x != x)
#endif




/* Precalculate some factors/index used for temperature interpolation */
int RBRDynCorr_initCorrectionCoeff(RBRDynCorrParams *params, float Fs)
{
    params->_lagIndex = (int)(Fs * params->t_delay);
    params->_phi = (params->t_delay - params->_lagIndex/Fs)*Fs;

    DEBUG_LOG("RBRDynCorr_initCorrectionCoeff(): Fs= %.3f period= %.3f t_delay= %.3f index= %d, phi= %.8f\n", \
                Fs, 1.0/Fs, params->t_delay, params->_lagIndex, params->_phi);

    /* sanity check (parameter for Fs should have been checked before call) */
    if ( params->_lagIndex < 0 || params->_lagIndex >= DCORR_MAX_LAG_ARRAY )
    {
        DEBUG_LOG("RBRDynCorr_initCorrectionCoeff(): lagIndex=%u out of range (max %u)", params->_lagIndex, DCORR_MAX_LAG_ARRAY);
        return -1;
    }
    if ( params->_phi < 0.0f || params->_phi > 1.0f )
    {
        DEBUG_LOG("RBRDynCorr_initCorrectionCoeff(): invalid value for phi (%.3f)", params->_phi);
        return -1;
    }

    return 0;
}

/* initial arrays */
void RBRDynCorr_initLagArray(RBRDynCorrParams *params)
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

/* apply temparature interpolation */
float RBRDynCorr_applyTempCorr(RBRDynCorrParams *params, float T_meas)
{
    float T_cor;

    /* (first evaluation will be incorrect, but won't be used */
    T_cor = (1.0 - params->_phi)*params->_T_meas_lag + (params->_phi)*T_meas;

    return T_cor;
}

/* sanity check on data */
int32_t RBRDynCorr_checkData(RBRDynCorrMeasurement * measIn)
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

/* update all lagged variables */
int32_t RBRDynCorr_updateLag(RBRDynCorrParams *params, const RBRDynCorrMeasurement * measIn, RBRDynCorrMeasurement * meas_out)
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


RBRDynCorrError RBRDynCorr_init(RBRDynCorrParams *params, float Fs)
{
    float F_nyquist;

    /* sanity check */
    if ( 1.0/Fs > DCORR_MAX_LAG_ARRAY*DCORR_T_DELAY )
    {
        return RBR_DCORR_INVALID_SAMPLING_RATE;
    }

    params->Fs = Fs;
    params->t_delay = DCORR_T_DELAY;
    params->alpha = DCORR_ALPHA;
    params->beta = DCORR_BETA;
    params->CT_coeff = DCORR_CT_COEFF;

    params->_firstCall = 1;
    params->_blankingPeriod = 0;
    params->_T_meas_lag = -999.0f;
    params->_T_cor_lag = 0.0f;
    params->_T_adj_lag = 0.0f;
    
    // z = ( 8.0f * params->Fs ) / params->beta;
    // a = z * params->alpha / (1.0f + z);
    // b = (1.0f - z) / (1.0f + z);
    F_nyquist = params->Fs / 2.0f;
    params->_cte_a = 4.0f*F_nyquist * (params->alpha / params->beta) / (1.0 + (4.0f*F_nyquist/params->beta));
    params->_cte_b = 1.0f -  2.0f * params->_cte_a / params->alpha;
    
    RBRDynCorr_initCorrectionCoeff(params, Fs);
    RBRDynCorr_initLagArray(params);

    return RBR_DCORR_SUCCESS;
}

RBRDynCorrError RBRDynCorr_addMeasurement(RBRDynCorrParams *params, const RBRDynCorrMeasurement * measIn, RBRDynCorrMeasurement * corrMeasOut)
{
    RBRDynCorrMeasurement measLagged;
    float T_adj = 0.0f;
    float C_cor, T_cor, T_cell;
    float timestamp;
    float C_meas, T_meas, P_meas, T_cond;
    float S_cor;
    int32_t isValid;
    int32_t isDataError;
    RBRDynCorrError statusCode = DYN_CORR_UNKNOWN_ERROR;


    T_meas = measIn->marineTemperature;

    /* initial call */
    if ( params->_firstCall && ( ISNAN(measIn->marineTemperature) == 0) )
    {
        params->_T_meas_lag = measIn->marineTemperature;
        params->_T_cor_lag = measIn->marineTemperature;
        params->_firstCall = 0;
    }

    T_cor = RBRDynCorr_applyTempCorr(params, T_meas);

    /* the variables need to be delayed until all samples are available for calculation */
    isValid = RBRDynCorr_updateLag(params, measIn, &measLagged);

    if ( !isValid )
    {
        statusCode = RBR_DCORR_NOT_VALID_YET;
    }

    if ( isValid )
    {
        /* check input */
        isDataError = RBRDynCorr_checkData(&measLagged);

        /* if T_cor becoming NAN, it will not recover
         * (other variable will eventually cleared once correct data is available) */
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

        if ( isDataError )
        {
            /* blank period (make sure it is at least 1 sample) */
            params->_blankingPeriod = (int)(params->Fs * DCORR_ERROR_PERIOD) + 1;
        }

        /* conductivity correction */
        C_cor = C_meas / ( 1.0f + params->CT_coeff * (T_cond - T_cor) );

        /* apply filter */
        T_adj = -params->_cte_b*params->_T_adj_lag + params->_cte_a*(T_cor - params->_T_cor_lag);
        T_cell = T_cor - T_adj;

        /* calculate salinity based on corrected values */
        S_cor = RBRDynCorr_PSS78(C_cor, T_cell, P_meas);

        /* update lagged variables */
        params->_T_adj_lag = T_adj;
        params->_T_cor_lag = T_cor;

        /* DIAGNOSTIC */
        //DEBUG_LOG("time= %.4f, T_cor= %.8f, C_cor= %.8f, T_cell= %.8f\n", timestamp, T_cor, C_cor, T_cell);

        /* assign output */
        corrMeasOut->timestamp = timestamp;
        corrMeasOut->conductivity = C_cor;
        corrMeasOut->marineTemperature = T_cor;
        corrMeasOut->condTemperature = T_cell;
        corrMeasOut->pressure = P_meas;
        corrMeasOut->salinity = S_cor;

        statusCode = RBR_DCORR_SUCCESS;
    }

    /* flag the data with potential issue */
    if ( params->_blankingPeriod > 0 )
    {
        params->_blankingPeriod--;
        statusCode = DYN_CORR_CORRUPTED;
    }

    return statusCode;
}




