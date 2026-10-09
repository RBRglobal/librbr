/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file RBRGen3Internal.h
 *
 * \brief Internal functions used across the library.
 *
 * Not intended for consumption by end users. As such, interfaces and contracts
 * defined within this file may be unstable from version to version. If there's
 * something in here you think you need, please let us know and we'll discuss
 * how to expose it stably.
 */

#ifndef LIBRBR_RBRGEN3INTERNAL_H
#define LIBRBR_RBRGEN3INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RBRGen3.h"
#include "RBRGen3Streaming.h"

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
 * \brief Send the contents of the command buffer to the instrument.
 *
 * Sends the first RBRGen3.commandBufferLength bytes of
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
 * \return #RBRGEN3_COMMAND_TOO_LONG when the buffer is full
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRGen3_sendCommand() to send a string command
 */
RBRGen3Error RBRGen3_sendBuffer(RBRGen3 *conn);

/**
 * \brief Send a command to the instrument.
 *
 * The command will be formatted into
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
 * \return #RBRGEN3_COMMAND_TOO_LONG when the formatted command is too large for the command buffer
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRGen3_sendBuffer() to send raw data from the command buffer
 * \see RBRGen3_readResponse() to read the command response
 * \see RBRGen3_converse() for a send/receive shortcut
 */
RBRGen3Error RBRGen3_sendCommand(RBRGen3 *conn, const char *command, ...);

/**
 * \brief Start a new command in RBRGen3Environment.command.
 *
 * Discards whatever the buffer holds from the previous command, including a
 * length poisoned by a failed append, so that RBRGen3_appendCommand() writes
 * from the start of the buffer.
 *
 * \param [in] conn the instrument connection
 * \see RBRGen3_appendCommand() to add to the command
 */
void RBRGen3_beginCommand(RBRGen3 *conn);

/**
 * \brief Append formatted text to the command in RBRGen3Environment.command.
 *
 * Builds a command piece by piece, directly in the caller-supplied buffer.
 * Call RBRGen3_beginCommand() before the first piece, then send the whole
 * with RBRGen3_converseBuffer() or RBRGen3_sendBuffer().
 *
 * \param [in] conn the instrument connection
 * \param [in] command the text to append as a printf-style format string
 * \return #RBRGEN3_SUCCESS when the text is appended
 * \return #RBRGEN3_COMMAND_TOO_LONG when the text does not fit; the buffer then holds a truncated
 *         command which RBRGen3_sendBuffer() refuses
 * \see RBRGen3_converseBuffer() to send the buffer and await the response
 */
RBRGen3Error RBRGen3_appendCommand(RBRGen3 *conn, const char *command, ...);

/**
 * \brief Read a response from the instrument.
 *
 * This function will block until a complete response is read, or until the
 * callback returns #RBRGEN3_TIMEOUT or #RBRGEN3_CALLBACK_ERROR.
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
 * \param [in,out] sample where to put a parsed sample; RBRGen3Sample.readings and
 *                        RBRGen3Sample.size must be set by the caller
 * \param [in] startTime when the wait began; the command timeout is measured from here, so a caller
 *                       which loops over this function to skip unrelated lines bounds the whole
 *                       wait by passing the same value each time
 * \return #RBRGEN3_SUCCESS when a response was successfully read
 * \return #RBRGEN3_SAMPLE when a sample is read and \a sample is given
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_RESPONSE_TOO_LONG when the response exceeds the buffer
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the instrument reports a hardware error
 * \see RBRGen3_sendCommand() to send a command
 * \see RBRGen3_converse() for a send/receive shortcut
 */
RBRGen3Error RBRGen3_readResponse(RBRGen3 *conn, bool breakOnSample, RBRGen3Sample *sample,
                                  RBRGen3DateTime startTime);

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
 * A line too long for the response buffer met while waiting is drained and
 * skipped, since it may be a streamed sample rather than the reply. If the
 * correct response then never arrives, the oversized line most likely was
 * it, and #RBRGEN3_RESPONSE_TOO_LONG is returned in place of the timeout.
 *
 * \param [in] conn the instrument connection
 * \param [in] command the command to send as a printf-style format string
 * \return #RBRGEN3_SUCCESS when the command was successfully sent and a response was read
 * \return #RBRGEN3_COMMAND_TOO_LONG when the command does not fit the command buffer
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_RESPONSE_TOO_LONG when a line too long for the response buffer was met and the
 *         correct response never arrived
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the instrument reports a hardware error
 * \see RBRGen3_sendCommand() to send a command
 * \see RBRGen3_readResponse() to read the command response
 */
RBRGen3Error RBRGen3_converse(RBRGen3 *conn, const char *command, ...);

/**
 * \brief Send the command already in RBRGen3Environment.command and await an appropriate response.
 *
 * The buffer form of RBRGen3_converse(), for commands assembled with
 * RBRGen3_appendCommand(). A terminator is added if the command lacks one.
 * The response matching and retry behaviour is exactly that of
 * RBRGen3_converse(); a retry resends the buffer as it stands.
 *
 * \param [in] conn the instrument connection
 * \return #RBRGEN3_SUCCESS when the command was successfully sent and a response was read
 * \return #RBRGEN3_COMMAND_TOO_LONG when the buffer holds a truncated command or has no room for
 *         the terminator
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_RESPONSE_TOO_LONG when a line too long for the response buffer was met and the
 *         correct response never arrived
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the instrument reports a hardware error
 * \see RBRGen3_appendCommand() to build the command
 */
RBRGen3Error RBRGen3_converseBuffer(RBRGen3 *conn);

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
 * \return #RBRGEN3_SUCCESS when the command was successfully sent and a response was read
 * \return #RBRGEN3_COMMAND_TOO_LONG when the command does not fit the command buffer
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the instrument reports a hardware error
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
 * \return #RBRGEN3_SUCCESS when the command was successfully sent and a response was read
 * \return #RBRGEN3_COMMAND_TOO_LONG when the command does not fit the command buffer
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the instrument reports a hardware error
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
 * \return #RBRGEN3_SUCCESS when the command was successfully sent and a response was read
 * \return #RBRGEN3_COMMAND_TOO_LONG when the command does not fit the command buffer
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the instrument reports a hardware error
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
 * \param [in] conn the instrument connection
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
 * \brief Append a timestamp as “YYYYmmddHHMMSS”, the form the clock, deployment, calibration and
 * postprocessing commands take, to the command in RBRGen3Environment.command.
 *
 * \param [in] conn the instrument connection
 * \param [in] timestamp the timestamp
 * \return #RBRGEN3_SUCCESS when the timestamp is appended
 * \return #RBRGEN3_COMMAND_TOO_LONG when it does not fit
 * \see RBRGen3_appendCommand()
 */
RBRGen3Error RBRGen3_appendDateTime(RBRGen3 *conn, RBRGen3DateTime timestamp);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN3INTERNAL_H */
