/**
 * \file RBRInstrumentInternal.h
 *
 * \brief Internal functions used across the library.
 *
 * Not intended for consumption by end users. As such, interfaces and contracts
 * defined within this file may be unstable from version to version. If there's
 * something in here you think you need, please let us know and we'll discuss
 * how to expose it stably.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTINTERNAL_H
#define LIBRBR_RBRINSTRUMENTINTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Timestamp indicating that no instrument activity has occurred. */
#define RBRINSTRUMENT_NO_ACTIVITY ((RBRInstrumentDateTime) - 1)

/** \brief The terminator at the end of a command sent to the instrument. */
#define RBRINSTRUMENT_COMMAND_TERMINATOR "\r\n"
/** \brief The length of the command terminator. */
#define RBRINSTRUMENT_COMMAND_TERMINATOR_LEN 2

/**
 * \brief The length of the timestamp of a streamed sample.
 *
 * “YYYY-mm-dd HH:MM:SS.sss” format.
 */
#define RBRINSTRUMENT_SAMPLE_TIME_LEN 23

/**
 * \brief The length of the timestamp of schedule settings.
 *
 * “YYYYmmddHHMMSS” format.
 */
#define RBRINSTRUMENT_SCHEDULE_TIME_LEN 14

/**
 * \brief Simple error-checked return around a function call.
 *
 * Evaluates the function call passed as \a op. If it returns a value other
 * than #RBRINSTRUMENT_SUCCESS, then that value is returned again. Useful for
 * forwarding errors from other API functions.
 */
#define RBR_TRY(op) do { \
        RBRInstrumentError _tryErr; \
        if ((_tryErr = (op)) != RBRINSTRUMENT_SUCCESS) \
        { \
            return _tryErr; \
        } \
} while (0)

/**
 * Send the first RBRInstrument.commandBufferLength bytes of
 * RBRInstrument.commandBuffer to the instrument. No formatting or validation
 * of the contents of the buffer will be performed.
 *
 * You almost certainly want to use RBRInstrument_sendCommand() instead unless
 * you have a specific requirement for custom buffer management (like sending
 * a very large command in multiple pieces).
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENT_SUCCESS when the command is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_sendCommand() to send a string command
 */
RBRInstrumentError RBRInstrument_sendBuffer(RBRInstrument *instrument);

/**
 * Send a command to the instrument. The command will be formatted into
 * RBRInstrument.commandBuffer and RBRInstrument.commandBufferLength will be
 * updated accordingly. If the command does not include a terminating `\r\n`,
 * it will be added for you.
 *
 * This function should only be used to send commands which don't produce any
 * response, or in conjunction with response parsing via
 * RBRInstrument_readResponse(). If the command is known to produce a response
 * – even if you don't care about it – you should read it to get it out of the
 * response buffer. To combine command sending and response reading with basic
 * sanity-checking, use RBRInstrument_converse().
 *
 * \param [in] instrument the instrument connection
 * \param [in] command the command to send as a printf-style format string
 * \return #RBRINSTRUMENT_SUCCESS when the command is successfully written
 * \return #RBRINSTRUMENT_BUFFER_TOO_SMALL when the formatted command is too
 *                                         large for the command buffer
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_sendBuffer() to send raw data from the command buffer
 * \see RBRInstrument_readResponse() to read the command response
 * \see RBRInstrument_converse() for a send/receive shortcut
 */
RBRInstrumentError RBRInstrument_sendCommand(RBRInstrument *instrument,
                                             const char *command,
                                             ...);

/**
 * Read a response from the instrument. This function will block until a
 * complete response is read, or until the callback returns
 * #RBRINSTRUMENT_TIMEOUT or #RBRINSTRUMENT_CALLBACK_ERROR.
 *
 * The response will be returned via RBRInstrument.responseBuffer. The previous
 * complete response, if any, will be removed, and newly-read data will be
 * appended to any trailing incomplete response. Some minor parsing of the
 * response will be performed: any leading prompt will be stripped off; the
 * carriage return portion of the response line terminator will be replaced
 * with a null terminator; and RBRInstrument.response and
 * RBRInstrument.lastResponseLength will be populated appropriately.
 *
 * If \a breakOnSample is true, then the function will return
 * #RBRINSTRUMENT_SAMPLE immediately after parsing a sample. Otherwise, it will
 * handle the sample then continue to read further responses.
 *
 * If \a sample is given as a non-`NULL` pointer and a sample response (either
 * streamed or fetched) is found, that sample will be written to \a sample.
 * Otherwise, sample data will be sent to the RBRInstrumentSampleCallback set
 * via RBRInstrumentCallbacks.sample, if populated. It doesn't make much sense
 * to set this without also passing \a breakOnSample as true; if
 * \a breakOnSample is false then \a sample will be populated with the most
 * recent sample incidentally encountered while parsing other responses.
 *
 * \param [in] instrument the instrument connection
 * \param [in] breakOnSample whether to return early when a sample is parsed
 * \param [out] sample where to put a parsed sample
 * \return #RBRINSTRUMENT_SUCCESS when a response was successfully read
 * \return #RBRINSTRUMENT_SAMPLE when a sample is read and \a sample is given
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRInstrument_sendCommand() to send a command
 * \see RBRInstrument_converse() for a send/receive shortcut
 */
RBRInstrumentError RBRInstrument_readResponse(RBRInstrument *instrument,
                                              bool breakOnSample,
                                              RBRInstrumentSample *sample);

/**
 * \brief Send a command to the instrument and await an appropriate response.
 *
 * This function is more than just a combination of RBRInstrument_sendCommand()
 * and RBRInstrument_readResponse(): because it knows which command was sent,
 * it has some idea of which response should be received. As such, it will loop
 * on RBRInstrument_readResponse() until the first word of the response matches
 * the command sent. That means that a #RBRINSTRUMENT_TIMEOUT error returned
 * from this function means that a timeout was reached waiting for the
 * _correct_ response, not just _any_ response.
 *
 * \param [in] instrument the instrument connection
 * \param [in] command the command to send as a printf-style format string
 * \return #RBRINSTRUMENT_SUCCESS when the command was successfully sent and a
 *                                response was read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRInstrument_sendCommand() to send a command
 * \see RBRInstrument_readResponse() to read the command response
 */
RBRInstrumentError RBRInstrument_converse(RBRInstrument *instrument,
                                          const char *command,
                                          ...);

/**
 * \brief Read a single boolean parameter from the instrument.
 *
 * This function is a convenience specialization over RBRInstrument_converse()
 * and RBRInstrument_readResponse(): it sends a command in the standard format
 * for retrieving a single parameter (`command parameter`), then parses the
 * response looking for the value of that single parameter.
 *
 * \param [in] instrument the instrument connection
 * \param [in] command the name of the command
 * \param [in] parameter the name of the parameter
 * \param [out] value the parameter value
 * \return #RBRINSTRUMENT_SUCCESS when the command was successfully sent and a
 *                                response was read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRInstrument_converse() to send a command
 * \see RBRInstrument_readResponse() to read the command response
 * \see RBRInstrument_getFloat() for the float equivalent
 * \see RBRInstrument_getInt() for the integer equivalent
 */
RBRInstrumentError RBRInstrument_getBool(RBRInstrument *instrument,
                                         const char *command,
                                         const char *parameter,
                                         bool *value);

/**
 * \brief Read a single float parameter from the instrument.
 *
 * This function is a convenience specialization over RBRInstrument_converse()
 * and RBRInstrument_readResponse(): it sends a command in the standard format
 * for retrieving a single parameter (`command parameter`), then parses the
 * response looking for the value of that single parameter.
 *
 * \param [in] instrument the instrument connection
 * \param [in] command the name of the command
 * \param [in] parameter the name of the parameter
 * \param [out] value the parameter value
 * \return #RBRINSTRUMENT_SUCCESS when the command was successfully sent and a
 *                                response was read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRInstrument_converse() to send a command
 * \see RBRInstrument_readResponse() to read the command response
 * \see RBRInstrument_getBool() for the boolean equivalent
 * \see RBRInstrument_getInt() for the integer equivalent
 */
RBRInstrumentError RBRInstrument_getFloat(RBRInstrument *instrument,
                                          const char *command,
                                          const char *parameter,
                                          float *value);

/**
 * \brief Read a single integer parameter from the instrument.
 *
 * This function is a convenience specialization over RBRInstrument_converse()
 * and RBRInstrument_readResponse(): it sends a command in the standard format
 * for retrieving a single parameter (`command parameter`), then parses the
 * response looking for the value of that single parameter.
 *
 * \param [in] instrument the instrument connection
 * \param [in] command the name of the command
 * \param [in] parameter the name of the parameter
 * \param [out] value the parameter value
 * \return #RBRINSTRUMENT_SUCCESS when the command was successfully sent and a
 *                                response was read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRInstrument_converse() to send a command
 * \see RBRInstrument_readResponse() to read the command response
 * \see RBRInstrument_getBool() for the boolean equivalent
 * \see RBRInstrument_getFloat() for the float equivalent
 */
RBRInstrumentError RBRInstrument_getInt(RBRInstrument *instrument,
                                        const char *command,
                                        const char *parameter,
                                        int32_t *value);

/** \brief A parameter (key/value pair) from an instrument response. */
typedef struct RBRInstrumentResponseParameter
{
    /** \brief The number of index parameters prior to this parameter. */
    int32_t index;
    /** \brief The string value of the last index parameter. */
    char *indexValue;
    /** \brief The parameter key. */
    char *key;
    /** \brief The parameter value. */
    char *value;
    /** \brief The start of the next key. */
    char *nextKey;
} RBRInstrumentResponseParameter;

/**
 * \brief Attempt to parse (tokenize/split key/value pairs) a command response.
 *
 * If \a command _points to_ `NULL` (not if it is `NULL` itself!), the function
 * will begin parsing from the beginning of the buffer. Otherwise, it will use
 * the contents of \a parameter to figure out where it stopped last time and
 * resume from there.
 *
 * If the command response is an array (e.g., `channel allindices`,
 * `calibration alllabels`), then \a parameter.index is incremented whenever
 * the delimiter is found.
 *
 * This function mutates the instrument response buffer. As such, it can't be
 * called more than once on the same response.
 *
 * \param [in] instrument the instrument connection
 * \param [in,out] command the name of the command as indicated by the response
 * \param [in,out] parameter the most-recently-parsed response parameter
 */
void RBRInstrument_parseResponse(RBRInstrument *instrument,
                                 char **command,
                                 RBRInstrumentResponseParameter *parameter);

/**
 * \brief Parse a date/time string from a sample (i.e.,
 * “YYYY-mm-dd HH:MM:SS.sss” format) to a timestamp.
 *
 * If \a end is not given as `NULL`, it will be modified to point to the first
 * character after the timestamp in \a s. If the timestamp cannot be parsed, it
 * will be modified to point to `NULL`.
 *
 * \param [in] s the sample date/time string
 * \param [out] timestamp the parsed timestamp
 * \param [out] end the first character not parsed
 * \return #RBRINSTRUMENT_SUCCESS when the timestamp is successfully parsed
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when the time is invalid
 */
RBRInstrumentError RBRInstrumentDateTime_parseSampleTime(
    const char *s,
    RBRInstrumentDateTime *timestamp,
    char **end);

/**
 * \brief Parse a date/time string from a schedule setting (i.e.,
 * “YYYYmmddHHMMSS” format) to a timestamp.
 *
 * If \a end is not given as `NULL`, it will be modified to point to the first
 * character after the timestamp in \a s. If the timestamp cannot be parsed, it
 * will be modified to point to `NULL`.
 *
 * \param [in] s the sample date/time string
 * \param [out] timestamp the parsed timestamp
 * \param [out] end the first character not parsed
 * \return #RBRINSTRUMENT_SUCCESS when the timestamp is successfully parsed
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when the time is invalid
 */
RBRInstrumentError RBRInstrumentDateTime_parseScheduleTime(
    const char *s,
    RBRInstrumentDateTime *timestamp,
    char **end);

/**
 * \brief Convert a timestamp to a sample time/date string (i.e.,
 * “YYYY-mm-dd HH:MM:SS.sss” format).
 *
 * Exactly #RBRINSTRUMENT_SAMPLE_TIME_LEN + 1 characters will be written into
 * the buffer for the timestamp plus null terminator.
 *
 * \param [in] timestamp the timestamp
 * \param [out] s the destination buffer
 */
void RBRInstrumentDateTime_toSampleTime(RBRInstrumentDateTime timestamp,
                                        char *s);

/**
 * \brief Convert a timestamp to a schedule setting time/date string (i.e.,
 * “YYYYmmddHHMMSS” format).
 *
 * Exactly #RBRINSTRUMENT_SCHEDULE_TIME_LEN + 1 characters will be written into
 * the buffer for the timestamp plus null terminator.
 *
 * \param [in] timestamp the timestamp
 * \param [out] s the destination buffer
 */
void RBRInstrumentDateTime_toScheduleTime(RBRInstrumentDateTime timestamp,
                                          char *s);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTINTERNAL_H */
