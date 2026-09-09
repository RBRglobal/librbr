/**
 * \file posix-shared.c
 *
 * \brief Shared functions used by the libRBR POSIX examples.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
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

int openSerialFd(char *devicePath)
{
    int instrumentFd;
    if ((instrumentFd = open(devicePath, O_RDWR | O_NOCTTY)) < 0)
    {
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
#define B9600 9600
#endif

    /*important!!!
     change baudrate below if one is using 115200:
     */
    cfsetospeed(&portSettings, B9600);

    /* Input baud rate of 0 causes the output baud rate to be used. */
    cfsetispeed(&portSettings, B0);

    if (tcsetattr(instrumentFd, TCSANOW, &portSettings) < 0)
    {
        close(instrumentFd);
        return -1;
    }
    return instrumentFd;
}

RBRGen3Error instrumentTime(const struct RBRGen3 *instrument,
                                  RBRGen3DateTime *time)
{
    /* Unused. */
    (void) instrument;

    struct timespec result;
    clock_gettime(CLOCK_MONOTONIC, &result);
    *time = (result.tv_sec * 1000) + (result.tv_nsec / 1000000);
    return RBRGEN3_SUCCESS;
}

RBRGen3Error instrumentSleep(const struct RBRGen3 *instrument,
                                   RBRGen3DateTime time)
{
    /* Unused. */
    (void) instrument;

    struct timespec sleep = {
        .tv_sec  =  time / 1000,
        .tv_nsec = (time % 1000) * 1000000
    };
    nanosleep(&sleep, NULL);
    return RBRGEN3_SUCCESS;
}

RBRGen3Error instrumentRead(const struct RBRGen3 *instrument,
                                  void *data,
                                  int32_t *size)
{
    int *instrumentFd = (int *) RBRGen3_getUserData(instrument);

    /* A select() call to enforce a read timeout is unnecessary because we
     * configured the serial port in noncanonical mode and specified a read
     * timeout on the port itself. On one hand, this reduces the complexity of
     * read operations; on the other, it means timeouts can't conveniently be
     * changed based on context. For example, you might want to have a much
     * longer timeout for the `enable` or `memclear` commands than for `id`. */
    *size = read(*instrumentFd,
                 data,
                 *size);
    if (*size == 0)
    {
        return RBRGEN3_TIMEOUT;
    }
    else if (*size < 0)
    {
        return RBRGEN3_CALLBACK_ERROR;
    }
    else
    {
        return RBRGEN3_SUCCESS;
    }
}

RBRGen3Error instrumentWrite(const struct RBRGen3 *instrument,
                                   const void *const data,
                                   int32_t size)
{
    int *instrumentFd = (int *) RBRGen3_getUserData(instrument);
    const uint8_t *const byteData = (const uint8_t *const) data;
    int32_t written = 0;

    fd_set instrumentFdSet;
    FD_ZERO(&instrumentFdSet);
    /* We don't need to FD_SET on every loop iteration because there's only one
     * fd in the set, and we immediately return an error if it's omitted from
     * the response. */
    FD_SET(*instrumentFd, &instrumentFdSet);

    struct timeval writeTimeout;

    while (written < size)
    {
        /* select() may (and on Linux, does) update the timeout argument with
         * how much of the timeout remained upon return. We want every check to
         * have the same timeout, so we'll reset it before each use. */
        writeTimeout = (struct timeval) {
            .tv_sec  =  INSTRUMENT_CHARACTER_TIMEOUT_MSEC / 1000,
            .tv_usec = (INSTRUMENT_CHARACTER_TIMEOUT_MSEC % 1000) * 1000
        };

        /* We could just loop on write(), but we want to enforce a timeout, so
         * select() kills two birds with one stone: making sure the output
         * device is ready to be written to, and handling the timeout. */
        int instrumentReady = select(*instrumentFd + 1,
                                     NULL,
                                     &instrumentFdSet,
                                     NULL,
                                     &writeTimeout);
        if (instrumentReady < 0)
        {
            return RBRGEN3_CALLBACK_ERROR;
        }
        else if (instrumentReady == 0)
        {
            return RBRGEN3_TIMEOUT;
        }

        int32_t chunkWritten = write(*instrumentFd,
                                     byteData + written,
                                     size - written);
        /* select() told us we were good to go, so a 0-byte write is probably
         * an error, not just an unready device. */
        if (chunkWritten <= 0)
        {
            return RBRGEN3_CALLBACK_ERROR;
        }

        written += chunkWritten;
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error instrumentStart(RBRGen3 *instrument)
{
    RBRGen3Error err;

    RBRGen3DeploymentStatus status;
    if ((err = RBRGen3_disable(instrument, &status))
        != RBRGEN3_SUCCESS)
    {
        return err;
    }

    RBRGen3Sampling sampling;
    if ((err = RBRGen3_getSampling(instrument, &sampling))
        != RBRGEN3_SUCCESS)
    {
        return err;
    }
    sampling.mode = RBRGEN3_SAMPLING_CONTINUOUS;
    sampling.period = sampling.userPeriodLimit;
    if ((err = RBRGen3_setSampling(instrument, &sampling))
        != RBRGEN3_SUCCESS)
    {
        return err;
    }

    RBRGen3Deployment deployment = {
        .startTime = RBRGEN3_DATETIME_MIN,
        .endTime = RBRGEN3_DATETIME_MAX
    };
    if ((err = RBRGen3_setDeployment(instrument, &deployment))
        != RBRGEN3_SUCCESS)
    {
        return err;
    }

    if ((err = RBRGen3_setNewMemoryFormat(
             instrument,
             RBRGEN3_MEMFORMAT_CALBIN00))
        != RBRGEN3_SUCCESS)
    {
        return err;
    }

    RBRGen3Thresholding thresholding;
    err = RBRGen3_getThresholding(instrument, &thresholding);
    if (err == RBRGEN3_SUCCESS && thresholding.enabled)
    {
        thresholding.enabled = false;
        RBRGen3_setThresholding(instrument, &thresholding);
    }

    RBRGen3TwistActivation twistActivation;
    err = RBRGen3_getTwistActivation(instrument, &twistActivation);
    if (err == RBRGEN3_SUCCESS && twistActivation.enabled)
    {
        twistActivation.enabled = false;
        RBRGen3_setTwistActivation(instrument, &twistActivation);
    }

    if ((err = RBRGen3_enable(instrument, true, &status))
        != RBRGEN3_SUCCESS)
    {
        return err;
    }

    return RBRGEN3_SUCCESS;
}
