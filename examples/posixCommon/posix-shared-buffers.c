/**
 * \file posix-shared-buffers.c
 *
 * \brief Drive a Gen3 and a Gen4 instrument through one set of buffers.
 *
 * An application which talks to instruments of both generations, but only
 * one at a time, can give both connections the same command and response
 * storage. This example opens a Gen3 and a Gen4 connection on shared buffers
 * and alternates commands between them, resetting the response buffer each
 * time the other connection takes over.
 *
 * \copyright
 * Copyright (c) 2026 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Prerequisite for clock_gettime, struct timespec in time.h. */
#define _POSIX_C_SOURCE 200112L

/* Required for errno. */
#include <errno.h>
/* Required for open. */
#include <fcntl.h>
/* Required for PRId32, PRIu32. */
#include <inttypes.h>
/* Required for fprintf, printf. */
#include <stdio.h>
/* Required for EXIT_FAILURE, EXIT_SUCCESS. */
#include <stdlib.h>
/* Required for memset, strerror. */
#include <string.h>
/* Required for tcsetattr, struct termios. */
#include <termios.h>
/* Required for clock_gettime, nanosleep, struct timespec. */
#include <time.h>
/* Required for close, read, write. */
#include <unistd.h>

#include "RBRGen3.h"
#include "RBRGen3Other.h"
#include "RBRGen4.h"
#include "RBRGen4Instrument.h"

#define INSTRUMENT_CHARACTER_TIMEOUT_MSEC 4000
#define INSTRUMENT_COMMAND_TIMEOUT_MSEC   10000

/* One buffer set has to hold the longer of the two generations' commands and
 * responses. */
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define SHARED_COMMAND_BUFFER_SIZE \
    MAX(RBRGEN3_COMMAND_BUFFER_DEFAULT, RBRGEN4_COMMAND_BUFFER_DEFAULT)
#define SHARED_RESPONSE_BUFFER_SIZE \
    MAX(RBRGEN3_RESPONSE_BUFFER_DEFAULT, RBRGEN4_RESPONSE_BUFFER_DEFAULT)

/* Both connections read and write through the same POSIX helpers; the
 * per-generation callbacks below only adapt the types. */

static int openSerialFd(const char *devicePath)
{
    int fd;
    if ((fd = open(devicePath, O_RDWR | O_NOCTTY)) < 0) {
        return -1;
    }

    struct termios portSettings;
    memset(&portSettings, 0, sizeof(struct termios));
    portSettings.c_cflag = CS8 | CLOCAL | CREAD;
    portSettings.c_cc[VMIN] = 0;
    portSettings.c_cc[VTIME] = INSTRUMENT_CHARACTER_TIMEOUT_MSEC / 100;
#ifndef B115200
/* POSIX only defines termios baud rates up to 38,400 baud; macOS hides the
 * higher ones when _POSIX_C_SOURCE is defined. The constant is stable, so
 * define it ourselves, as the per-generation examples do. */
#define B115200 115200
#endif
    /* Irrelevant over USB; over a serial line, match each instrument. */
    cfsetospeed(&portSettings, B115200);
    cfsetispeed(&portSettings, B0);
    if (tcsetattr(fd, TCSANOW, &portSettings) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

static int64_t nowMsec(void)
{
    struct timespec result;
    clock_gettime(CLOCK_MONOTONIC, &result);
    return (result.tv_sec * 1000) + (result.tv_nsec / 1000000);
}

static void sleepMsec(int64_t msec)
{
    struct timespec sleep = {.tv_sec = msec / 1000, .tv_nsec = (msec % 1000) * 1000000};
    nanosleep(&sleep, NULL);
}

/* Returns the byte count read, 0 on timeout, or -1 on error. */
static int32_t readFd(int fd, void *data, int32_t size)
{
    return (int32_t) read(fd, data, size);
}

/* Returns 0 on success or -1 on error. */
static int writeFd(int fd, const void *data, int32_t size)
{
    const uint8_t *bytes = data;
    int32_t written = 0;
    while (written < size) {
        ssize_t n = write(fd, bytes + written, size - written);
        /* A zero-byte write makes no progress; treat it as an error rather
         * than spin. */
        if (n <= 0) {
            return -1;
        }
        written += (int32_t) n;
    }
    return 0;
}

static RBRGen3Error gen3Time(const struct RBRGen3 *conn, RBRGen3DateTime *time)
{
    (void) conn;
    *time = nowMsec();
    return RBRGEN3_SUCCESS;
}

static RBRGen3Error gen3Sleep(const struct RBRGen3 *conn, RBRGen3DateTime time)
{
    (void) conn;
    sleepMsec(time);
    return RBRGEN3_SUCCESS;
}

static RBRGen3Error gen3Read(const struct RBRGen3 *conn, void *data, int32_t *size)
{
    *size = readFd(*(int *) RBRGen3_getUserData(conn), data, *size);
    return *size == 0 ? RBRGEN3_TIMEOUT : *size < 0 ? RBRGEN3_CALLBACK_ERROR : RBRGEN3_SUCCESS;
}

static RBRGen3Error gen3Write(const struct RBRGen3 *conn, const void *const data, int32_t size)
{
    return writeFd(*(int *) RBRGen3_getUserData(conn), data, size) < 0 ? RBRGEN3_CALLBACK_ERROR
                                                                       : RBRGEN3_SUCCESS;
}

static RBRGen4Error gen4Time(const struct RBRGen4 *conn, RBRGen4DateTime *time)
{
    (void) conn;
    *time = nowMsec();
    return RBRGEN4_SUCCESS;
}

static RBRGen4Error gen4Sleep(const struct RBRGen4 *conn, RBRGen4DateTime time)
{
    (void) conn;
    sleepMsec(time);
    return RBRGEN4_SUCCESS;
}

static RBRGen4Error gen4Read(const struct RBRGen4 *conn, void *data, int32_t *size)
{
    *size = readFd(*(int *) RBRGen4_getUserData(conn), data, *size);
    return *size == 0 ? RBRGEN4_TIMEOUT : *size < 0 ? RBRGEN4_CALLBACK_ERROR : RBRGEN4_SUCCESS;
}

static RBRGen4Error gen4Write(const struct RBRGen4 *conn, const void *const data, int32_t size)
{
    return writeFd(*(int *) RBRGen4_getUserData(conn), data, size) < 0 ? RBRGEN4_CALLBACK_ERROR
                                                                       : RBRGEN4_SUCCESS;
}

/*
 * Each connection tracks how much unread data it left in the response buffer,
 * even when that is only the prompt after its last reply. Once the other
 * connection has written into the shared buffer, that tracking describes
 * bytes which are gone, so a connection resets it before its first command
 * after a switch. Neither instrument streams here, so the reset costs
 * nothing; a streaming connection would lose its undelivered samples on every
 * switch and should own its response buffer. The command buffer needs no
 * such care; it holds nothing between commands.
 */
typedef enum Owner {
    NOBODY,
    GEN3,
    GEN4
} Owner;
static Owner bufferOwner = NOBODY;

static void useGen3(RBRGen3 *conn)
{
    if (bufferOwner != GEN3) {
        RBRGen3_resetResponseBuffer(conn);
        bufferOwner = GEN3;
    }
}

static void useGen4(RBRGen4 *conn)
{
    if (bufferOwner != GEN4) {
        RBRGen4_resetResponseBuffer(conn);
        bufferOwner = GEN4;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s gen3-device gen4-device\n", argv[0]);
        return EXIT_FAILURE;
    }

    int status = EXIT_FAILURE;
    int gen3Fd = -1;
    int gen4Fd = -1;
    if ((gen3Fd = openSerialFd(argv[1])) < 0 || (gen4Fd = openSerialFd(argv[2])) < 0) {
        fprintf(stderr, "%s: Failed to open serial device: %s!\n", argv[0], strerror(errno));
        goto cleanup;
    }

    /* One set of storage, described to each generation in its own terms. */
    static uint8_t commandBuffer[SHARED_COMMAND_BUFFER_SIZE];
    static uint8_t responseBuffer[SHARED_RESPONSE_BUFFER_SIZE];

    printf("Contexts: RBRGen3 %zu bytes, RBRGen4 %zu bytes. Shared buffers: %zu bytes.\n",
           sizeof(RBRGen3),
           sizeof(RBRGen4),
           sizeof(commandBuffer) + sizeof(responseBuffer));

    RBRGen3 gen3;
    RBRGen3Environment gen3Environment = {
        .time = gen3Time,
        .sleep = gen3Sleep,
        .read = gen3Read,
        .write = gen3Write,
        .command = commandBuffer,
        .commandCapacity = sizeof(commandBuffer),
        .response = responseBuffer,
        .responseCapacity = sizeof(responseBuffer),
    };
    RBRGen3Error err3;
    /* Opening a connection starts it with an empty response buffer, so no
     * reset is needed before the first open. */
    bufferOwner = GEN3;
    if ((err3 = RBRGen3_open(&gen3, &gen3Environment, INSTRUMENT_COMMAND_TIMEOUT_MSEC, &gen3Fd)) !=
        RBRGEN3_SUCCESS) {
        fprintf(stderr,
                "%s: Failed to open the Gen3 instrument: %s!\n",
                argv[0],
                RBRGen3Error_name(err3));
        goto cleanup;
    }

    RBRGen4 gen4;
    RBRGen4Environment gen4Environment = {
        .time = gen4Time,
        .sleep = gen4Sleep,
        .read = gen4Read,
        .write = gen4Write,
        .command = commandBuffer,
        .commandCapacity = sizeof(commandBuffer),
        .response = responseBuffer,
        .responseCapacity = sizeof(responseBuffer),
    };
    RBRGen4Error err4;
    bufferOwner = GEN4;
    if ((err4 = RBRGen4_open(&gen4, &gen4Environment, INSTRUMENT_COMMAND_TIMEOUT_MSEC, &gen4Fd)) !=
        RBRGEN4_SUCCESS) {
        fprintf(stderr,
                "%s: Failed to open the Gen4 instrument: %s!\n",
                argv[0],
                RBRGen4Error_name(err4));
        goto closeGen3;
    }

    /* Alternate between the two instruments through the same storage. */
    for (int round = 0; round < 3; ++round) {
        RBRGen3Id id3;
        useGen3(&gen3);
        if ((err3 = RBRGen3_getId(&gen3, &id3)) != RBRGEN3_SUCCESS) {
            fprintf(stderr, "%s: Gen3 id failed: %s!\n", argv[0], RBRGen3Error_name(err3));
            goto closeGen4;
        }
        printf("Gen3: %s %" PRIu32 " firmware %s\n", id3.model, id3.serial, id3.version);

        RBRGen4Id4 id4;
        useGen4(&gen4);
        if ((err4 = RBRGen4_getId4(&gen4, &id4)) != RBRGEN4_SUCCESS) {
            fprintf(stderr, "%s: Gen4 id4 failed: %s!\n", argv[0], RBRGen4Error_name(err4));
            goto closeGen4;
        }
        printf("Gen4: %s %" PRId32 " firmware %s\n", id4.model, id4.sn, id4.fwversion);
    }
    status = EXIT_SUCCESS;

closeGen4:
    RBRGen4_close(&gen4);
closeGen3:
    RBRGen3_close(&gen3);
cleanup:
    if (gen4Fd >= 0) {
        close(gen4Fd);
    }
    if (gen3Fd >= 0) {
        close(gen3Fd);
    }
    return status;
}
