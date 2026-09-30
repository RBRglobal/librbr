/*
 * Copyright (c) 2022 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file RBRGen3Pauseresume.h
 *
 * \brief Instrument commands and structures pertaining to pauseresume.
 * This feature is available in firmware versions 1.116 or later.
 */

#ifndef LIBRBR_RBRGEN3PAUSERESUME_H
#define LIBRBR_RBRGEN3PAUSERESUME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RBRGen3.h"

/** \brief The state of a pauseresume condition. */
typedef enum RBRGen3PauseresumeState {
    /** \brief The pauseresuming condition is disabled, or sampling mode is regimes. */
    RBRGEN3_PAUSERESUME_NA,
    /** \brief Deployment is enaled and paused. */
    RBRGEN3_PAUSERESUME_PAUSED,
    /** \brief Deployment is enabled and not paused. */
    RBRGEN3_PAUSERESUME_RUNNING,
    /** feature is not allowed, or firmware in use doesn't support this feature. */
    RBRGEN3_UNKNOWN_PAUSERESUME
} RBRGen3PauseresumeState;

/**
 * \brief Get a human-readable string name for a pauseresume state.
 *
 * \param [in] state the pauseresume state
 * \return a string name for the gating state
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRGen3PauseresumeState_name(RBRGen3PauseresumeState state);

/**
 * \brief Possible instrument pause status.
 *
 * \see RBRGen3Pause
 */
typedef enum RBRGen3PauseStatus {
    /** Deployment is paused and no more samples will be taken once the current acquisition
     * finishes. */
    RBRGEN3_PAUSE_PAUSED,
    /** An unknown or unrecognized pause status. */
    RBRGEN3_UNKNOWN_PAUSE
} RBRGen3PauseStatus;

/**
 * \brief Get a human-readable string name for a pause status.
 *
 * \param [in] status the pause status
 * \return a string name for the pause status
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRGen3PauseStatus_name(RBRGen3PauseStatus status);

/**
 * \brief Possible instrument resume status.
 *
 * \see RBRGen3Resume
 */
typedef enum RBRGen3ResumeStatus {
    /** Deployment has resumed running as scheduled. */
    RBRGEN3_RESUME_PENDING,
    RBRGEN3_RESUME_LOGGING,
    /** An unknown or unrecognized resume status. */
    RBRGEN3_UNKNOWN_RESUME
} RBRGen3ResumeStatus;

/**
 * \brief Get a human-readable string name for a resume status.
 *
 * \param [in] status the resume status
 * \return a string name for the resume status
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRGen3ResumeStatus_name(RBRGen3ResumeStatus status);

/**
 * It allows the host to determine if the pauseresume feature is available on
 * the instrument. It allows an elevated host to allow and deny the feature
 * for the instrument.
 *
 * \param [in] conn the instrument connection
 * \param [in, out] state the state of pauseresume
 * \return #RBRGEN3_SUCCESS when the state is one of the following:
 * "n/a", "paused", or "running".
 * \return #RBRGEN3_UNSUPPORTED when the current firmware doesn't support
 * pauseresume feature, or pauseresume is not allowed.
 * \return #RBRGEN3_HARDWARE_ERROR when the response indicates an error.
 */
RBRGen3Error RBRGen3_getPauseresume(RBRGen3 *conn, RBRGen3PauseresumeState *state);

/**
 * It pauses an enabled deloyment.
 *
 * \param [in] conn the instrument connection
 * \param [in, out] status the status of pause
 * \return #RBRGEN3_SUCCESS when the status is "paused".
 * \return #RBRGEN3_UNSUPPORTED when the current firmware doesn't support
 * pauseresume feature, or pauseresume is not allowed.
 * \return #RBRGEN3_HARDWARE_ERROR when the response indicates an error.
 */
RBRGen3Error RBRGen3_pause(RBRGen3 *conn, RBRGen3PauseStatus *status);
/**
 * It resumes an enabled deployment which was previously
 * paused using the pause command
 *
 * \param [in] conn the instrument connection
 * \param [in, out] status the status of resume
 * \return #RBRGEN3_SUCCESS when the state is one of the following:
 * "pending", "logging".
 * \return #RBRGEN3_UNSUPPORTED when the current firmware doesn't support
 * pauseresume feature, or pauseresume is not allowed.
 * \return #RBRGEN3_HARDWARE_ERROR when the response indicates an error.
 */
RBRGen3Error RBRGen3_resume(RBRGen3 *conn, RBRGen3ResumeStatus *status);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN3PAUSERESUME_H */
