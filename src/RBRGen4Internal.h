/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file RBRGen4Internal.h
 *
 * \brief Internal functions used across the library.
 *
 * Not intended for consumption by end users. As such, interfaces and contracts
 * defined within this file may be unstable from version to version. If there's
 * something in here you think you need, please let us know and we'll discuss
 * how to expose it stably.
 */

#ifndef LIBRBR_RBRGEN4INTERNAL_H
#define LIBRBR_RBRGEN4INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RBRGen4.h"
#include "RBRGen4Realtime.h"

/** \brief Timestamp indicating that no instrument activity has occurred. */
#define RBRGEN4_NO_ACTIVITY ((RBRGen4DateTime) - 1)

/** \brief The terminator at the end of a command sent to the instrument. */
#define RBRGEN4_SEND_COMMAND_TERMINATOR     "\r"
/** \brief The length of the command terminator. */
#define RBRGEN4_SEND_COMMAND_TERMINATOR_LEN 1
/** \brief The terminator at the end of a command received from the instrument. */
#define RBRGEN4_RESPONSE_TERMINATOR         "\r\n"
/** \brief The length of the command terminator. */
#define RBRGEN4_RESPONSE_TERMINATOR_LEN     2
/** \brief The value an empty list is reported and sent as. */
#define RBRGEN4_EMPTY_LIST                  "none"

/**
 * \brief The length of the timestamp of a streamed sample.
 *
 * “YYYY-mm-dd HH:MM:SS.sss” format.
 */
#define RBRGEN4_SAMPLE_TIME_LEN 23

/**
 * \brief The length of the timestamp of schedule settings.
 *
 * “YYYYmmddHHMMSS” format.
 */
#define RBRGEN4_SCHEDULE_TIME_LEN 14

/**
 * \brief Simple error-checked return around a function call.
 *
 * Evaluates the function call passed as \a op. If it returns a value other
 * than #RBRGEN4_SUCCESS, then that value is returned again. Useful for
 * forwarding errors from other API functions.
 */
#define RBR_TRY(op)                                \
    do {                                           \
        RBRGen4Error _tryErr;                      \
        if ((_tryErr = (op)) != RBRGEN4_SUCCESS) { \
            return _tryErr;                        \
        }                                          \
    } while (0)

/**
 * \brief Zero every member of a structure except one.
 *
 * \param [in,out] object a pointer to the structure
 * \param [in] member the name of the member to keep
 */
#define RBR_RESET_EXCEPT(object, member)                                       \
    do {                                                                       \
        char *_begin = (char *) (object);                                      \
        char *_keepBegin = (char *) &(object)->member;                         \
        char *_keepEnd = _keepBegin + sizeof((object)->member);                \
        memset(_begin, 0, (size_t) (_keepBegin - _begin));                     \
        memset(_keepEnd, 0, sizeof(*(object)) - (size_t) (_keepEnd - _begin)); \
    } while (0)

/**
 * Send the first RBRGen4.commandBufferLength bytes of
 * RBRGen4Environment.command to the instrument. No formatting of the contents of
 * the buffer is performed; a buffer with no room left for a null byte is
 * refused, since it holds a command truncated by snprintf().
 *
 * You almost certainly want to use RBRGen4_sendCommand() instead unless
 * you have a specific requirement for custom buffer management (like sending
 * a very large command in multiple pieces).
 *
 * \param [in] conn the instrument connection
 * \return #RBRGEN4_SUCCESS when the command is successfully written
 * \return #RBRGEN4_COMMAND_TOO_LONG when the buffer is full
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRGen4_sendCommand() to send a string command
 */
RBRGen4Error RBRGen4_sendBuffer(RBRGen4 *conn);

/**
 * Send a command to the instrument. The command will be formatted into
 * RBRGen4Environment.command and RBRGen4.commandBufferLength will be
 * updated accordingly. If the command does not include a terminating `\r\n`,
 * it will be added for you.
 *
 * This function should only be used to send commands which don't produce any
 * response, or in conjunction with response parsing via
 * RBRGen4_readResponse(). If the command is known to produce a response
 * – even if you don't care about it – you should read it to get it out of the
 * response buffer. To combine command sending and response reading with basic
 * sanity-checking, use RBRGen4_converse().
 *
 * \param [in] conn the instrument connection
 * \param [in] command the command to send as a printf-style format string
 * \return #RBRGEN4_SUCCESS when the command is successfully written
 * \return #RBRGEN4_COMMAND_TOO_LONG when the formatted command is too
 *                                         large for the command buffer
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRGen4_sendBuffer() to send raw data from the command buffer
 * \see RBRGen4_readResponse() to read the command response
 * \see RBRGen4_converse() for a send/receive shortcut
 */
RBRGen4Error RBRGen4_sendCommand(RBRGen4 *conn, const char *command, ...);

/**
 * Read a response from the instrument. This function will block until a
 * complete response is read, or until the callback returns
 * #RBRGEN4_TIMEOUT or #RBRGEN4_CALLBACK_ERROR.
 *
 * The response will be returned via RBRGen4Environment.response. The previous
 * complete response, if any, will be removed, and newly-read data will be
 * appended to any trailing incomplete response. Some minor parsing of the
 * response will be performed: any leading prompt will be stripped off; the
 * carriage return portion of the response line terminator will be replaced
 * with a null terminator; and RBRGen4.response and
 * RBRGen4.lastResponseLength will be populated appropriately.
 *
 * If \a breakOnSample is true, then the function will return
 * #RBRGEN4_SAMPLE immediately after parsing a sample. Otherwise, it will
 * handle the sample then continue to read further responses.
 *
 * If \a sample is given as a non-`NULL` pointer and a sample response (either
 * streamed or fetched) is found, that sample will be written to \a sample.
 * Otherwise, sample data will be sent to the RBRGen4SampleCallback set
 * via RBRGen4Environment.sample, if populated. It doesn't make much sense
 * to set this without also passing \a breakOnSample as true; if
 * \a breakOnSample is false then \a sample will be populated with the most
 * recent sample incidentally encountered while parsing other responses.
 *
 * \param [in] conn the instrument connection
 * \param [in] breakOnSample whether to return early when a sample is parsed
 * \param [out] sample where to put a parsed sample
 * \param [in] startTime when the caller began waiting for this response
 * \param [in] timeout the longest to wait, in milliseconds, from \a startTime
 * \return #RBRGEN4_SUCCESS when a response was successfully read
 * \return #RBRGEN4_SAMPLE when a sample is read and \a sample is given
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when the response exceeds the buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRGen4_sendCommand() to send a command
 * \see RBRGen4_converse() for a send/receive shortcut
 */
RBRGen4Error RBRGen4_readResponse(RBRGen4 *conn, bool breakOnSample, RBRGen4Sample *sample,
                                  RBRGen4DateTime startTime, RBRGen4DateTime timeout);

/**
 * \brief Send a command to the instrument and await an appropriate response.
 *
 * This function is more than just a combination of RBRGen4_sendCommand()
 * and RBRGen4_readResponse(): because it knows which command was sent,
 * it has some idea of which response should be received. As such, it will loop
 * on RBRGen4_readResponse() until the first word of the response matches
 * the command sent. That means that a #RBRGEN4_TIMEOUT error returned
 * from this function means that a timeout was reached waiting for the
 * _correct_ response, not just _any_ response.
 *
 * A line too long for the response buffer met while waiting is drained and
 * skipped, since it may be a streamed sample rather than the reply. If the
 * correct response then never arrives, the oversized line most likely was
 * it, and #RBRGEN4_RESPONSE_TOO_LONG is returned in place of the timeout.
 *
 * \param [in] conn the instrument connection
 * \param [in] command the command to send as a printf-style format string
 * \return #RBRGEN4_SUCCESS when the command was successfully sent and a
 *                                response was read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a line too long for the response
 *         buffer was met and the correct response never arrived
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRGen4_sendCommand() to send a command
 * \see RBRGen4_readResponse() to read the command response
 */
RBRGen4Error RBRGen4_converse(RBRGen4 *conn, const char *command, ...);

/**
 * \brief Read a single boolean parameter from the instrument.
 *
 * This function is a convenience specialization over RBRGen4_converse()
 * and RBRGen4_readResponse(): it sends a command in the standard format
 * for retrieving a single parameter (`command parameter`), then parses the
 * response looking for the value of that single parameter.
 *
 * \param [in] conn the instrument connection
 * \param [in] command the name of the command
 * \param [in] parameter the name of the parameter
 * \param [out] value the parameter value
 * \return #RBRGEN4_SUCCESS when the command was successfully sent and a
 *                                response was read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRGen4_converse() to send a command
 * \see RBRGen4_readResponse() to read the command response
 * \see RBRGen4_getFloat() for the float equivalent
 * \see RBRGen4_getInt() for the integer equivalent
 */
RBRGen4Error RBRGen4_getBool(RBRGen4 *conn, const char *command, const char *parameter,
                             bool *value);

/**
 * \brief Read a single float parameter from the instrument.
 *
 * This function is a convenience specialization over RBRGen4_converse()
 * and RBRGen4_readResponse(): it sends a command in the standard format
 * for retrieving a single parameter (`command parameter`), then parses the
 * response looking for the value of that single parameter.
 *
 * \param [in] conn the instrument connection
 * \param [in] command the name of the command
 * \param [in] parameter the name of the parameter
 * \param [out] value the parameter value
 * \return #RBRGEN4_SUCCESS when the command was successfully sent and a
 *                                response was read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRGen4_converse() to send a command
 * \see RBRGen4_readResponse() to read the command response
 * \see RBRGen4_getBool() for the boolean equivalent
 * \see RBRGen4_getInt() for the integer equivalent
 */
RBRGen4Error RBRGen4_getFloat(RBRGen4 *conn, const char *command, const char *parameter,
                              float *value);

/**
 * \brief Read a single integer parameter from the instrument.
 *
 * This function is a convenience specialization over RBRGen4_converse()
 * and RBRGen4_readResponse(): it sends a command in the standard format
 * for retrieving a single parameter (`command parameter`), then parses the
 * response looking for the value of that single parameter.
 *
 * \param [in] conn the instrument connection
 * \param [in] command the name of the command
 * \param [in] parameter the name of the parameter
 * \param [out] value the parameter value
 * \return #RBRGEN4_SUCCESS when the command was successfully sent and a
 *                                response was read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRGen4_converse() to send a command
 * \see RBRGen4_readResponse() to read the command response
 * \see RBRGen4_getBool() for the boolean equivalent
 * \see RBRGen4_getFloat() for the float equivalent
 */
RBRGen4Error RBRGen4_getInt(RBRGen4 *conn, const char *command, const char *parameter,
                            int32_t *value);

/** \brief A parameter (key/value pair) from an instrument response. */
typedef struct RBRGen4ResponseParameter {
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
} RBRGen4ResponseParameter;

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
void RBRGen4_parseResponse(RBRGen4 *conn, char **command, RBRGen4ResponseParameter *parameter);

/**
 * \brief Check for errors or warnings in an instrument response.
 *
 * Updates RBRGen4.response as appropriate.
 *
 * \param [in,out] conn the instrument connection
 * \param [in] beginning the beginning of the textual response
 * \param [in] end the end of the textual response
 * \return #RBRGEN4_SUCCESS when the response is a warning or success
 * \return #RBRGEN4_HARDWARE_ERROR when the response indicates an error
 */
RBRGen4Error RBRGen4_errorCheckResponse(RBRGen4 *conn, char *beginning, char *end);

/**
 * \brief Parse a sample's timestamp: either a date/time string (i.e.,
 * “YYYY-mm-dd HH:MM:SS.sss” format) or a bare count of milliseconds.
 *
 * If \a end is not given as `NULL`, it will be modified to point to the first
 * character after the timestamp in \a s. If the timestamp cannot be parsed, it
 * will be modified to point to `NULL`.
 *
 * \param [in] s the sample date/time string
 * \param [out] timestamp the parsed timestamp
 * \param [out] end the first character not parsed
 * \return #RBRGEN4_SUCCESS when the timestamp is successfully parsed
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the time is invalid
 */
RBRGen4Error RBRGen4DateTime_parseSampleTime(const char *s, RBRGen4DateTime *timestamp, char **end);

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
 * \return #RBRGEN4_SUCCESS when the timestamp is successfully parsed
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the time is invalid
 */
RBRGen4Error RBRGen4DateTime_parseScheduleTime(const char *s, RBRGen4DateTime *timestamp,
                                               char **end);

/**
 * \brief Convert a timestamp to a sample time/date string (i.e.,
 * “YYYY-mm-dd HH:MM:SS.sss” format).
 *
 * Exactly #RBRGEN4_SAMPLE_TIME_LEN + 1 characters will be written into
 * the buffer for the timestamp plus null terminator.
 *
 * \param [in] timestamp the timestamp
 * \param [out] s the destination buffer
 */
void RBRGen4DateTime_toSampleTime(RBRGen4DateTime timestamp, char *s);

/**
 * \brief Convert a timestamp to a schedule setting time/date string (i.e.,
 * “YYYYmmddHHMMSS” format).
 *
 * Exactly #RBRGEN4_SCHEDULE_TIME_LEN + 1 characters will be written into
 * the buffer for the timestamp plus null terminator.
 *
 * \param [in] timestamp the timestamp
 * \param [out] s the destination buffer
 */
void RBRGen4DateTime_toScheduleTime(RBRGen4DateTime timestamp, char *s);

/**
 * \brief Terminate the first value of a list, and find the next one.
 *
 * Instrument responses separate the values of a list-valued parameter with
 * vertical bars: `list=temperature_00|pressure_00`.
 * Iterate over one by walking the value returned until it is `NULL`.
 *
 * \param [in,out] value the list, terminated after its first value
 * \return the next value in the list, or NULL at the end of the list
 */
char *RBRGen4_splitListValue(char *value);

/**
 * \brief Format a label list as a pipe-separated parameter value.
 *
 * An empty list is written as `none`.
 *
 * \param [out] value the buffer to write the list into
 * \param [in] size the size of \a value
 * \param [in] labelList the labels to write
 * \return #RBRGEN4_SUCCESS when the list is formatted
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when \a labelList is
 *                                          `NULL`, its length does
 *                                          not fit its array, or a
 *                                          label is empty
 * \return #RBRGEN4_COMMAND_TOO_LONG when the list does not fit
 */
RBRGen4Error RBRGen4_formatLabelList(char *value, int32_t size, const RBRGen4LabelList *labelList);

/**
 * \brief Copy a pipe-separated parameter value into a label list.
 *
 * `none` yields a zero length. Labels past the list's capacity are
 * discarded.
 *
 * \param [out] labelList the caller-provided label list
 * \param [in,out] value the response value, consumed in place
 * \return #RBRGEN4_SUCCESS when every label is stored
 * \return #RBRGEN4_TRUNCATED when labels were discarded
 */
RBRGen4Error RBRGen4_copyLabelList(RBRGen4LabelList *labelList, char *value);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN4INTERNAL_H */
