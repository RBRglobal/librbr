/**
 * \file RBRGen4Realtime.h
 *
 * \brief Instrument commands and structures pertaining to streamed and
 * on-demand data acquisition.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRGEN4REALTIME_H
#define LIBRBR_RBRGEN4REALTIME_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Error codes carried by an error reading.
 *
 * \see RBRGen4Reading_getError()
 * \see RBRGen4Reading_setError()
 */
typedef enum RBRGen4ReadingError {
    /** -NaN; General error condition; error from undefined mathematical operation */
    RBRGEN4_READING_ERROR_GENERAL,
    /** ADC error – end of conversion */
    RBRGEN4_READING_ERROR_ADC_END_OF_CONVERSION,
    /** ADC error – invalid value */
    RBRGEN4_READING_ERROR_ADC_INVALID_VALUE,
    /** Bus error – invalid address */
    RBRGEN4_READING_ERROR_BUS_INVALID_ADDRESS,
    /** Bus error – frame overflow */
    RBRGEN4_READING_ERROR_BUS_FRAME_OVERFLOW,
    /** Bus error – locked */
    RBRGEN4_READING_ERROR_BUS_LOCKED,
    /** Bus error – cannot transmit */
    RBRGEN4_READING_ERROR_BUS_CANNOT_TRANSMIT,
    /** Bus error – receive timed out */
    RBRGEN4_READING_ERROR_BUS_RECEIVE_TIMEOUT,
    /** Bus error – invalid frame */
    RBRGEN4_READING_ERROR_BUS_INVALID_FRAME,
    /** Sample error – no sample started */
    RBRGEN4_READING_ERROR_SAMPLE_NONE_STARTED,
    /** Sample error – sample in progress */
    RBRGEN4_READING_ERROR_SAMPLE_IN_PROGRESS,
    /** Sample error – sample failed */
    RBRGEN4_READING_ERROR_SAMPLE_FAILED,
    /** Sample error – averaging failed */
    RBRGEN4_READING_ERROR_SAMPLE_AVERAGING_FAILED,
    /** Bus error – packet truncated */
    RBRGEN4_READING_ERROR_BUS_PACKET_TRUNCATED,
    /** Data error – unable to compute */
    RBRGEN4_READING_ERROR_DATA_UNABLE_TO_COMPUTE,
    /** Safety – high power consumption */
    RBRGEN4_READING_ERROR_SAFETY_HIGH_POWER_CONSUMPTION,
    /** Data error – out of range */
    RBRGEN4_READING_ERROR_DATA_OUT_OF_RANGE,
    /** Data error – under range */
    RBRGEN4_READING_ERROR_DATA_UNDER_RANGE,
    /** Data error – over range */
    RBRGEN4_READING_ERROR_DATA_OVER_RANGE,
    /** Sensor error – communications timeout */
    RBRGEN4_READING_ERROR_SENSOR_COMMUNICATIONS_TIMEOUT,
    /** Sensor error – cannot parse response */
    RBRGEN4_READING_ERROR_SENSOR_CANNOT_PARSE_RESPONSE,
    /** Data error – not calibrated / invalid calibration */
    RBRGEN4_READING_ERROR_DATA_NOT_CALIBRATED,
    /** Data error – malformed floating point number */
    RBRGEN4_READING_ERROR_DATA_MALFORMED_NUMBER,
    /** Data error – no sample logged */
    RBRGEN4_READING_ERROR_DATA_NO_SAMPLE_LOGGED,
    /** The number of reading flags. */
    RBRGEN4_READING_ERROR_COUNT,
    /** An unknown or unrecognized reading flag. */
    RBRGEN4_UNKNOWN_READING_ERROR
} RBRGen4ReadingError;

/**
 * \brief Check whether a reading is an error rather than a value.
 *
 * Error readings are NaNs carrying an error code; a plain NaN reported by
 * the instrument is not an error reading.
 *
 * \param [in] reading the reading
 * \return whether the reading is an error
 * \see RBRGen4Reading_getError() to get the error code
 * \see RBRGen4Reading_setError() to create an error reading
 */
bool RBRGen4Reading_isError(double reading);

/**
 * \brief Get the error code from an error reading.
 *
 * Meaningful only when RBRGen4Reading_isError() is true; returns 0
 * otherwise.
 *
 * \param [in] reading the reading
 * \return the error code of the reading
 * \see RBRGen4Reading_isError() to check for an error first
 * \see RBRGen4Reading_setError() to create an error reading
 */
RBRGen4ReadingError RBRGen4Reading_getError(double reading);

/**
 * \brief Synthesize an error reading.
 *
 * \param [in] error the error code
 * \return the error reading
 * \see RBRGen4Reading_isError() to check for an error
 * \see RBRGen4Reading_getError() to get the error code
 */
double RBRGen4Reading_setError(RBRGen4ReadingError error);

/**
 * \brief An instrument sample.
 */
typedef struct RBRGen4Sample {
    /** \brief The timestamp of the sample. */
    RBRGen4DateTime timestamp;
    /**
     * \brief The schedule label reported with the sample.
     *
     * An empty string when the output format omits the schedule label.
     */
    char scheduleLabel[RBRGEN4_LABEL_NAME_MAX + 1];
    /** \brief The number of populated sample readings. */
    int32_t channelCount;
    /**
     * \brief The sample readings.
     *
     * Only the first RBRGen4Sample.channelCount readings will be populated.
     * Other readings will be set to 0.
     *
     * Readings are represented as double-precision floating point. If they
     * need to encode an error, it's stored in the trailing bits of a NaN.
     *
     * \see RBRGen4Reading_isError() to check for an error
     * \see RBRGen4Reading_getError() to get the error code
     * \see RBRGen4Reading_setError() to synthesize an error reading
     */
    double readings[RBRGEN4_CHANNEL_MAX];
} RBRGen4Sample;

/**
 * \brief Retrieve and parse data streamed from the instrument.
 *
 * This function waits for a streamed sample to arrive, parses it, then calls
 * the RBRGen4SampleCallback provided to the instrument via
 * RBRGen4Environment.sample, delivering the sample into
 * RBRGen4Environment.sampleBuffer.
 *
 * This requires RBRGen4Environment.sample and
 * RBRGen4Environment.sampleBuffer to be populated.
 *
 * \param [in] conn the instrument connection
 * \return #RBRGEN4_SUCCESS when a streaming sample has been read
 * \return #RBRGEN4_MISSING_CALLBACK when the connection was opened
 *         without a sample callback
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 */
RBRGen4Error RBRGen4_readSample(RBRGen4 *conn);

/**
 * \brief Requests an “on-demand” sample of every channel from the
 * instrument.
 * \note Issues the `poll` command.
 *
 * Unlike streaming data/RBRGen4_readSample(), polled data is
 * returned directly to the caller (independent of any
 * RBRGen4SampleCallback defined in
 * RBRGen4Environment.sample).
 *
 * With \a requireLabel set, only a sample labelled `polling` is returned;
 * any streamed samples read while waiting for it are passed to
 * RBRGen4Environment.sample instead. With \a requireLabel unset, the
 * first sample read is returned, which may be a streamed sample, not a
 * polled sample, if the instrument is streaming over this link.
 *
 * \param [in] conn the instrument connection
 * \param [in] requireLabel whether to require and wait for a sample
 *                          labelled `polling`
 * \param [out] sample the polled sample
 * \return #RBRGEN4_SUCCESS when a sample is successfully read
 * \return #RBRGEN4_UNSUPPORTED when \a requireLabel is set but
 *         instrument.outputformat.scheduleLabel is false
 * \return #RBRGEN4_TIMEOUT when a timeout occurs, or when no
 *         polled sample arrives within RBRGen4.pollTimeout
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_BUFFER_TOO_SMALL when the command does not fit the
 *         command buffer
 */
RBRGen4Error RBRGen4_poll(RBRGen4 *conn, bool requireLabel, RBRGen4Sample *sample);

/**
 * \brief Requests an “on-demand” sample of the given channels from the
 * instrument.
 * \note Issues the `poll channellist=<channel_list>` command.
 *
 * The labels of \a channelList are sent as the `channellist` parameter value.
 *
 * Unlike streaming data/RBRGen4_readSample(), polled data is
 * returned directly to the caller (independent of any
 * RBRGen4SampleCallback defined in
 * RBRGen4Environment.sample).
 *
 * With \a requireLabel set, only a sample labelled `polling` is returned;
 * any streamed samples read while waiting for it are passed to
 * RBRGen4Environment.sample instead. With \a requireLabel unset, the
 * first sample read is returned, which may be a streamed sample, not a
 * polled sample, if the instrument is streaming over this link.
 *
 * \param [in] conn the instrument connection
 * \param [in] requireLabel whether to require and wait for a sample
 *                          labelled `polling`
 * \param [in] channelList the channels to sample
 * \param [out] sample the polled sample
 * \return #RBRGEN4_SUCCESS when a sample is successfully read
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when \a channelList is `NULL`,
 *         empty, its count is out of range, or a channel label is empty
 * \return #RBRGEN4_BUFFER_TOO_SMALL when the list does not fit the
 *         command
 * \return #RBRGEN4_UNSUPPORTED when \a requireLabel is set but
 *         instrument.outputformat.scheduleLabel is false
 * \return #RBRGEN4_TIMEOUT when a timeout occurs, or when no
 *         polled sample arrives within RBRGen4.pollTimeout
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when an invalid channel is requested, or
 *                                      another hardware error occurs
 */
RBRGen4Error RBRGen4_pollChannels(RBRGen4 *conn, bool requireLabel,
                                  const RBRGen4LabelList *channelList, RBRGen4Sample *sample);

/**
 * \brief Requests an “on-demand” sample of the given groups of channels from
 * the instrument.
 * \note Issues the `poll grouplist=<group_list>` command.
 *
 * The labels of \a groupList are sent as the `grouplist` parameter value.
 *
 * Unlike streaming data/RBRGen4_readSample(), polled data is
 * returned directly to the caller (independent of any
 * RBRGen4SampleCallback defined in
 * RBRGen4Environment.sample).
 *
 * With \a requireLabel set, only a sample labelled `polling` is returned;
 * any streamed samples read while waiting for it are passed to
 * RBRGen4Environment.sample instead. With \a requireLabel unset, the
 * first sample read is returned, which may be a streamed sample, not a
 * polled sample, if the instrument is streaming over this link.
 *
 * \param [in] conn the instrument connection
 * \param [in] requireLabel whether to require and wait for a sample
 *                          labelled `polling`
 * \param [in] groupList the groups of channels to sample
 * \param [out] sample the polled sample
 * \return #RBRGEN4_SUCCESS when a sample is successfully read
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when \a groupList is `NULL`,
 *         empty, its count is out of range, or a group label is empty
 * \return #RBRGEN4_BUFFER_TOO_SMALL when the list does not fit the
 *         command
 * \return #RBRGEN4_UNSUPPORTED when \a requireLabel is set but
 *         instrument.outputformat.scheduleLabel is false
 * \return #RBRGEN4_TIMEOUT when a timeout occurs, or when no
 *         polled sample arrives within RBRGen4.pollTimeout
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when an invalid group is requested, or
 *                                      another hardware error occurs
 */
RBRGen4Error RBRGen4_pollGroups(RBRGen4 *conn, bool requireLabel, const RBRGen4LabelList *groupList,
                                RBRGen4Sample *sample);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN4REALTIME_H */
