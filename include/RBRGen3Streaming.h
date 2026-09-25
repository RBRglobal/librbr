/**
 * \file RBRGen3Streaming.h
 *
 * \brief Instrument commands and structures pertaining to real-time data
 * acquisition.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRGEN3STREAMING_H
#define LIBRBR_RBRGEN3STREAMING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RBRGen3.h"

/**
 * \brief The maximum number of characters in an output format name (e.g.,
 * “caltext01”).
 */
#define RBRGEN3_OUTPUT_FORMAT_NAME_MAX 15

/**
 * \brief Response to the `outputformat channelslist` command.
 *
 * \see RBRGen3_getChannelsList()
 */
typedef struct RBRGen3ChannelsList {
    /** \brief The number of active channels. */
    int32_t count;
    /** \brief The name and unit of each active channel. */
    struct {
        /** \brief The name of the channel as a null-terminated C string. */
        char name[RBRGEN3_CHANNEL_NAME_MAX + 1];
        /** \brief The unit of the channel as a null-terminated C string. */
        char unit[RBRGEN3_CHANNEL_UNIT_MAX + 1];
    } channels[RBRGEN3_CHANNEL_MAX];
} RBRGen3ChannelsList;

/**
 * \brief Report a list of names and units for active channels, in order.
 *
 * Helpful for identifying the channel corresponding to each value in the
 * transmitted data.
 *
 * RBRGen3ChannelsList.channels will be populated in the order reported
 * by the instrument. Unpopulated entries will have zero-length name and unit
 * members.
 *
 * \nol2 Use RBRGen3_getChannels() and RBRGen3_getChannel().
 *
 * \param [in] conn the instrument connection
 * \param [out] channelsList the channels list
 * \return #RBRGEN3_UNSUPPORTED for Logger2 instruments
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the command is unavailable, or another
 *                                 hardware error occurs
 * \see RBRGen3_getLabelsList()
 */
RBRGen3Error RBRGen3_getChannelsList(RBRGen3 *conn, RBRGen3ChannelsList *channelsList);

/**
 * \brief Response to the `outputformat labelslist` command.
 *
 * \see RBRGen3_getLabelsList()
 */
typedef struct RBRGen3LabelsList {
    /** \brief The number of active channels. */
    int32_t count;
    /**
     * \brief The label for each active channel as null-terminated C strings.
     */
    char labels[RBRGEN3_CHANNEL_MAX][RBRGEN3_CHANNEL_LABEL_MAX + 1];
} RBRGen3LabelsList;

/**
 * \brief Report a list of labels for active channels, in order.
 *
 * Helpful for identifying the channel corresponding to each value in the
 * transmitted data.
 *
 * RBRGen3LabelsList.channels will be populated in the order reported
 * by the instrument. Unpopulated entries will be zero-length.
 *
 * \nol2 Use RBRGen3_getChannels() and RBRGen3_getChannel().
 *
 * \param [in] conn the instrument connection
 * \param [out] labelsList the channel labels list
 * \return #RBRGEN3_UNSUPPORTED for Logger2 instruments
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the command is unavailable, or another
 *                                 hardware error occurs
 * \see RBRGen3_getChannelsList()
 */
RBRGen3Error RBRGen3_getLabelsList(RBRGen3 *conn, RBRGen3LabelsList *labelsList);

/**
 * \brief Instrument output formats.
 *
 * \see RBRGen3_getAvailableOutputFormats()
 * \see RBRGen3_getOutputFormat()
 * \see RBRGen3_setOutputFormat()
 */
typedef enum RBRGen3OutputFormat {
    /** No format. */
    RBRGEN3_OUTFORMAT_NONE = 0,
    /** Physical units to 4 decimal places. */
    RBRGEN3_OUTFORMAT_CALTEXT01 = 1 << 0,
    /** Physical units to 4 decimal places with units. */
    RBRGEN3_OUTFORMAT_CALTEXT02 = 1 << 1,
    /**
     * Physical units with sufficient significant digits to ensure no
     * resolution loss.
     */
    RBRGEN3_OUTFORMAT_CALTEXT03 = 1 << 2,
    /** Physical units expressed as “engineering-notation” floating point. */
    RBRGEN3_OUTFORMAT_CALTEXT04 = 1 << 3,
    /** Physical units to 4 decimal places.
     *  The output starts with the keyword "RBR" followed by the serial number.
     *  This format is available for LOGGER3 with fw 1.109 or later*/
    RBRGEN3_OUTFORMAT_CALTEXT07 = 1 << 4,
    /** Corresponds to the largest output format enum value. */
    RBRGEN3_OUTFORMAT_MAX = RBRGEN3_OUTFORMAT_CALTEXT07
} RBRGen3OutputFormat;

/**
 * \brief Get a human-readable string name for an output format.
 *
 * \param [in] format the output format
 * \return a string name for the output format
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRGen3OutputFormat_name(RBRGen3OutputFormat format);

/**
 * \brief Report a list of available output formats.
 *
 * \a outputFormats will be treated as a bit field representation of available
 * output formats as defined by RBRGen3OutputFormat. For details, consult
 * the Working with Bit Fields page of the documentation.
 *
 * \param [in] conn the instrument connection
 * \param [out] outputFormats available output formats
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 */
RBRGen3Error RBRGen3_getAvailableOutputFormats(RBRGen3 *conn, RBRGen3OutputFormat *outputFormats);

/**
 * \brief Get the current output format.
 *
 * \param [in] conn the instrument connection
 * \param [out] outputFormat the current output format
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRGen3_setOutputFormat()
 */
RBRGen3Error RBRGen3_getOutputFormat(RBRGen3 *conn, RBRGen3OutputFormat *outputFormat);

/**
 * \brief Set the current output format.
 *
 * \param [in] conn the instrument connection
 * \param [in] outputFormat the current output format
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when an unavailable output format is
 *                                 selected, or another hardware error occurs
 * \see RBRGen3_getOutputFormat()
 */
RBRGen3Error RBRGen3_setOutputFormat(RBRGen3 *conn, RBRGen3OutputFormat outputFormat);

/**
 * \brief Get the USB streaming state.
 *
 * \param [in] conn the instrument connection
 * \param [out] enabled whether USB streaming is enabled
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when USB streaming is unavailable, or
 *                                 another hardware error occurs
 * \see RBRGen3_setUSBStreamingState()
 */
RBRGen3Error RBRGen3_getUSBStreamingState(RBRGen3 *conn, bool *enabled);

/**
 * \brief Set the USB streaming state.
 *
 * \param [in] conn the instrument connection
 * \param [in] enabled whether USB streaming is enabled
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when USB streaming is unavailable, or
 *                                 another hardware error occurs
 * \see RBRGen3_getUSBStreamingState()
 */
RBRGen3Error RBRGen3_setUSBStreamingState(RBRGen3 *conn, bool enabled);

/**
 * \brief Get the serial streaming state.
 *
 * \param [in] conn the instrument connection
 * \param [out] enabled whether serial streaming is enabled
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when serial streaming is unavailable, or
 *                                 another hardware error occurs
 * \see RBRGen3_setSerialStreamingState()
 */
RBRGen3Error RBRGen3_getSerialStreamingState(RBRGen3 *conn, bool *enabled);

/**
 * \brief Set the serial streaming state.
 *
 * \param [in] conn the instrument connection
 * \param [in] enabled whether serial streaming is enabled
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when serial streaming is unavailable, or
 *                                 another hardware error occurs
 * \see RBRGen3_getSerialStreamingState()
 */
RBRGen3Error RBRGen3_setSerialStreamingState(RBRGen3 *conn, bool enabled);

/**
 * \brief Possible levels of the auxiliary output signal during the setup time,
 * data transmission, and hold time.
 *
 * \see RBRGen3AuxOutput
 */
typedef enum RBRGen3AuxOutputActiveLevel {
    /* Signal actively driven high. */
    RBRGEN3_ACTIVE_HIGH,
    /* Signal actively driven low. */
    RBRGEN3_ACTIVE_LOW,
    /** The number of active output levels. */
    RBRGEN3_ACTIVE_COUNT,
    /** An unknown or unrecognized active output level. */
    RBRGEN3_UNKNOWN_ACTIVE
} RBRGen3AuxOutputActiveLevel;

/**
 * \brief Get a human-readable string name for a signal level of an active
 * auxiliary output.
 *
 * \param [in] level the signal level
 * \return a string name for the signal level
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRGen3AuxOutputActiveLevel_name(RBRGen3AuxOutputActiveLevel level);

/**
 * \brief Possible levels of the auxiliary output signal while the instrument
 * is asleep.
 *
 * \see RBRGen3AuxOutput
 */
typedef enum RBRGen3AuxOutputSleepLevel {
    /* Passive, high-impedance signal. */
    RBRGEN3_SLEEP_TRISTATE,
    /* Signal actively driven high. */
    RBRGEN3_SLEEP_HIGH,
    /* Signal actively driven low. */
    RBRGEN3_SLEEP_LOW,
    /** The number of sleep output levels. */
    RBRGEN3_SLEEP_COUNT,
    /** An unknown or unrecognized sleep output level. */
    RBRGEN3_UNKNOWN_SLEEP
} RBRGen3AuxOutputSleepLevel;

/**
 * \brief Get a human-readable string name for a signal level of a sleeping
 * auxiliary output.
 *
 * \param [in] level the signal level
 * \return a string name for the signal level
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRGen3AuxOutputSleepLevel_name(RBRGen3AuxOutputSleepLevel level);

/**
 * \brief Instrument `streamserial` command parameters relating to the
 * auxiliary output signal functionality.
 *
 * \see RBRGen3_getAuxOutput()
 * \see RBRGen3_setAuxOutput()
 */
typedef struct RBRGen3AuxOutput {
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
    RBRGen3AuxOutputActiveLevel active;
    /**
     * \brief The level of the auxiliary output signal seen by the external
     * device while the instrument is asleep.
     */
    RBRGen3AuxOutputSleepLevel sleep;
} RBRGen3AuxOutput;

/**
 * \brief Get the instrument auxiliary output signal parameters.
 *
 * RBRGen3AuxOutput.aux must be set to the index of the auxiliary output
 * for which signal parameters are to be retrieved. Currently, it can only ever
 * be set to `1` (AUX1). For example:
 *
 * ~~~{.c}
 * RBRGen3AuxOutput auxOutput;
 * auxOutput.aux = 1;
 * RBRGen3_getAuxOutput(instrument, &auxOutput);
 * ~~~
 *
 * \param [in] conn the instrument connection
 * \param [in,out] auxOutput the auxiliary output signal parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the auxiliary output signal is
 *                                 unavailable, or another hardware error
 *                                 occurs
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when the auxiliary output
 *                                                signal index is not `1`
 * \see RBRGen3_setAuxOutput()
 */
RBRGen3Error RBRGen3_getAuxOutput(RBRGen3 *conn, RBRGen3AuxOutput *auxOutput);

/**
 * \brief Set the instrument auxiliary output signal parameters.
 *
 * Hardware errors may occur if:
 *
 * - the auxiliary output signal is not available for the instrument
 * - you set an out-of-bounds parameter the library fails to detect
 *
 * \param [in] conn the instrument connection
 * \param [out] auxOutput the auxiliary output signal parameters
 * \return #RBRGEN3_SUCCESS when the settings are successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the settings cannot be changed, or
 *                                 another hardware error occurs
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see RBRGen3_getAuxOutput()
 */
RBRGen3Error RBRGen3_setAuxOutput(RBRGen3 *conn, const RBRGen3AuxOutput *auxOutput);

/**
 * \brief A flag set on a sample reading.
 */
typedef enum RBRGen3ReadingFlag {
    /** No flag. */
    RBRGEN3_READING_FLAG_NONE,
    /** The channel is uncalibrated. */
    RBRGEN3_READING_FLAG_UNCALIBRATED,
    /** The reading is an error. */
    RBRGEN3_READING_FLAG_ERROR,
    /** The number of reading flags. */
    RBRGEN3_READING_FLAG_COUNT,
    /** An unknown or unrecognized reading flag. */
    RBRGEN3_UNKNOWN_READING_FLAG
} RBRGen3ReadingFlag;

/**
 * \brief Get a human-readable string name for a reading flag.
 *
 * \param [in] flag the reading flag
 * \return a string name for the reading flag
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRGen3ReadingFlag_name(RBRGen3ReadingFlag flag);

/**
 * \brief Get the error flag from a reading.
 *
 * If the reading is not a NaN, returns the error flag encoded within the NaN.
 * Otherwise, returns #RBRGEN3_READING_FLAG_NONE.
 *
 * \param reading the reading
 * \return the error flag of the reading, if present
 * \see RBRGen3Reading_getError() to get the error value, if present
 * \see RBRGen3Reading_setError() to create a reading with an error set
 */
RBRGen3ReadingFlag RBRGen3Reading_getFlag(double reading);

/**
 * \brief Get the error value from a reading.
 *
 * If the reading is not a NaN, returns the error value encoded within the NaN.
 * Otherwise, returns 0.
 *
 * \param reading the reading
 * \return the error value of the reading, if present
 * \see RBRGen3Reading_getFlag() to get the error flag, if present
 * \see RBRGen3Reading_setError() to create a reading with an error set
 */
uint8_t RBRGen3Reading_getError(double reading);

/**
 * \brief Synthesize a reading with an error set.
 *
 * \param flag the error flag
 * \param value the error value
 * \return the error reading
 * \see RBRGen3Reading_getFlag() to get the error flag, if present
 * \see RBRGen3Reading_getError() to get the error value, if present
 */
double RBRGen3Reading_setError(RBRGen3ReadingFlag flag, uint8_t value);

/**
 * \brief An instrument sample.
 */
typedef struct RBRGen3Sample {
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
     * \see RBRGen3Reading_getFlag() to get the error flag, if present
     * \see RBRGen3Reading_getError() to get the error value, if present
     * \see RBRGen3Reading_setError() to synthesize a error reading
     */
    double readings[RBRGEN3_CHANNEL_MAX];
} RBRGen3Sample;

/**
 * \brief Retrieve and parse data streamed from the instrument.
 *
 * This function waits for a streamed sample to arrive, parses it, then calls
 * the RBRGen3SampleCallback provided to the instrument via
 * RBRGen3Environment.sample.
 *
 * \param [in] conn the instrument connection
 * \return #RBRGEN3_SUCCESS when a streaming sample has been read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRGen3_fetchSample() for on-demand sample fetching
 */
RBRGen3Error RBRGen3_readSample(RBRGen3 *conn);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN3STREAMING_H */
