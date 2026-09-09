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

RBRGen3Error ZephyrRBRInstrumentGen3Time_get(const struct RBRGen3 *instrument,
                                               RBRGen3DateTime *time)
{
    (void) instrument;

    *time = k_uptime_get();
    LOG_DBG("now %" PRIi64 " ms", *time);

    return RBRGEN3_SUCCESS;
}

RBRGen3Error ZephyrRBRInstrumentGen3Time_sleep(const struct RBRGen3 *instrument,
                                                 RBRGen3DateTime time)
{
    (void) instrument;

    LOG_DBG("sleeping %" PRIi64 " ms...", time);
    k_sleep(K_MSEC(time));
    LOG_DBG("waking up again");

    return RBRGEN3_SUCCESS;
}
