/**
 * \file RBRGen3Gating.h
 *
 * \brief Instrument commands and structures pertaining to gated sampling.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRGEN3GATING_H
#define LIBRBR_RBRGEN3GATING_H

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The state of a gating condition. */
typedef enum RBRGen3GatingState
{
    /** \brief The gating condition is disabled. */
    RBRGEN3_GATING_NA,
    /** \brief Logging is paused due to the gating. */
    RBRGEN3_GATING_PAUSED,
    /** \brief Logging is running due to the gating. */
    RBRGEN3_GATING_RUNNING,
    /** The number of specific gating condition types. */
    RBRGEN3_GATING_COUNT,
    /** An unknown or unrecognized gating condition type. */
    RBRGEN3_UNKNOWN_GATING
} RBRGen3GatingState;

/**
 * \brief Get a human-readable string name for a gating state.
 *
 * \param [in] state the gating state
 * \return a string name for the gating state
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRGen3GatingState_name(RBRGen3GatingState state);

/**
 * \brief Means of instrument thresholding channel selection.
 *
 * \see RBRGen3Thresholding
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/thresholding
 */
typedef enum RBRGen3ThresholdingChannelSelection
{
    /** The channel is set by index. */
    RBRGEN3_THRESHOLD_CHANNEL_BY_INDEX,
    /** The channel is set by label. */
    RBRGEN3_THRESHOLD_CHANNEL_BY_LABEL
} RBRGen3ThresholdingChannelSelection;

/**
 * \brief Get a human-readable string name for a means of thresholding channel
 * selection.
 *
 * \param [in] selection the channel selection type
 * \return a string name for the channel selection type
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRGen3ThresholdingChannelSelection_name(
    RBRGen3ThresholdingChannelSelection selection);

/**
 * \brief Possible instrument thresholding conditions.
 *
 * \see RBRGen3Thresholding
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/thresholding
 */
typedef enum RBRGen3ThresholdingCondition
{
    /** Sampling occurs when the monitored parameter is above the threshold. */
    RBRGEN3_THRESHOLDING_ABOVE,
    /** Sampling occurs when the monitored parameter is below the threshold. */
    RBRGEN3_THRESHOLDING_BELOW,
    /** The number of thresholding conditions. */
    RBRGEN3_THRESHOLDING_COUNT,
    /** An unknown or unrecognized thresholding condition. */
    RBRGEN3_UNKNOWN_THRESHOLDING
} RBRGen3ThresholdingCondition;

/**
 * \brief Get a human-readable string name for a thresholding condition.
 *
 * \param [in] condition the thresholding condition
 * \return a string name for the thresholding condition
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRGen3ThresholdingCondition_name(
    RBRGen3ThresholdingCondition condition);

/**
 * \brief Instrument `thresholding` command parameters.
 *
 * \see RBRGen3_getThresholding()
 * \see RBRGen3_setThresholding()
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/thresholding
 */
typedef struct RBRGen3Thresholding
{
    /** \brief Enables or disables thresholding. */
    bool enabled;
    /**
     * \brief The state of logging based on the thresholding configuration.
     *
     * \readonly
     *
     * \nol2 Will always be retrieved as #RBRGEN3_UNKNOWN_GATING.
     */
    const RBRGen3GatingState state;
    /**
     * \brief Whether the thresholding channel should be configured by index or
     * by label.
     *
     * \writeonly It is set to #RBRGEN3_THRESHOLD_CHANNEL_BY_INDEX when
     * retrieving parameters. For Logger2 instruments, which do not have
     * channel labels, this must be given as
     * #RBRGEN3_THRESHOLD_CHANNEL_BY_INDEX or
     * RBRGen3_setThresholding() will return
     * #RBRGEN3_INVALID_PARAMETER_VALUE.
     *
     * \see RBRGen3_setThresholding()
     */
    RBRGen3ThresholdingChannelSelection channelSelection;
    /**
     * \brief The index of the channel to use for the threshold check.
     *
     * When setting the instrument thresholding channel, the value of this
     * field is only used when RBRGen3Thresholding.channelSelection is
     * set to #RBRGEN3_THRESHOLD_CHANNEL_BY_INDEX.
     */
    RBRGen3ChannelIndex channelIndex;
    /**
     * \brief The label of the channel to use for the threshold check.
     *
     * When setting the thresholding channel by label, should be provided as a
     * null-terminated C string. The value will be populated likewise when
     * retrieving thresholding parameters from the instrument.
     *
     * When setting the instrument thresholding channel, the value of this
     * field is only used when RBRGen3Thresholding.channelSelection is
     * set to #RBRGEN3_THRESHOLD_CHANNEL_BY_LABEL.
     *
     * \nol2 Use RBRGen3Thresholding.channelIndex instead.
     */
    char channelLabel[RBRGEN3_CHANNEL_LABEL_MAX + 1];
    /** \brief Specifies the condition under which sampling will occur. */
    RBRGen3ThresholdingCondition condition;
    /** \brief The threshold value in calibrated units. */
    float value;
    /**
     * \brief The interval between threshold checks.
     *
     * The value is given and reported in milliseconds, but must correspond to
     * a non-zero whole number of seconds (i.e., must be divisible by 1,000)
     * and must not be greater than 86,400,000 (24 hours).
     */
    RBRGen3Period interval;
} RBRGen3Thresholding;

/**
 * \brief Get the instrument thresholding settings.
 *
 * \param [in] conn the instrument connection
 * \param [out] threshold the thresholding parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the feature is unavailable, or another
 *                                 hardware error occurs
 * \see RBRGen3_setThresholding()
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/thresholding
 */
RBRGen3Error RBRGen3_getThresholding(
    RBRGen3 *conn,
    RBRGen3Thresholding *threshold);

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
 * \param [in] conn the instrument connection
 * \param [in] threshold the thresholding parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the settings cannot be changed, or
 *                                 another hardware error occurs
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see RBRGen3_getThresholding()
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/thresholding
 */
RBRGen3Error RBRGen3_setThresholding(
    RBRGen3 *conn,
    const RBRGen3Thresholding *threshold);

/**
 * \brief Instrument `twistactivation` command parameters.
 *
 * \see RBRGen3_getTwistActivation()
 * \see RBRGen3_setTwistActivation()
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/twistactivation
 */
typedef struct RBRGen3TwistActivation
{
    /** \brief Enables or disables twist activation. */
    bool enabled;
    /**
     * \brief The state of logging based on the twist activation configuration.
     *
     * \readonly
     *
     * \nol2 Will always be retrieved as #RBRGEN3_UNKNOWN_GATING.
     */
    const RBRGen3GatingState state;
} RBRGen3TwistActivation;

/**
 * \brief Get the instrument twist activation settings.
 *
 * \param [in] conn the instrument connection
 * \param [out] twistActivation the twist activation parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the feature is unavailable, or another
 *                                 hardware error occurs
 * \see RBRGen3_setTwistActivation()
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/twistactivation
 */
RBRGen3Error RBRGen3_getTwistActivation(
    RBRGen3 *conn,
    RBRGen3TwistActivation *twistActivation);

/**
 * \brief Set the instrument twist activation settings.
 *
 * Hardware errors may occur if:
 *
 * - twist activation is not available for the instrument
 * - the instrument is logging
 *
 * \param [in] conn the instrument connection
 * \param [in] twistActivation the twist activation parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the settings cannot be changed, or
 *                                 another hardware error occurs
 * \see RBRGen3_getTwistActivation()
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/twistactivation
 */
RBRGen3Error RBRGen3_setTwistActivation(
    RBRGen3 *conn,
    const RBRGen3TwistActivation *twistActivation);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN3GATING_H */
