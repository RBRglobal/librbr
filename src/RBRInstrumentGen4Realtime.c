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
/* Required for strcmp. */
#include <string.h>

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Internal.h"
#include "RBRInstrumentGen4Realtime.h"

/** \brief The schedule label reported with a polled (as opposed to
 * streamed) sample. */
#define RBRINSTRUMENTGEN4_POLL_SCHEDULE_LABEL "polling"

/**
 * \brief Send a poll command and read the resulting sample.
 *
 * \param [in] instrument the instrument connection
 * \param [in] requireLabel whether to require and wait for a sample
 *                          labelled #RBRINSTRUMENTGEN4_POLL_SCHEDULE_LABEL
 * \param [in] parameter the list parameter to send, or `NULL` for a bare
 *                       `poll`
 * \param [in] list the value of \a parameter
 * \param [out] sample the polled sample
 */
static RBRInstrumentGen4Error RBRInstrumentGen4_sendPoll(
    RBRInstrumentGen4 *instrument,
    bool requireLabel,
    const char *parameter,
    const char *list,
    RBRInstrumentGen4Sample *sample)
{
    if (requireLabel && !instrument->outputFormat.scheduleLabel)
    {
        return RBRINSTRUMENTGEN4_UNSUPPORTED;
    }

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

    RBRInstrumentGen4DateTime start;
    RBR_TRY(instrument->callbacks.time(instrument, &start));

    RBR_TRY(RBRInstrumentGen4_sendBuffer(instrument));

    RBRInstrumentGen4Error err;
    /* RBRInstrumentGen4_readResponse() returns #RBRINSTRUMENTGEN4_SAMPLE when
     * a sample is read to the given sample pointer; a return of
     * #RBRINSTRUMENTGEN4_SUCCESS means that it found some other command
     * response instead, so we'll loop until we get a “failure” value (which
     * we hope is SAMPLE). */
    do
    {
        err = RBRInstrumentGen4_readResponse(instrument,
                                             true,
                                             sample,
                                             start,
                                             instrument->pollTimeout);
        if (err == RBRINSTRUMENTGEN4_SAMPLE
            && requireLabel
            && 0 != strcmp(sample->scheduleLabel,
                           RBRINSTRUMENTGEN4_POLL_SCHEDULE_LABEL))
        {
            /* This is a streamed sample, not the polled one we're waiting
             * for. Forward it to the sample callback, if any, and keep
             * looking. */
            RBR_TRY(RBRInstrumentGen4_deliverSample(instrument, sample));
            err = RBRINSTRUMENTGEN4_SUCCESS;
        }
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
    bool requireLabel,
    RBRInstrumentGen4Sample *sample)
{
    return RBRInstrumentGen4_sendPoll(instrument,
                                      requireLabel,
                                      NULL,
                                      NULL,
                                      sample);
}

RBRInstrumentGen4Error RBRInstrumentGen4_pollChannels(
    RBRInstrumentGen4 *instrument,
    bool requireLabel,
    const char *channelList,
    RBRInstrumentGen4Sample *sample)
{
    return RBRInstrumentGen4_sendPoll(instrument,
                                      requireLabel,
                                      "channellist=",
                                      channelList,
                                      sample);
}

RBRInstrumentGen4Error RBRInstrumentGen4_pollGroups(
    RBRInstrumentGen4 *instrument,
    bool requireLabel,
    const char *groupList,
    RBRInstrumentGen4Sample *sample)
{
    return RBRInstrumentGen4_sendPoll(instrument,
                                      requireLabel,
                                      "grouplist=",
                                      groupList,
                                      sample);
}
