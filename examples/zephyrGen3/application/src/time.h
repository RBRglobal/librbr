/**
 * \file time.h
 *
 * \brief Time callback declarations.
 *
 * \copyright
 * Copyright (c) 2025 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_ZEPHYR_TIME_H
#define LIBRBR_ZEPHYR_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <RBRGen3.h>

RBRGen3Error ZephyrRBRGen3Time_get(const struct RBRGen3 *conn,
                                               RBRGen3DateTime *time);

RBRGen3Error ZephyrRBRGen3Time_sleep(const struct RBRGen3 *conn,
                                                 RBRGen3DateTime time);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_ZEPHYR_TIME_H */
