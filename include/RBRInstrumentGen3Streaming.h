/**
 * \file RBRInstrumentGen3Streaming.h
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

#ifndef LIBRBR_RBRINSTRUMENTGEN3STREAMING_H
#define LIBRBR_RBRINSTRUMENTGEN3STREAMING_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief The maximum number of characters in an output format name (e.g.,
 * “caltext01”).
 */
#define RBRINSTRUMENTGEN3_OUTPUT_FORMAT_NAME_MAX 15

/**
 * \brief Response to the `outputformat channelslist` command.
 *
 * \see RBRInstrumentGen3_getChannelsList()
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 */
typedef struct RBRInstrumentGen3ChannelsList
{
    /** \brief The number of active channels. */
    int32_t count;
    /** \brief The name and unit of each active channel. */
    struct
    {
        /** \brief The name of the channel as a null-terminated C string. */
        char name[RBRGEN3_CHANNEL_NAME_MAX + 1];
        /** \brief The unit of the channel as a null-terminated C string. */
        char unit[RBRGEN3_CHANNEL_UNIT_MAX + 1];
    } channels[RBRGEN3_CHANNEL_MAX];
} RBRInstrumentGen3ChannelsList;

/**
 * \brief Report a list of names and units for active channels, in order.
 *
 * Helpful for identifying the channel corresponding to each value in the
 * transmitted data.
 *
 * RBRInstrumentGen3ChannelsList.channels will be populated in the order reported
 * by the instrument. Unpopulated entries will have zero-length name and unit
 * members.
 *
 * \nol2 Use RBRGen3_getChannels() and RBRGen3_getChannel().
 *
 * \param [in] instrument the instrument connection
 * \param [out] channelsList the channels list
 * \return #RBRGEN3_UNSUPPORTED for Logger2 instruments
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the command is unavailable
 * \see RBRInstrumentGen3_getLabelsList()
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 */
RBRGen3Error RBRInstrumentGen3_getChannelsList(
    RBRGen3 *instrument,
    RBRInstrumentGen3ChannelsList *channelsList);

/**
 * \brief Response to the `outputformat labelslist` command.
 *
 * \see RBRInstrumentGen3_getLabelsList()
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 */
typedef struct RBRInstrumentGen3LabelsList
{
    /** \brief The number of active channels. */
    int32_t count;
    /**
     * \brief The label for each active channel as null-terminated C strings.
     */
    char labels[RBRGEN3_CHANNEL_MAX][RBRGEN3_CHANNEL_LABEL_MAX + 1];
} RBRInstrumentGen3LabelsList;

/**
 * \brief Report a list of labels for active channels, in order.
 *
 * Helpful for identifying the channel corresponding to each value in the
 * transmitted data.
 *
 * RBRInstrumentGen3LabelsList.channels will be populated in the order reported
 * by the instrument. Unpopulated entries will be zero-length.
 *
 * \nol2 Use RBRGen3_getChannels() and RBRGen3_getChannel().
 *
 * \param [in] instrument the instrument connection
 * \param [out] labelsList the channel labels list
 * \return #RBRGEN3_UNSUPPORTED for Logger2 instruments
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the command is unavailable
 * \see RBRInstrumentGen3_getChannelsList()
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 */
RBRGen3Error RBRInstrumentGen3_getLabelsList(
    RBRGen3 *instrument,
    RBRInstrumentGen3LabelsList *labelsList);

/**
 * \brief Instrument output formats.
 *
 * \see RBRInstrumentGen3_getAvailableOutputFormats()
 * \see RBRInstrumentGen3_getOutputFormat()
 * \see RBRInstrumentGen3_setOutputFormat()
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 */
typedef enum RBRInstrumentGen3OutputFormat
{
    /** No format. */
    RBRINSTRUMENTGEN3_OUTFORMAT_NONE      = 0,
    /** Physical units to 4 decimal places. */
    RBRINSTRUMENTGEN3_OUTFORMAT_CALTEXT01 = 1 << 0,
    /** Physical units to 4 decimal places with units. */
    RBRINSTRUMENTGEN3_OUTFORMAT_CALTEXT02 = 1 << 1,
    /**
     * Physical units with sufficient significant digits to ensure no
     * resolution loss.
     */
    RBRINSTRUMENTGEN3_OUTFORMAT_CALTEXT03 = 1 << 2,
    /** Physical units expressed as “engineering-notation” floating point. */
    RBRINSTRUMENTGEN3_OUTFORMAT_CALTEXT04 = 1 << 3,
    /** Physical units to 4 decimal places. 
     *  The output starts with the keyword "RBR" followed by the serial number. 
     *  This format is available for LOGGER3 with fw 1.109 or later*/
    RBRINSTRUMENTGEN3_OUTFORMAT_CALTEXT07 = 1 << 4,
    /** Corresponds to the largest output format enum value. */
    RBRINSTRUMENTGEN3_OUTFORMAT_MAX       = RBRINSTRUMENTGEN3_OUTFORMAT_CALTEXT07
} RBRInstrumentGen3OutputFormat;

/**
 * \brief Get a human-readable string name for an output format.
 *
 * \param [in] format the output format
 * \return a string name for the output format
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen3OutputFormat_name(RBRInstrumentGen3OutputFormat format);

/**
 * \brief Report a list of available output formats.
 *
 * \a outputFormats will be treated as a bit field representation of available
 * output formats as defined by RBRInstrumentGen3OutputFormat. For details, consult
 * [Working with Bit Fields](bitfields.md).
 *
 * \param [in] instrument the instrument connection
 * \param [out] outputFormats available output formats
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 */
RBRGen3Error RBRInstrumentGen3_getAvailableOutputFormats(
    RBRGen3 *instrument,
    RBRInstrumentGen3OutputFormat *outputFormats);

/**
 * \brief Get the current output format.
 *
 * \param [in] instrument the instrument connection
 * \param [out] outputFormat the current output format
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 */
RBRGen3Error RBRInstrumentGen3_getOutputFormat(
    RBRGen3 *instrument,
    RBRInstrumentGen3OutputFormat *outputFormat);

/**
 * \brief Set the current output format.
 *
 * \param [in] instrument the instrument connection
 * \param [in] outputFormat the current output format
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when an unavailable output format is
 *                                       selected
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 */
RBRGen3Error RBRInstrumentGen3_setOutputFormat(
    RBRGen3 *instrument,
    RBRInstrumentGen3OutputFormat outputFormat);

/**
 * \brief Get the USB streaming state.
 *
 * \param [in] instrument the instrument connection
 * \param [out] enabled whether USB streaming is enabled
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when USB streaming is unavailable
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/streamusb
 */
RBRGen3Error RBRInstrumentGen3_getUSBStreamingState(
    RBRGen3 *instrument,
    bool *enabled);

/**
 * \brief Set the USB streaming state.
 *
 * \param [in] instrument the instrument connection
 * \param [in] enabled whether USB streaming is enabled
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when USB streaming is unavailable
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/streamusb
 */
RBRGen3Error RBRInstrumentGen3_setUSBStreamingState(
    RBRGen3 *instrument,
    bool enabled);

/**
 * \brief Get the serial streaming state.
 *
 * \param [in] instrument the instrument connection
 * \param [out] enabled whether serial streaming is enabled
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when serial streaming is unavailable
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/streamserial
 */
RBRGen3Error RBRInstrumentGen3_getSerialStreamingState(
    RBRGen3 *instrument,
    bool *enabled);

/**
 * \brief Set the serial streaming state.
 *
 * \param [in] instrument the instrument connection
 * \param [in] enabled whether serial streaming is enabled
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when serial streaming is unavailable
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/streamserial
 */
RBRGen3Error RBRInstrumentGen3_setSerialStreamingState(
    RBRGen3 *instrument,
    bool enabled);

/**
 * \brief Possible levels of the auxiliary output signal during the setup time,
 * data transmission, and hold time.
 *
 * \see RBRInstrumentGen3AuxOutput
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/streamserial
 */
typedef enum RBRInstrumentGen3AuxOutputActiveLevel
{
    /* Signal actively driven high. */
    RBRINSTRUMENTGEN3_ACTIVE_HIGH,
    /* Signal actively driven low. */
    RBRINSTRUMENTGEN3_ACTIVE_LOW,
    /** The number of active output levels. */
    RBRINSTRUMENTGEN3_ACTIVE_COUNT,
    /** An unknown or unrecognized active output level. */
    RBRINSTRUMENTGEN3_UNKNOWN_ACTIVE
} RBRInstrumentGen3AuxOutputActiveLevel;

/**
 * \brief Get a human-readable string name for a signal level of an active
 * auxiliary output.
 *
 * \param [in] level the signal level
 * \return a string name for the signal level
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen3AuxOutputActiveLevel_name(
    RBRInstrumentGen3AuxOutputActiveLevel level);

/**
 * \brief Possible levels of the auxiliary output signal while the instrument
 * is asleep.
 *
 * \see RBRInstrumentGen3AuxOutput
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/streamserial
 */
typedef enum RBRInstrumentGen3AuxOutputSleepLevel
{
    /* Passive, high-impedance signal. */
    RBRINSTRUMENTGEN3_SLEEP_TRISTATE,
    /* Signal actively driven high. */
    RBRINSTRUMENTGEN3_SLEEP_HIGH,
    /* Signal actively driven low. */
    RBRINSTRUMENTGEN3_SLEEP_LOW,
    /** The number of sleep output levels. */
    RBRINSTRUMENTGEN3_SLEEP_COUNT,
    /** An unknown or unrecognized sleep output level. */
    RBRINSTRUMENTGEN3_UNKNOWN_SLEEP
} RBRInstrumentGen3AuxOutputSleepLevel;

/**
 * \brief Get a human-readable string name for a signal level of a sleeping
 * auxiliary output.
 *
 * \param [in] level the signal level
 * \return a string name for the signal level
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen3AuxOutputSleepLevel_name(
    RBRInstrumentGen3AuxOutputSleepLevel level);

/**
 * \brief Instrument `streamserial` command parameters relating to the
 * auxiliary output signal functionality.
 *
 * \see RBRInstrumentGen3_getAuxOutput()
 * \see RBRInstrumentGen3_setAuxOutput()
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/streamserial
 */
typedef struct RBRInstrumentGen3AuxOutput
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
    RBRInstrumentGen3AuxOutputActiveLevel active;
    /**
     * \brief The level of the auxiliary output signal seen by the external
     * device while the instrument is asleep.
     */
    RBRInstrumentGen3AuxOutputSleepLevel sleep;
} RBRInstrumentGen3AuxOutput;

/**
 * \brief Get the instrument auxiliary output signal parameters.
 *
 * RBRInstrumentGen3AuxOutput.aux must be set to the index of the auxiliary output
 * for which signal parameters are to be retrieved. Currently, it can only ever
 * be set to `1` (AUX1). For example:
 *
 * ~~~{.c}
 * RBRInstrumentGen3AuxOutput auxOutput;
 * auxOutput.aux = 1;
 * RBRInstrumentGen3_getAuxOutput(instrument, &auxOutput);
 * ~~~
 *
 * \param [in] instrument the instrument connection
 * \param [in,out] auxOutput the auxiliary output signal parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the auxiliary output signal is
 *                                       unavailable
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when the auxiliary output
 *                                                signal index is not `1`
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/streamserial
 */
RBRGen3Error RBRInstrumentGen3_getAuxOutput(
    RBRGen3 *instrument,
    RBRInstrumentGen3AuxOutput *auxOutput);

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
 * \return #RBRGEN3_SUCCESS when the settings are successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/streamserial
 */
RBRGen3Error RBRInstrumentGen3_setAuxOutput(
    RBRGen3 *instrument,
    const RBRInstrumentGen3AuxOutput *auxOutput);

/**
 * \brief A flag set on a sample reading.
 */
typedef enum RBRInstrumentGen3ReadingFlag
{
    /** No flag. */
    RBRINSTRUMENTGEN3_READING_FLAG_NONE,
    /** The channel is uncalibrated. */
    RBRGEN3_READING_FLAG_UNCALIBRATED,
    /** The reading is an error. */
    RBRGEN3_READING_FLAG_ERROR,
    /** The number of reading flags. */
    RBRINSTRUMENTGEN3_READING_FLAG_COUNT,
    /** An unknown or unrecognized reading flag. */
    RBRINSTRUMENTGEN3_UNKNOWN_READING_FLAG
} RBRInstrumentGen3ReadingFlag;

/**
 * \brief Get a human-readable string name for a reading flag.
 *
 * \param [in] flag the reading flag
 * \return a string name for the reading flag
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen3ReadingFlag_name(RBRInstrumentGen3ReadingFlag flag);

/**
 * \brief Get the error flag from a reading.
 *
 * If the reading is not a NaN, returns the error flag encoded within the NaN.
 * Otherwise, returns #RBRINSTRUMENTGEN3_READING_FLAG_NONE.
 *
 * \param reading the reading
 * \return the error flag of the reading, if present
 * \see RBRInstrumentGen3Reading_getError() to get the error value, if present
 * \see RBRInstrumentGen3Reading_setError() to create a reading with an error set
 */
RBRInstrumentGen3ReadingFlag RBRInstrumentGen3Reading_getFlag(double reading);

/**
 * \brief Get the error value from a reading.
 *
 * If the reading is not a NaN, returns the error value encoded within the NaN.
 * Otherwise, returns 0.
 *
 * \param reading the reading
 * \return the error value of the reading, if present
 * \see RBRInstrumentGen3Reading_getFlag() to get the error flag, if present
 * \see RBRInstrumentGen3Reading_setError() to create a reading with an error set
 */
uint8_t RBRInstrumentGen3Reading_getError(double reading);

/**
 * \brief Synthesize a reading with an error set.
 *
 * \param flag the error flag
 * \param value the error value
 * \return the error reading
 * \see RBRInstrumentGen3Reading_getFlag() to get the error flag, if present
 * \see RBRInstrumentGen3Reading_getError() to get the error value, if present
 */
double RBRInstrumentGen3Reading_setError(RBRInstrumentGen3ReadingFlag flag,
                                     uint8_t value);

/**
 * \brief An instrument sample.
 */
typedef struct RBRGen3Sample
{
    /** \brief The timestamp of the sample. */
    RBRGen3DateTime timestamp;
    /** \brief The number of populated sample readings. */
    int32_t channels;
    /**
     * \brief The sample readings.
     *
     * Only the first RBRGen3Sample.channels readings will be populated.
     * Other readings will be set to 0.
     *
     * Readings are represented as double-precision floating point. If they
     * need to encode an error, it's stored in the trailing bits of a NaN, and
     * a flag is set to indicate which sort of error.
     *
     * \see RBRInstrumentGen3Reading_getFlag() to get the error flag, if present
     * \see RBRInstrumentGen3Reading_getError() to get the error value, if present
     * \see RBRInstrumentGen3Reading_setError() to synthesize a error reading
     */
    double readings[RBRGEN3_CHANNEL_MAX];
} RBRGen3Sample;

/**
 * \brief Retrieve and parse data streamed from the instrument.
 *
 * This function waits for a streamed sample to arrive, parses it, then calls
 * the RBRGen3SampleCallback provided to the instrument via
 * RBRGen3Callbacks.sample.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRGEN3_SUCCESS when a streaming sample has been read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen3_fetchSample() for on-demand sample fetching
 */
RBRGen3Error RBRInstrumentGen3_readSample(RBRGen3 *instrument);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTGEN3STREAMING_H */
