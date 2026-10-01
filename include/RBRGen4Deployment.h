/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file RBRGen4Deployment.h
 *
 * \brief Instrument commands and structures pertaining to deployments.
 */

#ifndef LIBRBR_RBRGEN4DEPLOYMENT_H
#define LIBRBR_RBRGEN4DEPLOYMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RBRGen4.h"
/* Required for RBRGen4Config. */
#include "RBRGen4Configuration.h"
/* Required for RBRGen4InstrumentState. */
#include "RBRGen4Instrument.h"

/**
 * \brief Instrument `clock` command parameters.
 *
 * \see RBRGen4_getClock()
 * \see RBRGen4_setClock()
 */
typedef struct RBRGen4Clock {
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
 *
 * \command{clock}
 *
 * \param [in] conn the instrument connection
 * \param [out] clock the clock value
 * \return #RBRGEN4_SUCCESS when the settings are successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument reports a hardware error
 * \see RBRGen4_setClock()
 */
RBRGen4Error RBRGen4_getClock(RBRGen4 *conn, RBRGen4Clock *clock);

/**
 * \brief Set the instrument clock.
 *
 * \command{clock}
 *
 * \param [in] conn the instrument connection
 * \param [in] clock the clock value
 * \return #RBRGEN4_SUCCESS when the settings are successfully written
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the settings cannot be changed, or another hardware error
 *         occurs
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the date and time is outside #RBRGEN4_DATETIME_MIN
 *         to #RBRGEN4_DATETIME_MAX, or the UTC offset is `NAN`
 * \see RBRGen4_getClock()
 */
RBRGen4Error RBRGen4_setClock(RBRGen4 *conn, const RBRGen4Clock *clock);

/**
 * \brief Possible deployment statuses.
 *
 * \see RBRGen4InstrumentState
 * \see RBRGen4Deployment
 * \see RBRGen4_getDeployment()
 * \see RBRGen4_pause()
 * \see RBRGen4_resume()
 */
typedef enum RBRGen4DeploymentStatus {
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
    RBRGEN4_UNKNOWN_DEPLOYMENT_STATUS,
} RBRGen4DeploymentStatus;

/**
 * \brief Get a human-readable string name for a deployment status.
 *
 * \param [in] status the deployment status
 * \return a string name for the deployment status
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4DeploymentStatus_name(RBRGen4DeploymentStatus status);

/**
 * \brief Possible deployment gating conditions.
 *
 * \see RBRGen4Deployment
 */
typedef enum RBRGen4DeploymentGate {
    /** No gating condition. */
    RBRGEN4_DEPLOYMENT_GATE_NONE,
    /** Gated on the deployment start time. */
    RBRGEN4_DEPLOYMENT_GATE_TIME,
    /** Gated on the end cap position. */
    RBRGEN4_DEPLOYMENT_GATE_TWISTACTIVATION,
    /** Gated on the instrument being in the water. */
    RBRGEN4_DEPLOYMENT_GATE_WETSWITCH,
    /** The number of specific gating conditions. */
    RBRGEN4_DEPLOYMENT_GATE_COUNT,
    /** An unknown or unrecognized gating condition. */
    RBRGEN4_UNKNOWN_DEPLOYMENT_GATE,
} RBRGen4DeploymentGate;

/**
 * \brief Get a human-readable string name for a gating condition.
 *
 * \param [in] gate the gating condition
 * \return a string name for the gating condition
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4DeploymentGate_name(RBRGen4DeploymentGate gate);

/**
 * \brief Instrument `deployment` command parameters.
 *
 * \see RBRGen4_getDeployment()
 * \see RBRGen4_setDeployment()
 */
typedef struct RBRGen4Deployment {
    /**
     * \brief The start date and time of the next deployment.
     *
     * Only available while RBRGen4Deployment.gate is
     * #RBRGEN4_DEPLOYMENT_GATE_TIME.
     */
    RBRGen4DateTime startTime;

    /**
     * \brief The deployment status.
     *
     * \readonly
     */
    RBRGen4DeploymentStatus status;

    /** \brief The gating condition of the next deployment. */
    RBRGen4DeploymentGate gate;

    /**
     * \brief Whether any of the instrument's channels are being simulated.
     *
     * \readonly
     */
    bool simulation;
} RBRGen4Deployment;

/**
 * \brief Get the instrument deployment parameters.
 *
 * \command{deployment}
 *
 * \param [in] conn the instrument connection
 * \param [out] deployment the deployment parameters
 * \return #RBRGEN4_SUCCESS when the deployment is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument reports a hardware error
 * \see RBRGen4_setDeployment()
 */
RBRGen4Error RBRGen4_getDeployment(RBRGen4 *conn, RBRGen4Deployment *deployment);

/**
 * \brief Set the instrument deployment parameters.
 *
 * \command{deployment}
 *
 * RBRGen4Deployment.startTime is sent only when
 * RBRGen4Deployment.gate is #RBRGEN4_DEPLOYMENT_GATE_TIME.
 * RBRGen4Deployment.status and
 * RBRGen4Deployment.simulation are never sent.
 *
 * \param [in] conn the instrument connection
 * \param [in] deployment the deployment parameters
 * \return #RBRGEN4_SUCCESS when the deployment is successfully changed
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the deployment cannot be changed, or another hardware error
 *         occurs
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the gating condition is set to more than one
 *         condition, or the start time is being sent and is outside #RBRGEN4_DATETIME_MIN to
 *         #RBRGEN4_DATETIME_MAX
 * \see RBRGen4_getDeployment()
 */
RBRGen4Error RBRGen4_setDeployment(RBRGen4 *conn, const RBRGen4Deployment *deployment);

/**
 * \brief Pause an enabled deployment.
 *
 * \command{pause}
 *
 * \param [in] conn the instrument connection
 * \param [out] status the deployment status; untouched unless the command succeeds
 * \return #RBRGEN4_SUCCESS when the deployment is paused
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument is not enabled, or another hardware error
 *         occurs
 * \see RBRGen4_resume()
 */
RBRGen4Error RBRGen4_pause(RBRGen4 *conn, RBRGen4DeploymentStatus *status);

/**
 * \brief Resume a paused deployment.
 *
 * \command{resume}
 *
 * \param [in] conn the instrument connection
 * \param [out] status the deployment status; untouched unless the command succeeds
 * \return #RBRGEN4_SUCCESS when the deployment is resumed
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument is not enabled, or another hardware error
 *         occurs
 * \see RBRGen4_pause()
 */
RBRGen4Error RBRGen4_resume(RBRGen4 *conn, RBRGen4DeploymentStatus *status);

/**
 * \brief Possible data storage modes for a deployment.
 *
 * \see RBRGen4_verify()
 * \see RBRGen4_enable()
 */
typedef enum RBRGen4DeploymentStorageMode {
    /** Calibration equations are applied to all channel data. */
    RBRGEN4_STORAGE_MODE_NORMAL,
    /** Calibration equations are not applied. */
    RBRGEN4_STORAGE_MODE_CALIBRATION,
    /** The number of specific storage modes. */
    RBRGEN4_STORAGE_MODE_COUNT,
    /** An unknown or unrecognized storage mode. */
    RBRGEN4_UNKNOWN_STORAGE_MODE,
} RBRGen4DeploymentStorageMode;

/**
 * \brief Get a human-readable string name for a deployment storageMode.
 *
 * \param [in] storageMode the deployment storage mode
 * \return a string name for the deployment storage mode
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4DeploymentStorageMode_name(RBRGen4DeploymentStorageMode storageMode);

/**
 * \brief Perform the deployment consistency checks of the `enable` command
 * without enabling the instrument (a dry run).
 *
 * \command{verify}
 *
 * A `NULL` \a datasetLabel leaves the `dataset` parameter out. Only an
 * instrument which does not store data accepts that; one which does needs
 * a label even when no schedule stores its data.
 *
 * \param [in] conn the instrument connection
 * \param [in] config the configuration which would define this deployment
 * \param [in] datasetLabel the label which would be given to the deployment's dataset, or `NULL`
 * \param [in] storageMode the data storage mode which would be used
 * \param [out] state the state the instrument would assume; untouched unless the command succeeds
 * \return #RBRGEN4_SUCCESS when the checks all pass
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when a check fails, or another hardware error occurs
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the configuration or a given dataset label is empty
 *         or too long, or the storage mode is not a specific mode
 * \see RBRGen4_enable()
 */
RBRGen4Error RBRGen4_verify(RBRGen4 *conn, const RBRGen4Config *config, const char *datasetLabel,
                            RBRGen4DeploymentStorageMode storageMode,
                            RBRGen4InstrumentState *state);

/**
 * \brief Enable the instrument to sample for a new deployment.
 *
 * \command{enable}
 *
 * A `NULL` \a datasetLabel leaves the `dataset` parameter out. Only an
 * instrument which does not store data accepts that; one which does needs
 * a label even when no schedule stores its data. The command reports no
 * dataset, so read the deployment's dataset back with
 * RBRGen4_getDatasetPool().
 *
 * \param [in] conn the instrument connection
 * \param [in] config the configuration which defines this deployment
 * \param [in] datasetLabel the label for the deployment's dataset, or `NULL`
 * \param [in] storageMode the data storage mode for this deployment
 * \param [out] state the state of the instrument; untouched unless the command succeeds
 * \return #RBRGEN4_SUCCESS when the instrument is enabled
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument cannot be enabled, or another hardware error
 *         occurs
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the configuration or a given dataset label is empty
 *         or too long, or the storage mode is not a specific mode
 * \see RBRGen4_verify()
 * \see RBRGen4_disable()
 */
RBRGen4Error RBRGen4_enable(RBRGen4 *conn, const RBRGen4Config *config, const char *datasetLabel,
                            RBRGen4DeploymentStorageMode storageMode,
                            RBRGen4InstrumentState *state);

/**
 * \brief Terminate the current deployment.
 *
 * \command{disable}
 *
 * A warning from the instrument is reported as
 * #RBRGEN4_HARDWARE_ERROR with the response type set to
 * #RBRGEN4_RESPONSE_WARNING.
 *
 * \param [in] conn the instrument connection
 * \param [out] state the state of the instrument; untouched unless the command succeeds
 * \return #RBRGEN4_SUCCESS when the deployment is terminated
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument was not enabled, or another hardware error
 *         occurs
 * \see RBRGen4_enable()
 */
RBRGen4Error RBRGen4_disable(RBRGen4 *conn, RBRGen4InstrumentState *state);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN4DEPLOYMENT_H */
