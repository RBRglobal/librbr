/**
 * \file RBRInstrumentGen4.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for memcpy, memcmp, memset, strlen. */
#include <string.h>
#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Instrument.h"
#include "RBRInstrumentGen4Internal.h"
#include "RBRInstrumentGen4Streaming.h"
//to be deleted:
#include <stdio.h>

const char *RBRINSTRUMENTGEN4_LIB_NAME =
#ifdef RBR_LIB_NAME
    RBR_LIB_NAME
#else
    "libRBR"
#endif
;

const char *RBRINSTRUMENTGEN4_LIB_VERSION =
#ifdef RBR_LIB_VERSION
    RBR_LIB_VERSION
#else
    "unknown"
#endif
;

const char *RBRINSTRUMENTGEN4_LIB_BUILD_DATE =
#ifdef RBR_LIB_BUILD_DATE
    RBR_LIB_BUILD_DATE
#else
    "unknown"
#endif
;

const char *RBRInstrumentGen4Error_name(RBRInstrumentGen4Error error)
{
    switch (error)
    {
    case RBRINSTRUMENTGEN4_SUCCESS:
        return "success";
    case RBRINSTRUMENTGEN4_UNDERSIZED_STRUCTURE_ERROR:
        return "undersized structure error";
    case RBRINSTRUMENTGEN4_BUFFER_TOO_SMALL:
        return "buffer too small";
    case RBRINSTRUMENTGEN4_MISSING_CALLBACK:
        return "missing callback";
    case RBRINSTRUMENTGEN4_CALLBACK_ERROR:
        return "callback error";
    case RBRINSTRUMENTGEN4_TIMEOUT:
        return "timeout";
    case RBRINSTRUMENTGEN4_UNSUPPORTED:
        return "unsupported";
    case RBRINSTRUMENTGEN4_HARDWARE_ERROR:
        return "hardware error";
    case RBRINSTRUMENTGEN4_CHECKSUM_ERROR:
        return "checksum error";
    case RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE:
        return "invalid parameter value";
    case RBRINSTRUMENTGEN4_SAMPLE:
        return "sample";
    case RBRINSTRUMENTGEN4_ERROR_COUNT:
        return "error count";
    case RBRINSTRUMENTGEN4_UNKNOWN_ERROR:
    default:
        return "unknown error";
    }
}

const char *RBRInstrumentGen4DataType_name(RBRInstrumentGen4DataType dataType)
{
    switch(dataType){
        case RBRINSTRUMENTGEN4_DATATYPE_FLOAT32:
            return "float32";
        case RBRINSTRUMENTGEN4_DATATYPE_FLOAT64:
            return "float64";
        case RBRINSTRUMENTGEN4_DATATYPE_CALFLOAT64:
            return "calfloat64";
        case RBRINSTRUMENTGEN4_UNKNOWN_DATATYPE:
        default:
            return "unknown datatype";
    }
}

const char *RBRInstrumentGen4Generation_name(RBRInstrumentGen4Generation generation)
{
    switch (generation)
    {
    case RBRINSTRUMENTGEN4_LOGGER1:
        return "Logger1";
    case RBRINSTRUMENTGEN4_LOGGER2:
        return "Logger2";
    case RBRINSTRUMENTGEN4_LOGGER3:
        return "Logger3";
    case RBRINSTRUMENTGEN4_LOGGER4:
        return "Logger4";
    case RBRINSTRUMENTGEN4_GENERATION_COUNT:
        return "generation count";
    case RBRINSTRUMENTGEN4_UNKNOWN_GENERATION:
    default:
        return "unknown generation";
    }
}

void RBRInstrumentGen4OutputFormat_name(RBRInstrumentGen4Outputformat outputFormat, char *name)
{
    sprintf(
        name,
        "sn=%s schedulelabel=%s datetime=%s crc=%s", 
        (outputFormat & RBRINSTRUMENTGEN4_OUTPUTFORMAT_SERIAL) ? "on" : "off",
        (outputFormat & RBRINSTRUMENTGEN4_OUTPUTFORMAT_SCHEDULELABEL) ? "on" : "off",
        (outputFormat & RBRINSTRUMENTGEN4_OUTPUTFORMAT_TIMESTAMP) ? "on" : "off",
        (outputFormat & RBRINSTRUMENTGEN4_OUTPUTFORMAT_CRC) ? "on" : "off");
}

const char *RBRInstrumentGen4ResponseType_name(RBRInstrumentGen4ResponseType type)
{
    switch (type)
    {
    case RBRINSTRUMENTGEN4_RESPONSE_INFO:
        return "info";
    case RBRINSTRUMENTGEN4_RESPONSE_WARNING:
        return "warning";
    case RBRINSTRUMENTGEN4_RESPONSE_ERROR:
        return "error";
    case RBRINSTRUMENTGEN4_RESPONSE_TYPE_COUNT:
        return "response type count";
    case RBRINSTRUMENTGEN4_RESPONSE_UNKNOWN_TYPE:
    default:
        return "unknown response type";
    }
}

static RBRInstrumentGen4Error RBRInstrumentGen4_populateGeneration(
    RBRInstrumentGen4 *instrument)
{
    instrument->generation = RBRINSTRUMENTGEN4_UNKNOWN_GENERATION;

    /* If this isn't an RBR instrument, it'll just time out or the response
     * won't match. */
    RBRInstrumentGen4Error err = RBRInstrumentGen4_getId(instrument, &instrument->id);

    if (err != RBRINSTRUMENTGEN4_SUCCESS)
    {
        return RBRINSTRUMENTGEN4_UNSUPPORTED;
    }

    /* The concept of firmware type was introduced part-way through Logger2, so
     * early instruments with very old firmware won't report a firmware type.
     * Newer firmware versions and newer instruments within the generation will
     * report a firmware type of 100–103 (compact and standard loggers) or 200
     * (the RBRcoda T.ODO). This classification mirrors the Gen3 library's
     * (see RBRInstrument.c) so the two APIs always agree on an instrument's
     * generation. */
    if (instrument->id.fwtype == 0
        || (instrument->id.fwtype >= 100
            && instrument->id.fwtype <= 103)
        || instrument->id.fwtype == 200)
    {
        instrument->generation = RBRINSTRUMENTGEN4_LOGGER2;
    }
    else if ((instrument->id.fwtype >= 104
              && instrument->id.fwtype <= 110)
             || (instrument->id.fwtype >= 202
                 && instrument->id.fwtype <= 205))
    {
        instrument->generation = RBRINSTRUMENTGEN4_LOGGER3;
    }
    else
    {
        instrument->generation = RBRINSTRUMENTGEN4_LOGGER4;
    }
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_open(RBRInstrumentGen4 **instrument,
                                      const RBRInstrumentGen4Callbacks *callbacks,
                                      const RBRInstrumentGen4DateTime commandTimeout,
                                      void *userData)
{
    if (callbacks == NULL
        || callbacks->time == NULL
        || callbacks->sleep == NULL
        || callbacks->read == NULL
        || callbacks->write == NULL
        || (callbacks->sample != NULL && callbacks->sampleBuffer == NULL))
    {
        return RBRINSTRUMENTGEN4_MISSING_CALLBACK;
    }

    memset(*instrument, 0, sizeof(RBRInstrumentGen4));
    memcpy(&(*instrument)->callbacks,
           callbacks,
           sizeof(RBRInstrumentGen4Callbacks));
    /* We don't want the streaming sample data callback to be called before the
     * constructor has finished. */
    (*instrument)->callbacks.sample  = NULL;
    (*instrument)->commandTimeout    = commandTimeout;
    (*instrument)->userData          = userData;
    (*instrument)->lastActivityTime  = RBRINSTRUMENTGEN4_NO_ACTIVITY;
    (*instrument)->response.type     = RBRINSTRUMENTGEN4_RESPONSE_UNKNOWN_TYPE;
    (*instrument)->outputFormat      = RBRINSTRUMENTGEN4_DEFAULT_OUTPUTFORMAT;

    /**
     * Note that because we assume a default output format,
     * RBRInstrumentGen4Sample_parse in RBRInstrumentGen4_readResponse in
     * RBRInstrumentGen4_converse in RBRInstrumentGen4_populateGeneration and
     * RBRInstrumentGen4_getOutputformat will return an error if 
     * RBRInstrumentGen4_open is called on an instrument that
     * is streaming samples that are *not* in the default output format.
     */
    RBRInstrumentGen4Error err;
    err = RBRInstrumentGen4_populateGeneration(*instrument);

    if (err != RBRINSTRUMENTGEN4_SUCCESS)
    {
        return err;
    }

    if ((*instrument)->generation != RBRINSTRUMENTGEN4_LOGGER4)
    {
        return RBRINSTRUMENTGEN4_UNSUPPORTED;
    }

    err = RBRInstrumentGen4_getOutputformat(*instrument, &(*instrument)->outputFormat);

    if (err != RBRINSTRUMENTGEN4_SUCCESS)
    {
        return err;
    }

    /* Enable the streaming callback, if applicable. */
    (*instrument)->callbacks.sample = callbacks->sample;
    (*instrument)->callbacks.sampleBuffer = callbacks->sampleBuffer;

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_close(RBRInstrumentGen4 *instrument)
{
    memset(instrument, 0, sizeof(RBRInstrumentGen4));
    return RBRINSTRUMENTGEN4_SUCCESS;
}
RBRInstrumentGen4Generation RBRInstrumentGen4_getGeneration(
    const RBRInstrumentGen4 *instrument)
{
    return instrument->generation;
}

RBRInstrumentGen4DateTime RBRInstrumentGen4_getCommandTimeout(
    const RBRInstrumentGen4 *instrument)
{
    return instrument->commandTimeout;
}

void RBRInstrumentGen4_setCommandTimeout(RBRInstrumentGen4 *instrument,
                                     const RBRInstrumentGen4DateTime commandTimeout)
{
    instrument->commandTimeout = commandTimeout;
}

void *RBRInstrumentGen4_getUserData(const RBRInstrumentGen4 *instrument)
{
    return instrument->userData;
}

void RBRInstrumentGen4_setUserData(RBRInstrumentGen4 *instrument, void *userData)
{
    instrument->userData = userData;
}

RBRInstrumentGen4HardwareError RBRInstrumentGen4_getLastHardwareError(
    const RBRInstrumentGen4 *instrument)
{
    if (instrument->response.type == RBRINSTRUMENTGEN4_RESPONSE_ERROR
        || instrument->response.type == RBRINSTRUMENTGEN4_RESPONSE_WARNING)
    {
        return instrument->response.error;
    }
    else
    {
        return RBRINSTRUMENTGEN4_HARDWARE_ERROR_NONE;
    }
}

const char *RBRInstrumentGen4_getLastHardwareErrorMessage(
    const RBRInstrumentGen4 *instrument)
{
    if (instrument->response.type == RBRINSTRUMENTGEN4_RESPONSE_ERROR)
    {
        return instrument->response.response;
    }
    else
    {
        return NULL;
    }
}
