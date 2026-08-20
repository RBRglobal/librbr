/**
 * \file RBRInstrumentGen4Schedule.h
 *
 * \brief Instrument commands and structures pertaining to time and schedule.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTGEN4SCHEDULE_H
#define LIBRBR_RBRINSTRUMENTGEN4SCHEDULE_H

#include "RBRInstrumentGen4Configuration.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Instrument `clock` command parameters.
 *
 * \see RBRInstrumentGen4_getClock()
 * \see RBRInstrumentGen4_setClock()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830141/clock
 */
typedef struct RBRInstrumentGen4Clock
{
    /**
     * \brief The instrument's date and time.
     */
    RBRInstrumentGen4DateTime dateTime;
    /**
     * \brief The offset of the instrument's date and time from UTC.
     *
     * Specified in hours.
     *
     * When passing a date and time to the instrument, a `NAN` value will cause
     * the `offsetfromutc` parameter to be omitted from the command sent to the
     * instrument; otherwise, the parameter will be sent as the string
     * representation of the number to two decimal places.
     *
     * When receiving a date and time from the instrument, a `NAN` value
     * indicates that an offset from UTC was not provided when the instrument
     * clock was most recently set. Otherwise, the value will correspond to the
     * instrument clock offset from UTC.
     */
    float offsetFromUtc;
} RBRInstrumentGen4Clock;

/**
 * \note Issues the `clock` instrument command.
 * \brief Get the instrument clock.
 *
 * Because UTC offset is tracked as a setting on Logger2 instruments, not as a
 * parameter of the `now` command (as it is of `clock` on Logger3), this
 * function will internally issue two commands to Logger2 instruments to
 * separately receive the time and UTC offset. When retrieving the clock from
 * older Logger2 instruments which do not support the `offsetfromutc` setting,
 * RBRInstrumentGen4Clock.offsetFromUtc will always be `NAN`.
 *
 * \param [in] instrument the instrument connection
 * \param [out] clock the clock value
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_setClock
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830141/clock
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getClock(RBRInstrumentGen4 *instrument,
                                          RBRInstrumentGen4Clock *clock);

/**
 * \brief Set the instrument clock.
 * \note Issues the `clock` instrument command.
 *
 * Because UTC offset is tracked as a setting on Logger2 instruments, not as a
 * parameter of the `now` command (as it is of `clock` on Logger3), this
 * function will internally issue two commands to Logger2 instruments to
 * separately set the time and UTC offset. When setting the clock on older
 * Logger2 instruments which do not support the `offsetfromutc` setting, the
 * value of the RBRInstrumentGen4Clock.offsetFromUtc field will be ignored.
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - you set an out-of-bounds time the library fails to detect
 *
 * \param [in] instrument the instrument connection
 * \param [in] clock the clock value
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the clock values are out
 *                                                of range
 * \see RBRInstrumentGen4_getClock
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830141/clock
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setClock(RBRInstrumentGen4 *instrument,
                                          const RBRInstrumentGen4Clock *clock);

/**
 * \brief Possible instrument instrument states.
 * This tracks whether the deployment is running on the instrument.
 *
 * Returned by:
 * \see RBRInstrumentGen4_getInstrument()
 * \see RBRInstrumentGen4_enable()
 * \see RBRInstrumentGen4_verify()
 *
 * For the deployment state:
 * \see RBRInstrumentGen4DeploymentStatus
 * \see RBRInstrumentGen4Deployment
 */
typedef enum RBRInstrumentGen4InstrumentState
{
    /** Logging is not enabled. */
    RBRINSTRUMENTGEN4_INSTRUMENT_STATE_DISABLED,
    /** Logging for at least one deployment is enabled. */
    RBRINSTRUMENTGEN4_INSTRUMENT_STATE_ENABLED,
    /** The number of specific instrument states. */
    RBRINSTRUMENTGEN4_INSTRUMENT_STATE_COUNT,
    /** An unknown or unrecognized instrument state. */
    RBRINSTRUMENTGEN4_UNKNOWN_INSTRUMENT_STATE
} RBRInstrumentGen4InstrumentState;

/**
 * \brief Get a human-readable string name for a instrument state.
 *
 * \param [in] state the instrument state
 * \return a string name for the instrument state
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4InstrumentState_name(
    RBRInstrumentGen4InstrumentState status);

/**
 * \brief Possible deployment statuses.
 * This tracks the status of the deployment running on the instrument.
 *
 * \see RBRInstrumentGen4Deployment
 * \see RBRInstrumentGen4_getDeployment()
 * \see RBRInstrumentGen4_pause()
 * \see RBRInstrumentGen4_resume()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828403/deployment
 */
typedef enum RBRInstrumentGen4DeploymentStatus
{
    /** Logging is in progress. */
    RBRINSTRUMENTGEN4_STATUS_SAMPLING,
    /** Logging paused; awaiting satisfaction of a gating condition. */
    RBRINSTRUMENTGEN4_STATUS_GATED,
    /** Logging paused, waiting for a resume command. */
    RBRINSTRUMENTGEN4_STATUS_PAUSED,
    /**
     * A `disable` command was received.
     *
     * \see RBRInstrumentGen4_disable()
     */
    RBRINSTRUMENTGEN4_STATUS_INACTIVE,
    /** Memory full; logging has stopped. */
    RBRINSTRUMENTGEN4_STATUS_UNKNOWN,
    /** The number of specific statuses. */
    RBRINSTRUMENTGEN4_STATUS_COUNT,
    /** An unknown or unrecognized status. */
    RBRINSTRUMENTGEN4_UNKNOWN_STATUS
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
 * \brief Possible instrument gating conditions.
 *
 * \see RBRInstrumentGen4Deployment
 * \see RBRInstrumentGen4Gating.h
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/schedule
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-schedule
 */
typedef enum RBRInstrumentGen4Gate
{
    /** No gating. */
    RBRINSTRUMENTGEN4_GATE_NONE,
    /**
     * Threshold gating.
     *
     * \see RBRInstrumentGen4_setThresholding()
     */
    RBRINSTRUMENTGEN4_GATE_THRESHOLDING,
    /**
     * Twist-activated gating.
     *
     * \see RBRInstrumentGen4_setTwistActivation()
     */
    RBRINSTRUMENTGEN4_GATE_TWISTACTIVATION,
    /** The instrument considers its gating condition to be invalid. */
    RBRINSTRUMENTGEN4_GATE_INVALID,
    /** The number of specific schedule modes. */
    RBRINSTRUMENTGEN4_GATE_COUNT,
    /** An unknown or unrecognized schedule mode. */
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

/** \brief Instrument `deployment` command parameters.
 *
 * \see RBRInstrumentGen4_getDeployment()
 * \see RBRInstrumentGen4_setDeployment()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828403/deployment
 */
typedef struct RBRInstrumentGen4Deployment
{
    /** \brief The deployment start date and time.  */
    RBRInstrumentGen4DateTime startTime;

    /**
     * \brief The deployment status.
     *
     * \readonly
     */
    RBRInstrumentGen4DeploymentStatus status;

    /** \brief Gets any gating condition currently enabled.
     * options: none|wetswitch|twistactivation|invalid
     */
    RBRInstrumentGen4Gate gate;

    /** \brief Gets whether any of the instrument's channels are being simulated (on),
     * or whether they are all reporting true measure data(off).
     * Options: on|off.
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
 * As noted in the description of RBRInstrumentGen4Deployment.status, that field is
 * ignored when setting the deployment.
 * 
 * The value RBRInstrumentGen4Deployment.gate is ignored. The gating mode is
 * controlled via commands for the individual gating mechanisms: see
 * RBRInstrumentGen4_setTwistActivation() and RBRInstrumentGen4_setThresholding().
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - you set an out-of-bounds parameter the library fails to detect
 *
 * \param [in] instrument the instrument connection
 * \param [in] deployment the deployment parameters
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the deployment is successfully changed
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the deployment cannot be changed
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the start or end time
 *                                                values are out of range
 * \see RBRInstrumentGen4_getDeployment()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828403/deployment
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setDeployment(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Deployment *deployment);

/**
 * \brief Pauses an enabled deloyment.
 * \note Issues the `pause` instrument command.
 * 
 * \param [in] instrument the instrument connection
 * \param [out] status the deployment status
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the status is "paused".
 * \return #RBRINSTRUMENTGEN4_UNSUPPORTED when the current firmware doesn't support
 * pauseresume feature, or pauseresume is not allowed.
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the response indicates an error.
 * \see RBRInstrumentGen4_resume()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828461/pause
 */
RBRInstrumentGen4Error RBRInstrumentGen4_pause(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4DeploymentStatus *status);

/**
 * \brief Resumes an enabled deployment which was previously
 * paused using the pause command
 * \note Issues the `resume` instrument command.
 * 
 * \param [in] instrument the instrument connection
 * \param [out] status the deployment status
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the state is one of the following:
 * "sampling", "gated".
 * \return #RBRINSTRUMENTGEN4_UNSUPPORTED when the current firmware doesn't support
 * pauseresume feature, or pauseresume is not allowed.
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the response indicates an error.
 * \see RBRInstrumentGen4_pause()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828461/pause
 */
RBRInstrumentGen4Error RBRInstrumentGen4_resume(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4DeploymentStatus *status);


#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTGEN4SAMPLING_H */
