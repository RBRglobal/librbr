/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file RBRGen4Realtime.c
 *
 * \brief Library implementation.
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
    if (conn->environment.sample == NULL) {
        return RBRGEN4_MISSING_CALLBACK;
    }

    RBRGen4Error err;
    RBRGen4DateTime now;
    /* RBRGen4_readResponse() returns #RBRGEN4_SAMPLE when a sample
     * is read to the given sample pointer; a return of #RBRGEN4_SUCCESS
     * means that it found some other command response instead, so we'll loop
     * until we get a “failure” value (which we hope is SAMPLE). */
    do {
        RBR_TRY(conn->environment.time(conn, &now));
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
 * \brief Copy a sample into another with its own readings storage.
 *
 * Readings which do not fit the destination are dropped and flagged.
 */
static void RBRGen4Sample_copy(RBRGen4Sample *to, const RBRGen4Sample *from)
{
    to->timestamp = from->timestamp;
    memcpy(to->scheduleLabel, from->scheduleLabel, sizeof(to->scheduleLabel));
    to->channelCount = from->channelCount < to->size ? from->channelCount : to->size;
    to->readingsDropped = from->readingsDropped || to->channelCount < from->channelCount;
    memcpy(to->readings, from->readings, (size_t) to->channelCount * sizeof(*to->readings));
    memset(to->readings + to->channelCount,
           0,
           (size_t) (to->size - to->channelCount) * sizeof(*to->readings));
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
    if (sample->readings == NULL || sample->size <= 0) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }
    if (requireLabel && !conn->outputFormat.scheduleLabel) {
        return RBRGEN4_UNSUPPORTED;
    }

    RBRGen4_beginCommand(conn);
    RBR_TRY(RBRGen4_appendCommand(conn, "poll"));
    if (parameter != NULL) {
        /* An empty list is invalid */
        if (list == NULL || list->len == 0) {
            return RBRGEN4_INVALID_PARAMETER_VALUE;
        }

        RBR_TRY(RBRGen4_appendCommand(conn, " %s", parameter));
        RBR_TRY(RBRGen4_appendLabelList(conn, list));
    }
    RBR_TRY(RBRGen4_appendCommand(conn, RBRGEN4_SEND_COMMAND_TERMINATOR));

    RBRGen4DateTime start;
    RBR_TRY(conn->environment.time(conn, &start));

    RBR_TRY(RBRGen4_sendBuffer(conn));

    RBRGen4Error err;
    /* RBRGen4_readResponse() returns #RBRGEN4_SAMPLE when
     * a sample is read to the given sample pointer; a return of
     * #RBRGEN4_SUCCESS means that it found some other command
     * response instead, so we'll loop until we get a “failure” value (which
     * we hope is SAMPLE). */
    /* Streamed samples met while waiting are forwarded to the sample
     * callback. Parse into whichever of the two samples has the larger
     * readings storage, then copy into the other, so that neither loses
     * readings the other had room for. */
    RBRGen4Sample *callbackSample = conn->environment.sampleBuffer;
    RBRGen4Sample *target = sample;
    if (conn->environment.sample != NULL && callbackSample->size > sample->size) {
        target = callbackSample;
    }
    do {
        err = RBRGen4_readResponse(conn, true, target, start, conn->pollTimeout);
        if (err != RBRGEN4_SAMPLE) {
            continue;
        }
        if (requireLabel && 0 != strcmp(target->scheduleLabel, RBRGEN4_POLL_SCHEDULE_LABEL)) {
            /* This is a streamed sample, not the polled one we're waiting
             * for. Forward it to the sample callback, if any, and keep
             * looking. */
            if (conn->environment.sample != NULL) {
                if (target != callbackSample) {
                    RBRGen4Sample_copy(callbackSample, target);
                }
                RBR_TRY(conn->environment.sample(conn, callbackSample));
            }
            err = RBRGEN4_SUCCESS;
        } else if (target != sample) {
            RBRGen4Sample_copy(sample, target);
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
