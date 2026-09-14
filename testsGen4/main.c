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
#include <stdlib.h>

#include "RBRGen4.h"
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
    /* In some test cases, no command will be sent, such as testCase 2 in "sampling_set" for Logger3 in tests/schedule.c.
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

RBRGen4Error TestIOBuffers_time(
    const struct RBRGen4 *instrument,
    RBRGen4DateTime *time)
{
    /* No-op. */
    *time = 0;
    (void)instrument;
    return RBRGEN4_SUCCESS;
}

RBRGen4Error TestIOBuffers_sleep(
    const struct RBRGen4 *instrument,
    RBRGen4DateTime time)
{
    /* No-op. */
    (void)instrument;
    (void)time;
    return RBRGEN4_SUCCESS;
}

RBRGen4Error TestIOBuffers_read(
    const struct RBRGen4 *instrument,
    void *data,
    int32_t *size)
{
    TestIOBuffers *buffers;
    buffers = (TestIOBuffers *) RBRGen4_getUserData(instrument);

    int32_t readLength = buffers->readBufferSize - buffers->readBufferPos;
    /* If we're out of data, indicate a callback error. */
    if (readLength <= 0)
    {
        fprintf(
            stderr,
            "%s line %d, TestIOBuffers_read: read buffer underrun! (%" PRIi32 "B "
            "requested.)\n",
            __FILE__, __LINE__, *size);
        return RBRGEN4_CALLBACK_ERROR;
    }
    else if (readLength > *size)
    {
        readLength = *size;
    }
    /* Otherwise, provide as much as we can from the read buffer. */
    memcpy(data, buffers->readBuffer + buffers->readBufferPos, readLength);
    *size = readLength;
    buffers->readBufferPos += readLength;
    return RBRGEN4_SUCCESS;
}

RBRGen4Error TestIOBuffers_write(const struct RBRGen4 *instrument,
                                       const void *const data,
                                       int32_t size)
{
    TestIOBuffers *buffers;
    buffers = (TestIOBuffers *) RBRGen4_getUserData(instrument);

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
        return RBRGEN4_CALLBACK_ERROR;
    }
    /* Otherwise, store the data to the write buffer. */
    memcpy(buffers->writeBuffer + buffers->writeBufferPos, data, size);
    buffers->writeBufferPos += size;
    /* Null-terminate the buffer so we can do string comparisons with it. */
    buffers->writeBuffer[buffers->writeBufferPos] = '\0';
    return RBRGEN4_SUCCESS;
}

RBRGen4Error TestIOBuffers_sample(
    const struct RBRGen4 *instrument,
    const struct RBRGen4Sample *const sample)
{
    TestIOBuffers *buffers;
    buffers = (TestIOBuffers *) RBRGen4_getUserData(instrument);
    if (sample != &buffers->streamSample)
    {
        return RBRGEN4_CALLBACK_ERROR;
    }
    return RBRGEN4_SUCCESS;
}

RBRGen4Error TestParserBuffers_sample(
    const struct RBRParserGen4 *parser,
    const struct RBRGen4Sample *const sample)
{
    TestParserBuffers *buffers;
    buffers = (TestParserBuffers *) RBRParserGen4_getUserData(parser);
    if (buffers->samplesLength >= TESTPARSERBUFFERS_SAMPLES_MAX)
    {
        return RBRGEN4_CALLBACK_ERROR;
    }
    memcpy(&buffers->samples[buffers->samplesLength++],
           sample,
           sizeof(RBRGen4Sample));
    return RBRGEN4_SUCCESS;
}

RBRGen4Error TestParserBuffers_event(
    const struct RBRParserGen4 *parser,
    const struct RBRInstrumentGen4Event *const event)
{
    TestParserBuffers *buffers;
    buffers = (TestParserBuffers *) RBRParserGen4_getUserData(parser);
    if (buffers->eventsLength >= TESTPARSERBUFFERS_EVENTS_MAX)
    {
        return RBRGEN4_CALLBACK_ERROR;
    }
    memcpy(&buffers->events[buffers->eventsLength++],
           event,
           sizeof(RBRInstrumentGen4Event));
    return RBRGEN4_SUCCESS;
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

int main(void)
{
    RBRGen4Error err;
    TestIOBuffers ioBuffers;
    RBRGen4Callbacks instrumentCallbacks = {
        .time =  TestIOBuffers_time,
        .sleep = TestIOBuffers_sleep,
        .read = TestIOBuffers_read,
        .write = TestIOBuffers_write,
        .sample = TestIOBuffers_sample,
        .sampleBuffer = &ioBuffers.streamSample
    };

    RBRGen4 instrumentL4Buffer;
    RBRGen4 *instrumentL4 = &instrumentL4Buffer;

    TestIOBuffers_init(
        &ioBuffers,
        "id4 model=L4 sn=999999 fwversion=2.0.0 "
        "semver=2.0.0-rc1-10-g148bc5eb1 fwtype=150"
        RESPONSE_TERMINATOR
        "instrument outputformat sn=off schedulelabel=on datetime=on crc=off encoding=ascii datatype=float32"
        RESPONSE_TERMINATOR,
        0);
    err = RBRGen4_open(&instrumentL4,
                             &instrumentCallbacks,
                             /* command timeout */ 0,
                             &ioBuffers);
    if (err != RBRGEN4_SUCCESS)
    {
        fprintf(stderr,
                "Failure initializing Logger4 test instrument: %s.\n",
                RBRGen4Error_name(err));
        return EXIT_FAILURE;
    }
    else
    {
        printf("Initialized Logger4 test instrument.\n");
    }

    TestParserBuffers parserBuffers;
    RBRGen4Sample parserSample;
    RBRInstrumentGen4Event parserEvent;
    RBRParserGen4Callbacks parserCallbacks = {
        .sample = TestParserBuffers_sample,
        .sampleBuffer = &parserSample,
        .event = TestParserBuffers_event,
        .eventBuffer = &parserEvent
    };

    RBRParserGen4 parserBuffer;
    RBRParserGen4 *parser = &parserBuffer;

    printf("Running tests...\n");
    int success = EXIT_SUCCESS;
    RBRGen4 *testInstrument;
    int32_t testsTotal = 0;
    int32_t testsPassed = 0;
    for (int32_t i = 0; instrumentTests[i].function != NULL; i++)
    {
        if (instrumentTests[i].generation == RBRGEN4_LOGGER4)
        {
            testInstrument = instrumentL4;
        }
        else
        {
            printf("Error: only Logger4 test is covered!");
        }

        /* Start every test from an awake instrument. Commands which end a
         * session — `sleep`, `instrument reboot` — clear the activity time,
         * which would otherwise make the next test see an unexpected wake
         * sequence in its write buffer and leave the suite dependent on the
         * order the modules happen to be listed in. */
        testInstrument->lastActivityTime = 0;

        printf("Running %s test \"%s\"...",
               RBRGen4Generation_name(instrumentTests[i].generation),
               instrumentTests[i].name);
        ++testsTotal;
        if (instrumentTests[i].function(testInstrument, &ioBuffers))
        {
            printf(" \033[32mpass\033[0m\n");
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

        err = RBRParserGen4_init(&parser,
                             &parserCallbacks,
                             parserTests[i].config,
                             &parserBuffers);
        if (err != RBRGEN4_SUCCESS)
        {
            printf(" \033[31minit fail\033[0m: %s\n",
                   RBRGen4Error_name(err));
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
    }

    printf("Tests completed (%" PRIi32 "/%" PRIi32 " passed).\n",
           testsPassed,
           testsTotal);

    return success;
}
