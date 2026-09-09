/**
 * \file runner.c
 *
 * \brief Runner for library tests.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for isprint. */
#include <ctype.h>

#include "RBRGen3.h"
#include "tests.h"

char *rbr_strnesccntrl(char *destination, const char *source, size_t num)
{
    size_t pos = 0;
    while (*source && pos < num - 4)
    {
        if (!isprint((unsigned char)*source))
        {
            destination[pos++] = '<';

            if (*source == '\r')
            {
                destination[pos++] = 'C';
                destination[pos++] = 'R';
            }
            else if (*source == '\n')
            {
                destination[pos++] = 'L';
                destination[pos++] = 'F';
            }
            else
            {
                sprintf(&destination[pos], "%02X", *source);
                pos += 2;
            }

            destination[pos++] = '>';
        }
        else
        {
            destination[pos++] = *source;
        }

        ++source;
    }
    destination[pos] = '\0';

    return destination;
}

void rbr_prepareCommandResponse(const char *text, char *expectedCommand, char *response){
    if (strlen(text)!=0){
        strcpy(expectedCommand, text);
        strcat(expectedCommand, COMMAND_TERMINATOR);

        strcpy(response, text);
        strcat(response, RESPONSE_TERMINATOR);
    }
    /* In some test cases, no command will be sent, such as testCase 2 in "sampling_set" for Logger3 in testsGen3/schedule.c.
       And in such cases, expectedCommand should be empty string. */
    else{
        expectedCommand[0]='\0';
    }
}

void TestIOBuffers_init(TestIOBuffers *buffers,
                        const char *readBuffer,
                        int32_t readBufferSize)
{
    memset(buffers, 0, sizeof(TestIOBuffers));
    buffers->readBuffer = readBuffer;
    if (readBufferSize == 0)
    {
        buffers->readBufferSize = strlen(readBuffer);
    }
    else
    {
        buffers->readBufferSize = readBufferSize;
    }
}

RBRGen3Error TestIOBuffers_time(
    const struct RBRGen3 *instrument,
    RBRGen3DateTime *time)
{
    /* No-op. */
    *time = 0;
    return RBRGEN3_SUCCESS;
}

RBRGen3Error TestIOBuffers_sleep(
    const struct RBRGen3 *instrument,
    RBRGen3DateTime time)
{
    /* No-op. */
    return RBRGEN3_SUCCESS;
}

RBRGen3Error TestIOBuffers_read(
    const struct RBRGen3 *instrument,
    void *data,
    int32_t *size)
{
    TestIOBuffers *buffers;
    buffers = (TestIOBuffers *) RBRGen3_getUserData(instrument);

    int32_t readLength = buffers->readBufferSize - buffers->readBufferPos;
    /* If we're out of data, indicate a callback error. */
    if (readLength <= 0)
    {
        fprintf(
            stderr,
            "%s line %d, TestIOBuffers_read: read buffer underrun! (%" PRIi32 "B "
            "requested.)\n",
            __FILE__, __LINE__, *size);
        return RBRGEN3_CALLBACK_ERROR;
    }
    else if (readLength > *size)
    {
        readLength = *size;
    }
    /* Otherwise, provide as much as we can from the read buffer. */
    memcpy(data, buffers->readBuffer + buffers->readBufferPos, readLength);
    *size = readLength;
    buffers->readBufferPos += readLength;
    return RBRGEN3_SUCCESS;
}

RBRGen3Error TestIOBuffers_write(const struct RBRGen3 *instrument,
                                       const void *const data,
                                       int32_t size)
{
    TestIOBuffers *buffers;
    buffers = (TestIOBuffers *) RBRGen3_getUserData(instrument);

    int32_t remaining
        = TESTIOBUFFERS_WRITE_BUFFER_SIZE - buffers->writeBufferPos;
    /* If we're out of space, indicate a callback error. */
    if (remaining < size)
    {
        fprintf(
            stderr,
            "TestIOBuffers_write: write buffer full! (Tried to write %" PRIi32
            "B but only had space for %" PRIi32 "B.)\n",
            size,
            remaining);
        return RBRGEN3_CALLBACK_ERROR;
    }
    /* Otherwise, store the data to the write buffer. */
    memcpy(buffers->writeBuffer + buffers->writeBufferPos, data, size);
    buffers->writeBufferPos += size;
    /* Null-terminate the buffer so we can do string comparisons with it. */
    buffers->writeBuffer[buffers->writeBufferPos] = '\0';
    return RBRGEN3_SUCCESS;
}

RBRGen3Error TestIOBuffers_sample(
    const struct RBRGen3 *instrument,
    const struct RBRGen3Sample *const sample)
{
    TestIOBuffers *buffers;
    buffers = (TestIOBuffers *) RBRGen3_getUserData(instrument);
    if (sample != &buffers->streamSample)
    {
        return RBRGEN3_CALLBACK_ERROR;
    }
    return RBRGEN3_SUCCESS;
}

RBRGen3Error TestParserBuffers_sample(
    const struct RBRParserGen3 *parser,
    const struct RBRGen3Sample *const sample)
{
    TestParserBuffers *buffers;
    buffers = (TestParserBuffers *) RBRParserGen3_getUserData(parser);
    if (buffers->samplesLength >= TESTPARSERBUFFERS_SAMPLES_MAX)
    {
        return RBRGEN3_CALLBACK_ERROR;
    }
    memcpy(&buffers->samples[buffers->samplesLength++],
           sample,
           sizeof(RBRGen3Sample));
    return RBRGEN3_SUCCESS;
}

RBRGen3Error TestParserBuffers_event(
    const struct RBRParserGen3 *parser,
    const struct RBRInstrumentGen3Event *const event)
{
    TestParserBuffers *buffers;
    buffers = (TestParserBuffers *) RBRParserGen3_getUserData(parser);
    if (buffers->eventsLength >= TESTPARSERBUFFERS_EVENTS_MAX)
    {
        return RBRGEN3_CALLBACK_ERROR;
    }
    memcpy(&buffers->events[buffers->eventsLength++],
           event,
           sizeof(RBRInstrumentGen3Event));
    return RBRGEN3_SUCCESS;
}

const char *bool_name(bool value)
{
    if (value)
    {
        return "true";
    }
    else
    {
        return "false";
    }
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    RBRGen3Error err;
    TestIOBuffers ioBuffers;
    RBRGen3Callbacks instrumentCallbacks = {
        .time =  TestIOBuffers_time,
        .sleep = TestIOBuffers_sleep,
        .read = TestIOBuffers_read,
        .write = TestIOBuffers_write,
        .sample = TestIOBuffers_sample,
        .sampleBuffer = &ioBuffers.streamSample
    };

    RBRGen3 instrumentL2Buffer;
    RBRGen3 *instrumentL2 = &instrumentL2Buffer;
    TestIOBuffers_init(
        &ioBuffers,
        "RBR RBRoem 1.430 999999"
        RESPONSE_TERMINATOR
        "id model = RBRoem, version = 1.430, serial = 999999, fwtype = 103"
        RESPONSE_TERMINATOR,
        0);
    err = RBRGen3_open(&instrumentL2,
                             &instrumentCallbacks,
                             /* command timeout */ 0,
                             &ioBuffers);
    if (err != RBRGEN3_SUCCESS)
    {
        fprintf(stderr,
                "Failure initializing Logger2 test instrument: %s.\n",
                RBRGen3Error_name(err));
        return EXIT_FAILURE;
    }
    else
    {
        printf("Initialized Logger2 test instrument.\n");
    }

    RBRGen3 instrumentL3Buffer;
    RBRGen3 *instrumentL3 = &instrumentL3Buffer;
    TestIOBuffers_init(
        &ioBuffers,
        "RBR RBRduo3 1.090 999999"
        RESPONSE_TERMINATOR
        "id model = RBRoem3, version = 1.134, serial = 999999, fwtype = 104"
        RESPONSE_TERMINATOR,
        0);
    err = RBRGen3_open(&instrumentL3,
                             &instrumentCallbacks,
                             /* command timeout */ 0,
                             &ioBuffers);
    if (err != RBRGEN3_SUCCESS)
    {
        fprintf(stderr,
                "Failure initializing Logger3 test instrument: %s.\n",
                RBRGen3Error_name(err));
        return EXIT_FAILURE;
    }
    else
    {
        printf("Initialized Logger3 test instrument.\n");
    }

    RBRGen3 instrumentL4Buffer;
    RBRGen3 *instrumentL4 = &instrumentL4Buffer;
    TestIOBuffers_init(
        &ioBuffers,
        "id model = RBRduet4, version = 1.0.0, serial = 999999, fwtype = 131"
        RESPONSE_TERMINATOR,
        0);
    err = RBRGen3_open(&instrumentL4,
                             &instrumentCallbacks,
                             /* command timeout */ 0,
                             &ioBuffers);
    if (err == RBRGEN3_SUCCESS)
    {
        fprintf(stderr,
                "Unexpected success initializing Logger4 test instrument.\n");
        return EXIT_FAILURE;
    }
    else if (err != RBRGEN3_UNSUPPORTED)
    {
        fprintf(stderr,
                "Failure initializing Logger3 test instrument: %s.\n",
                RBRGen3Error_name(err));
        return EXIT_FAILURE;
    }
    else /* RBRGEN3_UNSUPPORTED */
    {
        if (instrumentL4->generation != RBRGEN3_LOGGER4)
        {
            fprintf(stderr,
                    "Unexpected generation Logger4 generation: %s.\n",
                    RBRGen3Generation_name(instrumentL4->generation));
            return EXIT_FAILURE;
        }
        else
        {
            printf("Successfully rejected Logger4 test instrument.\n");
        }
    }

    TestParserBuffers parserBuffers;
    RBRGen3Sample parserSample;
    RBRInstrumentGen3Event parserEvent;
    RBRParserGen3Callbacks parserCallbacks = {
        .sample = TestParserBuffers_sample,
        .sampleBuffer = &parserSample,
        .event = TestParserBuffers_event,
        .eventBuffer = &parserEvent
    };

    RBRParserGen3 parserBuffer;
    RBRParserGen3 *parser = &parserBuffer;

    printf("Running tests...\n");
    int success = EXIT_SUCCESS;
    RBRGen3 *testInstrument;
    int32_t testsTotal = 0;
    int32_t testsPassed = 0;
    for (int32_t i = 0; instrumentTests[i].function != NULL; i++)
    {
        if (instrumentTests[i].generation == RBRGEN3_LOGGER2)
        {
            testInstrument = instrumentL2;
        }
        else
        {
            testInstrument = instrumentL3;
        }

        printf("Running %s test \"%s\"...",
               RBRGen3Generation_name(instrumentTests[i].generation),
               instrumentTests[i].name);
        ++testsTotal;
        if (instrumentTests[i].function(testInstrument, &ioBuffers))
        {
            printf(" \033[32mok\033[0m\n");
            ++testsPassed;
        }
        else
        {
            printf(" \033[31mfail\033[0m\n");
            success = EXIT_FAILURE;
        }
    }

    for (int32_t i = 0; parserTests[i].function != NULL; i++)
    {
        printf("Running parser test \"%s\"...", parserTests[i].name);
        ++testsTotal;

        memset(&parserBuffers, 0, sizeof(TestParserBuffers));

        err = RBRParserGen3_init(&parser,
                             &parserCallbacks,
                             parserTests[i].config,
                             &parserBuffers);
        if (err != RBRGEN3_SUCCESS)
        {
            printf(" \033[31minit fail\033[0m: %s\n",
                   RBRGen3Error_name(err));
            success = EXIT_FAILURE;
            continue;
        }

        if (parserTests[i].function(parser, &parserBuffers))
        {
            printf(" \033[32mok\033[0m\n");
            ++testsPassed;
        }
        else
        {
            printf(" \033[31mfail\033[0m\n");
            success = EXIT_FAILURE;
        }

        RBRParserGen3_destroy(parser);
    }

    printf("Tests completed (%" PRIi32 "/%" PRIi32 " passed).\n",
           testsPassed,
           testsTotal);

    return success;
}
