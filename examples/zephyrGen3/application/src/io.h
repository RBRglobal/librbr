/*
 * Copyright (c) 2025 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file io.h
 *
 * \brief I/O callback declarations.
 */

#ifndef LIBRBR_ZEPHYR_IO_H
#define LIBRBR_ZEPHYR_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include <RBRGen3.h>

/** State for bindings from RBRGen3 callbacks to a Zephyr async UART. */
typedef struct ZephyrRBRGen3IO {
    /** The UART device. */
    const struct device *dev;

    /** Read state. */
    struct {
        /** Used to prevent mutual reads across threads. */
        struct k_mutex mut;
        /** Given when any data is available in the receive buffers. */
        struct k_sem sem;

        /** Circular receive buffer populated by DMA. */
        uint8_t buf[CONFIG_INSTRUMENT_RX_BUFFER_SIZE];
        /** The circular buffer index to read data from. */
        size_t tail;
        /** The circular buffer index to write data to. */
        size_t head;
    } rx;

    /** Write state. */
    struct {
        /** Used to prevent mutual reads across threads. */
        struct k_mutex mut;
        /** Given upon write completion. */
        struct k_sem sem;
    } tx;
} ZephyrRBRGen3IO;

RBRGen3Error ZephyrRBRGen3IO_init(ZephyrRBRGen3IO *io, const struct device *dev);

RBRGen3Error ZephyrRBRGen3IO_read(const RBRGen3 *conn, void *data, int32_t *size);

RBRGen3Error ZephyrRBRGen3IO_write(const RBRGen3 *conn, const void *const data, int32_t size);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_ZEPHYR_IO_H */
