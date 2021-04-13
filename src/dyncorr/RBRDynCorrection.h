/**
 * \file RBRDynamicCorrection.h
 *
 * \brief Library for salinity dynamic correction
 *
 * \copyright
 * Copyright (c) 2021 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_DYNCORRECTION_H
#define LIBRBR_DYNCORRECTION_H

#include <stdint.h>

#ifdef _DEBUG
#define DEBUG_LOG(x, ...) printf(x, __VA_ARGS__)
#else
#define DEBUG_LOG(x, ...) {}
#endif


/* Define the maximum amount of lag permitted.
 * (this need to be increase for faster sampling rate).
 * DCORR_MAX_LAG_ARRAY/F_s > t_delay */
#define DCORR_MAX_LAG_ARRAY  20

/* default parameters */
#define DCORR_T_DELAY       0.35f
#define DCORR_ALPHA         0.08f
#define DCORR_BETA          0.125f
#define DCORR_CT_COEFF      2.4e-4f
#define DCORR_ERROR_PERIOD  10.0f

/**
 * \brief Errors which can be returned from dynamic correction algorithm
 *
 * Algorithm will return error codes in lieu of
 * data values; data will be passed back to the caller via out pointers. This
 * allows for predictable and consistent error checking by the caller.
 */
typedef enum
{
    /** No error. */
    RBR_DCORR_SUCCESS = 0,
    /** Invalid sampling rate for given parameters */
    RBR_DCORR_INVALID_SAMPLING_RATE,
    /** Insufficient data injected in function to provide a result */
    RBR_DCORR_NOT_VALID_YET,
    /** Invalid correction (could be related to previous input) */
    DYN_CORR_CORRUPTED,
    /** Other error */
    DYN_CORR_UNKNOWN_ERROR
} RBR_DynCorrError;


typedef struct
{
    float t_delay;  // time delay (sec)
    float Fs;       // sampling rate (Hz)
    float alpha;
    float beta;
    float CT_coeff;
    // --- internal private data ---
    int32_t _firstCall;
    int32_t _blankingPeriod;
    int32_t _isError;
    float _phi;
    float _cte_a;
    float _cte_b;
    int32_t _lagIndex;
    float _T_meas_lag;
    float _C_meas_lag;
    float _T_cond_lag;
    float _P_meas_lag;
    float _T_cor_lag;
    float _T_adj_lag;
    int32_t _isValid_lagArray[DCORR_MAX_LAG_ARRAY];
    float _timestamp_lagArray[DCORR_MAX_LAG_ARRAY];
    float _C_meas_lagArray[DCORR_MAX_LAG_ARRAY];
    float _P_meas_lagArray[DCORR_MAX_LAG_ARRAY];
    float _T_cond_lagArray[DCORR_MAX_LAG_ARRAY];
} RBR_DynCorrParams;

typedef struct {
    float timestamp;            // time in seconds
    float conductivity;         // Conductivity measurement (mS/cm)
    float marineTemperature;   // Marine temperature measurement (Celcius)
    float condTemperature;     // Temperature of conductivity cell measurement (Celcius)
    float pressure;             // Pressure measurement (dBar)
    float salinity;             // Practical salinity (unitless)
} RBR_DynCorrMeasurement;


/**
 * @brief Initialize the dynamic correction algorithm.
 * 
 * Initialize the algorithm for the given sampling rate.
 * If the sampling is modified, the initialization need to be called
 * again (previous history will be discarded)
 *
 * @param params Parameters for dynamic correction algorithm
 * @param Fs sampling rate (Samples/sec)
 * @return error code (0 = no error)
 */
RBR_DynCorrError RBRDynCorr_init(RBR_DynCorrParams *params, float Fs);

/**
 * @brief Feed a new measurement in the algorithm.  
 * 
 * Return a corrected output (with proper time delay to align with all correction results)
 *
 * @param params Parameters for dynamic correction algorithm
 * @param measIn Input measurements for algorithm
 * @param corrMeasOut Output corrected measurements (time aligned)
 * @return error code (0 = no error)
 */
RBR_DynCorrError RBRDynCorr_addMeasurement(RBR_DynCorrParams *params, const RBR_DynCorrMeasurement * measIn, RBR_DynCorrMeasurement * corrMeasOut);


#endif // LIBRBR_DYNCORRECTION_H