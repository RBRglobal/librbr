/**
 * \file RBRInstrumentGen4Configuration.h
 *
 * \brief Instrument commands and structures pertaining to instrument
 * configuration information and calibration.
 *
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
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

#include "RBRInstrumentGen4.h"

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
 * \brief The maximum number of characters in a calibration equation name.
 *
 * Does not include any null terminator.
 * \see RBRInstrumentGen4Channel.equation
 */
#define RBRINSTRUMENTGEN4_CALIBRATION_EQUATION_MAX 32

/**
 * \brief The maximum number of gain settings for a channel.
 * \see RBRInstrumentGen4ChannelGain.availableGains
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
 * \brief The maximum number of configs count.
 * \see RBRInstrumentGen4ConfigPool.pool
 */
#define RBRINSTRUMENTGEN4_CONFIG_COUNT_MAX 16

/**
 * \brief The maximum number of groups count.
 * \see RBRInstrumentGen4GroupPool.pool
 */
#define RBRINSTRUMENTGEN4_GROUP_COUNT_MAX 16

/** \brief The maximum number of characters in a bus address.
 * The bus address is from 0 to 255, with some reserved addresses.
 */
#define RBRINSTRUMENTGEN4_BUS_ADDRESS_MAX 3

/** \brief The maximum number of fast periods. */
#define RBRINSTRUMENTGEN4_AVAILABLE_FAST_PERIODS_MAX 4

/**
 * \brief `calibration` command parameters.
 *
 * \see RBRInstrumentGen4Channel
 * \see RBRInstrumentGen4_getCalibration()
 * \see RBRInstrumentGen4_setCalibration()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828510/calibration
 */
typedef struct RBRInstrumentGen4Calibration
{
    /**
     * \brief The date/time of the calibration.
     * Unused entries should be set to 0.
     */
    RBRInstrumentGen4DateTime dateTime;

    /**
     * \brief userOffset is provided to permit users to apply a simple linear adjustment to 
     * the final value. It is to give a rough correction in the field when a proper
     * re-calbiration is not possible.
    */
    float userOffset;

    /** 
     * \brief userSlope is provided to permit users to apply a simple linear adjustment to 
     * the final value. It is to give a rough correction in the field when a proper
     * re-calbiration is not possible.
    */
    float userSlope;

    /**
     * \brief Calibration C coefficients.
     * Unused entries should be set to NaN.
     */
    float c[RBRINSTRUMENTGEN4_CALIBRATION_C_COEFFICIENT_MAX];

    /**
     * \brief Calibration X coefficients.
     * Unused entries should be set to NaN.
     */
    float x[RBRINSTRUMENTGEN4_CALIBRATION_X_COEFFICIENT_MAX];

    /**
     * \brief Pointers to input channels.
     * \readonly
     */
    void *n[RBRINSTRUMENTGEN4_CHANNEL_MAX];

    /**
     * \brief Pointers to parent channel.
     * \readonly
     */
    void *parent;
} RBRInstrumentGen4Calibration;

/** \brief An internal module identifier. */
typedef uint8_t RBRInstrumentGen4ModuleAddress;

/**
 * \brief Possible channel gain ranging modes.
 *
 * \see RBRInstrumentGen4Channel
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/47153202/channel
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
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/47153202/channel
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
 * \brief Instrument `channel <channel_label>` command parameters. 
 * \see RBRInstrumentGen4Group
 * \see RBRInstrumentGen4ChannelPool
 * \see RBRInstrumentGen4_getChannel()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/47153202/channel
 */
typedef struct RBRInstrumentGen4Channel
{
    /**
     * \brief An alphanumeric description of the physical parameter measured.
     * Stored as a null-terminated C string.
     * \readonly
     */
    char label[RBRINSTRUMENTGEN4_CHANNEL_LABEL_MAX + 1];

    /**
     * \brief A short, pre-defined “generic” name for the installed channel.
     * Stored as a null-terminated C string.
     * E.g., “temp09”, “pres19”, “cond05”.
     * \readonly
     */
    char type[RBRINSTRUMENTGEN4_CHANNEL_TYPE_MAX + 1];

    /**
     * \brief The internal address to which the channel responds.
     * \readonly
     */
    RBRInstrumentGen4ModuleAddress address;

    /**
     * \brief The minimum power-on settling delay required by this channel.
     * Specified in milliseconds.
     * \readonly
     */
    RBRInstrumentGen4Period settlingTime;

    /**
     * \brief The typical data acquisition time required by this channel.
     * Specified in milliseconds.
     * \readonly
     */
    RBRInstrumentGen4Period readTime;

    /**
     * \brief The minimum time for which the power must remain off once the
     * channel has been powered down. The logger will not turn the channel
     * back on again until this delay has expired.
     * Specified in milliseconds.
     * \readonly
    */
   int32_t guardTime;

    /**
     * \brief The type of formula used to convert raw readings to physical
     * measurement units.
     * Stored as a null-terminated C string.
     * \readonly
     * \see RBRInstrumentGen4Calibration
     */
    char equation[RBRINSTRUMENTGEN4_CALIBRATION_EQUATION_MAX + 1];

    /**
     * \brief The unit in which processed data is normally reported from the
     * logger.
     * E.g., “C” for Celsius, “V” for Volts, “dbar” for decibars.
     * Stored as a null-terminated C string.
     * \readonly
     */
    char userUnits[RBRINSTRUMENTGEN4_CHANNEL_UNIT_MAX + 1];

    /**
     * \brief Whether the channel is a derived channel.
     * \readonly
     */
    const bool derived;

    /** 
     * \brief The gain setting currently in use by the channel.
     * \see RBRInstrumentGen4ChannelGain
     */
    const RBRInstrumentGen4ChannelGain gain;

    /** \brief The calibration for the channel. */
    RBRInstrumentGen4Calibration calibration;

    /**
     * \brief Pointer to the parent channel pool.
     * \readonly
     */
    void *parent;
} RBRInstrumentGen4Channel;

/**
 * \brief Populate a channel with channel parameters from the logger.
 * \note Issues the `channel <channel_label>` command.
 *
 * \param [in] instrument the instrument connection
 * \param [inout] channel a pointer to the specified channel
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the channel cannot be read
 * \see RBRInstrumentGen4_getChannel()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/47153202/channel
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getChannel(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Channel *channel);

/**
 * \brief Instrument `channel` command parameters.
 * Serves as a persistent cache of channels configured on the logger.
 *
 * \see RBRInstrumentGen4_getChannelPool()
 * \see RBRInstrumentGen4_getGroup()
 */
typedef struct RBRInstrumentGen4ChannelPool
{
    /** \brief The number of channels configured on an instrument. */
    int32_t count;

    /** \brief List of channel objects. */
    RBRInstrumentGen4Channel pool[RBRINSTRUMENTGEN4_CHANNEL_MAX];
} RBRInstrumentGen4ChannelPool;

/**
 * \brief Populate the pool of channels with the labels of the schedules defined on the logger.
 * \note Issues the `channel` command.
 *
 * \param [in] instrument the instrument connection.
 * \param [out] channelPool the populated pool of supported channels.
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the channel pool cannot be read
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/47153202/channel
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getChannelPool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4ChannelPool *channelPool);

/**
 * \brief Reports a channel's calibration coefficients.
 * \note Issues the `calibration <channel_label>` instrument command.
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
 * \param [in] channelPool the pool of channels supported by the logger to search for dependent channels
 * \param [out] calibration the calibration coefficients for the channel
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the calibration cannot be read
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when no coefficients are
 *                                                populated
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828510/calibration
 * \see RBRInstrumentGen4_setCalibration()
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getCalibration(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Calibration *calibration);

/**
 * \brief Update a channel's calibration coefficients.
 * \note Issues the `calibration <calibration_label>` command.
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
 * \param [in] calibration the new calibration coefficients for the channel
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the calibration cannot be changed
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when no coefficients are
 *                                                populated
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828510/calibration
 * \see RBRInstrumentGen4_getCalibration()
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setCalibration(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Calibration *calibration);


/** 
 * \brief Instrument `settings` command parameters.
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828508/settings
 * \see RBRInstrumentGen4_getSettings()
 * \see RBRInstrumentGen4_setSettings()
 */
typedef struct RBRInstrumentGen4Settings{
    /**
     * \brief Whether the instrument returns the “Ready:” prompt following a
     * response. The as-shipped default value is on.
     */
    bool prompt;

    /**
     * \brief Whether the instrument returns a response from a create or
     * set/modify operation to verify the new state. The as-shipped default
     * value is on.
     * A response will always be sent when a parameter value is simply
     * requested.
     */
    bool confirmation;
} RBRInstrumentGen4Settings;

/**
 * \brief Get miscellaneous logger settings
 * \note Issues the instrument `settings` command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] settings the logger settings
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828508/settings
 * \see RBRInstrumentGen4_setSettings()
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getSettings(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Settings *settings);

/**
 * \brief Set the miscellaneous logger settings.
 * \note Issues the instrument `settings` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] settings the values for the settings in the logger
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the settings cannot be changed
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828508/settings
 * \see RBRInstrumentGen4_getSettings()
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setSettings(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Settings *settings);

/** 
 * \brief `parameters` command parameters.
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/42729486/parameters
 * \see RBRInstrumentGen4_getParameters()
 * \see RBRInstrumentGen4_setParameters()
 */
typedef struct RBRInstrumentGen4Parameters{
    /**
     * \brief the temperature coefficient used to correct the derived channel 
     * for specific conductivity to 25°C. Its value depends on the ionic 
     * composition of the water being monitored, and should be set to an 
     * appropriate value for best results. 
     * A typical range of values is 0.0191 to 0.0214, with the lower end 
     * suitable for KCl solutions and the upper end for NaCl solutions.
     */
    float specCondTempCo;
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
    /** \brief avgSoundSpeed in m/s, default value 1506.8*/
    float avgSoundSpeed;
} RBRInstrumentGen4Parameters;

/**
 * \brief Get parameters which may be required when computing calibrated output.
 * \note Issues the instrument `parameters` command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] settings the settings in the logger.
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/42729486/parameters
 * \see RBRInstrumentGen4_setParameters
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getParameters(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Parameters *parameters);

/**
 * \brief Set parameters which may be required when computing calibrated output.
 * \note Issues the instrument `parameters` command.
 *
 * \warning Hardware errors may occur if:
 * - the instrument is logging
 * - you set an out-of-bounds value the library fails to detect
 *
 * \param [in] instrument the instrument connection
 * \param [in] settings the values for the settings in the logger
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the settings cannot be changed
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/42729486/parameters
 * \see RBRInstrumentGen4_getParameters
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setParameters(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Parameters *parameters);

/**
 * \brief Commands that can be sent to the UV LED device.
 * \see RBRInstrumentGen4Uvled 
 */
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
} RBRInstrumentGen4UvledCommand;

/**
 * \brief Get a human-readable string name for uvled commands.
 *
 * \param [in] uvledCommand the uvled command.
 * \return a string name for the command.
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4UvledCommand_name(RBRInstrumentGen4UvledCommand uvledCommand);

/**
 * \brief `uvled` command parameters.
 * This is a specialized command available in firmware versions 1.130 
 * or later. It is used to return information about, and to allow some control 
 * over, the UV-LED antifouling device used in the RBRconcerto3 CTD|UV data 
 * logger, and other instruments equipped with the UV antifouling feature.
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830161/uvled 
 */
typedef struct RBRInstrumentGen4Uvled{
    /**
     * \brief The type of UV-device configured to be used with the instrument.
     * \readonly
     */
    const char id[RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];

    /** 
     * \brief Whether or not the UV-LED schedule is enabled for use.
     */
    bool scheduled;

    /** 
     * \brief The startup time foe the UV-LEDs in milliseconds.
     * \readonly
     */
    const RBRInstrumentGen4Period powerOnDelay;
    
    /**
     * \brief The shut down time for the UV-LEDs in milliseconds.
     * \readonly
     */
    const RBRInstrumentGen4Period powerOffDelay;

    /** \brief The interval between UV-LED episodes in milliseconds. */
    RBRInstrumentGen4Period interval;

    /** \brief The duration of an UV-LED episode in milliseconds. */
    RBRInstrumentGen4Period duration;

    /**
     * \brief The command applied at the beginning of a UV-LED episode.
     * \readonly
     */
    const char *startAction;

    /**
     * \brief The command applied at the end of a UV-LED episode.
     * \readonly
     */
    const char *endAction;

    /** \brief Sends a particular command to the UV-LED device. */
    RBRInstrumentGen4UvledCommand command;

    /** 
     * \brief The time remaining until the start of the next scheduled UV-LED episode.
     * \readonly
     */
    const RBRInstrumentGen4Period timeToEpisode;

    /**
     * \brief Whether the first UV-LED episode will occur at the start of the
     * schedule or a reset, or if the first UV LED episode will be delayed by
     * one interval after the start of the schedule or a reset.
     */
    bool startImmediate;

    /**
     * \brief The module address of the front end card controlling the UV-LED.
     * \readonly
     */
    const RBRInstrumentGen4ModuleAddress module;

    /**
     * \brief The cumulative time in milliseconds for which the UV-LEDs have been energized since configuration at the Factory. 
     * \readonly
     */
    const RBRInstrumentGen4Period operatingTime;

    /**
     * \brief Whether UV-LED events will be recorded in the instrument's memory when data logging is enabled.
     */
    bool episodeLog;
}RBRInstrumentGen4Uvled;

/** \brief This is a specialized command available in firmware versions 1.130 
 * or later. It is used to return information about, and to allow some control 
 * over, the UV-LED antifouling device used in the RBRconcerto3 CTD|UV data 
 * logger, and other instruments equipped with the UV antifouling feature. 
 * \note Issues the `uvled` instrument command.
 * 
 * \param [in] instrument the instrument connection
 * \param [out] uvled reports all the the UV-LED information about the UV-device.
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the UV-LED settings cannot be read
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830161/uvled
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getUvled(RBRInstrumentGen4 *instrument, RBRInstrumentGen4Uvled *uvled);

/** \brief This is a specialized command available in firmware versions 1.130 
 * or later.  It is used to return information about, and to allow some control 
 * over, the UV-LED antifouling device used in the RBRconcerto3 CTD|UV data 
 * logger, and other instruments equipped with the UV antifouling feature. 
 * \note Issues the `uvled` instrument command.
 * 
 * Hardware errors may occur if:
 * 
 * - you set an out-of-range or incorrect parameters
 * - you set too many parameters
 * - you set the wrong types of parameters
 *
 * \param [in] instrument the instrument connection.
 * \param [in] uvled all the the UV-LED information about the UV-device.
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the UV-LED settings cannot be changed
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830161/uvled
 */
RBRInstrumentGen4Error RBRInstrumentGen4Uvled_setUvled(RBRInstrumentGen4 *instrument, const RBRInstrumentGen4Uvled *uvled);

/**
 * \brief Instrument `group <group_label>` command parameters.
 *
 * \see RBRInstrumentGen4GroupPool
 * \see RBRInstrumentGen4Schedule
 * \see RBRInstrumentGen4_getGroup()
 * \see RBRInstrumentGen4_setGroup()
 * \see RBRInstrumentGen4_createGroup()
 * \see RBRInstrumentGen4_deleteGroup()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 */
typedef struct RBRInstrumentGen4Group {
    /**
     * \brief The group's label.
     * \warning It is subject to naming constraints.
     * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830131/Parameter+naming+constraints
     */
    char label[RBRINSTRUMENTGEN4_LABEL_NAME_MAX+1];

    /** \brief The number of channels in this group. */
    int32_t count;

    /**
     * \brief A list of all channel labels.
     * \note The order of this list is arbitrary and defined by the logger.
     */
    RBRInstrumentGen4Channel *channelList[RBRINSTRUMENTGEN4_CHANNEL_MAX];

    /**
     * \brief Pointer to the parent group pool.
     * \readonly
     */
    void *parent;
}RBRInstrumentGen4Group;

/**
 * \brief Instrument `group` command parameters.
 * Serves as a persistent cache of channel groupings configured on the logger.
 *
 * \see RBRInstrumentGen4_getGroupPool()
 * \see RBRInstrumentGen4_createGroup()
 * \see RBRInstrumentGen4_deleteGroup()
 * \see RBRInstrumentGen4_deleteGroupAll()
 * \see RBRInstrumentGen4_getSchedule()
 */
typedef struct RBRInstrumentGen4GroupPool {
    /** \brief The number of groups currently defined. */
    int32_t count;

    /** \brief List of channel objects. */
    RBRInstrumentGen4Group pool[RBRINSTRUMENTGEN4_GROUP_COUNT_MAX];
} RBRInstrumentGen4GroupPool;

/**
 * \brief Reports the properties of the specified channel grouping.
 * \note Issues the `group <group_label>` instrument command.
 *
 * Hardware errors may occur if:
 * - you specify a group that does not exist.
 *
 * \param [in] instrument the instrument connection
 * \param [in] channelPool the pool of the logger's channels to associate with the group
 * \param [inout] group the information of specified group
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the group is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the group cannot be read
 * \see RBRInstrumentGen4_getGroupPool()
 * \see RBRInstrumentGen4_setGroup()
 * \see RBRInstrumentGen4_createGroup()
 * \see RBRInstrumentGen4_deleteGroup()
 * \see RBRInstrumentGen4_deleteGroupAll()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getGroup(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4ChannelPool *channelPool,
    RBRInstrumentGen4Group *group);

/**
 * \brief Modifies the properties of the specified channel grouping.
 * \note Issues the `group <group_label>` instrument command.
 *
 * Hardware errors may occur if:
 *
 * - you include a channel that does not exist
 * - you include a channel multiple times
 * - you include too many channels
 *
 * \param [in] instrument the instrument connection
 * \param [in] group the information of the specified group
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the group is successfully changed
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the group cannot be changed
 * \see RBRInstrumentGen4_getGroupPool()
 * \see RBRInstrumentGen4_getGroup()
 * \see RBRInstrumentGen4_createGroup()
 * \see RBRInstrumentGen4_deleteGroup()
 * \see RBRInstrumentGen4_deleteGroupAll()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setGroup(
    RBRInstrumentGen4 *instrument, 
    RBRInstrumentGen4Group *group);

/**
 * \brief Creates a new empty group of channels.
 * \note Issues the `group create <group_label>` instrument command.
 *
 * Hardware errors may occur if:
 *
 * - you specify a group label that already exists
 * - you specify an invalid group label
 *
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830131/Parameter+naming+constraints
 *
 * \param [in] instrument the instrument connection
 * \param [in] newGroupLabel the label to give the new group
 * \param [inout] groupPool the pool of groups defined on the logger
 * \param [out] newGroup a pointer to the new group in groupPool
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the group is successfully created
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the group cannot be created
 * \see RBRInstrumentGen4_getGroupPool()
 * \see RBRInstrumentGen4_getGroup()
 * \see RBRInstrumentGen4_setGroup()
 * \see RBRInstrumentGen4_deleteGroup()
 * \see RBRInstrumentGen4_deleteGroupAll()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 */
RBRInstrumentGen4Error RBRInstrumentGen4_createGroup(
    RBRInstrumentGen4 *instrument,
    const char *newGroupLabel,
    RBRInstrumentGen4GroupPool *groupPool,
    RBRInstrumentGen4Group **newGroup);

/**
 * \brief Deletes a specific channel group.
 * \note Issues the `group delete <group_label>` instrument command.
 *
 * Hardware errors may occur if:
 *
 * - you specify a group that does not exist
 *
 * \param [in] instrument the instrument connection
 * \param [in] groupToDelete the single group to delete from the pool
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the group is successfully deleted
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the group cannot be deleted
 * \see RBRInstrumentGen4_getGroupPool()
 * \see RBRInstrumentGen4_getGroup()
 * \see RBRInstrumentGen4_setGroup()
 * \see RBRInstrumentGen4_createGroup()
 * \see RBRInstrumentGen4_deleteGroupAll()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 */
RBRInstrumentGen4Error RBRInstrumentGen4_deleteGroup(
    RBRInstrumentGen4 *instrument, 
    RBRInstrumentGen4Group *groupToDelete);

/**
 * \brief Deletes all channel groups.
 * \note Issues the `group delete all` instrument command.
 *
 * \param [in] instrument the instrument connection 
 * \param [inout] groupPoolToDelete the pool of groups defined on the logger
 * \return #RBRINSTRUMENTGEN4_SUCCESS when all groups are successfully deleted
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if any group cannot be deleted
 * \see RBRInstrumentGen4_getGroupPool()
 * \see RBRInstrumentGen4_getGroup()
 * \see RBRInstrumentGen4_setGroup()
 * \see RBRInstrumentGen4_createGroup()
 * \see RBRInstrumentGen4_deleteGroup()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 */
RBRInstrumentGen4Error RBRInstrumentGen4_deleteGroupAll(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4GroupPool *groupPoolToDelete);

/**
 * \brief Populate the pool of channel groupings wuth the labels of the groups defined on the logger.
 * \note Issues the `group` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] groupPool the pool of groups defined on the logger
 * \return #RBRINSTRUMENTGEN4_SUCCESS when all groups are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if any group cannot be read
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 * \see RBRInstrumentGen4_getGroup()
 * \see RBRInstrumentGen4_setGroup()
 * \see RBRInstrumentGen4_createGroup()
 * \see RBRInstrumentGen4_deleteGroup()
 * \see RBRInstrumentGen4_deleteGroupAll()
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getGroupPool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4GroupPool *groupPool);

/**
 * \brief Possible instrument sampling modes for a schedule.
 *
 * \see RBRInstrumentGen4Schedule
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
typedef enum RBRInstrumentGen4SamplingMode
{
    /** Continuous schedule mode. */
    RBRINSTRUMENTGEN4_SAMPLING_CONTINUOUS,
    /** Direction-dependent schedule mode.  */
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

/**
 * \brief Defines schedule mode-dependent-parameters: continuous mode.
 * \see RBRInstrumentGen4Schedule
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
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
    bool castDetection;
} RBRInstrumentGen4Continuous;

/**
 * \brief Defines schedule mode-dependent-parameters: average mode.
 * \see RBRInstrumentGen4Schedule
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
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
     * burst count (RBRInstrumentGen4Schedule.burstCount):
     *
     *     burst interval > (burst count × sampling period)
     */
    RBRInstrumentGen4Period burstInterval;

    /**
     * \brief The number of measurements taken in each burst.
     *
     * Specified in numbers of samples. Must be in the range 2—65,535.
     */
    int32_t burstCount;
} RBRInstrumentGen4Average;

/**
 * \brief Defines schedule mode-dependent-parameters: tide mode.
 * \see RBRInstrumentGen4Schedule
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
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
     * burst count (RBRInstrumentGen4Schedule.burstCount):
     *
     *     burst interval > (burst count × sampling period)
     */
    RBRInstrumentGen4Period burstInterval;

    /**
     * \brief The number of measurements taken in each burst.
     *
     * Specified in numbers of samples. Must be in the range 2—65,535.
     */
    int32_t burstCount;
} RBRInstrumentGen4Tide;

/**
 * \brief Defines schedule mode-dependent-parameters: burst modes.
 * \see RBRInstrumentGen4Schedule
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
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
     * burst count (RBRInstrumentGen4Schedule.burstCount):
     *
     *     burst interval > (burst count × sampling period)
     */
    RBRInstrumentGen4Period burstInterval;

    /**
     * \brief The number of measurements taken in each burst.
     *
     * Specified in numbers of samples. Must be in the range 2—65,535.
     */
    int32_t burstCount;
} RBRInstrumentGen4Burst;

/**
 * \brief Defines schedule mode-dependent-parameters: wave modes.
 * \see RBRInstrumentGen4Schedule
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
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
     * burst count (RBRInstrumentGen4Schedule.burstCount):
     *
     *     burst interval > (burst count × sampling period)
     */
    RBRInstrumentGen4Period burstInterval;

    /**
     * \brief The number of measurements taken in each burst.
     *
     * Specified in numbers of samples. Must be in the range 2—65,535.
     */
    int32_t burstCount;
} RBRInstrumentGen4Wave;

/**
 * \brief Whether settings apply to ascent or descent.
 * \see RBRInstrumentGen4Regimes
 * \see RBRInstrumentGen4DirectionDependentSampling
 * \see RBRInstrumentGen4Schedule
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
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

/**
 * \brief Defines schedule mode-dependent-parameters: ddsampling mode.
 * \see RBRInstrumentGen4Schedule
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
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
   bool castDetection;

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
 * \see RBRInstrumentGen4Schedule
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
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
 * \see RBRInstrumentGen4Schedule
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
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
 * \brief Destinations for a schedule's real-time data.
 *
 * \see RBRInstrumentGen4Schedule.stream
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
typedef enum RBRInstrumentGen4ScheduleStream
{
    /** Data for this schedule is not streamed in real time. */
    RBRINSTRUMENTGEN4_SCHEDULE_STREAM_OFF,
    /** Data for this schedule is streamed over the USB CDC link. */
    RBRINSTRUMENTGEN4_SCHEDULE_STREAM_USB,
    /** Data for this schedule is streamed over the serial link. */
    RBRINSTRUMENTGEN4_SCHEDULE_STREAM_SERIAL,
    /** The number of specific stream destinations. */
    RBRINSTRUMENTGEN4_SCHEDULE_STREAM_COUNT,
    /** An unknown or unrecognized stream destination. */
    RBRINSTRUMENTGEN4_UNKNOWN_SCHEDULE_STREAM
} RBRInstrumentGen4ScheduleStream;

/**
 * \brief Get a human-readable string name for a stream destination.
 *
 * \param [in] stream the stream destination
 * \return a string name for the stream destination
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4ScheduleStream_name(
    RBRInstrumentGen4ScheduleStream stream);

/**
 * \brief Instrument `schedule <schedule_label>` parameters.
 *
 * \see RBRInstrumentGen4SchedulePool
 * \see RBRInstrumentGen4Config
 * \see RBRInstrumentGen4_getSchedule()
 * \see RBRInstrumentGen4_setSchedule()
 * \see RBRInstrumentGen4_createSchedule()
 * \see RBRInstrumentGen4_deleteSchedule()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
typedef struct RBRInstrumentGen4Schedule
{
    /**
     * \brief The schedule's label.
     * \warning It is subject to naming constraints.
     * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830131/Parameter+naming+constraints
     */
    char label[RBRINSTRUMENTGEN4_LABEL_NAME_MAX+1];

    /** \brief The number of groups in this schedule. */
    int32_t count;

    /**
     * \brief A list of groups defining the channels that will be sampled
     * according to this schedule.
     */
    RBRInstrumentGen4Group *groupList[RBRINSTRUMENTGEN4_GROUP_COUNT_MAX];

    /**
     * \brief The communication link over which data for this schedule will be
     * streamed in real time during the deployment. At most one link may be
     * active for each schedule.
     * Defaults to #RBRINSTRUMENTGEN4_SCHEDULE_STREAM_OFF when the schedule is
     * created.
     */
    RBRInstrumentGen4ScheduleStream stream;

    /** 
     * \brief Whether data for this schedule will be sored in memory during the
     * deployment. 
     * For data logging instruments which can store data, this parameter
     * defaults to on when the schedule is created. For sensor instruments that
     * do not store data, schedules are created with this parameter set to off,
     * and the value can not be changed.
     */
    bool storage;

    /** \brief The sampling mode to be used by this schedule. */
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
    } modeDependentParameters;

    /**
     * \brief Pointer to the parent schedule pool.
     * \readonly
     */
    void *parent;
} RBRInstrumentGen4Schedule;

/**
 * \brief The sampling modes currently available in the instruments.
 * \see RBRInstrumentGen4SchedulePool
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
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

/**
 * \brief Instrument `schedule` command parameters.
 * Serves as a persistent cache of channel structs.
 *
 * \see RBRInstrumentGen4_getSchedulePool()
 * \see RBRInstrumentGen4_createSchedule()
 * \see RBRInstrumentGen4_deleteSchedule()
 * \see RBRInstrumentGen4_deleteScheduleAll()
 * \see RBRInstrumentGen4_getConfig()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
typedef struct RBRInstrumentGen4SchedulePool{
    /** \brief The number of schedules currently defined. */
    const int32_t count;

    /** \brief List of schedule objects. */
    RBRInstrumentGen4Schedule pool[RBRINSTRUMENTGEN4_SCHEDULE_COUNT_MAX];

    /** \brief lists all the sampling modes configured to be available in the instruments.
     * Items are specified as a bitfield.
     *
     * \readonly
     */
    const RBRInstrumentGen4Availablemodes availableModes;

    /**
     * \brief Fast measurement periods available for the logger for sampling
     * rates faster than 1Hz.
     *
     * Available fast periods are stored in the array in the order reported by
     * the instrument. Unused array elements are populated with `0`. If more
     * than #RBRINSTRUMENTGEN4_AVAILABLE_FAST_PERIODS_MAX are available, trailing
     * entries are discarded.
     *
     * \readonly
     */
    const RBRInstrumentGen4Period availableFastPeriods[RBRINSTRUMENTGEN4_AVAILABLE_FAST_PERIODS_MAX];
} RBRInstrumentGen4SchedulePool;

/**
 * \brief Instrument `config <config_label>` command parameters.
 *
 * \see RBRInstrumentGen4ConfigPool
 * \see RBRInstrumentGen4_getConfig()
 * \see RBRInstrumentGen4_setConfig()
 * \see RBRInstrumentGen4_createConfig()
 * \see RBRInstrumentGen4_deleteConfig()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
typedef struct RBRInstrumentGen4Config {
    /** \brief the configuration's label. */
    char label[RBRINSTRUMENTGEN4_LABEL_NAME_MAX+1];

    /** \brief The number of schedules in this config. */
    int32_t count;

    /**
     * \brief A list of schedules to be executed when this configuration is used to 
     * enable a deployment.
     * \note The order of schedules does not matter.
     * \note Any modifications made to a configuration will apply only when it
     * is used for future deployments; historical datasetPool in the logger's
     * memory are not affected.
     */
    RBRInstrumentGen4Schedule *scheduleList[RBRINSTRUMENTGEN4_SCHEDULE_COUNT_MAX];

    /**
     * \brief Pointer to the parent config pool.
     * \readonly
     */
    void *parent;
} RBRInstrumentGen4Config;

/** 
 * \brief Instrument `config` channel parameters.
 * Serves as a persistent cache of config structs.
 *
 * \see RBRInstrumentGen4_getConfigPool()
 * \see RBRInstrumentGen4_createConfig()
 * \see RBRInstrumentGen4_deleteConfig()
 * \see RBRInstrumentGen4_deleteConfigAll()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
typedef struct RBRInstrumentGen4ConfigPool{
    /** \brief The number of all defined configurations. */
    int32_t count;

    /**
     * \brief The array of pointers for the label of each defined configuration as null-terminated C strings.
     */
    RBRInstrumentGen4Config pool[RBRINSTRUMENTGEN4_CONFIG_COUNT_MAX];
} RBRInstrumentGen4ConfigPool;

/**
 * \brief Reports the properties of the specified configuration. 
 * \note Issues the `config <config_label>` instrument command.
 *
 * Hardware errors may occur if:
 *
 * - you specify a config that does not exist
 *
 * \param [in] instrument the instrument connection
 * \param [in] schedulePool the pool of the logger's schedules to associate with the config
 * \param [out] config the configuration reported
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the config is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the config cannot be read
 * \see RBRInstrumentGen4_getConfigPool()
 * \see RBRInstrumentGen4_setConfig()
 * \see RBRInstrumentGen4_createConfig()
 * \see RBRInstrumentGen4_deleteConfig()
 * \see RBRInstrumentGen4_deleteConfigAll()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getConfig(
    RBRInstrumentGen4 *instrument, 
    RBRInstrumentGen4SchedulePool *schedulePool,
    RBRInstrumentGen4Config *config);

/**
 * \brief Modifies the properties of the specified configuration. 
 * \note Issues the `config <config_label>` instrument command.
 *
 * Hardware errors may occur if:
 *
 * - you include a schedule that does not exist
 * - you include a schedule multiple times
 * - you include too many schedules
 *
 * \param [in] instrument the instrument connection
 * \param [in] config the configuration to be modified by label
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the config is successfully changed
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the config cannot be changed
 * \see RBRInstrumentGen4_getConfigPool()
 * \see RBRInstrumentGen4_getConfig()
 * \see RBRInstrumentGen4_createConfig()
 * \see RBRInstrumentGen4_deleteConfig()
 * \see RBRInstrumentGen4_deleteConfigAll()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setConfig(
    RBRInstrumentGen4 *instrument, 
    RBRInstrumentGen4Config *config);

/**
 * \brief Reports information about the pool of logger configurations.
 * \note Issues the `config` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] configPool the pool of logger configurations
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the configs are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if any config cannot be read
 * \see RBRInstrumentGen4_getConfig()
 * \see RBRInstrumentGen4_setConfig()
 * \see RBRInstrumentGen4_createConfig()
 * \see RBRInstrumentGen4_deleteConfig()
 * \see RBRInstrumentGen4_deleteConfigAll()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getConfigPool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4ConfigPool *configPool);

/**
 * \brief Creates a new empty configuration.
 * \note Issues the `config create <config_label>` instrument command.
 * 
 * Hardware errors may occur if:
 *
 * - you specify a config label that already exists
 * - you specify an invalid config label
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830131/Parameter+naming+constraints
 *
 * \param [in] instrument the instrument connection
 * \param [in] newConfigLabel the label to give the new config
 * \param [out] config a pointer to the new config
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the config is successfully created
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the config cannot be created
 * \see RBRInstrumentGen4_getConfigPool()
 * \see RBRInstrumentGen4_getConfig()
 * \see RBRInstrumentGen4_setConfig()
 * \see RBRInstrumentGen4_deleteConfig()
 * \see RBRInstrumentGen4_deleteConfigAll()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
RBRInstrumentGen4Error RBRInstrumentGen4_createConfig(
    RBRInstrumentGen4 *instrument, 
    const char *newConfigLabel,
    RBRInstrumentGen4ConfigPool *configPool,
    RBRInstrumentGen4Config **newConfig);

/**
 * \brief Deletes a configuration with optional user defined parameters.
 * \note Issues the `config delete <config_label>` instrument command.
 *
 * Hardware errors may occur if:
 *
 * - you specify a config that does not exist
 *
 * \param [in] instrument the instrument connection
 * \param [in] configToDelete the single configuration to delete
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the config is successfully deleted
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the config cannot be deleted
 * \see RBRInstrumentGen4_getConfigPool()
 * \see RBRInstrumentGen4_getConfig()
 * \see RBRInstrumentGen4_setConfig()
 * \see RBRInstrumentGen4_createConfig()
 * \see RBRInstrumentGen4_deleteConfigAll()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
RBRInstrumentGen4Error RBRInstrumentGen4_deleteConfig(
    RBRInstrumentGen4 *instrument, 
    RBRInstrumentGen4Config *configToDelete);

/**
 * \brief Deletes all configurations.
 * \note Issues the `config delete all` instrument command.
 *
 * Deleted configurations can no longer be used to enable deployments.
 * Historical deployments in the logger memory that used the deleted
 * configuration are not affected; all datasetPool include as part of their metadata
 * a snapshot of the configuration when the logger was enabled.
 *
 * \param [in] instrument the instrument connection
 * \param [inout] configPoolToDelete the pool of configs defined on the logger
 * \return #RBRINSTRUMENTGEN4_SUCCESS when all the configs are successfully deleted
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if any config cannot be deleted
 * \see RBRInstrumentGen4_getConfigPool()
 * \see RBRInstrumentGen4_getConfig()
 * \see RBRInstrumentGen4_setConfig()
 * \see RBRInstrumentGen4_createConfig()
 * \see RBRInstrumentGen4_deleteConfig()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
RBRInstrumentGen4Error RBRInstrumentGen4_deleteConfigAll(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4ConfigPool *configPoolToDelete);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTCONFIGURATION_H */
