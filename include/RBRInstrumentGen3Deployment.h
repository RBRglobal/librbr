/**
 * \file RBRInstrumentGen3Deployment.h
 *
 * \brief Instrument commands and structures pertaining to deployments.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/deployments
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTGEN3DEPLOYMENT_H
#define LIBRBR_RBRINSTRUMENTGEN3DEPLOYMENT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Perform a “dry run” of the `enable` command.
 *
 * A hardware error can be generated for a variety of reasons. See the `verify`
 * command documentation for a comprehensive list. In the event of a hardware
 * error, \a status will be set to #RBRINSTRUMENTGEN3_UNKNOWN_STATUS. While Logger2
 * hardware reports a status in addition to any error, Logger3 hardware does
 * not, and the value will always be the same as the current instrument status.
 *
 * The \a eraseMemory parameter is ignored by Logger2 instruments.
 *
 * \param [in] instrument the instrument connection
 * \param [in] eraseMemory whether to erase memory before enabling logging
 * \param [out] status the status which would be produced by enabling logging
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR if an error would occur when enabling
 *                                       logging
 * \see RBRInstrumentGen3_enable()
 * \see https://docs.rbr-global.com/L3commandreference/commands/deployments/verify
 */
RBRGen3Error RBRInstrumentGen3_verify(
    RBRGen3 *instrument,
    bool eraseMemory,
    RBRInstrumentGen3DeploymentStatus *status);

/**
 * \brief Enable the instrument to sample according to the programmed schedule.
 *
 * If \a eraseMemory is not `true`, RBRInstrumentGen3_memoryClear() must be used to
 * erase the memory beforehand as necessary.
 *
 * A hardware error can be generated for a variety of reasons. See the `enable`
 * command documentation for a comprehensive list. In the event of a hardware
 * error, \a status will be set to #RBRINSTRUMENTGEN3_UNKNOWN_STATUS.
 *
 * \param [in] instrument the instrument connection
 * \param [in] eraseMemory whether to erase memory before enabling logging
 * \param [out] status the instrument's status after having enabled logging
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when an error occurs enabling logging
 * \see https://docs.rbr-global.com/L3commandreference/commands/deployments/enable
 */
RBRGen3Error RBRInstrumentGen3_enable(
    RBRGen3 *instrument,
    bool eraseMemory,
    RBRInstrumentGen3DeploymentStatus *status);

/**
 * \brief If the instrument is logging, terminate the current deployment.
 *
 * \param [in] instrument the instrument connection
 * \param [out] status the instrument's status after having disabled logging
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/deployments/disable
 */
RBRGen3Error RBRInstrumentGen3_disable(
    RBRGen3 *instrument,
    RBRInstrumentGen3DeploymentStatus *status);

/**
 * \brief Instrument `simulation` command parameters.
 *
 * \see RBRInstrumentGen3_getSimulation()
 * \see RBRInstrumentGen3_setSimulation()
 * \see https://docs.rbr-global.com/L3commandreference/commands/deployments/simulation
 */
typedef struct RBRInstrumentGen3Simulation
{
    /** Whether simulation is enabled. */
    bool state;
    /**
     * The period of each simulated profile.
     *
     * Specified in milliseconds. Must be greater than 0.
     */
    RBRGen3Period period;
} RBRInstrumentGen3Simulation;

/**
 * \brief Get the instrument simulation settings.
 *
 * \param [in] instrument the instrument connection
 * \param [out] simulation the simulation parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the feature is unavailable
 * \see https://docs.rbr-global.com/L3commandreference/commands/deployments/simulation
 */
RBRGen3Error RBRInstrumentGen3_getSimulation(
    RBRGen3 *instrument,
    RBRInstrumentGen3Simulation *simulation);

/**
 * \brief Set the instrument simulation settings.
 *
 * Hardware errors may occur if:
 *
 * - simulations are not available for the instrument (either due to its
 *   firmware version or channel configuration)
 * - the instrument is logging
 * - you set an out-of-bounds parameter the library fails to detect
 *
 * \param [in] instrument the instrument connection
 * \param [out] simulation the simulation parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when an out-of-bounds
 *                                                simulation period is
 *                                                requested
 * \see https://docs.rbr-global.com/L3commandreference/commands/deployments/simulation
 */
RBRGen3Error RBRInstrumentGen3_setSimulation(
    RBRGen3 *instrument,
    const RBRInstrumentGen3Simulation *simulation);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTGEN3DEPLOYMENT_H */
