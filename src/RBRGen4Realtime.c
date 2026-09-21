/**
 * \file RBRGen4Realtime.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for isnan, NAN. */
#include <math.h>
/* Required for snprintf. */
#include <stdio.h>
/* Required for strcmp. */
#include <string.h>

#include "RBRGen4.h"
#include "RBRGen4Internal.h"
#include "RBRGen4Realtime.h"

#define READING_FLAG_MASK    0x00FF0000
/** \brief Marks a NaN as an error reading rather than a plain NaN. */
#define READING_ERROR_FLAG   0x00010000
#define READING_ERROR_MASK   0x0000FFFF
#define READING_ERROR_OFFSET (0 * 8)

/** \brief The schedule label reported with a polled (as opposed to
 * streamed) sample. */
#define RBRGEN4_POLL_SCHEDULE_LABEL "polling"

inline bool RBRGen4Reading_isError(double reading)
{
    if (!isnan(reading)) {
        return false;
    }

    union {
        double reading;
        uint64_t raw;
    } alias;
    alias.reading = reading;

    return (alias.raw & READING_FLAG_MASK) != 0;
}

inline RBRGen4ReadingError RBRGen4Reading_getError(double reading)
{

    if (!isnan(reading)) {
        return 0;
    }

    union {
        double reading;
        uint64_t raw;
    } alias;
    alias.reading = reading;

    uint8_t index = (alias.raw & READING_ERROR_MASK) >> READING_ERROR_OFFSET;
    return (RBRGen4ReadingError) (index);
}

inline double RBRGen4Reading_setError(RBRGen4ReadingError error)
{
    union {
        double reading;
        uint64_t raw;
    } alias;
    alias.reading = (double) NAN;

    alias.raw |=
        READING_ERROR_FLAG | (((uint64_t) error << READING_ERROR_OFFSET) & READING_ERROR_MASK);

    return alias.reading;
}

RBRGen4Error RBRGen4_readSample(RBRGen4 *conn)
{
    if (conn->callbacks.sample == NULL) {
        return RBRGEN4_MISSING_CALLBACK;
    }

    RBRGen4Error err;
    RBRGen4DateTime now;
    /* RBRGen4_readResponse() returns #RBRGEN4_SAMPLE when a sample
     * is read to the given sample pointer; a return of #RBRGEN4_SUCCESS
     * means that it found some other command response instead, so we'll loop
     * until we get a “failure” value (which we hope is SAMPLE). */
    do {
        RBR_TRY(conn->callbacks.time(conn, &now));
        err = RBRGen4_readResponse(conn, true, NULL, now, conn->commandTimeout);
    } while (err == RBRGEN4_SUCCESS);
    /* SAMPLE is what we were hoping for, so we'll translate to SUCCESS. Any
     * other errors can really be errors. */
    if (err == RBRGEN4_SAMPLE) {
        err = RBRGEN4_SUCCESS;
    }

    return err;
}

/**
 * \brief Send a poll command and read the resulting sample.
 *
 * \param [in] conn the instrument connection
 * \param [in] requireLabel whether to require and wait for a sample
 *                          labelled #RBRGEN4_POLL_SCHEDULE_LABEL
 * \param [in] parameter the list parameter to send, or `NULL` for a bare
 *                       `poll`
 * \param [in] list the labels to send as the value of \a parameter
 * \param [out] sample the polled sample
 */
static RBRGen4Error RBRGen4_sendPoll(RBRGen4 *conn, bool requireLabel, const char *parameter,
                                     const RBRGen4LabelList *list, RBRGen4Sample *sample)
{
    if (requireLabel && !conn->outputFormat.scheduleLabel) {
        return RBRGEN4_UNSUPPORTED;
    }

    /* Sized against the longest parameter so that a list which fits here also
     * fits the command. */
    char value[RBRGEN4_COMMAND_BUFFER_MAX -
               sizeof("poll channellist=" RBRGEN4_SEND_COMMAND_TERMINATOR)] = "";
    if (parameter != NULL) {
        /* An empty list is invalid */
        if (list == NULL || list->count == 0) {
            return RBRGEN4_INVALID_PARAMETER_VALUE;
        }

        RBR_TRY(RBRGen4_formatLabelList(value, (int32_t) sizeof(value), list));
    }

    char *commandBuffer = (char *) conn->commandBuffer;
    int32_t *commandBufferLength = &conn->commandBufferLength;

    *commandBufferLength = snprintf(commandBuffer,
                                    sizeof(conn->commandBuffer),
                                    "poll%s%s%s%s",
                                    parameter != NULL ? " " : "",
                                    parameter != NULL ? parameter : "",
                                    value,
                                    RBRGEN4_SEND_COMMAND_TERMINATOR);
    if ((size_t) *commandBufferLength >= sizeof(conn->commandBuffer)) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    RBRGen4DateTime start;
    RBR_TRY(conn->callbacks.time(conn, &start));

    RBR_TRY(RBRGen4_sendBuffer(conn));

    RBRGen4Error err;
    /* RBRGen4_readResponse() returns #RBRGEN4_SAMPLE when
     * a sample is read to the given sample pointer; a return of
     * #RBRGEN4_SUCCESS means that it found some other command
     * response instead, so we'll loop until we get a “failure” value (which
     * we hope is SAMPLE). */
    do {
        err = RBRGen4_readResponse(conn, true, sample, start, conn->pollTimeout);
        if (err == RBRGEN4_SAMPLE && requireLabel &&
            0 != strcmp(sample->scheduleLabel, RBRGEN4_POLL_SCHEDULE_LABEL)) {
            /* This is a streamed sample, not the polled one we're waiting
             * for. Forward it to the sample callback, if any, and keep
             * looking. */
            RBR_TRY(RBRGen4_deliverSample(conn, sample));
            err = RBRGEN4_SUCCESS;
        }
    } while (err == RBRGEN4_SUCCESS);
    /* SAMPLE is what we were hoping for, so we'll translate to SUCCESS. Any
     * other errors can really be errors. */
    if (err == RBRGEN4_SAMPLE) {
        err = RBRGEN4_SUCCESS;
    }

    return err;
}

RBRGen4Error RBRGen4_poll(RBRGen4 *conn, bool requireLabel, RBRGen4Sample *sample)
{
    return RBRGen4_sendPoll(conn, requireLabel, NULL, NULL, sample);
}

RBRGen4Error RBRGen4_pollChannels(RBRGen4 *conn, bool requireLabel,
                                  const RBRGen4LabelList *channelList, RBRGen4Sample *sample)
{
    return RBRGen4_sendPoll(conn, requireLabel, "channellist=", channelList, sample);
}

RBRGen4Error RBRGen4_pollGroups(RBRGen4 *conn, bool requireLabel, const RBRGen4LabelList *groupList,
                                RBRGen4Sample *sample)
{
    return RBRGen4_sendPoll(conn, requireLabel, "grouplist=", groupList, sample);
}
