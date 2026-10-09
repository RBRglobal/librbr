/*
 * Copyright (c) 2026 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file posix-shared.c
 *
 * \brief Shared functions used by the libRBR POSIX examples.
 */

/* Prerequisite for clock_gettime, struct timespec in time.h. */
#define _POSIX_C_SOURCE 200112L

/* Required for open. */
#include <fcntl.h>
/* Required for memset. */
#include <string.h>
/* Required for select. */
#include <sys/select.h>
/* Required for open. */
#include <sys/stat.h>
/* Required for tcsetattr, struct termios. */
#include <termios.h>
/* Required for clock_gettime, nanosleep, struct timespec. */
#include <time.h>
/* Required for read, write. */
#include <unistd.h>

#include "posix-shared.h"

int openSerialFd(const char *devicePath)
{
    int instrumentFd;
    if ((instrumentFd = open(devicePath, O_RDWR | O_NOCTTY)) < 0) {
        return -1;
    }

    struct termios portSettings;
    memset(&portSettings, 0, sizeof(struct termios));
    portSettings.c_iflag = 0;
    portSettings.c_oflag = 0;
    portSettings.c_cflag = CS8 | CLOCAL | CREAD;
    portSettings.c_lflag = 0;
    portSettings.c_cc[VMIN] = 0;
    portSettings.c_cc[VTIME] = INSTRUMENT_CHARACTER_TIMEOUT_MSEC / 100;

#ifndef B115200
/* POSIX technically only defines termios baud rates up to 38,400 baud. On most
 * platforms (Linux/Cygwin), higher baud rates are defined regardless (and on
 * Cygwin, the baud rate constants are not the literal baud rates). However,
 * the macOS termios headers guard the extended baud rate definitions with a
 * check for whether _POSIX_C_SOURCE has been left undefined. Technically, we
 * should use the IOSSIOSPEED ioctl to use arbitrary baud rates higher than
 * 38,400 baud on macOS, but defining the constant ourselves should be safe
 * enough: the constant definition _can_ be found in the termios headers (just
 * for different preconditions) and is nearly guaranteed not to change (as
 * doing so would break a vast number of existing applications). And this
 * approach is more platform-generic than an ioctl. */
#define B115200 115200
#endif

    /* The instrument default; irrelevant over USB. */
    cfsetospeed(&portSettings, B115200);

    /* Input baud rate of 0 causes the output baud rate to be used. */
    cfsetispeed(&portSettings, B0);

    if (tcsetattr(instrumentFd, TCSANOW, &portSettings) < 0) {
        close(instrumentFd);
        return -1;
    }
    return instrumentFd;
}

/*
 * The Gen3 and Gen4 callback signatures differ only in the connection and
 * error types they carry, so the actual communication with the instrument is
 * implemented once here; the per-generation callbacks below are thin
 * wrappers that translate to/from the right types.
 */

/** \brief The outcome of a raw I/O operation, generation-independent. */
typedef enum RawIoStatus {
    RAW_IO_SUCCESS,
    RAW_IO_TIMEOUT,
    RAW_IO_ERROR,
} RawIoStatus;

static int64_t rawTime(void)
{
    struct timespec result;
    clock_gettime(CLOCK_MONOTONIC, &result);
    return (result.tv_sec * 1000) + (result.tv_nsec / 1000000);
}

static void rawSleep(int64_t time)
{
    struct timespec sleep = {
        .tv_sec = time / 1000,
        .tv_nsec = (time % 1000) * 1000000,
    };
    nanosleep(&sleep, NULL);
}

static RawIoStatus rawRead(int fd, void *data, int32_t *size)
{
    /* A select() call to enforce a read timeout is unnecessary because we
     * configured the serial port in noncanonical mode and specified a read
     * timeout on the port itself. */
    *size = read(fd, data, *size);
    if (*size == 0) {
        return RAW_IO_TIMEOUT;
    } else if (*size < 0) {
        return RAW_IO_ERROR;
    } else {
        return RAW_IO_SUCCESS;
    }
}

static RawIoStatus rawWrite(int fd, const void *const data, int32_t size)
{
    const uint8_t *const byteData = (const uint8_t *const) data;
    int32_t written = 0;

    fd_set instrumentFdSet;
    FD_ZERO(&instrumentFdSet);
    /* We don't need to FD_SET on every loop iteration because there's only one
     * fd in the set, and we immediately return an error if it's omitted from
     * the response. */
    FD_SET(fd, &instrumentFdSet);

    struct timeval writeTimeout;

    while (written < size) {
        /* select() may (and on Linux, does) update the timeout argument with
         * how much of the timeout remained upon return. We want every check to
         * have the same timeout, so we'll reset it before each use. */
        writeTimeout = (struct timeval) {
            .tv_sec = INSTRUMENT_CHARACTER_TIMEOUT_MSEC / 1000,
            .tv_usec = (INSTRUMENT_CHARACTER_TIMEOUT_MSEC % 1000) * 1000,
        };

        /* We could just loop on write(), but we want to enforce a timeout, so
         * select() kills two birds with one stone: making sure the output
         * device is ready to be written to, and handling the timeout. */
        int instrumentReady = select(fd + 1, NULL, &instrumentFdSet, NULL, &writeTimeout);
        if (instrumentReady < 0) {
            return RAW_IO_ERROR;
        } else if (instrumentReady == 0) {
            return RAW_IO_TIMEOUT;
        }

        int32_t chunkWritten = write(fd, byteData + written, size - written);
        /* select() told us we were good to go, so a 0-byte write is probably
         * an error, not just an unready device. */
        if (chunkWritten <= 0) {
            return RAW_IO_ERROR;
        }

        written += chunkWritten;
    }

    return RAW_IO_SUCCESS;
}

RBRGen3Error gen3InstrumentTime(const RBRGen3 *conn, RBRGen3DateTime *time)
{
    /* Unused. */
    (void) conn;

    *time = rawTime();
    return RBRGEN3_SUCCESS;
}

RBRGen3Error gen3InstrumentSleep(const RBRGen3 *conn, RBRGen3DateTime time)
{
    /* Unused. */
    (void) conn;

    rawSleep(time);
    return RBRGEN3_SUCCESS;
}

RBRGen3Error gen3InstrumentRead(const RBRGen3 *conn, void *data, int32_t *size)
{
    int *instrumentFd = (int *) RBRGen3_getUserData(conn);

    switch (rawRead(*instrumentFd, data, size)) {
    case RAW_IO_TIMEOUT:
        return RBRGEN3_TIMEOUT;
    case RAW_IO_ERROR:
        return RBRGEN3_CALLBACK_ERROR;
    default:
        return RBRGEN3_SUCCESS;
    }
}

RBRGen3Error gen3InstrumentWrite(const RBRGen3 *conn, const void *const data, int32_t size)
{
    int *instrumentFd = (int *) RBRGen3_getUserData(conn);

    switch (rawWrite(*instrumentFd, data, size)) {
    case RAW_IO_TIMEOUT:
        return RBRGEN3_TIMEOUT;
    case RAW_IO_ERROR:
        return RBRGEN3_CALLBACK_ERROR;
    default:
        return RBRGEN3_SUCCESS;
    }
}

RBRGen4Error gen4InstrumentTime(const RBRGen4 *conn, RBRGen4DateTime *time)
{
    /* Unused. */
    (void) conn;

    *time = rawTime();
    return RBRGEN4_SUCCESS;
}

RBRGen4Error gen4InstrumentSleep(const RBRGen4 *conn, RBRGen4DateTime time)
{
    /* Unused. */
    (void) conn;

    rawSleep(time);
    return RBRGEN4_SUCCESS;
}

RBRGen4Error gen4InstrumentRead(const RBRGen4 *conn, void *data, int32_t *size)
{
    int *instrumentFd = (int *) RBRGen4_getUserData(conn);

    switch (rawRead(*instrumentFd, data, size)) {
    case RAW_IO_TIMEOUT:
        return RBRGEN4_TIMEOUT;
    case RAW_IO_ERROR:
        return RBRGEN4_CALLBACK_ERROR;
    default:
        return RBRGEN4_SUCCESS;
    }
}

RBRGen4Error gen4InstrumentWrite(const RBRGen4 *conn, const void *const data, int32_t size)
{
    int *instrumentFd = (int *) RBRGen4_getUserData(conn);

    switch (rawWrite(*instrumentFd, data, size)) {
    case RAW_IO_TIMEOUT:
        return RBRGEN4_TIMEOUT;
    case RAW_IO_ERROR:
        return RBRGEN4_CALLBACK_ERROR;
    default:
        return RBRGEN4_SUCCESS;
    }
}
