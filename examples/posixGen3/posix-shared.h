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

#include "RBRGen3.h"

#define INSTRUMENT_CHARACTER_TIMEOUT_MSEC 4000
#define INSTRUMENT_COMMAND_TIMEOUT_MSEC 10000

int openSerialFd(char *devicePath);

RBRGen3Error instrumentTime(const struct RBRGen3 *instrument,
                                  RBRGen3DateTime *time);

RBRGen3Error instrumentSleep(const struct RBRGen3 *instrument,
                                   RBRGen3DateTime time);

RBRGen3Error instrumentRead(const struct RBRGen3 *instrument,
                                  void *data,
                                  int32_t *size);

RBRGen3Error instrumentWrite(const struct RBRGen3 *instrument,
                                   const void *const data,
                                   int32_t size);

RBRGen3Error instrumentStart(RBRGen3 *instrument);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_POSIX_SHARED_H */
