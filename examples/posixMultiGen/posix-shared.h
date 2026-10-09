/*
 * Copyright (c) 2026 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file posix-shared.h
 *
 * \brief Shared functions used by the libRBR POSIX examples.
 */

#ifndef LIBRBR_POSIXMULTIGEN_POSIX_SHARED_H
#define LIBRBR_POSIXMULTIGEN_POSIX_SHARED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RBRGen3.h"
#include "RBRGen4.h"

#define INSTRUMENT_CHARACTER_TIMEOUT_MSEC 4000
#define INSTRUMENT_COMMAND_TIMEOUT_MSEC   10000

int openSerialFd(const char *devicePath);

RBRGen3Error gen3InstrumentTime(const RBRGen3 *conn, RBRGen3DateTime *time);

RBRGen3Error gen3InstrumentSleep(const RBRGen3 *conn, RBRGen3DateTime time);

RBRGen3Error gen3InstrumentRead(const RBRGen3 *conn, void *data, int32_t *size);

RBRGen3Error gen3InstrumentWrite(const RBRGen3 *conn, const void *const data, int32_t size);

RBRGen4Error gen4InstrumentTime(const RBRGen4 *conn, RBRGen4DateTime *time);

RBRGen4Error gen4InstrumentSleep(const RBRGen4 *conn, RBRGen4DateTime time);

RBRGen4Error gen4InstrumentRead(const RBRGen4 *conn, void *data, int32_t *size);

RBRGen4Error gen4InstrumentWrite(const RBRGen4 *conn, const void *const data, int32_t size);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_POSIXMULTIGEN_POSIX_SHARED_H */
