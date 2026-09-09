/**
 * \file time.c
 *
 * \brief Time callback implementations using Zephyr kernel timing.
 *
 * \copyright
 * Copyright (c) 2025 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "time.h"

LOG_MODULE_REGISTER(time, CONFIG_TIME_LOG_LEVEL);

RBRInstrumentGen3Error ZephyrRBRInstrumentGen3Time_get(const struct RBRInstrumentGen3 *instrument,
                                               RBRInstrumentGen3DateTime *time)
{
    (void) instrument;

    *time = k_uptime_get();
    LOG_DBG("now %" PRIi64 " ms", *time);

    return RBRINSTRUMENTGEN3_SUCCESS;
}

RBRInstrumentGen3Error ZephyrRBRInstrumentGen3Time_sleep(const struct RBRInstrumentGen3 *instrument,
                                                 RBRInstrumentGen3DateTime time)
{
    (void) instrument;

    LOG_DBG("sleeping %" PRIi64 " ms...", time);
    k_sleep(K_MSEC(time));
    LOG_DBG("waking up again");

    return RBRINSTRUMENTGEN3_SUCCESS;
}
