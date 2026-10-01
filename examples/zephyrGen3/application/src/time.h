/*
 * Copyright (c) 2025 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file time.h
 *
 * \brief Time callback declarations.
 */

#ifndef LIBRBR_ZEPHYR_TIME_H
#define LIBRBR_ZEPHYR_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <RBRGen3.h>

RBRGen3Error ZephyrRBRGen3Time_get(const RBRGen3 *conn, RBRGen3DateTime *time);

RBRGen3Error ZephyrRBRGen3Time_sleep(const RBRGen3 *conn, RBRGen3DateTime time);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_ZEPHYR_TIME_H */
