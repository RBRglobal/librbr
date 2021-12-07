/**
 * \file RBRDynamicCorrection.h
 *
 * \brief Library for salinity dynamic correction
 *
 * \copyright
 * Copyright (c) 2021 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_DYNAMICCORRECTION_H
#define LIBRBR_DYNAMICCORRECTION_H

#include <stdint.h>


/*! @def DCORR_MAX_LAG_ARRAY
* \brief Define the maximum amount of lag permitted.
*/

/* Define the maximum amount of lag permitted.
 * (this need to be increase for faster sampling rate).
 * DCORR_MAX_LAG_ARRAY/F_s > t_delay */
#define DCORR_MAX_LAG_ARRAY  20

/*! @def DCORR_ERROR_PERIOD
* \brief Define the period before an error condition in the algorithm is cleared (in seconds)
*/
#define DCORR_ERROR_PERIOD  10.0f


/* default parameters
 * (applicable for 10cm/sec ascent/descent rate) */

/*! @def DCORR_T_DELAY 
* \brief Define the C-T lag adjustment delay (in seconds)
*/
#define DCORR_T_DELAY       0.35f
/*! @def DCORR_ALPHA
* \brief Define the magnitude of short-term thermal mass correction (unitless)
*/
#define DCORR_ALPHA         0.041f
/*! 
*   @def DCORR_TAU
* \brief Define the time constant of short-term thermal mass correction (seconds)
*/
#define DCORR_TAU           8.11f
/*! 
*   @def DCORR_CT_COEFF
* \brief Define the magnitude of long-term thermal mass correction (unitless)
*/
#define DCORR_CT_COEFF      0.97e-2f


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
} RBRDynamicCorrectionError;

/** @struct RBRDynamicCorrectionParams
   *  This is a struct
   *
   *  @var RBRDynamicCorrectionParams::t_delay
   *    time delay (sec), or C-T lag
   *  @var RBRDynamicCorrectionParams::Fs
   *    sampling rate (Hz)
   *  @var RBRDynamicCorrectionParams::alpha
   *    magnitude of short-term thermal mass correction
   *  @var RBRDynamicCorrectionParams::tau
   *    time constant of short-term thermal mass correction
   *  @var RBRDynamicCorrectionParams::CT_coeff
   *    magnitude of long-term thermal mass correction
   */
typedef struct
{
    float t_delay;  // time delay (sec), or C-T lag
    float Fs;       // sampling rate (Hz)
    float alpha;
    float tau;
    float CT_coeff;
    // --- internal private data ---
    /// @cond
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
    float _T_short_lag;
    int32_t _isValid_lagArray[DCORR_MAX_LAG_ARRAY];
    float _timestamp_lagArray[DCORR_MAX_LAG_ARRAY];
    float _C_meas_lagArray[DCORR_MAX_LAG_ARRAY];
    float _P_meas_lagArray[DCORR_MAX_LAG_ARRAY];
    float _T_cond_lagArray[DCORR_MAX_LAG_ARRAY];    
    /// @endcond
} RBRDynamicCorrectionParams;

/** @struct RBRDynamicCorrectionMeasurement
   *  This is a struct
   *
   *  @var RBRDynamicCorrectionMeasurement::timestamp
   *    Time in seconds
   *  @var RBRDynamicCorrectionMeasurement::conductivity
   *    Conductivity measurement (mS/cm)
   *  @var RBRDynamicCorrectionMeasurement::marineTemperature
   *    Marine temperature measurement (°C)
   *  @var RBRDynamicCorrectionMeasurement::condTemperature
   *    Temperature of conductivity cell measurement (°C)
   *  @var RBRDynamicCorrectionMeasurement::pressure
   *    Pressure measurement (dbar)
   */
typedef struct {
    float timestamp;            // Time in seconds
    float conductivity;         // Conductivity measurement (mS/cm)
    float marineTemperature;    // Marine temperature measurement (°C)
    float condTemperature;      // Temperature of conductivity cell measurement (°C)
    float pressure;             // Pressure measurement (dbar)
} RBRDynamicCorrectionMeasurement;

/** @struct RBRDynamicCorrectionResult
   *  This is a struct
   *
   *  @var RBRDynamicCorrectionResult::timestamp
   *    Time in seconds
   *  @var RBRDynamicCorrectionResult::conductivity
   *    Conductivity measured (mS/cm)
   *  @var RBRDynamicCorrectionResult::corrTemperature
   *    Corrected temperature (°C)
   *  @var RBRDynamicCorrectionResult::pressure
   *    Sea pressure measurement (dbar)
   *  @var RBRDynamicCorrectionResult::corrSalinity
   *    Practical salinity after all corrections (corrected, unitless)
   */
typedef struct {
    float timestamp;            // Time in seconds
    float conductivity;         // Conductivity measurement (mS/cm)
    float corrTemperature;      // Corrected temperature (°C)
    float pressure;             // Sea pressure measurement (dbar)
    float corrSalinity;         // Practical salinity after all corrections (unitless)
} RBRDynamicCorrectionResult;

/**
 * @brief Initialize the dynamic correction algorithm.
 * 
 * Initialize the algorithm for the given sampling rate.
 * If the sampling is modified, the initialization need to be called
 * again (previous history will be discarded)
 *
 * @param params Parameters for dynamic correction algorithm
 * @param Fs sampling rate (Samples/sec)
 * @param t_delay default value DCORR_<paraName> used as input
 * @param alpha default value DCORR_<paraName> used as input
 * @param tau default value DCORR_<paraName> used as input
 * @param CT_coeff default value DCORR_<paraName> used as input
 * @return error code (0 = no error)
 */
RBRDynamicCorrectionError RBRDynamicCorrection_init(RBRDynamicCorrectionParams *params, float Fs,
                                            float t_delay, float alpha, float tau, float CT_coeff);

/**
 * @brief Change the sampling rate for the algorithm.
 *
 * @param params Parameters for dynamic correction algorithm
 * @param Fs sampling rate (Samples/sec)
 * @return error code (0 = no error)
 */
RBRDynamicCorrectionError RBRDynamicCorrection_update_Fs(RBRDynamicCorrectionParams *params, float Fs);

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
RBRDynamicCorrectionError RBRDynamicCorrection_addMeasurement(RBRDynamicCorrectionParams *params, const RBRDynamicCorrectionMeasurement * measIn, RBRDynamicCorrectionResult * corrMeasOut);


#endif // LIBRBR_DYNAMICCORRECTION_H
