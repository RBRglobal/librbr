/*
 * Copyright (c) 2025 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file time.c
 *
 * \brief Time callback implementations using Zephyr kernel timing.
 */

#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "time.h"

LOG_MODULE_REGISTER(time, CONFIG_TIME_LOG_LEVEL);

RBRGen3Error ZephyrRBRGen3Time_get(const RBRGen3 *conn, RBRGen3DateTime *time)
{
    (void) conn;

    *time = k_uptime_get();
    LOG_DBG("now %" PRIi64 " ms", *time);

    return RBRGEN3_SUCCESS;
}

RBRGen3Error ZephyrRBRGen3Time_sleep(const RBRGen3 *conn, RBRGen3DateTime time)
{
    (void) conn;

    LOG_DBG("sleeping %" PRIi64 " ms...", time);
    k_sleep(K_MSEC(time));
    LOG_DBG("waking up again");

    return RBRGEN3_SUCCESS;
}
