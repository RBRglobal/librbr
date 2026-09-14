/**
 * \file RBRGen4Deployment.h
 *
 * \brief Instrument commands and structures pertaining to deployments.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/deployments
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRGEN4DEPLOYMENT_H
#define LIBRBR_RBRGEN4DEPLOYMENT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "RBRGen4.h"
#include "RBRGen4Configuration.h"
/* Required for RBRInstrumentGen4InstrumentState. */
#include "RBRInstrumentGen4Instrument.h"

/**
 * \brief Instrument `clock` command parameters.
 *
 * \see RBRGen4_getClock()
 * \see RBRGen4_setClock()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830141/clock
 */
typedef struct RBRGen4Clock
{
    /** \brief The instrument's date and time. */
    RBRGen4DateTime dateTime;

    /**
     * \brief The offset of the instrument's date and time from UTC.
     *
     * Specified in hours. Fractional offsets are accepted.
     */
    float offsetFromUtc;
} RBRGen4Clock;

/**
 * \brief Get the instrument clock.
 * \note Issues the `clock` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] clock the clock value
 * \return #RBRGEN4_SUCCESS when the settings are successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRGen4_setClock()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830141/clock
 */
RBRGen4Error RBRGen4_getClock(RBRGen4 *instrument,
                                                 RBRGen4Clock *clock);

/**
 * \brief Set the instrument clock.
 * \note Issues the `clock` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] clock the clock value
 * \return #RBRGEN4_SUCCESS when the settings are successfully written
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the date and time is
 *         outside #RBRGEN4_DATETIME_MIN to
 *         #RBRGEN4_DATETIME_MAX, or the UTC offset is `NAN`
 * \see RBRGen4_getClock()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830141/clock
 */
RBRGen4Error RBRGen4_setClock(
    RBRGen4 *instrument,
    const RBRGen4Clock *clock);

/**
 * \brief Possible deployment statuses.
 *
 * \see RBRInstrumentGen4InstrumentState
 * \see RBRGen4Deployment
 * \see RBRGen4_getDeployment()
 * \see RBRGen4_pause()
 * \see RBRGen4_resume()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828403/deployment
 */
typedef enum RBRGen4DeploymentStatus
{
    /** The deployment is sampling. */
    RBRGEN4_DEPLOYMENT_STATUS_SAMPLING,
    /** The deployment is waiting on its gating condition. */
    RBRGEN4_DEPLOYMENT_STATUS_GATED,
    /** The deployment is paused. */
    RBRGEN4_DEPLOYMENT_STATUS_PAUSED,
    /** No deployment is active. */
    RBRGEN4_DEPLOYMENT_STATUS_INACTIVE,
    /** The number of specific deployment statuses. */
    RBRGEN4_DEPLOYMENT_STATUS_COUNT,
    /** An unknown or unrecognized deployment status. */
    RBRGEN4_UNKNOWN_DEPLOYMENT_STATUS
} RBRGen4DeploymentStatus;

/**
 * \brief Get a human-readable string name for a deployment status.
 *
 * \param [in] status the deployment status
 * \return a string name for the deployment status
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4DeploymentStatus_name(
    RBRGen4DeploymentStatus status);

/**
 * \brief Possible deployment gating conditions.
 *
 * \see RBRGen4Deployment
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828403/deployment
 */
typedef enum RBRGen4Gate
{
    /** No gating condition. */
    RBRGEN4_GATE_NONE,
    /** Gated on the deployment start time. */
    RBRGEN4_GATE_TIME,
    /** Gated on the end cap position. */
    RBRGEN4_GATE_TWISTACTIVATION,
    /** Gated on the instrument being in the water. */
    RBRGEN4_GATE_WETSWITCH,
    /** The number of specific gating conditions. */
    RBRGEN4_GATE_COUNT,
    /** An unknown or unrecognized gating condition. */
    RBRGEN4_UNKNOWN_GATE
} RBRGen4Gate;

/**
 * \brief Get a human-readable string name for a gating condition.
 *
 * \param [in] gate the gating condition
 * \return a string name for the gating condition
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4Gate_name(RBRGen4Gate gate);

/**
 * \brief Instrument `deployment` command parameters.
 *
 * \see RBRGen4_getDeployment()
 * \see RBRGen4_setDeployment()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828403/deployment
 */
typedef struct RBRGen4Deployment
{
    /**
     * \brief The start date and time of the next deployment.
     *
     * Only available while RBRGen4Deployment.gate is
     * #RBRGEN4_GATE_TIME.
     */
    RBRGen4DateTime startTime;

    /**
     * \brief The deployment status.
     *
     * \readonly
     */
    RBRGen4DeploymentStatus status;

    /** \brief The gating condition of the next deployment. */
    RBRGen4Gate gate;

    /**
     * \brief Whether any of the instrument's channels are being simulated.
     *
     * \readonly
     */
    bool simulation;
} RBRGen4Deployment;

/**
 * \brief Get the instrument deployment parameters.
 * \note Issues the `deployment` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] deployment the deployment parameters
 * \return #RBRGEN4_SUCCESS when the deployment is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRGen4_setDeployment()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828403/deployment
 */
RBRGen4Error RBRGen4_getDeployment(
    RBRGen4 *instrument,
    RBRGen4Deployment *deployment);

/**
 * \brief Set the instrument deployment parameters.
 * \note Issues the `deployment` instrument command.
 *
 * RBRGen4Deployment.startTime is sent only when
 * RBRGen4Deployment.gate is #RBRGEN4_GATE_TIME.
 * RBRGen4Deployment.status and
 * RBRGen4Deployment.simulation are never sent.
 *
 * \param [in] instrument the instrument connection
 * \param [in] deployment the deployment parameters
 * \return #RBRGEN4_SUCCESS when the deployment is successfully
 *         changed
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the deployment cannot be
 *         changed
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the gating condition
 *         is set to more than one condition, or the start time is being sent 
 *         and is outside #RBRGEN4_DATETIME_MIN to 
 *         #RBRGEN4_DATETIME_MAX
 * \see RBRGen4_getDeployment()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828403/deployment
 */
RBRGen4Error RBRGen4_setDeployment(
    RBRGen4 *instrument,
    const RBRGen4Deployment *deployment);

/**
 * \brief Pause an enabled deployment.
 * \note Issues the `pause` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] status the deployment status; untouched unless the command
 *                     succeeds
 * \return #RBRGEN4_SUCCESS when the deployment is paused
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument is not
 *         enabled
 * \see RBRGen4_resume()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828461/pause
 */
RBRGen4Error RBRGen4_pause(
    RBRGen4 *instrument,
    RBRGen4DeploymentStatus *status);

/**
 * \brief Resume a paused deployment.
 * \note Issues the `resume` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] status the deployment status; untouched unless the command
 *                     succeeds
 * \return #RBRGEN4_SUCCESS when the deployment is resumed
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument is not
 *         enabled
 * \see RBRGen4_pause()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828463/resume
 */
RBRGen4Error RBRGen4_resume(
    RBRGen4 *instrument,
    RBRGen4DeploymentStatus *status);

/**
 * \brief Possible data storage modes for a deployment.
 *
 * \see RBRGen4_verify()
 * \see RBRGen4_enable()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828476/enable
 */
typedef enum RBRGen4DeploymentStoragemode
{
    /** Calibration equations are applied to all channel data. */
    RBRGEN4_STORAGEMODE_NORMAL,
    /** Calibration equations are not applied. */
    RBRGEN4_STORAGEMODE_CALIBRATION,
    /** The number of specific storage modes. */
    RBRGEN4_STORAGEMODE_COUNT,
    /** An unknown or unrecognized storage mode. */
    RBRGEN4_UNKNOWN_STORAGEMODE,
} RBRGen4DeploymentStoragemode;

/**
 * \brief Get a human-readable string name for a deployment storageMode.
 *
 * \param [in] storageMode the deployment storage mode
 * \return a string name for the deployment storage mode
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4DeploymentStoragemode_name(
    RBRGen4DeploymentStoragemode storageMode);

/**
 * \brief Perform the deployment consistency checks of the `enable` command
 * without enabling the instrument (a dry run).
 * \note Issues the `verify` instrument command.
 *
 * All three parameters of the command are sent.
 *
 * \param [in] instrument the instrument connection
 * \param [in] config the configuration which would define this deployment
 * \param [in] datasetLabel the label which would be given to the deployment's
 *                          dataset
 * \param [in] storageMode the data storage mode which would be used
 * \param [out] state the state the instrument would assume; untouched unless
 *                    the command succeeds
 * \return #RBRGEN4_SUCCESS when the checks all pass
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when a check fails
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the configuration or
 *         dataset label is empty or too long, or the storage mode is not a
 *         specific mode
 * \see RBRGen4_enable()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828472/verify
 */
RBRGen4Error RBRGen4_verify(
    RBRGen4 *instrument,
    const RBRGen4Config *config,
    const char *datasetLabel,
    RBRGen4DeploymentStoragemode storageMode,
    RBRInstrumentGen4InstrumentState *state);

/**
 * \brief Enable the instrument to sample for a new deployment.
 * \note Issues the `enable` instrument command.
 *
 * All three parameters of the command are sent. The command reports no
 * dataset, so read the deployment's dataset back with
 * RBRInstrumentGen4_getDatasetPool().
 *
 * \param [in] instrument the instrument connection
 * \param [in] config the configuration which defines this deployment
 * \param [in] datasetLabel the label for the deployment's dataset
 * \param [in] storageMode the data storage mode for this deployment
 * \param [out] state the state of the instrument; untouched unless the command
 *                    succeeds
 * \return #RBRGEN4_SUCCESS when the instrument is enabled
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument cannot be
 *         enabled
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the configuration or
 *         dataset label is empty or too long, or the storage mode is not a
 *         specific mode
 * \see RBRGen4_verify()
 * \see RBRGen4_disable()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828476/enable
 */
RBRGen4Error RBRGen4_enable(
    RBRGen4 *instrument,
    const RBRGen4Config *config,
    const char *datasetLabel,
    RBRGen4DeploymentStoragemode storageMode,
    RBRInstrumentGen4InstrumentState *state);

/**
 * \brief Terminate the current deployment.
 * \note Issues the `disable` instrument command.
 *
 * A warning from the instrument is reported as
 * #RBRGEN4_HARDWARE_ERROR with the response type set to
 * #RBRGEN4_RESPONSE_WARNING.
 *
 * \param [in] instrument the instrument connection
 * \param [out] state the state of the instrument; untouched unless the command
 *                    succeeds
 * \return #RBRGEN4_SUCCESS when the deployment is terminated
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument was not enabled
 * \see RBRGen4_enable()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828481/disable
 */
RBRGen4Error RBRGen4_disable(
    RBRGen4 *instrument,
    RBRInstrumentGen4InstrumentState *state);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN4DEPLOYMENT_H */
