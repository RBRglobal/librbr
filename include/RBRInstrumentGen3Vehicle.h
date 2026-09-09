/**
 * \file RBRInstrumentGen3Vehicle.h
 *
 * \brief Instrument commands and structures pertaining to vehicle support.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTGEN3VEHICLE_H
#define LIBRBR_RBRINSTRUMENTGEN3VEHICLE_H

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The maximum number of regimes configurable on an instrument. */
#define RBRINSTRUMENTGEN3_REGIME_MAX 3

/** \brief The maximum regime boundary in dbar. */
#define RBRINSTRUMENTGEN3_REGIME_BOUNDARY_MAX 65535

/** \brief The maximum regime bin size in dbar. */
#define RBRINSTRUMENTGEN3_REGIME_BINSIZE_MAX 6553.5f

/** \brief The maximum sampling period within a regime. */
#define RBRINSTRUMENTGEN3_REGIME_SAMPLING_PERIOD_MAX 65000

/**
 * \brief Whether settings apply to ascent or descent.
 *
 * \see RBRInstrumentGen3Regimes
 * \see RBRInstrumentGen3DirectionDependentSampling
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/regimes
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/ddsampling
 */
typedef enum RBRInstrumentGen3Direction
{
    /** The settings apply while ascending. */
    RBRINSTRUMENTGEN3_DIRECTION_ASCENDING,
    /** The settings apply while descending. */
    RBRINSTRUMENTGEN3_DIRECTION_DESCENDING,
    /** The number of specific directions. */
    RBRINSTRUMENTGEN3_DIRECTION_COUNT,
    /** An unknown or unrecognized direction. */
    RBRINSTRUMENTGEN3_UNKNOWN_DIRECTION
} RBRInstrumentGen3Direction;

/**
 * \brief Get a human-readable string name for an instrument direction.
 *
 * \param [in] direction the direction
 * \return a string name for the direction
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen3Direction_name(RBRInstrumentGen3Direction direction);

/**
 * \brief The types of pressure available for use a reference for the
 * determination of the current regime and bin.
 *
 * \see RBRInstrumentGen3Regimes
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/regimes
 */
typedef enum RBRInstrumentGen3RegimesReference
{
    /** Absolute pressure is used as the reference. */
    RBRINSTRUMENTGEN3_REFERENCE_ABSOLUTE,
    /** Sea pressure is used as the reference. */
    RBRINSTRUMENTGEN3_REFERENCE_SEAPRESSURE,
    /** The number of specific regime reference types. */
    RBRINSTRUMENTGEN3_REFERENCE_COUNT,
    /** An unknown or unrecognized regime reference type. */
    RBRINSTRUMENTGEN3_UNKNOWN_REFERENCE
} RBRInstrumentGen3RegimesReference;

/**
 * \brief Get a human-readable string name for a regime pressure reference.
 *
 * \param [in] reference the pressure reference
 * \return a string name for the pressure reference
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen3RegimesReference_name(
    RBRInstrumentGen3RegimesReference reference);

/**
 * \brief Instrument `regimes` command parameters.
 *
 * \see RBRInstrumentGen3_getRegimes()
 * \see RBRInstrumentGen3_setRegimes()
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/regimes
 */
typedef struct RBRInstrumentGen3Regimes
{
    /** \brief The regimes-relevant direction through the water column. */
    RBRInstrumentGen3Direction direction;
    /**
     * \brief The number of regimes that are set.
     *
     * Must be in the range 0–3.
     */
    int32_t count;
    /** \brief The pressure type used for regime and bin determination. */
    RBRInstrumentGen3RegimesReference reference;
} RBRInstrumentGen3Regimes;

/**
 * \brief Get the instrument regimes settings.
 *
 * \param [in] instrument the instrument connection
 * \param [out] regimes the regimes parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the feature is unavailable
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/regimes
 */
RBRGen3Error RBRInstrumentGen3_getRegimes(
    RBRGen3 *instrument,
    RBRInstrumentGen3Regimes *regimes);

/**
 * \brief Set the instrument regimes settings.
 *
 * These settings are only used when the RBRGen3Sampling.mode is
 * #RBRGEN3_SAMPLING_REGIMES.
 *
 * Hardware errors may occur if:
 *
 * - regimes are not available for the instrument
 * - the instrument is logging
 *
 * \param [in] instrument the instrument connection
 * \param [in] regimes the regimes parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when too many regimes are
 *                                                requested
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/regimes
 */
RBRGen3Error RBRInstrumentGen3_setRegimes(
    RBRGen3 *instrument,
    const RBRInstrumentGen3Regimes *regimes);

/** \brief A regime identifier. */
typedef uint8_t RBRInstrumentGen3RegimeIndex;

/**
 * \brief Instrument `regime` command parameters.
 *
 * \see RBRInstrumentGen3_getRegime()
 * \see RBRInstrumentGen3_setRegime()
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/regime
 */
typedef struct RBRInstrumentGen3Regime
{
    /**
     * \brief The index of the regime in question.
     *
     * The value of this member is used to determine which regime settings are
     * being retrieved or set.
     *
     * Must be in the range 1–3.
     */
    RBRInstrumentGen3RegimeIndex index;
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
} RBRInstrumentGen3Regime;

/**
 * \brief Get the instrument regime settings.
 *
 * Set RBRInstrumentGen3Regime.index to indicate which regime settings are to be
 * retrieved.
 *
 * #RBRGEN3_INVALID_PARAMETER_VALUE will be returned right away if a
 * regime index greater than 3 is requested. #RBRGEN3_HARDWARE_ERROR will
 * be returned if a regime index is given which exceeds the number of regimes
 * currently configured (RBRInstrumentGen3Regimes.count).
 *
 * \param [in] instrument the instrument connection
 * \param [out] regime the regime parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the feature is unavailable, or if
 *                                       an invalid regime index is given
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE if an invalid regime index
 *                                                is given
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/regime
 */
RBRGen3Error RBRInstrumentGen3_getRegime(
    RBRGen3 *instrument,
    RBRInstrumentGen3Regime *regime);

/**
 * \brief Set the instrument regime settings.
 *
 * Hardware errors may occur if:
 *
 * - regimes are not available for the instrument
 * - the instrument is logging
 * - you set an out-of-bounds parameter the library fails to detect
 *
 * \param [in] instrument the instrument connection
 * \param [in] regime the regime parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/regime
 */
RBRGen3Error RBRInstrumentGen3_setRegime(
    RBRGen3 *instrument,
    const RBRInstrumentGen3Regime *regime);

/**
 * \brief Instrument `ddsampling` command parameters.
 *
 * \see RBRInstrumentGen3_getDirectionDependentSampling()
 * \see RBRInstrumentGen3_setDirectionDependentSampling()
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/ddsampling
 */
typedef struct RBRInstrumentGen3DirectionDependentSampling
{
    /** \brief In which direction the instrument samples at the fast rate. */
    RBRInstrumentGen3Direction direction;
    /**
     * \brief The same meaning as RBRGen3Sampling.period, but applies
     * only when the instrument is moving in the preferred direction.
     *
     * Must be shorter than RBRInstrumentGen3DirectionDependentSampling.slowPeriod.
     */
    RBRGen3Period fastPeriod;
    /**
     * \brief The same meaning as RBRGen3Sampling.period, but applies
     * only when the instrument is not moving in the preferred direction.
     *
     * Must be longer than RBRInstrumentGen3DirectionDependentSampling.fastPeriod.
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
} RBRInstrumentGen3DirectionDependentSampling;

/**
 * \brief Get the instrument direction-dependent sampling settings.
 *
 * \param [in] instrument the instrument connection
 * \param [out] ddsampling the direction-dependent sampling parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the feature is unavailable
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/ddsampling
 */
RBRGen3Error RBRInstrumentGen3_getDirectionDependentSampling(
    RBRGen3 *instrument,
    RBRInstrumentGen3DirectionDependentSampling *ddsampling);

/**
 * \brief Set the instrument regime settings.
 *
 * Hardware errors may occur if:
 *
 * - direction-dependent sampling is not available for the instrument
 * - the instrument is logging
 * - you set an out-of-bounds parameter the library fails to detect
 *
 * \param [in] instrument the instrument connection
 * \param [in] ddsampling the direction-dependent sampling parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/ddsampling
 */
RBRGen3Error RBRInstrumentGen3_setDirectionDependentSampling(
    RBRGen3 *instrument,
    RBRInstrumentGen3DirectionDependentSampling *ddsampling);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTGEN3VEHICLE_H */
