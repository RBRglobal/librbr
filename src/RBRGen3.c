/**
 * \file RBRGen3.c
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
#ifndef RBR_LIB_NODYNAMICMEMORYALLOCATION
#include <stdlib.h>
#endif

#include "RBRGen3.h"
#include "RBRGen3Internal.h"

const char *RBRGEN3_LIB_NAME =
#ifdef RBR_LIB_NAME
    RBR_LIB_NAME
#else
    "libRBR"
#endif
;

const char *RBRGEN3_LIB_VERSION =
#ifdef RBR_LIB_VERSION
    RBR_LIB_VERSION
#else
    "unknown"
#endif
;

const char *RBRGEN3_LIB_BUILD_DATE =
#ifdef RBR_LIB_BUILD_DATE
    RBR_LIB_BUILD_DATE
#else
    "unknown"
#endif
;

const char *RBRGen3Error_name(RBRGen3Error error)
{
    switch (error)
    {
    case RBRGEN3_SUCCESS:
        return "success";
    case RBRGEN3_ALLOCATION_FAILURE:
        return "allocation failure";
    case RBRGEN3_BUFFER_TOO_SMALL:
        return "buffer too small";
    case RBRGEN3_MISSING_CALLBACK:
        return "missing callback";
    case RBRGEN3_CALLBACK_ERROR:
        return "callback error";
    case RBRGEN3_TIMEOUT:
        return "timeout";
    case RBRGEN3_UNSUPPORTED:
        return "unsupported";
    case RBRGEN3_HARDWARE_ERROR:
        return "hardware error";
    case RBRGEN3_CHECKSUM_ERROR:
        return "checksum error";
    case RBRGEN3_INVALID_PARAMETER_VALUE:
        return "invalid parameter value";
    case RBRGEN3_SAMPLE:
        return "sample";
    case RBRGEN3_ERROR_COUNT:
        return "error count";
    case RBRGEN3_UNKNOWN_ERROR:
    default:
        return "unknown error";
    }
}

const char *RBRGen3Generation_name(RBRGen3Generation generation)
{
    switch (generation)
    {
    case RBRGEN3_LOGGER1:
        return "Logger1";
    case RBRGEN3_LOGGER2:
        return "Logger2";
    case RBRGEN3_LOGGER3:
        return "Logger3";
    case RBRGEN3_LOGGER4:
        return "Logger4";
    case RBRGEN3_GENERATION_COUNT:
        return "generation count";
    case RBRGEN3_UNKNOWN_GENERATION:
    default:
        return "unknown generation";
    }
}

const char *RBRGen3ResponseType_name(RBRGen3ResponseType type)
{
    switch (type)
    {
    case RBRGEN3_RESPONSE_INFO:
        return "info";
    case RBRGEN3_RESPONSE_WARNING:
        return "warning";
    case RBRGEN3_RESPONSE_ERROR:
        return "error";
    case RBRGEN3_RESPONSE_TYPE_COUNT:
        return "response type count";
    case RBRGEN3_RESPONSE_UNKNOWN_TYPE:
    default:
        return "unknown response type";
    }
}

static RBRGen3Error RBRGen3_populateGeneration(
    RBRGen3 *instrument)
{
    instrument->generation = RBRGEN3_UNKNOWN_GENERATION;

    /* If this isn't an RBR instrument, it'll just time out or the response
     * won't match. */
    RBRGen3Error err = RBRInstrumentGen3_getId(instrument, &instrument->id);
    if (err != RBRGEN3_SUCCESS)
    {
        return RBRGEN3_UNSUPPORTED;
    }

    /* The concept of firmware type was introduced part-way through Logger2, so
     * early instruments with very old firmware won't report a firmware type.
     * Newer firmware versions and newer instruments within the generation will
     * report a firmware type of 100–103 (compact and standard loggers) or 200
     * (the RBRcoda T.ODO). */
    if (instrument->id.fwtype == 0
        || (instrument->id.fwtype >= 100
            && instrument->id.fwtype <= 103)
        || instrument->id.fwtype == 200)
    {
        instrument->generation = RBRGEN3_LOGGER2;
    }
    else if ((instrument->id.fwtype >= 104
              && instrument->id.fwtype <= 110)
              || (instrument->id.fwtype >= 202
                  && instrument->id.fwtype <= 205))
    {
        instrument->generation = RBRGEN3_LOGGER3;
    }
    else
    {
        instrument->generation = RBRGEN3_LOGGER4;
    }
    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_open(RBRGen3 **instrument,
                                      const RBRGen3Callbacks *callbacks,
                                      RBRGen3DateTime commandTimeout,
                                      void *userData)
{
    if (callbacks == NULL
        || callbacks->time == NULL
        || callbacks->sleep == NULL
        || callbacks->read == NULL
        || callbacks->write == NULL
        || (callbacks->sample != NULL && callbacks->sampleBuffer == NULL))
    {
        return RBRGEN3_MISSING_CALLBACK;
    }

    bool allocated = false;
    if (*instrument == NULL)
    {
        allocated = true;
        #ifndef RBR_LIB_NODYNAMICMEMORYALLOCATION
        if ((*instrument = malloc(sizeof(RBRGen3))) == NULL)
        {
        #endif
            return RBRGEN3_ALLOCATION_FAILURE;
        #ifndef RBR_LIB_NODYNAMICMEMORYALLOCATION
        }
        #endif
    }

    memset(*instrument, 0, sizeof(RBRGen3));
    memcpy(&(*instrument)->callbacks,
           callbacks,
           sizeof(RBRGen3Callbacks));
    /* We don't want the streaming sample data callback to be called before the
     * constructor has finished. */
    (*instrument)->callbacks.sample  = NULL;
    (*instrument)->commandTimeout    = commandTimeout;
    (*instrument)->userData          = userData;
    (*instrument)->lastActivityTime  = RBRGEN3_NO_ACTIVITY;
    (*instrument)->response.type     = RBRGEN3_RESPONSE_UNKNOWN_TYPE;
    (*instrument)->managedAllocation = allocated;

    RBRGen3Error err;
    err = RBRGen3_populateGeneration(*instrument);
    if (err != RBRGEN3_SUCCESS)
    {
        if (allocated)
        {
            #ifndef RBR_LIB_NODYNAMICMEMORYALLOCATION
            free(*instrument);
            #endif
        }
        return err;
    }

    if ((*instrument)->generation != RBRGEN3_LOGGER2
        && (*instrument)->generation != RBRGEN3_LOGGER3)
    {
        if (allocated)
        {
            #ifndef RBR_LIB_NODYNAMICMEMORYALLOCATION
            free(*instrument);
            #endif
        }
        return RBRGEN3_UNSUPPORTED;
    }

    /* Enable the streaming callback, if applicable. */
    (*instrument)->callbacks.sample = callbacks->sample;
    (*instrument)->callbacks.sampleBuffer = callbacks->sampleBuffer;

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_close(RBRGen3 *instrument)
{
    if (instrument->managedAllocation)
    {
    #ifndef RBR_LIB_NODYNAMICMEMORYALLOCATION
        free(instrument);
    #endif
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Generation RBRGen3_getGeneration(
    const RBRGen3 *instrument)
{
    return instrument->generation;
}

RBRGen3DateTime RBRGen3_getCommandTimeout(
    const RBRGen3 *instrument)
{
    return instrument->commandTimeout;
}

void RBRGen3_setCommandTimeout(RBRGen3 *instrument,
                                     RBRGen3DateTime commandTimeout)
{
    instrument->commandTimeout = commandTimeout;
}

void *RBRGen3_getUserData(const RBRGen3 *instrument)
{
    return instrument->userData;
}

void RBRGen3_setUserData(RBRGen3 *instrument, void *userData)
{
    instrument->userData = userData;
}

RBRInstrumentGen3HardwareError RBRGen3_getLastHardwareError(
    const RBRGen3 *instrument)
{
    if (instrument->response.type == RBRGEN3_RESPONSE_ERROR
        || instrument->response.type == RBRGEN3_RESPONSE_WARNING)
    {
        return instrument->response.error;
    }
    else
    {
        return RBRINSTRUMENTGEN3_HARDWARE_ERROR_NONE;
    }
}

const char *RBRGen3_getLastHardwareErrorMessage(
    const RBRGen3 *instrument)
{
    if (instrument->response.type == RBRGEN3_RESPONSE_ERROR)
    {
        return instrument->response.response;
    }
    else
    {
        return NULL;
    }
}
