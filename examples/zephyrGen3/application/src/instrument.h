/*
 * Copyright (c) 2025 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file instrument.h
 *
 * \brief Instrument management helper declarations.
 */

#ifndef LIBRBR_ZEPHYR_INSTRUMENT_H
#define LIBRBR_ZEPHYR_INSTRUMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <RBRGen3.h>

RBRGen3Error instrumentStart(RBRGen3 *conn);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_ZEPHYR_INSTRUMENT_H */
