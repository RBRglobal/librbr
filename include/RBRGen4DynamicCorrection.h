/**
 * \file RBRGen4DynamicCorrection.h
 *
 * \brief Library for salinity dynamic correction
 *
 * \copyright
 * Copyright (c) 2021 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRGEN4DYNAMICCORRECTION_H
#define LIBRBR_RBRGEN4DYNAMICCORRECTION_H

#include <stdint.h>

/*! @def DCORR_MAX_LAG_ARRAY
* \brief Define the maximum amount of lag permitted.
*/

/* Define the maximum amount of lag permitted.
 * (this need to be increase for faster sampling rate).
 * DCORR_MAX_LAG_ARRAY/F_s > t_delay */
#define DCORR_MAX_LAG_ARRAY  20


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
/*! @def DCORR_ALPHA_A
* \brief Define the 'a' ascent-rate fit coefficient for alpha (unitless)
*/
#define DCORR_ALPHA_A         0.00323f
/*! @def DCORR_ALPHA_E
* \brief Define the 'e' ascent-rate fit coefficient for alpha (unitless)
*/
#define DCORR_ALPHA_E         -1.03f
/*! 
*   @def DCORR_TAU
* \brief Define the time constant of short-term thermal mass correction (seconds)
*/
#define DCORR_TAU           8.11f
/*! @def DCORR_TAU_A
* \brief Define the 'a' ascent-rate fit coefficient for alpha (unitless)
*/
#define DCORR_TAU_A         4.93f
/*! @def DCORR_TAU_E
* \brief Define the 'e' ascent-rate fit coefficient for alpha (unitless)
*/
#define DCORR_TAU_E         -0.26f
/*! 
*   @def DCORR_CT_COEFF
* \brief Define the magnitude of long-term thermal mass correction (unitless)
*/
#define DCORR_CT_COEFF      0.97e-2f
/*! @def DCORR_CT_COEFF_A
* \brief Define the 'a' ascent-rate fit coefficient for ctcoeff (unitless)
*/
#define DCORR_CT_COEFF_A         0.00139f
/*! @def DCORR_CT_COEFF_E
* \brief Define the 'e' ascent-rate fit coefficient for ctcoeff (unitless)
*/
#define DCORR_CT_COEFF_E         -1.00f
/*! 
*   @def DCORR_VP_MIN
* \brief Define the minimum for the range for ascent rate as pressure/time (dbar/sec)
*/
#define DCORR_VP_MIN      0.03f
/*! 
*   @def DCORR_VP_MAX
* \brief Define the minimum for the range for ascent rate as pressure/time (dbar/sec)
*/
#define DCORR_VP_MAX      0.45f
/*! 
*   @def DCORR_VP_FC
* \brief Define the filter cutoff frequency for ascent rate as pressure/time (Hz)
*/
#define DCORR_VP_FC      0.04f


/**
 * \brief Errors which can be returned from dynamic correction algorithm
 *
 * Algorithm will return error codes in lieu of
 * data values; data will be passed back to the caller via out pointers. This
 * allows for predictable and consistent error checking by the caller.
 */
typedef enum RBRGen4DynamicCorrectionError
{
    /** No error. */
    RBRGEN4DYNAMICCORRECTION_SUCCESS = 0,
    /** Invalid sampling rate for given parameters */
    RBRGEN4DYNAMICCORRECTION_INVALID_SAMPLING_RATE,
    /** Insufficient data injected in function to provide a result */
    RBRGEN4DYNAMICCORRECTION_NOT_VALID_YET,
    /** Invalid correction (could be related to previous input) */
    RBRGEN4DYNAMICCORRECTION_CORRUPTED,
    /** Invalid parameters (initialization failure) */
    RBRGEN4DYNAMICCORRECTION_BAD_PARAMS,
    /** Other error */
    RBRGEN4DYNAMICCORRECTION_UNKNOWN_ERROR
} RBRGen4DynamicCorrectionError;

/** \brief RBRGen4DynamicCorrectionParams
   *  This is a struct 
   */
typedef struct RBRGen4DynamicCorrectionParams
{
    /** \brief time delay (sec), or C-T lag*/
    float t_delay;

    /** \brief sampling rate (Hz)*/
    float Fs;

    /** \brief magnitude of short-term thermal mass correction*/
    float alpha;

    /** \brief time constant of short-term thermal mass correction*/
    float tau;

    /** \brief magnitude of long-term thermal mass correction*/
    float CT_coeff;

    /** \brief coefficient for alpha estimation*/
    float alpha_a;    /* alpha = alpha_a * powf(Vp * alpha_e) */

    /** \brief coefficient for alpha estimation*/
    float alpha_e;

    /** \brief coefficient for tau estimation*/
    float tau_a;      /* tau = tau_a * powf(Vp * tau_e) */

    /** \brief coefficient for tau estimation*/
    float tau_e;

    /** \brief coefficient for CT_coeff estimation*/
    float ctcoeff_a;  /* ctcoeff = ctcoeff_a * powf(Vp * ctcoeff_e) */
    
    /** \brief coefficient for CT_coeff estimation*/
    float ctcoeff_e;

    /** \brief minimum of ascent rate's validity range(m/s)*/
    float Vp_min;

    /** \brief maximum of ascent rate's validity range(m/s)*/
    float Vp_max;

    /** \brief frequency cut of ascent rate estimation low pass filter(Hz)*/
    float Vp_fc;

    /* --- internal private data --- */
    /** \cond */
    int32_t _firstCall;
    int32_t _isError;
    float _ascentRate;
    float _lastPressure;
    float _lastPressureTime;
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
    /** \endcond */
} RBRGen4DynamicCorrectionParams;

/** \brief RBRGen4DynamicCorrectionMeasurement
   *  This is a struct
   */
typedef struct RBRGen4DynamicCorrectionMeasurement{
    /** \brief Time in seconds*/
    float timestamp;

    /** \brief Conductivity measurement (mS/cm)*/
    float conductivity;

    /** \brief Marine temperature measurement (°C)*/
    float marineTemperature;

    /** \brief Temperature of conductivity cell measurement (°C)*/
    float condTemperature;

    /** \brief Pressure measurement (dbar)*/
    float pressure;
} RBRGen4DynamicCorrectionMeasurement;

/** \brief RBRGen4DynamicCorrectionResult
   *  This is a struct
   */
typedef struct RBRGen4DynamicCorrectionResult{
    /** \brief Time in seconds*/
    float timestamp;

    /** \brief Conductivity measured (mS/cm)*/
    float conductivity;

    /** \brief Corrected temperature (°C)*/
    float corrTemperature;

    /** \brief Sea pressure measurement (dbar)*/
    float pressure;

    /** \brief Practical salinity after all corrections (corrected, unitless)*/
    float corrSalinity;
} RBRGen4DynamicCorrectionResult;

/**
 * @brief Initialize the dynamic correction algorithm.
 * 
 * Initialize the algorithm for the given sampling rate.
 *
 * @param [in] Fs sampling rate (Samples/sec)
 * @param [in] t_delay default value DCORR_T_DELAY used as input
 * @param [in] alpha_a default value DCORR_ALPHA_A used as input
 * @param [in] alpha_e default value DCORR_ALPHA_E used as input
 * @param [in] tau_a default value DCORR_TAU_A used as input
 * @param [in] tau_e default value DCORR_TAU_E used as input
 * @param [in] ctcoeff_a default value DCORR_COEFF_A used as input
 * @param [in] ctcoeff_e default value DCORR_COEFF_E used as input
 * @param [in] Vp_min default value DCORR_VP_MIN used as input
 * @param [in] Vp_max default value DCORR_VP_MAX used as input
 * @param [in] Vp_fc default value DCORR_VP_FC used as input
 * @param [inout] params Parameters for dynamic correction algorithm
 * @return error code (0 = no error)
 */
RBRGen4DynamicCorrectionError RBRGen4DynamicCorrection_init(const float Fs,
                                            const float t_delay, const float alpha_a, const float alpha_e, 
                                            const float tau_a, const float tau_e, const float ctcoeff_a, const float ctcoeff_e, 
                                            const float Vp_min, const float Vp_max, const float Vp_fc,
                                            RBRGen4DynamicCorrectionParams *params);

/**
 * @brief Change the sampling rate for the algorithm.
 *
 * @param [in] Fs sampling rate (Samples/sec)
 * @param [inout] params Parameters for dynamic correction algorithm
 * @return error code (0 = no error)
 */
RBRGen4DynamicCorrectionError RBRGen4DynamicCorrection_update_Fs(const float Fs, RBRGen4DynamicCorrectionParams *params);

/**
 * @brief Feed a new measurement in the algorithm.  
 * 
 * Return a corrected output (with proper time delay to align with all correction results)
 * 
 * @param [in] measIn Input measurements for algorithm
 * @param [inout] params Parameters for dynamic correction algorithm
 * @param [out] corrMeasOut Output corrected measurements (time aligned)
 * @return error code (0 = no error)
 */
RBRGen4DynamicCorrectionError RBRGen4DynamicCorrection_addMeasurement(const RBRGen4DynamicCorrectionMeasurement * measIn, RBRGen4DynamicCorrectionParams *params, RBRGen4DynamicCorrectionResult * corrMeasOut);


#endif /* LIBRBR_RBRGEN4DYNAMICCORRECTION_H */
