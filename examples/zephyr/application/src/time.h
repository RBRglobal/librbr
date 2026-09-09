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

RBRInstrumentError ZephyrRBRInstrumentTime_get(const struct RBRInstrument *instrument,
                                               RBRInstrumentDateTime *time);

RBRInstrumentError ZephyrRBRInstrumentTime_sleep(const struct RBRInstrument *instrument,
                                                 RBRInstrumentDateTime time);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_ZEPHYR_TIME_H */
