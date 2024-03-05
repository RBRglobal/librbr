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
// RBRInstrumentGen4DateTime_toScheduleTime() is available in L3 _setDeployment() function.
// however it lives in RBRInstrumentInternal.c
void str_to_deploymentDatetime(RBRInstrumentGen4DateTime *targetDatetime,
                               const char *sourceDatetime)
{
    int size = 15;                                   // RBRINSTRUMENTGEN4_SCHEDULE_TIME_LEN defined in internal.h as 14.
    const char *format = "%04d%02d%02d%02d%02d%02d"; // RBRInstrumentGen4DateTime_scheduleFormat. Also in internal.c
    time_t t = *targetDatetime / 1000;
    struct tm *split = gmtime(&t);
    int milliseconds = (int) (*targetDatetime % 1000);
    snprintf((char *) sourceDatetime,
             size,
             format,
             split->tm_year + 1900,
             split->tm_mon + 1,
             split->tm_mday,
             split->tm_hour,
             split->tm_min,
             split->tm_sec,
             milliseconds);
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

// can be static.
RBRInstrumentGen4Error match_objs_to_labels_forChannel(RBRInstrumentGen4Channellist *targetPtrList,
                                                       RBRInstrumentGen4Channels *sourceObjList,
                                                       RBRlabel* specifiedLabels, int specifiedLabelCount)
{
    // find how many labels are specified by specifiedLabels
    int _specifiedLabelCnt = sizeof(specifiedLabels) / sizeof(specifiedLabels[0]);

    // init pointer array pointing to sourceObjects
    RBRInstrumentGen4Channel *_newPtrList[_specifiedLabelCnt]; // warning: modify according to sourceObjList structure.
    _newPtrList[0] = NULL;

    // find out each label specified, and compare with all sourceObjList.
    // and fill the pointer array.
    int _currentIndex = 0;
    int _totalSourceObjCnt = (*sourceObjList).count;

    bool flag; // set flag if a specified label is found.
    for (int j = 0; j < _specifiedLabelCnt; j++)
    {
        flag = false;
        for (int i = 0; i < _totalSourceObjCnt; i++)
        {
            if (strcmp((*sourceObjList).channels[i].label, specifiedLabels[j]) == 0) // warning: modify according to sourceObjList structure.
            {
                _newPtrList[_currentIndex] = &((*sourceObjList).channels[i]); // warning: modify according to sourceObjList structure.
                _currentIndex++;
                flag = true;
                break;
            }
        }
        if (flag == false) // label not found
        {
            fprintf(stderr, "Warning: label '%s' specified does not exist!\n", specifiedLabels[j]);
        }
    }

    if (_newPtrList[0] == NULL) // none of the label specified is valid.
    {
        fprintf(stderr, "Error: none of the specified labels exist!\n");
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    // fill targetPtrList with count and pointer array.
    targetPtrList->count = _specifiedLabelCnt;
    cpy_ptrArray_forChannel(targetPtrList->channels, _newPtrList, _specifiedLabelCnt); // warning: modify according to sourceObjList structure.
    return RBRINSTRUMENTGEN4_SUCCESS;
}

// can be static.
RBRInstrumentGen4Error match_objs_to_labels_forGroup(RBRInstrumentGen4Grouplist *targetPtrList,
                                                     RBRInstrumentGen4Groups *sourceObjList,
                                                     const char specifiedLabels[][31])
{
    // find how many labels are specified by specifiedLabels
    int _specifiedLabelCnt = sizeof(specifiedLabels) / sizeof(specifiedLabels[0]);

    // init pointer array pointing to sourceObjects
    RBRInstrumentGen4Group *_newPtrList[_specifiedLabelCnt]; // warning: modify according to sourceObjList structure.
    _newPtrList[0] = NULL;

    // find out each label specified, and compare with all sourceObjList.
    // and fill the pointer array.
    int _currentIndex = 0;
    int _totalSourceObjCnt = (*sourceObjList).grouplist.count;

    bool flag; // set flag if a specified label is found.
    for (int j = 0; j < _specifiedLabelCnt; j++)
    {
        flag = false;
        for (int i = 0; i < _totalSourceObjCnt; i++)
        {
            if (strcmp((*sourceObjList).grouplist.groups[i]->label, specifiedLabels[j]) == 0) // warning: modify according to sourceObjList structure.
            {
                _newPtrList[_currentIndex] = (*sourceObjList).grouplist.groups[i]; // warning: modify according to sourceObjList structure.
                _currentIndex++;
                flag = true;
                break;
            }
        }
        if (flag == false) // label not found
        {
            fprintf(stderr, "Warning: label '%s' specified does not exist!\n", specifiedLabels[j]);
        }
    }

    if (_newPtrList[0] == NULL) // none of the label specified is valid.
    {
        fprintf(stderr, "Error: none of the specified labels exist!\n");
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    // fill targetPtrList with count and pointer array.
    targetPtrList->count = _specifiedLabelCnt;
    cpy_ptrArray_forGroup(targetPtrList->groups, _newPtrList, _specifiedLabelCnt); // warning: modify according to sourceObjList structure.
    return RBRINSTRUMENTGEN4_SUCCESS;
}

// can be static.
RBRInstrumentGen4Error match_objs_to_labels_forSchedule(RBRInstrumentGen4Schedulelist *targetPtrList,
                                                        RBRInstrumentGen4Schedules *sourceObjList,
                                                        const char specifiedLabels[][31])
{
    // find how many labels are specified by specifiedLabels
    int _specifiedLabelCnt = sizeof(specifiedLabels) / sizeof(specifiedLabels[0]);

    // init pointer array pointing to sourceObjects
    RBRInstrumentGen4Schedule *_newPtrList[_specifiedLabelCnt]; // warning: modify according to sourceObjList structure.
    _newPtrList[0] = NULL;

    // find out each label specified, and compare with all sourceObjList.
    // and fill the pointer array.
    int _currentIndex = 0;
    int _totalSourceObjCnt = (*sourceObjList).schedulelist.count;

    bool flag; // set flag if a specified label is found.
    for (int j = 0; j < _specifiedLabelCnt; j++)
    {
        flag = false;
        for (int i = 0; i < _totalSourceObjCnt; i++)
        {
            if (strcmp((*sourceObjList).schedulelist.schedules[i]->label, specifiedLabels[j]) == 0) // warning: modify according to sourceObjList structure.
            {
                _newPtrList[_currentIndex] = (*sourceObjList).schedulelist.schedules[i]; // warning: modify according to sourceObjList structure.
                _currentIndex++;
                flag = true;
                break;
            }
        }
        if (flag == false) // label not found
        {
            fprintf(stderr, "Warning: label '%s' specified does not exist!\n", specifiedLabels[j]);
        }
    }

    if (_newPtrList[0] == NULL) // none of the label specified is valid.
    {
        fprintf(stderr, "Error: none of the specified labels exist!\n");
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    // fill targetPtrList with count and pointer array.
    targetPtrList->count = _specifiedLabelCnt;
    cpy_ptrArray_forSchedule(targetPtrList->schedules, _newPtrList, _specifiedLabelCnt); // warning: modify according to sourceObjList structure.
    return RBRINSTRUMENTGEN4_SUCCESS;
}
// can be static
RBRInstrumentGen4Error match_dataset_to_label(RBRInstrumentGen4Dataset **targetDataset,
                                              RBRInstrumentGen4Datasets *datasets,
                                              const char datasetLabel[])
{
    int _totalSourceObjCnt = datasets->datasetlist.count;
    if (_totalSourceObjCnt <= 0)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }
    else
    {
        bool _flag = false;
        for (int i = 0; i < _totalSourceObjCnt; i++)
        {
            if (strcmp((*datasets).datasetlist.datasets[i]->label, datasetLabel) == 0) // warning: modify according to sourceObjList structure.
            {
                *targetDataset = ((*datasets).datasetlist.datasets[i]); // warning: modify according to sourceObjList structure.
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

RBRInstrumentGen4Error init_groupStructure(RBRInstrumentGen4 *instrument,
                                           const char newGroupLabel[],
                                           const char specifiedChannelLabels[][31],
                                           RBRInstrumentGen4Channels *channels,
                                           RBRInstrumentGen4Group *newGroup,
                                           RBRInstrumentGen4Groups *groups)
{
    strncpy(newGroup->label, newGroupLabel, sizeof(newGroup->label));
    match_objs_to_labels_forChannel(&(newGroup->channellist), channels, specifiedChannelLabels); // warning: read errs!
    RBRInstrumentGen4_createGroup(instrument, newGroup, groups);                                 // warning: read errs!
    return RBRINSTRUMENTGEN4_SUCCESS;
}
// can be static
RBRInstrumentGen4Error init_scheduleStructure(RBRInstrumentGen4 *instrument,
                                              const char newScheduleLabel[],
                                              const char specifiedGroupLabels[][31],
                                              RBRInstrumentGen4SamplingMode mode,
                                              RBRInstrumentGen4Groups *groups,
                                              RBRInstrumentGen4Schedule *newSchedule,
                                              RBRInstrumentGen4Schedules *schedules)
{
    // depending on mode, create a mode-dependent structure.
    // Warning: in this example, i'll just create a regimes structure. But this function should handle all other modes as well.
    strncpy(newSchedule->label, newScheduleLabel, sizeof(newSchedule->label));
    match_objs_to_labels_forGroup(newSchedule->grouplist, groups, specifiedGroupLabels); // warning: read err!!!
    newSchedule->mode = mode;
    newSchedule->stream = RBRINSTRUMENTGEN4_UNKNOWN_LINK;                 // default value.
    newSchedule->store = false;                                           // default value.
    RBRInstrumentGen4_createSchedule(instrument, newSchedule, schedules); // warning: read err!!!
    return RBRINSTRUMENTGEN4_SUCCESS;
}
// can be static
RBRInstrumentGen4Error populate_schedule_continuous(RBRInstrumentGen4Schedule *targetSchedule,
                                                    RBRInstrumentGen4Period period,
                                                    bool castdetection)
{
    targetSchedule->RBRInstrumentGen4ModeDependentParameters.continuous.period = period;
    targetSchedule->RBRInstrumentGen4ModeDependentParameters.continuous.castdetection = castdetection;
    return RBRINSTRUMENTGEN4_SUCCESS;
}
// can be static
RBRInstrumentGen4Error populate_schedule_regimes(RBRInstrumentGen4Schedule *targetSchedule,
                                                 RBRInstrumentGen4Regimes regimes)
{

    targetSchedule->RBRInstrumentGen4ModeDependentParameters.regimes.direction = regimes.direction;
    targetSchedule->RBRInstrumentGen4ModeDependentParameters.regimes.count = regimes.count;
    targetSchedule->RBRInstrumentGen4ModeDependentParameters.regimes.reference = regimes.reference;
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
        targetSchedule->RBRInstrumentGen4ModeDependentParameters.regimes.boundary1 = regimes.boundary1;
        targetSchedule->RBRInstrumentGen4ModeDependentParameters.regimes.binSize1 = regimes.binSize1;
        targetSchedule->RBRInstrumentGen4ModeDependentParameters.regimes.period1 = regimes.period1;

        if ((count == 2) || (count == 3))
        {
            targetSchedule->RBRInstrumentGen4ModeDependentParameters.regimes.boundary2 = regimes.boundary2;
            targetSchedule->RBRInstrumentGen4ModeDependentParameters.regimes.binSize2 = regimes.binSize2;
            targetSchedule->RBRInstrumentGen4ModeDependentParameters.regimes.period2 = regimes.period2;
        }

        if (count == 3)
        {
            targetSchedule->RBRInstrumentGen4ModeDependentParameters.regimes.boundary3 = regimes.boundary3;
            targetSchedule->RBRInstrumentGen4ModeDependentParameters.regimes.binSize3 = regimes.binSize3;
            targetSchedule->RBRInstrumentGen4ModeDependentParameters.regimes.period3 = regimes.period3;
        }
    }
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error init_schedule_regimes(RBRInstrumentGen4 *instrument,
                                             const char newScheduleLabel[],
                                             const char specifiedGroupLabels[][31],
                                             RBRInstrumentGen4SamplingMode mode,
                                             RBRInstrumentGen4Groups *groups,
                                             RBRInstrumentGen4Regimes regimes,
                                             RBRInstrumentGen4Schedule *newSchedule,
                                             RBRInstrumentGen4Schedules *schedules)
{
    init_scheduleStructure(instrument,
                           newScheduleLabel,
                           specifiedGroupLabels,
                           mode,
                           groups,
                           newSchedule,
                           schedules);
    populate_schedule_regimes(newSchedule, regimes);
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error init_schedule_continuous(RBRInstrumentGen4 *instrument,
                                                const char newScheduleLabel[],
                                                const char specifiedGroupLabels[][31],
                                                RBRInstrumentGen4SamplingMode mode,
                                                RBRInstrumentGen4Groups *groups,
                                                RBRInstrumentGen4Period period,
                                                bool castdetection,
                                                RBRInstrumentGen4Schedule *newSchedule,
                                                RBRInstrumentGen4Schedules *schedules)
{
    init_scheduleStructure(instrument,
                           newScheduleLabel,
                           specifiedGroupLabels,
                           mode,
                           groups,
                           newSchedule,
                           schedules);
    populate_schedule_continuous(newSchedule, period, castdetection);
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error init_configStructure(RBRInstrumentGen4 *instrument,
                                            const char configLabel[],
                                            const char scheduleLabels[][31],
                                            RBRInstrumentGen4Schedules *schedules,
                                            RBRInstrumentGen4Config *newConfig,
                                            RBRInstrumentGen4Configs *configs)
{
    strncpy(newConfig->label, configLabel, sizeof(newConfig->label));
    match_objs_to_labels_forSchedule(newConfig->schedulelist, schedules, scheduleLabels); // warning: read err!!!
    RBRInstrumentGen4_createConfig(instrument, newConfig, configs);                       // warning: read err!!!
    return RBRINSTRUMENTGEN4_SUCCESS;
}
//-------------------------------------------------------------------------------