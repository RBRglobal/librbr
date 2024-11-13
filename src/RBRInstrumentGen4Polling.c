/**
 * \file RBRInstrumentGen4Polling.c
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

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Internal.h"
#include "RBRInstrumentGen4Polling.h"

RBRInstrumentGen4Error RBRInstrumentGen4_pollOneChannel(RBRInstrumentGen4 *instrument,
                                       const char *channelLabel,
                                       RBRInstrumentGen4Sample *sample)
{
    char *commandBuffer = (char *) instrument->commandBuffer;
    int32_t *commandBufferLength = &instrument->commandBufferLength;

    //Max channel name is 31 characters. (RBRINSTRMENTGEN4_CHANNEL_LABEL_MAX)
    //Then maximum length for this command is 63 chars + terminator. It won't exceed the instrument->commandBuffer[120]
    *commandBufferLength = snprintf(
        commandBuffer,
        sizeof(instrument->commandBuffer),
        "poll channellist=%s%s",
        channelLabel, RBRINSTRUMENTGEN4_SEND_COMMAND_TERMINATOR);

    RBR_TRY(RBRInstrumentGen4_sendBuffer(instrument));

    RBRInstrumentGen4Error err;
    /* RBRInstrumentGen4_readResponse() returns #RBRINSTRUMENT_SAMPLE when a sample
     * is read to the given sample pointer; a return of #RBRINSTRUMENT_SUCCESS
     * means that it found some other command response instead, so we'll loop
     * until we get a “failure” value (which we hope is SAMPLE). */
    do
    {
        err = RBRInstrumentGen4_readResponse(instrument, true, sample);
    } while (err == RBRINSTRUMENTGEN4_SUCCESS);
    /* SAMPLE is what we were hoping for, so we'll translate to SUCCESS. Any
     * other errors can really be errors. */
    if (err == RBRINSTRUMENTGEN4_SAMPLE)
    {
        err = RBRINSTRUMENTGEN4_SUCCESS;
    }

    return err;
}

RBRInstrumentGen4Error RBRInstrumentGen4_pollOneGroup(RBRInstrumentGen4 *instrument,
                                       const char *groupLabel,
                                       RBRInstrumentGen4Sample *sample)
{
    char *commandBuffer = (char *) instrument->commandBuffer;
    int32_t *commandBufferLength = &instrument->commandBufferLength;

    //Max channel name is 31 characters. (RBRINSTRMENTGEN4_CHANNEL_LABEL_MAX)
    //Then maximum length for this command is 63 chars + terminator. It won't exceed the instrument->commandBuffer[120]
    *commandBufferLength = snprintf(
        commandBuffer,
        sizeof(instrument->commandBuffer),
        "poll grouplist=%s%s",
        groupLabel, RBRINSTRUMENTGEN4_SEND_COMMAND_TERMINATOR);

    RBR_TRY(RBRInstrumentGen4_sendBuffer(instrument));

    RBRInstrumentGen4Error err;
    /* RBRInstrumentGen4_readResponse() returns #RBRINSTRUMENT_SAMPLE when a sample
     * is read to the given sample pointer; a return of #RBRINSTRUMENT_SUCCESS
     * means that it found some other command response instead, so we'll loop
     * until we get a “failure” value (which we hope is SAMPLE). */
    do
    {
        err = RBRInstrumentGen4_readResponse(instrument, true, sample);
    } while (err == RBRINSTRUMENTGEN4_SUCCESS);
    /* SAMPLE is what we were hoping for, so we'll translate to SUCCESS. Any
     * other errors can really be errors. */
    if (err == RBRINSTRUMENTGEN4_SAMPLE)
    {
        err = RBRINSTRUMENTGEN4_SUCCESS;
    }

    return err;
}

RBRInstrumentGen4Error RBRInstrumentGen4_pollAllChannels(RBRInstrumentGen4 *instrument,
                                       RBRInstrumentGen4Sample *sample)
{
    char *commandBuffer = (char *) instrument->commandBuffer;
    int32_t *commandBufferLength = &instrument->commandBufferLength;

    //maximum length for this command is 35 chars + terminator. It won't exceed the instrument->commandBuffer[120]
    *commandBufferLength = snprintf(
        commandBuffer,
        sizeof(instrument->commandBuffer),
        "poll channellist=all%s",
        RBRINSTRUMENTGEN4_SEND_COMMAND_TERMINATOR);

    RBR_TRY(RBRInstrumentGen4_sendBuffer(instrument));

    RBRInstrumentGen4Error err;
    /* RBRInstrumentGen4_readResponse() returns #RBRINSTRUMENT_SAMPLE when a sample
     * is read to the given sample pointer; a return of #RBRINSTRUMENT_SUCCESS
     * means that it found some other command response instead, so we'll loop
     * until we get a “failure” value (which we hope is SAMPLE). */
    do
    {
        err = RBRInstrumentGen4_readResponse(instrument, true, sample);
    } while (err == RBRINSTRUMENTGEN4_SUCCESS);
    /* SAMPLE is what we were hoping for, so we'll translate to SUCCESS. Any
     * other errors can really be errors. */
    if (err == RBRINSTRUMENTGEN4_SAMPLE)
    {
        err = RBRINSTRUMENTGEN4_SUCCESS;
    }
    return err;
}