/**
 * \file RBRInstrumentGen4Streaming.h
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

#ifndef LIBRBR_RBRINSTRUMENTGEN4STREAMING_H
#define LIBRBR_RBRINSTRUMENTGEN4STREAMING_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Properties of `outputformat` command that can be set.
 * each paramter is treated as a bit field.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 */
typedef enum RBRInstrumentGen4Outputformat
{
    /** Determins whether or not the output begins with a peramble 
     * consisting of the string RBR, then the logger's 6-digit serial number.
     * The default state is off.*/
    RBRINSTRUMENTGEN4_OUTPUTFORMAT_SERIAL = 1 << 0,
    /** Determines whether or not the schedule label appears timestamp.
     * The default state is off.
     */
    RBRINSTRUMENTGEN4_OUTPUTFORMAT_SCHEDULELABEL = 1 << 1,
    /** Determines whether or not a cyclic redudant check (CRC) is included at
     * the end of the line immediately before the terminating "\r""\n". The dfault
     * state is off.
     */
    RBRINSTRUMENTGEN4_OUTPUTFORMAT_CRC = 1 << 2
}RBRInstrumentGen4Outputformat;

/**
 * \brief Get the current output format.
 *
 * \param [in] instrument the instrument connection
 * \param [out] outputformat the current output format
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getOutputformat(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Outputformat *outputformat);

/**
 * \brief Set the current output format.
 *
 * \param [in] instrument the instrument connection
 * \param [in] outputformat the desired output format
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 * \see hhttps://docs.rbr-global.com/L3commandreference/commands/memory-and-data-retrieval/meminfo
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setOutputformat(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Outputformat outputformat);

/*************************************************************************************************/
/**
 * \brief A flag set on a sample reading.
 */
typedef enum RBRInstrumentGen4ReadingFlag
{
    /** No flag. */
    RBRINSTRUMENTGEN4_READING_FLAG_NONE,
    /** The channel is uncalibrated. */
    RBRINSTRUMENTGEN4_READING_FLAG_UNCALIBRATED,
    /** The reading is an error. */
    RBRINSTRUMENTGEN4_READING_FLAG_ERROR,
    /** The number of reading flags. */
    RBRINSTRUMENTGEN4_READING_FLAG_COUNT,
    /** An unknown or unrecognized reading flag. */
    RBRINSTRUMENTGEN4_UNKNOWN_READING_FLAG
} RBRInstrumentGen4ReadingFlag;

/**
 * \brief Get a human-readable string name for a reading flag.
 *
 * \param [in] flag the reading flag
 * \return a string name for the reading flag
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4ReadingFlag_name(RBRInstrumentGen4ReadingFlag flag);

/**
 * \brief Get the error flag from a reading.
 *
 * If the reading is not a NaN, returns the error flag encoded within the NaN.
 * Otherwise, returns #RBRINSTRUMENTGEN4_READING_FLAG_NONE.
 *
 * \param [in] reading the reading
 * \return the error flag of the reading, if present
 * \see RBRInstrumentGen4Reading_getError() to get the error value, if present
 * \see RBRInstrumentGen4Reading_setError() to create a reading with an error set
 */
RBRInstrumentGen4ReadingFlag RBRInstrumentGen4Reading_getFlag(double reading);

/**
 * \brief Get the error value from a reading.
 *
 * If the reading is not a NaN, returns the error value encoded within the NaN.
 * Otherwise, returns 0.
 *
 * \param [in] reading the reading
 * \return the error value of the reading, if present
 * \see RBRInstrumentGen4Reading_getFlag() to get the error flag, if present
 * \see RBRInstrumentGen4Reading_setError() to create a reading with an error set
 */
uint8_t RBRInstrumentGen4Reading_getError(double reading);

/**
 * \brief Synthesize a reading with an error set.
 *
 * \param [in] flag the error flag
 * \param [in] value the error value
 * \return the error reading
 * \see RBRInstrumentGen4Reading_getFlag() to get the error flag, if present
 * \see RBRInstrumentGen4Reading_getError() to get the error value, if present
 */
double RBRInstrumentGen4Reading_setError(RBRInstrumentGen4ReadingFlag flag,
                                     uint8_t value);

/**
 * \brief An instrument sample.
 */
typedef struct RBRInstrumentGen4Sample
{
    /** \brief The timestamp of the sample. */
    RBRInstrumentGen4DateTime timestamp;
    /** \brief The number of populated sample readings. */
    int32_t channels;
    /**
     * \brief The sample readings.
     *
     * Only the first RBRInstrumentGen4Sample.channels readings will be populated.
     * Other readings will be set to 0.
     *
     * Readings are represented as double-precision floating point. If they
     * need to encode an error, it's stored in the trailing bits of a NaN, and
     * a flag is set to indicate which sort of error.
     *
     * \see RBRInstrumentGen4Reading_getFlag() to get the error flag, if present
     * \see RBRInstrumentGen4Reading_getError() to get the error value, if present
     * \see RBRInstrumentGen4Reading_setError() to synthesize a error reading
     */
    double readings[RBRINSTRUMENTGEN4_CHANNEL_MAX];
} RBRInstrumentGen4Sample;

/**
 * \brief Retrieve and parse data streamed from the instrument.
 *
 * This function waits for a streamed sample to arrive, parses it, then calls
 * the RBRInstrumentGen4SampleCallback provided to the instrument via
 * RBRInstrumentGen4Callbacks.sample.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENTGEN4_SUCCESS when a streaming sample has been read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_fetchSample() for on-demand sample fetching
 */
RBRInstrumentGen4Error RBRInstrumentGen4_readSample(RBRInstrumentGen4 *instrument);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTSTREAMING_H */
