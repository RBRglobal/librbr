/**
 * \file RBRDynamicCorrectionGen3.h
 *
 * \brief Library for salinity dynamic correction
 *
 * \copyright
 * Copyright (c) 2021 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRDYNAMICCORRECTIONGEN3_H
#define LIBRBR_RBRDYNAMICCORRECTIONGEN3_H

#include <stdint.h>

/*! @def DCORR_MAX_LAG_ARRAY
 * \brief Define the maximum amount of lag permitted.
 */

/* Define the maximum amount of lag permitted.
 * (this need to be increase for faster sampling rate).
 * DCORR_MAX_LAG_ARRAY/F_s > t_delay */
#define DCORR_MAX_LAG_ARRAY 20

/* default parameters
 * (applicable for 10cm/sec ascent/descent rate) */

/*! @def DCORR_T_DELAY
 * \brief Define the C-T lag adjustment delay (in seconds)
 */
#define DCORR_T_DELAY 0.35f
/*! @def DCORR_ALPHA
 * \brief Define the magnitude of short-term thermal mass correction (unitless)
 */
#define DCORR_ALPHA 0.041f
/*! @def DCORR_ALPHA_A
 * \brief Define the 'a' ascent-rate fit coefficient for alpha (unitless)
 */
#define DCORR_ALPHA_A 0.00323f
/*! @def DCORR_ALPHA_E
 * \brief Define the 'e' ascent-rate fit coefficient for alpha (unitless)
 */
#define DCORR_ALPHA_E -1.03f
/*!
 *   @def DCORR_TAU
 * \brief Define the time constant of short-term thermal mass correction (seconds)
 */
#define DCORR_TAU 8.11f
/*! @def DCORR_TAU_A
 * \brief Define the 'a' ascent-rate fit coefficient for alpha (unitless)
 */
#define DCORR_TAU_A 4.93f
/*! @def DCORR_TAU_E
 * \brief Define the 'e' ascent-rate fit coefficient for alpha (unitless)
 */
#define DCORR_TAU_E -0.26f
/*!
 *   @def DCORR_CT_COEFF
 * \brief Define the magnitude of long-term thermal mass correction (unitless)
 */
#define DCORR_CT_COEFF 0.97e-2f
/*! @def DCORR_CT_COEFF_A
 * \brief Define the 'a' ascent-rate fit coefficient for ctcoeff (unitless)
 */
#define DCORR_CT_COEFF_A 0.00139f
/*! @def DCORR_CT_COEFF_E
 * \brief Define the 'e' ascent-rate fit coefficient for ctcoeff (unitless)
 */
#define DCORR_CT_COEFF_E -1.00f
/*!
 *   @def DCORR_VP_MIN
 * \brief Define the minimum for the range for ascent rate as pressure/time (dbar/sec)
 */
#define DCORR_VP_MIN 0.03f
/*!
 *   @def DCORR_VP_MAX
 * \brief Define the minimum for the range for ascent rate as pressure/time (dbar/sec)
 */
#define DCORR_VP_MAX 0.45f
/*!
 *   @def DCORR_VP_FC
 * \brief Define the filter cutoff frequency for ascent rate as pressure/time (Hz)
 */
#define DCORR_VP_FC 0.04f

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
    RBRDYNAMICCORRECTIONGEN3_SUCCESS = 0,
    /** Invalid sampling rate for given parameters */
    RBRDYNAMICCORRECTIONGEN3_INVALID_SAMPLING_RATE,
    /** Insufficient data injected in function to provide a result */
    RBRDYNAMICCORRECTIONGEN3_NOT_VALID_YET,
    /** Invalid correction (could be related to previous input) */
    RBRDYNAMICCORRECTIONGEN3_CORRUPTED,
    /** Invalid parameters (initialization failure) */
    RBRDYNAMICCORRECTIONGEN3_BAD_PARAMS,
    /** Other error */
    RBRDYNAMICCORRECTIONGEN3_UNKNOWN_ERROR
} RBRDynamicCorrectionGen3Error;

/** @struct RBRDynamicCorrectionGen3Params
 *  This is a struct
 *
 *  @var RBRDynamicCorrectionGen3Params::t_delay
 *    time delay (sec), or C-T lag
 *  @var RBRDynamicCorrectionGen3Params::Fs
 *    sampling rate (Hz)
 *  @var RBRDynamicCorrectionGen3Params::alpha
 *    magnitude of short-term thermal mass correction
 *  @var RBRDynamicCorrectionGen3Params::tau
 *    time constant of short-term thermal mass correction
 *  @var RBRDynamicCorrectionGen3Params::CT_coeff
 *    magnitude of long-term thermal mass correction
 */
typedef struct
{
    float t_delay;  // time delay (sec), or C-T lag
    float Fs;       // sampling rate (Hz)
    float alpha;
    float tau;
    float CT_coeff;
    float alpha_a;  // alpha = alpha_a * powf(Vp * alpha_e)
    float alpha_e;
    float tau_a;  // tau = tau_a * powf(Vp * tau_e)
    float tau_e;
    float ctcoeff_a;  // ctcoeff = ctcoeff_a * powf(Vp * ctcoeff_e)
    float ctcoeff_e;
    float Vp_min;
    float Vp_max;
    float Vp_fc;
    // --- internal private data ---
    /// @cond
    int32_t _firstCall;
    int32_t _isError;
    float _ascentRate;
    float _lastPressure;
    int64_t _lastPressureTime; //Time in milliseconds
    float _phi;
    float _cte_a;
    float _cte_b;
    int32_t _isFasterSampling;
    int32_t _lagIndex;
    float _T_meas_lag;
    float _C_meas_lag;
    float _T_cond_lag;
    float _P_meas_lag;
    float _T_cor_lag;
    float _T_short_lag;
    int32_t _isValid_lagArray[DCORR_MAX_LAG_ARRAY];
    int64_t _timestamp_lagArray[DCORR_MAX_LAG_ARRAY]; //in milliseconds
    float _C_meas_lagArray[DCORR_MAX_LAG_ARRAY];
    float _P_meas_lagArray[DCORR_MAX_LAG_ARRAY];
    float _T_cond_lagArray[DCORR_MAX_LAG_ARRAY];
    /// @endcond
} RBRDynamicCorrectionGen3Params;

/** @struct RBRDynamicCorrectionGen3Measurement
 *  This is a struct
 *
 *  @var RBRDynamicCorrectionGen3Measurement::timestamp
 *    Time in milliseconds
 *  @var RBRDynamicCorrectionGen3Measurement::conductivity
 *    Conductivity measurement (mS/cm)
 *  @var RBRDynamicCorrectionGen3Measurement::marineTemperature
 *    Marine temperature measurement (°C)
 *  @var RBRDynamicCorrectionGen3Measurement::condTemperature
 *    Temperature of conductivity cell measurement (°C)
 *  @var RBRDynamicCorrectionGen3Measurement::pressure
 *    Pressure measurement (dbar)
 */
typedef struct
{
    int64_t timestamp;        // Time in milliseconds
    float conductivity;       // Conductivity measurement (mS/cm)
    float marineTemperature;  // Marine temperature measurement (°C)
    float condTemperature;    // Temperature of conductivity cell measurement (°C)
    float pressure;           // Pressure measurement (dbar)
} RBRDynamicCorrectionGen3Measurement;

/** @struct RBRDynamicCorrectionGen3Result
 *  This is a struct
 *
 *  @var RBRDynamicCorrectionGen3Result::timestamp
 *    Time in milliseconds
 *  @var RBRDynamicCorrectionGen3Result::conductivity
 *    Conductivity measured (mS/cm)
 *  @var RBRDynamicCorrectionGen3Result::corrTemperature
 *    Corrected temperature (°C)
 *  @var RBRDynamicCorrectionGen3Result::pressure
 *    Sea pressure measurement (dbar)
 *  @var RBRDynamicCorrectionGen3Result::corrSalinity
 *    Practical salinity after all corrections (corrected, unitless)
 */
typedef struct
{
    int64_t timestamp;      // Time in milliseconds
    float conductivity;     // Conductivity measurement (mS/cm)
    float corrTemperature;  // Corrected temperature (°C)
    float pressure;         // Sea pressure measurement (dbar)
    float corrSalinity;     // Practical salinity after all corrections (unitless)
} RBRDynamicCorrectionGen3Result;

/**
 * @brief Initialize the dynamic correction algorithm.
 *
 * Initialize the algorithm for the given sampling rate.
 *
 * @param params Parameters for dynamic correction algorithm
 * @param Fs sampling rate (Samples/sec)
 * @param t_delay default value DCORR_T_DELAY used as input
 * @param alpha_a default value DCORR_ALPHA_A used as input
 * @param alpha_e default value DCORR_ALPHA_E used as input
 * @param tau_a default value DCORR_TAU_A used as input
 * @param tau_e default value DCORR_TAU_E used as input
 * @param ctcoeff_a default value DCORR_COEFF_A used as input
 * @param ctcoeff_e default value DCORR_COEFF_E used as input
 * @param Vp_min default value DCORR_VP_MIN used as input
 * @param Vp_max default value DCORR_VP_MAX used as input
 * @param Vp_fc default value DCORR_VP_FC used as input
 * @return error code (0 = no error)
 */
RBRDynamicCorrectionGen3Error RBRDynamicCorrectionGen3_init(RBRDynamicCorrectionGen3Params *params, float Fs, float t_delay,
                                                    float alpha_a, float alpha_e, float tau_a, float tau_e,
                                                    float ctcoeff_a, float ctcoeff_e, float Vp_min, float Vp_max,
                                                    float Vp_fc);

/**
 * @brief Change the sampling rate for the algorithm.
 *
 * @param params Parameters for dynamic correction algorithm
 * @param Fs sampling rate (Samples/sec)
 * @return error code (0 = no error)
 */
RBRDynamicCorrectionGen3Error RBRDynamicCorrectionGen3_update_Fs(RBRDynamicCorrectionGen3Params *params, float Fs);

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
RBRDynamicCorrectionGen3Error RBRDynamicCorrectionGen3_addMeasurement(RBRDynamicCorrectionGen3Params *params,
                                                              const RBRDynamicCorrectionGen3Measurement *measIn,
                                                              RBRDynamicCorrectionGen3Result *corrMeasOut);

#endif  // LIBRBR_RBRDYNAMICCORRECTIONGEN3_H
