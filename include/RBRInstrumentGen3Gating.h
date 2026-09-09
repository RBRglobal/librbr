/**
 * \file RBRInstrumentGen3Gating.h
 *
 * \brief Instrument commands and structures pertaining to gated sampling.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTGEN3GATING_H
#define LIBRBR_RBRINSTRUMENTGEN3GATING_H

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The state of a gating condition. */
typedef enum RBRInstrumentGen3GatingState
{
    /** \brief The gating condition is disabled. */
    RBRINSTRUMENTGEN3_GATING_NA,
    /** \brief Logging is paused due to the gating. */
    RBRINSTRUMENTGEN3_GATING_PAUSED,
    /** \brief Logging is running due to the gating. */
    RBRINSTRUMENTGEN3_GATING_RUNNING,
    /** The number of specific gating condition types. */
    RBRINSTRUMENTGEN3_GATING_COUNT,
    /** An unknown or unrecognized gating condition type. */
    RBRINSTRUMENTGEN3_UNKNOWN_GATING
} RBRInstrumentGen3GatingState;

/**
 * \brief Get a human-readable string name for a gating state.
 *
 * \param [in] state the gating state
 * \return a string name for the gating state
 * \see RBRInstrumentGen3Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen3GatingState_name(RBRInstrumentGen3GatingState state);

/**
 * \brief Means of instrument thresholding channel selection.
 *
 * \see RBRInstrumentGen3Thresholding
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/thresholding
 */
typedef enum RBRInstrumentGen3ThresholdingChannelSelection
{
    /** The channel is set by index. */
    RBRINSTRUMENTGEN3_THRESHOLD_CHANNEL_BY_INDEX,
    /** The channel is set by label. */
    RBRINSTRUMENTGEN3_THRESHOLD_CHANNEL_BY_LABEL
} RBRInstrumentGen3ThresholdingChannelSelection;

/**
 * \brief Get a human-readable string name for a means of thresholding channel
 * selection.
 *
 * \param [in] selection the channel selection type
 * \return a string name for the channel selection type
 * \see RBRInstrumentGen3Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen3ThresholdingChannelSelection_name(
    RBRInstrumentGen3ThresholdingChannelSelection selection);

/**
 * \brief Possible instrument thresholding conditions.
 *
 * \see RBRInstrumentGen3Thresholding
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/thresholding
 */
typedef enum RBRInstrumentGen3ThresholdingCondition
{
    /** Sampling occurs when the monitored parameter is above the threshold. */
    RBRINSTRUMENTGEN3_THRESHOLDING_ABOVE,
    /** Sampling occurs when the monitored parameter is below the threshold. */
    RBRINSTRUMENTGEN3_THRESHOLDING_BELOW,
    /** The number of thresholding conditions. */
    RBRINSTRUMENTGEN3_THRESHOLDING_COUNT,
    /** An unknown or unrecognized thresholding condition. */
    RBRINSTRUMENTGEN3_UNKNOWN_THRESHOLDING
} RBRInstrumentGen3ThresholdingCondition;

/**
 * \brief Get a human-readable string name for a thresholding condition.
 *
 * \param [in] condition the thresholding condition
 * \return a string name for the thresholding condition
 * \see RBRInstrumentGen3Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen3ThresholdingCondition_name(
    RBRInstrumentGen3ThresholdingCondition condition);

/**
 * \brief Instrument `thresholding` command parameters.
 *
 * \see RBRInstrumentGen3_getThresholding()
 * \see RBRInstrumentGen3_setThresholding()
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/thresholding
 */
typedef struct RBRInstrumentGen3Thresholding
{
    /** \brief Enables or disables thresholding. */
    bool enabled;
    /**
     * \brief The state of logging based on the thresholding configuration.
     *
     * \readonly
     *
     * \nol2 Will always be retrieved as #RBRINSTRUMENTGEN3_UNKNOWN_GATING.
     */
    const RBRInstrumentGen3GatingState state;
    /**
     * \brief Whether the thresholding channel should be configured by index or
     * by label.
     *
     * \writeonly It is set to #RBRINSTRUMENTGEN3_THRESHOLD_CHANNEL_BY_INDEX when
     * retrieving parameters. For Logger2 instruments, which do not have
     * channel labels, this must be given as
     * #RBRINSTRUMENTGEN3_THRESHOLD_CHANNEL_BY_INDEX or
     * RBRInstrumentGen3_setThresholding() will return
     * #RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE.
     *
     * \see RBRInstrumentGen3_setThresholding()
     */
    RBRInstrumentGen3ThresholdingChannelSelection channelSelection;
    /**
     * \brief The index of the channel to use for the threshold check.
     *
     * When setting the instrument thresholding channel, the value of this
     * field is only used when RBRInstrumentGen3Thresholding.channelSelection is
     * set to #RBRINSTRUMENTGEN3_THRESHOLD_CHANNEL_BY_INDEX.
     */
    RBRInstrumentGen3ChannelIndex channelIndex;
    /**
     * \brief The label of the channel to use for the threshold check.
     *
     * When setting the thresholding channel by label, should be provided as a
     * null-terminated C string. The value will be populated likewise when
     * retrieving thresholding parameters from the instrument.
     *
     * When setting the instrument thresholding channel, the value of this
     * field is only used when RBRInstrumentGen3Thresholding.channelSelection is
     * set to #RBRINSTRUMENTGEN3_THRESHOLD_CHANNEL_BY_LABEL.
     *
     * \nol2 Use RBRInstrumentGen3Thresholding.channelIndex instead.
     */
    char channelLabel[RBRINSTRUMENTGEN3_CHANNEL_LABEL_MAX + 1];
    /** \brief Specifies the condition under which sampling will occur. */
    RBRInstrumentGen3ThresholdingCondition condition;
    /** \brief The threshold value in calibrated units. */
    float value;
    /**
     * \brief The interval between threshold checks.
     *
     * The value is given and reported in milliseconds, but must correspond to
     * a non-zero whole number of seconds (i.e., must be divisible by 1,000)
     * and must not be greater than 86,400,000 (24 hours).
     */
    RBRInstrumentGen3Period interval;
} RBRInstrumentGen3Thresholding;

/**
 * \brief Get the instrument thresholding settings.
 *
 * \param [in] instrument the instrument connection
 * \param [out] threshold the thresholding parameters
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN3_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN3_HARDWARE_ERROR when the feature is unavailable
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/thresholding
 */
RBRInstrumentGen3Error RBRInstrumentGen3_getThresholding(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3Thresholding *threshold);

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
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENTGEN3_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN3_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/thresholding
 */
RBRInstrumentGen3Error RBRInstrumentGen3_setThresholding(
    RBRInstrumentGen3 *instrument,
    const RBRInstrumentGen3Thresholding *threshold);

/**
 * \brief Instrument `twistactivation` command parameters.
 *
 * \see RBRInstrumentGen3_getTwistActivation()
 * \see RBRInstrumentGen3_setTwistActivation()
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/twistactivation
 */
typedef struct RBRInstrumentGen3TwistActivation
{
    /** \brief Enables or disables twist activation. */
    bool enabled;
    /**
     * \brief The state of logging based on the twist activation configuration.
     *
     * \readonly
     *
     * \nol2 Will always be retrieved as #RBRINSTRUMENTGEN3_UNKNOWN_GATING.
     */
    const RBRInstrumentGen3GatingState state;
} RBRInstrumentGen3TwistActivation;

/**
 * \brief Get the instrument twist activation settings.
 *
 * \param [in] instrument the instrument connection
 * \param [out] twistActivation the twist activation parameters
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN3_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN3_HARDWARE_ERROR when the feature is unavailable
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/twistactivation
 */
RBRInstrumentGen3Error RBRInstrumentGen3_getTwistActivation(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3TwistActivation *twistActivation);

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
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENTGEN3_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN3_HARDWARE_ERROR when the settings cannot be changed
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/twistactivation
 */
RBRInstrumentGen3Error RBRInstrumentGen3_setTwistActivation(
    RBRInstrumentGen3 *instrument,
    const RBRInstrumentGen3TwistActivation *twistActivation);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTGEN3GATING_H */
