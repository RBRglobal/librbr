/**
 * \file RBRInstrumentGen3Fetching.c
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

#include "RBRInstrumentGen3.h"
#include "RBRInstrumentGen3Internal.h"

RBRInstrumentGen3Error RBRInstrumentGen3_fetch(RBRInstrumentGen3 *instrument,
                                       RBRInstrumentGen3LabelsList *channels,
                                       bool sleepAfter,
                                       RBRInstrumentGen3Sample *sample)
{
    char *commandBuffer = (char *) instrument->commandBuffer;
    int32_t *commandBufferLength = &instrument->commandBufferLength;

    *commandBufferLength = snprintf(
        commandBuffer,
        sizeof(instrument->commandBuffer),
        "fetch sleepafter = %s",
        sleepAfter ? "true" : "false");

    /*
     * If we have channel labels to pass, we can quickly exceed the length of
     * the command buffer. We'll add each one to the buffer, and whenever we
     * run out of room, we'll flush.
     *
     * The description of RBRInstrumentGen3WriteCallback says:
     *
     * > The library will attempt to call this function only for complete
     * > commands
     *
     * This function is currently the only case where we have to split a single
     * command across multiple callbacks.
     */
    if (channels != NULL
        && channels->count > 0
        && instrument->generation != RBRINSTRUMENTGEN3_LOGGER2)
    {
        *commandBufferLength += snprintf(
            commandBuffer + *commandBufferLength,
            sizeof(instrument->commandBuffer) - *commandBufferLength,
            ", channels =");

        char separator = ' ';
        for (int32_t channel = 0; channel < channels->count; ++channel)
        {
            if (*commandBufferLength + 1 + strlen(channels->labels[channel])
                > sizeof(instrument->commandBuffer))
            {
                RBR_TRY(RBRInstrumentGen3_sendBuffer(instrument));
                *commandBufferLength = 0;
            }

            *commandBufferLength += snprintf(
                commandBuffer + *commandBufferLength,
                sizeof(instrument->commandBuffer) - *commandBufferLength,
                "%c%s",
                separator,
                channels->labels[channel]);
            separator = '|';
        }
    }

    if ((size_t) *commandBufferLength + RBRINSTRUMENTGEN3_SEND_COMMAND_TERMINATOR_LEN
        > sizeof(instrument->commandBuffer))
    {
        RBR_TRY(RBRInstrumentGen3_sendBuffer(instrument));
        *commandBufferLength = 0;
    }

    *commandBufferLength += snprintf(
        commandBuffer + *commandBufferLength,
        sizeof(instrument->commandBuffer) - *commandBufferLength,
        RBRINSTRUMENTGEN3_SEND_COMMAND_TERMINATOR);

    RBR_TRY(RBRInstrumentGen3_sendBuffer(instrument));

    RBRInstrumentGen3Error err;
    /* RBRInstrumentGen3_readResponse() returns #RBRINSTRUMENTGEN3_SAMPLE when a sample
     * is read to the given sample pointer; a return of #RBRINSTRUMENTGEN3_SUCCESS
     * means that it found some other command response instead, so we'll loop
     * until we get a “failure” value (which we hope is SAMPLE). */
    do
    {
        err = RBRInstrumentGen3_readResponse(instrument, true, sample);
    } while (err == RBRINSTRUMENTGEN3_SUCCESS);
    /* SAMPLE is what we were hoping for, so we'll translate to SUCCESS. Any
     * other errors can really be errors. */
    if (err == RBRINSTRUMENTGEN3_SAMPLE)
    {
        err = RBRINSTRUMENTGEN3_SUCCESS;
    }
    
    if(sleepAfter)
    {
    /* Instrument was put to sleep with "sleepAfter=true". */
        instrument->lastActivityTime = RBRINSTRUMENTGEN3_NO_ACTIVITY;
    }
    return err;
}
