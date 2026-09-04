/**
 * \file RBRInstrumentGen4Internal.h
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

#ifndef LIBRBR_RBRINSTRUMENTGEN4INTERNAL_H
#define LIBRBR_RBRINSTRUMENTGEN4INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Streaming.h"

/** \brief Timestamp indicating that no instrument activity has occurred. */
#define RBRINSTRUMENTGEN4_NO_ACTIVITY ((RBRInstrumentGen4DateTime) - 1)

/** \brief The terminator at the end of a command sent to the instrument. */
#define RBRINSTRUMENTGEN4_SEND_COMMAND_TERMINATOR "\r"
/** \brief The length of the command terminator. */
#define RBRINSTRUMENTGEN4_SEND_COMMAND_TERMINATOR_LEN 1
/** \brief The terminator at the end of a command received from the instrument. */
#define RBRINSTRUMENTGEN4_RESPONSE_TERMINATOR "\r\n"
/** \brief The length of the command terminator. */
#define RBRINSTRUMENTGEN4_RESPONSE_TERMINATOR_LEN 2
/** \brief The value an empty list is reported and sent as. */
#define RBRINSTRUMENTGEN4_EMPTY_LIST "none"

/**
 * \brief The length of the timestamp of a streamed sample.
 *
 * “YYYY-mm-dd HH:MM:SS.sss” format.
 */
#define RBRINSTRUMENTGEN4_SAMPLE_TIME_LEN 23

/**
 * \brief The length of the timestamp of schedule settings.
 *
 * “YYYYmmddHHMMSS” format.
 */
#define RBRINSTRUMENTGEN4_SCHEDULE_TIME_LEN 14

/**
 * \brief Simple error-checked return around a function call.
 *
 * Evaluates the function call passed as \a op. If it returns a value other
 * than #RBRINSTRUMENTGEN4_SUCCESS, then that value is returned again. Useful for
 * forwarding errors from other API functions.
 */
#define RBR_TRY(op) do { \
        RBRInstrumentGen4Error _tryErr; \
        if ((_tryErr = (op)) != RBRINSTRUMENTGEN4_SUCCESS) \
        { \
            return _tryErr; \
        } \
} while (0)

/**
 * \brief Zero every member of a structure except one.
 *
 * \param [in,out] object a pointer to the structure
 * \param [in] member the name of the member to keep
 */
#define RBR_RESET_EXCEPT(object, member) do { \
        char *_begin = (char *) (object); \
        char *_keepBegin = (char *) &(object)->member; \
        char *_keepEnd = _keepBegin + sizeof((object)->member); \
        memset(_begin, 0, (size_t) (_keepBegin - _begin)); \
        memset(_keepEnd, \
               0, \
               sizeof(*(object)) - (size_t) (_keepEnd - _begin)); \
} while (0)

/**
 * Send the first RBRInstrumentGen4.commandBufferLength bytes of
 * RBRInstrumentGen4.commandBuffer to the instrument. No formatting or validation
 * of the contents of the buffer will be performed.
 *
 * You almost certainly want to use RBRInstrumentGen4_sendCommand() instead unless
 * you have a specific requirement for custom buffer management (like sending
 * a very large command in multiple pieces).
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the command is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_sendCommand() to send a string command
 */
RBRInstrumentGen4Error RBRInstrumentGen4_sendBuffer(RBRInstrumentGen4 *instrument);

/**
 * Send a command to the instrument. The command will be formatted into
 * RBRInstrumentGen4.commandBuffer and RBRInstrumentGen4.commandBufferLength will be
 * updated accordingly. If the command does not include a terminating `\r\n`,
 * it will be added for you.
 *
 * This function should only be used to send commands which don't produce any
 * response, or in conjunction with response parsing via
 * RBRInstrumentGen4_readResponse(). If the command is known to produce a response
 * – even if you don't care about it – you should read it to get it out of the
 * response buffer. To combine command sending and response reading with basic
 * sanity-checking, use RBRInstrumentGen4_converse().
 *
 * \param [in] instrument the instrument connection
 * \param [in] command the command to send as a printf-style format string
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the command is successfully written
 * \return #RBRINSTRUMENTGEN4_BUFFER_TOO_SMALL when the formatted command is too
 *                                         large for the command buffer
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_sendBuffer() to send raw data from the command buffer
 * \see RBRInstrumentGen4_readResponse() to read the command response
 * \see RBRInstrumentGen4_converse() for a send/receive shortcut
 */
RBRInstrumentGen4Error RBRInstrumentGen4_sendCommand(RBRInstrumentGen4 *instrument,
                                             const char *command,
                                             ...);

/**
 * Read a response from the instrument. This function will block until a
 * complete response is read, or until the callback returns
 * #RBRINSTRUMENTGEN4_TIMEOUT or #RBRINSTRUMENTGEN4_CALLBACK_ERROR.
 *
 * The response will be returned via RBRInstrumentGen4.responseBuffer. The previous
 * complete response, if any, will be removed, and newly-read data will be
 * appended to any trailing incomplete response. Some minor parsing of the
 * response will be performed: any leading prompt will be stripped off; the
 * carriage return portion of the response line terminator will be replaced
 * with a null terminator; and RBRInstrumentGen4.response and
 * RBRInstrumentGen4.lastResponseLength will be populated appropriately.
 *
 * If \a breakOnSample is true, then the function will return
 * #RBRINSTRUMENTGEN4_SAMPLE immediately after parsing a sample. Otherwise, it will
 * handle the sample then continue to read further responses.
 *
 * If \a sample is given as a non-`NULL` pointer and a sample response (either
 * streamed or fetched) is found, that sample will be written to \a sample.
 * Otherwise, sample data will be sent to the RBRInstrumentGen4SampleCallback set
 * via RBRInstrumentGen4Callbacks.sample, if populated. It doesn't make much sense
 * to set this without also passing \a breakOnSample as true; if
 * \a breakOnSample is false then \a sample will be populated with the most
 * recent sample incidentally encountered while parsing other responses.
 *
 * \param [in] instrument the instrument connection
 * \param [in] breakOnSample whether to return early when a sample is parsed
 * \param [out] sample where to put a parsed sample
 * \return #RBRINSTRUMENTGEN4_SUCCESS when a response was successfully read
 * \return #RBRINSTRUMENTGEN4_SAMPLE when a sample is read and \a sample is given
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRInstrumentGen4_sendCommand() to send a command
 * \see RBRInstrumentGen4_converse() for a send/receive shortcut
 */
RBRInstrumentGen4Error RBRInstrumentGen4_readResponse(RBRInstrumentGen4 *instrument,
                                              bool breakOnSample,
                                              RBRInstrumentGen4Sample *sample);

/**
 * Deliver a sample to the RBRInstrumentGen4SampleCallback set via
 * RBRInstrumentGen4Callbacks.sample, if any. If \a sample is not already
 * RBRInstrumentGen4Callbacks.sampleBuffer, it will be copied there first;
 * RBRInstrumentGen4_open() guarantees that sampleBuffer is non-`NULL`
 * whenever the callback is set.
 *
 * \param [in] instrument the instrument connection
 * \param [in] sample the sample to deliver
 * \return #RBRINSTRUMENTGEN4_SUCCESS when no callback is set, or the value
 *         returned by the callback otherwise
 * \see RBRInstrumentGen4_open() for the sampleBuffer guarantee
 */
RBRInstrumentGen4Error RBRInstrumentGen4_deliverSample(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Sample *sample);

/**
 * \brief Send a command to the instrument and await an appropriate response.
 *
 * This function is more than just a combination of RBRInstrumentGen4_sendCommand()
 * and RBRInstrumentGen4_readResponse(): because it knows which command was sent,
 * it has some idea of which response should be received. As such, it will loop
 * on RBRInstrumentGen4_readResponse() until the first word of the response matches
 * the command sent. That means that a #RBRINSTRUMENTGEN4_TIMEOUT error returned
 * from this function means that a timeout was reached waiting for the
 * _correct_ response, not just _any_ response.
 *
 * \param [in] instrument the instrument connection
 * \param [in] command the command to send as a printf-style format string
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the command was successfully sent and a
 *                                response was read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRInstrumentGen4_sendCommand() to send a command
 * \see RBRInstrumentGen4_readResponse() to read the command response
 */
RBRInstrumentGen4Error RBRInstrumentGen4_converse(RBRInstrumentGen4 *instrument,
                                          const char *command,
                                          ...);

/**
 * \brief Read a single boolean parameter from the instrument.
 *
 * This function is a convenience specialization over RBRInstrumentGen4_converse()
 * and RBRInstrumentGen4_readResponse(): it sends a command in the standard format
 * for retrieving a single parameter (`command parameter`), then parses the
 * response looking for the value of that single parameter.
 *
 * \param [in] instrument the instrument connection
 * \param [in] command the name of the command
 * \param [in] parameter the name of the parameter
 * \param [out] value the parameter value
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the command was successfully sent and a
 *                                response was read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRInstrumentGen4_converse() to send a command
 * \see RBRInstrumentGen4_readResponse() to read the command response
 * \see RBRInstrumentGen4_getFloat() for the float equivalent
 * \see RBRInstrumentGen4_getInt() for the integer equivalent
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getBool(RBRInstrumentGen4 *instrument,
                                         const char *command,
                                         const char *parameter,
                                         bool *value);

/**
 * \brief Read a single float parameter from the instrument.
 *
 * This function is a convenience specialization over RBRInstrumentGen4_converse()
 * and RBRInstrumentGen4_readResponse(): it sends a command in the standard format
 * for retrieving a single parameter (`command parameter`), then parses the
 * response looking for the value of that single parameter.
 *
 * \param [in] instrument the instrument connection
 * \param [in] command the name of the command
 * \param [in] parameter the name of the parameter
 * \param [out] value the parameter value
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the command was successfully sent and a
 *                                response was read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRInstrumentGen4_converse() to send a command
 * \see RBRInstrumentGen4_readResponse() to read the command response
 * \see RBRInstrumentGen4_getBool() for the boolean equivalent
 * \see RBRInstrumentGen4_getInt() for the integer equivalent
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getFloat(RBRInstrumentGen4 *instrument,
                                          const char *command,
                                          const char *parameter,
                                          float *value);

/**
 * \brief Read a single integer parameter from the instrument.
 *
 * This function is a convenience specialization over RBRInstrumentGen4_converse()
 * and RBRInstrumentGen4_readResponse(): it sends a command in the standard format
 * for retrieving a single parameter (`command parameter`), then parses the
 * response looking for the value of that single parameter.
 *
 * \param [in] instrument the instrument connection
 * \param [in] command the name of the command
 * \param [in] parameter the name of the parameter
 * \param [out] value the parameter value
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the command was successfully sent and a
 *                                response was read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRInstrumentGen4_converse() to send a command
 * \see RBRInstrumentGen4_readResponse() to read the command response
 * \see RBRInstrumentGen4_getBool() for the boolean equivalent
 * \see RBRInstrumentGen4_getFloat() for the float equivalent
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getInt(RBRInstrumentGen4 *instrument,
                                        const char *command,
                                        const char *parameter,
                                        int32_t *value);

/** \brief A parameter (key/value pair) from an instrument response. */
typedef struct RBRInstrumentGen4ResponseParameter
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
} RBRInstrumentGen4ResponseParameter;

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
void RBRInstrumentGen4_parseResponse(RBRInstrumentGen4 *instrument,
                                 char **command,
                                 RBRInstrumentGen4ResponseParameter *parameter);


/**
 * \brief Check for errors or warnings in an instrument response.
 *
 * Updates RBRInstrumentGen4.response as appropriate.
 *
 * \param [in,out] instrument the instrument connection
 * \param [in] beginning the beginning of the textual response
 * \param [in] end the end of the textual response
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the response is a warning or success
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the response indicates an error
 */
RBRInstrumentGen4Error RBRInstrumentGen4_errorCheckResponse(
    RBRInstrumentGen4 *instrument, 
    char *beginning, 
    char *end);

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
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the timestamp is successfully parsed
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the time is invalid
 */
RBRInstrumentGen4Error RBRInstrumentGen4DateTime_parseSampleTime(
    const char *s,
    RBRInstrumentGen4DateTime *timestamp,
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
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the timestamp is successfully parsed
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the time is invalid
 */
RBRInstrumentGen4Error RBRInstrumentGen4DateTime_parseScheduleTime(
    const char *s,
    RBRInstrumentGen4DateTime *timestamp,
    char **end);

/**
 * \brief Convert a timestamp to a sample time/date string (i.e.,
 * “YYYY-mm-dd HH:MM:SS.sss” format).
 *
 * Exactly #RBRINSTRUMENTGEN4_SAMPLE_TIME_LEN + 1 characters will be written into
 * the buffer for the timestamp plus null terminator.
 *
 * \param [in] timestamp the timestamp
 * \param [out] s the destination buffer
 */
void RBRInstrumentGen4DateTime_toSampleTime(RBRInstrumentGen4DateTime timestamp,
                                        char *s);

/**
 * \brief Convert a timestamp to a schedule setting time/date string (i.e.,
 * “YYYYmmddHHMMSS” format).
 *
 * Exactly #RBRINSTRUMENTGEN4_SCHEDULE_TIME_LEN + 1 characters will be written into
 * the buffer for the timestamp plus null terminator.
 *
 * \param [in] timestamp the timestamp
 * \param [out] s the destination buffer
 */
void RBRInstrumentGen4DateTime_toScheduleTime(RBRInstrumentGen4DateTime timestamp,
                                          char *s);

/**
 * \brief Terminate the first value of a list, and find the next one.
 *
 * Instrument responses separate the values of a list-valued parameter with
 * vertical bars: `availablebaudrates=4800|9600`, `list=self|fe4_cond_00`.
 * Iterate over one by walking the value returned until it is `NULL`.
 *
 * \param [in,out] value the list, terminated after its first value
 * \return the next value in the list, or NULL at the end of the list
 */
char *RBRInstrumentGen4_splitListValue(char *value);

/**
 * \brief Format a label list as a pipe-separated parameter value.
 *
 * An empty list is written as `none`.
 *
 * \param [out] value the buffer to write the list into
 * \param [in] size the size of \a value
 * \param [in] labelList the labels to write
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the list is formatted
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when \a labelList is
 *                                                    `NULL`, its count does
 *                                                    not fit its array, or a
 *                                                    label is empty
 * \return #RBRINSTRUMENTGEN4_BUFFER_TOO_SMALL when the list does not fit
 */
RBRInstrumentGen4Error RBRInstrumentGen4_formatLabelList(
    char *value,
    int32_t size,
    const RBRInstrumentGen4LabelList *labelList);

/**
 * \brief Copy a pipe-separated parameter value into a label list.
 *
 * `none` yields a zero count. The count is the number of labels reported;
 * labels past the list's capacity are discarded.
 *
 * \param [out] labelList the caller-provided label list
 * \param [in,out] value the response value, consumed in place
 * \return #RBRINSTRUMENTGEN4_SUCCESS when every label is stored
 * \return #RBRINSTRUMENTGEN4_TRUNCATED when labels were discarded
 */
RBRInstrumentGen4Error RBRInstrumentGen4_copyLabelList(
    RBRInstrumentGen4LabelList *labelList,
    char *value);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTGEN4INTERNAL_H */
