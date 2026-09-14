/**
 * \file posix-test.c
 *
 * \brief Test file for posix examples development, using simulated I/O
 * buffers instead of a live instrument.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for errno. */
#include <errno.h>
/* Required for isnan. */
#include <math.h>
/* Required for fprintf, printf, snprintf. */
#include <stdio.h>
/* Required for EXIT_SUCCESS, etc. */
#include <stdlib.h>
/* Required for strerror. */
#include <string.h>
/* Required for close. */
#include <unistd.h>

#include "posix-shared.h"

#define RESPONSE_TERMINATOR "\r\n"
/**
 * \brief The I/O buffers used for tests.
 */
typedef struct TestIOBuffers
{
    /** \brief The instrument under test will read from this buffer. */
    const char *readBuffer;
    /** \brief The size of the test read buffer. */
    int32_t readBufferSize;
    /** \brief How far into the read buffer the instrument has read. */
    int32_t readBufferPos;
    /** \brief The instrument under test will write back into this buffer. */
    char writeBuffer[4096];
    /** \brief How far into the write buffer the instrument has written. */
    int32_t writeBufferPos;
    /** \brief The last sample received from the test instrument. */
    RBRGen4Sample streamSample;
} TestIOBuffers;

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
    (void)instrument;
    (void)time;
    *time = 0;
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
            __FILE__,
            __LINE__,
            *size);
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

    int32_t remaining = 4096 - buffers->writeBufferPos;
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

int main(void)
{
    RBRGen4 *instrument = NULL;
    RBRGen4 instrumentSpace;
    instrument = &instrumentSpace;
    TestIOBuffers ioBuffers;
    int status = EXIT_SUCCESS;
    RBRGen4Error err;

    RBRGen4Callbacks instrumentCallbacks = {
        .time = TestIOBuffers_time,
        .sleep = TestIOBuffers_sleep,
        .read = TestIOBuffers_read,
        .write = TestIOBuffers_write,
        .sample = TestIOBuffers_sample,
        .sampleBuffer = &ioBuffers.streamSample
    };

    TestIOBuffers_init(
    &ioBuffers,
    "id model = RBRconcerto4, version = 1.14.5+202310150927, serial = 092431, fwtype = 130"
    RESPONSE_TERMINATOR,
    0);

    err = RBRGen4_open(&instrument,
                             &instrumentCallbacks,
                             /* command timeout */ 0,
                             &ioBuffers);
    if (err != RBRGEN4_SUCCESS)
    {
        fprintf(stderr,
                "Failure to open instrument: %s.\n",
                RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    // TestIOBuffers_init(
    //     &ioBuffers,
    //         "2000-01-01 03:22:42.000, -129.993424e+000, 349.649536e-003, "
    //         "500.022304e-003" RESPONSE_TERMINATOR,
    //     0);

        // TestIOBuffers_init(
        // &ioBuffers,
        //     "RBR 999999, 2000-01-01 03:22:42.000, -129.993424e+000, 349.649536e-003, "
        //     "500.022304e-003" RESPONSE_TERMINATOR,
        // 0);

        TestIOBuffers_init(
            &ioBuffers,
            "verify config = profiling, dataset = test, simulation = off, "
            "storageMode = normal, status = pending, warning = none" RESPONSE_TERMINATOR,
        0);

        RBRGen4Config config = {
            .label = "profiling"
        };
        char datasetLabel[] = "test";
        RBRInstrumentGen4InstrumentState verifyStatus
            = RBRINSTRUMENTGEN4_UNKNOWN_INSTRUMENT_STATE;

   if ((err = RBRGen4_verify(
                  instrument,
                  &config,
                  datasetLabel,
                  RBRGEN4_STORAGEMODE_NORMAL,
                  &verifyStatus)) != RBRGEN4_SUCCESS)
    {
        fprintf(stderr, "./posix-test.c: %s!\n",
                RBRGen4Error_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup; //Failure case, memory allocated by this constructor is freed.
    }

fileCleanup:
    return status;
}
