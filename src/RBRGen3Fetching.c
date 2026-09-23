/**
 * \file RBRGen3Fetching.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for snprintf. */
#include <stdio.h>
/* Required for memset. */
#include <string.h>

#include "RBRGen3.h"
#include "RBRGen3Internal.h"
#include "RBRGen3Fetching.h"
#include "RBRGen3Streaming.h"

RBRGen3Error RBRGen3_fetch(RBRGen3 *conn, RBRGen3LabelsList *channels, bool sleepAfter,
                           RBRGen3Sample *sample)
{
    if (sample->readings == NULL || sample->size <= 0) {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }
    char *commandBuffer = (char *) conn->environment.command;
    int32_t *commandBufferLength = &conn->commandBufferLength;

    *commandBufferLength = snprintf(commandBuffer,
                                    (size_t) conn->environment.commandCapacity,
                                    "fetch sleepafter = %s",
                                    sleepAfter ? "true" : "false");

    /*
     * If we have channel labels to pass, we can quickly exceed the length of
     * the command buffer. We'll add each one to the buffer, and whenever we
     * run out of room, we'll flush.
     *
     * The description of RBRGen3WriteCallback says:
     *
     * > The library will attempt to call this function only for complete
     * > commands
     *
     * This function is currently the only case where we have to split a single
     * command across multiple callbacks.
     */
    if (channels != NULL && channels->count > 0 && conn->generation != RBRCOMMON_LOGGER2) {
        /* Each label is written with its separator in one piece, so a label
         * which cannot fit an empty buffer must be refused before any part of
         * the command has gone out. */
        for (int32_t channel = 0; channel < channels->count; ++channel) {
            if (1 + strlen(channels->labels[channel]) >=
                (size_t) conn->environment.commandCapacity) {
                return RBRGEN3_BUFFER_TOO_SMALL;
            }
        }

        const char channelsParameter[] = ", channels =";
        if (*commandBufferLength + (int32_t) sizeof(channelsParameter) >
            conn->environment.commandCapacity) {
            RBR_TRY(RBRGen3_sendBuffer(conn));
            *commandBufferLength = 0;
        }
        *commandBufferLength +=
            snprintf(commandBuffer + *commandBufferLength,
                     (size_t) (conn->environment.commandCapacity - *commandBufferLength),
                     "%s",
                     channelsParameter);

        char separator = ' ';
        for (int32_t channel = 0; channel < channels->count; ++channel) {
            /* snprintf() needs a byte for its null beyond the piece. */
            if ((size_t) *commandBufferLength + 1 + strlen(channels->labels[channel]) >=
                (size_t) conn->environment.commandCapacity) {
                RBR_TRY(RBRGen3_sendBuffer(conn));
                *commandBufferLength = 0;
            }

            *commandBufferLength +=
                snprintf(commandBuffer + *commandBufferLength,
                         (size_t) (conn->environment.commandCapacity - *commandBufferLength),
                         "%c%s",
                         separator,
                         channels->labels[channel]);
            separator = '|';
        }
    }

    if ((size_t) *commandBufferLength + RBRGEN3_SEND_COMMAND_TERMINATOR_LEN >=
        (size_t) conn->environment.commandCapacity) {
        RBR_TRY(RBRGen3_sendBuffer(conn));
        *commandBufferLength = 0;
    }

    *commandBufferLength +=
        snprintf(commandBuffer + *commandBufferLength,
                 (size_t) (conn->environment.commandCapacity - *commandBufferLength),
                 RBRGEN3_SEND_COMMAND_TERMINATOR);

    RBR_TRY(RBRGen3_sendBuffer(conn));

    RBRGen3Error err;
    /* RBRGen3_readResponse() returns #RBRGEN3_SAMPLE when a sample
     * is read to the given sample pointer; a return of #RBRGEN3_SUCCESS
     * means that it found some other command response instead, so we'll loop
     * until we get a “failure” value (which we hope is SAMPLE). */
    do {
        err = RBRGen3_readResponse(conn, true, sample);
    } while (err == RBRGEN3_SUCCESS);
    /* SAMPLE is what we were hoping for, so we'll translate to SUCCESS. Any
     * other errors can really be errors. */
    if (err == RBRGEN3_SAMPLE) {
        err = RBRGEN3_SUCCESS;
    }

    if (sleepAfter) {
        /* Instrument was put to sleep with "sleepAfter=true". */
        conn->lastActivityTime = RBRGEN3_NO_ACTIVITY;
    }
    return err;
}
