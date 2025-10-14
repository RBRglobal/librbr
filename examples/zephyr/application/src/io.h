/**
 * \file io.h
 *
 * \brief I/O callback declarations.
 *
 * \copyright
 * Copyright (c) 2025 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_ZEPHYR_IO_H
#define LIBRBR_ZEPHYR_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include <RBRInstrument.h>

/** State for bindings from RBRInstrument callbacks to a Zephyr async UART. */
typedef struct ZephyrRBRInstrumentIO
{
    /** The UART device. */
    const struct device *dev;

    /** Read state. */
    struct
    {
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
    struct
    {
        /** Used to prevent mutual reads across threads. */
        struct k_mutex mut;
        /** Given upon write completion. */
        struct k_sem sem;
    } tx;
} ZephyrRBRInstrumentIO;

RBRInstrumentError ZephyrRBRInstrumentIO_init(ZephyrRBRInstrumentIO *io,
                                              const struct device *dev);

RBRInstrumentError ZephyrRBRInstrumentIO_read(const struct RBRInstrument *instrument,
                                              void *data,
                                              int32_t *size);

RBRInstrumentError ZephyrRBRInstrumentIO_write(const struct RBRInstrument *instrument,
                                               const void *const data,
                                               int32_t size);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_ZEPHYR_IO_H */
