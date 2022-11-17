/**
 * \file RBRInstrumentStreaming.h
 *
 * \brief Instrument commands and structures pertaining to real-time data
 * acquisition.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTSTREAMING_H
#define LIBRBR_RBRINSTRUMENTSTREAMING_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief The maximum number of characters in an output format name (e.g.,
 * “caltext01”).
 */
#define RBRINSTRUMENT_OUTPUT_FORMAT_NAME_MAX 15

/**
 * \brief Response to the `outputformat channelslist` command.
 *
 * \see RBRInstrument_getChannelsList()
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 */
typedef struct RBRInstrumentChannelsList
{
    /** \brief The number of active channels. */
    int32_t count;
    /** \brief The name and unit of each active channel. */
    struct
    {
        /** \brief The name of the channel as a null-terminated C string. */
        char name[RBRINSTRUMENT_CHANNEL_NAME_MAX + 1];
        /** \brief The unit of the channel as a null-terminated C string. */
        char unit[RBRINSTRUMENT_CHANNEL_UNIT_MAX + 1];
    } channels[RBRINSTRUMENT_CHANNEL_MAX];
} RBRInstrumentChannelsList;

/**
 * \brief Report a list of names and units for active channels, in order.
 *
 * Helpful for identifying the channel corresponding to each value in the
 * transmitted data.
 *
 * RBRInstrumentChannelsList.channels will be populated in the order reported
 * by the instrument. Unpopulated entries will have zero-length name and unit
 * members.
 *
 * \nol2 Use RBRInstrument_getChannels() and RBRInstrument_getChannel().
 *
 * \param [in] instrument the instrument connection
 * \param [out] channelsList the channels list
 * \return #RBRINSTRUMENT_UNSUPPORTED for Logger2 instruments
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the command is unavailable
 * \see RBRInstrument_getLabelsList()
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 */
RBRInstrumentError RBRInstrument_getChannelsList(
    RBRInstrument *instrument,
    RBRInstrumentChannelsList *channelsList);

/**
 * \brief Response to the `outputformat labelslist` command.
 *
 * \see RBRInstrument_getLabelsList()
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 */
typedef struct RBRInstrumentLabelsList
{
    /** \brief The number of active channels. */
    int32_t count;
    /**
     * \brief The label for each active channel as null-terminated C strings.
     */
    char labels[RBRINSTRUMENT_CHANNEL_MAX][RBRINSTRUMENT_CHANNEL_LABEL_MAX + 1];
} RBRInstrumentLabelsList;

/**
 * \brief Report a list of labels for active channels, in order.
 *
 * Helpful for identifying the channel corresponding to each value in the
 * transmitted data.
 *
 * RBRInstrumentLabelsList.channels will be populated in the order reported
 * by the instrument. Unpopulated entries will be zero-length.
 *
 * \nol2 Use RBRInstrument_getChannels() and RBRInstrument_getChannel().
 *
 * \param [in] instrument the instrument connection
 * \param [out] labelsList the channel labels list
 * \return #RBRINSTRUMENT_UNSUPPORTED for Logger2 instruments
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the command is unavailable
 * \see RBRInstrument_getChannelsList()
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 */
RBRInstrumentError RBRInstrument_getLabelsList(
    RBRInstrument *instrument,
    RBRInstrumentLabelsList *labelsList);

/**
 * \brief Instrument output formats.
 *
 * \see RBRInstrument_getAvailableOutputFormats()
 * \see RBRInstrument_getOutputFormat()
 * \see RBRInstrument_setOutputFormat()
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 */
typedef enum RBRInstrumentOutputFormat
{
    /** No format. */
    RBRINSTRUMENT_OUTFORMAT_NONE      = 0,
    /** Physical units to 4 decimal places. */
    RBRINSTRUMENT_OUTFORMAT_CALTEXT01 = 1 << 0,
    /** Physical units to 4 decimal places with units. */
    RBRINSTRUMENT_OUTFORMAT_CALTEXT02 = 1 << 1,
    /**
     * Physical units with sufficient significant digits to ensure no
     * resolution loss.
     */
    RBRINSTRUMENT_OUTFORMAT_CALTEXT03 = 1 << 2,
    /** Physical units expressed as “engineering-notation” floating point. */
    RBRINSTRUMENT_OUTFORMAT_CALTEXT04 = 1 << 3,
    /** Physical units to 4 decimal places. 
     *  The output starts with the keyword "RBR" followed by the serial number. 
     *  This format is available for LOGGER3 with fw 1.109 or later*/
    RBRINSTRUMENT_OUTFORMAT_CALTEXT07 = 1 << 4,
    /** Corresponds to the largest output format enum value. */
    RBRINSTRUMENT_OUTFORMAT_MAX       = RBRINSTRUMENT_OUTFORMAT_CALTEXT07
} RBRInstrumentOutputFormat;

/**
 * \brief Get a human-readable string name for an output format.
 *
 * \param [in] format the output format
 * \return a string name for the output format
 * \see RBRInstrumentError_name() for a description of the format of names
 */
const char *RBRInstrumentOutputFormat_name(RBRInstrumentOutputFormat format);

/**
 * \brief Report a list of available output formats.
 *
 * \a outputFormats will be treated as a bit field representation of available
 * output formats as defined by RBRInstrumentOutputFormat. For details, consult
 * [Working with Bit Fields](bitfields.md).
 *
 * \param [in] instrument the instrument connection
 * \param [out] outputFormats available output formats
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 */
RBRInstrumentError RBRInstrument_getAvailableOutputFormats(
    RBRInstrument *instrument,
    RBRInstrumentOutputFormat *outputFormats);

/**
 * \brief Get the current output format.
 *
 * \param [in] instrument the instrument connection
 * \param [out] outputFormat the current output format
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 */
RBRInstrumentError RBRInstrument_getOutputFormat(
    RBRInstrument *instrument,
    RBRInstrumentOutputFormat *outputFormat);

/**
 * \brief Set the current output format.
 *
 * \param [in] instrument the instrument connection
 * \param [in] outputFormat the current output format
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when an unavailable output format is
 *                                       selected
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 */
RBRInstrumentError RBRInstrument_setOutputFormat(
    RBRInstrument *instrument,
    RBRInstrumentOutputFormat outputFormat);

/**
 * \brief Get the USB streaming state.
 *
 * \param [in] instrument the instrument connection
 * \param [out] enabled whether USB streaming is enabled
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when USB streaming is unavailable
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/streamusb
 */
RBRInstrumentError RBRInstrument_getUSBStreamingState(
    RBRInstrument *instrument,
    bool *enabled);

/**
 * \brief Set the USB streaming state.
 *
 * \param [in] instrument the instrument connection
 * \param [in] enabled whether USB streaming is enabled
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when USB streaming is unavailable
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/streamusb
 */
RBRInstrumentError RBRInstrument_setUSBStreamingState(
    RBRInstrument *instrument,
    bool enabled);

/**
 * \brief Get the serial streaming state.
 *
 * \param [in] instrument the instrument connection
 * \param [out] enabled whether serial streaming is enabled
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when serial streaming is unavailable
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/streamserial
 */
RBRInstrumentError RBRInstrument_getSerialStreamingState(
    RBRInstrument *instrument,
    bool *enabled);

/**
 * \brief Set the serial streaming state.
 *
 * \param [in] instrument the instrument connection
 * \param [in] enabled whether serial streaming is enabled
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when serial streaming is unavailable
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/streamserial
 */
RBRInstrumentError RBRInstrument_setSerialStreamingState(
    RBRInstrument *instrument,
    bool enabled);

/**
 * \brief Possible levels of the auxiliary output signal during the setup time,
 * data transmission, and hold time.
 *
 * \see RBRInstrumentAuxOutput
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/streamserial
 */
typedef enum RBRInstrumentAuxOutputActiveLevel
{
    /* Signal actively driven high. */
    RBRINSTRUMENT_ACTIVE_HIGH,
    /* Signal actively driven low. */
    RBRINSTRUMENT_ACTIVE_LOW,
    /** The number of active output levels. */
    RBRINSTRUMENT_ACTIVE_COUNT,
    /** An unknown or unrecognized active output level. */
    RBRINSTRUMENT_UNKNOWN_ACTIVE
} RBRInstrumentAuxOutputActiveLevel;

/**
 * \brief Get a human-readable string name for a signal level of an active
 * auxiliary output.
 *
 * \param [in] level the signal level
 * \return a string name for the signal level
 * \see RBRInstrumentError_name() for a description of the format of names
 */
const char *RBRInstrumentAuxOutputActiveLevel_name(
    RBRInstrumentAuxOutputActiveLevel level);

/**
 * \brief Possible levels of the auxiliary output signal while the instrument
 * is asleep.
 *
 * \see RBRInstrumentAuxOutput
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/streamserial
 */
typedef enum RBRInstrumentAuxOutputSleepLevel
{
    /* Passive, high-impedance signal. */
    RBRINSTRUMENT_SLEEP_TRISTATE,
    /* Signal actively driven high. */
    RBRINSTRUMENT_SLEEP_HIGH,
    /* Signal actively driven low. */
    RBRINSTRUMENT_SLEEP_LOW,
    /** The number of sleep output levels. */
    RBRINSTRUMENT_SLEEP_COUNT,
    /** An unknown or unrecognized sleep output level. */
    RBRINSTRUMENT_UNKNOWN_SLEEP
} RBRInstrumentAuxOutputSleepLevel;

/**
 * \brief Get a human-readable string name for a signal level of a sleeping
 * auxiliary output.
 *
 * \param [in] level the signal level
 * \return a string name for the signal level
 * \see RBRInstrumentError_name() for a description of the format of names
 */
const char *RBRInstrumentAuxOutputSleepLevel_name(
    RBRInstrumentAuxOutputSleepLevel level);

/**
 * \brief Instrument `streamserial` command parameters relating to the
 * auxiliary output signal functionality.
 *
 * \see RBRInstrument_getAuxOutput()
 * \see RBRInstrument_setAuxOutput()
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/streamserial
 */
typedef struct RBRInstrumentAuxOutput
{
    /**
     * \brief Which auxiliary output signal settings to retrieve or configure.
     *
     * Can currently only ever be set to `1` (AUX1).
     */
    uint8_t aux;
    /** \brief Enables or disables the auxiliary output. */
    bool enabled;
    /**
     * \brief The signal set-up time.
     *
     * When the logger is sampling and is about to stream data over the serial
     * link, this is the time for which the auxiliary output signal will be
     * held at the active level before the streaming transmission begins.
     *
     * Specified in milliseconds. Must be in the range 10—120,000 (10 ms to 2
     * minutes). The default value is 1,000.
     */
    int32_t setup;
    /**
     * \brief The signal hold time.
     *
     * This is the time for which the auxiliary output signal will be held at
     * the active level after the streaming transmission has finished.
     *
     * Specified in milliseconds. Must be in the range 10—120,000 (10 ms to 2
     * minutes). The default value is 1,000.
     */
    int32_t hold;
    /**
     * \brief The active level of the auxiliary output signal seen by the
     * external device during the setup time, data transmission, and hold time.
     */
    RBRInstrumentAuxOutputActiveLevel active;
    /**
     * \brief The level of the auxiliary output signal seen by the external
     * device while the instrument is asleep.
     */
    RBRInstrumentAuxOutputSleepLevel sleep;
} RBRInstrumentAuxOutput;

/**
 * \brief Get the instrument auxiliary output signal parameters.
 *
 * RBRInstrumentAuxOutput.aux must be set to the index of the auxiliary output
 * for which signal parameters are to be retrieved. Currently, it can only ever
 * be set to `1` (AUX1). For example:
 *
 * ~~~{.c}
 * RBRInstrumentAuxOutput auxOutput;
 * auxOutput.aux = 1;
 * RBRInstrument_getAuxOutput(instrument, &auxOutput);
 * ~~~
 *
 * \param [in] instrument the instrument connection
 * \param [in,out] auxOutput the auxiliary output signal parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the auxiliary output signal is
 *                                       unavailable
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when the auxiliary output
 *                                                signal index is not `1`
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/streamserial
 */
RBRInstrumentError RBRInstrument_getAuxOutput(
    RBRInstrument *instrument,
    RBRInstrumentAuxOutput *auxOutput);

/**
 * \brief Set the instrument auxiliary output signal parameters.
 *
 * Hardware errors may occur if:
 *
 * - the auxiliary output signal is not available for the instrument
 * - you set an out-of-bounds parameter the library fails to detect
 *
 * \param [in] instrument the instrument connection
 * \param [out] auxOutput the auxiliary output signal parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/streamserial
 */
RBRInstrumentError RBRInstrument_setAuxOutput(
    RBRInstrument *instrument,
    const RBRInstrumentAuxOutput *auxOutput);

/**
 * \brief A flag set on a sample reading.
 */
typedef enum RBRInstrumentReadingFlag
{
    /** No flag. */
    RBRINSTRUMENT_READING_FLAG_NONE,
    /** The channel is uncalibrated. */
    RBRINSTRUMENT_READING_FLAG_UNCALIBRATED,
    /** The reading is an error. */
    RBRINSTRUMENT_READING_FLAG_ERROR,
    /** The number of reading flags. */
    RBRINSTRUMENT_READING_FLAG_COUNT,
    /** An unknown or unrecognized reading flag. */
    RBRINSTRUMENT_UNKNOWN_READING_FLAG
} RBRInstrumentReadingFlag;

/**
 * \brief Get a human-readable string name for a reading flag.
 *
 * \param [in] flag the reading flag
 * \return a string name for the reading flag
 * \see RBRInstrumentError_name() for a description of the format of names
 */
const char *RBRInstrumentReadingFlag_name(RBRInstrumentReadingFlag flag);

/**
 * \brief Get the error flag from a reading.
 *
 * If the reading is not a NaN, returns the error flag encoded within the NaN.
 * Otherwise, returns #RBRINSTRUMENT_READING_FLAG_NONE.
 *
 * \param reading the reading
 * \return the error flag of the reading, if present
 * \see RBRInstrumentReading_getError() to get the error value, if present
 * \see RBRInstrumentReading_setError() to create a reading with an error set
 */
RBRInstrumentReadingFlag RBRInstrumentReading_getFlag(double reading);

/**
 * \brief Get the error value from a reading.
 *
 * If the reading is not a NaN, returns the error value encoded within the NaN.
 * Otherwise, returns 0.
 *
 * \param reading the reading
 * \return the error value of the reading, if present
 * \see RBRInstrumentReading_getFlag() to get the error flag, if present
 * \see RBRInstrumentReading_setError() to create a reading with an error set
 */
uint8_t RBRInstrumentReading_getError(double reading);

/**
 * \brief Synthesize a reading with an error set.
 *
 * \param flag the error flag
 * \param value the error value
 * \return the error reading
 * \see RBRInstrumentReading_getFlag() to get the error flag, if present
 * \see RBRInstrumentReading_getError() to get the error value, if present
 */
double RBRInstrumentReading_setError(RBRInstrumentReadingFlag flag,
                                     uint8_t value);

/**
 * \brief An instrument sample.
 */
typedef struct RBRInstrumentSample
{
    /** \brief The timestamp of the sample. */
    RBRInstrumentDateTime timestamp;
    /** \brief The number of populated sample readings. */
    int32_t channels;
    /**
     * \brief The sample readings.
     *
     * Only the first RBRInstrumentSample.channels readings will be populated.
     * Other readings will be set to 0.
     *
     * Readings are represented as double-precision floating point. If they
     * need to encode an error, it's stored in the trailing bits of a NaN, and
     * a flag is set to indicate which sort of error.
     *
     * \see RBRInstrumentReading_getFlag() to get the error flag, if present
     * \see RBRInstrumentReading_getError() to get the error value, if present
     * \see RBRInstrumentReading_setError() to synthesize a error reading
     */
    double readings[RBRINSTRUMENT_CHANNEL_MAX];
} RBRInstrumentSample;

/**
 * \brief Retrieve and parse data streamed from the instrument.
 *
 * This function waits for a streamed sample to arrive, parses it, then calls
 * the RBRInstrumentSampleCallback provided to the instrument via
 * RBRInstrumentCallbacks.sample.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENT_SUCCESS when a streaming sample has been read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_fetchSample() for on-demand sample fetching
 */
RBRInstrumentError RBRInstrument_readSample(RBRInstrument *instrument);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTSTREAMING_H */
