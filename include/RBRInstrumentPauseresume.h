/**
 * \file RBRInstrumentPauseresume.h
 *
 * \brief Instrument commands and structures pertaining to pauseresume.
 * This feature is available in firmware versions 1.116 or later.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/pauseresume
 *
 * \copyright
 * Copyright (c) 2022 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTPAUSERESUME_H
#define LIBRBR_RBRINSTRUMENTPAUSERESUME_H

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The state of a pauseresume condition. */
typedef enum RBRInstrumentPauseresumeState
{
    /** \brief The pauseresuming condition is disabled, or sampling mode is regimes. */
    RBRINSTRUMENT_PAUSERESUME_NA,
    /** \brief Deployment is enaled and paused. */
    RBRINSTRUMENT_PAUSERESUME_PAUSED,
    /** \brief Deployment is enabled and not paused. */
    RBRINSTRUMENT_PAUSERESUME_RUNNING,
    /** feature is not allowed, or firmware in use doesn't support this feature. */
    RBRINSTRUMENT_UNKNOWN_PAUSERESUME
} RBRInstrumentPauseresumeState;

/**
 * \brief Get a human-readable string name for a pauseresume state.
 *
 * \param [in] state the pauseresume state
 * \return a string name for the gating state
 * \see RBRInstrumentError_name() for a description of the format of names
 */
const char *RBRInstrumentPauseresumeState_name(RBRInstrumentPauseresumeState state);

/**
 * \brief Possible instrument pause status.
 *
 * \see RBRInstrumentPause
 * \see https://docs.rbr-global.com/L3commandreference/commands/pause
 */
typedef enum RBRInstrumentPauseStatus
{
    /** Deployment is paused and no more samples will be taken once the current acquisition
     * finishes. */
    RBRINSTRUMENT_PAUSE_PAUSED,
    /** An unknown or unrecognized pause status. */
    RBRINSTRUMENT_UNKNOWN_PAUSE
} RBRInstrumentPauseStatus;

/**
 * \brief Get a human-readable string name for a pause status.
 *
 * \param [in] status the pause status
 * \return a string name for the pause status
 * \see RBRInstrumentError_name() for a description of the format of names
 */
const char *RBRInstrumentPauseStatus_name(RBRInstrumentPauseStatus status);

/**
 * \brief Possible instrument resume status.
 *
 * \see RBRInstrumentResume
 * \see https://docs.rbr-global.com/L3commandreference/commands/resume
 */
typedef enum RBRInstrumentResumeStatus
{
    /** Deployment has resumed running as scheduled. */
    RBRINSTRUMENT_RESUME_PENDING,
    RBRINSTRUMENT_RESUME_LOGGING,
    /** An unknown or unrecognized resume status. */
    RBRINSTRUMENT_UNKNOWN_RESUME
} RBRInstrumentResumeStatus;

/**
 * \brief Get a human-readable string name for a resume status.
 *
 * \param [in] status the resume status
 * \return a string name for the resume status
 * \see RBRInstrumentError_name() for a description of the format of names
 */
const char *RBRInstrumentResumeStatus_name(RBRInstrumentResumeStatus status);

/**
 * It allows the host to determine if the pauseresume feature is available on
 * the instrument. It allows an elevated host to allow and deny the feature
 * for the instrument.
 *
 * \param [in] instrument the instrument connection
 * \param [in, out] state the state of pauseresume
 * \return #RBRINSTRUMENT_SUCCESS when the state is one of the following:
 * "n/a", "paused", or "running".
 * \return #RBRINSTRUMENT_UNSUPPORTED when the current firmware doesn't support
 * pauseresume feature, or pauseresume is not allowed.
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the response indicates an error.
 */
RBRInstrumentError RBRInstrument_getPauseresume(RBRInstrument *instrument,
                                                RBRInstrumentPauseresumeState *state);

/**
 * It pauses an enabled deloyment.
 *
 * \param [in] instrument the instrument connection
 * \param [in, out] status the status of pause
 * \return #RBRINSTRUMENT_SUCCESS when the status is "paused".
 * \return #RBRINSTRUMENT_UNSUPPORTED when the current firmware doesn't support
 * pauseresume feature, or pauseresume is not allowed.
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the response indicates an error.
 */
RBRInstrumentError RBRInstrument_pause(RBRInstrument *instrument, RBRInstrumentPauseStatus *status);
/**
 * It resumes an enabled deployment which was previously
 * paused using the pause command
 *
 * \param [in] instrument the instrument connection
 * \param [in, out] status the status of resume
 * \return #RBRINSTRUMENT_SUCCESS when the state is one of the following:
 * "pending", "logging".
 * \return #RBRINSTRUMENT_UNSUPPORTED when the current firmware doesn't support
 * pauseresume feature, or pauseresume is not allowed.
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the response indicates an error.
 */
RBRInstrumentError RBRInstrument_resume(RBRInstrument *instrument,
                                        RBRInstrumentResumeStatus *status);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTGATING_H */
