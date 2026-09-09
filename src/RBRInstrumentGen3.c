/**
 * \file RBRInstrumentGen3.c
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

#include "RBRInstrumentGen3.h"
#include "RBRInstrumentGen3Internal.h"

const char *RBRINSTRUMENTGEN3_LIB_NAME =
#ifdef RBR_LIB_NAME
    RBR_LIB_NAME
#else
    "libRBR"
#endif
;

const char *RBRINSTRUMENTGEN3_LIB_VERSION =
#ifdef RBR_LIB_VERSION
    RBR_LIB_VERSION
#else
    "unknown"
#endif
;

const char *RBRINSTRUMENTGEN3_LIB_BUILD_DATE =
#ifdef RBR_LIB_BUILD_DATE
    RBR_LIB_BUILD_DATE
#else
    "unknown"
#endif
;

const char *RBRInstrumentGen3Error_name(RBRInstrumentGen3Error error)
{
    switch (error)
    {
    case RBRINSTRUMENTGEN3_SUCCESS:
        return "success";
    case RBRINSTRUMENTGEN3_ALLOCATION_FAILURE:
        return "allocation failure";
    case RBRINSTRUMENTGEN3_BUFFER_TOO_SMALL:
        return "buffer too small";
    case RBRINSTRUMENTGEN3_MISSING_CALLBACK:
        return "missing callback";
    case RBRINSTRUMENTGEN3_CALLBACK_ERROR:
        return "callback error";
    case RBRINSTRUMENTGEN3_TIMEOUT:
        return "timeout";
    case RBRINSTRUMENTGEN3_UNSUPPORTED:
        return "unsupported";
    case RBRINSTRUMENTGEN3_HARDWARE_ERROR:
        return "hardware error";
    case RBRINSTRUMENTGEN3_CHECKSUM_ERROR:
        return "checksum error";
    case RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE:
        return "invalid parameter value";
    case RBRINSTRUMENTGEN3_SAMPLE:
        return "sample";
    case RBRINSTRUMENTGEN3_ERROR_COUNT:
        return "error count";
    case RBRINSTRUMENTGEN3_UNKNOWN_ERROR:
    default:
        return "unknown error";
    }
}

const char *RBRInstrumentGen3Generation_name(RBRInstrumentGen3Generation generation)
{
    switch (generation)
    {
    case RBRINSTRUMENTGEN3_LOGGER1:
        return "Logger1";
    case RBRINSTRUMENTGEN3_LOGGER2:
        return "Logger2";
    case RBRINSTRUMENTGEN3_LOGGER3:
        return "Logger3";
    case RBRINSTRUMENTGEN3_LOGGER4:
        return "Logger4";
    case RBRINSTRUMENTGEN3_GENERATION_COUNT:
        return "generation count";
    case RBRINSTRUMENTGEN3_UNKNOWN_GENERATION:
    default:
        return "unknown generation";
    }
}

const char *RBRInstrumentGen3ResponseType_name(RBRInstrumentGen3ResponseType type)
{
    switch (type)
    {
    case RBRINSTRUMENTGEN3_RESPONSE_INFO:
        return "info";
    case RBRINSTRUMENTGEN3_RESPONSE_WARNING:
        return "warning";
    case RBRINSTRUMENTGEN3_RESPONSE_ERROR:
        return "error";
    case RBRINSTRUMENTGEN3_RESPONSE_TYPE_COUNT:
        return "response type count";
    case RBRINSTRUMENTGEN3_RESPONSE_UNKNOWN_TYPE:
    default:
        return "unknown response type";
    }
}

static RBRInstrumentGen3Error RBRInstrumentGen3_populateGeneration(
    RBRInstrumentGen3 *instrument)
{
    instrument->generation = RBRINSTRUMENTGEN3_UNKNOWN_GENERATION;

    /* If this isn't an RBR instrument, it'll just time out or the response
     * won't match. */
    RBRInstrumentGen3Error err = RBRInstrumentGen3_getId(instrument, &instrument->id);
    if (err != RBRINSTRUMENTGEN3_SUCCESS)
    {
        return RBRINSTRUMENTGEN3_UNSUPPORTED;
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
        instrument->generation = RBRINSTRUMENTGEN3_LOGGER2;
    }
    else if ((instrument->id.fwtype >= 104
              && instrument->id.fwtype <= 110)
              || (instrument->id.fwtype >= 202
                  && instrument->id.fwtype <= 205))
    {
        instrument->generation = RBRINSTRUMENTGEN3_LOGGER3;
    }
    else
    {
        instrument->generation = RBRINSTRUMENTGEN3_LOGGER4;
    }
    return RBRINSTRUMENTGEN3_SUCCESS;
}

RBRInstrumentGen3Error RBRInstrumentGen3_open(RBRInstrumentGen3 **instrument,
                                      const RBRInstrumentGen3Callbacks *callbacks,
                                      RBRInstrumentGen3DateTime commandTimeout,
                                      void *userData)
{
    if (callbacks == NULL
        || callbacks->time == NULL
        || callbacks->sleep == NULL
        || callbacks->read == NULL
        || callbacks->write == NULL
        || (callbacks->sample != NULL && callbacks->sampleBuffer == NULL))
    {
        return RBRINSTRUMENTGEN3_MISSING_CALLBACK;
    }

    bool allocated = false;
    if (*instrument == NULL)
    {
        allocated = true;
        #ifndef RBR_LIB_NODYNAMICMEMORYALLOCATION
        if ((*instrument = malloc(sizeof(RBRInstrumentGen3))) == NULL)
        {
        #endif
            return RBRINSTRUMENTGEN3_ALLOCATION_FAILURE;
        #ifndef RBR_LIB_NODYNAMICMEMORYALLOCATION
        }
        #endif
    }

    memset(*instrument, 0, sizeof(RBRInstrumentGen3));
    memcpy(&(*instrument)->callbacks,
           callbacks,
           sizeof(RBRInstrumentGen3Callbacks));
    /* We don't want the streaming sample data callback to be called before the
     * constructor has finished. */
    (*instrument)->callbacks.sample  = NULL;
    (*instrument)->commandTimeout    = commandTimeout;
    (*instrument)->userData          = userData;
    (*instrument)->lastActivityTime  = RBRINSTRUMENTGEN3_NO_ACTIVITY;
    (*instrument)->response.type     = RBRINSTRUMENTGEN3_RESPONSE_UNKNOWN_TYPE;
    (*instrument)->managedAllocation = allocated;

    RBRInstrumentGen3Error err;
    err = RBRInstrumentGen3_populateGeneration(*instrument);
    if (err != RBRINSTRUMENTGEN3_SUCCESS)
    {
        if (allocated)
        {
            #ifndef RBR_LIB_NODYNAMICMEMORYALLOCATION
            free(*instrument);
            #endif
        }
        return err;
    }

    if ((*instrument)->generation != RBRINSTRUMENTGEN3_LOGGER2
        && (*instrument)->generation != RBRINSTRUMENTGEN3_LOGGER3)
    {
        if (allocated)
        {
            #ifndef RBR_LIB_NODYNAMICMEMORYALLOCATION
            free(*instrument);
            #endif
        }
        return RBRINSTRUMENTGEN3_UNSUPPORTED;
    }

    /* Enable the streaming callback, if applicable. */
    (*instrument)->callbacks.sample = callbacks->sample;
    (*instrument)->callbacks.sampleBuffer = callbacks->sampleBuffer;

    return RBRINSTRUMENTGEN3_SUCCESS;
}

RBRInstrumentGen3Error RBRInstrumentGen3_close(RBRInstrumentGen3 *instrument)
{
    if (instrument->managedAllocation)
    {
    #ifndef RBR_LIB_NODYNAMICMEMORYALLOCATION
        free(instrument);
    #endif
    }

    return RBRINSTRUMENTGEN3_SUCCESS;
}

RBRInstrumentGen3Generation RBRInstrumentGen3_getGeneration(
    const RBRInstrumentGen3 *instrument)
{
    return instrument->generation;
}

RBRInstrumentGen3DateTime RBRInstrumentGen3_getCommandTimeout(
    const RBRInstrumentGen3 *instrument)
{
    return instrument->commandTimeout;
}

void RBRInstrumentGen3_setCommandTimeout(RBRInstrumentGen3 *instrument,
                                     RBRInstrumentGen3DateTime commandTimeout)
{
    instrument->commandTimeout = commandTimeout;
}

void *RBRInstrumentGen3_getUserData(const RBRInstrumentGen3 *instrument)
{
    return instrument->userData;
}

void RBRInstrumentGen3_setUserData(RBRInstrumentGen3 *instrument, void *userData)
{
    instrument->userData = userData;
}

RBRInstrumentGen3HardwareError RBRInstrumentGen3_getLastHardwareError(
    const RBRInstrumentGen3 *instrument)
{
    if (instrument->response.type == RBRINSTRUMENTGEN3_RESPONSE_ERROR
        || instrument->response.type == RBRINSTRUMENTGEN3_RESPONSE_WARNING)
    {
        return instrument->response.error;
    }
    else
    {
        return RBRINSTRUMENTGEN3_HARDWARE_ERROR_NONE;
    }
}

const char *RBRInstrumentGen3_getLastHardwareErrorMessage(
    const RBRInstrumentGen3 *instrument)
{
    if (instrument->response.type == RBRINSTRUMENTGEN3_RESPONSE_ERROR)
    {
        return instrument->response.response;
    }
    else
    {
        return NULL;
    }
}
