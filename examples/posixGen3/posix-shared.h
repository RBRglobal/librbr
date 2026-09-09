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

#include "RBRInstrumentGen3.h"

#define INSTRUMENT_CHARACTER_TIMEOUT_MSEC 4000
#define INSTRUMENT_COMMAND_TIMEOUT_MSEC 10000

int openSerialFd(char *devicePath);

RBRInstrumentGen3Error instrumentTime(const struct RBRInstrumentGen3 *instrument,
                                  RBRInstrumentGen3DateTime *time);

RBRInstrumentGen3Error instrumentSleep(const struct RBRInstrumentGen3 *instrument,
                                   RBRInstrumentGen3DateTime time);

RBRInstrumentGen3Error instrumentRead(const struct RBRInstrumentGen3 *instrument,
                                  void *data,
                                  int32_t *size);

RBRInstrumentGen3Error instrumentWrite(const struct RBRInstrumentGen3 *instrument,
                                   const void *const data,
                                   int32_t size);

RBRInstrumentGen3Error instrumentStart(RBRInstrumentGen3 *instrument);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_POSIX_SHARED_H */
