/**
 * \file RBRInstrumentGen4Realtime.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for snprintf. */
#include <stdio.h>

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Internal.h"
#include "RBRInstrumentGen4Realtime.h"

/**
 * \brief Send a poll command and read the resulting sample.
 *
 * \param [in] instrument the instrument connection
 * \param [in] parameter the list parameter to send, or `NULL` for a bare
 *                       `poll`
 * \param [in] list the value of \a parameter
 * \param [out] sample the polled sample
 */
static RBRInstrumentGen4Error RBRInstrumentGen4_sendPoll(
    RBRInstrumentGen4 *instrument,
    const char *parameter,
    const char *list,
    RBRInstrumentGen4Sample *sample)
{
    char *commandBuffer = (char *) instrument->commandBuffer;
    int32_t *commandBufferLength = &instrument->commandBufferLength;

    *commandBufferLength = snprintf(
        commandBuffer,
        sizeof(instrument->commandBuffer),
        "poll%s%s%s%s",
        parameter != NULL ? " " : "",
        parameter != NULL ? parameter : "",
        parameter != NULL ? list : "",
        RBRINSTRUMENTGEN4_SEND_COMMAND_TERMINATOR);
    if ((size_t) *commandBufferLength >= sizeof(instrument->commandBuffer))
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    RBR_TRY(RBRInstrumentGen4_sendBuffer(instrument));

    RBRInstrumentGen4Error err;
    RBRInstrumentGen4DateTime now;
    /* RBRInstrumentGen4_readResponse() returns #RBRINSTRUMENTGEN4_SAMPLE when
     * a sample is read to the given sample pointer; a return of
     * #RBRINSTRUMENTGEN4_SUCCESS means that it found some other command
     * response instead, so we'll loop until we get a “failure” value (which
     * we hope is SAMPLE). */
    do
    {
        RBR_TRY(instrument->callbacks.time(instrument, &now));
        err = RBRInstrumentGen4_readResponse(instrument,
                                             true,
                                             sample,
                                             now,
                                             instrument->pollTimeout);
    } while (err == RBRINSTRUMENTGEN4_SUCCESS);
    /* SAMPLE is what we were hoping for, so we'll translate to SUCCESS. Any
     * other errors can really be errors. */
    if (err == RBRINSTRUMENTGEN4_SAMPLE)
    {
        err = RBRINSTRUMENTGEN4_SUCCESS;
    }

    return err;
}

RBRInstrumentGen4Error RBRInstrumentGen4_poll(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Sample *sample)
{
    return RBRInstrumentGen4_sendPoll(instrument, NULL, NULL, sample);
}

RBRInstrumentGen4Error RBRInstrumentGen4_pollChannels(
    RBRInstrumentGen4 *instrument,
    const char *channelList,
    RBRInstrumentGen4Sample *sample)
{
    return RBRInstrumentGen4_sendPoll(instrument,
                                      "channellist=",
                                      channelList,
                                      sample);
}

RBRInstrumentGen4Error RBRInstrumentGen4_pollGroups(
    RBRInstrumentGen4 *instrument,
    const char *groupList,
    RBRInstrumentGen4Sample *sample)
{
    return RBRInstrumentGen4_sendPoll(instrument,
                                      "grouplist=",
                                      groupList,
                                      sample);
}
