/**
 * \file RBRInstrumentGen4Streaming.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for isnan, NAN. */
#include <math.h>
/* Required for strchr, strcmp. */
#include <string.h>
/* Required for snprintf. */
#include <stdio.h>

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Internal.h"
#include "RBRInstrumentGen4Streaming.h"

#define READING_FLAG_MASK 0x00FF0000
#define READING_FLAG_OFFSET (2 * 8)
#define READING_ERROR_MASK 0x0000FFFF
#define READING_ERROR_OFFSET (0 * 8)


const char *RBRInstrumentGen4ReadingFlag_name(RBRInstrumentGen4ReadingFlag flag)
{
    switch (flag)
    {
    case RBRINSTRUMENTGEN4_READING_FLAG_NONE:
        return "none";
    case RBRINSTRUMENTGEN4_READING_FLAG_UNCALIBRATED:
        return "uncalibrated";
    case RBRINSTRUMENTGEN4_READING_FLAG_ERROR:
        return "error";
    case RBRINSTRUMENTGEN4_READING_FLAG_COUNT:
        return "reading flag count";
    case RBRINSTRUMENTGEN4_UNKNOWN_READING_FLAG:
    default:
        return "unknown reading flag";
    }
}

inline RBRInstrumentGen4ReadingFlag RBRInstrumentGen4Reading_getFlag(double reading)
{
    if (!isnan(reading))
    {
        return RBRINSTRUMENTGEN4_READING_FLAG_NONE;
    }

    union
    {
        double reading;
        uint64_t raw;
    } alias;
    alias.reading = reading;

    return (alias.raw & READING_FLAG_MASK) >> READING_FLAG_OFFSET;
}

inline RBRInstrumentGen4ReadingError RBRInstrumentGen4Reading_getError(double reading)
{

    if (!isnan(reading))
    {
        return 0;
    }

    union
    {
        double reading;
        uint64_t raw;
    } alias;
    alias.reading = reading;

    uint8_t index = (alias.raw & READING_ERROR_MASK) >> READING_ERROR_OFFSET;
    return (RBRInstrumentGen4ReadingError)(index);
}

inline double RBRInstrumentGen4Reading_setError(RBRInstrumentGen4ReadingFlag flag,
                                                uint8_t error)
{
    union
    {
        double reading;
        uint64_t raw;
    } alias;
    alias.reading = (double) NAN;

    alias.raw |= ((flag << READING_FLAG_OFFSET) & READING_FLAG_MASK) | ((error << READING_ERROR_OFFSET) & READING_ERROR_MASK);

    return alias.reading;
}

RBRInstrumentGen4Error RBRInstrumentGen4_readSample(RBRInstrumentGen4 *instrument)
{
    RBRInstrumentGen4Error err;
    /* RBRInstrumentGen4_readResponse() returns #RBRINSTRUMENTGEN4_SAMPLE when a sample
     * is read to the given sample pointer; a return of #RBRINSTRUMENTGEN4_SUCCESS
     * means that it found some other command response instead, so we'll loop
     * until we get a “failure” value (which we hope is SAMPLE). */
    do
    {
        err = RBRInstrumentGen4_readResponse(instrument, true, NULL);
    } while (err == RBRINSTRUMENTGEN4_SUCCESS);
    /* SAMPLE is what we were hoping for, so we'll translate to SUCCESS. Any
     * other errors can really be errors. */
    if (err == RBRINSTRUMENTGEN4_SAMPLE)
    {
        err = RBRINSTRUMENTGEN4_SUCCESS;
    }

    return err;
}
