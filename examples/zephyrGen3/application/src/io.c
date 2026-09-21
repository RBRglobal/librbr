/**
 * \file io.c
 *
 * \brief I/O callback implementations using the Zephyr async UART API.
 *
 * \copyright
 * Copyright (c) 2025 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include <stddef.h>
#include <string.h>

#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "io.h"

LOG_MODULE_REGISTER(io, CONFIG_IO_LOG_LEVEL);

static int ZephyrRBRGen3IO_enableRead(ZephyrRBRGen3IO *io)
{
    return uart_rx_enable(
        io->dev, io->rx.buf, sizeof(io->rx.buf), CONFIG_INSTRUMENT_INTER_CHARACTER_TIMEOUT_USEC);
}

static void ZephyrRBRGen3IO_event(const struct device *dev, struct uart_event *event,
                                  void *userData)
{
    ZephyrRBRGen3IO *io = userData;

    switch (event->type) {
    case UART_TX_DONE:
        k_sem_give(&io->tx.sem);
        return;
    case UART_TX_ABORTED:
        /* Ignored. */
        return;

    case UART_RX_RDY:
        if (event->data.rx.offset != io->rx.head) {
            /* DMA didn't put the data at the expected offset. (We're probably
             * reading from a zephyr,native-pty-uart UART, which only ever
             * transfers to offset 0.) The safest thing we can do is reset both
             * the tail and head indices based on the indicated offset. */
            io->rx.tail = event->data.rx.offset;
            io->rx.head = event->data.rx.offset + event->data.rx.len;
        } else {
            /* If we were inverted (write position trailing the read position),
             * temporarily bump the write position by the length of the buffer
             * so we can directly compare the head and tail positions. */
            if (io->rx.head < io->rx.tail) {
                io->rx.head += sizeof(io->rx.buf);
            }

            io->rx.head += event->data.rx.len;

            /* If the distance between head and tail exceeds the usable size of
             * the buffer, bump the tail forward. */
            if (io->rx.head - io->rx.tail > sizeof(io->rx.buf) - 1) {
                io->rx.tail = io->rx.head - sizeof(io->rx.buf) + 1;
                io->rx.tail %= sizeof(io->rx.buf);
            }

            io->rx.head %= sizeof(io->rx.buf);
        }

        k_sem_give(&io->rx.sem);

        return;
    case UART_RX_BUF_REQUEST:
        /* Emulate circular buffering on DMA controllers that don't support it
         * natively (e.g., STM32 DMA v1) by reusing the same buffer. */
        uart_rx_buf_rsp(io->dev, io->rx.buf, sizeof(io->rx.buf));

        return;
    case UART_RX_BUF_RELEASED:
        /* Ignored. */
        return;
    case UART_RX_STOPPED:
        LOG_ERR("UART stopped: 0x%02X", event->data.rx_stop.reason);
        __fallthrough;
    case UART_RX_DISABLED:
        /* We don't expect reading to stop, but if it does, start it again. */
        (void) ZephyrRBRGen3IO_enableRead(io);
        return;
    }
}

RBRGen3Error ZephyrRBRGen3IO_init(ZephyrRBRGen3IO *io, const struct device *dev)
{
    memset(io, 0, sizeof(*io));

    io->dev = dev;

    k_mutex_init(&io->rx.mut);
    k_sem_init(&io->rx.sem, 0, 1);

    k_mutex_init(&io->tx.mut);
    k_sem_init(&io->tx.sem, 0, 1);

    int err = uart_callback_set(io->dev, ZephyrRBRGen3IO_event, io);
    if (err != 0) {
        return RBRGEN3_CALLBACK_ERROR;
    }

    err = ZephyrRBRGen3IO_enableRead(io);
    if (err != 0) {
        LOG_ERR("read: starting: %s", strerror(-err));
        return RBRGEN3_CALLBACK_ERROR;
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error ZephyrRBRGen3IO_read(const struct RBRGen3 *conn, void *data, int32_t *size)
{
    ZephyrRBRGen3IO *io = RBRGen3_getUserData(conn);

    k_timeout_t timeout = K_MSEC(RBRGen3_getCommandTimeout(conn));
    k_timepoint_t deadline = sys_timepoint_calc(timeout);

    int err = k_mutex_lock(&io->rx.mut, sys_timepoint_timeout(deadline));
    if (err != 0) {
        LOG_ERR("read: device busy");
        return RBRGEN3_TIMEOUT;
    }

    size_t cap = *size;

    uint8_t *buf = (uint8_t *) data;
    *size = 0;

    LOG_DBG("reading up to %d B", cap);

    err = k_sem_take(&io->rx.sem, sys_timepoint_timeout(deadline));
    if (err != 0) {
        LOG_ERR("read: timeout");
        goto done;
    }

    while (*size < cap && io->rx.tail != io->rx.head) {
        buf[*size] = io->rx.buf[io->rx.tail];

        *size += 1;

        io->rx.tail += 1;
        io->rx.tail %= sizeof(io->rx.buf);
    }

    if (io->rx.tail != io->rx.head) {
        /* Destination buffer filled before we ran out of data, so we'll give
         * the semaphore back so the next call to this function yields more
         * data without blocking. */
        k_sem_give(&io->rx.sem);
    }

#ifdef CONFIG_TRACE_IO
    LOG_DBG("read %d B: %.*s", *size, *size, (const char *) data);
#else
    LOG_DBG("read %d B", *size);
#endif

done:
    k_mutex_unlock(&io->rx.mut);

    switch (err) {
    case 0:
        return RBRGEN3_SUCCESS;
    case -EAGAIN:
        return RBRGEN3_TIMEOUT;
    default:
        return RBRGEN3_CALLBACK_ERROR;
    }
}

RBRGen3Error ZephyrRBRGen3IO_write(const struct RBRGen3 *conn, const void *const data, int32_t size)
{
    ZephyrRBRGen3IO *io = RBRGen3_getUserData(conn);

    k_timeout_t timeout = K_MSEC(RBRGen3_getCommandTimeout(conn));
    k_timepoint_t deadline = sys_timepoint_calc(timeout);

    int err = k_mutex_lock(&io->tx.mut, sys_timepoint_timeout(deadline));
    if (err != 0) {
        LOG_ERR("write: device busy");
        return RBRGEN3_TIMEOUT;
    }

    /* Ensure there is no other write in progress. */
    (void) uart_tx_abort(io->dev);

    LOG_DBG("enqueueing %d B", size);

    k_sem_reset(&io->tx.sem);

    err = uart_tx(io->dev, data, size, SYS_FOREVER_US);
    if (err != 0) {
        LOG_ERR("write: starting: %s", strerror(-err));
        goto done;
    }

    err = k_sem_take(&io->tx.sem, sys_timepoint_timeout(deadline));
    if (err == 0) {
#ifdef CONFIG_TRACE_IO
        LOG_DBG("wrote %d B: %.*s", size, size, (const char *) data);
#else
        LOG_DBG("wrote %d B", size);
#endif
        goto done;
    }

    LOG_ERR("write: timeout");

    /* Abort the write so that the caller can safely release or reuse data. */
    (void) uart_tx_abort(io->dev);

done:
    k_mutex_unlock(&io->tx.mut);

    switch (err) {
    case 0:
        return RBRGEN3_SUCCESS;
    case -EAGAIN:
        return RBRGEN3_TIMEOUT;
    default:
        return RBRGEN3_CALLBACK_ERROR;
    }
}
