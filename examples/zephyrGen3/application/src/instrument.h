/**
 * \file instrument.h
 *
 * \brief Instrument management helper declarations.
 *
 * \copyright
 * Copyright (c) 2025 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_ZEPHYR_INSTRUMENT_H
#define LIBRBR_ZEPHYR_INSTRUMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <RBRGen3.h>

RBRGen3Error instrumentStart(RBRGen3 *instrument);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_ZEPHYR_IO_H */
