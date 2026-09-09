/**
 * \file RBRInstrumentGating.h
 *
 * \brief Instrument commands and structures pertaining to gated sampling.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTGATING_H
#define LIBRBR_RBRINSTRUMENTGATING_H

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The state of a gating condition. */
typedef enum RBRInstrumentGatingState
{
    /** \brief The gating condition is disabled. */
    RBRINSTRUMENT_GATING_NA,
    /** \brief Logging is paused due to the gating. */
    RBRINSTRUMENT_GATING_PAUSED,
    /** \brief Logging is running due to the gating. */
    RBRINSTRUMENT_GATING_RUNNING,
    /** The number of specific gating condition types. */
    RBRINSTRUMENT_GATING_COUNT,
    /** An unknown or unrecognized gating condition type. */
    RBRINSTRUMENT_UNKNOWN_GATING
} RBRInstrumentGatingState;

/**
 * \brief Get a human-readable string name for a gating state.
 *
 * \param [in] state the gating state
 * \return a string name for the gating state
 * \see RBRInstrumentError_name() for a description of the format of names
 */
const char *RBRInstrumentGatingState_name(RBRInstrumentGatingState state);

/**
 * \brief Means of instrument thresholding channel selection.
 *
 * \see RBRInstrumentThresholding
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/thresholding
 */
typedef enum RBRInstrumentThresholdingChannelSelection
{
    /** The channel is set by index. */
    RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_INDEX,
    /** The channel is set by label. */
    RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_LABEL
} RBRInstrumentThresholdingChannelSelection;

/**
 * \brief Get a human-readable string name for a means of thresholding channel
 * selection.
 *
 * \param [in] selection the channel selection type
 * \return a string name for the channel selection type
 * \see RBRInstrumentError_name() for a description of the format of names
 */
const char *RBRInstrumentThresholdingChannelSelection_name(
    RBRInstrumentThresholdingChannelSelection selection);

/**
 * \brief Possible instrument thresholding conditions.
 *
 * \see RBRInstrumentThresholding
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/thresholding
 */
typedef enum RBRInstrumentThresholdingCondition
{
    /** Sampling occurs when the monitored parameter is above the threshold. */
    RBRINSTRUMENT_THRESHOLDING_ABOVE,
    /** Sampling occurs when the monitored parameter is below the threshold. */
    RBRINSTRUMENT_THRESHOLDING_BELOW,
    /** The number of thresholding conditions. */
    RBRINSTRUMENT_THRESHOLDING_COUNT,
    /** An unknown or unrecognized thresholding condition. */
    RBRINSTRUMENT_UNKNOWN_THRESHOLDING
} RBRInstrumentThresholdingCondition;

/**
 * \brief Get a human-readable string name for a thresholding condition.
 *
 * \param [in] condition the thresholding condition
 * \return a string name for the thresholding condition
 * \see RBRInstrumentError_name() for a description of the format of names
 */
const char *RBRInstrumentThresholdingCondition_name(
    RBRInstrumentThresholdingCondition condition);

/**
 * \brief Instrument `thresholding` command parameters.
 *
 * \see RBRInstrument_getThresholding()
 * \see RBRInstrument_setThresholding()
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/thresholding
 */
typedef struct RBRInstrumentThresholding
{
    /** \brief Enables or disables thresholding. */
    bool enabled;
    /**
     * \brief The state of logging based on the thresholding configuration.
     *
     * \readonly
     *
     * \nol2 Will always be retrieved as #RBRINSTRUMENT_UNKNOWN_GATING.
     */
    const RBRInstrumentGatingState state;
    /**
     * \brief Whether the thresholding channel should be configured by index or
     * by label.
     *
     * \writeonly It is set to #RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_INDEX when
     * retrieving parameters. For Logger2 instruments, which do not have
     * channel labels, this must be given as
     * #RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_INDEX or
     * RBRInstrument_setThresholding() will return
     * #RBRINSTRUMENT_INVALID_PARAMETER_VALUE.
     *
     * \see RBRInstrument_setThresholding()
     */
    RBRInstrumentThresholdingChannelSelection channelSelection;
    /**
     * \brief The index of the channel to use for the threshold check.
     *
     * When setting the instrument thresholding channel, the value of this
     * field is only used when RBRInstrumentThresholding.channelSelection is
     * set to #RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_INDEX.
     */
    RBRInstrumentChannelIndex channelIndex;
    /**
     * \brief The label of the channel to use for the threshold check.
     *
     * When setting the thresholding channel by label, should be provided as a
     * null-terminated C string. The value will be populated likewise when
     * retrieving thresholding parameters from the instrument.
     *
     * When setting the instrument thresholding channel, the value of this
     * field is only used when RBRInstrumentThresholding.channelSelection is
     * set to #RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_LABEL.
     *
     * \nol2 Use RBRInstrumentThresholding.channelIndex instead.
     */
    char channelLabel[RBRINSTRUMENT_CHANNEL_LABEL_MAX + 1];
    /** \brief Specifies the condition under which sampling will occur. */
    RBRInstrumentThresholdingCondition condition;
    /** \brief The threshold value in calibrated units. */
    float value;
    /**
     * \brief The interval between threshold checks.
     *
     * The value is given and reported in milliseconds, but must correspond to
     * a non-zero whole number of seconds (i.e., must be divisible by 1,000)
     * and must not be greater than 86,400,000 (24 hours).
     */
    RBRInstrumentPeriod interval;
} RBRInstrumentThresholding;

/**
 * \brief Get the instrument thresholding settings.
 *
 * \param [in] instrument the instrument connection
 * \param [out] threshold the thresholding parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the feature is unavailable
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/thresholding
 */
RBRInstrumentError RBRInstrument_getThresholding(
    RBRInstrument *instrument,
    RBRInstrumentThresholding *threshold);

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
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/thresholding
 */
RBRInstrumentError RBRInstrument_setThresholding(
    RBRInstrument *instrument,
    const RBRInstrumentThresholding *threshold);

/**
 * \brief Instrument `twistactivation` command parameters.
 *
 * \see RBRInstrument_getTwistActivation()
 * \see RBRInstrument_setTwistActivation()
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/twistactivation
 */
typedef struct RBRInstrumentTwistActivation
{
    /** \brief Enables or disables twist activation. */
    bool enabled;
    /**
     * \brief The state of logging based on the twist activation configuration.
     *
     * \readonly
     *
     * \nol2 Will always be retrieved as #RBRINSTRUMENT_UNKNOWN_GATING.
     */
    const RBRInstrumentGatingState state;
} RBRInstrumentTwistActivation;

/**
 * \brief Get the instrument twist activation settings.
 *
 * \param [in] instrument the instrument connection
 * \param [out] twistActivation the twist activation parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the feature is unavailable
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/twistactivation
 */
RBRInstrumentError RBRInstrument_getTwistActivation(
    RBRInstrument *instrument,
    RBRInstrumentTwistActivation *twistActivation);

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
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the settings cannot be changed
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling/twistactivation
 */
RBRInstrumentError RBRInstrument_setTwistActivation(
    RBRInstrument *instrument,
    const RBRInstrumentTwistActivation *twistActivation);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTGATING_H */
