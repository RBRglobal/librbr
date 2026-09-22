/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file RBRGen3Configuration.h
 *
 * \brief Instrument commands and structures pertaining to instrument
 * configuration information and calibration.
 */

#ifndef LIBRBR_RBRGEN3CONFIGURATION_H
#define LIBRBR_RBRGEN3CONFIGURATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RBRGen3.h"

/**
 * \brief The maximum number of C calibration coefficients per channel.
 *
 * \see RBRGen3Calibration.c
 */
#ifndef RBRGEN3_CALIBRATION_C_COEFFICIENT_MAX
#define RBRGEN3_CALIBRATION_C_COEFFICIENT_MAX 8
#endif

/**
 * \brief The maximum number of X calibration coefficients per channel.
 *
 * \see RBRGen3Calibration.x
 */
#ifndef RBRGEN3_CALIBRATION_X_COEFFICIENT_MAX
#define RBRGEN3_CALIBRATION_X_COEFFICIENT_MAX 16
#endif

/**
 * \brief The maximum number of input channel indices per channel.
 *
 * \see RBRGen3Calibration.n
 */
#ifndef RBRGEN3_CALIBRATION_N_COEFFICIENT_MAX
#define RBRGEN3_CALIBRATION_N_COEFFICIENT_MAX 8
#endif

/**
 * \brief An in-band representation of the special “value” calibration
 * correction value.
 *
 * \see RBRGen3Calibration.n
 */
#define RBRGEN3_VALUE_COEFFICIENT 0xFF

/**
 * \brief The maximum number of characters in a calibration equation name.
 *
 * Does not include any null terminator.
 */
#define RBRGEN3_CALIBRATION_EQUATION_MAX 31

/**
 * \brief The maximum number of gain settings per channel.
 */
#ifndef RBRGEN3_CHANNEL_GAINS_MAX
#define RBRGEN3_CHANNEL_GAINS_MAX 8
#endif

/** \brief The minimum input timeout. */
#define RBRGEN3_INPUT_TIMEOUT_MIN 10000

/** \brief The maximum input timeout. */
#define RBRGEN3_INPUT_TIMEOUT_MAX 240000

/**
 * \brief The maximum number of characters in a sensor parameter key.
 *
 * Does not include any null terminator.
 */
#define RBRGEN3_SENSOR_PARAMETER_KEY_MAX 63

/**
 * \brief The maximum number of characters in a sensor parameter value.
 *
 * Does not include any null terminator.
 */
#define RBRGEN3_SENSOR_PARAMETER_VALUE_MAX 63

/**
 * \brief A channel identifier.
 *
 * Channel indices are always 1-based. A value of 0 means the index is unset
 * or empty.
 */
typedef uint8_t RBRGen3ChannelIndex;

/** \brief An internal module identifier. */
typedef uint8_t RBRGen3ModuleAddress;

/**
 * \brief A channel calibration.
 *
 * \see RBRGen3Channel
 * \see RBRGen3_setCalibration()
 */
typedef struct RBRGen3Calibration {
    /**
     * \brief The date/time of the calibration.
     *
     * Unused entries should be set to 0.
     */
    RBRGen3DateTime dateTime;

    /** \brief The number of c coefficients the equation uses. */
    int32_t cCount;

    /** \brief Calibration C coefficients. */
    float c[RBRGEN3_CALIBRATION_C_COEFFICIENT_MAX];

    /** \brief The number of x coefficients the equation uses. */
    int32_t xCount;

    /** \brief Calibration X coefficients. */
    float x[RBRGEN3_CALIBRATION_X_COEFFICIENT_MAX];

    /** \brief The number of input channel indices the equation uses. */
    int32_t nCount;

    /**
     * \brief Input channel indices.
     *
     * Entries corresponding to the special “value” value are set to
     * #RBRGEN3_VALUE_COEFFICIENT.
     */
    RBRGen3ChannelIndex n[RBRGEN3_CALIBRATION_N_COEFFICIENT_MAX];
} RBRGen3Calibration;

/**
 * \brief Possible channel gain ranging modes.
 *
 * \see RBRGen3Channel
 */
typedef enum RBRGen3ChannelRangingMode {
    /** No gain ranging is available. */
    RBRGEN3_RANGING_NONE,
    /** A fixed gain is used. */
    RBRGEN3_RANGING_MANUAL,
    /** The channel auto-ranges over the available gain settings. */
    RBRGEN3_RANGING_AUTO,
    /** The number of specific gain ranging modes. */
    RBRGEN3_RANGING_COUNT,
    /** An unknown or unrecognized gain ranging mode. */
    RBRGEN3_UNKNOWN_RANGING
} RBRGen3ChannelRangingMode;

/**
 * \brief Get a human-readable string name for a channel gain ranging mode.
 *
 * \param [in] mode the ranging mode
 * \return a string name for the ranging mode
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRGen3ChannelRangingMode_name(RBRGen3ChannelRangingMode mode);

/**
 * Gain parameters for a channel.
 *
 * \see RBRGen3Channel
 * \see RBRGen3_setChannelGain()
 */
typedef struct RBRGen3ChannelGain {
    /** \brief The gain selection mode employed by the sensor. */
    RBRGen3ChannelRangingMode rangingMode;

    /**
     * \brief The gain value in use by the sensor.
     *
     * Only applies when RBRGen3ChannelGain.rangingMode is
     * #RBRGEN3_RANGING_MANUAL. Otherwise set to NaN.
     */
    float currentGain;

    /**
     * \brief The gain settings supported by the sensor.
     *
     * Only applies where RBRGen3ChannelGain.rangingMode is
     * #RBRGEN3_RANGING_MANUAL or #RBRGEN3_RANGING_AUTO. Otherwise
     * all values are set to NaN.
     *
     * Unused entries are set to NaN.
     */
    float availableGains[RBRGEN3_CHANNEL_GAINS_MAX];
} RBRGen3ChannelGain;

/**
 * \brief Details reported by the instrument `channel` command.
 *
 * \see RBRGen3Channels
 */
typedef struct RBRGen3Channel {
    /**
     * \brief A short, pre-defined “generic” name for the installed channel as
     * a null-terminated C string.
     *
     * E.g., “temp09”, “pres19”, “cond05”.
     */
    char type[RBRGEN3_CHANNEL_TYPE_MAX + 1];

    /** \brief The internal address to which the channel responds. */
    RBRGen3ModuleAddress moduleAddr;

    /**
     * \brief Whether the channel is activated for sampling.
     *
     * \see RBRGen3_setChannelStatus()
     */
    bool status;

    /**
     * \brief The minimum power-on settling time required by this channel.
     *
     * Specified in milliseconds.
     */
    RBRGen3Period settlingTime;

    /**
     * \brief The typical data acquisition time required by this channel.
     *
     * Specified in milliseconds.
     */
    RBRGen3Period readTime;

    /**
     * \brief The type of formula used to convert raw readings to physical
     * measurement units as a null-terminated C string.
     */
    char equation[RBRGEN3_CALIBRATION_EQUATION_MAX + 1];

    /**
     * \brief The unit in which processed data is normally reported from the
     * logger as a null-terminated C string.
     *
     * E.g., “C” for Celsius, “V” for Volts, “dbar” for decibars.
     */
    char userUnits[RBRGEN3_CHANNEL_UNIT_MAX + 1];

    /** \brief Gain parameters for the channel. */
    RBRGen3ChannelGain gain;

    /** \brief Whether the channel is a derived channel. */
    bool derived;

    /**
     * \brief An alphanumeric description of the physical parameter measured as
     * a null-terminated C string.
     *
     * Set upon request for OEM customers. If not set, reported as “none”.
     *
     * \nol2 Always populated with “none”.
     */
    char label[RBRGEN3_CHANNEL_LABEL_MAX + 1];

    /** \brief The calibration for the channel. */
    RBRGen3Calibration calibration;
} RBRGen3Channel;

/**
 * \brief Details reported by a combination of the instrument `channels`,
 * `channel`, and `calibration` commands.
 *
 * \see RBRGen3_getChannels()
 * \see RBRGen3_getChannelsWithoutCalibrations()
 */
typedef struct RBRGen3Channels {
    /** \brief The number of channels RBRGen3Channels.channels can hold. */
    int32_t size;
    /**
     * \brief The number of channels stored in RBRGen3Channels.channels. Never
     * exceeds RBRGen3Channels.size.
     *
     * \see RBRGen3_getChannelCount() for the number the instrument has
     */
    int32_t len;
    /**
     * \brief The maximum power-on settling settling time across all enabled
     * channels.
     *
     * Specified in milliseconds.
     */
    RBRGen3Period settlingTime;
    /**
     * \brief The maximum overall reading time across all enabled channels.
     *
     * Specified in milliseconds.
     */
    RBRGen3Period readTime;
    /**
     * \brief The minimum sampling period with the currently-active channels.
     *
     * Specified in milliseconds.
     */
    RBRGen3Period minimumPeriod;
    /**
     * \brief Specific channel details, in caller-supplied storage.
     *
     * The first RBRGen3Channels.len entries are populated.
     */
    RBRGen3Channel *channels;
} RBRGen3Channels;

/**
 * \brief Get the number of channels installed in the instrument.
 *
 * \note Issues the `channels count` command.
 *
 * \param [in] conn the instrument connection
 * \param [out] count the number of installed and configured channels
 * \return #RBRGEN3_SUCCESS when the count is successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRGen3_getEnabledChannelCount()
 * \see RBRGen3_getChannels()
 */
RBRGen3Error RBRGen3_getChannelCount(RBRGen3 *conn, int32_t *count);

/**
 * \brief Get the number of enabled channels, which excludes any turned off by
 * the user.
 *
 * \note Issues the `channels on` command.
 *
 * \param [in] conn the instrument connection
 * \param [out] count the number of enabled channels
 * \return #RBRGEN3_SUCCESS when the count is successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRGen3_getChannelCount()
 * \see RBRGen3Channel.status
 */
RBRGen3Error RBRGen3_getEnabledChannelCount(RBRGen3 *conn, int32_t *count);

/**
 * \brief Get channel information for the instrument.
 *
 * Channel information is composed from a combination of the `channels`,
 * `channel`, and `calibration` commands. The information returned by this
 * function should comprise a complete model of an instrument's channels.
 *
 * \param [in] conn the instrument connection
 * \param [out] channels the channel information; RBRGen3Channels.channels
 *                       and RBRGen3Channels.size must be set by the caller
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TRUNCATED when the instrument has more channels than the
 *         list holds; the first RBRGen3Channels.size are populated and
 *         RBRGen3_getChannelCount() reports how many there are
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when the list has no storage
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRGen3_getChannelsWithoutCalibrations()
 * \see RBRGen3_getSensorParameters()
 */
RBRGen3Error RBRGen3_getChannels(RBRGen3 *conn, RBRGen3Channels *channels);

/**
 * \brief Get channel information for the instrument without calibration data.
 *
 * Channel information is composed from combining the `channels` and `channel`
 * commands. Unlike RBRGen3_getChannels(), calibration information is
 * _not_ populated. This saves bandwidth and time communicating with the
 * instrument when calibration information is unnecessary.
 *
 * \param [in] conn the instrument connection
 * \param [out] channels the channel information; RBRGen3Channels.channels
 *                       and RBRGen3Channels.size must be set by the caller
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TRUNCATED when the instrument has more channels than the
 *         list holds; the first RBRGen3Channels.size are populated and
 *         RBRGen3_getChannelCount() reports how many there are
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when the list has no storage
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRGen3_getChannels()
 * \see RBRGen3_getSensorParameters()
 */
RBRGen3Error RBRGen3_getChannelsWithoutCalibrations(RBRGen3 *conn, RBRGen3Channels *channels);

/**
 * \brief Set the status of a channel.
 *
 * \param [in] conn the instrument connection
 * \param [in] channel the index of the channel to update
 * \param [in] status whether the channel is activated for sampling
 * \return #RBRGEN3_SUCCESS when the setting is successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR if the instrument is logging, or another
 *                                 hardware error occurs
 * \see RBRGen3_getChannels()
 */
RBRGen3Error RBRGen3_setChannelStatus(RBRGen3 *conn, RBRGen3ChannelIndex channel, bool status);

/**
 * \brief Set the gain parameters of a channel.
 *
 * RBRGen3ChannelGain.rangingMode must be either
 * #RBRGEN3_RANGING_MANUAL or #RBRGEN3_RANGING_AUTO. Otherwise,
 * #RBRGEN3_INVALID_PARAMETER_VALUE is returned.
 *
 * For manual gain selection, the gain given by
 * RBRGen3ChannelGain.currentGain must be one of the available gains
 * reported by the instrument. Otherwise, the instrument will produce a
 * hardware error. If RBRGen3ChannelGain.availableGains is populated
 * (contains at least one leading non-NaN entry), this function will verify the
 * presence of the chosen gain. If it is not found,
 * #RBRGEN3_INVALID_PARAMETER_VALUE is returned.
 *
 * RBRGen3ChannelGain.availableGains is only used for parameter
 * verification. It is not sent to the instrument.
 *
 * \param [in] conn the instrument connection
 * \param [in] channel the index of the channel to update
 * \param [in] gain the gain parameters for the channel
 * \return #RBRGEN3_SUCCESS when the settings are successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR if the instrument is logging, or an invalid
 *                                 gain value is given, or another hardware
 *                                 error occurs
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE if the ranging mode is
 *                                                invalid, or if the gain value
 *                                                can be conclusively
 *                                                determined to be invalid
 * \see RBRGen3_getChannels()
 */
RBRGen3Error RBRGen3_setChannelGain(RBRGen3 *conn, RBRGen3ChannelIndex channel,
                                    RBRGen3ChannelGain *gain);

/**
 * \brief Update a channel's calibration coefficients.
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - you set an out-of-range or incorrect coefficient
 * - you set too many coefficients
 * - you set the wrong types of coefficients
 *
 * If coefficients used by the channel are omitted from the set sent, then
 * those coefficients will retain their current values. You can call
 * RBRGen3_getChannels() after updating coefficients to confirm the
 * values written.
 *
 * Sends RBRGen3Calibration.cCount c and RBRGen3Calibration.xCount x
 * coefficients. The _n_ coefficient group (RBRGen3Calibration.nCount and
 * RBRGen3Calibration.n) is ignored.
 *
 * \param [in] conn the instrument connection
 * \param [in] channel the index of the channel to update
 * \param [in] calibration the new calibration coefficients for the channel
 * \return #RBRGEN3_SUCCESS when the setting is successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the calibration cannot be changed, or
 *                                 another hardware error occurs
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when the date/time of the
 *                                                calibration is out of range,
 *                                                when a count is out of range,
 *                                                or when no coefficients are
 *                                                given
 * \see RBRGen3_getChannels()
 */
RBRGen3Error RBRGen3_setCalibration(RBRGen3 *conn, RBRGen3ChannelIndex channel,
                                    const RBRGen3Calibration *calibration);

/**
 * \brief Get the fetch power-off delay.
 *
 * The fetch power-off delay delay in milliseconds between the successful
 * completion of a fetch command and power to the front end sensors being
 * removed by the instrument.
 *
 * \param [in] conn the instrument connection
 * \param [out] fetchPowerOffDelay the fetch power-off delay
 * \return #RBRGEN3_SUCCESS when the setting is successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRGen3_setFetchPowerOffDelay()
 */
RBRGen3Error RBRGen3_getFetchPowerOffDelay(RBRGen3 *conn, RBRGen3Period *fetchPowerOffDelay);

/**
 * \brief Set the fetch power-off delay.
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - you set an out-of-bounds time the library fails to detect
 *
 * \param [in] conn the instrument connection
 * \param [in] fetchPowerOffDelay the fetch power-off delay
 * \return #RBRGEN3_SUCCESS when the setting is successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the settings cannot be changed, or
 *                                 another hardware error occurs
 * \see RBRGen3_getFetchPowerOffDelay()
 */
RBRGen3Error RBRGen3_setFetchPowerOffDelay(RBRGen3 *conn, RBRGen3Period fetchPowerOffDelay);

/**
 * \brief Get whether sensor power is always on.
 *
 * The instrument does not have to power down front end sensors between
 * samples. This can be useful for sensors with very long power-on
 * stabilization times.
 *
 * \param [in] conn the instrument connection
 * \param [out] sensorPowerAlwaysOn whether sensor power is always on
 * \return #RBRGEN3_SUCCESS when the setting is successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRGen3_setSensorPowerAlwaysOn()
 */
RBRGen3Error RBRGen3_isSensorPowerAlwaysOn(RBRGen3 *conn, bool *sensorPowerAlwaysOn);

/**
 * \brief Set whether sensor power is always on.
 *
 * A hardware error will occur if the instrument is logging.
 *
 * \param [in] conn the instrument connection
 * \param [in] sensorPowerAlwaysOn whether sensor power is always on
 * \return #RBRGEN3_SUCCESS when the setting is successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR if the instrument is logging, or another
 *                                 hardware error occurs
 * \see RBRGen3_isSensorPowerAlwaysOn()
 */
RBRGen3Error RBRGen3_setSensorPowerAlwaysOn(RBRGen3 *conn, bool sensorPowerAlwaysOn);

/**
 * \brief Get whether cast detection is enabled.
 *
 * The instrument can automatically detect upcasts and downcasts and generate
 * cast detection events in the datastream.
 *
 * \param [in] conn the instrument connection
 * \param [out] castDetection whether cast detection is enabled
 * \return #RBRGEN3_SUCCESS when the setting is successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRGen3_setCastDetection()
 */
RBRGen3Error RBRGen3_getCastDetection(RBRGen3 *conn, bool *castDetection);

/**
 * \brief Set whether cast detection is enabled.
 *
 * A hardware error will occur if the instrument is logging.
 *
 * \param [in] conn the instrument connection
 * \param [in] castDetection whether cast detection is enabled
 * \return #RBRGEN3_SUCCESS when the setting is successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR if the instrument is logging, or another
 *                                 hardware error occurs
 * \see RBRGen3_getCastDetection()
 */
RBRGen3Error RBRGen3_setCastDetection(RBRGen3 *conn, bool castDetection);

/**
 * \brief Get the timeout for output suppression while receiving commands.
 *
 * Specified in milliseconds. Must be between 10,000 and 240,000,
 * inclusive; partial seconds are rounded up to the next whole second by the
 * instrument.
 *
 * \param [in] conn the instrument connection
 * \param [out] inputTimeout the timeout for output suppression
 * \return #RBRGEN3_SUCCESS when the setting is successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRGen3_setInputTimeout()
 */
RBRGen3Error RBRGen3_getInputTimeout(RBRGen3 *conn, RBRGen3Period *inputTimeout);

/**
 * \brief Set the timeout for output suppression while receiving commands.
 *
 * Must be between 10,000 and 240,000, inclusive; partial seconds are rounded
 * up to the next whole second.
 *
 * A hardware error will occur if the instrument is logging.
 *
 * \param [in] conn the instrument connection
 * \param [in] inputTimeout the timeout for output suppression
 * \return #RBRGEN3_SUCCESS when the setting is successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR if the instrument is logging, or another
 *                                 hardware error occurs
 * \see RBRGen3_getInputTimeout()
 */
RBRGen3Error RBRGen3_setInputTimeout(RBRGen3 *conn, RBRGen3Period inputTimeout);

/**
 * \brief Value settings the instrument uses for calculation of derived
 * channels, or as defaults for physical parameters the instrument does not
 * measure.
 *
 * See the command reference documentation for details on the use and default
 * values for each of these parameters.
 *
 * \see RBRGen3_getValueSetting()
 * \see RBRGen3_setValueSetting()
 */
typedef enum RBRGen3ValueSetting {
    /**
     * The temperature coefficient used to correct the derived channel for
     * specific conductivity to 25°C.
     *
     * Specified in degrees Celsius.
     */
    RBRGEN3_SETTING_SPECCONDTEMPCO,
    /**
     * The height above the seabed at which the logger is deployed.
     *
     * Specified in metres.
     */
    RBRGEN3_SETTING_ALTITUDE,
    /**
     * The default temperature.
     *
     * Specified in degrees Celsius.
     */
    RBRGEN3_SETTING_TEMPERATURE,
    /**
     * The default absolute pressure.
     *
     * Specified in dbar.
     */
    RBRGEN3_SETTING_PRESSURE,
    /**
     * The default conductivity.
     *
     * Specified in ms/cm².
     *
     * \nol3 It is only available on early Logger2 instruments.
     */
    RBRGEN3_SETTING_CONDUCTIVITY,
    /**
     * The default atmospheric pressure.
     *
     * Specified in dbar.
     */
    RBRGEN3_SETTING_ATMOSPHERE,
    /**
     * The default water density.
     *
     * Specified in g/cm³.
     */
    RBRGEN3_SETTING_DENSITY,
    /**
     * The default salinity.
     *
     * Specified in PSU.
     */
    RBRGEN3_SETTING_SALINITY,
    /**
     * The default average speed of sound.
     *
     * Specified in m/s.
     */
    RBRGEN3_SETTING_AVGSOUNDSPEED,
    /** The number of specific value settings. */
    RBRGEN3_SETTING_COUNT,
    /** An unknown or unrecognized value setting. */
    RBRGEN3_UNKNOWN_SETTING
} RBRGen3ValueSetting;

/**
 * \brief Get a human-readable string name for an instrument value setting.
 *
 * \param [in] setting the value setting
 * \return a string name for the value setting
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRGen3ValueSetting_name(RBRGen3ValueSetting setting);

/**
 * \brief Read a value setting from the instrument.
 *
 * \param [in] conn the instrument connection
 * \param [in] setting the setting to retrieve
 * \param [out] value the value of the setting
 * \return #RBRGEN3_SUCCESS when the setting is successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when an unrecognized setting
 *                                                is requested
 * \see RBRGen3_setValueSetting()
 */
RBRGen3Error RBRGen3_getValueSetting(RBRGen3 *conn, RBRGen3ValueSetting setting, float *value);

/**
 * \brief Write the a value setting to the instrument.
 *
 * A hardware error will occur if the instrument is logging.
 *
 * \param [in] conn the instrument connection
 * \param [in] setting the setting to retrieve
 * \param [in] value the value of the setting
 * \return #RBRGEN3_SUCCESS when the setting is successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR if the instrument is logging, or another
 *                                 hardware error occurs
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when an unrecognized setting
 *                                                is requested or when the
 *                                                value is NaN
 * \see RBRGen3_getValueSetting()
 */
RBRGen3Error RBRGen3_setValueSetting(RBRGen3 *conn, RBRGen3ValueSetting setting, float value);

/**
 * \brief A sensor parameter.
 *
 * \see RBRGen3_getSensorParameter()
 * \see RBRGen3_getSensorParameters()
 * \see RBRGen3_setSensorParameter()
 */
typedef struct RBRGen3SensorParameter {
    /** \brief The name of the parameter as a null-terminated C string. */
    char key[RBRGEN3_SENSOR_PARAMETER_KEY_MAX + 1];
    /** \brief The parameter value as a null-terminated C string. */
    char value[RBRGEN3_SENSOR_PARAMETER_VALUE_MAX + 1];
} RBRGen3SensorParameter;

/**
 * \brief Retrieve a single sensor parameter for a channel.
 *
 * If the parameter is not configured, its value is set to “n/a”. This is the
 * native behaviour of Logger3 instruments. However, at a hardware level,
 * attempting to retrieve a nonexistent sensor parameter from a Logger2
 * instrument results in the error “E0501 item is not configured”. To simplify
 * things for users, the library emulates the Logger3 behaviour for Logger2
 * instruments.
 *
 * \param conn the instrument connection
 * \param [in] channel the index of the channel from which the parameter is to
 *                     be retrieved
 * \param [in,out] parameter initially, the sensor parameter to be retrieved;
 *                           after return, the instrument response
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRGen3_getChannels()
 * \see RBRGen3_getSensorParameters()
 * \see RBRGen3_setSensorParameter()
 */
RBRGen3Error RBRGen3_getSensorParameter(RBRGen3 *conn, RBRGen3ChannelIndex channel,
                                        RBRGen3SensorParameter *parameter);

/**
 * \brief Retrieve the sensor parameters for a channel.
 *
 * To ease memory requirements, sensor parameters are not included with other
 * channel information retrieved by RBRGen3_getChannels().
 *
 * \param conn the instrument connection
 * \param [in] channel the index of the channel for which sensor parameters are
 *                     to be retrieved
 * \param [out] parameters the sensor parameters for the channel
 * \param [in,out] size initially, the maximum number of elements which can be
 *                      written to \a parameters; after return, the number of
 *                      parameters actually written
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRGen3_getChannels()
 * \see RBRGen3_getSensorParameter()
 * \see RBRGen3_setSensorParameter()
 */
RBRGen3Error RBRGen3_getSensorParameters(RBRGen3 *conn, RBRGen3ChannelIndex channel,
                                         RBRGen3SensorParameter *parameters, int32_t *size);

/**
 * \brief Set a sensor parameter for a channel.
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - the parameter name has not been defined at the RBR factory
 *
 * \param conn the instrument connection
 * \param [in] channel the index of the channel the sensor parameter of which
 *                     is to be updated
 * \param [in] parameter the sensor parameter for the channel
 * \return #RBRGEN3_SUCCESS when the setting is successfully written
 * \return #RBRGEN3_COMMAND_TOO_LONG when the command does not fit the
 *         command buffer
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the parameter cannot be changed, or
 *                                 another hardware error occurs
 * \see RBRGen3_getChannels()
 * \see RBRGen3_getSensorParameter()
 * \see RBRGen3_getSensorParameters()
 */
RBRGen3Error RBRGen3_setSensorParameter(RBRGen3 *conn, RBRGen3ChannelIndex channel,
                                        RBRGen3SensorParameter *parameter);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN3CONFIGURATION_H */
