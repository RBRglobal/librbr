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
    switch (generation) {
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
    conn->generation = RBRGEN3_UNKNOWN_GENERATION;

    /* If this isn't an RBR instrument, it'll just time out or the response
     * won't match. */
    RBRGen3Error err = RBRGen3_getId(conn, &conn->id);
    if (err != RBRGEN3_SUCCESS) {
        return RBRGEN3_UNSUPPORTED;
    }

    /* The concept of firmware type was introduced part-way through Logger2, so
     * early instruments with very old firmware won't report a firmware type.
     * Newer firmware versions and newer instruments within the generation will
     * report a firmware type of 100–103 (compact and standard loggers) or 200
     * (the RBRcoda T.ODO). */
    if (conn->id.fwtype == 0 || (conn->id.fwtype >= 100 && conn->id.fwtype <= 103) ||
        conn->id.fwtype == 200) {
        conn->generation = RBRGEN3_LOGGER2;
    } else if ((conn->id.fwtype >= 104 && conn->id.fwtype <= 110) ||
               (conn->id.fwtype >= 202 && conn->id.fwtype <= 205)) {
        conn->generation = RBRGEN3_LOGGER3;
    } else {
        conn->generation = RBRGEN3_LOGGER4;
    }
    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_open(RBRGen3 *conn, const RBRGen3Callbacks *callbacks,
                          RBRGen3DateTime commandTimeout, void *userData)
{
    if (callbacks == NULL || callbacks->time == NULL || callbacks->sleep == NULL ||
        callbacks->read == NULL || callbacks->write == NULL ||
        (callbacks->sample != NULL && callbacks->sampleBuffer == NULL)) {
        return RBRGEN3_MISSING_CALLBACK;
    }

    memset(conn, 0, sizeof(RBRGen3));
    memcpy(&conn->callbacks, callbacks, sizeof(RBRGen3Callbacks));
    /* We don't want the streaming sample data callback to be called before the
     * constructor has finished. */
    conn->callbacks.sample = NULL;
    conn->commandTimeout = commandTimeout;
    conn->userData = userData;
    conn->lastActivityTime = RBRGEN3_NO_ACTIVITY;
    conn->response.type = RBRGEN3_RESPONSE_UNKNOWN_TYPE;

    RBRGen3Error err;
    err = RBRGen3_populateGeneration(conn);
    if (err != RBRGEN3_SUCCESS) {
        return err;
    }

    if (conn->generation != RBRGEN3_LOGGER2 && conn->generation != RBRGEN3_LOGGER3) {
        return RBRGEN3_UNSUPPORTED;
    }

    /* Enable the streaming callback, if applicable. */
    conn->callbacks.sample = callbacks->sample;
    conn->callbacks.sampleBuffer = callbacks->sampleBuffer;

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_close(RBRGen3 *conn)
{
    memset(conn, 0, sizeof(RBRGen3));
    return RBRGEN3_SUCCESS;
}

RBRGen3Generation RBRGen3_getGeneration(const RBRGen3 *conn)
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
