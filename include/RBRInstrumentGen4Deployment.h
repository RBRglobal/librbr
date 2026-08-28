/**
 * \file RBRInstrumentGen4Deployment.h
 *
 * \brief Instrument commands and structures pertaining to deployments.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/deployments
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTGEN4DEPLOYMENT_H
#define LIBRBR_RBRINSTRUMENTGEN4DEPLOYMENT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Configuration.h"
/* Required for RBRInstrumentGen4InstrumentState. */
#include "RBRInstrumentGen4Instrument.h"

/**
 * \brief Instrument `clock` command parameters.
 *
 * \see RBRInstrumentGen4_getClock()
 * \see RBRInstrumentGen4_setClock()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830141/clock
 */
typedef struct RBRInstrumentGen4Clock
{
    /** \brief The instrument's date and time. */
    RBRInstrumentGen4DateTime dateTime;

    /**
     * \brief The offset of the instrument's date and time from UTC.
     *
     * Specified in hours. Fractional offsets are accepted.
     */
    float offsetFromUtc;
} RBRInstrumentGen4Clock;

/**
 * \brief Get the instrument clock.
 * \note Issues the `clock` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] clock the clock value
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_setClock()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830141/clock
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getClock(RBRInstrumentGen4 *instrument,
                                                 RBRInstrumentGen4Clock *clock);

/**
 * \brief Set the instrument clock.
 * \note Issues the `clock` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] clock the clock value
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the date and time is
 *         outside #RBRINSTRUMENTGEN4_DATETIME_MIN to
 *         #RBRINSTRUMENTGEN4_DATETIME_MAX, or the UTC offset is `NAN`
 * \see RBRInstrumentGen4_getClock()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830141/clock
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setClock(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Clock *clock);

/**
 * \brief Possible deployment statuses.
 *
 * \see RBRInstrumentGen4InstrumentState
 * \see RBRInstrumentGen4Deployment
 * \see RBRInstrumentGen4_getDeployment()
 * \see RBRInstrumentGen4_pause()
 * \see RBRInstrumentGen4_resume()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828403/deployment
 */
typedef enum RBRInstrumentGen4DeploymentStatus
{
    /** The deployment is sampling. */
    RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_SAMPLING,
    /** The deployment is waiting on its gating condition. */
    RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_GATED,
    /** The deployment is paused. */
    RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_PAUSED,
    /** No deployment is active. */
    RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_INACTIVE,
    /** The number of specific deployment statuses. */
    RBRINSTRUMENTGEN4_DEPLOYMENT_STATUS_COUNT,
    /** An unknown or unrecognized deployment status. */
    RBRINSTRUMENTGEN4_UNKNOWN_DEPLOYMENT_STATUS
} RBRInstrumentGen4DeploymentStatus;

/**
 * \brief Get a human-readable string name for a deployment status.
 *
 * \param [in] status the deployment status
 * \return a string name for the deployment status
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4DeploymentStatus_name(
    RBRInstrumentGen4DeploymentStatus status);

/**
 * \brief Possible deployment gating conditions.
 *
 * \see RBRInstrumentGen4Deployment
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828403/deployment
 */
typedef enum RBRInstrumentGen4Gate
{
    /** No gating condition. */
    RBRINSTRUMENTGEN4_GATE_NONE,
    /** Gated on the deployment start time. */
    RBRINSTRUMENTGEN4_GATE_TIME,
    /** Gated on the end cap position. */
    RBRINSTRUMENTGEN4_GATE_TWISTACTIVATION,
    /** Gated on the instrument being in the water. */
    RBRINSTRUMENTGEN4_GATE_WETSWITCH,
    /** The number of specific gating conditions. */
    RBRINSTRUMENTGEN4_GATE_COUNT,
    /** An unknown or unrecognized gating condition. */
    RBRINSTRUMENTGEN4_UNKNOWN_GATE
} RBRInstrumentGen4Gate;

/**
 * \brief Get a human-readable string name for a gating condition.
 *
 * \param [in] gate the gating condition
 * \return a string name for the gating condition
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4Gate_name(RBRInstrumentGen4Gate gate);

/**
 * \brief Instrument `deployment` command parameters.
 *
 * \see RBRInstrumentGen4_getDeployment()
 * \see RBRInstrumentGen4_setDeployment()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828403/deployment
 */
typedef struct RBRInstrumentGen4Deployment
{
    /**
     * \brief The start date and time of the next deployment.
     *
     * Only available while RBRInstrumentGen4Deployment.gate is
     * #RBRINSTRUMENTGEN4_GATE_TIME.
     */
    RBRInstrumentGen4DateTime startTime;

    /**
     * \brief The deployment status.
     *
     * \readonly
     */
    const RBRInstrumentGen4DeploymentStatus status;

    /** \brief The gating condition of the next deployment. */
    RBRInstrumentGen4Gate gate;

    /**
     * \brief Whether any of the instrument's channels are being simulated.
     *
     * \readonly
     */
    const bool simulation;
} RBRInstrumentGen4Deployment;

/**
 * \brief Get the instrument deployment parameters.
 * \note Issues the `deployment` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] deployment the deployment parameters
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the deployment is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_setDeployment()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828403/deployment
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getDeployment(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Deployment *deployment);

/**
 * \brief Set the instrument deployment parameters.
 * \note Issues the `deployment` instrument command.
 *
 * RBRInstrumentGen4Deployment.startTime is sent only when
 * RBRInstrumentGen4Deployment.gate is #RBRINSTRUMENTGEN4_GATE_TIME.
 * RBRInstrumentGen4Deployment.status and
 * RBRInstrumentGen4Deployment.simulation are never sent.
 *
 * \param [in] instrument the instrument connection
 * \param [in] deployment the deployment parameters
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the deployment is successfully
 *         changed
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the deployment cannot be
 *         changed
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the gating condition
 *         is set to more than one condition, or the start time is being sent 
 *         and is outside #RBRINSTRUMENTGEN4_DATETIME_MIN to 
 *         #RBRINSTRUMENTGEN4_DATETIME_MAX
 * \see RBRInstrumentGen4_getDeployment()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828403/deployment
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setDeployment(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Deployment *deployment);

/**
 * \brief Pause an enabled deployment.
 * \note Issues the `pause` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] status the deployment status; untouched unless the command
 *                     succeeds
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the deployment is paused
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the instrument is not
 *         enabled
 * \see RBRInstrumentGen4_resume()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828461/pause
 */
RBRInstrumentGen4Error RBRInstrumentGen4_pause(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4DeploymentStatus *status);

/**
 * \brief Resume a paused deployment.
 * \note Issues the `resume` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] status the deployment status; untouched unless the command
 *                     succeeds
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the deployment is resumed
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the instrument is not
 *         enabled
 * \see RBRInstrumentGen4_pause()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828463/resume
 */
RBRInstrumentGen4Error RBRInstrumentGen4_resume(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4DeploymentStatus *status);

/**
 * \brief Possible data storage modes for a deployment.
 *
 * \see RBRInstrumentGen4_verify()
 * \see RBRInstrumentGen4_enable()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828476/enable
 */
typedef enum RBRInstrumentGen4DeploymentStoragemode
{
    /** Calibration equations are applied to all channel data. */
    RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL,
    /** Calibration equations are not applied. */
    RBRINSTRUMENTGEN4_STORAGEMODE_CALIBRATION,
    /** The number of specific storage modes. */
    RBRINSTRUMENTGEN4_STORAGEMODE_COUNT,
    /** An unknown or unrecognized storage mode. */
    RBRINSTRUMENTGEN4_UNKNOWN_STORAGEMODE,
} RBRInstrumentGen4DeploymentStoragemode;

/**
 * \brief Get a human-readable string name for a deployment storageMode.
 *
 * \param [in] storageMode the deployment storage mode
 * \return a string name for the deployment storage mode
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4DeploymentStoragemode_name(
    RBRInstrumentGen4DeploymentStoragemode storageMode);

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
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the checks all pass
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when a check fails
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the configuration or
 *         dataset label is empty or too long, or the storage mode is not a
 *         specific mode
 * \see RBRInstrumentGen4_enable()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828472/verify
 */
RBRInstrumentGen4Error RBRInstrumentGen4_verify(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Config *config,
    const char *datasetLabel,
    RBRInstrumentGen4DeploymentStoragemode storageMode,
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
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the instrument is enabled
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the instrument cannot be
 *         enabled
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the configuration or
 *         dataset label is empty or too long, or the storage mode is not a
 *         specific mode
 * \see RBRInstrumentGen4_verify()
 * \see RBRInstrumentGen4_disable()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828476/enable
 */
RBRInstrumentGen4Error RBRInstrumentGen4_enable(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Config *config,
    const char *datasetLabel,
    RBRInstrumentGen4DeploymentStoragemode storageMode,
    RBRInstrumentGen4InstrumentState *state);

/**
 * \brief Terminate the current deployment.
 * \note Issues the `disable` instrument command.
 *
 * A warning from the instrument is reported as
 * #RBRINSTRUMENTGEN4_HARDWARE_ERROR with the response type set to
 * #RBRINSTRUMENTGEN4_RESPONSE_WARNING.
 *
 * \param [in] instrument the instrument connection
 * \param [out] state the state of the instrument; untouched unless the command
 *                    succeeds
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the deployment is terminated
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the instrument was not enabled
 * \see RBRInstrumentGen4_enable()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828481/disable
 */
RBRInstrumentGen4Error RBRInstrumentGen4_disable(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4InstrumentState *state);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTDEPLOYMENT_H */
