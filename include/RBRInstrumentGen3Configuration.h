/**
 * \file RBRInstrumentGen3Configuration.h
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

#ifndef LIBRBR_RBRINSTRUMENTGEN3CONFIGURATION_H
#define LIBRBR_RBRINSTRUMENTGEN3CONFIGURATION_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief The maximum number of C calibration coefficients per channel.
 *
 * \see RBRInstrumentGen3Calibration.c
 */
#ifndef RBRINSTRUMENTGEN3_CALIBRATION_C_COEFFICIENT_MAX
#define RBRINSTRUMENTGEN3_CALIBRATION_C_COEFFICIENT_MAX 8
#endif

/**
 * \brief The maximum number of X calibration coefficients per channel.
 *
 * \see RBRInstrumentGen3Calibration.x
 */
#ifndef RBRINSTRUMENTGEN3_CALIBRATION_X_COEFFICIENT_MAX
#define RBRINSTRUMENTGEN3_CALIBRATION_X_COEFFICIENT_MAX 16
#endif

/**
 * \brief The maximum number of input channel indices per channel.
 *
 * \see RBRInstrumentGen3Calibration.n
 */
#ifndef RBRINSTRUMENTGEN3_CALIBRATION_N_COEFFICIENT_MAX
#define RBRINSTRUMENTGEN3_CALIBRATION_N_COEFFICIENT_MAX 8
#endif

/**
 * \brief An in-band representation of the special “value” calibration
 * correction value.
 *
 * \see RBRInstrumentGen3Calibration.n
 */
#define RBRINSTRUMENTGEN3_VALUE_COEFFICIENT 0xFF

/**
 * \brief The maximum number of characters in a calibration equation name.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENTGEN3_CALIBRATION_EQUATION_MAX 31

/**
 * \brief The maximum number of gain settings per channel.
 */
#ifndef RBRINSTRUMENTGEN3_CHANNEL_GAINS_MAX
#define RBRINSTRUMENTGEN3_CHANNEL_GAINS_MAX 8
#endif

/** \brief The minimum input timeout. */
#define RBRINSTRUMENTGEN3_INPUT_TIMEOUT_MIN 10000

/** \brief The maximum input timeout. */
#define RBRINSTRUMENTGEN3_INPUT_TIMEOUT_MAX 240000

/**
 * \brief The maximum number of characters in a sensor parameter key.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENTGEN3_SENSOR_PARAMETER_KEY_MAX 63

/**
 * \brief The maximum number of characters in a sensor parameter value.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENTGEN3_SENSOR_PARAMETER_VALUE_MAX 63

/**
 * \brief A channel identifier.
 *
 * Channel indices are always 1-based. A value of 0 means the index is unset
 * or empty.
 */
typedef uint8_t RBRInstrumentGen3ChannelIndex;

/** \brief An internal module identifier. */
typedef uint8_t RBRInstrumentGen3ModuleAddress;

/**
 * \brief A channel calibration.
 *
 * \see RBRInstrumentGen3Channel
 * \see RBRInstrumentGen3_setCalibration()
 */
typedef struct RBRInstrumentGen3Calibration
{
    /**
     * \brief The date/time of the calibration.
     *
     * Unused entries should be set to 0.
     */
    RBRInstrumentGen3DateTime dateTime;
    /**
     *\brief Calibration C coefficients.
     *
     * Unused entries should be set to NaN.
     */
    float c[RBRINSTRUMENTGEN3_CALIBRATION_C_COEFFICIENT_MAX];
    /**
     *\brief Calibration X coefficients.
     *
     * Unused entries should be set to NaN.
     */
    float x[RBRINSTRUMENTGEN3_CALIBRATION_X_COEFFICIENT_MAX];
    /**
     *\brief Input channel indices.
     *
     * Unused entries should be set to 0. Entries corresponding to the special
     * “value” value are set to #RBRINSTRUMENTGEN3_VALUE_COEFFICIENT.
     */
    RBRInstrumentGen3ChannelIndex n[RBRINSTRUMENTGEN3_CALIBRATION_N_COEFFICIENT_MAX];
} RBRInstrumentGen3Calibration;

/**
 * \brief Possible channel gain ranging modes.
 *
 * \see RBRInstrumentGen3Channel
 * \see https://docs.rbr-global.com/L3commandreference/format-of-stored-data/standard-rawbin00-format/deployment-header/version-2-001
 */
typedef enum RBRInstrumentGen3ChannelRangingMode
{
    /** No gain ranging is available. */
    RBRINSTRUMENTGEN3_RANGING_NONE,
    /** A fixed gain is used. */
    RBRINSTRUMENTGEN3_RANGING_MANUAL,
    /** The channel auto-ranges over the available gain settings. */
    RBRINSTRUMENTGEN3_RANGING_AUTO,
    /** The number of specific gain ranging modes. */
    RBRINSTRUMENTGEN3_RANGING_COUNT,
    /** An unknown or unrecognized gain ranging mode. */
    RBRINSTRUMENTGEN3_UNKNOWN_RANGING
} RBRInstrumentGen3ChannelRangingMode;

/**
 * \brief Get a human-readable string name for a channel gain ranging mode.
 *
 * \param [in] mode the ranging mode
 * \return a string name for the ranging mode
 * \see RBRInstrumentGen3Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen3ChannelRangingMode_name(
    RBRInstrumentGen3ChannelRangingMode mode);

/**
 * Gain parameters for a channel.
 *
 * \see RBRInstrumentGen3Channel
 * \see RBRInstrumentGen3_setChannelGain()
 */
typedef struct RBRInstrumentGen3ChannelGain
{
    /** \brief The gain selection mode employed by the sensor. */
    RBRInstrumentGen3ChannelRangingMode rangingMode;

    /**
     * \brief The gain value in use by the sensor.
     *
     * Only applies when RBRInstrumentGen3ChannelGain.rangingMode is
     * #RBRINSTRUMENTGEN3_RANGING_MANUAL. Otherwise set to NaN.
     */
    float currentGain;

    /**
     * \brief The gain settings supported by the sensor.
     *
     * Only applies where RBRInstrumentGen3ChannelGain.rangingMode is
     * #RBRINSTRUMENTGEN3_RANGING_MANUAL or #RBRINSTRUMENTGEN3_RANGING_AUTO. Otherwise
     * all values are set to NaN.
     *
     * Unused entries are set to NaN.
     */
    float availableGains[RBRINSTRUMENTGEN3_CHANNEL_GAINS_MAX];
} RBRInstrumentGen3ChannelGain;

/**
 * \brief Details reported by the instrument `channel` command.
 *
 * \see RBRInstrumentGen3Channels
 */
typedef struct RBRInstrumentGen3Channel
{
    /**
     * \brief A short, pre-defined “generic” name for the installed channel as
     * a null-terminated C string.
     *
     * E.g., “temp09”, “pres19”, “cond05”.
     *
     * \see https://docs.rbr-global.com/L3commandreference/supported-channel-types
     */
    char type[RBRINSTRUMENTGEN3_CHANNEL_TYPE_MAX + 1];

    /** \brief The internal address to which the channel responds. */
    RBRInstrumentGen3ModuleAddress module;

    /**
     * \brief Whether the channel is activated for sampling.
     *
     * \see RBRInstrumentGen3_setChannelStatus()
     */
    bool status;

    /**
     * \brief The minimum power-on settling time required by this channel.
     *
     * Specified in milliseconds.
     */
    RBRInstrumentGen3Period settlingTime;

    /**
     * \brief The typical data acquisition time required by this channel.
     *
     * Specified in milliseconds.
     */
    RBRInstrumentGen3Period readTime;

    /**
     * \brief The type of formula used to convert raw readings to physical
     * measurement units as a null-terminated C string.
     *
     * \see https://docs.rbr-global.com/L3commandreference/calibration-equations-and-cross-channel-dependencies
     */
    char equation[RBRINSTRUMENTGEN3_CALIBRATION_EQUATION_MAX + 1];

    /**
     * \brief The unit in which processed data is normally reported from the
     * logger as a null-terminated C string.
     *
     * E.g., “C” for Celsius, “V” for Volts, “dbar” for decibars.
     */
    char userUnits[RBRINSTRUMENTGEN3_CHANNEL_UNIT_MAX + 1];

    /** \brief Gain parameters for the channel. */
    RBRInstrumentGen3ChannelGain gain;

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
    char label[RBRINSTRUMENTGEN3_CHANNEL_LABEL_MAX + 1];

    /** \brief The calibration for the channel. */
    RBRInstrumentGen3Calibration calibration;
} RBRInstrumentGen3Channel;

/**
 * \brief Details reported by a combination of the instrument `channels`,
 * `channel`, and `calibration` commands.
 *
 * \see RBRInstrumentGen3_getChannels()
 * \see RBRInstrumentGen3_getChannelsWithoutCalibrations()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/channels
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/channel
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/calibration
 */
typedef struct RBRInstrumentGen3Channels
{
    /** \brief The number of installed and configured instrument channels. */
    int32_t count;
    /**
     * \brief The number of active channels, which excludes any turned off by
     * the user.
     *
     * \see RBRInstrumentGen3Channel.status
     */
    int32_t on;
    /**
     * \brief The maximum power-on settling settling time across all enabled
     * channels.
     *
     * Specified in milliseconds.
     */
    RBRInstrumentGen3Period settlingTime;
    /**
     * \brief The maximum overall reading time across all enabled channels.
     *
     * Specified in milliseconds.
     */
    RBRInstrumentGen3Period readTime;
    /**
     * \brief The minimum sampling period with the currently-active channels.
     *
     * Specified in milliseconds.
     */
    RBRInstrumentGen3Period minimumPeriod;
    /**
     * \brief Specific channel details.
     *
     * The first RBRInstrumentGen3Channel.count entries will be populated.
     */
    RBRInstrumentGen3Channel channels[RBRINSTRUMENTGEN3_CHANNEL_MAX];
} RBRInstrumentGen3Channels;

/**
 * \brief Get channel information for the instrument.
 *
 * Channel information is composed from a combination of the `channels`,
 * `channel`, and `calibration` commands. The information returned by this
 * function should comprise a complete model of an instrument's channels.
 *
 * \param [in] instrument the instrument connection
 * \param [out] channels the channel information
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN3_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen3_getChannelsWithoutCalibrations()
 * \see RBRInstrumentGen3_getSensorParameters()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/channels
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/channel
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/calibration
 */
RBRInstrumentGen3Error RBRInstrumentGen3_getChannels(RBRInstrumentGen3 *instrument,
                                             RBRInstrumentGen3Channels *channels);

/**
 * \brief Get channel information for the instrument without calibration data.
 *
 * Channel information is composed from combining the `channels` and `channel`
 * commands. Unlike RBRInstrumentGen3_getChannels(), calibration information is
 * _not_ populated. This saves bandwidth and time communicating with the
 * instrument when calibration information is unnecessary.
 *
 * \param [in] instrument the instrument connection
 * \param [out] channels the channel information
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN3_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen3_getChannels()
 * \see RBRInstrumentGen3_getSensorParameters()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/channels
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/channel
 */
RBRInstrumentGen3Error RBRInstrumentGen3_getChannelsWithoutCalibrations(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3Channels *channels);

/**
 * \brief Set the status of a channel.
 *
 * \param [in] instrument the instrument connection
 * \param [in] channel the index of the channel to update
 * \param [in] status whether the channel is activated for sampling
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN3_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN3_HARDWARE_ERROR if the instrument is logging
 * \see RBRInstrumentGen3_getChannels()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/channel
 */
RBRInstrumentGen3Error RBRInstrumentGen3_setChannelStatus(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3ChannelIndex channel,
    bool status);

/**
 * \brief Set the gain parameters of a channel.
 *
 * RBRInstrumentGen3ChannelGain.rangingMode must be either
 * #RBRINSTRUMENTGEN3_RANGING_MANUAL or #RBRINSTRUMENTGEN3_RANGING_AUTO. Otherwise,
 * #RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE is returned.
 *
 * For manual gain selection, the gain given by
 * RBRInstrumentGen3ChannelGain.currentGain must be one of the available gains
 * reported by the instrument. Otherwise, the instrument will produce a
 * hardware error. If RBRInstrumentGen3ChannelGain.availableGains is populated
 * (contains at least one leading non-NaN entry), this function will verify the
 * presence of the chosen gain. If it is not found,
 * #RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE is returned.
 *
 * RBRInstrumentGen3ChannelGain.availableGains is only used for parameter
 * verification. It is not sent to the instrument.
 *
 * \param [in] instrument the instrument connection
 * \param [in] channel the index of the channel to update
 * \param [in] gain the gain parameters for the channel
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENTGEN3_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN3_HARDWARE_ERROR if the instrument is logging, or an
 *                                       invalid gain value is given
 * \return #RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE if the ranging mode is
 *                                                invalid, or if the gain value
 *                                                can be conclusively
 *                                                determined to be invalid
 * \see RBRInstrumentGen3_getChannels()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/channel
 */
RBRInstrumentGen3Error RBRInstrumentGen3_setChannelGain(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3ChannelIndex channel,
    RBRInstrumentGen3ChannelGain *gain);

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
 * RBRInstrumentGen3_getChannels() after updating coefficients to confirm the
 * values written.
 *
 * Values of in the _n_ coefficient group (RBRInstrumentGen3Calibration.n) are
 * ignored.
 *
 * \param [in] instrument the instrument connection
 * \param [in] channel the index of the channel to update
 * \param [in] calibration the new calibration coefficients for the channel
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN3_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN3_HARDWARE_ERROR when the calibration cannot be changed
 * \return #RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE when the date/time of the
 *                                                calibration is out of range,
 *                                                or when no coefficients are
 *                                                populated
 * \see RBRInstrumentGen3_getChannels()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/calibration
 */
RBRInstrumentGen3Error RBRInstrumentGen3_setCalibration(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3ChannelIndex channel,
    const RBRInstrumentGen3Calibration *calibration);

/**
 * \brief Get the fetch power-off delay.
 *
 * The fetch power-off delay delay in milliseconds between the successful
 * completion of a fetch command and power to the front end sensors being
 * removed by the instrument.
 *
 * \param [in] instrument the instrument connection
 * \param [out] fetchPowerOffDelay the fetch power-off delay
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENTGEN3_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen3_setFetchPowerOffDelay()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
RBRInstrumentGen3Error RBRInstrumentGen3_getFetchPowerOffDelay(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3Period *fetchPowerOffDelay);

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
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN3_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN3_HARDWARE_ERROR when the settings cannot be changed
 * \see RBRInstrumentGen3_getFetchPowerOffDelay()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
RBRInstrumentGen3Error RBRInstrumentGen3_setFetchPowerOffDelay(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3Period fetchPowerOffDelay);

/**
 * \brief Get whether sensor power is always on.
 *
 * The instrument does not have to power down front end sensors between
 * samples. This can be useful for sensors with very long power-on
 * stabilization times.
 *
 * \param [in] instrument the instrument connection
 * \param [out] sensorPowerAlwaysOn whether sensor power is always on
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENTGEN3_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen3_setSensorPowerAlwaysOn()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
RBRInstrumentGen3Error RBRInstrumentGen3_isSensorPowerAlwaysOn(
    RBRInstrumentGen3 *instrument,
    bool *sensorPowerAlwaysOn);

/**
 * \brief Set whether sensor power is always on.
 *
 * A hardware error will occur if the instrument is logging.
 *
 * \param [in] instrument the instrument connection
 * \param [in] sensorPowerAlwaysOn whether sensor power is always on
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN3_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN3_HARDWARE_ERROR if the instrument is logging
 * \see RBRInstrumentGen3_isSensorPowerAlwaysOn()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
RBRInstrumentGen3Error RBRInstrumentGen3_setSensorPowerAlwaysOn(
    RBRInstrumentGen3 *instrument,
    bool sensorPowerAlwaysOn);

/**
 * \brief Get whether cast detection is enabled.
 *
 * The instrument can automatically detect upcasts and downcasts and generate
 * cast detection events in the datastream.
 *
 * \param [in] instrument the instrument connection
 * \param [out] castDetection whether cast detection is enabled
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENTGEN3_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen3_setCastDetection()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
RBRInstrumentGen3Error RBRInstrumentGen3_getCastDetection(RBRInstrumentGen3 *instrument,
                                                  bool *castDetection);

/**
 * \brief Set whether cast detection is enabled.
 *
 * A hardware error will occur if the instrument is logging.
 *
 * \param [in] instrument the instrument connection
 * \param [in] castDetection whether cast detection is enabled
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN3_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN3_HARDWARE_ERROR if the instrument is logging
 * \see RBRInstrumentGen3_getCastDetection()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
RBRInstrumentGen3Error RBRInstrumentGen3_setCastDetection(RBRInstrumentGen3 *instrument,
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
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENTGEN3_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen3_setInputTimeout()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
RBRInstrumentGen3Error RBRInstrumentGen3_getInputTimeout(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3Period *inputTimeout);

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
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN3_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN3_HARDWARE_ERROR if the instrument is logging
 * \see RBRInstrumentGen3_getInputTimeout()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
RBRInstrumentGen3Error RBRInstrumentGen3_setInputTimeout(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3Period inputTimeout);

/**
 * \brief Value settings the instrument uses for calculation of derived
 * channels, or as defaults for physical parameters the instrument does not
 * measure.
 *
 * See the command reference documentation for details on the use and default
 * values for each of these parameters.
 *
 * \see RBRInstrumentGen3_getValueSetting()
 * \see RBRInstrumentGen3_setValueSetting()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
typedef enum RBRInstrumentGen3ValueSetting
{
    /**
     * The temperature coefficient used to correct the derived channel for
     * specific conductivity to 25°C.
     *
     * Specified in degrees Celsius.
     */
    RBRINSTRUMENTGEN3_SETTING_SPECCONDTEMPCO,
    /**
     * The height above the seabed at which the logger is deployed.
     *
     * Specified in metres.
     */
    RBRINSTRUMENTGEN3_SETTING_ALTITUDE,
    /**
     * The default temperature.
     *
     * Specified in degrees Celsius.
     */
    RBRINSTRUMENTGEN3_SETTING_TEMPERATURE,
    /**
     * The default absolute pressure.
     *
     * Specified in dbar.
     */
    RBRINSTRUMENTGEN3_SETTING_PRESSURE,
    /**
     * The default conductivity.
     *
     * Specified in ms/cm².
     *
     * \nol3 It is only available on early Logger2 instruments.
     */
    RBRINSTRUMENTGEN3_SETTING_CONDUCTIVITY,
    /**
     * The default atmospheric pressure.
     *
     * Specified in dbar.
     */
    RBRINSTRUMENTGEN3_SETTING_ATMOSPHERE,
    /**
     * The default water density.
     *
     * Specified in g/cm³.
     */
    RBRINSTRUMENTGEN3_SETTING_DENSITY,
    /**
     * The default salinity.
     *
     * Specified in PSU.
     */
    RBRINSTRUMENTGEN3_SETTING_SALINITY,
    /**
     * The default average speed of sound.
     *
     * Specified in m/s.
     */
    RBRINSTRUMENTGEN3_SETTING_AVGSOUNDSPEED,
    /** The number of specific value settings. */
    RBRINSTRUMENTGEN3_SETTING_COUNT,
    /** An unknown or unrecognized value setting. */
    RBRINSTRUMENTGEN3_UNKNOWN_SETTING
} RBRInstrumentGen3ValueSetting;

/**
 * \brief Get a human-readable string name for an instrument value setting.
 *
 * \param [in] setting the value setting
 * \return a string name for the value setting
 * \see RBRInstrumentGen3Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen3ValueSetting_name(RBRInstrumentGen3ValueSetting setting);

/**
 * \brief Read a value setting from the instrument.
 *
 * \param [in] instrument the instrument connection
 * \param [in] setting the setting to retrieve
 * \param [out] value the value of the setting
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENTGEN3_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE when an unrecognized setting
 *                                                is requested
 * \see RBRInstrumentGen3_setValueSetting()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
RBRInstrumentGen3Error RBRInstrumentGen3_getValueSetting(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3ValueSetting setting,
    float *value);

/**
 * \brief Write the a value setting to the instrument.
 *
 * A hardware error will occur if the instrument is logging.
 *
 * \param [in] instrument the instrument connection
 * \param [in] setting the setting to retrieve
 * \param [in] value the value of the setting
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN3_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN3_HARDWARE_ERROR if the instrument is logging
 * \return #RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE when an unrecognized setting
 *                                                is requested or when the
 *                                                value is NaN
 * \see RBRInstrumentGen3_getValueSetting()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
RBRInstrumentGen3Error RBRInstrumentGen3_setValueSetting(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3ValueSetting setting,
    float value);

/**
 * \brief A sensor parameter.
 *
 * \see RBRInstrumentGen3_getSensorParameter()
 * \see RBRInstrumentGen3_getSensorParameters()
 * \see RBRInstrumentGen3_setSensorParameter()
 */
typedef struct RBRInstrumentGen3SensorParameter
{
    /** \brief The name of the parameter as a null-terminated C string. */
    char key[RBRINSTRUMENTGEN3_SENSOR_PARAMETER_KEY_MAX + 1];
    /** \brief The parameter value as a null-terminated C string. */
    char value[RBRINSTRUMENTGEN3_SENSOR_PARAMETER_VALUE_MAX + 1];
} RBRInstrumentGen3SensorParameter;

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
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN3_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen3_getChannels()
 * \see RBRInstrumentGen3_getSensorParameters()
 * \see RBRInstrumentGen3_setSensorParameter()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/sensor
 */
RBRInstrumentGen3Error RBRInstrumentGen3_getSensorParameter(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3ChannelIndex channel,
    RBRInstrumentGen3SensorParameter *parameter);

/**
 * \brief Retrieve the sensor parameters for a channel.
 *
 * To ease memory requirements, sensor parameters are not included with other
 * channel information retrieved by RBRInstrumentGen3_getChannels().
 *
 * \param instrument the instrument connection
 * \param [in] channel the index of the channel for which sensor parameters are
 *                     to be retrieved
 * \param [out] parameters the sensor parameters for the channel
 * \param [in,out] size initially, the maximum number of elements which can be
 *                      written to \a parameters; after return, the number of
 *                      parameters actually written
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN3_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen3_getChannels()
 * \see RBRInstrumentGen3_getSensorParameter()
 * \see RBRInstrumentGen3_setSensorParameter()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/sensor
 */
RBRInstrumentGen3Error RBRInstrumentGen3_getSensorParameters(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3ChannelIndex channel,
    RBRInstrumentGen3SensorParameter *parameters,
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
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN3_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN3_HARDWARE_ERROR when the parameter cannot be changed
 * \see RBRInstrumentGen3_getChannels()
 * \see RBRInstrumentGen3_getSensorParameter()
 * \see RBRInstrumentGen3_getSensorParameters()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/sensor
 */
RBRInstrumentGen3Error RBRInstrumentGen3_setSensorParameter(
    RBRInstrumentGen3 *instrument,
    RBRInstrumentGen3ChannelIndex channel,
    RBRInstrumentGen3SensorParameter *parameter);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTGEN3CONFIGURATION_H */
