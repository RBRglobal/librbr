/**
 * \file RBRInstrumentConfiguration.h
 *
 * \brief Instrument commands and structures pertaining to instrument
 * configuration information and calibration.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTCONFIGURATION_H
#define LIBRBR_RBRINSTRUMENTCONFIGURATION_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief The maximum number of C calibration coefficients.
 *
 * \see RBRInstrumentCalibration.c
 */
#define RBRINSTRUMENT_CALIBRATION_C_COEFFICIENT_MAX 8

/**
 * \brief The maximum number of X calibration coefficients.
 *
 * \see RBRInstrumentCalibration.x
 */
#define RBRINSTRUMENT_CALIBRATION_X_COEFFICIENT_MAX 16

/**
 * \brief The maximum number of input channel indices.
 *
 * \see RBRInstrumentCalibration.n
 */
#define RBRINSTRUMENT_CALIBRATION_N_COEFFICIENT_MAX 8

/**
 * \brief An in-band representation of the special “value” calibration
 * correction value.
 *
 * \see RBRInstrumentCalibration.n
 */
#define RBRINSTRUMENT_VALUE_COEFFICIENT 0xFF

/**
 * \brief The maximum number of characters in a calibration equation name.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENT_CALIBRATION_EQUATION_MAX 31

/**
 * \brief The maximum number of gain settings for a channel.
 */
#define RBRINSTRUMENT_CHANNEL_GAINS_MAX 8

/** \brief The minimum input timeout. */
#define RBRINSTRUMENT_INPUT_TIMEOUT_MIN 10000

/** \brief The maximum input timeout. */
#define RBRINSTRUMENT_INPUT_TIMEOUT_MAX 240000

/**
 * \brief The maximum number of characters in a sensor parameter key.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENT_SENSOR_PARAMETER_KEY_MAX 63

/**
 * \brief The maximum number of characters in a sensor parameter value.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENT_SENSOR_PARAMETER_VALUE_MAX 63

/**
 * \brief A channel identifier.
 *
 * Channel indices are always 1-based. A value of 0 means the index is unset
 * or empty.
 */
typedef uint8_t RBRInstrumentChannelIndex;

/** \brief An internal module identifier. */
typedef uint8_t RBRInstrumentModuleAddress;

/**
 * \brief A channel calibration.
 *
 * \see RBRInstrumentChannel
 * \see RBRInstrument_setCalibration()
 */
typedef struct RBRInstrumentCalibration
{
    /**
     * \brief The date/time of the calibration.
     *
     * Unused entries should be set to 0.
     */
    RBRInstrumentDateTime dateTime;
    /**
     *\brief Calibration C coefficients.
     *
     * Unused entries should be set to NaN.
     */
    float c[RBRINSTRUMENT_CALIBRATION_C_COEFFICIENT_MAX];
    /**
     *\brief Calibration X coefficients.
     *
     * Unused entries should be set to NaN.
     */
    float x[RBRINSTRUMENT_CALIBRATION_X_COEFFICIENT_MAX];
    /**
     *\brief Input channel indices.
     *
     * Unused entries should be set to 0. Entries corresponding to the special
     * “value” value are set to #RBRINSTRUMENT_VALUE_COEFFICIENT.
     */
    RBRInstrumentChannelIndex n[RBRINSTRUMENT_CALIBRATION_N_COEFFICIENT_MAX];
} RBRInstrumentCalibration;

/**
 * \brief Possible channel gain ranging modes.
 *
 * \see RBRInstrumentChannel
 * \see https://docs.rbr-global.com/L3commandreference/format-of-stored-data/standard-rawbin00-format/deployment-header/version-2-001
 */
typedef enum RBRInstrumentChannelRangingMode
{
    /** No gain ranging is available. */
    RBRINSTRUMENT_RANGING_NONE,
    /** A fixed gain is used. */
    RBRINSTRUMENT_RANGING_MANUAL,
    /** The channel auto-ranges over the available gain settings. */
    RBRINSTRUMENT_RANGING_AUTO,
    /** The number of specific gain ranging modes. */
    RBRINSTRUMENT_RANGING_COUNT,
    /** An unknown or unrecognized gain ranging mode. */
    RBRINSTRUMENT_UNKNOWN_RANGING
} RBRInstrumentChannelRangingMode;

/**
 * \brief Get a human-readable string name for a channel gain ranging mode.
 *
 * \param [in] mode the ranging mode
 * \return a string name for the ranging mode
 * \see RBRInstrumentError_name() for a description of the format of names
 */
const char *RBRInstrumentChannelRangingMode_name(
    RBRInstrumentChannelRangingMode mode);

/**
 * Gain parameters for a channel.
 *
 * \see RBRInstrumentChannel
 * \see RBRInstrument_setChannelGain()
 */
typedef struct RBRInstrumentChannelGain
{
    /** \brief The gain selection mode employed by the sensor. */
    RBRInstrumentChannelRangingMode rangingMode;

    /**
     * \brief The gain value in use by the sensor.
     *
     * Only applies when RBRInstrumentChannelGain.rangingMode is
     * #RBRINSTRUMENT_RANGING_MANUAL. Otherwise set to NaN.
     */
    float currentGain;

    /**
     * \brief The gain settings supported by the sensor.
     *
     * Only applies where RBRInstrumentChannelGain.rangingMode is
     * #RBRINSTRUMENT_RANGING_MANUAL or #RBRINSTRUMENT_RANGING_AUTO. Otherwise
     * all values are set to NaN.
     *
     * Unused entries are set to NaN.
     */
    float availableGains[RBRINSTRUMENT_CHANNEL_GAINS_MAX];
} RBRInstrumentChannelGain;

/**
 * \brief Details reported by the instrument `channel` command.
 *
 * \see RBRInstrumentChannels
 */
typedef struct RBRInstrumentChannel
{
    /**
     * \brief A short, pre-defined “generic” name for the installed channel as
     * a null-terminated C string.
     *
     * E.g., “temp09”, “pres19”, “cond05”.
     *
     * \see https://docs.rbr-global.com/L3commandreference/supported-channel-types
     */
    char type[RBRINSTRUMENT_CHANNEL_TYPE_MAX + 1];

    /** \brief The internal address to which the channel responds. */
    RBRInstrumentModuleAddress module;

    /**
     * \brief Whether the channel is activated for sampling.
     *
     * \see RBRInstrument_setChannelStatus()
     */
    bool status;

    /**
     * \brief The minimum power-on settling time required by this channel.
     *
     * Specified in milliseconds.
     */
    RBRInstrumentPeriod settlingTime;

    /**
     * \brief The typical data acquisition time required by this channel.
     *
     * Specified in milliseconds.
     */
    RBRInstrumentPeriod readTime;

    /**
     * \brief The type of formula used to convert raw readings to physical
     * measurement units as a null-terminated C string.
     *
     * \see https://docs.rbr-global.com/L3commandreference/calibration-equations-and-cross-channel-dependencies
     */
    char equation[RBRINSTRUMENT_CALIBRATION_EQUATION_MAX + 1];

    /**
     * \brief The unit in which processed data is normally reported from the
     * logger as a null-terminated C string.
     *
     * E.g., “C” for Celsius, “V” for Volts, “dbar” for decibars.
     */
    char userUnits[RBRINSTRUMENT_CHANNEL_UNIT_MAX + 1];

    /** \brief Gain parameters for the channel. */
    RBRInstrumentChannelGain gain;

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
    char label[RBRINSTRUMENT_CHANNEL_LABEL_MAX + 1];

    /** \brief The calibration for the channel. */
    RBRInstrumentCalibration calibration;
} RBRInstrumentChannel;

/**
 * \brief Details reported by a combination of the instrument `channels`,
 * `channel`, and `calibration` commands.
 *
 * \see RBRInstrument_getChannels()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/channels
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/channel
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/calibration
 */
typedef struct RBRInstrumentChannels
{
    /** \brief The number of installed and configured instrument channels. */
    int32_t count;
    /**
     * \brief The number of active channels, which excludes any turned off by
     * the user.
     *
     * \see RBRInstrumentChannel.status
     */
    int32_t on;
    /**
     * \brief The maximum power-on settling settling time across all enabled
     * channels.
     *
     * Specified in milliseconds.
     */
    RBRInstrumentPeriod settlingTime;
    /**
     * \brief The maximum overall reading time across all enabled channels.
     *
     * Specified in milliseconds.
     */
    RBRInstrumentPeriod readTime;
    /**
     * \brief The minimum sampling period with the currently-active channels.
     *
     * Specified in milliseconds.
     */
    RBRInstrumentPeriod minimumPeriod;
    /**
     * \brief Specific channel details.
     *
     * The first RBRInstrumentChannel.count entries will be populated.
     */
    RBRInstrumentChannel channels[RBRINSTRUMENT_CHANNEL_MAX];
} RBRInstrumentChannels;

/**
 * \brief Get channel information for the instrument.
 *
 * Channel information is composed from a combination of the `channels`,
 * `channel`, and `calibration` commands. The information returned by this
 * function should comprise a complete model of an instrument's channels.
 *
 * \param [in] instrument the instrument connection
 * \param [out] channels the channel information
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_getSensorParameters()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/channels
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/channel
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/calibration
 */
RBRInstrumentError RBRInstrument_getChannels(RBRInstrument *instrument,
                                             RBRInstrumentChannels *channels);

/**
 * \brief Set the status of a channel.
 *
 * \param [in] instrument the instrument connection
 * \param [in] channel the index of the channel to update
 * \param [in] status whether the channel is activated for sampling
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument is logging
 * \see RBRInstrument_getChannels()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/channel
 */
RBRInstrumentError RBRInstrument_setChannelStatus(
    RBRInstrument *instrument,
    RBRInstrumentChannelIndex channel,
    bool status);

/**
 * \brief Set the gain parameters of a channel.
 *
 * RBRInstrumentChannelGain.rangingMode must be either
 * #RBRINSTRUMENT_RANGING_MANUAL or #RBRINSTRUMENT_RANGING_AUTO. Otherwise,
 * #RBRINSTRUMENT_INVALID_PARAMETER_VALUE is returned.
 *
 * For manual gain selection, the gain given by
 * RBRInstrumentChannelGain.currentGain must be one of the available gains
 * reported by the instrument. Otherwise, the instrument will produce a
 * hardware error. If RBRInstrumentChannelGain.availableGains is populated
 * (contains at least one leading non-NaN entry), this function will verify the
 * presence of the chosen gain. If it is not found,
 * #RBRINSTRUMENT_INVALID_PARAMETER_VALUE is returned.
 *
 * RBRInstrumentChannelGain.availableGains is only used for parameter
 * verification. It is not sent to the instrument.
 *
 * \param [in] instrument the instrument connection
 * \param [in] channel the index of the channel to update
 * \param [in] gain the gain parameters for the channel
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument is logging, or an
 *                                       invalid gain value is given
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE if the ranging mode is
 *                                                invalid, or if the gain value
 *                                                can be conclusively
 *                                                determined to be invalid
 * \see RBRInstrument_getChannels()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/channel
 */
RBRInstrumentError RBRInstrument_setChannelGain(
    RBRInstrument *instrument,
    RBRInstrumentChannelIndex channel,
    RBRInstrumentChannelGain *gain);

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
 * RBRInstrument_getChannels() after updating coefficients to confirm the
 * values written.
 *
 * Values of in the _n_ coefficient group (RBRInstrumentCalibration.n) are
 * ignored.
 *
 * \param [in] instrument the instrument connection
 * \param [in] channel the index of the channel to update
 * \param [in] calibration the new calibration coefficients for the channel
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the calibration cannot be changed
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when no coefficients are
 *                                                populated
 * \see RBRInstrument_getChannels()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/calibration
 */
RBRInstrumentError RBRInstrument_setCalibration(
    RBRInstrument *instrument,
    RBRInstrumentChannelIndex channel,
    const RBRInstrumentCalibration *calibration);

/**
 * \brief Get the fetch power-off delay.
 *
 * The fetch power-off delay delay in milliseconds between the successful
 * completion of a fetch command and power to the front end sensors being
 * removed by the instrument.
 *
 * \param [in] instrument the instrument connection
 * \param [out] fetchPowerOffDelay the fetch power-off delay
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_setFetchPowerOffDelay()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
RBRInstrumentError RBRInstrument_getFetchPowerOffDelay(
    RBRInstrument *instrument,
    RBRInstrumentPeriod *fetchPowerOffDelay);

/**
 * \brief Set the fetch power-off delay.
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - you set an out-of-bounds time the library fails to detect
 *
 * \param [in] instrument the instrument connection
 * \param [in] fetchPowerOffDelay the fetch power-off delay
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the settings cannot be changed
 * \see RBRInstrument_getFetchPowerOffDelay()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
RBRInstrumentError RBRInstrument_setFetchPowerOffDelay(
    RBRInstrument *instrument,
    RBRInstrumentPeriod fetchPowerOffDelay);

/**
 * \brief Get whether sensor power is always on.
 *
 * The instrument does not have to power down front end sensors between
 * samples. This can be useful for sensors with very long power-on
 * stabilization times.
 *
 * \param [in] instrument the instrument connection
 * \param [out] sensorPowerAlwaysOn whether sensor power is always on
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_setSensorPowerAlwaysOn()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
RBRInstrumentError RBRInstrument_isSensorPowerAlwaysOn(
    RBRInstrument *instrument,
    bool *sensorPowerAlwaysOn);

/**
 * \brief Set whether sensor power is always on.
 *
 * A hardware error will occur if the instrument is logging.
 *
 * \param [in] instrument the instrument connection
 * \param [in] sensorPowerAlwaysOn whether sensor power is always on
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument is logging
 * \see RBRInstrument_isSensorPowerAlwaysOn()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
RBRInstrumentError RBRInstrument_setSensorPowerAlwaysOn(
    RBRInstrument *instrument,
    bool sensorPowerAlwaysOn);

/**
 * \brief Get whether cast detection is enabled.
 *
 * The instrument can automatically detect upcasts and downcasts and generate
 * cast detection events in the datastream.
 *
 * \param [in] instrument the instrument connection
 * \param [out] castDetection whether cast detection is enabled
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_setCastDetection()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
RBRInstrumentError RBRInstrument_getCastDetection(RBRInstrument *instrument,
                                                  bool *castDetection);

/**
 * \brief Set whether cast detection is enabled.
 *
 * A hardware error will occur if the instrument is logging.
 *
 * \param [in] instrument the instrument connection
 * \param [in] castDetection whether cast detection is enabled
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument is logging
 * \see RBRInstrument_getCastDetection()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
RBRInstrumentError RBRInstrument_setCastDetection(RBRInstrument *instrument,
                                                  bool castDetection);

/**
 * \brief Get the timeout for output suppression while receiving commands.
 *
 * Specified in milliseconds. Must be between 10,000 and 240,000,
 * inclusive; partial seconds are rounded up to the next whole second by the
 * instrument.
 *
 * \param [in] instrument the instrument connection
 * \param [out] inputTimeout the timeout for output suppression
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_setInputTimeout()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
RBRInstrumentError RBRInstrument_getInputTimeout(
    RBRInstrument *instrument,
    RBRInstrumentPeriod *inputTimeout);

/**
 * \brief Set the timeout for output suppression while receiving commands.
 *
 * Must be between 10,000 and 240,000, inclusive; partial seconds are rounded
 * up to the next whole second.
 *
 * A hardware error will occur if the instrument is logging.
 *
 * \param [in] instrument the instrument connection
 * \param [in] inputTimeout the timeout for output suppression
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument is logging
 * \see RBRInstrument_getInputTimeout()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
RBRInstrumentError RBRInstrument_setInputTimeout(
    RBRInstrument *instrument,
    RBRInstrumentPeriod inputTimeout);

/**
 * \brief Value settings the instrument uses for calculation of derived
 * channels, or as defaults for physical parameters the instrument does not
 * measure.
 *
 * See the command reference documentation for details on the use and default
 * values for each of these parameters.
 *
 * \see RBRInstrument_getValueSetting()
 * \see RBRInstrument_setValueSetting()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
typedef enum RBRInstrumentValueSetting
{
    /**
     * The temperature coefficient used to correct the derived channel for
     * specific conductivity to 25°C.
     *
     * Specified in degrees Celsius.
     */
    RBRINSTRUMENT_SETTING_SPECCONDTEMPCO,
    /**
     * The height above the seabed at which the logger is deployed.
     *
     * Specified in metres.
     */
    RBRINSTRUMENT_SETTING_ALTITUDE,
    /**
     * The default temperature.
     *
     * Specified in degrees Celsius.
     */
    RBRINSTRUMENT_SETTING_TEMPERATURE,
    /**
     * The default absolute pressure.
     *
     * Specified in dbar.
     */
    RBRINSTRUMENT_SETTING_PRESSURE,
    /**
     * The default conductivity.
     *
     * Specified in ms/cm².
     *
     * \nol3 It is only available on early Logger2 instruments.
     */
    RBRINSTRUMENT_SETTING_CONDUCTIVITY,
    /**
     * The default atmospheric pressure.
     *
     * Specified in dbar.
     */
    RBRINSTRUMENT_SETTING_ATMOSPHERE,
    /**
     * The default water density.
     *
     * Specified in g/cm³.
     */
    RBRINSTRUMENT_SETTING_DENSITY,
    /**
     * The default salinity.
     *
     * Specified in PSU.
     */
    RBRINSTRUMENT_SETTING_SALINITY,
    /**
     * The default average speed of sound.
     *
     * Specified in m/s.
     */
    RBRINSTRUMENT_SETTING_AVGSOUNDSPEED,
    /** The number of specific value settings. */
    RBRINSTRUMENT_SETTING_COUNT,
    /** An unknown or unrecognized value setting. */
    RBRINSTRUMENT_UNKNOWN_SETTING
} RBRInstrumentValueSetting;

/**
 * \brief Get a human-readable string name for an instrument value setting.
 *
 * \param [in] setting the value setting
 * \return a string name for the value setting
 * \see RBRInstrumentError_name() for a description of the format of names
 */
const char *RBRInstrumentValueSetting_name(RBRInstrumentValueSetting setting);

/**
 * \brief Read a value setting from the instrument.
 *
 * \param [in] instrument the instrument connection
 * \param [in] setting the setting to retrieve
 * \param [out] value the value of the setting
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when an unrecognized setting
 *                                                is requested
 * \see RBRInstrument_setValueSetting()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
RBRInstrumentError RBRInstrument_getValueSetting(
    RBRInstrument *instrument,
    RBRInstrumentValueSetting setting,
    float *value);

/**
 * \brief Write the a value setting to the instrument.
 *
 * A hardware error will occur if the instrument is logging.
 *
 * \param [in] instrument the instrument connection
 * \param [in] setting the setting to retrieve
 * \param [in] value the value of the setting
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument is logging
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when an unrecognized setting
 *                                                is requested or when the
 *                                                value is NaN
 * \see RBRInstrument_getValueSetting()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
RBRInstrumentError RBRInstrument_setValueSetting(
    RBRInstrument *instrument,
    RBRInstrumentValueSetting setting,
    float value);

/**
 * \brief A sensor parameter.
 *
 * \see RBRInstrument_getSensorParameter()
 * \see RBRInstrument_getSensorParameters()
 * \see RBRInstrument_setSensorParameter()
 */
typedef struct RBRInstrumentSensorParameter
{
    /** \brief The name of the parameter as a null-terminated C string. */
    char key[RBRINSTRUMENT_SENSOR_PARAMETER_KEY_MAX + 1];
    /** \brief The parameter value as a null-terminated C string. */
    char value[RBRINSTRUMENT_SENSOR_PARAMETER_VALUE_MAX + 1];
} RBRInstrumentSensorParameter;

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
 * \param instrument the instrument connection
 * \param [in] channel the index of the channel from which the parameter is to
 *                     be retrieved
 * \param [in,out] parameter initially, the sensor parameter to be retrieved;
 *                           after return, the instrument response
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_getChannels()
 * \see RBRInstrument_getSensorParameters()
 * \see RBRInstrument_setSensorParameter()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/sensor
 */
RBRInstrumentError RBRInstrument_getSensorParameter(
    RBRInstrument *instrument,
    RBRInstrumentChannelIndex channel,
    RBRInstrumentSensorParameter *parameter);

/**
 * \brief Retrieve the sensor parameters for a channel.
 *
 * To ease memory requirements, sensor parameters are not included with other
 * channel information retrieved by RBRInstrument_getChannels().
 *
 * \param instrument the instrument connection
 * \param [in] channel the index of the channel for which sensor parameters are
 *                     to be retrieved
 * \param [out] parameters the sensor parameters for the channel
 * \param [in,out] size initially, the maximum number of elements which can be
 *                      written to \a parameters; after return, the number of
 *                      parameters actually written
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_getChannels()
 * \see RBRInstrument_getSensorParameter()
 * \see RBRInstrument_setSensorParameter()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/sensor
 */
RBRInstrumentError RBRInstrument_getSensorParameters(
    RBRInstrument *instrument,
    RBRInstrumentChannelIndex channel,
    RBRInstrumentSensorParameter *parameters,
    int32_t *size);

/**
 * \brief Set a sensor parameter for a channel.
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - the parameter name has not been defined at the RBR factory
 *
 * \param instrument the instrument connection
 * \param [in] channel the index of the channel the sensor parameter of which
 *                     is to be updated
 * \param [in] parameter the sensor parameter for the channel
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the parameter cannot be changed
 * \see RBRInstrument_getChannels()
 * \see RBRInstrument_getSensorParameter()
 * \see RBRInstrument_getSensorParameters()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/sensor
 */
RBRInstrumentError RBRInstrument_setSensorParameter(
    RBRInstrument *instrument,
    RBRInstrumentChannelIndex channel,
    RBRInstrumentSensorParameter *parameter);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTCONFIGURATION_H */
