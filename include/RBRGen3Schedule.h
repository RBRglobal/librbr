/**
 * \file RBRGen3Schedule.h
 *
 * \brief Instrument commands and structures pertaining to time and schedule.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRGEN3SCHEDULE_H
#define LIBRBR_RBRGEN3SCHEDULE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief The maximum number of available fast sampling periods to parse from
 * the instrument.
 *
 * \see RBRGen3Sampling.availableFastPeriods
 */
#define RBRGEN3_AVAILABLE_FAST_PERIODS_MAX 32

/** \brief The maximum sampling period in milliseconds. */
#define RBRGEN3_SAMPLING_PERIOD_MAX 86400000

/**
 * \brief Instrument `clock` command parameters.
 *
 * \see RBRGen3_getClock()
 * \see RBRGen3_setClock()
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/clock
 */
typedef struct RBRGen3Clock
{
    /**
     * \brief The instrument's date and time.
     */
    RBRGen3DateTime dateTime;
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
} RBRGen3Clock;

/**
 * \brief Get the instrument clock.
 *
 * Because UTC offset is tracked as a setting on Logger2 instruments, not as a
 * parameter of the `now` command (as it is of `clock` on Logger3), this
 * function will internally issue two commands to Logger2 instruments to
 * separately receive the time and UTC offset. When retrieving the clock from
 * older Logger2 instruments which do not support the `offsetfromutc` setting,
 * RBRGen3Clock.offsetFromUtc will always be `NAN`.
 *
 * \param [in] instrument the instrument connection
 * \param [out] clock the clock value
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/clock
 */
RBRGen3Error RBRGen3_getClock(RBRGen3 *instrument,
                                          RBRGen3Clock *clock);

/**
 * \brief Set the instrument clock.
 *
 * Because UTC offset is tracked as a setting on Logger2 instruments, not as a
 * parameter of the `now` command (as it is of `clock` on Logger3), this
 * function will internally issue two commands to Logger2 instruments to
 * separately set the time and UTC offset. When setting the clock on older
 * Logger2 instruments which do not support the `offsetfromutc` setting, the
 * value of the RBRGen3Clock.offsetFromUtc field will be ignored.
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - you set an out-of-bounds time the library fails to detect
 *
 * \param [in] instrument the instrument connection
 * \param [in] clock the clock value
 * \return #RBRGEN3_SUCCESS when the settings are successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when the clock values are out
 *                                                of range
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/clock
 */
RBRGen3Error RBRGen3_setClock(RBRGen3 *instrument,
                                          const RBRGen3Clock *clock);

/**
 * \brief Possible instrument sampling modes.
 *
 * \see RBRGen3Sampling
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/sampling
 */
typedef enum RBRGen3SamplingMode
{
    /** Continuous sampling mode. */
    RBRGEN3_SAMPLING_CONTINUOUS,
    /** Burst sampling mode. */
    RBRGEN3_SAMPLING_BURST,
    /** Wave sampling mode. */
    RBRGEN3_SAMPLING_WAVE,
    /** Average sampling mode. */
    RBRGEN3_SAMPLING_AVERAGE,
    /** Tide sampling mode. */
    RBRGEN3_SAMPLING_TIDE,
    /** Regime sampling mode. */
    RBRGEN3_SAMPLING_REGIMES,
    /**
     * Direction-dependent sampling mode.
     *
     * \see RBRGen3Vehicle.h
     * \see RBRGen3_setDirectionDependentSampling()
     */
    RBRGEN3_SAMPLING_DDSAMPLING,
    /** The number of specific sampling modes. */
    RBRGEN3_SAMPLING_COUNT,
    /** An unknown or unrecognized sampling mode. */
    RBRGEN3_UNKNOWN_SAMPLING
} RBRGen3SamplingMode;

/**
 * \brief Get a human-readable string name for a sampling mode.
 *
 * \param [in] mode the sampling mode
 * \return a string name for the sampling mode
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRGen3SamplingMode_name(RBRGen3SamplingMode mode);

/**
 * \brief Possible instrument gating conditions.
 *
 * \see RBRGen3Sampling
 * \see RBRGen3Gating.h
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/sampling
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling
 */
typedef enum RBRGen3Gate
{
    /** No gating. */
    RBRGEN3_GATE_NONE,
    /**
     * Threshold gating.
     *
     * \see RBRGen3_setThresholding()
     */
    RBRGEN3_GATE_THRESHOLDING,
    /**
     * Twist-activated gating.
     *
     * \see RBRGen3_setTwistActivation()
     */
    RBRGEN3_GATE_TWISTACTIVATION,
    /** The instrument considers its gating condition to be invalid. */
    RBRGEN3_GATE_INVALID,
    /** The number of specific sampling modes. */
    RBRGEN3_GATE_COUNT,
    /** An unknown or unrecognized sampling mode. */
    RBRGEN3_UNKNOWN_GATE
} RBRGen3Gate;

/**
 * \brief Get a human-readable string name for a gating condition.
 *
 * \param [in] gate the gating condition
 * \return a string name for the gating condition
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRGen3Gate_name(RBRGen3Gate gate);

/**
 * \brief Instrument `sampling` command parameters.
 *
 * See the command reference for details on valid parameter values.
 *
 * \see RBRGen3_getSampling()
 * \see RBRGen3_setSampling()
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/sampling
 */
typedef struct RBRGen3Sampling
{
    /** \brief The instrument sampling mode. */
    RBRGen3SamplingMode mode;
    /**
     * \brief Time between measurements.
     *
     * Specified in milliseconds. Must be in the range
     * RBRGen3Sampling.userPeriodLimit—86,400,000.
     *
     * - When < 1,000, must be in RBRGen3Sampling.availableFastPeriods.
     * - When ≥ 1,000, must be an integer multiple of 1,000.
     */
    RBRGen3Period period;
    /**
     * \brief Fast measurement periods available for the logger for sampling
     * rates faster than 1Hz.
     *
     * Available fast periods are stored in the array in the order reported by
     * the instrument. Unused array elements are populated with `0`. If more
     * than #RBRGEN3_AVAILABLE_FAST_PERIODS_MAX are available, trailing
     * entries are discarded.
     *
     * Logger2 instruments do not report available fast periods. For
     * convenience, RBRGen3_getSampling() will synthesize the contents of
     * this field based on the value of RBRGen3Sampling.userPeriodLimit.
     *
     * \readonly
     */
    const RBRGen3Period
        availableFastPeriods[RBRGEN3_AVAILABLE_FAST_PERIODS_MAX];
    /**
     * \brief The minimum period which can be used in fast sampling modes.
     *
     * Specified in milliseconds.
     *
     * This is the minimum RBRGen3Sampling.period value.
     *
     * \readonly
     */
    const RBRGen3Period userPeriodLimit;
    /**
     * \brief The number of measurements taken in each burst.
     *
     * Specified in numbers of samples. Must be in the range 2—65,535.
     */
    int32_t burstLength;
    /**
     * \brief The time between the first measurement of two consecutive bursts.
     *
     * Specified in milliseconds. Must be in the range 1,000—86,400,000 and
     * must be an integer multiple of 1,000. The burst interval is additionally
     * constrained by the sampling period (RBRGen3Sampling.period) and
     * burst length (RBRGen3Sampling.burstLength):
     *
     *     burst interval > (burst length × sampling period)
     */
    RBRGen3Period burstInterval;
    /** \brief The sampling gating condition. */
    RBRGen3Gate gate;
} RBRGen3Sampling;

/**
 * \brief Get the instrument sampling parameters.
 *
 * \param [in] instrument the instrument connection
 * \param [out] sampling the sampling parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/sampling
 */
RBRGen3Error RBRGen3_getSampling(
    RBRGen3 *instrument,
    RBRGen3Sampling *sampling);

/**
 * \brief Set the instrument sampling mode and period.
 *
 * This does _not_ set burst parameters (RBRGen3Sampling.burstLength and
 * RBRGen3Sampling.burstInterval). On instruments which do not support
 * bursting/averaging, attempting to set these parameters results in a hardware
 * error; to avoid this, bursting parameters may be configured via
 * RBRGen3_setBurstSampling().
 *
 * The values of RBRGen3Sampling.userPeriodLimit and
 * RBRGen3Sampling.availableFastPeriods are not sent to the instrument,
 * but they are used to validate the chosen RBRGen3Sampling.period.
 * If RBRGen3Sampling.userPeriodLimit is non-zero, then the specified
 * sampling period must be equal or greater. And if the period is less than
 * 1,000 and the RBRGen3Sampling.availableFastPeriods are populated, then
 * the period must be one of those available fast periods.
 *
 * Periods greater than or equal to 1,000 (one second) must be an integer
 * multiple of 1,000 and must be less than or equal to 86,400,000 (24 hours).
 *
 * The value RBRGen3Sampling.gate is also ignored. The gating mode is
 * controlled via commands for the individual gating mechanisms: see
 * RBRGen3_setTwistActivation() and RBRGen3_setThresholding().
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - you set an out-of-bounds parameter the library fails to detect
 * - you attempt to set sampling parameters for an RBRcoda (where sampling is
 *   not supported)
 *
 * \param [in] instrument the instrument connection
 * \param [in] sampling the sampling parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/sampling
 * \see RBRGen3_setBurstSampling()
 */
RBRGen3Error RBRGen3_setSampling(
    RBRGen3 *instrument,
    const RBRGen3Sampling *sampling);

/**
 * \brief Set the instrument burst sampling length and interval.
 *
 * This sets only burst parameters (RBRGen3Sampling.burstLength and
 * RBRGen3Sampling.burstInterval). To configure the sampling mode and
 * period, use RBRGen3_setSampling().
 *
 * Only burst parameters are sent to the instrument. However, the sampling
 * period (RBRGen3Sampling.period) is used to validate the burst
 * interval, which is itself validated by RBRGen3Sampling.userPeriodLimit
 * and RBRGen3Sampling.availableFastPeriods. See
 * RBRGen3Sampling.burstInterval for details.
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - you set an out-of-bounds parameter the library fails to detect
 * - bursting/averaging is not supported by the instrument
 *
 * \param [in] instrument the instrument connection
 * \param [in] sampling the sampling parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/sampling
 * \see RBRGen3_setSampling()
 */
RBRGen3Error RBRGen3_setBurstSampling(
    RBRGen3 *instrument,
    const RBRGen3Sampling *sampling);

/**
 * \brief Possible instrument logging statuses.
 *
 * \see RBRGen3Deployment
 * \see RBRGen3_getDeployment()
 * \see RBRGen3_enable()
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/deployment
 * \see https://docs.rbr-global.com/L3commandreference/commands/deployments/enable
 */
typedef enum RBRGen3DeploymentStatus
{
    /** Logging is not enabled. */
    RBRGEN3_STATUS_DISABLED,
    /** Logging is enabled but the start time has not yet passed. */
    RBRGEN3_STATUS_PENDING,
    /** Logging is in progress. */
    RBRGEN3_STATUS_LOGGING,
    /** Logging paused; awaiting satisfaction of a gating condition. */
    RBRGEN3_STATUS_GATED,
    /** The programmed end time has been passed. */
    RBRGEN3_STATUS_FINISHED,
    /**
     * A `disable` command was received.
     *
     * \see RBRGen3_disable()
     */
    RBRGEN3_STATUS_STOPPED,
    /** Memory full; logging has stopped. */
    RBRGEN3_STATUS_FULLANDSTOPPED,
    /** Memory full; logger continues to stream data. */
    RBRGEN3_STATUS_FULL,
    /** Stopped; internal error. */
    RBRGEN3_STATUS_FAILED,
    /** Memory failed to erase. */
    RBRGEN3_STATUS_NOTBLANK,
    /** Instrument internal error; state unknown. */
    RBRGEN3_STATUS_UNKNOWN,
    /** The number of specific statuses. */
    RBRGEN3_STATUS_COUNT,
    /** An unknown or unrecognized status. */
    RBRGEN3_UNKNOWN_STATUS
} RBRGen3DeploymentStatus;

/**
 * \brief Get a human-readable string name for a deployment status.
 *
 * \param [in] status the deployment status
 * \return a string name for the deployment status
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRGen3DeploymentStatus_name(
    RBRGen3DeploymentStatus status);

/**
 * \brief Instrument `deployment` command parameters.
 *
 * \see RBRGen3_getDeployment()
 * \see RBRGen3_setDeployment()
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/deployment
 */
typedef struct RBRGen3Deployment
{
    /**
     * \brief The deployment start date and time.
     *
     * Must be before the end time.
     */
    RBRGen3DateTime startTime;
    /**
     * \brief The deployment end date and time.
     *
     * Must be after the start time.
     */
    RBRGen3DateTime endTime;
    /**
     * \brief The deployment status.
     *
     * \readonly
     */
    const RBRGen3DeploymentStatus status;
} RBRGen3Deployment;

/**
 * \brief Get the instrument deployment parameters.
 *
 * \param [in] instrument the instrument connection
 * \param [out] deployment the deployment parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/deployment
 */
RBRGen3Error RBRGen3_getDeployment(
    RBRGen3 *instrument,
    RBRGen3Deployment *deployment);

/**
 * \brief Set the instrument deployment parameters.
 *
 * As noted in the description of RBRGen3Deployment.status, that field is
 * ignored when setting the deployment.
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - you set an out-of-bounds parameter the library fails to detect
 *
 * \param [in] instrument the instrument connection
 * \param [in] deployment the deployment parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when the start or end time
 *                                                values are out of range
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/deployment
 */
RBRGen3Error RBRGen3_setDeployment(
    RBRGen3 *instrument,
    const RBRGen3Deployment *deployment);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN3SCHEDULE_H */
