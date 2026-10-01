/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file RBRGen3Vehicle.h
 *
 * \brief Instrument commands and structures pertaining to vehicle support.
 */

#ifndef LIBRBR_RBRGEN3VEHICLE_H
#define LIBRBR_RBRGEN3VEHICLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RBRGen3.h"

/** \brief The maximum number of regimes configurable on an instrument. */
#define RBRGEN3_REGIME_MAX 3

/** \brief The maximum regime boundary in dbar. */
#define RBRGEN3_REGIME_BOUNDARY_MAX 65535

/** \brief The maximum regime bin size in dbar. */
#define RBRGEN3_REGIME_BINSIZE_MAX 6553.5f

/** \brief The maximum sampling period within a regime. */
#define RBRGEN3_REGIME_SAMPLING_PERIOD_MAX 65000

/**
 * \brief Whether settings apply to ascent or descent.
 *
 * \see RBRGen3Regimes
 * \see RBRGen3DirectionDependentSampling
 */
typedef enum RBRGen3Direction {
    /** The settings apply while ascending. */
    RBRGEN3_DIRECTION_ASCENDING,
    /** The settings apply while descending. */
    RBRGEN3_DIRECTION_DESCENDING,
    /** The number of specific directions. */
    RBRGEN3_DIRECTION_COUNT,
    /** An unknown or unrecognized direction. */
    RBRGEN3_UNKNOWN_DIRECTION,
} RBRGen3Direction;

/**
 * \brief Get a human-readable string name for an instrument direction.
 *
 * \param [in] direction the direction
 * \return a string name for the direction
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRGen3Direction_name(RBRGen3Direction direction);

/**
 * \brief The types of pressure available for use a reference for the
 * determination of the current regime and bin.
 *
 * \see RBRGen3Regimes
 */
typedef enum RBRGen3RegimesReference {
    /** Absolute pressure is used as the reference. */
    RBRGEN3_REFERENCE_ABSOLUTE,
    /** Sea pressure is used as the reference. */
    RBRGEN3_REFERENCE_SEAPRESSURE,
    /** The number of specific regime reference types. */
    RBRGEN3_REFERENCE_COUNT,
    /** An unknown or unrecognized regime reference type. */
    RBRGEN3_UNKNOWN_REFERENCE,
} RBRGen3RegimesReference;

/**
 * \brief Get a human-readable string name for a regime pressure reference.
 *
 * \param [in] reference the pressure reference
 * \return a string name for the pressure reference
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRGen3RegimesReference_name(RBRGen3RegimesReference reference);

/**
 * \brief Instrument `regimes` command parameters.
 *
 * \see RBRGen3_getRegimes()
 * \see RBRGen3_setRegimes()
 */
typedef struct RBRGen3Regimes {
    /** \brief The regimes-relevant direction through the water column. */
    RBRGen3Direction direction;
    /**
     * \brief The number of regimes that are set.
     *
     * Must be in the range 0–3.
     */
    int32_t count;
    /** \brief The pressure type used for regime and bin determination. */
    RBRGen3RegimesReference reference;
} RBRGen3Regimes;

/**
 * \brief Get the instrument regimes settings.
 *
 * \command{regimes}
 *
 * \param [in] conn the instrument connection
 * \param [out] regimes the regimes parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_RESPONSE_TOO_LONG when a response does not fit the
 *         response buffer
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the feature is unavailable, or another
 *                                 hardware error occurs
 * \see RBRGen3_setRegimes()
 */
RBRGen3Error RBRGen3_getRegimes(RBRGen3 *conn, RBRGen3Regimes *regimes);

/**
 * \brief Set the instrument regimes settings.
 *
 * \command{regimes}
 *
 * These settings are only used when the RBRGen3Sampling.mode is
 * #RBRGEN3_SAMPLING_REGIMES.
 *
 * Hardware errors may occur if:
 *
 * - regimes are not available for the instrument
 * - the instrument is logging
 *
 * \param [in] conn the instrument connection
 * \param [in] regimes the regimes parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_RESPONSE_TOO_LONG when a response does not fit the
 *         response buffer
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the settings cannot be changed, or
 *                                 another hardware error occurs
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when too many regimes are
 *                                                requested
 * \see RBRGen3_getRegimes()
 */
RBRGen3Error RBRGen3_setRegimes(RBRGen3 *conn, const RBRGen3Regimes *regimes);

/** \brief A regime identifier. */
typedef uint8_t RBRGen3RegimeIndex;

/**
 * \brief Instrument `regime` command parameters.
 *
 * \see RBRGen3_getRegime()
 * \see RBRGen3_setRegime()
 */
typedef struct RBRGen3Regime {
    /**
     * \brief The index of the regime in question.
     *
     * The value of this member is used to determine which regime settings are
     * being retrieved or set.
     *
     * Must be in the range 1–3.
     */
    RBRGen3RegimeIndex index;
    /**
     * \brief The first boundary in a region.
     *
     * Specified in dbar.
     */
    float boundary;
    /**
     * \brief The size used for each averaged bin.
     *
     * Specified in dbar.
     */
    float binSize;
    /**
     * \brief The same meaning as RBRGen3Sampling.period, but applies
     * only to this particular regime.
     *
     * May not be greater than 65,000.
     */
    RBRGen3Period samplingPeriod;
} RBRGen3Regime;

/**
 * \brief Get the instrument regime settings.
 *
 * \command{regime}
 *
 * Set RBRGen3Regime.index to indicate which regime settings are to be
 * retrieved.
 *
 * #RBRGEN3_INVALID_PARAMETER_VALUE will be returned right away if a
 * regime index greater than 3 is requested. #RBRGEN3_HARDWARE_ERROR will
 * be returned if a regime index is given which exceeds the number of regimes
 * currently configured (RBRGen3Regimes.count).
 *
 * \param [in] conn the instrument connection
 * \param [out] regime the regime parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_RESPONSE_TOO_LONG when a response does not fit the
 *         response buffer
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the feature is unavailable, or if an
 *                                 invalid regime index is given, or another
 *                                 hardware error occurs
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when an invalid regime index
 *                                                is given
 * \see RBRGen3_setRegime()
 */
RBRGen3Error RBRGen3_getRegime(RBRGen3 *conn, RBRGen3Regime *regime);

/**
 * \brief Set the instrument regime settings.
 *
 * \command{regime}
 *
 * Hardware errors may occur if:
 *
 * - regimes are not available for the instrument
 * - the instrument is logging
 * - you set an out-of-bounds parameter the library fails to detect
 *
 * \param [in] conn the instrument connection
 * \param [in] regime the regime parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_RESPONSE_TOO_LONG when a response does not fit the
 *         response buffer
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the settings cannot be changed, or
 *                                 another hardware error occurs
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see RBRGen3_getRegime()
 */
RBRGen3Error RBRGen3_setRegime(RBRGen3 *conn, const RBRGen3Regime *regime);

/**
 * \brief Instrument `ddsampling` command parameters.
 *
 * \see RBRGen3_getDirectionDependentSampling()
 * \see RBRGen3_setDirectionDependentSampling()
 */
typedef struct RBRGen3DirectionDependentSampling {
    /** \brief In which direction the instrument samples at the fast rate. */
    RBRGen3Direction direction;
    /**
     * \brief The same meaning as RBRGen3Sampling.period, but applies
     * only when the instrument is moving in the preferred direction.
     *
     * Must be shorter than RBRGen3DirectionDependentSampling.slowPeriod.
     */
    RBRGen3Period fastPeriod;
    /**
     * \brief The same meaning as RBRGen3Sampling.period, but applies
     * only when the instrument is not moving in the preferred direction.
     *
     * Must be longer than RBRGen3DirectionDependentSampling.fastPeriod.
     */
    RBRGen3Period slowPeriod;
    /**
     * \brief Sets the boundary, based on the previous profile, where the
     * instrument should switch to the fast period sampling.
     *
     * Specified in dbar. The minimum precision is 0.1 and the value should be
     * greater than 0.
     */
    float fastThreshold;
    /**
     * \brief Sets the boundary, based on the previous profile, where the
     * instrument should switch to the slow period sampling.
     *
     * Specified in dbar. The minimum precision is 0.1 and the value should be
     * greater than 0.
     */
    float slowThreshold;
} RBRGen3DirectionDependentSampling;

/**
 * \brief Get the instrument direction-dependent sampling settings.
 *
 * \command{ddsampling}
 *
 * \param [in] conn the instrument connection
 * \param [out] ddsampling the direction-dependent sampling parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_RESPONSE_TOO_LONG when a response does not fit the
 *         response buffer
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the feature is unavailable, or another
 *                                 hardware error occurs
 * \see RBRGen3_setDirectionDependentSampling()
 */
RBRGen3Error RBRGen3_getDirectionDependentSampling(RBRGen3 *conn,
                                                   RBRGen3DirectionDependentSampling *ddsampling);

/**
 * \brief Set the instrument regime settings.
 *
 * \command{ddsampling}
 *
 * Hardware errors may occur if:
 *
 * - direction-dependent sampling is not available for the instrument
 * - the instrument is logging
 * - you set an out-of-bounds parameter the library fails to detect
 *
 * \param [in] conn the instrument connection
 * \param [in] ddsampling the direction-dependent sampling parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_RESPONSE_TOO_LONG when a response does not fit the
 *         response buffer
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the settings cannot be changed, or
 *                                 another hardware error occurs
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see RBRGen3_getDirectionDependentSampling()
 */
RBRGen3Error RBRGen3_setDirectionDependentSampling(RBRGen3 *conn,
                                                   RBRGen3DirectionDependentSampling *ddsampling);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN3VEHICLE_H */
