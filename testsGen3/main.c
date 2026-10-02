/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file main.c
 *
 * \brief Runner for library tests.
 */

/* Required for isprint. */
#include <ctype.h>

#include "RBRGen3.h"
#include "tests.h"
#include "RBRGen3Parser.h"
#include "RBRGen3Streaming.h"

char *rbr_strnesccntrl(char *destination, const char *source, size_t num)
{
    size_t pos = 0;
    while (*source && pos < num - 4) {
        if (!isprint((unsigned char) *source)) {
            destination[pos++] = '<';

            if (*source == '\r') {
                destination[pos++] = 'C';
                destination[pos++] = 'R';
            } else if (*source == '\n') {
                destination[pos++] = 'L';
                destination[pos++] = 'F';
            } else {
                sprintf(&destination[pos], "%02X", *source);
                pos += 2;
            }

            destination[pos++] = '>';
        } else {
            destination[pos++] = *source;
        }

        ++source;
    }
    destination[pos] = '\0';

    return destination;
}

void rbr_prepareCommandResponse(const char *text, char *expectedCommand, char *response)
{
    if (strlen(text) != 0) {
        strcpy(expectedCommand, text);
        strcat(expectedCommand, COMMAND_TERMINATOR);

        strcpy(response, text);
        strcat(response, RESPONSE_TERMINATOR);
    }
    /* In some test cases, no command will be sent, such as testCase 2 in "sampling_set" for Logger3
       in testsGen3/schedule.c. And in such cases, expectedCommand should be empty string. */
    else {
        expectedCommand[0] = '\0';
    }
}

void TestIOBuffers_init(TestIOBuffers *buffers, const char *readBuffer, int32_t readBufferSize)
{
    memset(buffers, 0, sizeof(TestIOBuffers));
    buffers->streamSample.readings = buffers->streamReadings;
    buffers->streamSample.size = TESTS_CHANNEL_MAX;
    buffers->readBuffer = readBuffer;
    if (readBufferSize == 0) {
        buffers->readBufferSize = strlen(readBuffer);
    } else {
        buffers->readBufferSize = readBufferSize;
    }
}

RBRGen3Error TestIOBuffers_time(const RBRGen3 *conn, RBRGen3DateTime *time)
{
    /* No-op. */
    *time = 0;
    return RBRGEN3_SUCCESS;
}

RBRGen3Error TestIOBuffers_sleep(const RBRGen3 *conn, RBRGen3DateTime time)
{
    /* No-op. */
    return RBRGEN3_SUCCESS;
}

RBRGen3Error TestIOBuffers_read(const RBRGen3 *conn, void *data, int32_t *size)
{
    TestIOBuffers *buffers;
    buffers = (TestIOBuffers *) RBRGen3_getUserData(conn);

    int32_t readLength = buffers->readBufferSize - buffers->readBufferPos;
    /* If we're out of data, indicate a callback error. */
    if (readLength <= 0) {
        fprintf(stderr,
                "%s line %d, TestIOBuffers_read: read buffer underrun! (%" PRIi32 "B "
                "requested.)\n",
                __FILE__,
                __LINE__,
                *size);
        return RBRGEN3_CALLBACK_ERROR;
    } else if (readLength > *size) {
        readLength = *size;
    }
    /* Otherwise, provide as much as we can from the read buffer. */
    memcpy(data, buffers->readBuffer + buffers->readBufferPos, readLength);
    *size = readLength;
    buffers->readBufferPos += readLength;
    return RBRGEN3_SUCCESS;
}

RBRGen3Error TestIOBuffers_write(const RBRGen3 *conn, const void *const data, int32_t size)
{
    TestIOBuffers *buffers;
    buffers = (TestIOBuffers *) RBRGen3_getUserData(conn);

    int32_t remaining = TESTIOBUFFERS_WRITE_BUFFER_SIZE - buffers->writeBufferPos;
    /* If we're out of space, indicate a callback error. */
    if (remaining < size) {
        fprintf(stderr,
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

RBRGen3Error TestIOBuffers_sample(const RBRGen3 *conn, const RBRGen3Sample *const sample)
{
    TestIOBuffers *buffers;
    buffers = (TestIOBuffers *) RBRGen3_getUserData(conn);
    if (sample != &buffers->streamSample) {
        return RBRGEN3_CALLBACK_ERROR;
    }
    return RBRGEN3_SUCCESS;
}

RBRGen3Error TestParserBuffers_sample(const RBRGen3Parser *parser,
                                      const RBRGen3Sample *const sample)
{
    TestParserBuffers *buffers;
    buffers = (TestParserBuffers *) RBRGen3Parser_getUserData(parser);
    if (buffers->samplesLength >= TESTPARSERBUFFERS_SAMPLES_MAX) {
        return RBRGEN3_CALLBACK_ERROR;
    }
    RBRGen3Sample *copy = &buffers->samples[buffers->samplesLength];
    *copy = *sample;
    copy->readings = buffers->samplesReadings[buffers->samplesLength];
    copy->size = TESTS_CHANNEL_MAX;
    int32_t stored =
        sample->channelCount < TESTS_CHANNEL_MAX ? sample->channelCount : TESTS_CHANNEL_MAX;
    memcpy(copy->readings, sample->readings, (size_t) stored * sizeof(*copy->readings));
    ++buffers->samplesLength;
    return RBRGEN3_SUCCESS;
}

RBRGen3Error TestParserBuffers_event(const RBRGen3Parser *parser, const RBRGen3Event *const event)
{
    TestParserBuffers *buffers;
    buffers = (TestParserBuffers *) RBRGen3Parser_getUserData(parser);
    if (buffers->eventsLength >= TESTPARSERBUFFERS_EVENTS_MAX) {
        return RBRGEN3_CALLBACK_ERROR;
    }
    memcpy(&buffers->events[buffers->eventsLength++], event, sizeof(RBRGen3Event));
    return RBRGEN3_SUCCESS;
}

const char *bool_name(bool value)
{
    if (value) {
        return "true";
    } else {
        return "false";
    }
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    RBRGen3Error err;
    TestIOBuffers ioBuffers;
    /* The three test connections are open at once, so each one needs its own
     * buffers: a response buffer belongs to a single live connection. */
    static uint8_t commandBuffers[3][RBRGEN3_COMMAND_BUFFER_DEFAULT];
    static uint8_t responseBuffers[3][RBRGEN3_RESPONSE_BUFFER_DEFAULT];
    RBRGen3Environment instrumentEnvironment[3];
    for (size_t i = 0; i < 3; ++i) {
        instrumentEnvironment[i] = (RBRGen3Environment) {
            .time = TestIOBuffers_time,
            .sleep = TestIOBuffers_sleep,
            .read = TestIOBuffers_read,
            .write = TestIOBuffers_write,
            .sample = TestIOBuffers_sample,
            .sampleBuffer = &ioBuffers.streamSample,
            .command = commandBuffers[i],
            .commandCapacity = sizeof(commandBuffers[i]),
            .response = responseBuffers[i],
            .responseCapacity = sizeof(responseBuffers[i]),
        };
    }

    RBRGen3 instrumentL2Buffer;
    RBRGen3 *instrumentL2 = &instrumentL2Buffer;
    TestIOBuffers_init(
        &ioBuffers,
        "RBR RBRoem 1.430 999999" RESPONSE_TERMINATOR
        "id model = RBRoem, version = 1.430, serial = 999999, fwtype = 103" RESPONSE_TERMINATOR,
        0);
    err = RBRGen3_open(instrumentL2,
                       &instrumentEnvironment[0],
                       /* command timeout */ 0,
                       &ioBuffers);
    if (err != RBRGEN3_SUCCESS) {
        fprintf(
            stderr, "Failure initializing Logger2 test instrument: %s.\n", RBRGen3Error_name(err));
        return EXIT_FAILURE;
    } else {
        printf("Initialized Logger2 test instrument.\n");
    }

    RBRGen3 instrumentL3Buffer;
    RBRGen3 *instrumentL3 = &instrumentL3Buffer;
    TestIOBuffers_init(
        &ioBuffers,
        "RBR RBRduo3 1.090 999999" RESPONSE_TERMINATOR
        "id model = RBRoem3, version = 1.134, serial = 999999, fwtype = 104" RESPONSE_TERMINATOR,
        0);
    err = RBRGen3_open(instrumentL3,
                       &instrumentEnvironment[1],
                       /* command timeout */ 0,
                       &ioBuffers);
    if (err != RBRGEN3_SUCCESS) {
        fprintf(
            stderr, "Failure initializing Logger3 test instrument: %s.\n", RBRGen3Error_name(err));
        return EXIT_FAILURE;
    } else {
        printf("Initialized Logger3 test instrument.\n");
    }

    RBRGen3 instrumentL4Buffer;
    RBRGen3 *instrumentL4 = &instrumentL4Buffer;
    TestIOBuffers_init(
        &ioBuffers,
        "id model = RBRduet4, version = 1.0.0, serial = 999999, fwtype = 131" RESPONSE_TERMINATOR,
        0);
    err = RBRGen3_open(instrumentL4,
                       &instrumentEnvironment[2],
                       /* command timeout */ 0,
                       &ioBuffers);
    if (err == RBRGEN3_SUCCESS) {
        fprintf(stderr, "Unexpected success initializing Logger4 test instrument.\n");
        return EXIT_FAILURE;
    } else if (err != RBRGEN3_UNSUPPORTED) {
        fprintf(
            stderr, "Failure initializing Logger3 test instrument: %s.\n", RBRGen3Error_name(err));
        return EXIT_FAILURE;
    } else /* RBRGEN3_UNSUPPORTED */
    {
        if (RBRGen3_getGeneration(instrumentL4) != RBRCOMMON_LOGGER4) {
            fprintf(stderr,
                    "Unexpected generation Logger4 generation: %s.\n",
                    RBRCommonGeneration_name(RBRGen3_getGeneration(instrumentL4)));
            return EXIT_FAILURE;
        } else {
            printf("Successfully rejected Logger4 test instrument.\n");
        }
    }

    TestParserBuffers parserBuffers;
    RBRGEN3_SAMPLE_DECL(parserSample, TESTS_CHANNEL_MAX);
    RBRGen3Event parserEvent;
    RBRGen3ParserCallbacks parserCallbacks = {
        .sample = TestParserBuffers_sample,
        .sampleBuffer = &parserSample,
        .event = TestParserBuffers_event,
        .eventBuffer = &parserEvent,
    };

    RBRGen3Parser parserBuffer;
    RBRGen3Parser *parser = &parserBuffer;

    printf("Running tests...\n");
    int success = EXIT_SUCCESS;
    RBRGen3 *testInstrument;
    int32_t testsTotal = 0;
    int32_t testsPassed = 0;
    for (int32_t i = 0; instrumentTests[i].function != NULL; i++) {
        if (instrumentTests[i].generation == RBRCOMMON_LOGGER2) {
            testInstrument = instrumentL2;
        } else {
            testInstrument = instrumentL3;
        }

        printf("Running %s test \"%s\"...",
               RBRCommonGeneration_name(instrumentTests[i].generation),
               instrumentTests[i].name);
        ++testsTotal;
        if (instrumentTests[i].function(testInstrument, &ioBuffers)) {
            printf(" \033[32mok\033[0m\n");
            ++testsPassed;
        } else {
            printf(" \033[31mfail\033[0m\n");
            success = EXIT_FAILURE;
        }
    }

    for (int32_t i = 0; parserTests[i].function != NULL; i++) {
        printf("Running parser test \"%s\"...", parserTests[i].name);
        ++testsTotal;

        memset(&parserBuffers, 0, sizeof(TestParserBuffers));

        err = RBRGen3Parser_init(parser, &parserCallbacks, parserTests[i].config, &parserBuffers);
        if (err != RBRGEN3_SUCCESS) {
            printf(" \033[31minit fail\033[0m: %s\n", RBRGen3Error_name(err));
            success = EXIT_FAILURE;
            continue;
        }

        if (parserTests[i].function(parser, &parserBuffers)) {
            printf(" \033[32mok\033[0m\n");
            ++testsPassed;
        } else {
            printf(" \033[31mfail\033[0m\n");
            success = EXIT_FAILURE;
        }

        RBRGen3Parser_destroy(parser);
    }

    printf("Tests completed (%" PRIi32 "/%" PRIi32 " passed).\n", testsPassed, testsTotal);

    return success;
}
