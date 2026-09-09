/**
 * \file RBRInstrumentGen3Pauseresume.h
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

#ifndef LIBRBR_RBRINSTRUMENTGEN3PAUSERESUME_H
#define LIBRBR_RBRINSTRUMENTGEN3PAUSERESUME_H

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The state of a pauseresume condition. */
typedef enum RBRInstrumentGen3PauseresumeState
{
    /** \brief The pauseresuming condition is disabled, or sampling mode is regimes. */
    RBRINSTRUMENTGEN3_PAUSERESUME_NA,
    /** \brief Deployment is enaled and paused. */
    RBRINSTRUMENTGEN3_PAUSERESUME_PAUSED,
    /** \brief Deployment is enabled and not paused. */
    RBRINSTRUMENTGEN3_PAUSERESUME_RUNNING,
    /** feature is not allowed, or firmware in use doesn't support this feature. */
    RBRINSTRUMENTGEN3_UNKNOWN_PAUSERESUME
} RBRInstrumentGen3PauseresumeState;

/**
 * \brief Get a human-readable string name for a pauseresume state.
 *
 * \param [in] state the pauseresume state
 * \return a string name for the gating state
 * \see RBRInstrumentGen3Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen3PauseresumeState_name(RBRInstrumentGen3PauseresumeState state);

/**
 * \brief Possible instrument pause status.
 *
 * \see RBRInstrumentGen3Pause
 * \see https://docs.rbr-global.com/L3commandreference/commands/pause
 */
typedef enum RBRInstrumentGen3PauseStatus
{
    /** Deployment is paused and no more samples will be taken once the current acquisition finishes. */
    RBRINSTRUMENTGEN3_PAUSE_PAUSED,
    /** An unknown or unrecognized pause status. */
    RBRINSTRUMENTGEN3_UNKNOWN_PAUSE
} RBRInstrumentGen3PauseStatus;

/**
 * \brief Get a human-readable string name for a pause status.
 *
 * \param [in] status the pause status
 * \return a string name for the pause status
 * \see RBRInstrumentGen3Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen3PauseStatus_name(
    RBRInstrumentGen3PauseStatus status);

/**
 * \brief Possible instrument resume status.
 *
 * \see RBRInstrumentGen3Resume
 * \see https://docs.rbr-global.com/L3commandreference/commands/resume
 */
typedef enum RBRInstrumentGen3ResumeStatus
{
    /** Deployment has resumed running as scheduled. */
    RBRINSTRUMENTGEN3_RESUME_PENDING,
    RBRINSTRUMENTGEN3_RESUME_LOGGING,
    /** An unknown or unrecognized resume status. */
    RBRINSTRUMENTGEN3_UNKNOWN_RESUME
} RBRInstrumentGen3ResumeStatus;

/**
 * \brief Get a human-readable string name for a resume status.
 *
 * \param [in] status the resume status
 * \return a string name for the resume status
 * \see RBRInstrumentGen3Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen3ResumeStatus_name(
    RBRInstrumentGen3ResumeStatus status);

/**
 * It allows the host to determine if the pauseresume feature is available on
 * the instrument. It allows an elevated host to allow and deny the feature
 * for the instrument.
 * 
 * \param [in] instrument the instrument connection
 * \param [in, out] state the state of pauseresume
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the state is one of the following:
 * "n/a", "paused", or "running".
 * \return #RBRINSTRUMENTGEN3_UNSUPPORTED when the current firmware doesn't support
 * pauseresume feature, or pauseresume is not allowed.
 * \return #RBRINSTRUMENTGEN3_HARDWARE_ERROR when the response indicates an error.
 */
RBRInstrumentGen3Error RBRInstrumentGen3_getPauseresume(RBRInstrumentGen3 *instrument,
                                       RBRInstrumentGen3PauseresumeState *state);

/**
 * It pauses an enabled deloyment.
 * 
 * \param [in] instrument the instrument connection
 * \param [in, out] status the status of pause
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the status is "paused".
 * \return #RBRINSTRUMENTGEN3_UNSUPPORTED when the current firmware doesn't support
 * pauseresume feature, or pauseresume is not allowed.
 * \return #RBRINSTRUMENTGEN3_HARDWARE_ERROR when the response indicates an error.
 */
RBRInstrumentGen3Error RBRInstrumentGen3_pause(RBRInstrumentGen3 *instrument,
                                       RBRInstrumentGen3PauseStatus *status);
/**
 * It resumes an enabled deployment which was previously
 * paused using the pause command
 * 
 * \param [in] instrument the instrument connection
 * \param [in, out] status the status of resume
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the state is one of the following:
 * "pending", "logging".
 * \return #RBRINSTRUMENTGEN3_UNSUPPORTED when the current firmware doesn't support
 * pauseresume feature, or pauseresume is not allowed.
 * \return #RBRINSTRUMENTGEN3_HARDWARE_ERROR when the response indicates an error.
 */
RBRInstrumentGen3Error RBRInstrumentGen3_resume(RBRInstrumentGen3 *instrument,
                                       RBRInstrumentGen3ResumeStatus *status);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTGEN3PAUSERESUME_H */
