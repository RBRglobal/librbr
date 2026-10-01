/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file RBRGen4Internal.c
 *
 * \brief Library implementation.
 */

/* Required for isspace. */
#include <ctype.h>
/* Required for INFINITY, NAN. */
#include <math.h>
/* Required for vsnprintf, va_list, va_start, va_end. */
#include <stdarg.h>
/* Required for strtod, strtol. */
#include <stdlib.h>
/* Required for memcmp, memcpy, memmove, memset, strlen, strstr. */
#include <string.h>
/* Required for snprintf, sscanf. */
#include <stdio.h>
/* Required for gmtime, struct tm, mktime. */
#include <time.h>

#include "RBRGen4.h"
#include "RBRGen4Internal.h"
#include "RBRGen4Memory.h"
#include "RBRGen4Realtime.h"

/** \brief 10-second command timeout. */
#define COMMAND_TIMEOUT (10 * 1000)

/* The command reference suggests using a carriage return (`\r`) as the wake
 * character with a 10ms pause. This works well when talking directly to the
 * instrument over a USB or serial link. However, without giving the user the
 * option of reconfiguring wake behaviour, we need to consider how possible
 * alternate transports might have different requirements. */

/**
 * \brief The character sequence to send to wake the instrument.
 *
 * To improve compatibility with modems and other serial converters which
 * packetize conservatively (e.g., serial-over-Ethernet devices), we'll use
 * both a carriage return and a line feed. One or the other should satisfy the
 * default transmission criteria for most packetizers, and by reusing the
 * command terminator, we can make life easier for users of systems which only
 * support a single match criteria by ensuring that attempts to wake the
 * instrument will trigger the same behaviour as regular commands.
 */
#define WAKE_COMMAND      RBRGEN4_RESPONSE_TERMINATOR
/** \brief The length of the wake sequence. */
#define WAKE_COMMAND_LEN  RBRGEN4_RESPONSE_TERMINATOR_LEN
/**
 * \brief How long to wait after the wake sequence.
 *
 * 50ms is far longer than it will take to generate any regular command, so
 * this should be enough margin for any transport which transmits based only on
 * idle time.
 */
#define WAKE_COMMAND_WAIT 50

#define COMMAND_PROMPT     "ready: "
#define COMMAND_PROMPT_LEN 7

#define ARRAY_SEPARATOR_L4           '|'
#define PARAMETER_SEPARATOR_L4       ' '
#define PARAMETER_VALUE_SEPARATOR_L4 '='

#define OFFSET_UNINITIALIZED (-1)

#define ERROR_PARAMETER     "ERR-"
#define ERROR_PARAMETER_LEN ((long) (sizeof(ERROR_PARAMETER) - 1))
#define ERROR_NUMBER_LEN    3

#define WARNING_PARAMETER     "WRN-"
#define WARNING_PARAMETER_LEN ((long) (sizeof(WARNING_PARAMETER) - 1))
#define WARNING_NUMBER_LEN    3

#define SAMPLE_NAN              "nan"
#define SAMPLE_INF              "inf"
#define SAMPLE_NINF             "-inf"
#define SAMPLE_ERROR_PREFIX     "Error-"
#define SAMPLE_ERROR_PREFIX_LEN ((long) (sizeof(SAMPLE_ERROR_PREFIX) - 1))

static const char *RBRGen4DateTime_sampleScanFormat = "%04d-%02d-%02d %02d:%02d:%02d.%03d%n";

static const char *RBRGen4DateTime_sampleMillisecondsScanFormat = "%" SCNi64 "%n";

static const char *RBRGen4DateTime_scheduleFormat = "%04d%02d%02d%02d%02d%02d";

static const char *RBRGen4DateTime_scheduleScanFormat = "%04d%02d%02d%02d%02d%02d%n";

static RBRGen4DateTime localTimeOffset = OFFSET_UNINITIALIZED;

/**
 * \brief Like strstr, but for memory.
 *
 * Static: the Gen3 sources define an identical helper of the same name, and
 * both generations can be linked into one image.
 */
static void *rbr_memmem(void *ptr1, size_t num1, const void *ptr2, size_t num2)
{
    if (num2 > num1) {
        return NULL;
    }

    for (size_t offset = 0; offset <= num1 - num2; ++offset) {
        if (memcmp((uint8_t *) ptr1 + offset, ptr2, num2) == 0) {
            return (uint8_t *) ptr1 + offset;
        }
    }

    return NULL;
}

/**
 * \brief Wake the instrument from sleep, if necessary.
 *
 * \param [in] conn the instrument connection
 * \return #RBRGEN4_SUCCESS when the instrument has been woken
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR when an unrecoverable error occurs
 */
static RBRGen4Error RBRGen4_wake(const RBRGen4 *conn)
{
    RBRGen4DateTime now;
    RBR_TRY(conn->environment.time(conn, &now));

    if (conn->lastActivityTime >= 0 && now - conn->lastActivityTime < COMMAND_TIMEOUT) {
        return RBRGEN4_SUCCESS;
    }

    /* Send the wake sequence twice to make sure it gets noticed. */
    for (int pass = 0; pass < 2; ++pass) {
        RBR_TRY(conn->environment.write(conn, WAKE_COMMAND, WAKE_COMMAND_LEN));
        RBR_TRY(conn->environment.sleep(conn, WAKE_COMMAND_WAIT));
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_sendBuffer(RBRGen4 *conn)
{
    /*
     * The command builders accumulate snprintf()'s return value, which is the
     * number of characters it would have written given unlimited room, not
     * the number it actually wrote, and snprintf() always spends one byte of
     * the buffer on a terminating null. A length at or beyond the capacity
     * therefore means some snprintf() call ran out of room and the buffer
     * holds an incomplete command with a null where its last character should
     * be. Refuse it before anything, including the wake sequence, goes out.
     */
    if (conn->commandBufferLength >= conn->environment.commandCapacity) {
        return RBRGEN4_COMMAND_TOO_LONG;
    }

    /* Wake the instrument if necessary. */
    RBR_TRY(RBRGen4_wake(conn));

    /* Send the command to the instrument. */
    RBR_TRY(conn->environment.write(conn, conn->environment.command, conn->commandBufferLength));
    RBR_TRY(conn->environment.time(conn, &conn->lastActivityTime));
    return RBRGEN4_SUCCESS;
}

static RBRGen4Error RBRGen4_vAppendCommand(RBRGen4 *conn, const char *command, va_list format)
{
    /* The remaining space is only meaningful while the length is still within
     * the buffer. */
    if (conn->commandBufferLength < 0 ||
        conn->commandBufferLength >= conn->environment.commandCapacity) {
        return RBRGEN4_COMMAND_TOO_LONG;
    }

    int32_t written =
        vsnprintf((char *) conn->environment.command + conn->commandBufferLength,
                  (size_t) (conn->environment.commandCapacity - conn->commandBufferLength),
                  command,
                  format);

    /* Make sure we're within buffer bounds. This is a greater-or-equal check,
     * not just a greater-than check, because vsnprintf doesn't include the
     * null terminator in its return value. The longest value vsnprintf can
     * write is commandBufferCapacity - 1 bytes. */
    if (written < 0 || conn->commandBufferLength + written >= conn->environment.commandCapacity) {
        conn->commandBufferLength = conn->environment.commandCapacity;
        return RBRGEN4_COMMAND_TOO_LONG;
    }

    conn->commandBufferLength += written;
    return RBRGEN4_SUCCESS;
}

void RBRGen4_beginCommand(RBRGen4 *conn)
{
    conn->commandBufferLength = 0;
}

RBRGen4Error RBRGen4_appendCommand(RBRGen4 *conn, const char *command, ...)
{
    RBRGen4Error err;
    va_list format;
    va_start(format, command);
    err = RBRGen4_vAppendCommand(conn, command, format);
    va_end(format);
    return err;
}

RBRGen4Error RBRGen4_appendLabelList(RBRGen4 *conn, const RBRGen4LabelList *labelList)
{
    if (conn->commandBufferLength < 0 ||
        conn->commandBufferLength >= conn->environment.commandCapacity) {
        return RBRGEN4_COMMAND_TOO_LONG;
    }

    char *value = (char *) conn->environment.command + conn->commandBufferLength;
    RBRGen4Error err = RBRGen4_formatLabelList(
        value, conn->environment.commandCapacity - conn->commandBufferLength, labelList);
    if (err != RBRGEN4_SUCCESS) {
        /* Labels may already have been written past the length. Poison the
         * length as RBRGen4_appendCommand() does so RBRGen4_sendBuffer()
         * refuses the partial command. */
        conn->commandBufferLength = conn->environment.commandCapacity;
        return err;
    }
    conn->commandBufferLength += (int32_t) strlen(value);
    return RBRGEN4_SUCCESS;
}

/**
 * \brief Make sure the command in the buffer carries the send terminator.
 *
 * \param [in] conn the instrument connection
 * \return #RBRGEN4_SUCCESS when the command is terminated
 * \return #RBRGEN4_COMMAND_TOO_LONG when there is no room for the terminator
 */
static RBRGen4Error RBRGen4_terminateCommand(RBRGen4 *conn)
{
    if (conn->commandBufferLength >= RBRGEN4_SEND_COMMAND_TERMINATOR_LEN &&
        memcmp(conn->environment.command + conn->commandBufferLength -
                   RBRGEN4_SEND_COMMAND_TERMINATOR_LEN,
               RBRGEN4_SEND_COMMAND_TERMINATOR,
               RBRGEN4_SEND_COMMAND_TERMINATOR_LEN) == 0) {
        return RBRGEN4_SUCCESS;
    }

    return RBRGen4_appendCommand(conn, RBRGEN4_SEND_COMMAND_TERMINATOR);
}

RBRGen4Error RBRGen4_sendCommand(RBRGen4 *conn, const char *command, ...)
{
    RBRGen4Error err;
    va_list format;
    va_start(format, command);
    RBRGen4_beginCommand(conn);
    err = RBRGen4_vAppendCommand(conn, command, format);
    va_end(format);
    if (err != RBRGEN4_SUCCESS) {
        return err;
    }

    RBR_TRY(RBRGen4_terminateCommand(conn));
    return RBRGen4_sendBuffer(conn);
}

/**
 * \brief Remove the last response from of the response buffer.
 *
 * \param [in] conn the instrument connection
 */
static void RBRGen4_removeLastResponse(RBRGen4 *conn)
{
    if (conn->lastResponseLength <= 0 || conn->responseBufferLength == conn->lastResponseLength) {
        conn->responseBufferLength = 0;
        conn->lastResponseLength = 0;
        return;
    }

    memmove(conn->environment.response,
            conn->environment.response + conn->lastResponseLength,
            conn->responseBufferLength - conn->lastResponseLength);
    conn->responseBufferLength -= conn->lastResponseLength;
    conn->lastResponseLength = 0;
}

/**
 * \brief Read data until we find the command termination sequence or the
 *        callback indicates a timeout.
 *
 * \param [in,out] conn the instrument connection
 * \param [in] startTime when we started trying to read the command response
 * \param [in] timeout the longest to wait, in milliseconds, from \a startTime
 * \param [out] end the end of the response within the response buffer
 * \return #RBRGEN4_SUCCESS when data is successfully read
 * \return #RBRGEN4_RESPONSE_TOO_LONG when the response exceeds the buffer
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR when an unrecoverable error occurs
 */
static RBRGen4Error RBRGen4_readSingleResponse(RBRGen4 *conn, RBRGen4DateTime startTime,
                                               RBRGen4DateTime timeout, char **end)
{
    /*
    fprintf(stdout, "In RBRGen4_readSingleResponse:\n");
    */
    RBRGen4DateTime now;
    int32_t readLength;
    RBRGen4Error err = RBRGEN4_SUCCESS;
    /* Whether the line being read has already overflowed the buffer. */
    bool overflowed = false;
    while ((*end = (char *) rbr_memmem(conn->environment.response,
                                       conn->responseBufferLength,
                                       RBRGEN4_RESPONSE_TERMINATOR,
                                       RBRGEN4_RESPONSE_TERMINATOR_LEN)) == NULL) {
        /*
         * The buffer is full and the response still hasn't ended, so it is
         * longer than the buffer the caller gave us. Throw out what we have
         * and keep reading: we'll try to flush the rest of the response here,
         * so the next interaction with the instrument doesn't have to.
         */
        if (conn->responseBufferLength == conn->environment.responseCapacity) {
            /* The terminator may straddle the reset, so keep its possible
             * first bytes at the front where the next read continues it. */
            int32_t kept = RBRGEN4_RESPONSE_TERMINATOR_LEN - 1;
            memmove(conn->environment.response,
                    conn->environment.response + conn->responseBufferLength - kept,
                    kept);
            overflowed = true;
            conn->responseBufferLength = kept;
        }

        /*
         * If we're not seeing any response at all then the read callback
         * should return a character-level timeout. But if we're reading
         * characters, then we could also hit the timeout if:
         *
         * - We're not seeing the command terminator. This could happen if our
         *   connection to the instrument isn't direct and there's some
         *   improper CR-LF translation happening, or if we're not talking to
         *   an RBR instrument.
         * - We've seen the command terminator, but the only accompanying
         *   responses have been samples. If the command got lost en-route and
         *   the instrument is streaming then we'll keep seeing complete
         *   responses, but none of them will be for the command. Because the
         *   start time is supplied by the caller of RBRGen4_readResponse(),
         *   we have context for the total amount of time spent attempting to
         *   read a command response, not just how long has been spent on
         *   _this_ response.
         *
         * In either of these cases, we want to give up and indicate a timeout
         * to the caller.
         */
        if ((err = conn->environment.time(conn, &now)) != RBRGEN4_SUCCESS) {
            break;
        }

        if (now - startTime > timeout) {
            err = RBRGEN4_TIMEOUT;
            break;
        }

        /* calculate the remaining space in the responseBuffer. */
        readLength = conn->environment.responseCapacity - conn->responseBufferLength;

        /* read from the instrument and update responseBuffer length. */
        if ((err = conn->environment.read(conn,
                                          conn->environment.response + conn->responseBufferLength,
                                          &readLength)) != RBRGEN4_SUCCESS) {
            break;
        }

        conn->responseBufferLength += readLength;
    }

    if (err != RBRGEN4_SUCCESS) {
        if (overflowed && err == RBRGEN4_TIMEOUT) {
            /*
             * We hit a timeout while flushing the link after the response
             * buffer overflowed. We can't know whether the link is broken or
             * if the response is just taking a really long time to get here.
             * If it's the latter case, the next call to this function will read
             * the tail end of this response -- it's up to the caller to verify
             * that the response starts with the expected command so they can
             * discard it.
             */
            return RBRGEN4_RESPONSE_TOO_LONG;
        }
        return err;
    }

    if (overflowed) {
        /*
         * The response buffer overflowed, but we received a response terminator
         * after flushing it. Mark the tail of the oversized response,
         * terminator included, as the last response: the next read slides it
         * off and keeps whatever arrived behind it, exactly as it would for a
         * response which fit.
         */
        conn->lastResponseLength = (int32_t) ((uint8_t *) *end + RBRGEN4_RESPONSE_TERMINATOR_LEN -
                                              conn->environment.response);
        return RBRGEN4_RESPONSE_TOO_LONG;
    }

    return RBRGEN4_SUCCESS;
}

/**
 * \brief Find the beginning of a response and null-terminate the end.
 *
 * \param [in,out] conn the instrument connection
 * \param [out] beginning the beginning of the response
 * \param [in] end the end of the response
 */
static void RBRGen4_terminateResponse(RBRGen4 *conn, char **beginning, char *end)
{
    /*
     * Put a null terminator over the “\r” in the line terminator, but let
     * the length of the response include both characters of the
     * terminator. So if buffer looks like this:
     *
     * - --+---+---+---+---+----+----+---+---+---+---+---+---+---+
     * ... | = |   | o | n | \r | \n | R | e | a | d | y | : |   |
     * - --+---+---+---+---+----+----+---+---+---+---+---+---+---+
     *                        ^      ^
     *                        |      |
     *                        a      b
     *
     * ...then the null terminator will be set at index “a” (replacing the
     * “\r”), and lastResponseLength will be set to length “b” – one more
     * than what strlen() would return. This accurate reflection of the end
     * of the original terminating characters lets us easily get rid of the
     * entire old response in RBRGen4_removeLastResponse() without
     * leaving a trailing linefeed character in the buffer.
     */

    *beginning = (char *) conn->environment.response;
    *end = '\0';
    conn->lastResponseLength = end + RBRGEN4_RESPONSE_TERMINATOR_LEN - *beginning;

    /* Fast-forward leftover line termination characters. This shouldn't happen
     * in the middle of a standing conversation with an instrument, but it
     * might happen when initially establishing communication with a streaming
     * instrument: we'll be intruding on the data stream at who knows what
     * point and might encounter anything.
     *
     * In summary, it trims leading whitespace. */
    while (isspace((unsigned char) **beginning) && **beginning != '\0') {
        ++*beginning;
    }

    /* Trim leading “Ready: ” prompts in the buffer, if any. */
    while (end - *beginning >= COMMAND_PROMPT_LEN &&
           memcmp(*beginning, COMMAND_PROMPT, COMMAND_PROMPT_LEN) == 0) {
        *beginning += COMMAND_PROMPT_LEN;
    }
}

/**
 * \brief Search a string for the first byte after the next delimiter or consecutive delimiters.
 *
 * \param [in] str the null-terminated string to parse
 * \param [in] delimiter the delimiter to search for
 * \return the first byte after the next delimiter or consecutive delimiters
 * \return NULL if no such byte exists
 */
static char *seek(const char *str, char delimiter)
{
    char *token = (char *) str;
    if (token == NULL || *token == '\0') {
        return NULL;
    }

    while (*token != delimiter && *token != '\0') {
        ++token;
    }
    if (*token == '\0') {
        return NULL;
    }

    while (*token == delimiter && *token != '\0') {
        ++token;
    }
    if (*token == '\0') {
        return NULL;
    }
    return token;
}

/**
 * \brief Attempt to parse a sample from a response.
 *
 * \param [out] sample the sample; RBRGen4Sample.readings and
 *                     RBRGen4Sample.size must be set by the caller
 * \param [in] outputFormat the format of the response to parse
 * \param [in] response the response to parse
 * \return RBRGEN4_SUCCESS if the response is a sample
 * \return RBRGEN4_INVALID_PARAMETER_VALUE if the response does not
 *         follow the specified output format
 * \return RBRGEN4_CHECKSUM_ERROR if the CRC does not match
 */
static RBRGen4Error RBRGen4Sample_parse(RBRGen4Sample *sample, RBRGen4OutputFormat *outputFormat,
                                        char *response)
{
    /* Store the readings buffer and its size before zeroing it and the
     * sample struct */
    int32_t size = sample->size;
    double *readings = sample->readings;
    memset(readings, 0, (size_t) size * sizeof(*readings));
    memset(sample, 0, sizeof(RBRGen4Sample));

    /* Restore the readings buffer */
    sample->readings = readings;
    sample->size = size;

    double reading;
    sample->channelCount = 0;
    char *token = response;
    /* Serial numbers are expected to be of the form 'RBR [0-9]+' */
    if (outputFormat->sn) {
        if (memcmp(token, "RBR", 3) != 0) {
            return RBRGEN4_INVALID_PARAMETER_VALUE;
        }
        /* Explicitly check for a ' ', not PARAMETER_SEPARATOR_L4 */
        if ((token = seek(token, ' ')) == NULL) {
            return RBRGEN4_INVALID_PARAMETER_VALUE;
        }
        /* Assume the serial number is valid if it starts with a digit */
        if (!isdigit(*token)) {
            return RBRGEN4_INVALID_PARAMETER_VALUE;
        }
        if ((token = seek(token, PARAMETER_SEPARATOR_L4)) == NULL) {
            return RBRGEN4_INVALID_PARAMETER_VALUE;
        }
    }

    /* Labels' naming constraints are enforced by the instrument, not here. */
    if (outputFormat->scheduleLabel) {
        char *labelEnd = token;
        while (*labelEnd != PARAMETER_SEPARATOR_L4 && *labelEnd != '\0') {
            ++labelEnd;
        }
        size_t labelLen = (size_t) (labelEnd - token);
        if (labelLen > RBRGEN4_LABEL_NAME_MAX) {
            labelLen = RBRGEN4_LABEL_NAME_MAX;
        }
        memcpy(sample->scheduleLabel, token, labelLen);
        sample->scheduleLabel[labelLen] = '\0';

        if ((token = seek(token, PARAMETER_SEPARATOR_L4)) == NULL) {
            return RBRGEN4_INVALID_PARAMETER_VALUE;
        }
    }

    /* The timestamp is either a date/time or a bare millisecond count. */
    if (outputFormat->dateTime) {
        char *timestampEnd;
        RBR_TRY(RBRGen4DateTime_parseSampleTime(token, &sample->timestamp, &timestampEnd));
        if ((token = seek(timestampEnd, PARAMETER_SEPARATOR_L4)) == NULL) {
            return RBRGEN4_INVALID_PARAMETER_VALUE;
        }
    }

    while (token != NULL) {
        char *readingEnd = token;
        if (memcmp(token, SAMPLE_NAN, 3) == 0) {
            reading = (double) NAN;
        } else if (memcmp(token, SAMPLE_INF, 3) == 0) {
            reading = (double) INFINITY;
        } else if (memcmp(token, SAMPLE_NINF, 4) == 0) {
            reading = -(double) INFINITY;
        } else if (memcmp(token, SAMPLE_ERROR_PREFIX, SAMPLE_ERROR_PREFIX_LEN) == 0) {
            /* Uh-oh. We'll encode the error in a NaN. Filtering, etc. will
             * ignore the value and the sample formatter will output it just as
             * we received it. */
            reading = RBRGen4Reading_setError(strtol(token + SAMPLE_ERROR_PREFIX_LEN, NULL, 10));
        } else if (memcmp(token, "0x", 2) == 0) {
            if (outputFormat->crc) {
                /* Calculate the CRC. The CRC includes all characters already
                 * sent on this line, starting with the first, up to
                 * and including the last space character before the <CRC>. */
                uint16_t realCrc = strtol(token, &readingEnd, 16);
                if (readingEnd <= token + 2) {
                    /* No value was parsed. */
                    return RBRGEN4_INVALID_PARAMETER_VALUE;
                }
                uint16_t calCrc;
                calCrc = RBRGen4_calculateCrc(response, token - response);
                if (calCrc != realCrc) {
                    return RBRGEN4_CHECKSUM_ERROR;
                }
                return RBRGEN4_SUCCESS;
            } else {
                return RBRGEN4_INVALID_PARAMETER_VALUE;
            }
        } else {
            reading = strtod(token, &readingEnd);
            if (reading == 0 && token == readingEnd) {
                /* No value was parsed. */
                return RBRGEN4_INVALID_PARAMETER_VALUE;
            }
        }

        /* Readings past the caller's storage are dropped and flagged. */
        if (sample->channelCount < sample->size) {
            sample->readings[sample->channelCount++] = reading;
        } else {
            sample->readingsDropped = true;
        }
        token = seek(token, PARAMETER_SEPARATOR_L4);
    }
    return RBRGEN4_SUCCESS;
}

/**
 * \brief Check for errors or warnings in an instrument response.
 *
 * Updates RBRGen4.response as appropriate.
 *
 * \param [in] conn the instrument connection
 * \param [in] beginning the beginning of the textual response
 * \param [in] end the end of the textual response
 * \return #RBRGEN4_SUCCESS when the response is a warning or success
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument reports a hardware error
 */
RBRGen4Error RBRGen4_errorCheckResponse(RBRGen4 *conn, char *beginning, char *end)
{
    /*
     * In Gen4, errors and warnings are found at the beginning of commands and
     * are followed by a message. E.g.,
     *
     * >> enable config=pH_cal dataset=d_pHcal_20240401 storagemode=calibration
     * << WRN-408 instrument was already enabled
     *
     * >> enable config=pH_cal dataset=d_pHcal_20240401 storagemode=calibration
     * << ERR-436 instrument was already enabled with different settings
     */
    if (end - beginning >= (ERROR_PARAMETER_LEN + ERROR_NUMBER_LEN) &&
        memcmp(beginning, ERROR_PARAMETER, ERROR_PARAMETER_LEN) == 0) {
        conn->response.type = RBRGEN4_RESPONSE_ERROR;
        conn->response.error = strtol(beginning + ERROR_PARAMETER_LEN, NULL, 10);
        /* Make sure we actually have a message to go along with the error.
         * There should be one, but it's best to play safe. */
        if (end - beginning >= (ERROR_PARAMETER_LEN + ERROR_NUMBER_LEN)) {
            conn->response.response = beginning + ERROR_PARAMETER_LEN + ERROR_NUMBER_LEN;
        } else {
            conn->response.response = NULL;
        }
        return RBRGEN4_HARDWARE_ERROR;
    } else if (end - beginning >= (WARNING_PARAMETER_LEN + WARNING_NUMBER_LEN) &&
               memcmp(beginning, WARNING_PARAMETER, WARNING_PARAMETER_LEN) == 0) {
        conn->response.type = RBRGEN4_RESPONSE_WARNING;
        conn->response.error = strtol(beginning + WARNING_PARAMETER_LEN, NULL, 10);
        if (end - beginning >= (WARNING_PARAMETER_LEN + WARNING_NUMBER_LEN)) {
            conn->response.response = beginning + WARNING_PARAMETER_LEN + WARNING_NUMBER_LEN;
        } else {
            conn->response.response = NULL;
        }
        return RBRGEN4_HARDWARE_ERROR;
    }

    conn->response.type = RBRGEN4_RESPONSE_INFO;
    conn->response.error = RBRGEN4_HARDWARE_ERROR_NONE;
    conn->response.response = beginning;

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_readResponse(RBRGen4 *conn, bool breakOnSample, RBRGen4Sample *sample,
                                  RBRGen4DateTime startTime, RBRGen4DateTime timeout)
{
    /* Reset the response state. */
    conn->response.type = RBRGEN4_RESPONSE_UNKNOWN_TYPE;
    conn->response.error = RBRGEN4_HARDWARE_ERROR_NONE;
    conn->response.response = NULL;

    RBRGen4Sample *sampleTarget;
    if (sample == NULL) {
        sampleTarget = conn->environment.sampleBuffer;
    } else {
        sampleTarget = sample;
    }

    /* Skip over streaming samples until we find a real command response, or
     * until we exceed the timeout. */
    while (true) {
        RBRGen4_removeLastResponse(conn);
        char *beginning;
        char *end;
        RBR_TRY(RBRGen4_readSingleResponse(conn, startTime, timeout, &end));

        RBRGen4_terminateResponse(conn, &beginning, end);

        if (sampleTarget != NULL &&
            RBRGen4Sample_parse(sampleTarget, &conn->outputFormat, beginning) == RBRGEN4_SUCCESS) {
            if (conn->environment.sample != NULL && sample == NULL) {
                RBR_TRY(conn->environment.sample(conn, sampleTarget));
            }
            if (breakOnSample) {
                return RBRGEN4_SAMPLE;
            }
        } else {
            return RBRGen4_errorCheckResponse(conn, beginning, end);
        }
    }
}

void RBRGen4_parseResponse(RBRGen4 *conn, char **command, RBRGen4ResponseParameter *parameter)
{
    /*
     * If this has not been run, the command string is null, so the command end
     * seeks forward from the start of the response until finding the parameter
     * separator (' ') or the end of the string.
     */
    if (*command == NULL) {
        memset(parameter, 0, sizeof(RBRGen4ResponseParameter));

        *command = conn->response.response;
        char *commandEnd = *command;

        while (true) {
            switch (*commandEnd) {
            case '\0':
                parameter->key = NULL;
                parameter->value = NULL;
                parameter->nextKey = NULL;
                return;
            case PARAMETER_SEPARATOR_L4:
                goto foundCommandEnd;
            default:
                ++commandEnd;
            }
        }
    foundCommandEnd:
        /*
         * Finding the parameter separator (' ') indicates the start of a key.
         * Move the next key past the command_end pointer.
         */
        *commandEnd = '\0';
        parameter->nextKey = commandEnd + 1;
    }

    if (parameter->nextKey == NULL) {
        parameter->key = NULL;
        parameter->value = NULL;
        parameter->nextKey = NULL;
        return;
    }

    /*
     * Some commands (e.g., channel, regime) take an index parameter and return
     * it with the response. E.g.,
     *
     *     >> channel conductivity_00
     *     << channel conductivity_00 type=cond00 address=32 settlingtime=50 readtime=260
     * guardtime=20 userunits=mS/cm derived=off grouplist=none sensor=none
     *
     * This gets parsed into:
     *
     *     << channel\0conductivity_00\0type\0cond00\0address=32 settlingtime=50 readtime=260
     * guardtime=20 userunits=mS/cm derived=off grouplist=none sensor=none\0 ^command ^indexValue
     * ^key  ^value  ^nextKey ^instrument->response.response
     *
     * Next, the value seeks forward from the key pointer until finding the
     * value separator ('='), saving the position of the rightmost parameter
     * separator (' '), which separates the end of the index parameter from the
     * beginning of the parameter key. Because this can theoretically happen both
     * after the initial command word and after the array member separator,
     * we'll check for this each time we parse a parameter.
     */
    parameter->key = parameter->nextKey;
    char *previousSpace = NULL;
    parameter->value = parameter->key;

    while (true) {
        if (*parameter->value == '\0') {
            /*
             * Nothing left to do if value reaches the end without finding the
             * value separator.
             */
            parameter->nextKey = NULL;
            return;
        } else if (*parameter->value == PARAMETER_VALUE_SEPARATOR_L4) {
            /* Null-terminate the value if found. */
            *parameter->value = '\0';
            ++parameter->value;
            break;
        } else if (*parameter->value == PARAMETER_SEPARATOR_L4) {
            previousSpace = parameter->value;
        }
        ++parameter->value;
    }

    /*
     * Finding the parameter separator between the key and the value indicates
     * an index value.
     * Set the index value to the current key and move the key past the separator.
     */
    if (previousSpace != NULL) {
        *previousSpace = '\0';
        ++parameter->index;
        parameter->indexValue = parameter->key;
        parameter->key = previousSpace + 1;
    }

    /*
     * L4 uses the pipe character as the separator for parameters returning
     * lists. E.g.,
     *
     *     >> channel
     *     << channel count=2 list=temperature_00|pressure_00
     *
     * The whole pipe-joined string is a single value; parsing the list
     * parameter gets to:
     *     << channel\0count\02\0list\0temperature_00|pressure_00\0
     *        ^command           ^key  ^value
     *
     * RBRGen4_splitListValue() then walks the values within it.
     *
     * At last, the next key seeks forward from the value until the parameter
     * separator (' ') or the end of the string.
     */
    parameter->nextKey = parameter->value;
    while (true) {
        if (*parameter->nextKey == '\0') {
            /*
             * Nothing left to do if next key reaches the end without finding
             * the parameter separator.
             */
            parameter->nextKey = NULL;
            return;
        } else if (*parameter->nextKey == PARAMETER_SEPARATOR_L4) {
            /* Null-terminate the next key. */
            *parameter->nextKey = '\0';
            ++parameter->nextKey;
            break;
        }
        ++parameter->nextKey;
    }
}

RBRGen4Error RBRGen4_converse(RBRGen4 *conn, const char *command, ...)
{
    RBRGen4Error err;
    va_list format;
    va_start(format, command);
    RBRGen4_beginCommand(conn);
    err = RBRGen4_vAppendCommand(conn, command, format);
    va_end(format);
    if (err != RBRGEN4_SUCCESS) {
        return err;
    }

    return RBRGen4_converseBuffer(conn);
}

RBRGen4Error RBRGen4_converseBuffer(RBRGen4 *conn)
{
    RBRGen4Error err;

    RBR_TRY(RBRGen4_terminateCommand(conn));

    /* Keep firing off the command and looking for a response until we find one
     * which matches. */
    bool retry;
    /* Whether a line too long for the response buffer was met while waiting
     * for the reply. */
    bool responseTooLong = false;
    do {
        /* The retry flag might be set on by the “ERR-102 invalid command” error
         * handling below. It needs to be reset every time we send the command
         * so that we don't accidentally retry infinitely. */
        retry = false;
        responseTooLong = false;

        /* The reads touch only the response buffer, so the command buffer
         * still holds the command when it has to be sent again. */
        err = RBRGen4_sendBuffer(conn);
        if (err != RBRGEN4_SUCCESS) {
            break;
        }

        /* We can detect whether we've got the expected response based whether
         * its command word matches what we sent. To match that, we'll find the
         * first word of the command. */
        int32_t commandLength = 0;
        while (!isspace(conn->environment.command[commandLength]) &&
               conn->environment.command[commandLength] != '\0') {
            ++commandLength;
        }

        /* The command timeout bounds the whole wait for the reply, however
         * many unrelated lines are skipped on the way. */
        RBRGen4DateTime startTime;
        err = conn->environment.time(conn, &startTime);
        if (err != RBRGEN4_SUCCESS) {
            break;
        }

        do {
            err = RBRGen4_readResponse(conn, false, NULL, startTime, conn->commandTimeout);
            if (err == RBRGEN4_RESPONSE_TOO_LONG) {
                /* The oversized line may have been a streamed sample rather
                 * than the reply, so keep waiting. If the reply never comes
                 * the oversized line most likely was it, and that is what
                 * gets reported. */
                responseTooLong = true;
                continue;
            }
            /*
             * There are a few reasons the instrument might generate an “ERR-102
             * invalid command” error, and we can make the user's life a bit
             * easier by handling it.
             *
             * - If the command in the error message matches the command we
             *   sent, then the error message is legitimate and should be
             *   forwarded to the user. This shouldn't happen for any commands
             *   generated by library functions, but it could happen if the
             *   user invokes RBRGen4_converse() directly.
             * - If the command in the error message ends with the command we
             *   sent, then there was likely garbage sitting in the
             *   instrument's receive buffer when we sent the command. This can
             *   happen as serial cables are connected/disconnected, or if
             *   we're not the only thing talking to the instrument and it
             *   leaves garbage behind, or for any other number of reasons. In
             *   this case, we'll resend our command.
             * - Otherwise, the error message wasn't related to this command at
             *   all and should be ignored.
             */
            if (err == RBRGEN4_HARDWARE_ERROR &&
                (conn->response.error == RBRGEN4_HARDWARE_ERROR_INVALID_COMMAND)) {
                /* We have no message to inspect, so we can only assume the
                 * error is legitimate and pass it along to the user. */
                if (conn->response.response == NULL) {
                    break;
                }

                /* The error message indicates what the invalid command was.
                 * It's enclosed in single quotes, so we can look for those to
                 * find its bounds. */
                char *invalidCommand = strchr(conn->response.response, '\'');
                if (invalidCommand == NULL) {
                    break;
                }
                ++invalidCommand;

                char *invalidCommandEnd = strchr(invalidCommand, '\'');
                if (invalidCommandEnd == NULL) {
                    break;
                }

                int32_t invalidCommandLength = invalidCommandEnd - invalidCommand;

                /* The command was actually invalid. Whoops. */
                if (invalidCommandLength == commandLength &&
                    memcmp(invalidCommand, conn->environment.command, commandLength) == 0) {
                    break;
                }
                /* We were on the right track, but there was garbage in the
                 * buffer. Retry. */
                else if (invalidCommandLength > commandLength &&
                         memcmp(invalidCommand + invalidCommandLength - commandLength,
                                conn->environment.command,
                                commandLength) == 0) {
                    retry = true;
                    break;
                }
                /* Not our garbage, not our problem. We won't retry, but we'll
                 * keep trying to read a response. */
                else {
                    continue;
                }
            } else if (err != RBRGEN4_SUCCESS) {
                break;
            }
        } while (
            (conn->response.response == NULL || strncmp(conn->response.response,
                                                        (const char *) conn->environment.command,
                                                        commandLength) != 0));
        if (err == RBRGEN4_TIMEOUT && responseTooLong) {
            err = RBRGEN4_RESPONSE_TOO_LONG;
        }
    } while (retry);

    return err;
}

RBRGen4Error RBRGen4_getBool(RBRGen4 *conn, const char *command, const char *parameter, bool *value)
{
    *value = false;

    RBR_TRY(RBRGen4_converse(conn, "%s %s", command, parameter));

    char *responseCommand = NULL;
    RBRGen4ResponseParameter responseParameter;
    do {
        RBRGen4_parseResponse(conn, &responseCommand, &responseParameter);

        if (responseParameter.key == NULL) {
            break;
        } else if (strcmp(responseParameter.key, parameter) != 0) {
            continue;
        }

        *value = (strcmp(responseParameter.value, "on") == 0);
    } while (true);

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_getFloat(RBRGen4 *conn, const char *command, const char *parameter,
                              float *value)
{
    *value = NAN;

    RBR_TRY(RBRGen4_converse(conn, "%s %s", command, parameter));

    char *responseCommand = NULL;
    RBRGen4ResponseParameter responseParameter;
    while (true) {
        RBRGen4_parseResponse(conn, &responseCommand, &responseParameter);

        if (responseParameter.key == NULL) {
            break;
        } else if (strcmp(responseParameter.key, parameter) != 0) {
            continue;
        }

        *value = strtod(responseParameter.value, NULL);
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_getInt(RBRGen4 *conn, const char *command, const char *parameter,
                            int32_t *value)
{
    *value = 0;

    RBR_TRY(RBRGen4_converse(conn, "%s %s", command, parameter));

    char *responseCommand = NULL;
    RBRGen4ResponseParameter responseParameter;
    while (true) {
        RBRGen4_parseResponse(conn, &responseCommand, &responseParameter);

        if (responseParameter.key == NULL || responseParameter.value == NULL) {
            break;
        } else if (strcmp(responseParameter.key, parameter) != 0) {
            continue;
        }

        *value = strtol(responseParameter.value, NULL, 10);
    }

    return RBRGEN4_SUCCESS;
}

/** \brief Ensure localTimeOffset is initialized. */
static inline void RBRGen4DateTime_initializeOffset(void)
{
    if (localTimeOffset == OFFSET_UNINITIALIZED) {
        struct tm instrumentMinTimestamp = {
            .tm_year = 100,
            .tm_mon = 0,
            .tm_mday = 1,
            .tm_hour = 0,
            .tm_min = 0,
            .tm_sec = 0,
        };
        localTimeOffset =
            RBRGEN4_DATETIME_MIN - ((RBRGen4DateTime) mktime(&instrumentMinTimestamp) * 1000);
    }
}

/**
 * \brief Parse a broken-down time into a millisecond timestamp.
 *
 * Any millisecond value should already be present in \a timestamp.
 *
 * \param [in] split the broken-down time
 * \param [in,out] timestamp the timestamp
 * \return #RBRGEN4_SUCCESS when the timestamp is successfully parsed
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the time is invalid
 */
static RBRGen4Error RBRGen4DateTime_parse(struct tm *split, RBRGen4DateTime *timestamp)
{
    /* struct tm/mktime() expects years to be counted from 1900... */
    split->tm_year -= 1900;
    /* ...and months to be 0-based. */
    split->tm_mon -= 1;

    /* Sanity check. */
    if (split->tm_year < 100 || split->tm_year >= 200 || split->tm_mon > 11 ||
        split->tm_mday > 31 || split->tm_hour > 23 || split->tm_min > 59 ||
        split->tm_sec > 59 /* Instrument doesn't know about leap seconds. */
        || *timestamp > 999) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    RBRGen4DateTime_initializeOffset();

    *timestamp += (((RBRGen4DateTime) mktime(split)) * 1000) + localTimeOffset;

    if (*timestamp < RBRGEN4_DATETIME_MIN || *timestamp > RBRGEN4_DATETIME_MAX) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4DateTime_parseSampleTime(const char *s, RBRGen4DateTime *timestamp, char **end)
{
    *timestamp = 0;
    if (end != NULL) {
        *end = NULL;
    }

    int32_t timestampLength = 0;
    struct tm split = {0};
    int milliseconds;
    int64_t elapsed;

    if (sscanf(s,
               RBRGen4DateTime_sampleScanFormat,
               &split.tm_year,
               &split.tm_mon,
               &split.tm_mday,
               &split.tm_hour,
               &split.tm_min,
               &split.tm_sec,
               &milliseconds,
               &timestampLength) == 7) {
        *timestamp = milliseconds;
        RBR_TRY(RBRGen4DateTime_parse(&split, timestamp));
    }
    /* A bare count of milliseconds must make up the whole token so that a
     * reading is never mistaken for one. */
    else if (sscanf(s, RBRGen4DateTime_sampleMillisecondsScanFormat, &elapsed, &timestampLength) ==
                 1 &&
             timestampLength > 0 &&
             (s[timestampLength] == PARAMETER_SEPARATOR_L4 || s[timestampLength] == '\0')) {
        *timestamp = elapsed;
    } else {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    if (end != NULL) {
        *end = (char *) s + timestampLength;
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4DateTime_parseScheduleTime(const char *s, RBRGen4DateTime *timestamp,
                                               char **end)
{
    *timestamp = 0;
    if (end != NULL) {
        *end = NULL;
    }

    int32_t timestampLength;
    struct tm split = {0};
    if (sscanf(s,
               RBRGen4DateTime_scheduleScanFormat,
               &split.tm_year,
               &split.tm_mon,
               &split.tm_mday,
               &split.tm_hour,
               &split.tm_min,
               &split.tm_sec,
               &timestampLength) < 6) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    RBR_TRY(RBRGen4DateTime_parse(&split, timestamp));
    if (end != NULL) {
        *end = (char *) s + timestampLength;
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_appendDateTime(RBRGen4 *conn, RBRGen4DateTime timestamp)
{
    time_t t = timestamp / 1000;
    struct tm *split = gmtime(&t);
    return RBRGen4_appendCommand(conn,
                                 RBRGen4DateTime_scheduleFormat,
                                 split->tm_year + 1900,
                                 split->tm_mon + 1,
                                 split->tm_mday,
                                 split->tm_hour,
                                 split->tm_min,
                                 split->tm_sec);
}

char *RBRGen4_splitListValue(char *value)
{
    char *nextValue = strchr(value, ARRAY_SEPARATOR_L4);

    if (nextValue != NULL) {
        *nextValue = '\0';
        ++nextValue;
    }

    return nextValue;
}

RBRGen4Error RBRGen4_formatLabelList(char *value, int32_t size, const RBRGen4LabelList *labelList)
{
    if (labelList == NULL || labelList->len < 0 || labelList->len > labelList->size) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    int32_t length = 0;

    if (labelList->len == 0) {
        length = snprintf(value, size, RBRGEN4_EMPTY_LIST);
        return length > 0 && length < size ? RBRGEN4_SUCCESS : RBRGEN4_COMMAND_TOO_LONG;
    }

    for (int32_t i = 0; i < labelList->len; ++i) {
        if (labelList->labels[i][0] == '\0') {
            return RBRGEN4_INVALID_PARAMETER_VALUE;
        }

        int32_t written = snprintf(
            value + length, size - length, "%s%s", i == 0 ? "" : "|", labelList->labels[i]);

        if (written < 0 || length + written >= size) {
            return RBRGEN4_COMMAND_TOO_LONG;
        }

        length += written;
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_copyLabelList(RBRGen4LabelList *labelList, char *value)
{
    labelList->len = 0;

    if (strcmp(value, RBRGEN4_EMPTY_LIST) == 0) {
        return RBRGEN4_SUCCESS;
    }

    while (value != NULL) {
        if (labelList->len >= labelList->size) {
            return RBRGEN4_TRUNCATED;
        }

        char *nextValue = RBRGen4_splitListValue(value);

        snprintf(labelList->labels[labelList->len],
                 sizeof(labelList->labels[labelList->len]),
                 "%s",
                 value);
        labelList->len++;

        value = nextValue;
    }

    return RBRGEN4_SUCCESS;
}
