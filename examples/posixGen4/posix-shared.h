/**
 * \file posix-shared.h
 *
 * \brief Shared functions used by the libRBR POSIX examples.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_POSIX_SHARED_H
#define LIBRBR_POSIX_SHARED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../include/RBRGen4.h"

#define INSTRUMENT_CHARACTER_TIMEOUT_MSEC 4000
#define INSTRUMENT_COMMAND_TIMEOUT_MSEC   10000

int openSerialFd(char *devicePath);

/**
 * \brief Callback to get the current time.
 * \see RBRGen4Callbacks
 * \see RBRGen4TimeCallback
 */
RBRGen4Error instrumentTime(const struct RBRGen4 *conn, RBRGen4DateTime *time);

/**
 * \brief Callback to run when the instrument goes to sleep.
 * \see RBRGen4Callbacks
 * \see RBRGen4TimeCallback
 */
RBRGen4Error instrumentSleep(const struct RBRGen4 *conn, RBRGen4DateTime time);

/**
 * \brief Callback to read from the instrument.
 * \see RBRGen4Callbacks
 * \see RBRGen4ReadCallback
 */
RBRGen4Error instrumentRead(const struct RBRGen4 *conn, void *data, int32_t *size);

/**
 * \brief Callback to write to the instrument.
 * \see RBRGen4Callbacks
 * \see RBRGen4WriteCallback
 */
RBRGen4Error instrumentWrite(const struct RBRGen4 *conn, const void *const data, int32_t size);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_POSIX_SHARED_H */
