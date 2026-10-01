/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file RBRGen3.c
 *
 * \brief Library implementation.
 */

/* Required for memcpy, memcmp, memset, strlen. */
#include <string.h>
#include "RBRGen3.h"
#include "RBRGen3Internal.h"
#include "RBRGen3Other.h"

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
    switch (error) {
    case RBRGEN3_SUCCESS:
        return "success";
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
    case RBRGEN3_TRUNCATED:
        return "truncated";
    case RBRGEN3_SAMPLE:
        return "sample";
    case RBRGEN3_ERROR_COUNT:
        return "error count";
    case RBRGEN3_UNKNOWN_ERROR:
    default:
        return "unknown error";
    }
}

const char *RBRGen3ResponseType_name(RBRGen3ResponseType type)
{
    switch (type) {
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

static RBRGen3Error RBRGen3_populateGeneration(RBRGen3 *conn)
{
    conn->generation = RBRCOMMON_UNKNOWN_GENERATION;

    /* If this isn't an RBR instrument, the exchange times out or the
     * response won't match: an instrument of another generation answers with
     * an error line it can produce, and anything else answers with nothing
     * this parser recognises. Both mean "not one of ours". A failing callback
     * or a buffer too small for the exchange is the caller's problem, though,
     * and is reported as itself. */
    RBRGen3Error err = RBRGen3_getId(conn, &conn->id);
    if (err == RBRGEN3_TIMEOUT || err == RBRGEN3_HARDWARE_ERROR) {
        return RBRGEN3_UNSUPPORTED;
    }
    RBR_TRY(err);

    conn->generation = RBRCommonGeneration_fromFwtype(conn->id.fwtype);
    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_open(RBRGen3 *conn, const RBRGen3Environment *environment,
                          RBRGen3DateTime commandTimeout, void *userData)
{
    if (environment == NULL || environment->time == NULL || environment->sleep == NULL ||
        environment->read == NULL || environment->write == NULL ||
        (environment->sample != NULL && environment->sampleBuffer == NULL)) {
        return RBRGEN3_MISSING_CALLBACK;
    }
    if (environment->sampleBuffer != NULL &&
        (environment->sampleBuffer->readings == NULL || environment->sampleBuffer->size <= 0)) {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    memset(conn, 0, sizeof(RBRGen3));
    conn->generation = RBRCOMMON_UNKNOWN_GENERATION;
    memcpy(&conn->environment, environment, sizeof(RBRGen3Environment));
    RBR_TRY(RBRGen3_setCommandBuffer(conn, environment->command, environment->commandCapacity));
    RBR_TRY(RBRGen3_setResponseBuffer(conn, environment->response, environment->responseCapacity));
    /* We don't want the streaming sample data callback to be called before the
     * constructor has finished. */
    conn->environment.sample = NULL;
    conn->commandTimeout = commandTimeout;
    conn->userData = userData;
    conn->lastActivityTime = RBRGEN3_NO_ACTIVITY;
    conn->response.type = RBRGEN3_RESPONSE_UNKNOWN_TYPE;

    RBRGen3Error err;
    err = RBRGen3_populateGeneration(conn);
    if (err != RBRGEN3_SUCCESS) {
        return err;
    }

    if (conn->generation != RBRCOMMON_LOGGER2 && conn->generation != RBRCOMMON_LOGGER3) {
        return RBRGEN3_UNSUPPORTED;
    }

    /* Enable the streaming callback, if applicable. */
    conn->environment.sample = environment->sample;
    conn->environment.sampleBuffer = environment->sampleBuffer;

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_close(RBRGen3 *conn)
{
    /* The library holds no resources, so there is nothing to release. This
     * function is kept so that callers pair every open with a close and so
     * that resource management can be added later without an API change. */
    memset(conn, 0, sizeof(RBRGen3));
    return RBRGEN3_SUCCESS;
}

void RBRGen3_resetResponseBuffer(RBRGen3 *conn)
{
    conn->responseBufferLength = 0;
    conn->lastResponseLength = 0;
    conn->response.type = RBRGEN3_RESPONSE_UNKNOWN_TYPE;
    conn->response.error = RBRGEN3_HARDWARE_ERROR_NONE;
    conn->response.response = NULL;
}

RBRGen3Error RBRGen3_setCommandBuffer(RBRGen3 *conn, uint8_t *command, int32_t capacity)
{
    if (command == NULL || capacity <= 0) {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    conn->environment.command = command;
    conn->environment.commandCapacity = capacity;
    conn->commandBufferLength = 0;
    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_setResponseBuffer(RBRGen3 *conn, uint8_t *response, int32_t capacity)
{
    if (response == NULL || capacity <= RBRGEN3_COMMAND_TERMINATOR_LEN) {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    conn->environment.response = response;
    conn->environment.responseCapacity = capacity;
    RBRGen3_resetResponseBuffer(conn);
    return RBRGEN3_SUCCESS;
}

RBRCommonGeneration RBRGen3_getGeneration(const RBRGen3 *conn)
{
    return conn->generation;
}

RBRGen3DateTime RBRGen3_getCommandTimeout(const RBRGen3 *conn)
{
    return conn->commandTimeout;
}

void RBRGen3_setCommandTimeout(RBRGen3 *conn, RBRGen3DateTime commandTimeout)
{
    conn->commandTimeout = commandTimeout;
}

void *RBRGen3_getUserData(const RBRGen3 *conn)
{
    return conn->userData;
}

void RBRGen3_setUserData(RBRGen3 *conn, void *userData)
{
    conn->userData = userData;
}

RBRGen3HardwareError RBRGen3_getLastHardwareError(const RBRGen3 *conn)
{
    if (conn->response.type == RBRGEN3_RESPONSE_ERROR ||
        conn->response.type == RBRGEN3_RESPONSE_WARNING) {
        return conn->response.error;
    } else {
        return RBRGEN3_HARDWARE_ERROR_NONE;
    }
}

const char *RBRGen3_getLastHardwareErrorMessage(const RBRGen3 *conn)
{
    if (conn->response.type == RBRGEN3_RESPONSE_ERROR) {
        return conn->response.response;
    } else {
        return NULL;
    }
}
