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

#include <RBRInstrumentGen3.h>

RBRInstrumentGen3Error ZephyrRBRInstrumentGen3Time_get(const struct RBRInstrumentGen3 *instrument,
                                               RBRInstrumentGen3DateTime *time);

RBRInstrumentGen3Error ZephyrRBRInstrumentGen3Time_sleep(const struct RBRInstrumentGen3 *instrument,
                                                 RBRInstrumentGen3DateTime time);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_ZEPHYR_TIME_H */
