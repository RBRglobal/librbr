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

#define ARRAY_SEPARATOR_L2 " | "
#define ARRAY_SEPARATOR_LEN_L2 3
#define ARRAY_SEPARATOR_L3 " || "
#define ARRAY_SEPARATOR_LEN_L3 4

#define PARAMETER_SEPARATOR ", "
#define PARAMETER_SEPARATOR_LEN 2
#define PARAMETER_VALUE_SEPARATOR " = "
#define PARAMETER_VALUE_SEPARATOR_LEN 3

#define OFFSET_UNINITIALIZED (-1)

/* The length of an error number plus trailing space: “Exxxx ”. */
#define ERROR_LEN 6

/*
 * Logger2 instruments don't distinguish between warnings and errors. Some of
 * the “errors” produced by `verify`/`enable`/`stop` are non-fatal, but the
 * output doesn't distinguish between them – the consumer needs to be aware of
 * the difference. This is a list of error numbers which are actually warnings.
 */
static const RBRInstrumentGen4HardwareError WARNING_NUMBERS[] = {
    RBRINSTRUMENTGEN4_HARDWARE_ERROR_ESTIMATED_MEMORY_USAGE_EXCEEDS_CAPACITY,
    RBRINSTRUMENTGEN4_HARDWARE_ERROR_NOT_LOGGING
};
#define WARNING_NUMBER_COUNT \
    ((long) (sizeof(WARNING_NUMBERS) / sizeof(WARNING_NUMBERS[0])))

#define WARNING_PARAMETER ", warning = W"
#define WARNING_PARAMETER_LEN ((long) (sizeof(WARNING_PARAMETER) - 1))
#define WARNING_NUMBER_LEN 4

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
 * \brief Attempt to parse a sample from a response.
 *
 * \param [out] sample the sample
 * \param [in] response the response to parse
 * \return RBRINSTRUMENTGEN4_SUCCESS if the response is a sample
 * \return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE if the response is not a
 *                                               sample
 */
static RBRInstrumentGen4Error RBRInstrumentGen4Sample_parse(
    RBRInstrumentGen4Sample *sample,
    char *response)
{
    memset(sample, 0, sizeof(RBRInstrumentGen4Sample));

    /*
    All samples scenario:
    2023-09-10 11:24:14.125, 38.6671142e+000, 22.0217124e+000
    <RBR 999999, >poll, 2023-09-10 11:24:14.125, 38.6671142e+000, 22.0217124e+000<, 0xABCD>
    <RBR 999999, ><scheduleLabel, >2023-09-10 11:24:14.125, 38.6671142e+000, 22.0217124e+000<, 0xABCD>
    */
    char *values; //will be used in "foundTimestamp" statement.
    int notTokenized = 1;
    char *conditionalPtr = response;

    char *pch;
    /* Find first whitespace or "," in the response. 
    If whitespace found, and it's not "RBR ", it's not a sample.
    Not likely to be NULL with RBR commands. But just in case.
    */
    if ((pch = strpbrk(response, " ,")) != NULL)
    {
        if ((*pch == ' ') && (strncmp(response, "RBR ", 4) != 0))
        {
            // if it's not sample, it will return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE.
            goto foundTimestamp;
        }
        else
        // otherwise it's a sample. (i.e. (1) found ',' first, or (2) found whitespace but starts with "RBR ".)
        {
            char *crcptr = NULL;
            // if CRC exist, check CRC first before response is modified.
            // after this case, CRC will be chopped from response string.
            if ((crcptr = strstr(response, " 0x")) != NULL)
            {
                // get the string before CRC
                // e.g. RBR 999999, poll, 2020-01-01 00:19:15.001, 0.00000000e+000, 0.00000000e+000|, 0x4A63

                /*
                 * calculate CRC.
                 * The CRC includes all characters already sent on this line, starting with the first, up to
                 * and including the last space character before the <CRC>.
                 */
                uint16_t _realCrc = strtol(crcptr + 3, NULL, 16);
                uint16_t _calCrc;
                response[crcptr - response + 1] = '\0';
                _calCrc = calculateCrc(response, strlen(response));
                if (_calCrc != _realCrc)
                {
                    return RBRINSTRUMENTGEN4_CHECKSUM_ERROR;
                }
            }

            // Find timestamp and populate sample values.
            char *_token;
            _token = strtok(response, ",");
            notTokenized = 0;
            if (_token != NULL)
            {
                // check if the first token is timestamp.
                if (isdigit((int) _token[0]))
                // first token is timestamp:
                {
                    goto foundTimestamp;
                }
                else
                /* first token is not timestamp. keep skipping "RBR <serial>" and/or "<schedulelabel>/poll"
                 * until we find a token starts with " <digit>".
                 */
                {
                    while (((_token = strtok(NULL, ",")) != NULL) && strlen(_token) >= 2)
                    {
                        if (isdigit((int) _token[1]))
                        {
                            _token++; // get rid of leading whitespace.
                            goto foundTimestamp;
                        }
                    }
                }

            foundTimestamp:
                if (notTokenized == 0){
                    conditionalPtr = _token;
                }
                RBR_TRY(RBRInstrumentGen4DateTime_parseSampleTime(conditionalPtr,
                                                    &sample->timestamp,
                                                    &values));
                // reset sample->channels if it has random initialization
                if (sample->channels >= RBRINSTRUMENTGEN4_CHANNEL_MAX)
                {
                    sample->channels = 0;
                }
                double reading;
                
                if(notTokenized == 0){
                    conditionalPtr = NULL;
                }
                else if(notTokenized == 1){
                    conditionalPtr = values;
                }

                while ((_token = strtok(conditionalPtr, ",")) != NULL && strlen(_token)>=2 &&sample->channels < RBRINSTRUMENTGEN4_CHANNEL_MAX)
                {
                    /* The token will always have a leading space. */
                    /* If sample has CRC, then there's a trailing ", " that should not be considered a channel reading.*/
                    if(notTokenized == 1)
                    {conditionalPtr = NULL;}

                    ++_token;

                    if (strcmp(_token, SAMPLE_NAN) == 0)
                    {
                        reading = NAN;
                    }
                    else if (strcmp(_token, SAMPLE_INF) == 0)
                    {
                        reading = INFINITY;
                    }
                    else if (strcmp(_token, SAMPLE_NINF) == 0)
                    {
                        reading = -INFINITY;
                    }
                    else if (strcmp(_token, SAMPLE_UNCAL) == 0)
                    {
                        reading = RBRInstrumentGen4Reading_setError(
                            RBRINSTRUMENTGEN4_READING_FLAG_UNCALIBRATED,
                            0);
                    }
                    else if (memcmp(_token,
                                    SAMPLE_ERROR_PREFIX,
                                    SAMPLE_ERROR_PREFIX_LEN) == 0)
                    {
                        /* Uh-oh. We'll encode the error in a NaN. Filtering, etc. will
                         * ignore the value and the sample formatter will output it just as
                         * we received it. */
                        reading = RBRInstrumentGen4Reading_setError(
                            RBRINSTRUMENTGEN4_READING_FLAG_ERROR,
                            strtol(_token + SAMPLE_ERROR_PREFIX_LEN,
                                   NULL,
                                   10));
                    }
                    else
                    {
                        reading = strtod(_token, NULL);
                    }

                    sample->readings[sample->channels++] = reading;
                }
                return RBRINSTRUMENTGEN4_SUCCESS;
            }
            else
            { // return error if no "," present in the response. Not likely but just in case.
                return RBRINSTRUMENTGEN4_UNKNOWN_ERROR;
            }
        }

    }
    else // no whitespace nor "," in the response. Not likely but just in case.
    {
        return RBRINSTRUMENTGEN4_UNKNOWN_ERROR;
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
     * Errors will be found at the beginning of commands. E.g.,
     *
     *     >> wifi timeout = whenever
     *     << E0108 invalid argument to command: 'whenever'
     */
    if (*beginning == 'E')
    {
        instrument->response.type = RBRINSTRUMENTGEN4_RESPONSE_ERROR;
        instrument->response.error = strtol(beginning + 1, NULL, 10);
        /* Make sure we actually have a message to go along with the error.
         * There should be one, but it's best to play safe. */
        if (end - beginning >= ERROR_LEN)
        {
            instrument->response.response = beginning + ERROR_LEN;
        }
        else
        {
            instrument->response.response = NULL;
        }

        /* Logger2 instruments don't distinguish between warnings and errors,
         * so if we get an error response, we'll check whether it needs to be
         * translated into a warning. */
        if (instrument->generation == RBRINSTRUMENTGEN4_LOGGER2)
        {
            for (int i = 0; i < WARNING_NUMBER_COUNT; ++i)
            {
                if (instrument->response.error != WARNING_NUMBERS[i])
                {
                    continue;
                }

                instrument->response.type = RBRINSTRUMENTGEN4_RESPONSE_WARNING;

                /*
                 * The actual command response will be after the warning, so
                 * we'll fast-forward past it.
                 *
                 * This doesn't support fast-forwarding past warning messages
                 * containing commas. However, while there are multiple error
                 * messages which contain commas, there are no such warnings.
                 */
                if (instrument->response.response != NULL)
                {
                    instrument->response.response = strchr(
                        instrument->response.response,
                        ',');
                }

                if (instrument->response.response != NULL)
                {
                    instrument->response.response += 2;
                }

                return RBRINSTRUMENTGEN4_SUCCESS;
            }
        }

        /* Not being Logger2 or not having performed a substitution means it's
         * a real error. */
        return RBRINSTRUMENTGEN4_HARDWARE_ERROR;
    }

    instrument->response.type = RBRINSTRUMENTGEN4_RESPONSE_INFO;
    instrument->response.error = RBRINSTRUMENTGEN4_HARDWARE_ERROR_NONE;
    instrument->response.response = beginning;

    /*
     * In Logger3, warnings are at the end of a response. E.g.,
     *
     *     >> verify
     *     << verify status = logging, warning = W0401
     *
     * Warnings never have a message and the number is always zero-padded to
     * four digits, so we can just check for the presence of a “warning”
     * parameter in a fixed position, parse out the number, and then truncate
     * the response so the parser doesn't have to deal with it.
     */
    if (end - beginning >= (WARNING_PARAMETER_LEN + WARNING_NUMBER_LEN) && memcmp(end - WARNING_PARAMETER_LEN - WARNING_NUMBER_LEN,
                                                                                  WARNING_PARAMETER,
                                                                                  WARNING_PARAMETER_LEN) == 0)
    {
        instrument->response.type = RBRINSTRUMENTGEN4_RESPONSE_WARNING;
        instrument->response.error = strtol(end - WARNING_NUMBER_LEN,
                                            NULL,
                                            10);
        *(end - WARNING_PARAMETER_LEN - WARNING_NUMBER_LEN) = '\0';
    }

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

        if (sampleTarget != NULL && RBRInstrumentGen4Sample_parse(sampleTarget, beginning) == RBRINSTRUMENTGEN4_SUCCESS)
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
    bool hasParameters = true;
    if (*command == NULL)
    {
        memset(parameter, 0, sizeof(RBRInstrumentGen4ResponseParameter));

        *command = instrument->response.response;
        // char testcommand[103] = "id model = RBRoem, fwtype = 130, version = 1.14.5+202310150927, serial = 092431";
        // *command = testcommand;
        char *commandEnd = *command;

        while (true)
        {
            switch (*commandEnd)
            {
            case '\0':
                hasParameters = false;
            /* Fallthrough. */
            case ' ':
                goto foundCommandEnd;
            default:
                ++commandEnd;
            }
        }
    foundCommandEnd:
        /*
         * All L3 commands return at least one parameter. However, lots of
         * simple L2 commands (e.g., link) use the command itself as a
         * parameter. E.g.,
         *
         *     >> link
         *     << link = usb
         *
         * So before terminating the command, we'll check if it should also be
         * used as the first parameter key. If so, we won't null-terminate it:
         * that will be done for us when the value is parsed.
         */
        if (hasParameters)
        {
            if (memcmp(commandEnd,
                       PARAMETER_VALUE_SEPARATOR,
                       PARAMETER_VALUE_SEPARATOR_LEN) == 0)
            {
                parameter->nextKey = *command;
            }
            else
            {
                *commandEnd = '\0';
                parameter->nextKey = commandEnd + 1;
            }
        }
    }

    if (!hasParameters || parameter->nextKey == NULL)
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
     *     >> regime 1
     *     << regime 1 boundary = 50, binsize = 0.1, samplingperiod = 63
     *
     * We'll look for the value separator and remember where we most recently
     * saw a space before it. That space separates the end of the index value
     * from the beginning of the parameter key. Because this can happen both
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
            parameter->nextKey = NULL;
            return;
        }
        else if (memcmp(parameter->value,
                        PARAMETER_VALUE_SEPARATOR,
                        PARAMETER_VALUE_SEPARATOR_LEN) == 0)
        {
            /* Null-terminate the key. */
            *parameter->value = '\0';
            parameter->value += PARAMETER_VALUE_SEPARATOR_LEN;
            break;
        }
        else if (*parameter->value == ' ')
        {
            previousSpace = parameter->value;
        }

        ++parameter->value;
    }

    /* If we found whitespace between the beginning of the key and the value
     * separator, that means there's an index value. */
    if (previousSpace != NULL)
    {
        *previousSpace = '\0';
        ++parameter->index;
        parameter->indexValue = parameter->key;
        parameter->key = previousSpace + 1;
    }
    /*
     * L3 uses the pipe character as the separator for parameters returning
     * lists. E.g.,
     *
     *     >> memformat availabletypes type
     *     << memformat type = calbin00, availabletypes = rawbin00|calbin00
     *
     * However, L2 used a comma:
     *
     *     >> memformat support type
     *     << memformat support = rawbin00, calbin00, type = rawbin00
     *                                    ^         ^      ^
     *                                    |         |      |
     *                                    c         b      a
     *
     * So to find the end of the value (in this example, the value of the
     *  “support” parameter), we need to seek forward to the next value
     * separator (“a”), then seek _backwards_ to find the parameter separator
     * (“b”). Any earlier parameter separator (“c”) might be part of the value.
     * We also need to check for the separator used by array responses (“ | ”
     * for Logger2, “ || ” for Logger3).
     */
    parameter->nextKey = strstr(parameter->value, PARAMETER_VALUE_SEPARATOR);

    if (parameter->nextKey == NULL)
    {
        return;
    }

    int32_t separatorLength = -1;
    while (parameter->nextKey > parameter->value && separatorLength < 0)
    {
        if (memcmp(parameter->nextKey,
                   PARAMETER_SEPARATOR,
                   PARAMETER_SEPARATOR_LEN) == 0)
        {
            separatorLength = PARAMETER_SEPARATOR_LEN;
        }
        else if (instrument->generation == RBRINSTRUMENTGEN4_LOGGER2 && memcmp(parameter->nextKey,
                                                                               ARRAY_SEPARATOR_L2,
                                                                               ARRAY_SEPARATOR_LEN_L2) == 0)
        {
            separatorLength = ARRAY_SEPARATOR_LEN_L2;
        }
        else if (memcmp(parameter->nextKey,
                        ARRAY_SEPARATOR_L3,
                        ARRAY_SEPARATOR_LEN_L3) == 0)
        {
            /* L3 separates array members in responses with the separator _and_
             * the command name. */
            separatorLength = ARRAY_SEPARATOR_LEN_L3 + strlen(*command) + 1;
        }
        else
        {
            --parameter->nextKey;
        }
    }

    if (separatorLength >= 0)
    {
        /* Null-terminate the value. */
        *parameter->nextKey = '\0';
        parameter->nextKey += separatorLength;
    }
    else
    {
        /* Something went horribly wrong: we found what we thought was the
         * start of the next key, but then didn't find any separators between
         * it and the start of the value. Give up. */
        parameter->nextKey = NULL;
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
