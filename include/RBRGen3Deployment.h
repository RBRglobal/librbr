/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file RBRGen3Deployment.h
 *
 * \brief Instrument commands and structures pertaining to deployments.
 */

#ifndef LIBRBR_RBRGEN3DEPLOYMENT_H
#define LIBRBR_RBRGEN3DEPLOYMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RBRGen3.h"
/* Required for RBRGen3DeploymentStatus. */
#include "RBRGen3Schedule.h"

/**
 * \brief Perform a “dry run” of the `enable` command.
 *
 * \command{verify}
 *
 * A hardware error can be generated for a variety of reasons. See the `verify`
 * command documentation for a comprehensive list. In the event of a hardware
 * error, \a status will be set to #RBRGEN3_UNKNOWN_STATUS. While Logger2
 * hardware reports a status in addition to any error, Logger3 hardware does
 * not, and the value will always be the same as the current instrument status.
 *
 * The \a eraseMemory parameter is ignored by Logger2 instruments.
 *
 * \param [in] conn the instrument connection
 * \param [in] eraseMemory whether to erase memory before enabling logging
 * \param [out] status the status which would be produced by enabling logging
 * \return #RBRGEN3_SUCCESS when the status is successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when an error would occur when enabling logging, or another
 *         hardware error occurs
 * \see RBRGen3_enable()
 * \see RBRGen3DeploymentStatus
 */
RBRGen3Error RBRGen3_verify(RBRGen3 *conn, bool eraseMemory, RBRGen3DeploymentStatus *status);

/**
 * \brief Enable the instrument to sample according to the programmed schedule.
 *
 * \command{enable}
 *
 * If \a eraseMemory is not `true`, RBRGen3_memoryClear() must be used to
 * erase the memory beforehand as necessary.
 *
 * A hardware error can be generated for a variety of reasons. See the `enable`
 * command documentation for a comprehensive list. In the event of a hardware
 * error, \a status will be set to #RBRGEN3_UNKNOWN_STATUS.
 *
 * \param [in] conn the instrument connection
 * \param [in] eraseMemory whether to erase memory before enabling logging
 * \param [out] status the instrument's status after having enabled logging
 * \return #RBRGEN3_SUCCESS when logging is successfully enabled
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when an error occurs enabling logging, or another hardware error
 *         occurs
 * \see RBRGen3DeploymentStatus
 */
RBRGen3Error RBRGen3_enable(RBRGen3 *conn, bool eraseMemory, RBRGen3DeploymentStatus *status);

/**
 * \brief If the instrument is logging, terminate the current deployment.
 *
 * \par Command:
 * `disable`, or `stop` for Logger2
 *
 * \param [in] conn the instrument connection
 * \param [out] status the instrument's status after having disabled logging
 * \return #RBRGEN3_SUCCESS when logging is successfully disabled
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the instrument reports a hardware error
 * \see RBRGen3DeploymentStatus
 */
RBRGen3Error RBRGen3_disable(RBRGen3 *conn, RBRGen3DeploymentStatus *status);

/**
 * \brief Instrument `simulation` command parameters.
 *
 * \see RBRGen3_getSimulation()
 * \see RBRGen3_setSimulation()
 */
typedef struct RBRGen3Simulation {
    /** Whether simulation is enabled. */
    bool state;
    /**
     * The period of each simulated profile.
     *
     * Specified in milliseconds. Must be greater than 0.
     */
    RBRGen3Period period;
} RBRGen3Simulation;

/**
 * \brief Get the instrument simulation settings.
 *
 * \command{simulation}
 *
 * \param [in] conn the instrument connection
 * \param [out] simulation the simulation parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the feature is unavailable, or another hardware error occurs
 * \see RBRGen3_setSimulation()
 */
RBRGen3Error RBRGen3_getSimulation(RBRGen3 *conn, RBRGen3Simulation *simulation);

/**
 * \brief Set the instrument simulation settings.
 *
 * \command{permit,simulation}
 *
 * Hardware errors may occur if:
 *
 * - simulations are not available for the instrument (either due to its
 *   firmware version or channel configuration)
 * - the instrument is logging
 * - you set an out-of-bounds parameter the library fails to detect
 *
 * \param [in] conn the instrument connection
 * \param [in] simulation the simulation parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the settings cannot be changed, or another hardware error
 *         occurs
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when an out-of-bounds simulation period is requested
 * \see RBRGen3_getSimulation()
 */
RBRGen3Error RBRGen3_setSimulation(RBRGen3 *conn, const RBRGen3Simulation *simulation);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN3DEPLOYMENT_H */
