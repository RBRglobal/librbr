/**
 * \file RBRGen3Internal.h
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

#ifndef LIBRBR_RBRGEN3INTERNAL_H
#define LIBRBR_RBRGEN3INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Timestamp indicating that no instrument activity has occurred. */
#define RBRGEN3_NO_ACTIVITY ((RBRGen3DateTime) - 1)

/** \brief The terminator at the end of a command sent to the instrument. */
#define RBRGEN3_SEND_COMMAND_TERMINATOR     "\r"
/** \brief The length of the command terminator. */
#define RBRGEN3_SEND_COMMAND_TERMINATOR_LEN 1
/** \brief The terminator at the end of a command received from the instrument. */
#define RBRGEN3_COMMAND_TERMINATOR          "\r\n"
/** \brief The length of the command terminator. */
#define RBRGEN3_COMMAND_TERMINATOR_LEN      2

/**
 * \brief The length of the timestamp of a streamed sample.
 *
 * “YYYY-mm-dd HH:MM:SS.sss” format.
 */
#define RBRGEN3_SAMPLE_TIME_LEN 23

/**
 * \brief The length of the timestamp of schedule settings.
 *
 * “YYYYmmddHHMMSS” format.
 */
#define RBRGEN3_SCHEDULE_TIME_LEN 14

/**
 * \brief Simple error-checked return around a function call.
 *
 * Evaluates the function call passed as \a op. If it returns a value other
 * than #RBRGEN3_SUCCESS, then that value is returned again. Useful for
 * forwarding errors from other API functions.
 */
#define RBR_TRY(op)                                \
    do {                                           \
        RBRGen3Error _tryErr;                      \
        if ((_tryErr = (op)) != RBRGEN3_SUCCESS) { \
            return _tryErr;                        \
        }                                          \
    } while (0)

/**
 * Send the first RBRGen3.commandBufferLength bytes of
 * RBRGen3Environment.command to the instrument. No formatting of the contents of
 * the buffer is performed; a buffer with no room left for a null byte is
 * refused, since it holds a command truncated by snprintf().
 *
 * You almost certainly want to use RBRGen3_sendCommand() instead unless
 * you have a specific requirement for custom buffer management (like sending
 * a very large command in multiple pieces).
 *
 * \param [in] conn the instrument connection
 * \return #RBRGEN3_SUCCESS when the command is successfully written
 * \return #RBRGEN3_BUFFER_TOO_SMALL when the buffer is full
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRGen3_sendCommand() to send a string command
 */
RBRGen3Error RBRGen3_sendBuffer(RBRGen3 *conn);

/**
 * Send a command to the instrument. The command will be formatted into
 * RBRGen3Environment.command and RBRGen3.commandBufferLength will be
 * updated accordingly. If the command does not include a terminating `\r\n`,
 * it will be added for you.
 *
 * This function should only be used to send commands which don't produce any
 * response, or in conjunction with response parsing via
 * RBRGen3_readResponse(). If the command is known to produce a response
 * – even if you don't care about it – you should read it to get it out of the
 * response buffer. To combine command sending and response reading with basic
 * sanity-checking, use RBRGen3_converse().
 *
 * \param [in] conn the instrument connection
 * \param [in] command the command to send as a printf-style format string
 * \return #RBRGEN3_SUCCESS when the command is successfully written
 * \return #RBRGEN3_BUFFER_TOO_SMALL when the formatted command is too
 *                                         large for the command buffer
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRGen3_sendBuffer() to send raw data from the command buffer
 * \see RBRGen3_readResponse() to read the command response
 * \see RBRGen3_converse() for a send/receive shortcut
 */
RBRGen3Error RBRGen3_sendCommand(RBRGen3 *conn, const char *command, ...);

/**
 * Read a response from the instrument. This function will block until a
 * complete response is read, or until the callback returns
 * #RBRGEN3_TIMEOUT or #RBRGEN3_CALLBACK_ERROR.
 *
 * The response will be returned via RBRGen3Environment.response. The previous
 * complete response, if any, will be removed, and newly-read data will be
 * appended to any trailing incomplete response. Some minor parsing of the
 * response will be performed: any leading prompt will be stripped off; the
 * carriage return portion of the response line terminator will be replaced
 * with a null terminator; and RBRGen3.response and
 * RBRGen3.lastResponseLength will be populated appropriately.
 *
 * If \a breakOnSample is true, then the function will return
 * #RBRGEN3_SAMPLE immediately after parsing a sample. Otherwise, it will
 * handle the sample then continue to read further responses.
 *
 * If \a sample is given as a non-`NULL` pointer and a sample response (either
 * streamed or fetched) is found, that sample will be written to \a sample.
 * Otherwise, sample data will be sent to the RBRGen3SampleCallback set
 * via RBRGen3Environment.sample, if populated. It doesn't make much sense
 * to set this without also passing \a breakOnSample as true; if
 * \a breakOnSample is false then \a sample will be populated with the most
 * recent sample incidentally encountered while parsing other responses.
 *
 * \param [in] conn the instrument connection
 * \param [in] breakOnSample whether to return early when a sample is parsed
 * \param [out] sample where to put a parsed sample
 * \return #RBRGEN3_SUCCESS when a response was successfully read
 * \return #RBRGEN3_SAMPLE when a sample is read and \a sample is given
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRGen3_sendCommand() to send a command
 * \see RBRGen3_converse() for a send/receive shortcut
 */
RBRGen3Error RBRGen3_readResponse(RBRGen3 *conn, bool breakOnSample, RBRGen3Sample *sample);

/**
 * \brief Send a command to the instrument and await an appropriate response.
 *
 * This function is more than just a combination of RBRGen3_sendCommand()
 * and RBRGen3_readResponse(): because it knows which command was sent,
 * it has some idea of which response should be received. As such, it will loop
 * on RBRGen3_readResponse() until the first word of the response matches
 * the command sent. That means that a #RBRGEN3_TIMEOUT error returned
 * from this function means that a timeout was reached waiting for the
 * _correct_ response, not just _any_ response.
 *
 * \param [in] conn the instrument connection
 * \param [in] command the command to send as a printf-style format string
 * \return #RBRGEN3_SUCCESS when the command was successfully sent and a
 *                                response was read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRGen3_sendCommand() to send a command
 * \see RBRGen3_readResponse() to read the command response
 */
RBRGen3Error RBRGen3_converse(RBRGen3 *conn, const char *command, ...);

/**
 * \brief Read a single boolean parameter from the instrument.
 *
 * This function is a convenience specialization over RBRGen3_converse()
 * and RBRGen3_readResponse(): it sends a command in the standard format
 * for retrieving a single parameter (`command parameter`), then parses the
 * response looking for the value of that single parameter.
 *
 * \param [in] conn the instrument connection
 * \param [in] command the name of the command
 * \param [in] parameter the name of the parameter
 * \param [out] value the parameter value
 * \return #RBRGEN3_SUCCESS when the command was successfully sent and a
 *                                response was read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRGen3_converse() to send a command
 * \see RBRGen3_readResponse() to read the command response
 * \see RBRGen3_getFloat() for the float equivalent
 * \see RBRGen3_getInt() for the integer equivalent
 */
RBRGen3Error RBRGen3_getBool(RBRGen3 *conn, const char *command, const char *parameter,
                             bool *value);

/**
 * \brief Read a single float parameter from the instrument.
 *
 * This function is a convenience specialization over RBRGen3_converse()
 * and RBRGen3_readResponse(): it sends a command in the standard format
 * for retrieving a single parameter (`command parameter`), then parses the
 * response looking for the value of that single parameter.
 *
 * \param [in] conn the instrument connection
 * \param [in] command the name of the command
 * \param [in] parameter the name of the parameter
 * \param [out] value the parameter value
 * \return #RBRGEN3_SUCCESS when the command was successfully sent and a
 *                                response was read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRGen3_converse() to send a command
 * \see RBRGen3_readResponse() to read the command response
 * \see RBRGen3_getBool() for the boolean equivalent
 * \see RBRGen3_getInt() for the integer equivalent
 */
RBRGen3Error RBRGen3_getFloat(RBRGen3 *conn, const char *command, const char *parameter,
                              float *value);

/**
 * \brief Read a single integer parameter from the instrument.
 *
 * This function is a convenience specialization over RBRGen3_converse()
 * and RBRGen3_readResponse(): it sends a command in the standard format
 * for retrieving a single parameter (`command parameter`), then parses the
 * response looking for the value of that single parameter.
 *
 * \param [in] conn the instrument connection
 * \param [in] command the name of the command
 * \param [in] parameter the name of the parameter
 * \param [out] value the parameter value
 * \return #RBRGEN3_SUCCESS when the command was successfully sent and a
 *                                response was read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRGen3_converse() to send a command
 * \see RBRGen3_readResponse() to read the command response
 * \see RBRGen3_getBool() for the boolean equivalent
 * \see RBRGen3_getFloat() for the float equivalent
 */
RBRGen3Error RBRGen3_getInt(RBRGen3 *conn, const char *command, const char *parameter,
                            int32_t *value);

/** \brief A parameter (key/value pair) from an instrument response. */
typedef struct RBRGen3ResponseParameter {
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
} RBRGen3ResponseParameter;

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
 * \param [in] conn the instrument connection
 * \param [in,out] command the name of the command as indicated by the response
 * \param [in,out] parameter the most-recently-parsed response parameter
 */
void RBRGen3_parseResponse(RBRGen3 *conn, char **command, RBRGen3ResponseParameter *parameter);

/**
 * \brief Check for errors or warnings in an instrument response.
 *
 * Updates RBRGen3.response as appropriate.
 *
 * \param [in,out] conn the instrument connection
 * \param [in] beginning the beginning of the textual response
 * \param [in] end the end of the textual response
 * \return #RBRGEN3_SUCCESS when the response is a warning or success
 * \return #RBRGEN3_HARDWARE_ERROR when the response indicates an error
 */
RBRGen3Error RBRGen3_errorCheckResponse(RBRGen3 *conn, char *beginning, char *end);

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
 * \return #RBRGEN3_SUCCESS when the timestamp is successfully parsed
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when the time is invalid
 */
RBRGen3Error RBRGen3DateTime_parseSampleTime(const char *s, RBRGen3DateTime *timestamp, char **end);

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
 * \return #RBRGEN3_SUCCESS when the timestamp is successfully parsed
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when the time is invalid
 */
RBRGen3Error RBRGen3DateTime_parseScheduleTime(const char *s, RBRGen3DateTime *timestamp,
                                               char **end);

/**
 * \brief Convert a timestamp to a sample time/date string (i.e.,
 * “YYYY-mm-dd HH:MM:SS.sss” format).
 *
 * Exactly #RBRGEN3_SAMPLE_TIME_LEN + 1 characters will be written into
 * the buffer for the timestamp plus null terminator.
 *
 * \param [in] timestamp the timestamp
 * \param [out] s the destination buffer
 */
void RBRGen3DateTime_toSampleTime(RBRGen3DateTime timestamp, char *s);

/**
 * \brief Convert a timestamp to a schedule setting time/date string (i.e.,
 * “YYYYmmddHHMMSS” format).
 *
 * Exactly #RBRGEN3_SCHEDULE_TIME_LEN + 1 characters will be written into
 * the buffer for the timestamp plus null terminator.
 *
 * \param [in] timestamp the timestamp
 * \param [out] s the destination buffer
 */
void RBRGen3DateTime_toScheduleTime(RBRGen3DateTime timestamp, char *s);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN3INTERNAL_H */
