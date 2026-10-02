/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file RBRGen4.c
 *
 * \brief Library implementation.
 */

/* Required for memcpy, memset. */
#include <string.h>

#include "RBRGen4.h"
#include "RBRGen4Internal.h"
#include "RBRGen4Instrument.h"

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
    switch (error) {
    case RBRGEN4_SUCCESS:
        return "success";
    case RBRGEN4_BUFFER_TOO_SMALL:
        return "buffer too small";
    case RBRGEN4_COMMAND_TOO_LONG:
        return "command too long";
    case RBRGEN4_RESPONSE_TOO_LONG:
        return "response too long";
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
    switch (dataType) {
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

const char *RBRGen4ResponseType_name(RBRGen4ResponseType type)
{
    switch (type) {
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

static RBRGen4Error RBRGen4_populateGeneration(RBRGen4 *conn)
{
    conn->generation = RBRCOMMON_UNKNOWN_GENERATION;

    /* If this isn't an RBR instrument, the exchange times out or the
     * response won't match: an instrument of another generation answers with
     * an error line it can produce, and anything else answers with nothing
     * this parser recognises. Both mean "not one of ours". A failing callback
     * or a buffer too small for the exchange is the caller's problem, though,
     * and is reported as itself. */
    RBRGen4Error err = RBRGen4_getId4(conn, &conn->id);
    if (err == RBRGEN4_TIMEOUT || err == RBRGEN4_HARDWARE_ERROR) {
        return RBRGEN4_UNSUPPORTED;
    }
    RBR_TRY(err);

    conn->generation = RBRCommonGeneration_fromFwType(conn->id.fwType);
    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_open(RBRGen4 *conn, const RBRGen4Environment *environment,
                          const RBRGen4DateTime commandTimeout, void *userData)
{
    if (environment == NULL || environment->time == NULL || environment->sleep == NULL ||
        environment->read == NULL || environment->write == NULL ||
        (environment->sample != NULL && environment->sampleBuffer == NULL)) {
        return RBRGEN4_MISSING_CALLBACK;
    }
    if (environment->sampleBuffer != NULL &&
        (environment->sampleBuffer->readings == NULL || environment->sampleBuffer->size <= 0)) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    memset(conn, 0, sizeof(RBRGen4));
    conn->generation = RBRCOMMON_UNKNOWN_GENERATION;
    memcpy(&conn->environment, environment, sizeof(RBRGen4Environment));
    RBR_TRY(RBRGen4_setCommandBuffer(conn, environment->command, environment->commandCapacity));
    RBR_TRY(RBRGen4_setResponseBuffer(conn, environment->response, environment->responseCapacity));
    /* We don't want the streaming sample data callback to be called before the
     * constructor has finished. */
    conn->environment.sample = NULL;
    conn->commandTimeout = commandTimeout;
    conn->pollTimeout = 2 * commandTimeout;
    conn->userData = userData;
    conn->lastActivityTime = RBRGEN4_NO_ACTIVITY;
    conn->response.type = RBRGEN4_RESPONSE_UNKNOWN_TYPE;
    conn->outputFormat = RBRGEN4_DEFAULT_OUTPUT_FORMAT;

    /* We assume a default output format until it's read below, so samples
     * streamed in any other format in the meantime are dropped as
     * unrecognised responses. See the RBRGen4_open() doc comment. */
    RBRGen4Error err;
    err = RBRGen4_populateGeneration(conn);

    if (err != RBRGEN4_SUCCESS) {
        return err;
    }

    if (conn->generation != RBRCOMMON_LOGGER4) {
        return RBRGEN4_UNSUPPORTED;
    }

    /* Caches the sample field flags into the instrument for the parser. */
    err = RBRGen4_getOutputFormat(conn, &conn->outputFormat);

    if (err != RBRGEN4_SUCCESS) {
        return err;
    }

    /* Enable the streaming callback, if applicable. */
    conn->environment.sample = environment->sample;
    conn->environment.sampleBuffer = environment->sampleBuffer;

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_close(RBRGen4 *conn)
{
    /* The library holds no resources, so there is nothing to release. This
     * function is kept so that callers pair every open with a close and so
     * that resource management can be added later without an API change. */
    memset(conn, 0, sizeof(RBRGen4));
    return RBRGEN4_SUCCESS;
}

void RBRGen4_resetResponseBuffer(RBRGen4 *conn)
{
    conn->responseBufferLength = 0;
    conn->lastResponseLength = 0;
    conn->response.type = RBRGEN4_RESPONSE_UNKNOWN_TYPE;
    conn->response.error = RBRGEN4_HARDWARE_ERROR_NONE;
    conn->response.response = NULL;
}

RBRGen4Error RBRGen4_setCommandBuffer(RBRGen4 *conn, uint8_t *command, int32_t capacity)
{
    if (command == NULL || capacity <= 0) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    conn->environment.command = command;
    conn->environment.commandCapacity = capacity;
    conn->commandBufferLength = 0;
    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_setResponseBuffer(RBRGen4 *conn, uint8_t *response, int32_t capacity)
{
    if (response == NULL || capacity <= RBRGEN4_RESPONSE_TERMINATOR_LEN) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    conn->environment.response = response;
    conn->environment.responseCapacity = capacity;
    RBRGen4_resetResponseBuffer(conn);
    return RBRGEN4_SUCCESS;
}

RBRCommonGeneration RBRGen4_getGeneration(const RBRGen4 *conn)
{
    return conn->generation;
}

RBRGen4DateTime RBRGen4_getCommandTimeout(const RBRGen4 *conn)
{
    return conn->commandTimeout;
}

void RBRGen4_setCommandTimeout(RBRGen4 *conn, const RBRGen4DateTime commandTimeout)
{
    conn->commandTimeout = commandTimeout;
}

RBRGen4DateTime RBRGen4_getPollTimeout(const RBRGen4 *conn)
{
    return conn->pollTimeout;
}

void RBRGen4_setPollTimeout(RBRGen4 *conn, const RBRGen4DateTime pollTimeout)
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

RBRGen4HardwareError RBRGen4_getLastHardwareError(const RBRGen4 *conn)
{
    if (conn->response.type == RBRGEN4_RESPONSE_ERROR ||
        conn->response.type == RBRGEN4_RESPONSE_WARNING) {
        return conn->response.error;
    } else {
        return RBRGEN4_HARDWARE_ERROR_NONE;
    }
}

const char *RBRGen4_getLastHardwareErrorMessage(const RBRGen4 *conn)
{
    if (conn->response.type == RBRGEN4_RESPONSE_ERROR) {
        return conn->response.response;
    } else {
        return NULL;
    }
}
