/**
 * \file RBRGen4.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for memcpy, memcmp, memset, strlen. */
#include <string.h>
#include "RBRGen4.h"
#include "RBRGen4Instrument.h"
#include "RBRGen4Internal.h"

const char *RBRGEN4_LIB_NAME =
#ifdef RBR_LIB_NAME
    RBR_LIB_NAME
#else
    "libRBR"
#endif
;

const char *RBRGEN4_LIB_VERSION =
#ifdef RBR_LIB_VERSION
    RBR_LIB_VERSION
#else
    "unknown"
#endif
;

const char *RBRGEN4_LIB_BUILD_DATE =
#ifdef RBR_LIB_BUILD_DATE
    RBR_LIB_BUILD_DATE
#else
    "unknown"
#endif
;

const char *RBRGen4Error_name(RBRGen4Error error)
{
    switch (error)
    {
    case RBRGEN4_SUCCESS:
        return "success";
    case RBRGEN4_UNDERSIZED_STRUCTURE_ERROR:
        return "undersized structure error";
    case RBRGEN4_BUFFER_TOO_SMALL:
        return "buffer too small";
    case RBRGEN4_MISSING_CALLBACK:
        return "missing callback";
    case RBRGEN4_CALLBACK_ERROR:
        return "callback error";
    case RBRGEN4_TIMEOUT:
        return "timeout";
    case RBRGEN4_UNSUPPORTED:
        return "unsupported";
    case RBRGEN4_HARDWARE_ERROR:
        return "hardware error";
    case RBRGEN4_CHECKSUM_ERROR:
        return "checksum error";
    case RBRGEN4_INVALID_PARAMETER_VALUE:
        return "invalid parameter value";
    case RBRGEN4_TRUNCATED:
        return "truncated";
    case RBRGEN4_SAMPLE:
        return "sample";
    case RBRGEN4_ERROR_COUNT:
        return "error count";
    case RBRGEN4_UNKNOWN_ERROR:
    default:
        return "unknown error";
    }
}

const char *RBRGen4DataType_name(RBRGen4DataType dataType)
{
    switch(dataType){
        case RBRGEN4_DATA_TYPE_FLOAT32:
            return "float32";
        case RBRGEN4_DATA_TYPE_FLOAT64:
            return "float64";
        case RBRGEN4_DATA_TYPE_CALFLOAT64:
            return "calfloat64";
        case RBRGEN4_UNKNOWN_DATA_TYPE:
        default:
            return "unknown datatype";
    }
}

const char *RBRGen4Generation_name(RBRGen4Generation generation)
{
    switch (generation)
    {
    case RBRGEN4_LOGGER1:
        return "Logger1";
    case RBRGEN4_LOGGER2:
        return "Logger2";
    case RBRGEN4_LOGGER3:
        return "Logger3";
    case RBRGEN4_LOGGER4:
        return "Logger4";
    case RBRGEN4_GENERATION_COUNT:
        return "generation count";
    case RBRGEN4_UNKNOWN_GENERATION:
    default:
        return "unknown generation";
    }
}

const char *RBRGen4Encoding_name(RBRGen4Encoding encoding)
{
    switch (encoding)
    {
    case RBRGEN4_ENCODING_ASCII:
        return "ascii";
    case RBRGEN4_ENCODING_BINARY:
        return "binary";
    case RBRGEN4_ENCODING_COUNT:
        return "encoding count";
    case RBRGEN4_UNKNOWN_ENCODING:
    default:
        return "unknown encoding";
    }
}

const char *RBRGen4ResponseType_name(RBRGen4ResponseType type)
{
    switch (type)
    {
    case RBRGEN4_RESPONSE_INFO:
        return "info";
    case RBRGEN4_RESPONSE_WARNING:
        return "warning";
    case RBRGEN4_RESPONSE_ERROR:
        return "error";
    case RBRGEN4_RESPONSE_TYPE_COUNT:
        return "response type count";
    case RBRGEN4_RESPONSE_UNKNOWN_TYPE:
    default:
        return "unknown response type";
    }
}

static RBRGen4Error RBRGen4_populateGeneration(
    RBRGen4 *conn)
{
    conn->generation = RBRGEN4_UNKNOWN_GENERATION;

    /* If this isn't an RBR instrument, it'll just time out or the response
     * won't match. */
    RBRGen4Error err = RBRGen4_getId4(conn,
                                                          &conn->id);

    if (err != RBRGEN4_SUCCESS)
    {
        return RBRGEN4_UNSUPPORTED;
    }

    /* The concept of firmware type was introduced part-way through Logger2, so
     * early instruments with very old firmware won't report a firmware type.
     * Newer firmware versions and newer instruments within the generation will
     * report a firmware type of 100–103 (compact and standard loggers) or 200
     * (the RBRcoda T.ODO). This classification mirrors the Gen3 library's
     * (see RBRGen3.c) so the two APIs always agree on an instrument's
     * generation. */
    if (conn->id.fwtype == 0
        || (conn->id.fwtype >= 100
            && conn->id.fwtype <= 103)
        || conn->id.fwtype == 200)
    {
        conn->generation = RBRGEN4_LOGGER2;
    }
    else if ((conn->id.fwtype >= 104
              && conn->id.fwtype <= 110)
             || (conn->id.fwtype >= 202
                 && conn->id.fwtype <= 205))
    {
        conn->generation = RBRGEN4_LOGGER3;
    }
    else
    {
        conn->generation = RBRGEN4_LOGGER4;
    }
    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_open(RBRGen4 **conn,
                                      const RBRGen4Callbacks *callbacks,
                                      const RBRGen4DateTime commandTimeout,
                                      void *userData)
{
    if (callbacks == NULL
        || callbacks->time == NULL
        || callbacks->sleep == NULL
        || callbacks->read == NULL
        || callbacks->write == NULL
        || (callbacks->sample != NULL && callbacks->sampleBuffer == NULL))
    {
        return RBRGEN4_MISSING_CALLBACK;
    }

    memset(*conn, 0, sizeof(RBRGen4));
    memcpy(&(*conn)->callbacks,
           callbacks,
           sizeof(RBRGen4Callbacks));
    /* We don't want the streaming sample data callback to be called before the
     * constructor has finished. */
    (*conn)->callbacks.sample  = NULL;
    (*conn)->commandTimeout    = commandTimeout;
    (*conn)->pollTimeout       = 2 * commandTimeout;
    (*conn)->userData          = userData;
    (*conn)->lastActivityTime  = RBRGEN4_NO_ACTIVITY;
    (*conn)->response.type     = RBRGEN4_RESPONSE_UNKNOWN_TYPE;
    (*conn)->outputFormat      = RBRGEN4_DEFAULT_OUTPUT_FORMAT;

    /* We assume a default output format until it's read below, so samples
     * streamed in any other format in the meantime are dropped as
     * unrecognised responses. See the RBRGen4_open() doc comment. */
    RBRGen4Error err;
    err = RBRGen4_populateGeneration(*conn);

    if (err != RBRGEN4_SUCCESS)
    {
        return err;
    }

    if ((*conn)->generation != RBRGEN4_LOGGER4)
    {
        return RBRGEN4_UNSUPPORTED;
    }

    /* Caches the sample field flags into the instrument for the parser. */
    err = RBRGen4_getOutputFormat(*conn,
                                            &(*conn)->outputFormat);

    if (err != RBRGEN4_SUCCESS)
    {
        return err;
    }

    /* Enable the streaming callback, if applicable. */
    (*conn)->callbacks.sample = callbacks->sample;
    (*conn)->callbacks.sampleBuffer = callbacks->sampleBuffer;

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_close(RBRGen4 *conn)
{
    memset(conn, 0, sizeof(RBRGen4));
    return RBRGEN4_SUCCESS;
}
RBRGen4Generation RBRGen4_getGeneration(
    const RBRGen4 *conn)
{
    return conn->generation;
}

RBRGen4DateTime RBRGen4_getCommandTimeout(
    const RBRGen4 *conn)
{
    return conn->commandTimeout;
}

void RBRGen4_setCommandTimeout(RBRGen4 *conn,
                                     const RBRGen4DateTime commandTimeout)
{
    conn->commandTimeout = commandTimeout;
}

RBRGen4DateTime RBRGen4_getPollTimeout(
    const RBRGen4 *conn)
{
    return conn->pollTimeout;
}

void RBRGen4_setPollTimeout(RBRGen4 *conn,
                                     const RBRGen4DateTime pollTimeout)
{
    conn->pollTimeout = pollTimeout;
}

void *RBRGen4_getUserData(const RBRGen4 *conn)
{
    return conn->userData;
}

void RBRGen4_setUserData(RBRGen4 *conn, void *userData)
{
    conn->userData = userData;
}

RBRGen4HardwareError RBRGen4_getLastHardwareError(
    const RBRGen4 *conn)
{
    if (conn->response.type == RBRGEN4_RESPONSE_ERROR
        || conn->response.type == RBRGEN4_RESPONSE_WARNING)
    {
        return conn->response.error;
    }
    else
    {
        return RBRGEN4_HARDWARE_ERROR_NONE;
    }
}

const char *RBRGen4_getLastHardwareErrorMessage(
    const RBRGen4 *conn)
{
    if (conn->response.type == RBRGEN4_RESPONSE_ERROR)
    {
        return conn->response.response;
    }
    else
    {
        return NULL;
    }
}
