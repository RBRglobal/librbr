/**
 * \file RBRInstrumentGen4Gating.h
 *
 * \brief Instrument commands and structures pertaining to gated sampling.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTGEN4GATING_H
#define LIBRBR_RBRINSTRUMENTGEN4GATING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RBRInstrumentGen4.h"

/** \brief The state of a gating condition. */
typedef enum RBRInstrumentGen4GatingState
{
    /** The gating condition is disabled. */
    RBRINSTRUMENTGEN4_GATING_NA,
    /** Logging is paused due to the gating. */
    RBRINSTRUMENTGEN4_GATING_PAUSED,
    /** Logging is running due to the gating. */
    RBRINSTRUMENTGEN4_GATING_RUNNING,
    /** The number of specific gating condition types. */
    RBRINSTRUMENTGEN4_GATING_COUNT,
    /** An unknown or unrecognized gating condition type. */
    RBRINSTRUMENTGEN4_UNKNOWN_GATING
} RBRInstrumentGen4GatingState;

/**
 * \brief Get a human-readable string name for a gating state.
 *
 * \param [in] state the gating state
 * \return a string name for the gating state
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4GatingState_name(RBRInstrumentGen4GatingState state);

/**
 * \brief Possible instrument thresholding conditions.
 *
 * \see RBRInstrumentGen4Thresholding
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/thresholding
 */
typedef enum RBRInstrumentGen4ThresholdingCondition
{
    /** Sampling occurs when the monitored parameter is above the threshold. */
    RBRINSTRUMENTGEN4_THRESHOLDING_ABOVE,
    /** Sampling occurs when the monitored parameter is below the threshold. */
    RBRINSTRUMENTGEN4_THRESHOLDING_BELOW,
    /** The number of thresholding conditions. */
    RBRINSTRUMENTGEN4_THRESHOLDING_COUNT,
    /** An unknown or unrecognized thresholding condition. */
    RBRINSTRUMENTGEN4_UNKNOWN_THRESHOLDING
} RBRInstrumentGen4ThresholdingCondition;

/**
 * \brief Get a human-readable string name for a thresholding condition.
 *
 * \param [in] condition the thresholding condition
 * \return a string name for the thresholding condition
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4ThresholdingCondition_name(
    RBRInstrumentGen4ThresholdingCondition condition);

/**
 * \brief Instrument `thresholding` command parameters.
 *
 * \see RBRInstrumentGen4_getThresholding()
 * \see RBRInstrumentGen4_setThresholding()
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/thresholding
 */
typedef struct RBRInstrumentGen4Thresholding
{
    /** \brief Enables or disables thresholding. */
    bool enabled;
    /**
     * \brief The state of logging based on the thresholding configuration.
     *
     * \readonly
     *
     * \nol2 Will always be retrieved as #RBRINSTRUMENTGEN4_UNKNOWN_GATING.
     */
    const RBRInstrumentGen4GatingState state;

    /**
     * \brief The label of the channel to use for the threshold check.
     *
     * When setting the thresholding channel by label, should be provided as a
     * null-terminated C string. The value will be populated likewise when
     * retrieving thresholding parameters from the instrument.
     *
     * When setting the instrument thresholding channel, the value of this
     * field is only used when RBRInstrumentGen4Thresholding.channelSelection is
     * set to RBRINSTRUMENTGEN4_THRESHOLD_CHANNEL_BY_LABEL.
     *
     * \nol2 Use RBRInstrumentGen4Thresholding.channelIndex instead.
     */
    char channelLabel[RBRINSTRUMENTGEN4_CHANNEL_LABEL_MAX + 1];
    /** \brief Specifies the condition under which sampling will occur. */
    RBRInstrumentGen4ThresholdingCondition condition;
    /** \brief The threshold value in calibrated units. */
    float value;
    /**
     * \brief The interval between threshold checks.
     *
     * The value is given and reported in milliseconds, but must correspond to
     * a non-zero whole number of seconds (i.e., must be divisible by 1,000)
     * and must not be greater than 86,400,000 (24 hours).
     */
    RBRInstrumentGen4Period interval;
} RBRInstrumentGen4Thresholding;

/**
 * \brief Get the instrument thresholding settings.
 *
 * \param [in] instrument the instrument connection
 * \param [out] threshold the thresholding parameters
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the feature is unavailable
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/thresholding
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getThresholding(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Thresholding *threshold);

/**
 * \brief Set the instrument thresholding settings.
 *
 * Hardware errors may occur if:
 *
 * - thresholding is not available for the instrument
 * - the instrument is logging
 * - you set an out-of-bounds parameter the library fails to detect
 * - the thresholding channel selected is uncalibrated
 *
 * \param [in] instrument the instrument connection
 * \param [in] threshold the thresholding parameters
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/thresholding
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setThresholding(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Thresholding *threshold);

/**
 * \brief Instrument `twistactivation` command parameters.
 *
 * \see RBRInstrumentGen4_getTwistActivation()
 * \see RBRInstrumentGen4_setTwistActivation()
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/twistactivation
 */
typedef struct RBRInstrumentGen4TwistActivation
{
    /** \brief Enables or disables twist activation. */
    bool enabled;
    /**
     * \brief The state of logging based on the twist activation configuration.
     *
     * \readonly
     *
     * \nol2 Will always be retrieved as #RBRINSTRUMENTGEN4_UNKNOWN_GATING.
     */
    const RBRInstrumentGen4GatingState state;
} RBRInstrumentGen4TwistActivation;

/**
 * \brief Get the instrument twist activation settings.
 *
 * \param [in] instrument the instrument connection
 * \param [out] twistActivation the twist activation parameters
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the feature is unavailable
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/twistactivation
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getTwistActivation(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4TwistActivation *twistActivation);

/**
 * \brief Set the instrument twist activation settings.
 *
 * Hardware errors may occur if:
 *
 * - twist activation is not available for the instrument
 * - the instrument is logging
 *
 * \param [in] instrument the instrument connection
 * \param [in] twistActivation the twist activation parameters
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the settings cannot be changed
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/twistactivation
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setTwistActivation(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4TwistActivation *twistActivation);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTGATING_H */
