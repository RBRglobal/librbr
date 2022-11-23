/**
 * \file RBRInstrument.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for memcpy, memcmp, memset, strlen. */
#include <string.h>
/* Required for free/malloc. */
#include <stdlib.h>

#include "RBRInstrument.h"
#include "RBRInstrumentInternal.h"

const char *RBRINSTRUMENT_LIB_NAME =
#ifdef RBR_LIB_NAME
    RBR_LIB_NAME
#else
    "libRBR"
#endif
;

const char *RBRINSTRUMENT_LIB_VERSION =
#ifdef RBR_LIB_VERSION
    RBR_LIB_VERSION
#else
    "unknown"
#endif
;

const char *RBRINSTRUMENT_LIB_BUILD_DATE =
#ifdef RBR_LIB_BUILD_DATE
    RBR_LIB_BUILD_DATE
#else
    "unknown"
#endif
;

const char *RBRInstrumentError_name(RBRInstrumentError error)
{
    switch (error)
    {
    case RBRINSTRUMENT_SUCCESS:
        return "success";
    case RBRINSTRUMENT_ALLOCATION_FAILURE:
        return "allocation failure";
    case RBRINSTRUMENT_BUFFER_TOO_SMALL:
        return "buffer too small";
    case RBRINSTRUMENT_MISSING_CALLBACK:
        return "missing callback";
    case RBRINSTRUMENT_CALLBACK_ERROR:
        return "callback error";
    case RBRINSTRUMENT_TIMEOUT:
        return "timeout";
    case RBRINSTRUMENT_UNSUPPORTED:
        return "unsupported";
    case RBRINSTRUMENT_HARDWARE_ERROR:
        return "hardware error";
    case RBRINSTRUMENT_CHECKSUM_ERROR:
        return "checksum error";
    case RBRINSTRUMENT_INVALID_PARAMETER_VALUE:
        return "invalid parameter value";
    case RBRINSTRUMENT_SAMPLE:
        return "sample";
    case RBRINSTRUMENT_ERROR_COUNT:
        return "error count";
    case RBRINSTRUMENT_UNKNOWN_ERROR:
    default:
        return "unknown error";
    }
}

const char *RBRInstrumentGeneration_name(RBRInstrumentGeneration generation)
{
    switch (generation)
    {
    case RBRINSTRUMENT_LOGGER1:
        return "Logger1";
    case RBRINSTRUMENT_LOGGER2:
        return "Logger2";
    case RBRINSTRUMENT_LOGGER3:
        return "Logger3";
    case RBRINSTRUMENT_GENERATION_COUNT:
        return "generation count";
    case RBRINSTRUMENT_UNKNOWN_GENERATION:
    default:
        return "unknown generation";
    }
}

const char *RBRInstrumentResponseType_name(RBRInstrumentResponseType type)
{
    switch (type)
    {
    case RBRINSTRUMENT_RESPONSE_INFO:
        return "info";
    case RBRINSTRUMENT_RESPONSE_WARNING:
        return "warning";
    case RBRINSTRUMENT_RESPONSE_ERROR:
        return "error";
    case RBRINSTRUMENT_RESPONSE_TYPE_COUNT:
        return "response type count";
    case RBRINSTRUMENT_RESPONSE_UNKNOWN_TYPE:
    default:
        return "unknown response type";
    }
}

static RBRInstrumentError RBRInstrument_populateGeneration(
    RBRInstrument *instrument)
{
    instrument->generation = RBRINSTRUMENT_UNKNOWN_GENERATION;

    /* If this isn't an RBR instrument, it'll just time out or the response
     * won't match. */
    RBRInstrumentError err = RBRInstrument_getId(instrument, &instrument->id);
    if (err != RBRINSTRUMENT_SUCCESS)
    {
        return RBRINSTRUMENT_UNSUPPORTED;
    }

    /* The concept of firmware type was introduced part-way through Logger2, so
     * early instruments with very old firmware won't report a firmware type.
     * Newer firmware versions and newer instruments within the generation will
     * report a firmware type of 100–103. */
    if (instrument->id.fwtype == 0
        || (instrument->id.fwtype >= 100
            && instrument->id.fwtype <= 103))
    {
        instrument->generation = RBRINSTRUMENT_LOGGER2;
    }
    else
    {
        instrument->generation = RBRINSTRUMENT_LOGGER3;
    }
    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_open(RBRInstrument **instrument,
                                      const RBRInstrumentCallbacks *callbacks,
                                      RBRInstrumentDateTime commandTimeout,
                                      void *userData)
{
    if (callbacks == NULL
        || callbacks->time == NULL
        || callbacks->sleep == NULL
        || callbacks->read == NULL
        || callbacks->write == NULL
        || (callbacks->sample != NULL && callbacks->sampleBuffer == NULL))
    {
        return RBRINSTRUMENT_MISSING_CALLBACK;
    }

    bool allocated = false;
    if (*instrument == NULL)
    {
        allocated = true;
        if ((*instrument = malloc(sizeof(RBRInstrument))) == NULL)
        {
            return RBRINSTRUMENT_ALLOCATION_FAILURE;
        }
    }

    memset(*instrument, 0, sizeof(RBRInstrument));
    memcpy(&(*instrument)->callbacks,
           callbacks,
           sizeof(RBRInstrumentCallbacks));
    /* We don't want the streaming sample data callback to be called before the
     * constructor has finished. */
    (*instrument)->callbacks.sample  = NULL;
    (*instrument)->commandTimeout    = commandTimeout;
    (*instrument)->userData          = userData;
    (*instrument)->lastActivityTime  = RBRINSTRUMENT_NO_ACTIVITY;
    (*instrument)->response.type     = RBRINSTRUMENT_RESPONSE_UNKNOWN_TYPE;
    (*instrument)->managedAllocation = allocated;

    RBRInstrumentError err;
    err = RBRInstrument_populateGeneration(*instrument);
    if (err != RBRINSTRUMENT_SUCCESS)
    {
        if (allocated)
        {
            free(*instrument);
        }
        return err;
    }

    if ((*instrument)->generation != RBRINSTRUMENT_LOGGER2
        && (*instrument)->generation != RBRINSTRUMENT_LOGGER3)
    {
        if (allocated)
        {
            free(*instrument);
        }
        return RBRINSTRUMENT_UNSUPPORTED;
    }

    /* Enable the streaming callback, if applicable. */
    (*instrument)->callbacks.sample = callbacks->sample;
    (*instrument)->callbacks.sampleBuffer = callbacks->sampleBuffer;

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_close(RBRInstrument *instrument)
{
    if (instrument->managedAllocation)
    {
        free(instrument);
    }

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentGeneration RBRInstrument_getGeneration(
    const RBRInstrument *instrument)
{
    return instrument->generation;
}

RBRInstrumentDateTime RBRInstrument_getCommandTimeout(
    const RBRInstrument *instrument)
{
    return instrument->commandTimeout;
}

void RBRInstrument_setCommandTimeout(RBRInstrument *instrument,
                                     RBRInstrumentDateTime commandTimeout)
{
    instrument->commandTimeout = commandTimeout;
}

void *RBRInstrument_getUserData(const RBRInstrument *instrument)
{
    return instrument->userData;
}

void RBRInstrument_setUserData(RBRInstrument *instrument, void *userData)
{
    instrument->userData = userData;
}

RBRInstrumentHardwareError RBRInstrument_getLastHardwareError(
    const RBRInstrument *instrument)
{
    if (instrument->response.type == RBRINSTRUMENT_RESPONSE_ERROR
        || instrument->response.type == RBRINSTRUMENT_RESPONSE_WARNING)
    {
        return instrument->response.error;
    }
    else
    {
        return RBRINSTRUMENT_HARDWARE_ERROR_NONE;
    }
}

const char *RBRInstrument_getLastHardwareErrorMessage(
    const RBRInstrument *instrument)
{
    if (instrument->response.type == RBRINSTRUMENT_RESPONSE_ERROR)
    {
        return instrument->response.response;
    }
    else
    {
        return NULL;
    }
}
