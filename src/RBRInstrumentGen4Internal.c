/**
 * \file RBRInstrumentGen4Internal.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
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

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Internal.h"
#include "RBRInstrumentGen4Memory.h"
#include "RBRInstrumentGen4Internal.h"
#include "RBRInstrumentGen4Streaming.h"

/** \brief 10-second command timeout. */
#define COMMAND_TIMEOUT (10 * 1000)

/* At https://docs.rbr-global.com/L3commandreference/introduction/command-
 * processing-and-timeouts/timeouts-output-blanking-and-power-saving, the
 * command reference suggests using a carriage return (`\r`) as the wake
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
#define WAKE_COMMAND RBRINSTRUMENTGEN4_RESPONSE_TERMINATOR
/** \brief The length of the wake sequence. */
#define WAKE_COMMAND_LEN RBRINSTRUMENTGEN4_RESPONSE_TERMINATOR_LEN
/**
 * \brief How long to wait after the wake sequence.
 *
 * 50ms is far longer than it will take to generate any regular command, so
 * this should be enough margin for any transport which transmits based only on
 * idle time.
 */
#define WAKE_COMMAND_WAIT 50

#define COMMAND_PROMPT "Ready: "
#define COMMAND_PROMPT_LEN 7

#define ARRAY_SEPARATOR_L4 '|'
#define PARAMETER_SEPARATOR_L4 ' '
#define PARAMETER_VALUE_SEPARATOR_L4 '='

#define OFFSET_UNINITIALIZED (-1)

#define ERROR_PARAMETER "ERR-"
#define ERROR_PARAMETER_LEN ((long) (sizeof(ERROR_PARAMETER) - 1))
#define ERROR_NUMBER_LEN 3

#define WARNING_PARAMETER "WRN-"
#define WARNING_PARAMETER_LEN ((long) (sizeof(WARNING_PARAMETER) - 1))
#define WARNING_NUMBER_LEN 3

#define SAMPLE_NAN "nan"
#define SAMPLE_INF "inf"
#define SAMPLE_NINF "-inf"
#define SAMPLE_UNCAL "###"
#define SAMPLE_ERROR_PREFIX "Error-"
#define SAMPLE_ERROR_PREFIX_LEN ((long) (sizeof(SAMPLE_ERROR_PREFIX) - 1))

static const char *RBRInstrumentGen4DateTime_sampleFormat = "%04d-%02d-%02d %02d:%02d:%02d.%03d";

static const char *RBRInstrumentGen4DateTime_sampleScanFormat = "%04d-%02d-%02d %02d:%02d:%02d.%03d%n";

static const char *RBRInstrumentGen4DateTime_scheduleFormat = "%04d%02d%02d%02d%02d%02d";

static const char *RBRInstrumentGen4DateTime_scheduleScanFormat = "%04d%02d%02d%02d%02d%02d%n";

static RBRInstrumentGen4DateTime localTimeOffset = OFFSET_UNINITIALIZED;

/**
 * \brief Like strstr, but for memory.
 */
void *rbr_memmem(void *ptr1, size_t num1, const void *ptr2, size_t num2)
{
    if (num2 > num1)
    {
        return NULL;
    }

    for (size_t offset = 0; offset <= num1 - num2; ++offset)
    {
        if (memcmp((uint8_t *) ptr1 + offset, ptr2, num2) == 0)
        {
            return (uint8_t *) ptr1 + offset;
        }
    }

    return NULL;
}

/**
 * \brief Wake the instrument from sleep, if necessary.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the instrument has been woken
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR when an unrecoverable error occurs
 */
static RBRInstrumentGen4Error RBRInstrumentGen4_wake(const RBRInstrumentGen4 *instrument)
{
    RBRInstrumentGen4DateTime now;
    RBR_TRY(instrument->callbacks.time(instrument, &now));

    if (instrument->lastActivityTime >= 0 && now - instrument->lastActivityTime < COMMAND_TIMEOUT)
    {
        return RBRINSTRUMENTGEN4_SUCCESS;
    }

    /* Send the wake sequence twice to make sure it gets noticed. */
    for (int pass = 0; pass < 2; ++pass)
    {
        RBR_TRY(instrument->callbacks.write(instrument,
                                            WAKE_COMMAND,
                                            WAKE_COMMAND_LEN));
        RBR_TRY(instrument->callbacks.sleep(instrument, WAKE_COMMAND_WAIT));
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_sendBuffer(RBRInstrumentGen4 *instrument)
{
    /* Wake the instrument if necessary. */
    RBR_TRY(RBRInstrumentGen4_wake(instrument));

    if (instrument->commandBufferLength > RBRINSTRUMENTGEN4_COMMAND_BUFFER_MAX)
    {
        instrument->commandBufferLength = RBRINSTRUMENTGEN4_COMMAND_BUFFER_MAX;
    }

    /* Send the command to the instrument. */
    RBR_TRY(instrument->callbacks.write(instrument,
                                        instrument->commandBuffer,
                                        instrument->commandBufferLength));
    RBR_TRY(instrument->callbacks.time(instrument,
                                       &instrument->lastActivityTime));
    return RBRINSTRUMENTGEN4_SUCCESS;
}

static RBRInstrumentGen4Error RBRInstrumentGen4_vSendCommand(RBRInstrumentGen4 *instrument,
                                                             const char *command,
                                                             va_list format)
{
    /* Prepare the command. */

    instrument->commandBufferLength = vsnprintf(
        (char *) instrument->commandBuffer,
        sizeof(instrument->commandBuffer),
        command,
        format);

    /* Debug print
    fprintf(stdout, ">>%s\n", (char *) instrument->commandBuffer);
    */

    /* Make sure we're within buffer bounds. This is a greater-or-equal check,
     * not just a greater-than check, because vsnprintf doesn't include the
     * null terminator in its return value. The longest value vsnprintf can
     * write is RBRINSTRUMENTGEN4_COMMAND_BUFFER_MAX - 1 bytes. */
    if (instrument->commandBufferLength >= RBRINSTRUMENTGEN4_COMMAND_BUFFER_MAX)
    {
        instrument->commandBufferLength = RBRINSTRUMENTGEN4_COMMAND_BUFFER_MAX;
        return RBRINSTRUMENTGEN4_BUFFER_TOO_SMALL;
    }

    /* Make sure the command is LF-terminated. */
    if (instrument->commandBufferLength < RBRINSTRUMENTGEN4_SEND_COMMAND_TERMINATOR_LEN || memcmp(instrument->commandBuffer + instrument->commandBufferLength - RBRINSTRUMENTGEN4_SEND_COMMAND_TERMINATOR_LEN,
                                                                                                  RBRINSTRUMENTGEN4_SEND_COMMAND_TERMINATOR,
                                                                                                  RBRINSTRUMENTGEN4_SEND_COMMAND_TERMINATOR_LEN) != 0)
    {
        /* It isn't. Make sure there's room before adding it. */
        if (instrument->commandBufferLength + RBRINSTRUMENTGEN4_SEND_COMMAND_TERMINATOR_LEN > RBRINSTRUMENTGEN4_COMMAND_BUFFER_MAX)
        {
            return RBRINSTRUMENTGEN4_BUFFER_TOO_SMALL;
        }

        memcpy(instrument->commandBuffer + instrument->commandBufferLength,
               RBRINSTRUMENTGEN4_SEND_COMMAND_TERMINATOR,
               RBRINSTRUMENTGEN4_SEND_COMMAND_TERMINATOR_LEN);
        instrument->commandBufferLength +=
            RBRINSTRUMENTGEN4_SEND_COMMAND_TERMINATOR_LEN;
    }
    return RBRInstrumentGen4_sendBuffer(instrument);
}

RBRInstrumentGen4Error RBRInstrumentGen4_sendCommand(RBRInstrumentGen4 *instrument,
                                                     const char *command,
                                                     ...)
{
    RBRInstrumentGen4Error err;
    va_list format;
    va_start(format, command);
    err = RBRInstrumentGen4_vSendCommand(instrument, command, format);
    va_end(format);
    return err;
}

/**
 * \brief Remove the last response from of the response buffer.
 *
 * \param [in] instrument the instrument connection
 */
static void RBRInstrumentGen4_removeLastResponse(RBRInstrumentGen4 *instrument)
{
    if (instrument->lastResponseLength <= 0 || instrument->responseBufferLength == instrument->lastResponseLength)
    {
        instrument->responseBufferLength = 0;
        instrument->lastResponseLength = 0;
        return;
    }

    memmove(instrument->responseBuffer,
            instrument->responseBuffer + instrument->lastResponseLength,
            instrument->responseBufferLength - instrument->lastResponseLength);
    instrument->responseBufferLength -= instrument->lastResponseLength;
    instrument->lastResponseLength = 0;
}

/**
 * \brief Read data until we find the command termination sequence or the
 *        callback indicates a timeout.
 *
 * \param [in,out] instrument the instrument connection
 * \param [in] startTime when we started trying to read the command response
 * \param [out] end the end of the response within the response buffer
 * \return #RBRINSTRUMENTGEN4_SUCCESS when data is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR when an unrecoverable error occurs
 */
static RBRInstrumentGen4Error RBRInstrumentGen4_readSingleResponse(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4DateTime startTime,
    char **end)
{
    /*
    fprintf(stdout, "In RBRInstrumentGen4_readSingleResponse:\n");
    */
    RBRInstrumentGen4DateTime now;
    int32_t readLength;
    while ((*end = (char *) rbr_memmem(
                instrument->responseBuffer,
                instrument->responseBufferLength,
                RBRINSTRUMENTGEN4_RESPONSE_TERMINATOR,
                RBRINSTRUMENTGEN4_RESPONSE_TERMINATOR_LEN)) == NULL)
    {
        /*
         * If we're not seeing any response at all then the read callback
         * should return a character-level timeout. But if we're reading
         * characters, then we could also hit the command timeout if:
         *
         * - We're not seeing the command terminator. This could happen if our
         *   connection to the instrument isn't direct and there's some
         *   improper CR-LF translation happening, or if we're not talking to
         *   an RBR instrument.
         * - We've seen the command terminator, but the only accompanying
         *   responses have been samples. If the command got lost en-route and
         *   the instrument is streaming then we'll keep seeing complete
         *   responses, but none of them will be for the command. Because the
         *   start time is set in RBRInstrumentGen4_readResponse(), we have context
         *   for the total amount of time spent attempting to read a command
         *   response, not just how long has been spent on _this_ response.
         *
         * In either of these cases, we want to give up and indicate a timeout
         * to the caller.
         */
        RBR_TRY(instrument->callbacks.time(instrument, &now));

        /*
        fprintf(stdout,
                "now: %ld, startTime: %ld, now - startTime: %ld, commandTimeout: %ld\n",
                now,
                startTime,
                now - startTime,
                instrument->commandTimeout);
        */
        if (now - startTime > instrument->commandTimeout)
        {
            return RBRINSTRUMENTGEN4_TIMEOUT;
        }

        /* If the buffer is full but doesn't contain a terminator, there's
         * not much we can do about it: throw out the buffer, then keep
         * trying to fill it. */
        if (instrument->responseBufferLength == RBRINSTRUMENTGEN4_RESPONSE_BUFFER_MAX)
        {
            instrument->responseBufferLength = 0;
            instrument->lastResponseLength = 0;
        }

        /* calculate the remaining space in the responseBuffer. */
        readLength = RBRINSTRUMENTGEN4_RESPONSE_BUFFER_MAX - instrument->responseBufferLength;

        /* read from the instrument and update responseBuffer length. */
        RBR_TRY(instrument->callbacks.read(
            instrument,
            instrument->responseBuffer + instrument->responseBufferLength,
            &readLength));

        instrument->responseBufferLength += readLength;
    }

    /*
    fprintf(stdout, "responseBuffer (%dB):\n", instrument->responseBufferLength);
    fprintf(stdout, "<<");
    for (int32_t i = 0; i < instrument->responseBufferLength; i++)
    {
        fprintf(stdout, "%c", instrument->responseBuffer[i]);
    }
    fprintf(stdout, "\n");
    for (int32_t i = 0; i < instrument->responseBufferLength; i++)
    {
        fprintf(stdout, "%x ", instrument->responseBuffer[i]);
    }
    fprintf(stdout, "\n");
    */

    return RBRINSTRUMENTGEN4_SUCCESS;
}

/**
 * \brief Find the beginning of a response and null-terminate the end.
 *
 * \param [in,out] instrument the instrument connection
 * \param [out] beginning the beginning of the response
 * \param [in] end the end of the response
 */
static void RBRInstrumentGen4_terminateResponse(
    RBRInstrumentGen4 *instrument,
    char **beginning,
    char *end)
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
     * entire old response in RBRInstrumentGen4_removeLastResponse() without
     * leaving a trailing linefeed character in the buffer.
     */

    *beginning = (char *) instrument->responseBuffer;
    *end = '\0';
    instrument->lastResponseLength =
        end + RBRINSTRUMENTGEN4_RESPONSE_TERMINATOR_LEN - *beginning;

    /* Fast-forward leftover line termination characters. This shouldn't happen
     * in the middle of a standing conversation with an instrument, but it
     * might happen when initially establishing communication with a streaming
     * instrument: we'll be intruding on the data stream at who knows what
     * point and might encounter anything.
     *
     * In summary, it trims leading whitespace. */
    while (isspace((unsigned char) **beginning) && **beginning != '\0')
    {
        ++*beginning;
    }

    /* Trim leading “Ready: ” prompts in the buffer, if any. */
    while (end - *beginning >= COMMAND_PROMPT_LEN && memcmp(*beginning,
                                                            COMMAND_PROMPT,
                                                            COMMAND_PROMPT_LEN) == 0)
    {
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
char *seek(const char *str, char delimiter)
{
    char *token = (char *)str;
    if (token == NULL
        || *token == '\0')
    {
        return NULL;
    }

    while (*token != delimiter
           && *token != '\0')
    {
        ++token;
    }
    if (*token == '\0')
    {
        return NULL;
    }

    while (*token == delimiter
           && *token != '\0')
    {
        ++token;
    }
    if (*token == '\0')
    {
        return NULL;
    }
    return token;
}

/**
 * \brief Attempt to parse a sample from a response.
 *
 * \param [out] sample the sample
 * \param [in] outputFormat the format of the response to parse
 * \param [in] response the response to parse
 * \return RBRINSTRUMENTGEN4_SUCCESS if the response is a sample
 * \return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE if the response does not
 *         follow the specified output format
 */

static RBRInstrumentGen4Error RBRInstrumentGen4Sample_parse(
    RBRInstrumentGen4Sample *sample,
    RBRInstrumentGen4Outputformat *outputFormat,
    char *response)
{
    memset(sample, 0, sizeof(RBRInstrumentGen4Sample));

    /*
    All samples scenario:
    2023-09-10 11:24:14.125 38.6671142e+000 22.0217124e+000
    <RBR 999999 >polling 2023-09-10 11:24:14.125 38.6671142e+000 22.0217124e+000< 0xABCD>
    <RBR 999999 ><scheduleLabel >2023-09-10 11:24:14.125 38.6671142e+000 22.0217124e+000< 0xABCD>
    */

    double reading;
    sample->channelCount = 0;
    if (strchr(response, PARAMETER_SEPARATOR_L4) != NULL)
    {
        char *token = response;
        if ((*outputFormat) & RBRINSTRUMENTGEN4_OUTPUTFORMAT_SERIAL)
        {
            if (memcmp(token, "RBR", 3) != 0)
            {
                return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
            }
            if ((token = seek(token, PARAMETER_SEPARATOR_L4)) == NULL)
            {
                return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
            }
            if (!isdigit(*token))
            {
                return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
            }
            if ((token = seek(token, PARAMETER_SEPARATOR_L4)) == NULL)
            {
                return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
            }
        }

        if ((*outputFormat) & RBRINSTRUMENTGEN4_OUTPUTFORMAT_SCHEDULELABEL)
        {
            if ((token = seek(token, PARAMETER_SEPARATOR_L4)) == NULL)
            {
                return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
            }
        }

        if ((*outputFormat) & RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP)
        {
            char* timestamp_end;
            RBR_TRY(RBRInstrumentGen4DateTime_parseSampleTime(token,
                                                        &sample->timestamp,
                                                        &timestamp_end));
            token = timestamp_end;
        }

        while ((token = seek(token, PARAMETER_SEPARATOR_L4)) != NULL
               && sample->channelCount < RBRINSTRUMENTGEN4_CHANNEL_MAX)
        {
            char *reading_end = token;
            /*
            if (memcmp(token,
                       RBRINSTRUMENTGEN4_RESPONSE_TERMINATOR,
                       RBRINSTRUMENTGEN4_RESPONSE_TERMINATOR_LEN))
            {
                return RBRINSTRUMENTGEN4_SUCCESS;
            }
            else */
            if (memcmp(token, SAMPLE_NAN, 3) == 0)
            {
                reading = NAN;
            }
            else if (memcmp(token, SAMPLE_INF, 3) == 0)
            {
                reading = INFINITY;
            }
            else if (memcmp(token, SAMPLE_NINF, 4) == 0)
            {
                reading = -INFINITY;
            }
            else if (memcmp(token, SAMPLE_UNCAL, 3) == 0)
            {
                reading = RBRInstrumentGen4Reading_setError(
                    RBRINSTRUMENTGEN4_READING_FLAG_UNCALIBRATED,
                    0);
            }
            else if (memcmp(token,
                            SAMPLE_ERROR_PREFIX,
                            SAMPLE_ERROR_PREFIX_LEN) == 0)
            {
                /* Uh-oh. We'll encode the error in a NaN. Filtering, etc. will
                    * ignore the value and the sample formatter will output it just as
                    * we received it. */
                reading = RBRInstrumentGen4Reading_setError(
                    RBRINSTRUMENTGEN4_READING_FLAG_ERROR,
                    strtol(token + SAMPLE_ERROR_PREFIX_LEN,
                           NULL,
                           10));
            }
            else if (memcmp(token, "0x", 2) == 0)
            {
                if ((*outputFormat) & RBRINSTRUMENTGEN4_OUTPUTFORMAT_CRC)
                {
                    /*
                    * calculate CRC.
                    * The CRC includes all characters already sent on this line, starting with the first, up to
                    * and including the last space character before the <CRC>.
                    */
                    uint16_t realCrc = strtol(token, &reading_end, 16);
                    if (reading == 0 && token == reading_end)
                    {
                        /* No value was parsed. */
                        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
                    }
                    uint16_t calCrc;
                    calCrc = calculateCrc(response, token - response);
                    if (calCrc != realCrc)
                    {
                        return RBRINSTRUMENTGEN4_CHECKSUM_ERROR;
                    }
                    return RBRINSTRUMENTGEN4_SUCCESS;
                }
                else
                {
                    return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
                }
            }
            else
            {
                reading = strtod(token, &reading_end);
                if (reading == 0 && token == reading_end)
                {
                    /* No value was parsed. */
                    return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
                }
            }

            sample->readings[sample->channelCount++] = reading;
        }
        return RBRINSTRUMENTGEN4_SUCCESS;
    }
    else 
    {
        /* No spaces in the response. Not likely but just in case. */
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }
}

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
    char *end)
{
    /*
     * In L3.5 and L4, errors and warnings are found at the beginning of
     * commands and are followed by a message. E.g.,
     *
     * >> enable config=pH_cal dataset=d_pHcal_20240401 storagemode=calibration
     * << WRN-408 instrument was already enabled
     *
     * >> enable config=pH_cal dataset=d_pHcal_20240401 storagemode=calibration
     * << ERR-128 instrument was already enabled with different settings
     */
    if (end - beginning >= (ERROR_PARAMETER_LEN + ERROR_NUMBER_LEN)
        && memcmp(beginning, ERROR_PARAMETER, ERROR_PARAMETER_LEN) == 0)
    {
        instrument->response.type = RBRINSTRUMENTGEN4_RESPONSE_ERROR;
        instrument->response.error = strtol(beginning + ERROR_PARAMETER_LEN, NULL, 10);
        /* Make sure we actually have a message to go along with the error.
         * There should be one, but it's best to play safe. */
        if (end - beginning >= (ERROR_PARAMETER_LEN + ERROR_NUMBER_LEN))
        {
            instrument->response.response = beginning + ERROR_PARAMETER_LEN + ERROR_NUMBER_LEN;
        }
        else
        {
            instrument->response.response = NULL;
        }
        return RBRINSTRUMENTGEN4_HARDWARE_ERROR;
    }
    else if (end - beginning >= (WARNING_PARAMETER_LEN + WARNING_NUMBER_LEN)
             && memcmp(beginning, WARNING_PARAMETER, WARNING_PARAMETER_LEN) == 0)
    {
        instrument->response.type = RBRINSTRUMENTGEN4_RESPONSE_WARNING;
        instrument->response.error = strtol(beginning + WARNING_PARAMETER_LEN, NULL, 10);
        if (end - beginning >= (WARNING_PARAMETER_LEN + WARNING_NUMBER_LEN))
        {
            instrument->response.response = beginning + WARNING_PARAMETER_LEN + WARNING_NUMBER_LEN;
        }
        else
        {
            instrument->response.response = NULL;
        }
        /* Create an additional status for warnings? */
        return RBRINSTRUMENTGEN4_HARDWARE_ERROR;
    }

    instrument->response.type = RBRINSTRUMENTGEN4_RESPONSE_INFO;
    instrument->response.error = RBRINSTRUMENTGEN4_HARDWARE_ERROR_NONE;
    instrument->response.response = beginning;

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_readResponse(RBRInstrumentGen4 *instrument,
                                                      bool breakOnSample,
                                                      RBRInstrumentGen4Sample *sample)
{
    /* Reset the response state. */
    instrument->response.type = RBRINSTRUMENTGEN4_RESPONSE_UNKNOWN_TYPE;
    instrument->response.error = RBRINSTRUMENTGEN4_HARDWARE_ERROR_NONE;
    instrument->response.response = NULL;

    RBRInstrumentGen4Sample *sampleTarget;
    if (sample == NULL)
    {
        sampleTarget = instrument->callbacks.sampleBuffer;
    }
    else
    {
        sampleTarget = sample;
    }

    /* Skip over streaming samples until we find a real command response, or
     * until we exceed the command timeout. */
    RBRInstrumentGen4DateTime startTime;
    RBR_TRY(instrument->callbacks.time(instrument, &startTime));
    while (true)
    {
        RBRInstrumentGen4_removeLastResponse(instrument);
        char *beginning;
        char *end;
        RBR_TRY(RBRInstrumentGen4_readSingleResponse(instrument, startTime, &end));

        RBRInstrumentGen4_terminateResponse(instrument, &beginning, end);

        if (sampleTarget != NULL && RBRInstrumentGen4Sample_parse(sampleTarget, &instrument->outputFormat, beginning) == RBRINSTRUMENTGEN4_SUCCESS)
        {
            if (instrument->callbacks.sample != NULL && sample == NULL)
            {
                RBR_TRY(instrument->callbacks.sample(instrument,
                                                     sampleTarget));
            }
            if (breakOnSample)
            {
                return RBRINSTRUMENTGEN4_SAMPLE;
            }
        }
        else
        {
            return RBRInstrumentGen4_errorCheckResponse(instrument,
                                                        beginning,
                                                        end);
        }
    }
}

void RBRInstrumentGen4_parseResponse(RBRInstrumentGen4 *instrument,
                                     char **command,
                                     RBRInstrumentGen4ResponseParameter *parameter)
{
    /*
     * If this has not been run, the command string is null, so the command end
     * seeks forward from the start of the response until finding the parameter
     * separator (' ') or the end of the string.
     */
    if (*command == NULL)
    {
        memset(parameter, 0, sizeof(RBRInstrumentGen4ResponseParameter));

        *command = instrument->response.response;
        // char testcommand[103] = "id model=RBRoem fwtype=120 version=1.14.5+202310150927 serial=092431";
        // *command = testcommand;
        char *commandEnd = *command;

        while (true)
        {
            switch (*commandEnd)
            {
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

    if (parameter->nextKey == NULL)
    {
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
     *     << channel conductivity_00 type=cond00 address=32 settlingtime=50 readtime=260 guardtime=20 userunits=mS/cm derived=off grouplist=none sensor=none
     *
     * This gets parsed into:
     * 
     *     << channel\0conductivity_00\0type\0cond00\0address=32 settlingtime=50 readtime=260 guardtime=20 userunits=mS/cm derived=off grouplist=none sensor=none\0
     *        ^command ^indexValue      ^key  ^value  ^nextKey
     *        ^instrument->response.response 
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

    while (true)
    {
        if (*parameter->value == '\0')
        {
            /* 
             * Nothing left to do if value reaches the end without finding the
             * value separator.
             */
            parameter->nextKey = NULL;
            return;
        }
        else if (*parameter->value == PARAMETER_VALUE_SEPARATOR_L4)
        {
            /* Null-terminate the value if found. */
            *parameter->value = '\0';
            ++parameter->value;
            break;
        }
        else if (*parameter->value == PARAMETER_SEPARATOR_L4)
        {
            previousSpace = parameter->value;
        }
        ++parameter->value;
    }

    /* 
     * Finding the parameter separator between the key and the value indicates
     * an index value. 
     * Set the index value to the current key and move the key past the separator.
     */
    if (previousSpace != NULL)
    {
        *previousSpace = '\0';
        ++parameter->index;
        parameter->indexValue = parameter->key;
        parameter->key = previousSpace + 1;
    }

    /*
     * L4 uses the pipe character as the separator for parameters returning
     * lists. E.g.,
     *
     *     >> link serial availablebaudrates availablemodes
     *     << link serial availablebaudrates=115200|19200|9600|4800|2400|1200|230400|460800 availablemodes=rs232|rs485f|uart|uart_idlelow
     *
     * This gets parsed into:
     *     << link serial\0availablebaudrates\0115200|19200|9600|4800|2400|1200|230400|460800\0availablemodes=rs232|rs485f|uart|uart_idlelow\0
     *        ^command     ^key               ^value                                           ^nextKey
     *        ^instrument->response.response 
     *
     * At last, the next key seeks forward from the value until the parameter
     * separator (' ') or the end of the string.
     */
    parameter->nextKey = parameter->value;
    while (true)
    {
        if (*parameter->nextKey == '\0')
        {
            /* 
             * Nothing left to do if next key reaches the end without finding
             * the parameter separator.
             */
            parameter->nextKey = NULL;
            return;
        }
        else if (*parameter->nextKey == PARAMETER_SEPARATOR_L4)
        {
            /* Null-terminate the next key. */
            *parameter->nextKey = '\0';
            ++parameter->nextKey;
            break;
        }
        ++parameter->nextKey;
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_converse(RBRInstrumentGen4 *instrument,
                                                  const char *command,
                                                  ...)
{
    RBRInstrumentGen4Error err;
    va_list format;
    va_list formatSend;
    va_start(format, command);

    /* Keep firing off the command and looking for a response until we find one
     * which matches. */
    bool retry;
    do
    {
        /* The retry flag might be set on by the “E0102 invalid command” error
         * handling below. It needs to be reset every time we send the command
         * so that we don't accidentally retry infinitely. */
        retry = false;

        /* Can't use RBR_TRY anywhere within these while loops because we need
         * to be sure to call va_end() on both va_lists before returning. */
        va_copy(formatSend, format);
        err = RBRInstrumentGen4_vSendCommand(instrument, command, formatSend);
        va_end(formatSend);

        if (err != RBRINSTRUMENTGEN4_SUCCESS)
        {
            break;
        }

        /* We can detect whether we've got the expected response based whether
         * its command word matches what we sent. To match that, we'll find the
         * first word of the command. */
        int32_t commandLength = 0;
        while (!isspace(instrument->commandBuffer[commandLength]) && instrument->commandBuffer[commandLength] != '\0')
        {
            ++commandLength;
        }

        /* The Logger2 “read” command response don't start with the command
         * itself. It's the only such command, so we'll just handle it here
         * rather than add another parameter to the function to indicate the
         * expected response, or to specialize the command handling function
         * to include error checking/retry. */
        uint8_t *commandResponse = instrument->commandBuffer;
        if (commandLength == 4 && memcmp("read", instrument->commandBuffer, 4) == 0)
        {
            commandResponse = (uint8_t *) "data";
        }

        do
        {
            err = RBRInstrumentGen4_readResponse(instrument, false, NULL);
            /*
             * There are a few reasons the instrument might generate an “E0102
             * invalid command” error, and we can make the user's life a bit
             * easier by handling it.
             *
             * - If the command in the error message matches the command we
             *   sent, then the error message is legitimate and should be
             *   forwarded to the user. This shouldn't happen for any commands
             *   generated by library functions, but it could happen if the
             *   user invokes RBRInstrumentGen4_converse() directly.
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
            if (err == RBRINSTRUMENTGEN4_HARDWARE_ERROR && (instrument->response.error ==
                                                            RBRINSTRUMENTGEN4_HARDWARE_ERROR_INVALID_COMMAND))
            {
                /* We have no message to inspect, so we can only assume the
                 * error is legitimate and pass it along to the user. */
                if (instrument->response.response == NULL)
                {
                    break;
                }

                /* The error message indicates what the invalid command was.
                 * It's enclosed in single quotes, so we can look for those to
                 * find its bounds. */
                char *invalidCommand = strchr(instrument->response.response,
                                              '\'');
                if (invalidCommand == NULL)
                {
                    break;
                }
                ++invalidCommand;

                char *invalidCommandEnd = strchr(invalidCommand, '\'');
                if (invalidCommandEnd == NULL)
                {
                    break;
                }

                int32_t invalidCommandLength = invalidCommandEnd - invalidCommand;

                /* The command was actually invalid. Whoops. */
                if (invalidCommandLength == commandLength && memcmp(invalidCommand,
                                                                    instrument->commandBuffer,
                                                                    commandLength) == 0)
                {
                    break;
                }
                /* We were on the right track, but there was garbage in the
                 * buffer. Retry. */
                else if (invalidCommandLength > commandLength && memcmp(invalidCommand + invalidCommandLength - commandLength,
                                                                        instrument->commandBuffer,
                                                                        commandLength) == 0)
                {
                    retry = true;
                    break;
                }
                /* Not our garbage, not our problem. We won't retry, but we'll
                 * keep trying to read a response. */
                else
                {
                    continue;
                }
            }
            else if (err != RBRINSTRUMENTGEN4_SUCCESS)
            {
                break;
            }
        } while ((instrument->response.response == NULL || memcmp(instrument->response.response,
                                                                  commandResponse,
                                                                  commandLength) != 0));
    } while (retry);

    va_end(format);

    return err;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getBool(RBRInstrumentGen4 *instrument,
                                                 const char *command,
                                                 const char *parameter,
                                                 bool *value)
{
    *value = false;

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "%s %s", command, parameter));

    char *responseCommand = NULL;
    RBRInstrumentGen4ResponseParameter responseParameter;
    do
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                        &responseCommand,
                                        &responseParameter);

        if (responseParameter.key == NULL)
        {
            break;
        }
        else if (strcmp(responseParameter.key, parameter) != 0)
        {
            continue;
        }

        *value = (strcmp(responseParameter.value, "on") == 0);
    } while (true);

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getFloat(RBRInstrumentGen4 *instrument,
                                                  const char *command,
                                                  const char *parameter,
                                                  float *value)
{
    *value = NAN;

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "%s %s", command, parameter));

    char *responseCommand = NULL;
    RBRInstrumentGen4ResponseParameter responseParameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                        &responseCommand,
                                        &responseParameter);

        if (responseParameter.key == NULL)
        {
            break;
        }
        else if (strcmp(responseParameter.key, parameter) != 0)
        {
            continue;
        }

        *value = strtod(responseParameter.value, NULL);
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getInt(RBRInstrumentGen4 *instrument,
                                                const char *command,
                                                const char *parameter,
                                                int32_t *value)
{
    *value = 0;

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "%s %s", command, parameter));

    char *responseCommand = NULL;
    RBRInstrumentGen4ResponseParameter responseParameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                        &responseCommand,
                                        &responseParameter);

        if (responseParameter.key == NULL)
        {
            break;
        }
        else if (strcmp(responseParameter.key, parameter) != 0)
        {
            continue;
        }

        *value = strtol(responseParameter.value, NULL, 10);
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

/** \brief Ensure localTimeOffset is initialized. */
static inline void RBRInstrumentGen4DateTime_initializeOffset()
{
    if (localTimeOffset == OFFSET_UNINITIALIZED)
    {
        struct tm instrumentMinTimestamp = {
            .tm_year = 100,
            .tm_mon = 0,
            .tm_mday = 1,
            .tm_hour = 0,
            .tm_min = 0,
            .tm_sec = 0
        };
        localTimeOffset =
            RBRINSTRUMENTGEN4_DATETIME_MIN - ((RBRInstrumentGen4DateTime) mktime(&instrumentMinTimestamp) * 1000);
    }
}

/**
 * \brief Parse a broken-down time into a millisecond timestamp.
 *
 * Any millisecond value should already be present in \a timestamp.
 *
 * \param [in] split the broken-down time
 * \param [in,out] timestamp the timestamp
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the timestamp is successfully parsed
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the time is invalid
 */
static RBRInstrumentGen4Error RBRInstrumentGen4DateTime_parse(
    struct tm *split,
    RBRInstrumentGen4DateTime *timestamp)
{
    /* struct tm/mktime() expects years to be counted from 1900... */
    split->tm_year -= 1900;
    /* ...and months to be 0-based. */
    split->tm_mon -= 1;

    /* Sanity check. */
    if (split->tm_year < 100 || split->tm_year >= 200 || split->tm_mon > 11 || split->tm_mday > 31 || split->tm_hour > 23 || split->tm_min > 59 || split->tm_sec > 59 /* Instrument doesn't know about leap seconds. */
        || *timestamp > 999)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    RBRInstrumentGen4DateTime_initializeOffset();

    *timestamp +=
        (((RBRInstrumentGen4DateTime) mktime(split)) * 1000) + localTimeOffset;

    if (*timestamp < RBRINSTRUMENTGEN4_DATETIME_MIN || *timestamp > RBRINSTRUMENTGEN4_DATETIME_MAX)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4DateTime_parseSampleTime(
    const char *s,
    RBRInstrumentGen4DateTime *timestamp,
    char **end)
{
    *timestamp = 0;
    if (end != NULL)
    {
        *end = NULL;
    }

    int32_t timestampLength;
    struct tm split = { 0 };
    int milliseconds;

    if (sscanf(s,
               RBRInstrumentGen4DateTime_sampleScanFormat,
               &split.tm_year,
               &split.tm_mon,
               &split.tm_mday,
               &split.tm_hour,
               &split.tm_min,
               &split.tm_sec,
               &milliseconds,
               &timestampLength) < 7)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    *timestamp = milliseconds;
    RBR_TRY(RBRInstrumentGen4DateTime_parse(&split, timestamp));
    if (end != NULL)
    {
        *end = (char *) s + timestampLength;
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4DateTime_parseScheduleTime(
    const char *s,
    RBRInstrumentGen4DateTime *timestamp,
    char **end)
{
    *timestamp = 0;
    if (end != NULL)
    {
        *end = NULL;
    }

    int32_t timestampLength;
    struct tm split = { 0 };
    if (sscanf(s,
               RBRInstrumentGen4DateTime_scheduleScanFormat,
               &split.tm_year,
               &split.tm_mon,
               &split.tm_mday,
               &split.tm_hour,
               &split.tm_min,
               &split.tm_sec,
               &timestampLength) < 6)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    RBR_TRY(RBRInstrumentGen4DateTime_parse(&split, timestamp));
    if (end != NULL)
    {
        *end = (char *) s + timestampLength;
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

static void RBRInstrumentGen4DateTime_toFormat(RBRInstrumentGen4DateTime timestamp,
                                               char *s,
                                               size_t size,
                                               const char *format)
{
    time_t t = timestamp / 1000;
    struct tm *split = gmtime(&t);
    int milliseconds = (int) (timestamp % 1000);
    snprintf(s,
             size,
             format,
             split->tm_year + 1900,
             split->tm_mon + 1,
             split->tm_mday,
             split->tm_hour,
             split->tm_min,
             split->tm_sec,
             milliseconds);
}

void RBRInstrumentGen4DateTime_toSampleTime(RBRInstrumentGen4DateTime timestamp,
                                            char *s)
{
    RBRInstrumentGen4DateTime_toFormat(timestamp,
                                       s,
                                       RBRINSTRUMENTGEN4_SAMPLE_TIME_LEN + 1,
                                       RBRInstrumentGen4DateTime_sampleFormat);
}

void RBRInstrumentGen4DateTime_toScheduleTime(RBRInstrumentGen4DateTime timestamp,
                                              char *s)
{
    RBRInstrumentGen4DateTime_toFormat(timestamp,
                                       s,
                                       RBRINSTRUMENTGEN4_SCHEDULE_TIME_LEN + 1,
                                       RBRInstrumentGen4DateTime_scheduleFormat);
}
