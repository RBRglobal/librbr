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
 * \brief Check whether a reading is an error rather than a value.
 *
 * Error readings are NaNs carrying an error code; a plain NaN reported by
 * the instrument is not an error reading.
 *
 * \param [in] reading the reading
 * \return whether the reading is an error
 * \see RBRInstrumentGen4Reading_getError() to get the error code
 * \see RBRInstrumentGen4Reading_setError() to create an error reading
 */
bool RBRInstrumentGen4Reading_isError(double reading);

/**
 * \brief Get the error code from an error reading.
 *
 * Meaningful only when RBRInstrumentGen4Reading_isError() is true; returns 0
 * otherwise.
 *
 * \param [in] reading the reading
 * \return the error code of the reading
 * \see RBRInstrumentGen4Reading_isError() to check for an error first
 * \see RBRInstrumentGen4Reading_setError() to create an error reading
 */
RBRInstrumentGen4ReadingError RBRInstrumentGen4Reading_getError(double reading);

/**
 * \brief Synthesize an error reading.
 *
 * \param [in] error the error code
 * \return the error reading
 * \see RBRInstrumentGen4Reading_isError() to check for an error
 * \see RBRInstrumentGen4Reading_getError() to get the error code
 */
double RBRInstrumentGen4Reading_setError(RBRInstrumentGen4ReadingError error);

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
     * need to encode an error, it's stored in the trailing bits of a NaN.
     *
     * \see RBRInstrumentGen4Reading_isError() to check for an error
     * \see RBRInstrumentGen4Reading_getError() to get the error code
     * \see RBRInstrumentGen4Reading_setError() to synthesize an error reading
     */
    double readings[RBRINSTRUMENTGEN4_CHANNEL_MAX];
} RBRInstrumentGen4Sample;

/**
 * \brief Retrieve and parse data streamed from the instrument.
 *
 * This function waits for a streamed sample to arrive, parses it, then calls
 * the RBRInstrumentGen4SampleCallback provided to the instrument via
 * RBRInstrumentGen4Callbacks.sample, delivering the sample into
 * RBRInstrumentGen4Callbacks.sampleBuffer.
 *
 * This requires RBRInstrumentGen4Callbacks.sample and
 * RBRInstrumentGen4Callbacks.sampleBuffer to be populated.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENTGEN4_SUCCESS when a streaming sample has been read
 * \return #RBRINSTRUMENTGEN4_MISSING_CALLBACK when the connection was opened
 *         without a sample callback
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 */
RBRInstrumentGen4Error RBRInstrumentGen4_readSample(RBRInstrumentGen4 *instrument);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTSTREAMING_H */
