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

#include "RBRInstrumentGen4.h"

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

typedef enum RBRInstrumentGen4ReadingError
{
    /** -NaN; General error condition; error from undefined mathematical operation */
    RBRINSTRUMENTGEN4_READING_ERROR_GENERAL,
    /** ADC error – end of conversion */
    RBRINSTRUMENTGEN4_READING_ERROR_ADC_END_OF_CONVERSION,
    /** ADC error – invalid value */
    RBRINSTRUMENTGEN4_READING_ERROR_ADC_INVALID_VALUE,
    /** Bus error – invalid address */
    RBRINSTRUMENTGEN4_READING_ERROR_BUS_INVALID_ADDRESS,
    /** Bus error – frame overflow */
    RBRINSTRUMENTGEN4_READING_ERROR_BUS_FRAME_OVERFLOW,
    /** Bus error – locked */
    RBRINSTRUMENTGEN4_READING_ERROR_BUS_LOCKED,
    /** Bus error – cannot transmit */
    RBRINSTRUMENTGEN4_READING_ERROR_BUS_CANNOT_TRANSMIT,
    /** Bus error – receive timed out */
    RBRINSTRUMENTGEN4_READING_ERROR_BUS_RECEIVE_TIMEOUT,
    /** Bus error – invalid frame */
    RBRINSTRUMENTGEN4_READING_ERROR_BUS_INVALID_FRAME,
    /** Sample error – no sample started */
    RBRINSTRUMENTGEN4_READING_ERROR_SAMPLE_NONE_STARTED,
    /** Sample error – sample in progress */
    RBRINSTRUMENTGEN4_READING_ERROR_SAMPLE_IN_PROGRESS,
    /** Sample error – sample failed */
    RBRINSTRUMENTGEN4_READING_ERROR_SAMPLE_FAILED,
    /** Sample error – averaging failed */
    RBRINSTRUMENTGEN4_READING_ERROR_SAMPLE_AVERAGING_FAILED,
    /** Bus error – packet truncated */
    RBRINSTRUMENTGEN4_READING_ERROR_BUS_PACKET_TRUNCATED,
    /** Data error – unable to compute */
    RBRINSTRUMENTGEN4_READING_ERROR_DATA_UNABLE_TO_COMPUTE,
    /** Safety – high power consumption */
    RBRINSTRUMENTGEN4_READING_ERROR_SAFETY_HIGH_POWER_CONSUMPTION,
    /** Data error – out of range */
    RBRINSTRUMENTGEN4_READING_ERROR_DATA_OUT_OF_RANGE,
    /** Data error – under range */
    RBRINSTRUMENTGEN4_READING_ERROR_DATA_UNDER_RANGE,
    /** Data error – over range */
    RBRINSTRUMENTGEN4_READING_ERROR_DATA_OVER_RANGE,
    /** Sensor error – communications timeout */
    RBRINSTRUMENTGEN4_READING_ERROR_SENSOR_COMMUNICATIONS_TIMEOUT,
    /** Sensor error – cannot parse response */
    RBRINSTRUMENTGEN4_READING_ERROR_SENSOR_CANNOT_PARSE_RESPONSE,
    /** Data error – not calibrated / invalid calibration */
    RBRINSTRUMENTGEN4_READING_ERROR_DATA_NOT_CALIBRATED,
    /** Data error – malformed floating point number */
    RBRINSTRUMENTGEN4_READING_ERROR_DATA_MALFORMED_NUMBER,
    /** Data error – no sample logged */
    RBRINSTRUMENTGEN4_READING_ERROR_DATA_NO_SAMPLE_LOGGED,
    /** The number of reading flags. */
    RBRINSTRUMENTGEN4_READING_ERROR_COUNT,
    /** An unknown or unrecognized reading flag. */
    RBRINSTRUMENTGEN4_UNKNOWN_READING_ERROR
} RBRInstrumentGen4ReadingError;

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
RBRInstrumentGen4ReadingError RBRInstrumentGen4Reading_getError(double reading);

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
    /**
     * \brief The schedule label reported with the sample.
     *
     * An empty string when the output format omits the schedule label.
     */
    char scheduleLabel[RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];
    /** \brief The number of populated sample readings. */
    int32_t channelCount;
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
