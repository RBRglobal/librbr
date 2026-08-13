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
/* Required for fprintf, printf, snprintf. */
#include <stdio.h>

/* Required for checking errors in channel label. */
#include <stdbool.h>
#include <inttypes.h>

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Commands.h"
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
    cfsetospeed(&portSettings, B115200);

    /* Input baud rate of 0 causes the output baud rate to be used. */
    cfsetispeed(&portSettings, B0);

    if (tcsetattr(instrumentFd, TCSANOW, &portSettings) < 0)
    {
        close(instrumentFd);
        return -1;
    }
    return instrumentFd;
}

RBRInstrumentGen4Error instrumentTime(const struct RBRInstrumentGen4 *instrument,
                                      RBRInstrumentGen4DateTime *time)
{
    /* Unused. */
    (void) instrument;
    struct timespec result;
    clock_gettime(CLOCK_MONOTONIC, &result);
    *time = (result.tv_sec * 1000) + (result.tv_nsec / 1000000);
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error instrumentSleep(const struct RBRInstrumentGen4 *instrument,
                                       RBRInstrumentGen4DateTime time)
{
    /* Unused. */
    (void) instrument;

    struct timespec sleep = {
        .tv_sec = time / 1000,
        .tv_nsec = (time % 1000) * 1000000
    };
    nanosleep(&sleep, NULL);
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error instrumentRead(const struct RBRInstrumentGen4 *instrument,
                                      void *data,
                                      int32_t *size)
{
    int *instrumentFd = (int *) RBRInstrumentGen4_getUserData(instrument);

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
        return RBRINSTRUMENTGEN4_TIMEOUT;
    }
    else if (*size < 0)
    {
        return RBRINSTRUMENTGEN4_CALLBACK_ERROR;
    }
    else
    {
        return RBRINSTRUMENTGEN4_SUCCESS;
    }
}

RBRInstrumentGen4Error instrumentWrite(const struct RBRInstrumentGen4 *instrument,
                                       const void *const data,
                                       int32_t size)
{
    int *instrumentFd = (int *) RBRInstrumentGen4_getUserData(instrument);
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
        writeTimeout = (struct timeval){
            .tv_sec = INSTRUMENT_CHARACTER_TIMEOUT_MSEC / 1000,
            .tv_usec = (INSTRUMENT_CHARACTER_TIMEOUT_MSEC % 1000) * 1000000
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
            return RBRINSTRUMENTGEN4_CALLBACK_ERROR;
        }
        else if (instrumentReady == 0)
        {
            return RBRINSTRUMENTGEN4_TIMEOUT;
        }

        int32_t chunkWritten = write(*instrumentFd,
                                     byteData + written,
                                     size - written);
        /* select() told us we were good to go, so a 0-byte write is probably
         * an error, not just an unready device. */
        if (chunkWritten <= 0)
        {
            return RBRINSTRUMENTGEN4_CALLBACK_ERROR;
        }

        written += chunkWritten;
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

/******************* posix helper functions ***********************/
//---------------- define helper functions --------------------------------------------------
// Parses a schedule-format time string ("YYYYMMDDhhmmss", UTC) into a
// millisecond Unix timestamp. The library's own parser,
// RBRInstrumentGen4DateTime_parseScheduleTime(), lives in
// RBRInstrumentGen4Internal.c and is not part of the public API.
void str_to_deploymentDatetime(RBRInstrumentGen4DateTime *targetDatetime,
                               const char *sourceDatetime)
{
    // Days from the start of the year to the start of each month (non-leap).
    static const int daysBeforeMonth[12] = {
        0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
    };

    int year, month, day, hour, minute, second;
    if (sscanf(sourceDatetime,
               "%4d%2d%2d%2d%2d%2d",
               &year,
               &month,
               &day,
               &hour,
               &minute,
               &second) != 6
        || year < 1970
        || month < 1 || month > 12
        || day < 1 || day > 31
        || hour > 23 || minute > 59 || second > 59)
    {
        fprintf(stderr,
                "Error: invalid deployment datetime '%s'!\n",
                sourceDatetime);
        *targetDatetime = 0;
        return;
    }

    // Whole days since the Unix epoch, counting leap days: every fourth
    // year, except century years not divisible by 400.
    RBRInstrumentGen4DateTime days =
        (RBRInstrumentGen4DateTime) (year - 1970) * 365
        + (year - 1969) / 4
        - (year - 1901) / 100
        + (year - 1601) / 400
        + daysBeforeMonth[month - 1]
        + (day - 1);
    if (month > 2
        && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0))
    {
        days += 1;
    }

    *targetDatetime =
        (((days * 24 + hour) * 60 + minute) * 60 + second) * 1000;
}

// can be static.
void cpy_ptrArray_forChannel(RBRInstrumentGen4Channel *target[],
                             RBRInstrumentGen4Channel *source[],
                             int count)
{
    for (int i = 0; i < count; i++)
    {
        target[i] = source[i];
    }
}

// can be static.
void cpy_ptrArray_forGroup(RBRInstrumentGen4Group *target[],
                           RBRInstrumentGen4Group *source[],
                           int count)
{
    for (int i = 0; i < count; i++)
    {
        target[i] = source[i];
    }
}

// can be static.
void cpy_ptrArray_forSchedule(RBRInstrumentGen4Schedule *target[],
                              RBRInstrumentGen4Schedule *source[],
                              int count)
{
    for (int i = 0; i < count; i++)
    {
        target[i] = source[i];
    }
}

/* Channel, group, and schedule configurations are TBD. */
// can be static.
RBRInstrumentGen4Error RBRInstrumentGen4_populateGroupChannels(
    RBRInstrumentGen4Group *group,
    RBRInstrumentGen4ChannelPool *channelPool,
    const char specifiedChannelLabels[][RBRINSTRUMENTGEN4_CHANNEL_LABEL_MAX],
    int32_t specifiedChannelLabelCnt)
{
    if (specifiedChannelLabelCnt < 1
        || specifiedChannelLabelCnt > RBRINSTRUMENTGEN4_CHANNEL_MAX)
    {
        fprintf(stderr, "Error: invalid channel label count!\n");
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    // init pointer array pointing to sourceObjects
    RBRInstrumentGen4Channel *_newPtrList[RBRINSTRUMENTGEN4_CHANNEL_MAX] = { NULL };

    // find out each label specified, and compare with all sourceObjList.
    // and fill the pointer array.
    int32_t _currentIndex = 0;
    int32_t _totalSourceObjCnt = channelPool->count;

    bool flag; // set flag if a specified label is found.
    for (int32_t j = 0; j < specifiedChannelLabelCnt; j++)
    {
        flag = false;
        for (int32_t i = 0; i < _totalSourceObjCnt; i++)
        {
            if (strcmp(channelPool->pool[i].label, specifiedChannelLabels[j]) == 0)
            {
                _newPtrList[_currentIndex] = &(channelPool->pool[i]);
                _currentIndex++;
                flag = true;
                break;
            }
        }
        if (flag == false) // label not found
        {
            fprintf(stderr, "Warning: label '%s' specified does not exist!\n", specifiedChannelLabels[j]);
        }
    }

    if (_currentIndex == 0) // none of the label specified is valid.
    {
        fprintf(stderr, "Error: none of the specified labels exist!\n");
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    // Only the channels which were actually found are copied to the group.
    cpy_ptrArray_forChannel(group->channelList, _newPtrList, _currentIndex);
    group->count = _currentIndex;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

// can be static.
RBRInstrumentGen4Error RBRInstrumentGen4_populateScheduleGroups(
    RBRInstrumentGen4Schedule *schedule,
    RBRInstrumentGen4GroupPool *groupPool,
    const char specifiedGroupLabels[][RBRINSTRUMENTGEN4_LABEL_NAME_MAX],
    int32_t specifiedGroupLabelCnt)
{
    (void)schedule;
    (void)groupPool;
    (void)specifiedGroupLabels;
    (void)specifiedGroupLabelCnt;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

// can be static.
RBRInstrumentGen4Error RBRInstrumentGen4_populateConfigSchedules(
    RBRInstrumentGen4Config *config,
    RBRInstrumentGen4SchedulePool *schedulePool,
    const char specifiedScheduleLabels[][RBRINSTRUMENTGEN4_LABEL_NAME_MAX],
    int32_t specifiedScheduleLabelCnt)
{
    (void)config;
    (void)schedulePool;
    (void)specifiedScheduleLabels;
    (void)specifiedScheduleLabelCnt;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

// can be static.
RBRInstrumentGen4Error RBRInstrumentGen4_getDatasetFromPool(
    RBRInstrumentGen4Dataset **targetDataset,
    RBRInstrumentGen4DatasetPool *datasetPool,
    const char datasetLabel[])
{
    int _totalSourceObjCnt = datasetPool->count;
    if (_totalSourceObjCnt <= 0)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }
    else
    {
        bool _flag = false;
        for (int i = 0; i < _totalSourceObjCnt; i++)
        {
            if (strcmp(datasetPool->pool[i].label, datasetLabel) == 0) // warning: modify according to sourceObjList structure.
            {
                *targetDataset = &(datasetPool->pool[i]); // warning: modify according to sourceObjList structure.
                _flag = true;
                break;
            }
        }
        if (_flag == false) // label not found
        {
            fprintf(stderr, "Warning: label '%s' specified does not exist!\n", datasetLabel);
            return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
        }
    }
    return RBRINSTRUMENTGEN4_SUCCESS;
}

// can be static.
RBRInstrumentGen4Error RBRInstrumentGen4_getChannelFromPool(
RBRInstrumentGen4Channel **targetChannel,
    RBRInstrumentGen4ChannelPool *channelPool,
    const char channelLabel[])
{
    (void)targetChannel;
    (void)channelPool;
    (void)channelLabel;
    return RBRINSTRUMENTGEN4_SUCCESS;
}
// can be static.
RBRInstrumentGen4Error RBRInstrumentGen4_getGroupFromPool(
    RBRInstrumentGen4Group **targetGroup,
    RBRInstrumentGen4GroupPool *groupPool,
    const char groupLabel[])
{
    (void)targetGroup;
    (void)groupPool;
    (void)groupLabel;
    return RBRINSTRUMENTGEN4_SUCCESS;
}
// can be static.
RBRInstrumentGen4Error RBRInstrumentGen4_getScheduleFromPool(
    RBRInstrumentGen4Schedule **targetSchedule,
    RBRInstrumentGen4SchedulePool *schedulePool,
    const char scheduleLabel[])
{
    (void)targetSchedule;
    (void)schedulePool;
    (void)scheduleLabel;
    return RBRINSTRUMENTGEN4_SUCCESS;
}
// can be static.
RBRInstrumentGen4Error RBRInstrumentGen4_getConfigFromPool(
    RBRInstrumentGen4Config **targetConfig,
    RBRInstrumentGen4ConfigPool *configPool,
    const char configLabel[])
{
    (void)targetConfig;
    (void)configPool;
    (void)configLabel;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_initNewGroup(
    RBRInstrumentGen4 *instrument,
    const char newGroupLabel[],
    const char specifiedChannelLabels[][RBRINSTRUMENTGEN4_CHANNEL_LABEL_MAX],
    int32_t specifiedChannelLabelCnt,
    RBRInstrumentGen4ChannelPool *channelPool,
    RBRInstrumentGen4GroupPool *groupPool,
    RBRInstrumentGen4Group **newGroup)
{
    RBRInstrumentGen4_createGroup(instrument, newGroupLabel, groupPool, newGroup);
    RBRInstrumentGen4_populateGroupChannels(*newGroup,
                                    channelPool,
                                    specifiedChannelLabels,
                                    specifiedChannelLabelCnt);
    RBRInstrumentGen4_setGroup(instrument, *newGroup);
    return RBRINSTRUMENTGEN4_SUCCESS;
}
// can be static
RBRInstrumentGen4Error RBRInstrumentGen4_initNewSchedule(
    RBRInstrumentGen4 *instrument,
    const char newScheduleLabel[],
    const char specifiedGroupLabels[][RBRINSTRUMENTGEN4_LABEL_NAME_MAX],
    int32_t specifiedGroupLabelCnt,
    RBRInstrumentGen4SamplingMode mode,
    RBRInstrumentGen4GroupPool *groupPool,
    RBRInstrumentGen4SchedulePool *schedulePool,
    RBRInstrumentGen4Schedule **newSchedule)
{
    // depending on mode, create a mode-dependent structure.
    // Warning: in this example, i'll just create a regimes structure. But this function should handle all other modes as well.
    RBRInstrumentGen4_createSchedule(instrument, newScheduleLabel, schedulePool, newSchedule);
    RBRInstrumentGen4_populateScheduleGroups(*newSchedule, groupPool, specifiedGroupLabels, specifiedGroupLabelCnt); // warning: read err!!!
    (*newSchedule)->mode = mode;
    (*newSchedule)->stream = RBRINSTRUMENTGEN4_SCHEDULE_STREAM_OFF; // default value.
    (*newSchedule)->storage = false;                     // default value.
    RBRInstrumentGen4_setSchedule(instrument, *newSchedule); // warning: read err!!!
    return RBRINSTRUMENTGEN4_SUCCESS;
}
// can be static
RBRInstrumentGen4Error RBRInstrumentGen4_populateScheduleContinuous(
    RBRInstrumentGen4Schedule *targetSchedule,
    RBRInstrumentGen4Period period,
    bool castDetection)
{
    targetSchedule->modeDependentParameters.continuous.period = period;
    targetSchedule->modeDependentParameters.continuous.castDetection = castDetection;
    return RBRINSTRUMENTGEN4_SUCCESS;
}
// can be static
RBRInstrumentGen4Error RBRInstrumentGen4_populateScheduleRegimes(
    RBRInstrumentGen4Schedule *targetSchedule,
    RBRInstrumentGen4Regimes regimes)
{

    targetSchedule->modeDependentParameters.regimes.direction = regimes.direction;
    targetSchedule->modeDependentParameters.regimes.count = regimes.count;
    targetSchedule->modeDependentParameters.regimes.reference = regimes.reference;
    int count = regimes.count;
    if ((count < 1) || (count > 3))
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }
    else
    {
        // if the regimes provided more than what count specified, populate according to count.
        // If less, already returned error in above case.

        // code below: if count==1, setup only regime1. etc.
        // sacrifised some code readability.
        targetSchedule->modeDependentParameters.regimes.boundary1 = regimes.boundary1;
        targetSchedule->modeDependentParameters.regimes.binSize1 = regimes.binSize1;
        targetSchedule->modeDependentParameters.regimes.period1 = regimes.period1;

        if ((count == 2) || (count == 3))
        {
            targetSchedule->modeDependentParameters.regimes.boundary2 = regimes.boundary2;
            targetSchedule->modeDependentParameters.regimes.binSize2 = regimes.binSize2;
            targetSchedule->modeDependentParameters.regimes.period2 = regimes.period2;
        }

        if (count == 3)
        {
            targetSchedule->modeDependentParameters.regimes.boundary3 = regimes.boundary3;
            targetSchedule->modeDependentParameters.regimes.binSize3 = regimes.binSize3;
            targetSchedule->modeDependentParameters.regimes.period3 = regimes.period3;
        }
    }
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_initNewScheduleRegimes(
    RBRInstrumentGen4 *instrument,
    const char newScheduleLabel[],
    const char specifiedGroupLabels[][RBRINSTRUMENTGEN4_LABEL_NAME_MAX],
    int32_t specifiedGroupLabelCnt,
    RBRInstrumentGen4SamplingMode mode,
    RBRInstrumentGen4Regimes regimes,
    RBRInstrumentGen4GroupPool *groupPool,
    RBRInstrumentGen4SchedulePool *schedulePool,
    RBRInstrumentGen4Schedule **newSchedule)
{
    RBRInstrumentGen4_initNewSchedule(instrument,
                           newScheduleLabel,
                           specifiedGroupLabels,
                           specifiedGroupLabelCnt,
                           mode,
                           groupPool,
                           schedulePool,
                           newSchedule);
    RBRInstrumentGen4_populateScheduleRegimes(*newSchedule, regimes);
    RBRInstrumentGen4_setSchedule(instrument, *newSchedule); // warning: read err!!!
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_initNewScheduleContinuous(
    RBRInstrumentGen4 *instrument,
    const char newScheduleLabel[],
    const char specifiedGroupLabels[][RBRINSTRUMENTGEN4_LABEL_NAME_MAX],
    int32_t specifiedGroupLabelCnt,
    RBRInstrumentGen4SamplingMode mode,
    RBRInstrumentGen4Period period,
    bool castDetection,
    RBRInstrumentGen4GroupPool *groupPool,
    RBRInstrumentGen4SchedulePool *schedulePool,
    RBRInstrumentGen4Schedule **newSchedule)
{
    RBRInstrumentGen4_initNewSchedule(instrument,
                           newScheduleLabel,
                           specifiedGroupLabels,
                           specifiedGroupLabelCnt,
                           mode,
                           groupPool,
                           schedulePool,
                           newSchedule);
    RBRInstrumentGen4_populateScheduleContinuous(*newSchedule, period, castDetection);
    RBRInstrumentGen4_setSchedule(instrument, *newSchedule); // warning: read err!!!
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_initNewConfig(
    RBRInstrumentGen4 *instrument,
    const char newConfigLabel[],
    const char specifiedScheduleLabels[][RBRINSTRUMENTGEN4_LABEL_NAME_MAX],
    int32_t specifiedScheduleLabelCnt,
    RBRInstrumentGen4SchedulePool *schedulePool,
    RBRInstrumentGen4ConfigPool *configPool,
    RBRInstrumentGen4Config **newConfig)
{
    RBRInstrumentGen4_createConfig(instrument, newConfigLabel, configPool, newConfig); // warning: read err!!!
    RBRInstrumentGen4_populateConfigSchedules(*newConfig, schedulePool, specifiedScheduleLabels, specifiedScheduleLabelCnt); // warning: read err!!!
    RBRInstrumentGen4_setConfig(instrument, *newConfig); // warning: read err!!!
    return RBRINSTRUMENTGEN4_SUCCESS;
}
//-------------------------------------------------------------------------------
