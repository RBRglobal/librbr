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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief The maximum number of available fast schedule periods to parse from
 * the instrument.
 *
 * \see RBRInstrumentGen4Deployment.availableFastPeriods
 */
#define RBRINSTRUMENTGEN4_AVAILABLE_FAST_PERIODS_MAX 32

/**
 * \brief Instrument `clock` command parameters.
 *
 * \see RBRInstrumentGen4_getClock()
 * \see RBRInstrumentGen4_setClock()
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/clock
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
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/clock
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getClock(RBRInstrumentGen4 *instrument,
                                          RBRInstrumentGen4Clock *clock);

/**
 * \brief Set the instrument clock.
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
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/clock
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setClock(RBRInstrumentGen4 *instrument,
                                          const RBRInstrumentGen4Clock *clock);

/**
 * \brief Possible instrument logging statuses.
 *
 * \see RBRInstrumentGen4Deployment
 * \see RBRInstrumentGen4_getDeployment()
 * \see RBRInstrumentGen4_enable()
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/deployment
 * \see https://docs.rbr-global.com/L3commandreference/commands/deployments/enable
 */
typedef enum RBRInstrumentGen4DeploymentStatus
{
    /** Logging is not enabled. */
    RBRINSTRUMENTGEN4_STATUS_DISABLED,
    /** Logging is enabled but the start time has not yet passed. */
    RBRINSTRUMENTGEN4_STATUS_PENDING,
    /** Logging is in progress. */
    RBRINSTRUMENTGEN4_STATUS_LOGGING,
    /** Logging paused; awaiting satisfaction of a gating condition. */
    RBRINSTRUMENTGEN4_STATUS_GATED,
    /** Logging paused, waiting for a resume command. */
    RBRINSTRUMENTGEN4_STATUS_PAUSED,
    /** The programmed end time has been passed. */
    RBRINSTRUMENTGEN4_STATUS_FINISHED,
    /**
     * A `disable` command was received.
     *
     * \see RBRInstrumentGen4_disable()
     */
    RBRINSTRUMENTGEN4_STATUS_STOPPED,
    /** Memory full; logging has stopped. */
    RBRINSTRUMENTGEN4_STATUS_FULLANDSTOPPED,
    /** Memory full; logger continues to stream data. */
    RBRINSTRUMENTGEN4_STATUS_FULL,
    /** Stopped; internal error. */
    RBRINSTRUMENTGEN4_STATUS_FAILED,
    /** Memory failed to erase. */
    RBRINSTRUMENTGEN4_STATUS_NOTBLANK,
    /** Instrument internal error; state unknown. */
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
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/deployment
 */
typedef struct RBRInstrumentGen4Deployment
{
    /**
     * \brief The deployment start date and time.
     *
     * Must be before the end time.
     */
    RBRInstrumentGen4DateTime startTime;
    /**
     * \brief The deployment end date and time.
     *
     * Must be after the start time.
     */
    RBRInstrumentGen4DateTime endTime;
    /**
     * \brief The deployment status.
     *
     * \readonly
     */
    const RBRInstrumentGen4DeploymentStatus status;

    /** \brief Gets the label of the configuration currently being used to run an
     * active deployment. 
     * If a deployment is not actively in progress, the word "none" is reported.
    */
    const char config[RBRINSTRUMENTGEN4_LABEL_NAME_MAX+1];

    /** \brief Gets the label of the dataset currently being written to the instrument's
     * memory during an active deployment.
     * If a deployment is not actively in progress, the word "none" is reported.
    */
    const char dataset[RBRINSTRUMENTGEN4_LABEL_NAME_MAX+1];
        /**
     * \brief Fast measurement periods available for the logger for sampling
     * rates faster than 1Hz.
     *
     * Available fast periods are stored in the array in the order reported by
     * the instrument. Unused array elements are populated with `0`. If more
     * than #RBRINSTRUMENTGEN4_AVAILABLE_FAST_PERIODS_MAX are available, trailing
     * entries are discarded.
     *
     * Logger2 instruments do not report available fast periods. 
     *
     * \readonly
     */
    const RBRInstrumentGen4Period
        availableFastPeriods[RBRINSTRUMENTGEN4_AVAILABLE_FAST_PERIODS_MAX];

    /** \brief Gets any gating condition currently enabled.
     * options: none|thresholding|twistactivation|invalid
    */
    const RBRInstrumentGen4Gate gate;

    /** \brief Gets whether any of the instrument's channels are being simulated (on),
     * or whether they are all reporting true measure data(off).
     * Options: on|off.
    */
    const bool simulation;
} RBRInstrumentGen4Deployment;

/**
 * \brief Get the instrument deployment parameters.
 *
 * \param [in] instrument the instrument connection
 * \param [out] deployment the deployment parameters
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/deployment
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getDeployment(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Deployment *deployment);

/**
 * \brief Set the instrument deployment parameters.
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
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the start or end time
 *                                                values are out of range
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/deployment
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setDeployment(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Deployment *deployment);

/** \brief The status of a pauseresume condition. */
typedef enum RBRInstrumentGen4PauseresumeStatus
{
    /** Deployment is enaled and paused. */
    RBRINSTRUMENTGEN4_PAUSERESUME_STATUS_PAUSED,
    /** Deployment is enabled and will start at starttime. */
    RBRINSTRUMENTGEN4_PAUSERESUME_STATUS_PENDING,
    /** Deployment is enabled and running. */
    RBRINSTRUMENTGEN4_PAUSERESUME_STATUS_LOGGING,
    /** feature is not allowed, or firmware in use doesn't support this feature. */
    RBRINSTRUMENTGEN4_UNKNOWN_PAUSERESUME_STATUS
} RBRInstrumentGen4PauseresumeStatus;

/**
 * \brief Get a human-readable string name for a pauseresume status.
 *
 * \param [in] status the pauseresume state
 * \return a string name for the gating state
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4PauseresumeStatus_name(RBRInstrumentGen4PauseresumeStatus status);

/**
 * \brief It pauses an enabled deloyment.
 * 
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the status is "paused".
 * \return #RBRINSTRUMENTGEN4_UNSUPPORTED when the current firmware doesn't support
 * pauseresume feature, or pauseresume is not allowed.
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the response indicates an error.
 */
RBRInstrumentGen4Error RBRInstrumentGen4_pause(RBRInstrumentGen4 *instrument);
/**
 * \brief It resumes an enabled deployment which was previously
 * paused using the pause command
 * 
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the state is one of the following:
 * "pending", "logging".
 * \return #RBRINSTRUMENTGEN4_UNSUPPORTED when the current firmware doesn't support
 * pauseresume feature, or pauseresume is not allowed.
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the response indicates an error.
 */
RBRInstrumentGen4Error RBRInstrumentGen4_resume(RBRInstrumentGen4 *instrument);

/**
 * \brief Get a human-readable string name for a schedule mode.
 *
 * \param [in] mode the schedule mode
 * \return a string name for the schedule mode
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4SamplingMode_name(RBRInstrumentGen4SamplingMode mode);


/**
 * \brief Get a human-readable string name for an instrument direction.
 *
 * \param [in] direction the direction
 * \return a string name for the direction
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4Direction_name(RBRInstrumentGen4Direction direction);

/**
 * \brief Get a human-readable string name for a regime pressure reference.
 *
 * \param [in] reference the pressure reference
 * \return a string name for the pressure reference
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4RegimesReference_name(
    RBRInstrumentGen4RegimesReference reference);

/**
 * \brief Get the pool of logger schedules.
 *
 * \param [in] instrument the instrument connection
 * \param [out] schedules the schedules of this instrument.
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/schedules
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getSchedules(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Schedules *schedules);

/**
 * \brief Get the instrument schedule parameters.
 *
 * \param [in] instrument the instrument connection
 * \param [in] schedulelabel the schedule label
 * \param [in] schedule the schedule to populate
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/schedule
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getSchedule(
    RBRInstrumentGen4 *instrument,
    const char *schedulelabel,
    RBRInstrumentGen4Schedule *schedule);

/**
 * \brief Set the instrument schedule.
 *
 * Configlist is readonly. Attempts to set parameters that are not included in the specific schedule results in a hardware
 * error; 
 *
 * The values of RBRInstrumentGen4Deployment.availableFastPeriods are not sent to the instrument,
 * but they are used to validate the chosen period in RBRInstrumentGen4Schedule.
 * If the period is less than 1,000 and the RBRInstrumentGen4Deployment.availableFastPeriods are populated, then
 * the period must be one of those available fast periods.
 *
 * Periods greater than or equal to 1,000 (one second) must be an integer
 * multiple of 1,000 and must be less than or equal to 86,400,000 (24 hours).
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - you set an out-of-bounds parameter the library fails to detect
 * - you attempt to set schedule parameters for an Instruments prior to GEN4 (where schedule is
 *   not supported)
 *
 * \param [in] instrument the instrument connection
 * \param [in] schedule the schedule to be set
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/schedule
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setSchedule(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Schedule *schedule);

/**
 * \brief Creates a schedule with optional user defined parameters.
 *
 * Schedulelabel is required, grouplist and mode are optional. 
 * default grouplist is empty.
 * default mode is the simplest in availablemodes in command `schedules`.
 * \param [in] instrument the instrument connection
 * \param [in] schedule the created schedule.
 * \param [inout] schedules where the schedules are mapped with a pointer array.
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the operation is successful.
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/createschedule
 */
RBRInstrumentGen4Error RBRInstrumentGen4_createSchedule(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Schedule *schedule,
    RBRInstrumentGen4Schedules *schedules
    );

/**
 * \brief Deletes one schedule. Schedules may not be deleted while logging is enabled.
 *
 * \param [in] instrument the instrument connection
 * \param [in] schedulelabel specifies by label a single schedule to delete from the pool
 * \param [in] schedules all the schedules
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the operation is successful.
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/createschedule
 */
RBRInstrumentGen4Error RBRInstrumentGen4_deleteSchedule(
    RBRInstrumentGen4 *instrument,
    const char *schedulelabel,
    RBRInstrumentGen4Schedules *schedules);

/**
 * \brief Deletes more than one schedules. Schedules may not be deleted while logging is enabled.
 *
 * \param [in] instrument the instrument connection
 * \param [in] schedulelist specifies by their labels on or more schedules to delete from the pool
 * \param [in] schedules all the schedules
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the operation is successful.
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/createschedule
 */
RBRInstrumentGen4Error RBRInstrumentGen4_deleteScheduleMultiple(
    RBRInstrumentGen4 *instrument,
    const char *schedulelist,
    RBRInstrumentGen4Schedules *schedules);

/**
 * \brief Deletes all schedules. Schedules may not be deleted while logging is enabled.
 *
 * \param [in] instrument the instrument connection
 * \param [in] schedules all the schedules
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the operation is successful.
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/createschedule
 */
RBRInstrumentGen4Error RBRInstrumentGen4_deleteScheduleAll(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Schedules *schedules);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTGEN4SAMPLING_H */
