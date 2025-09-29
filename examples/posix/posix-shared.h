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

#include "RBRInstrument.h"

#define INSTRUMENT_CHARACTER_TIMEOUT_MSEC 4000
#define INSTRUMENT_COMMAND_TIMEOUT_MSEC   10000

int openSerialFd(char *devicePath);

RBRInstrumentError instrumentTime(const struct RBRInstrument *instrument,
                                  RBRInstrumentDateTime *time);

RBRInstrumentError instrumentSleep(const struct RBRInstrument *instrument,
                                   RBRInstrumentDateTime time);

RBRInstrumentError instrumentRead(const struct RBRInstrument *instrument, void *data,
                                  int32_t *size);

RBRInstrumentError instrumentWrite(const struct RBRInstrument *instrument, const void *const data,
                                   int32_t size);

RBRInstrumentError instrumentStart(RBRInstrument *instrument);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_POSIX_SHARED_H */
