/**
 * \file RBRInstrumentSchedule.h
 *
 * \brief Instrument commands and structures pertaining to time and schedule.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTSCHEDULE_H
#define LIBRBR_RBRINSTRUMENTSCHEDULE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief The maximum number of available fast sampling periods to parse from
 * the instrument.
 *
 * \see RBRInstrumentSampling.availableFastPeriods
 */
#define RBRINSTRUMENT_AVAILABLE_FAST_PERIODS_MAX 32

/** \brief The maximum sampling period in milliseconds. */
#define RBRINSTRUMENT_SAMPLING_PERIOD_MAX 86400000

/**
 * \brief Instrument `clock` command parameters.
 *
 * \see RBRInstrument_getClock()
 * \see RBRInstrument_setClock()
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/clock
 */
typedef struct RBRInstrumentClock
{
    /**
     * \brief The instrument's date and time.
     */
    RBRInstrumentDateTime dateTime;
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
} RBRInstrumentClock;

/**
 * \brief Get the instrument clock.
 *
 * Because UTC offset is tracked as a setting on Logger2 instruments, not as a
 * parameter of the `now` command (as it is of `clock` on Logger3), this
 * function will internally issue two commands to Logger2 instruments to
 * separately receive the time and UTC offset. When retrieving the clock from
 * older Logger2 instruments which do not support the `offsetfromutc` setting,
 * RBRInstrumentClock.offsetFromUtc will always be `NAN`.
 *
 * \param [in] instrument the instrument connection
 * \param [out] clock the clock value
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/clock
 */
RBRInstrumentError RBRInstrument_getClock(RBRInstrument *instrument,
                                          RBRInstrumentClock *clock);

/**
 * \brief Set the instrument clock.
 *
 * Because UTC offset is tracked as a setting on Logger2 instruments, not as a
 * parameter of the `now` command (as it is of `clock` on Logger3), this
 * function will internally issue two commands to Logger2 instruments to
 * separately set the time and UTC offset. When setting the clock on older
 * Logger2 instruments which do not support the `offsetfromutc` setting, the
 * value of the RBRInstrumentClock.offsetFromUtc field will be ignored.
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - you set an out-of-bounds time the library fails to detect
 *
 * \param [in] instrument the instrument connection
 * \param [in] clock the clock value
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when the clock values are out
 *                                                of range
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/clock
 */
RBRInstrumentError RBRInstrument_setClock(RBRInstrument *instrument,
                                          const RBRInstrumentClock *clock);

/**
 * \brief Possible instrument sampling modes.
 *
 * \see RBRInstrumentSampling
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/sampling
 */
typedef enum RBRInstrumentSamplingMode
{
    /** Continuous sampling mode. */
    RBRINSTRUMENT_SAMPLING_CONTINUOUS,
    /** Burst sampling mode. */
    RBRINSTRUMENT_SAMPLING_BURST,
    /** Wave sampling mode. */
    RBRINSTRUMENT_SAMPLING_WAVE,
    /** Average sampling mode. */
    RBRINSTRUMENT_SAMPLING_AVERAGE,
    /** Tide sampling mode. */
    RBRINSTRUMENT_SAMPLING_TIDE,
    /** Regime sampling mode. */
    RBRINSTRUMENT_SAMPLING_REGIMES,
    /**
     * Direction-dependent sampling mode.
     *
     * \see RBRInstrumentVehicle.h
     * \see RBRInstrument_setDirectionDependentSampling()
     */
    RBRINSTRUMENT_SAMPLING_DDSAMPLING,
    /** The number of specific sampling modes. */
    RBRINSTRUMENT_SAMPLING_COUNT,
    /** An unknown or unrecognized sampling mode. */
    RBRINSTRUMENT_UNKNOWN_SAMPLING
} RBRInstrumentSamplingMode;

/**
 * \brief Get a human-readable string name for a sampling mode.
 *
 * \param [in] mode the sampling mode
 * \return a string name for the sampling mode
 * \see RBRInstrumentError_name() for a description of the format of names
 */
const char *RBRInstrumentSamplingMode_name(RBRInstrumentSamplingMode mode);

/**
 * \brief Possible instrument gating conditions.
 *
 * \see RBRInstrumentSampling
 * \see RBRInstrumentGating.h
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/sampling
 * \see https://docs.rbr-global.com/L3commandreference/commands/gated-sampling
 */
typedef enum RBRInstrumentGate
{
    /** No gating. */
    RBRINSTRUMENT_GATE_NONE,
    /**
     * Threshold gating.
     *
     * \see RBRInstrument_setThresholding()
     */
    RBRINSTRUMENT_GATE_THRESHOLDING,
    /**
     * Twist-activated gating.
     *
     * \see RBRInstrument_setTwistActivation()
     */
    RBRINSTRUMENT_GATE_TWISTACTIVATION,
    /** The instrument considers its gating condition to be invalid. */
    RBRINSTRUMENT_GATE_INVALID,
    /** The number of specific sampling modes. */
    RBRINSTRUMENT_GATE_COUNT,
    /** An unknown or unrecognized sampling mode. */
    RBRINSTRUMENT_UNKNOWN_GATE
} RBRInstrumentGate;

/**
 * \brief Get a human-readable string name for a gating condition.
 *
 * \param [in] gate the gating condition
 * \return a string name for the gating condition
 * \see RBRInstrumentError_name() for a description of the format of names
 */
const char *RBRInstrumentGate_name(RBRInstrumentGate gate);

/**
 * \brief Instrument `sampling` command parameters.
 *
 * See the command reference for details on valid parameter values.
 *
 * \see RBRInstrument_getSampling()
 * \see RBRInstrument_setSampling()
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/sampling
 */
typedef struct RBRInstrumentSampling
{
    /** \brief The instrument sampling mode. */
    RBRInstrumentSamplingMode mode;
    /**
     * \brief Time between measurements.
     *
     * Specified in milliseconds. Must be in the range
     * RBRInstrumentSampling.userPeriodLimit—86,400,000.
     *
     * - When < 1,000, must be in RBRInstrumentSampling.availableFastPeriods.
     * - When ≥ 1,000, must be an integer multiple of 1,000.
     */
    RBRInstrumentPeriod period;
    /**
     * \brief Fast measurement periods available for the logger for sampling
     * rates faster than 1Hz.
     *
     * Available fast periods are stored in the array in the order reported by
     * the instrument. Unused array elements are populated with `0`. If more
     * than #RBRINSTRUMENT_AVAILABLE_FAST_PERIODS_MAX are available, trailing
     * entries are discarded.
     *
     * Logger2 instruments do not report available fast periods. For
     * convenience, RBRInstrument_getSampling() will synthesize the contents of
     * this field based on the value of RBRInstrumentSampling.userPeriodLimit.
     *
     * \readonly
     */
    const RBRInstrumentPeriod
        availableFastPeriods[RBRINSTRUMENT_AVAILABLE_FAST_PERIODS_MAX];
    /**
     * \brief The minimum period which can be used in fast sampling modes.
     *
     * Specified in milliseconds.
     *
     * This is the minimum RBRInstrumentSampling.period value.
     *
     * \readonly
     */
    const RBRInstrumentPeriod userPeriodLimit;
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
     * constrained by the sampling period (RBRInstrumentSampling.period) and
     * burst length (RBRInstrumentSampling.burstLength):
     *
     *     burst interval > (burst length × sampling period)
     */
    RBRInstrumentPeriod burstInterval;
    /** \brief The sampling gating condition. */
    RBRInstrumentGate gate;
} RBRInstrumentSampling;

/**
 * \brief Get the instrument sampling parameters.
 *
 * \param [in] instrument the instrument connection
 * \param [out] sampling the sampling parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/sampling
 */
RBRInstrumentError RBRInstrument_getSampling(
    RBRInstrument *instrument,
    RBRInstrumentSampling *sampling);

/**
 * \brief Set the instrument sampling mode and period.
 *
 * This does _not_ set burst parameters (RBRInstrumentSampling.burstLength and
 * RBRInstrumentSampling.burstInterval). On instruments which do not support
 * bursting/averaging, attempting to set these parameters results in a hardware
 * error; to avoid this, bursting parameters may be configured via
 * RBRInstrument_setBurstSampling().
 *
 * The values of RBRInstrumentSampling.userPeriodLimit and
 * RBRInstrumentSampling.availableFastPeriods are not sent to the instrument,
 * but they are used to validate the chosen RBRInstrumentSampling.period.
 * If RBRInstrumentSampling.userPeriodLimit is non-zero, then the specified
 * sampling period must be equal or greater. And if the period is less than
 * 1,000 and the RBRInstrumentSampling.availableFastPeriods are populated, then
 * the period must be one of those available fast periods.
 *
 * Periods greater than or equal to 1,000 (one second) must be an integer
 * multiple of 1,000 and must be less than or equal to 86,400,000 (24 hours).
 *
 * The value RBRInstrumentSampling.gate is also ignored. The gating mode is
 * controlled via commands for the individual gating mechanisms: see
 * RBRInstrument_setTwistActivation() and RBRInstrument_setThresholding().
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
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/sampling
 * \see RBRInstrument_setBurstSampling()
 */
RBRInstrumentError RBRInstrument_setSampling(
    RBRInstrument *instrument,
    const RBRInstrumentSampling *sampling);

/**
 * \brief Set the instrument burst sampling length and interval.
 *
 * This sets only burst parameters (RBRInstrumentSampling.burstLength and
 * RBRInstrumentSampling.burstInterval). To configure the sampling mode and
 * period, use RBRInstrument_setSampling().
 *
 * Only burst parameters are sent to the instrument. However, the sampling
 * period (RBRInstrumentSampling.period) is used to validate the burst
 * interval, which is itself validated by RBRInstrumentSampling.userPeriodLimit
 * and RBRInstrumentSampling.availableFastPeriods. See
 * RBRInstrumentSampling.burstInterval for details.
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - you set an out-of-bounds parameter the library fails to detect
 * - bursting/averaging is not supported by the instrument
 *
 * \param [in] instrument the instrument connection
 * \param [in] sampling the sampling parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/sampling
 * \see RBRInstrument_setSampling()
 */
RBRInstrumentError RBRInstrument_setBurstSampling(
    RBRInstrument *instrument,
    const RBRInstrumentSampling *sampling);

/**
 * \brief Possible instrument logging statuses.
 *
 * \see RBRInstrumentDeployment
 * \see RBRInstrument_getDeployment()
 * \see RBRInstrument_enable()
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/deployment
 * \see https://docs.rbr-global.com/L3commandreference/commands/deployments/enable
 */
typedef enum RBRInstrumentDeploymentStatus
{
    /** Logging is not enabled. */
    RBRINSTRUMENT_STATUS_DISABLED,
    /** Logging is enabled but the start time has not yet passed. */
    RBRINSTRUMENT_STATUS_PENDING,
    /** Logging is in progress. */
    RBRINSTRUMENT_STATUS_LOGGING,
    /** Logging paused; awaiting satisfaction of a gating condition. */
    RBRINSTRUMENT_STATUS_GATED,
    /** The programmed end time has been passed. */
    RBRINSTRUMENT_STATUS_FINISHED,
    /**
     * A `disable` command was received.
     *
     * \see RBRInstrument_disable()
     */
    RBRINSTRUMENT_STATUS_STOPPED,
    /** Memory full; logging has stopped. */
    RBRINSTRUMENT_STATUS_FULLANDSTOPPED,
    /** Memory full; logger continues to stream data. */
    RBRINSTRUMENT_STATUS_FULL,
    /** Stopped; internal error. */
    RBRINSTRUMENT_STATUS_FAILED,
    /** Memory failed to erase. */
    RBRINSTRUMENT_STATUS_NOTBLANK,
    /** Instrument internal error; state unknown. */
    RBRINSTRUMENT_STATUS_UNKNOWN,
    /** The number of specific statuses. */
    RBRINSTRUMENT_STATUS_COUNT,
    /** An unknown or unrecognized status. */
    RBRINSTRUMENT_UNKNOWN_STATUS
} RBRInstrumentDeploymentStatus;

/**
 * \brief Get a human-readable string name for a deployment status.
 *
 * \param [in] status the deployment status
 * \return a string name for the deployment status
 * \see RBRInstrumentError_name() for a description of the format of names
 */
const char *RBRInstrumentDeploymentStatus_name(
    RBRInstrumentDeploymentStatus status);

/**
 * \brief Instrument `deployment` command parameters.
 *
 * \see RBRInstrument_getDeployment()
 * \see RBRInstrument_setDeployment()
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/deployment
 */
typedef struct RBRInstrumentDeployment
{
    /**
     * \brief The deployment start date and time.
     *
     * Must be before the end time.
     */
    RBRInstrumentDateTime startTime;
    /**
     * \brief The deployment end date and time.
     *
     * Must be after the start time.
     */
    RBRInstrumentDateTime endTime;
    /**
     * \brief The deployment status.
     *
     * \readonly
     */
    const RBRInstrumentDeploymentStatus status;
} RBRInstrumentDeployment;

/**
 * \brief Get the instrument deployment parameters.
 *
 * \param [in] instrument the instrument connection
 * \param [out] deployment the deployment parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/deployment
 */
RBRInstrumentError RBRInstrument_getDeployment(
    RBRInstrument *instrument,
    RBRInstrumentDeployment *deployment);

/**
 * \brief Set the instrument deployment parameters.
 *
 * As noted in the description of RBRInstrumentDeployment.status, that field is
 * ignored when setting the deployment.
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - you set an out-of-bounds parameter the library fails to detect
 *
 * \param [in] instrument the instrument connection
 * \param [in] deployment the deployment parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when the start or end time
 *                                                values are out of range
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/deployment
 */
RBRInstrumentError RBRInstrument_setDeployment(
    RBRInstrument *instrument,
    const RBRInstrumentDeployment *deployment);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTSCHEDULE_H */
