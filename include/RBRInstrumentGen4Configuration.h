/**
 * \file RBRInstrumentGen4Configuration.h
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

#ifndef LIBRBR_RBRINSTRUMENTGEN4CONFIGURATION_H
#define LIBRBR_RBRINSTRUMENTGEN4CONFIGURATION_H

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The maximum number of schedules count. */
#define RBRINSTRUMENTGEN4_SCHEDULE_COUNT_MAX 16

/** \brief The maximum number of permissionlist count. */
#define RBRINSTRUMENTGEN4_PERMISSION_COUNT_MAX 14

/** \brief The maximum schedule period in milliseconds. */
#define RBRINSTRUMENTGEN4_SAMPLING_PERIOD_MAX 86400000

/** \brief The maximum regime boundary in dbar. */
#define RBRINSTRUMENTGEN4_REGIME_BOUNDARY_MAX 65535

/** \brief The maximum regime bin size in dbar. */
#define RBRINSTRUMENTGEN4_REGIME_BINSIZE_MAX 6553.5

/** \brief The maximum sampling period within a regime. */
#define RBRINSTRUMENTGEN4_REGIME_SAMPLING_PERIOD_MAX 65000

/**
 * \brief The maximum number of C calibration coefficients.
 *
 * \see RBRInstrumentGen4Calibration.c
 */
#define RBRINSTRUMENTGEN4_CALIBRATION_C_COEFFICIENT_MAX 24

/**
 * \brief The maximum number of X calibration coefficients.
 *
 * \see RBRInstrumentGen4Calibration.x
 */
#define RBRINSTRUMENTGEN4_CALIBRATION_X_COEFFICIENT_MAX 8

/**
 * \brief The maximum number of input channel indices.
 *
 * \see RBRInstrumentGen4Calibration.n
 */
#define RBRINSTRUMENTGEN4_CALIBRATION_N_COEFFICIENT_MAX 8

/**
 * \brief An in-band representation of the special “value” calibration
 * correction value.
 *
 * \see RBRInstrumentGen4Calibration.n
 */
#define RBRINSTRUMENTGEN4_VALUE_COEFFICIENT 0xFF

/**
 * \brief The maximum number of characters in a calibration equation name.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENTGEN4_CALIBRATION_EQUATION_MAX 32

/**
 * \brief The maximum number of gain settings for a channel.
 */
#define RBRINSTRUMENTGEN4_CHANNEL_GAINS_MAX 8

/** \brief The minimum input timeout. */
#define RBRINSTRUMENTGEN4_INPUT_TIMEOUT_MIN 10000

/** \brief The maximum input timeout. */
#define RBRINSTRUMENTGEN4_INPUT_TIMEOUT_MAX 240000

/**
 * \brief The maximum number of characters in a sensor parameter key.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENTGEN4_SENSOR_PARAMETER_KEY_MAX 63

/**
 * \brief The maximum number of characters in a sensor parameter value.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENTGEN4_SENSOR_PARAMETER_VALUE_MAX 63

/** \brief The maximum number of configs count. */
#define RBRINSTRUMENTGEN4_CONFIG_COUNT_MAX 16

/** \brief The maximum number of groups count. */
#define RBRINSTRUMENTGEN4_GROUP_COUNT_MAX 16

/** \brief The maximum number of characters in a bus address.
 * The bus address is from 0 to 255, with some reserved addresses.
*/
#define RBRINSTRUMENTGEN4_BUS_ADDRESS_MAX 3


/**
 * \brief A channel identifier.
 *
 * Channel indices are always 1-based. A value of 0 means the index is unset
 * or empty.
 */
typedef uint8_t RBRInstrumentGen4ChannelIndex;

/**
 * \brief A channel calibration.
 *
 * \see RBRInstrumentGen4Channel
 * \see RBRInstrumentGen4_setCalibration()
 */
typedef struct RBRInstrumentGen4Calibration
{
    /**
     * \brief The date/time of the calibration.
     *
     * Unused entries should be set to 0.
     */
    RBRInstrumentGen4DateTime dateTime;

    /** \brief useroffset is provided to permit users to apply a simple linear adjustment to 
     * the final value. It is to give a rough correction in the field when a proper
     * re-calbiration is not possible.
    */
    float useroffset;
    /** \brief userslope is provided to permit users to apply a simple linear adjustment to 
     * the final value. It is to give a rough correction in the field when a proper
     * re-calbiration is not possible.
    */
    float userslope;
    /**
     *\brief Calibration C coefficients.
     *
     * Unused entries should be set to NaN.
     */
    float c[RBRINSTRUMENTGEN4_CALIBRATION_C_COEFFICIENT_MAX];
    /**
     *\brief Calibration X coefficients.
     *
     * Unused entries should be set to NaN.
     */
    float x[RBRINSTRUMENTGEN4_CALIBRATION_X_COEFFICIENT_MAX];
    /**
     *\brief Input channel indices.
     *
     * Unused entries should be set to 0. Entries corresponding to the special
     * “value” value are set to #RBRINSTRUMENTGEN4_VALUE_COEFFICIENT.
     */
    const RBRInstrumentGen4ChannelIndex n[RBRINSTRUMENTGEN4_CALIBRATION_N_COEFFICIENT_MAX];
} RBRInstrumentGen4Calibration;

/**
 * \brief Reports a channel's calibration coefficients.
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
 * RBRInstrumentGen4_getChannels() after updating coefficients to confirm the
 * values written.
 *
 * Values of in the _n_ coefficient group (RBRInstrumentGen4Calibration.n) are
 * ignored.
 *
 * \param [in] instrument the instrument connection
 * \param [in] channellabel the channel label of the specified channel
 * \param [out] calibration the calibration coefficients for the channel
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the calibration cannot be changed
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when no coefficients are
 *                                                populated
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/calibration
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getCalibration(
    RBRInstrumentGen4 *instrument,
    const char *channellabel,
    RBRInstrumentGen4Calibration *calibration);

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
 * RBRInstrumentGen4_getChannels() after updating coefficients to confirm the
 * values written.
 *
 * Values of in the _n_ coefficient group (RBRInstrumentGen4Calibration.n) are
 * ignored.
 *
 * \param [in] instrument the instrument connection
 * \param [in] channellabel the channel label of the channel to update
 * \param [in] calibration the new calibration coefficients for the channel
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the calibration cannot be changed
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when no coefficients are
 *                                                populated
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/calibration
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setCalibration(
    RBRInstrumentGen4 *instrument,
    const char *channellabel,
    const RBRInstrumentGen4Calibration *calibration);

/** \brief An internal module identifier. */
typedef uint8_t RBRInstrumentGen4ModuleAddress;

/**
 * \brief Possible channel gain ranging modes.
 *
 * \see RBRInstrumentGen4Channel
 * \see https://docs.rbr-global.com/L3commandreference/format-of-stored-data/standard-rawbin00-format/deployment-header/version-2-001
 */
typedef enum RBRInstrumentGen4ChannelGainMode
{
    /** No gain ranging is available. */
    RBRINSTRUMENTGEN4_GAIN_NONE,
    /** A fixed gain is used. */
    RBRINSTRUMENTGEN4_GAIN_FIXED,
    /** The channel auto-ranges over the available gain settings. */
    RBRINSTRUMENTGEN4_GAIN_AUTO,
    /** The number of specific gain modes. */
    RBRINSTRUMENTGEN4_GAIN_COUNT,
    /** An unknown or unrecognized gain mode. */
    RBRINSTRUMENTGEN4_UNKNOWN_GAIN
} RBRInstrumentGen4ChannelGainMode;

/**
 * \brief Get a human-readable string name for a channel gain ranging mode.
 *
 * \param [in] mode the gain mode or current value
 * \return a string name for the gain mode
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4ChannelGainMode_name(
    RBRInstrumentGen4ChannelGainMode mode);

/**
 * \brief Gain parameters for a channel.
 *
 * \see RBRInstrumentGen4Channel
 */
typedef struct RBRInstrumentGen4ChannelGain
{
    /** \brief The gain selection mode employed by the sensor. */
    RBRInstrumentGen4ChannelGainMode mode;
    /**
     * \brief The gain value in use by the sensor.
     *
     * Only applies when RBRInstrumentGen4ChannelGain.rangingMode is
     * RBRINSTRUMENTGEN4_RANGING_MANUAL. Otherwise set to NaN.
     */
    float currentGain;
        /**
     * \brief The gain settings supported by the sensor.
     *
     * Only applies where RBRInstrumentGen4ChannelGain.rangingMode is
     * RBRINSTRUMENTGEN4_RANGING_MANUAL or RBRINSTRUMENTGEN4_RANGING_AUTO. Otherwise
     * all values are set to NaN.
     *
     * Unused entries are set to NaN.
     */
    const float availableGains[RBRINSTRUMENTGEN4_CHANNEL_GAINS_MAX];

} RBRInstrumentGen4ChannelGain;

/**
 * \brief Details reported by the instrument `channel` command. 
 *
 * \see RBRInstrumentGen4Channel
 */
typedef struct RBRInstrumentGen4Channel
{
    /**
     * \brief A short, pre-defined “generic” name for the installed channel as
     * a null-terminated C string.
     *
     * E.g., “temp09”, “pres19”, “cond05”.
     *
     * \see https://docs.rbr-global.com/L3commandreference/supported-channel-types
     */
    char type[RBRINSTRUMENTGEN4_CHANNEL_TYPE_MAX+1];

    /** \brief The internal address to which the channel responds. */
    RBRInstrumentGen4ModuleAddress module;

    /**
     * \brief The minimum power-on settling time required by this channel.
     *
     * Specified in milliseconds.
     */
    RBRInstrumentGen4Period settlingTime;

    /**
     * \brief The typical data acquisition time required by this channel.
     *
     * Specified in milliseconds.
     */
    RBRInstrumentGen4Period readTime;

    /**
     * \brief The minimum time in miliseconds for which the power must remain 
     * off once the channel has been powered down. The logger will not turn the
     * channels back on again until this delay has expired.
    */
   int32_t guardtime;
    /**
     * \brief The type of formula used to convert raw readings to physical
     * measurement units as a null-terminated C string.
     *
     * \see https://docs.rbr-global.com/L3commandreference/calibration-equations-and-cross-channel-dependencies
     */
    char equation[RBRINSTRUMENTGEN4_CALIBRATION_EQUATION_MAX + 1];

    /**
     * \brief The unit in which processed data is normally reported from the
     * logger as a null-terminated C string.
     *
     * E.g., “C” for Celsius, “V” for Volts, “dbar” for decibars.
     */
    char userUnits[RBRINSTRUMENTGEN4_CHANNEL_UNIT_MAX+1];

    /** \brief Whether the channel is a derived channel. */
    const bool derived;

    /** \brief Gain parameters for the channel. */
    const RBRInstrumentGen4ChannelGain gain;

    /**
     * \brief An alphanumeric description of the physical parameter measured as
     * a null-terminated C string.
     *
     * Set upon request for OEM customers. If not set, reported as “none”.
     */
    char label[RBRINSTRUMENTGEN4_CHANNEL_LABEL_MAX+1];

    /** \brief The calibration for the channel. */
    RBRInstrumentGen4Calibration calibration;
} RBRInstrumentGen4Channel;

/**
 * \brief Get all information about the channel with the specified channel label.
 *
 * \param [in] instrument the instrument connection
 * \param [in] channellabel the channel label
 * \param [inout] channel information about the channel with specified channel label
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the instrument is logging
 * \see RBRInstrumentGen4_getChannel()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/channel
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getChannel(RBRInstrumentGen4 *instrument,
                                                const char *channellabel,
                                                RBRInstrumentGen4Channel *channel);

/** \brief Response to the `channels channellist` command.
 * \see RBRInstrument_getChannels()
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/channels
 */
typedef struct RBRInstrumentGen4Channellist
{
    /** \brief The number of active channels. */
    int32_t count;
    /**
     * \brief The pointer array for active channels.
     */
    RBRInstrumentGen4Channel *channels[RBRINSTRUMENTGEN4_CHANNEL_MAX];
} RBRInstrumentGen4Channellist;

/**
 * \brief Details reported by a combination of the instrument `channels`,
 * `channel`, and `calibration` commands.
 *
 * \see RBRInstrumentGen4_getChannels()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/channels
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/channel
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/calibration
 */
typedef struct RBRInstrumentGen4Channels
{
    /** \brief The number of installed and configured instrument channels. */
    int32_t count;
    /** \brief Channellist pointing to the channels. */
    RBRInstrumentGen4Channel channels[RBRINSTRUMENTGEN4_CHANNEL_MAX];
} RBRInstrumentGen4Channels;

/**
 * \brief Get channels information for the instrument.
 * \param [in] instrument the instrument connection
 * \param [out] channels the channels information
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/channels
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getChannels(RBRInstrumentGen4 *instrument,
                                             RBRInstrumentGen4Channels *channels);

/** \brief miscellaneous settings in the logger as described below. */
typedef struct RBRInstrumentGen4Settings{
    /** \brief the delay in milliseconds between successful completion of a `poll` commmand,
     * and power to the front end sensors being removed by the logger.
     * Power is left on for a short time to avoid excessive power cycling when sending
     * repeated `poll` commands;
     * The default value is 8000.
    */
    RBRInstrumentGen4Period pollpoweroffdelay;
    /** \brief a flag which is either ON or OFF. When ON, the logger doesn't
     * remove power from the front end sensors between samples. This can be
     * useful for sensors very long power-on stabilization times, but note 
     * that currently all sensors are kept powered up between readings. The 
     * default setting is off.
    */
    bool sensorpoweralwayson;
    /** \brief specifies the value of a timeout used by the logger when 
     * receiving command input; it is used to temporarily blank other 
     * output such as streamed data, and to assist in power saving by turning 
     * off the serial communication interface if it is not needed. 
     * The value is specified in milliseconds; 
     * the default value for all instruments is 10000 (10 seconds).*/
    RBRInstrumentGen4Period inputtimeout;
    /** \brief the temperature coefficient used to correct the derived channel 
     * for specific conductivity to 25°C. Its value depends on the ionic 
     * composition of the water being monitored, and should be set to an 
     * appropriate value for best results. 
     * A typical range of values is 0.0191 to 0.0214, with the lower end 
     * suitable for KCl solutions and the upper end for NaCl solutions.*/
    float speccondtempco;
    /** \brief the height above the seabed in metres at which the logger 
     * is deployed. This is a user-entered parameter which is required by 
     * host software to calculate statistics and parameters for 
     * wave analysis. Can be ignored if not used.*/
    float altitude;
    /** \brief below are default parameter values, to be used when the logger does 
     * not have a channel which measures the named parameter, but one or more 
     * cross-channel calibration equations requires it as an input.
     * temperature in °C, default value 15.0*/
    float temperature;
    /** \brief absolute pressure in dbar, default value 10.132501 (1 standard atmosphere)*/
    float pressure;
    /** \brief atmospheric pressure in dbar, default value 10.132501*/
    float atmosphere;
    /** \brief water density in g/cm3, default value 1.026021*/
    float density;
    /** \brief salinity in PSU, default value 35*/
    float salinity;
    /** \brief avgsoundspeed in m/s, default value 1506.8*/
    float avgsoundspeed;
}RBRInstrumentGen4Settings;

/**
 * \brief Get the values of miscellaneous settings in the logger.
 *
 * \param [in] instrument the instrument connection
 * \param [out] settings the settings in the logger.
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
RBRInstrumentGen4Error RBRInstrumentGen4Settings_getSettings(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Settings *settings);

/**
 * \brief Set the values of miscellaneous settings in the logger.
 *
 * Hardware errors may occur if:
 * - the instrument is logging
 * - you set an out-of-bounds time the library fails to detect
 *
 * \param [in] instrument the instrument connection
 * \param [in] settings the values for the settings in the logger
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the settings cannot be changed
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/settings
 */
RBRInstrumentGen4Error RBRInstrumentGen4Settings_setSettings(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Settings *settings);

/**
 * \brief A sensor parameter.
 *
 * \see RBRInstrumentGen4_getSensorParameter()
 * \see RBRInstrumentGen4_getSensorParameters()
 * \see RBRInstrumentGen4_setSensorParameter()
 */
typedef struct RBRInstrumentGen4SensorParameter
{
    /** \brief The name of the parameter as a null-terminated C string. */
    char key[RBRINSTRUMENTGEN4_SENSOR_PARAMETER_KEY_MAX + 1];
    /** \brief The parameter value as a null-terminated C string. */
    char value[RBRINSTRUMENTGEN4_SENSOR_PARAMETER_VALUE_MAX + 1];
} RBRInstrumentGen4SensorParameter;

/**
 * \brief Retrieve a single sensor parameter for a channel.
 *
 * If the parameter is not configured, its value is set to “n/a”. 
 * A special value of "alllabels" may be given for channel_label causing the
 * requested paramters to be reported for all channels.
 * 
 * All parameters might be obtained using the all keyword in lieu of parameter 
 * names but this one is optional.
 *
 * \param [in] instrument the instrument connection
 * \param [in] channellabel the channel label of the channel from which the parameter is to
 *                     be retrieved
 * \param [inout] parameter initially, the sensor parameter to be retrieved;
 *                           after return, the instrument response
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_getChannels()
 * \see RBRInstrumentGen4_getSensorParameters()
 * \see RBRInstrumentGen4_setSensorParameter()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/sensor
 */
RBRInstrumentGen4Error RBRInstrumentGen4Sensor_getSensorParameter(
    RBRInstrumentGen4 *instrument,
    const char *channellabel,
    RBRInstrumentGen4SensorParameter *parameter);

/**
 * \brief Retrieve the sensor parameters for a channel.
 *
 * To ease memory requirements, sensor parameters are not included with other
 * channel information retrieved by RBRInstrumentGen4_getChannels().
 *
 * \param [in] instrument the instrument connection
 * \param [in] channellabel the channel label for which sensor parameters are
 *                     to be retrieved
 * \param [inout] size initially, the maximum number of elements which can be
 *                      written to \a parameters; after return, the number of
 *                      parameters actually written
* \param [out] parameters the sensor parameters for the channel
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_getChannels()
 * \see RBRInstrumentGen4_getSensorParameter()
 * \see RBRInstrumentGen4_setSensorParameter()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/sensor
 */
RBRInstrumentGen4Error RBRInstrumentGen4Sensor_getSensorParameters(
    RBRInstrumentGen4 *instrument,
    const char *channellabel,
    int32_t *size,
    const RBRInstrumentGen4SensorParameter *parameters);

/**
 * \brief Set a single sensor parameter for a channel.
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - the parameter name has not been defined at the RBR factory
 *
 * \param [in] instrument the instrument connection
 * \param [in] channellabel the label of the channel the sensor parameter of which
 *                     is to be updated
 * \param [in] parameter the sensor parameter for the channel
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the parameter cannot be changed
 * \see RBRInstrumentGen4_getChannels()
 * \see RBRInstrumentGen4_getSensorParameter()
 * \see RBRInstrumentGen4_getSensorParameters()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/sensor
 */
RBRInstrumentGen4Error RBRInstrumentGen4Sensor_setSensorParameter(
    RBRInstrumentGen4 *instrument,
    const char *channellabel,
    const RBRInstrumentGen4SensorParameter *parameter);

/** \brief Option of particular commands that can be sent to the UV-LED device. */
typedef enum RBRInstrumentGen4UvledCommand{
    /** turns the UV-LEDs ON. */
    RBRINSTRUMENTGEN4_UVLED_ACTIVATE,
    /** turns the UV-LEDs OFF. */
    RBRINSTRUMENTGEN4_UVLED_DEACTIVATE,
    /** reports the current ON/OFF state of the UV-LEDs. */
    RBRINSTRUMENTGEN4_UVLED_STATUS,
    /** The number of specific uvled commands. */
    RBRINSTRUMENTGEN4_UVLED_COUNT,
    /** An unknown or unrecognized uvled command.*/
    RBRINSTRUMENTGEN4_UNKNOWN_UVLED,
}RBRInstrumentGen4UvledCommand;

/**
 * \brief Get a human-readable string name for uvled commands.
 *
 * \param [in] uvledCommand the uvled command.
 * \return a string name for the command.
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4UvledCommand_name(RBRInstrumentGen4UvledCommand uvledCommand);

/**
 * \brief This is a specialized command available in firmware versions 1.130 
 * or later.  It is used to return information about, and to allow some control 
 * over, the UV-LED antifouling device used in the RBRconcerto3 CTD|UV data 
 * logger, and other instruments equipped with the UV antifouling feature.
 */
typedef struct RBRInstrumentGen4Uvled{
    /** \brief reports (read-only) the type of UV-device configured to be used with the instrument. */
    const char id[RBRINSTRUMENTGEN4_LABEL_NAME_MAX+1];
    /** \brief reports or sets whether or not the UV-LED schedule is enabled for use. */
    bool scheduled;
    /** reports (read-only) the startup time for the UV-LEDs in milliseconds */
    const RBRInstrumentGen4Period powerondelay;
    /** reports (read-only) the shut down time for the UV-LEDs in milliseconds. */
    const RBRInstrumentGen4Period poweroffdelay;
    /** reports or sets the interval between UV-LED episodes in milliseconds. */
    RBRInstrumentGen4Period interval;
    /** reports or sets the duration of an UV-LED episode in milliseconds. */
    RBRInstrumentGen4Period duration;
    /**
     * reports (read-only) the command applied at the beginning of a UV-LED episode.
    */
    const char *startaction;
    /**
     * reports (read-only) the command applied at the end of a UV-LED episode.
    */
    const char *endaction;
    /** sends a particular command to the UV-LED device;  */
    RBRInstrumentGen4UvledCommand command;
    /** reports (read-only) the time remaining until the start of the next scheduled UV-LED episode. */
    const RBRInstrumentGen4Period timetoepisode;
    /** reports or sets the point in the schedule at which the first UV-LED episode will occur. */
    bool startimmediate;
    /** reports (read only) the module address of the front end card controlling 
     * the UV-LED.*/
    const RBRInstrumentGen4ModuleAddress module;
    /** reports (read-only) in milliseconds the cumulative time for which the UV-LEDs have been energized since configuration at the Factory. */
    const RBRInstrumentGen4Period operatingtime;
    /** can be used to enable, disable, or report whether UV-LED events will be recorded in the instrument's memory when data logging is enabled. */
    bool episodelog;
}RBRInstrumentGen4Uvled;

/** \brief This is a specialized command available in firmware versions 1.130 
 * or later.  It is used to return information about, and to allow some control 
 * over, the UV-LED antifouling device used in the RBRconcerto3 CTD|UV data 
 * logger, and other instruments equipped with the UV antifouling feature. 
 * 
 * \param [in] instrument the instrument connection
 * \param [out] uvled reports all the the UV-LED information about the UV-device.
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/uvled
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getUvled(RBRInstrumentGen4 *instrument, RBRInstrumentGen4Uvled *uvled);

/** \brief This is a specialized command available in firmware versions 1.130 
 * or later.  It is used to return information about, and to allow some control 
 * over, the UV-LED antifouling device used in the RBRconcerto3 CTD|UV data 
 * logger, and other instruments equipped with the UV antifouling feature. 
 * 
 * \param [in] instrument the instrument connection.
 * \param [in] uvled all the the UV-LED information about the UV-device.
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/uvled
 */
RBRInstrumentGen4Error RBRInstrumentGen4Uvled_setUvled(RBRInstrumentGen4 *instrument, const RBRInstrumentGen4Uvled *uvled);

/**
 * \brief Instrument `group` command parameters.
 *
 * \see RBRInstrumentGen4_getSGroup()
 * \see RBRInstrumentGen4_setGroup()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/group
 */
typedef struct RBRInstrumentGen4Group{
    /** \brief The group's label */
    char label[RBRINSTRUMENTGEN4_LABEL_NAME_MAX+1];
    /** \brief a list of logger channels that are included in this group. Labels in 
     * the list must be separated by a pipe charactor with no spaces. */
    RBRInstrumentGen4Channellist channellist;
}RBRInstrumentGen4Group;

/** \brief grouplist.
*/
typedef struct RBRInstrumentGen4Grouplist{
    /** \brief The number of active groups. */
    int32_t count;
    /**
     * \brief The array of pointers for the label of each active group as 
     * null-terminated C strings.
     */
    RBRInstrumentGen4Group *groups[RBRINSTRUMENTGEN4_GROUP_COUNT_MAX];
} RBRInstrumentGen4Grouplist;

/**
 * \brief Instrument `groups` command parameters.
 *
 * \see RBRInstrumentGen4_getSGroups()
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/groups
 */
typedef struct RBRInstrumentGen4Groups{
    /** \brief the maximum number of groups that the logger can hold in its pool at any given time. */
    const int32_t maxcount;
    /** \brief lists by label all defined groups; labels in the list are separated by 
     * a pipe character with no spaces.  Labels are reported in the order that 
     * the groups were created, earliest first. */
    const RBRInstrumentGen4Grouplist grouplist;
}RBRInstrumentGen4Groups;

/**
 * \brief Reports the properties of the specified group.
 *
 * \param [in] instrument the instrument connection
 * \param [in] grouplabel the group label
 * \param [out] group the information of specified group
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/group
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getGroup(RBRInstrumentGen4 *instrument,
                                                const char *grouplabel,
                                                RBRInstrumentGen4Group *group);
/**
 * \brief Modifies the properties of the specified group.
 * modifications are not permitted while logging is enabled.
 *
 * \param [in] instrument the instrument connection
 * \param [in] grouplabel the group label
 * \param [in] group the information of specified group
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/group
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setGroup(RBRInstrumentGen4 *instrument, 
                                                const char *grouplabel,
                                                RBRInstrumentGen4Group *group);

/**
 * \brief Creates a grouping of logger channels with optional user defined parameters.
 * This command is not available while the instrument is enabled for logging.
 *
 * \param [in] instrument the instrument connection.
 * \param [in] group user-specified group.
 * \param [inout] groups the pool of user defined groups.
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/group
 */
RBRInstrumentGen4Error RBRInstrumentGen4_createGroup(RBRInstrumentGen4 *instrument,  
                                                    const RBRInstrumentGen4Group *group, 
                                                    RBRInstrumentGen4Groups *groups);

/**
 * \brief Deletes a specific group.
 *
 * \param [in] instrument the instrument connection
 * \param [in] grouplabel specifis by label a single group to delete from the pool. 
 * \param [inout] groups the pool of user defined groups.
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/group
 */
RBRInstrumentGen4Error RBRInstrumentGen4_deleteGroup(RBRInstrumentGen4 *instrument, 
                                                    const char *grouplabel, 
                                                    RBRInstrumentGen4Groups *groups);

/**
 * \brief Deletes multiple groups with optional user defined parameters.
 * 
 * Configurations to 
 * delete are specified by their labels; if a list is given, the 
 * configuration_labels in the list must be separated by a pipe character with 
 * no spaces.  At least one existing configuration must be specified.
 * 
 * A deleted configuration can no longer be used for a future deployment.  
 * Any historical deployments in the logger memory that used the deleted
 *  configuration are not affected; all datasets include as part of their metadata
 *  a snapshot of the configuration when the logger was enabled.
 *
 * \param [in] instrument the instrument connection
 * \param [in] grouplabellist the configuration label list of configurations to be deleted
 * \param [inout] groups the pool of logger configurations
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/deleteconfig
 */
RBRInstrumentGen4Error RBRInstrumentGen4_deleteGroupMultiple(RBRInstrumentGen4 *instrument, 
                                                    const char *grouplabellist,
                                                    RBRInstrumentGen4Groups *groups);
/**
 * \brief Deletes all groups.
 *
 * \param [in] instrument the instrument connection 
 * \param [inout] groups the pool of user defined groups.
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/group
 */
RBRInstrumentGen4Error RBRInstrumentGen4_deleteGroupAll(RBRInstrumentGen4 *instrument,
                                                    RBRInstrumentGen4Groups *groups);                                                                                                


/**
 * \brief Reports read-only information about the logger's pool of channel groupings.
 *
 * \param [in] instrument the instrument connection
 * \param [out] groups the information about the logger's pool of channel groupings.
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/groups
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getGroups(RBRInstrumentGen4 *instrument, RBRInstrumentGen4Groups *groups);

/************************************************************************************/
/**
 * \brief Possible instrument sampling modes by a schedule.
 *
 * \see RBRInstrumentGen4Schedule
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/schedule
 */
typedef enum RBRInstrumentGen4SamplingMode
{
    /** Continuous schedule mode. */
    RBRINSTRUMENTGEN4_SAMPLING_CONTINUOUS,
    /**
    * Direction-dependent schedule mode.
    *
    */
    RBRINSTRUMENTGEN4_SAMPLING_DDSAMPLING,
    /** Regime schedule mode. */
    RBRINSTRUMENTGEN4_SAMPLING_REGIMES,
    /** Average schedule mode. */
    RBRINSTRUMENTGEN4_SAMPLING_AVERAGE,
    /** Tide schedule mode. */
    RBRINSTRUMENTGEN4_SAMPLING_TIDE,
    /** Burst schedule mode. */
    RBRINSTRUMENTGEN4_SAMPLING_BURST,
    /** Wave schedule mode. */
    RBRINSTRUMENTGEN4_SAMPLING_WAVE,
    /** The number of specific schedule modes. */
    RBRINSTRUMENTGEN4_SAMPLING_COUNT,
    /** An unknown or unrecognized schedule mode. */
    RBRINSTRUMENTGEN4_UNKNOWN_SAMPLING
} RBRInstrumentGen4SamplingMode;

/** \brief Defines schedule mode-dependent-parameters: continuous mode.
* \see https://docs.rbr-global.com/L3commandreference/commands
*/
typedef struct RBRInstrumentGen4Continuous
{
    /** \brief Time between measurements.
     *
     * Specified in milliseconds. Must be in the range
     * RBRInstrumentGen4Sampling.userPeriodLimit—86,400,000.
     *
     * - When < 1,000, must be in RBRInstrumentGen4Sampling.availableFastPeriods.
     * - When ≥ 1,000, must be an integer multiple of 1,000.
     */
    RBRInstrumentGen4Period period;

    /** \brief Get/set automatic upcast and downcast detection in profiling deployment.
     * Options: on|off. Defaults to off.
    */
    bool castdetection;
} RBRInstrumentGen4Continuous;

/** \brief Defines schedule mode-dependent-parameters: average mode.
* \see https://docs.rbr-global.com/L3commandreference/commands
*/
typedef struct RBRInstrumentGen4Average
{
    /**
     * \brief Time between measurements.
     *
     * Specified in milliseconds. Must be in the range
     * RBRInstrumentGen4Sampling.userPeriodLimit—86,400,000.
     *
     * - When < 1,000, must be in RBRInstrumentGen4Sampling.availableFastPeriods.
     * - When ≥ 1,000, must be an integer multiple of 1,000.
     */
    RBRInstrumentGen4Period period;
    /**
     * \brief The time between the first measurement of two consecutive bursts.
     *
     * Specified in milliseconds. Must be in the range 1,000—86,400,000 and
     * must be an integer multiple of 1,000. The burst interval is additionally
     * constrained by the sampling period (RBRInstrumentGen4Schedule.period) and
     * burst count (RBRInstrumentGen4Schedule.burstcount):
     *
     *     burst interval > (burst count × sampling period)
     */
    RBRInstrumentGen4Period burstinterval;

    /**
     * \brief The number of measurements taken in each burst.
     *
     * Specified in numbers of samples. Must be in the range 2—65,535.
     */
    int32_t burstcount;
} RBRInstrumentGen4Average;

/** \brief Defines schedule mode-dependent-parameters: tide mode.
* \see https://docs.rbr-global.com/L3commandreference/commands
*/
typedef struct RBRInstrumentGen4Tide
{
    /**
     * \brief Time between measurements.
     *
     * Specified in milliseconds. Must be in the range
     * RBRInstrumentGen4Sampling.userPeriodLimit—86,400,000.
     *
     * - When < 1,000, must be in RBRInstrumentGen4Sampling.availableFastPeriods.
     * - When ≥ 1,000, must be an integer multiple of 1,000.
     */
    RBRInstrumentGen4Period period;
    /**
     * \brief The time between the first measurement of two consecutive bursts.
     *
     * Specified in milliseconds. Must be in the range 1,000—86,400,000 and
     * must be an integer multiple of 1,000. The burst interval is additionally
     * constrained by the sampling period (RBRInstrumentGen4Schedule.period) and
     * burst count (RBRInstrumentGen4Schedule.burstcount):
     *
     *     burst interval > (burst count × sampling period)
     */
    RBRInstrumentGen4Period burstinterval;

    /**
     * \brief The number of measurements taken in each burst.
     *
     * Specified in numbers of samples. Must be in the range 2—65,535.
     */
    int32_t burstcount;
} RBRInstrumentGen4Tide;

/** \brief Defines schedule mode-dependent-parameters: burst modes.
* \see https://docs.rbr-global.com/L3commandreference/commands
*/
typedef struct RBRInstrumentGen4Burst
{
    /**
     * \brief Time between measurements.
     *
     * Specified in milliseconds. Must be in the range
     * RBRInstrumentGen4Sampling.userPeriodLimit—86,400,000.
     *
     * - When < 1,000, must be in RBRInstrumentGen4Sampling.availableFastPeriods.
     * - When ≥ 1,000, must be an integer multiple of 1,000.
     */
    RBRInstrumentGen4Period period;
    /**
     * \brief The time between the first measurement of two consecutive bursts.
     *
     * Specified in milliseconds. Must be in the range 1,000—86,400,000 and
     * must be an integer multiple of 1,000. The burst interval is additionally
     * constrained by the sampling period (RBRInstrumentGen4Schedule.period) and
     * burst count (RBRInstrumentGen4Schedule.burstcount):
     *
     *     burst interval > (burst count × sampling period)
     */
    RBRInstrumentGen4Period burstinterval;

    /**
     * \brief The number of measurements taken in each burst.
     *
     * Specified in numbers of samples. Must be in the range 2—65,535.
     */
    int32_t burstcount;
} RBRInstrumentGen4Burst;

/** \brief Defines schedule mode-dependent-parameters: wave modes.
* \see https://docs.rbr-global.com/L3commandreference/commands
*/
typedef struct RBRInstrumentGen4Wave
{
    /**
     * \brief Time between measurements.
     *
     * Specified in milliseconds. Must be in the range
     * RBRInstrumentGen4Sampling.userPeriodLimit—86,400,000.
     *
     * - When < 1,000, must be in RBRInstrumentGen4Sampling.availableFastPeriods.
     * - When ≥ 1,000, must be an integer multiple of 1,000.
     */
    RBRInstrumentGen4Period period;
    /**
     * \brief The time between the first measurement of two consecutive bursts.
     *
     * Specified in milliseconds. Must be in the range 1,000—86,400,000 and
     * must be an integer multiple of 1,000. The burst interval is additionally
     * constrained by the sampling period (RBRInstrumentGen4Schedule.period) and
     * burst count (RBRInstrumentGen4Schedule.burstcount):
     *
     *     burst interval > (burst count × sampling period)
     */
    RBRInstrumentGen4Period burstinterval;

    /**
     * \brief The number of measurements taken in each burst.
     *
     * Specified in numbers of samples. Must be in the range 2—65,535.
     */
    int32_t burstcount;
} RBRInstrumentGen4Wave;
/**
 * \brief Whether settings apply to ascent or descent.
 *
 * \see RBRInstrumentGen4Regimes
 * \see RBRInstrumentGen4DirectionDependentSampling
 * \see https://docs.rbr-global.com/L3commandreference/commands
 * \see https://docs.rbr-global.com/L3commandreference/commands
 */
typedef enum RBRInstrumentGen4Direction
{
    /** The settings apply while ascending. */
    RBRINSTRUMENTGEN4_DIRECTION_ASCENDING,
    /** The settings apply while descending. */
    RBRINSTRUMENTGEN4_DIRECTION_DESCENDING,
    /** The number of specific directions. */
    RBRINSTRUMENTGEN4_DIRECTION_COUNT,
    /** An unknown or unrecognized direction. */
    RBRINSTRUMENTGEN4_UNKNOWN_DIRECTION
} RBRInstrumentGen4Direction;

/** \brief Defines schedule mode-dependent-parameters: ddsampling mode.
* \see https://docs.rbr-global.com/L3commandreference/commands
*/
typedef struct RBRInstrumentGen4DirectionDependentSampling
{
    /** \brief In which direction the instrument samples at the fast rate. */
    RBRInstrumentGen4Direction direction;

    /** \brief Get/set the auto upcasts and downcasts detection in proviling deployment.
     * and will generate cast detection events in the stored data.
     * This option must be set to ON for the ddsampling mode. 
     * Options: on|off. Defaults to on.
    */
   bool castdetection;

    /**
     * \brief The same meaning as RBRInstrumentGen4Sampling.period, but applies
     * only when the instrument is moving in the preferred direction.
     *
     * Must be shorter than RBRInstrumentGen4DirectionDependentSampling.slowPeriod.
     */
    RBRInstrumentGen4Period fastPeriod;
    /**
     * \brief The same meaning as RBRInstrumentGen4Sampling.period, but applies
     * only when the instrument is not moving in the preferred direction.
     *
     * Must be longer than RBRInstrumentGen4DirectionDependentSampling.fastPeriod.
     */
    RBRInstrumentGen4Period slowPeriod;
    /**
     * \brief Sets the boundary, based on the previous profile, where the
     * instrument should switch to the fast period sampling.
     *
     * Specified in dbar. The minimum precision is 0.1 and the value should be
     * greater than 0.
     */
    float fastThreshold;
    /**
     * \brief Sets the boundary, based on the previous profile, where the
     * instrument should switch to the slow period sampling.
     *
     * Specified in dbar. The minimum precision is 0.1 and the value should be
     * greater than 0.
     */
    float slowThreshold;
} RBRInstrumentGen4DirectionDependentSampling;

/**
 * \brief The types of pressure available for use a reference for the
 * determination of the current regime and bin.
 *
 * \see RBRInstrumentGen4Regimes
 * \see https://docs.rbr-global.com/L3commandreference/commands
 */
typedef enum RBRInstrumentGen4RegimesReference
{
    /** Absolute pressure is used as the reference. */
    RBRINSTRUMENTGEN4_REFERENCE_ABSOLUTE,
    /** Sea pressure is used as the reference. */
    RBRINSTRUMENTGEN4_REFERENCE_SEAPRESSURE,
    /** The number of specific regime reference types. */
    RBRINSTRUMENTGEN4_REFERENCE_COUNT,
    /** An unknown or unrecognized regime reference type. */
    RBRINSTRUMENTGEN4_UNKNOWN_REFERENCE
} RBRInstrumentGen4RegimesReference;

/** \brief Defines schedule mode-dependent-parameters: Regimes mode.
* \see https://docs.rbr-global.com/L3commandreference/commands
*/
typedef struct RBRInstrumentGen4Regimes
{
    /** \brief The regimes-relevant direction through the water column. */
    RBRInstrumentGen4Direction direction;

    /**
     * \brief The number of regimes that are set.
     * Options: 1|2|3.
     */
    int32_t count;
    /** \brief The pressure type used for regime and bin determination. */
    RBRInstrumentGen4RegimesReference reference;

    /** \brief The first boundary in a region. Specified in dbar.*/
    float boundary1;

    /** \brief The size used for each averaged bin.Specified in dbar.*/
    float binSize1;
    /**
     * \brief The same meaning as RBRInstrumentGen4Sampling.period, but applies
     * only to this particular regime.
     *
     * May not be greater than 65,000.
     */
    RBRInstrumentGen4Period period1;

    /** \brief The second boundary in a region. Specified in dbar.*/
    float boundary2;

    /** \brief The size used for each averaged bin.Specified in dbar.*/
    float binSize2;
    /**
     * \brief The same meaning as RBRInstrumentGen4Sampling.period, but applies
     * only to this particular regime.
     *
     * May not be greater than 65,000.
     */
    RBRInstrumentGen4Period period2;

    /** \brief The first boundary in a region. Specified in dbar.*/
    float boundary3;

    /** \brief The size used for each averaged bin.Specified in dbar.*/
    float binSize3;
    /**
     * \brief The same meaning as RBRInstrumentGen4Sampling.period, but applies
     * only to this particular regime.
     *
     * May not be greater than 65,000.
     */
    RBRInstrumentGen4Period period3;

} RBRInstrumentGen4Regimes;

/**
 * \brief Instrument `schedule` command parameters.
 *
 * See the command reference for details on valid parameter values.
 *
 * \see RBRInstrumentGen4_getSchedule()
 * \see RBRInstrumentGen4_setSchedule()
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/schedule
 */
typedef struct RBRInstrumentGen4Schedule
{
    /** \brief Label is redundant for reporting, but allows the schedule's label
     * to be modified. It is subject to the naming constraints.
     * see Gen4 command reference-> "Parameter naming constraints" for details.
    */
    char label[RBRINSTRUMENTGEN4_LABEL_NAME_MAX+1];

    /** \brief Get/set a list of groups defining the channels that will be sampled according to this schedule.
     * Labels must be separated by a pipe character, no spaces.
     * The list must specify at least one group for the schedule to be valid.
    */
    RBRInstrumentGen4Grouplist *grouplist;

    /** \brief Get/set communication link over which data for this schedule will be streamed in real time
     * during the deployment. At most one link may be active for each schedule. 
     * Options: serial|usb|off. Defaults to off.
    */
    RBRInstrumentGen4Link stream;

    /** \brief Get/set. Determines whether data for this schedule will be sored in memory
     * during the deployment. 
     * Options: on|off. Defaults to off.
     */
    bool store;

    /** \brief Get/set. The instrument schedule mode. */
    RBRInstrumentGen4SamplingMode mode;

    /** \brief Defines schedule Mode-dependent-parameters. 
     */
    union {
        /** \brief continuous mode struct*/
        RBRInstrumentGen4Continuous continuous;
        /** \brief average mode struct*/
        RBRInstrumentGen4Average average;
        /** \brief tide mode struct*/
        RBRInstrumentGen4Tide tide;
        /** \brief burst mode struct*/
        RBRInstrumentGen4Burst burst;
        /** \brief wave mode struct*/
        RBRInstrumentGen4Wave wave;
        /** \brief ddsampling mode struct*/
        RBRInstrumentGen4DirectionDependentSampling ddsampling;
        /** \brief regimes mode struct*/
        RBRInstrumentGen4Regimes regimes;
    } RBRInstrumentGen4ModeDependentParameters;
} RBRInstrumentGen4Schedule;

/** \brief schedulelist.
*/
typedef struct RBRInstrumentGen4Schedulelist{
    /** \brief The number of active schedules. */
    int32_t count;
    /**
     * \brief The array of pointers for the label of each active schedule as null-terminated C strings.
     */
    RBRInstrumentGen4Schedule *schedules[RBRINSTRUMENTGEN4_SCHEDULE_COUNT_MAX];
} RBRInstrumentGen4Schedulelist;

/** \brief lists all the sampling modes currently available in the instruments.
 * It's setup in Factory Mode, with `permissions` command.
 */
typedef enum RBRInstrumentGen4Availablemodes{
    /** \brief Prompt*/
    RBRINSTRUMENTGEN4_PROMPT= 1 << 0,
    /** \brief confirmation*/
    RBRINSTRUMENTGEN4_CONFIRMATION= 1 << 1,
    /** \brief streamusb*/
    RBRINSTRUMENTGEN4_STREAMUSB= 1 << 2,
    /** \brief streamserial*/
    RBRINSTRUMENTGEN4_STREAMSERIAL= 1 << 3,
    /** \brief average mode*/
    RBRINSTRUMENTGEN4_AVERAGE= 1 << 4,
    /** \brief burst mode*/
    RBRINSTRUMENTGEN4_BURST= 1 << 5,
    /** \brief tide mode*/
    RBRINSTRUMENTGEN4_TIDE= 1 << 6,
    /** \brief wave mode*/
    RBRINSTRUMENTGEN4_WAVE= 1 << 7,
    /** \brief thresholding*/
    RBRINSTRUMENTGEN4_THRESHOLDING= 1 << 8,
    /** \brief twistactivation*/
    RBRINSTRUMENTGEN4_TWISTACTIVATION= 1 << 9,
    /** \brief regimes mode*/
    RBRINSTRUMENTGEN4_REGIMES= 1 << 10,
    /** \brief ddsampling mode*/
    RBRINSTRUMENTGEN4_DDSAMPLING= 1 << 11,
    /** \brief wifi*/
    RBRINSTRUMENTGEN4_WIFI= 1 << 12,
    /** \brief pauseresume*/
    RBRINSTRUMENTGEN4_PAUSERESUME= 1 << 13
}RBRInstrumentGen4Availablemodes;

/** \brief Instrument `schedules` command parameters.
 *
 * See the command reference for details on valid parameter values.
 *
 * \see RBRInstrumentGen4_getSchedules()
 * \see https://docs.rbr-global.com/L3commandreference/commands/time-and-schedule/schedules
 */
typedef struct RBRInstrumentGen4Schedules{
    /** \brief gives the maximum number of schedules that the logger can hold at any given time.
     */
    const int32_t maxcount;
    /** \brief lists all defined schedules by label; labels are separated by a pipe charactor without spaces.
     * Labels are repoted in the order that the schedules were created, earlist first.
     */
    const RBRInstrumentGen4Schedulelist schedulelist;
    /** \brief lists all the sampling modes configured to be available in the instruments.
     * Items are separeted by a pipe charater without space.
     */
    const RBRInstrumentGen4Availablemodes availablemodes;
}RBRInstrumentGen4Schedules;
/************************************************************************************/

/**
 * \brief Instrument `config` command parameters.
 *
 * \see RBRInstrumentGen4_getConfig()
 * \see RBRInstrumentGen4_setConfig()
 * \see https://docs.rbr-global.com/L3commandreference/commands/Configuration+Information+and+Calibration/config
 */
typedef struct RBRInstrumentGen4Config {
    /** \brief the configuration's label. */
    char label[RBRINSTRUMENTGEN4_LABEL_NAME_MAX+1];
    /** \brief a list of schedules to be executed when this configuration is used to 
     * enable a deployment. Labels in the list must be separated by a pipe 
     * character ('|'), with no spaces.  The list must specify at least one 
     * valid schedule before the configuration can be used. 
     */
    RBRInstrumentGen4Schedulelist *schedulelist;
}RBRInstrumentGen4Config;

/** \brief configlist.
*/
typedef struct RBRInstrumentGen4Configlist{
    /** \brief The number of all defined configurations. */
    int32_t count;
    /**
     * \brief The array of pointers for the label of each defined configuration as null-terminated C strings.
     */
    RBRInstrumentGen4Config *configs[RBRINSTRUMENTGEN4_CONFIG_COUNT_MAX];
} RBRInstrumentGen4Configlist;

/**
 * \brief readonly information about the pool of logger configurations. 
 * \see RBRInstrumentGen4_getConfigs()
 * \see RBRInstrumentGen4_setConfigs()
 * \see https://docs.rbr-global.com/L3commandreference/commands/Configuration+Information+and+Calibration/configs
*/
typedef struct RBRInstrumentGen4Configs {
    /** \brief maximum number of configurations that the logger can hold in its pool at any given time. */
    const int32_t maxcount;
    /** \brief all defined configurations. 
     * Reported in the order that the configurations were created, earliest first.
     * */
    const RBRInstrumentGen4Configlist *configlist;
}RBRInstrumentGen4Configs;

/**
 * \brief Reports the properties of the specified configuration. 
 * \param [in] instrument the instrument connection
 * \param [in] configlabel the config label
 * \param [inout] config the configuration reported
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/config
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getConfig(RBRInstrumentGen4 *instrument, 
                                                    const char *configlabel,
                                                    RBRInstrumentGen4Config *config);

/**
 * \brief Modifies the properties of the specified configuration. 
 * modifications are not permitted while logging is enabled.
 *
 * \param [in] instrument the instrument connection
 * \param [in] configlabel the config label
 * \param [in] config the configuration to be modified by label
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/config
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setConfig(RBRInstrumentGen4 *instrument, 
                                                    const char *configlabel,
                                                    const RBRInstrumentGen4Config *config);

/**
 * \brief Reports read-only information about the pool of logger configurations.
 *
 * \param [in] instrument the instrument connection
 * \param [out] configs the poll of logger configurations
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/configs
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getConfigs(RBRInstrumentGen4 *instrument, RBRInstrumentGen4Configs *configs);

/**
 * \brief Creates a configuration with optional user defined parameters.
 * 
 * The user-specified label for the configuration is required. 
 * schedule list is optional, which specifies the schedules to execute when 
 * this configuration is used to enable a deployment. If this option is 
 * supplied, the schedules in the schedulelist must already exist. The schedule 
 * labels in the list must be separated by a pipe charactor without spaces.
 * 
 * A configuration can be created with an unspecified(i.e. empty) schedule 
 * list, but there must be at least one valid schedule in the list before using 
 * the configuration to enable the logger.The maximum number of schedules that 
 * can be executed by a single configuration is eight.
 *
 * \param [in] instrument the instrument connection
 * \param [in] config a configuration with a user-specified label 
 * and a list of valid schedule labels specifying the schedules to execute 
 * when this configuration is used to enable a deployment.
 * \param [out] configs the pool of logger configurations.
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/createconfig
 */
RBRInstrumentGen4Error RBRInstrumentGen4_createConfig(RBRInstrumentGen4 *instrument, 
                                                    const RBRInstrumentGen4Config *config,
                                                    RBRInstrumentGen4Configs *configs);

/**
 * \brief Deletes a configuration with optional user defined parameters.
 * 
 * The Configurations to delete are specified by its label; 
 * 
 * A deleted configuration can no longer be used for a future deployment.  
 * Any historical deployments in the logger memory that used the deleted
 *  configuration are not affected; all datasets include as part of their metadata
 *  a snapshot of the configuration when the logger was enabled.
 *
 * \param [in] instrument the instrument connection
 * \param [in] configlabel the label of configuration to be deleted
 * \param [inout] configs the pool of logger configurations
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/deleteconfig
 */
RBRInstrumentGen4Error RBRInstrumentGen4_deleteConfig(RBRInstrumentGen4 *instrument, 
                                                    const char *configlabel,
                                                    RBRInstrumentGen4Configs *configs);

/**
 * \brief Deletes multiple configurations with optional user defined parameters.
 * 
 * Configurations to 
 * delete are specified by their labels; if a list is given, the 
 * configuration_labels in the list must be separated by a pipe character with 
 * no spaces.  At least one existing configuration must be specified.
 * 
 * A deleted configuration can no longer be used for a future deployment.  
 * Any historical deployments in the logger memory that used the deleted
 *  configuration are not affected; all datasets include as part of their metadata
 *  a snapshot of the configuration when the logger was enabled.
 *
 * \param [in] instrument the instrument connection
 * \param [in] configlabellist the configuration label list of configurations to be deleted
 * \param [inout] configs the pool of logger configurations
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/deleteconfig
 */
RBRInstrumentGen4Error RBRInstrumentGen4_deleteConfigMultiple(RBRInstrumentGen4 *instrument, 
                                                    const char *configlabellist,
                                                    RBRInstrumentGen4Configs *configs);
/**
 * \brief Deletes all configurations.
 *
 * A deleted configuration can no longer be used for a future deployment.  
 * Any historical deployments in the logger memory that used the deleted
 *  configuration are not affected; all datasets include as part of their metadata
 *  a snapshot of the configuration when the logger was enabled.
 *
 * \param [in] instrument the instrument connection
 * \param [inout] configs the pool of logger configurations
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/deleteconfig
 */
RBRInstrumentGen4Error RBRInstrumentGen4_deleteConfigAll(RBRInstrumentGen4 *instrument,
                                                    RBRInstrumentGen4Configs *configs);                                                                                                
/**
 * \brief Returns the logger's sampling configuration to a factory-set state.
 * Unsafe and protected - not allowed while logging is enabled.
 * 
 * ALL user-defined data relating to configurations, schedules and groups will 
 * be lost, although historical datasets stored in the logger's memory are not 
 * affected.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/factory
 */
RBRInstrumentGen4Error RBRInstrumentGen4_factoryReset(RBRInstrumentGen4 *instrument);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTCONFIGURATION_H */
