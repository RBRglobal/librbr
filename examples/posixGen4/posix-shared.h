/**
 * \file posix-shared.h
 *
 * \brief Shared functions used by the libRBR POSIX examples.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_POSIX_SHARED_H
#define LIBRBR_POSIX_SHARED_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../include/RBRInstrumentGen4.h"

#define INSTRUMENT_CHARACTER_TIMEOUT_MSEC 4000
#define INSTRUMENT_COMMAND_TIMEOUT_MSEC 10000

    int openSerialFd(char *devicePath);

    RBRInstrumentGen4Error instrumentTime(const struct RBRInstrumentGen4 *instrument,
                                          RBRInstrumentGen4DateTime *time);

    RBRInstrumentGen4Error instrumentSleep(const struct RBRInstrumentGen4 *instrument,
                                           RBRInstrumentGen4DateTime time);

    RBRInstrumentGen4Error instrumentRead(const struct RBRInstrumentGen4 *instrument,
                                          void *data,
                                          int32_t *size);

    RBRInstrumentGen4Error instrumentWrite(const struct RBRInstrumentGen4 *instrument,
                                           const void *const data,
                                           int32_t size);

    // RBRInstrumentGen4DateTime_toScheduleTime() is available in L3 _setDeployment() function.
    // however it lives in RBRInstrumentInternal.c
    void str_to_deploymentDatetime(RBRInstrumentGen4DateTime *targetDatetime,
                                   const char *sourceDatetime);

    // can be static.
    void cpy_ptrArray_forChannel(RBRInstrumentGen4Channel *target[],
                                 RBRInstrumentGen4Channel *source[],
                                 int count);
    // can be static.
    void cpy_ptrArray_forGroup(RBRInstrumentGen4Group *target[],
                               RBRInstrumentGen4Group *source[],
                               int count);
    // can be static.
    void cpy_ptrArray_forSchedule(RBRInstrumentGen4Schedule *target[],
                                  RBRInstrumentGen4Schedule *source[],
                                  int count);
    // can be static.
    RBRInstrumentGen4Error match_objs_to_labels_forChannel(RBRInstrumentGen4Channellist *targetPtrList,
                                                           RBRInstrumentGen4Channels *sourceObjList,
                                                           const char specifiedLabels[][31]);
    // can be static.
    RBRInstrumentGen4Error match_objs_to_labels_forGroup(RBRInstrumentGen4Grouplist *targetPtrList,
                                                         RBRInstrumentGen4Groups *sourceObjList,
                                                         const char specifiedLabels[][31]);
    // can be static.
    RBRInstrumentGen4Error match_objs_to_labels_forSchedule(RBRInstrumentGen4Schedulelist *targetPtrList,
                                                            RBRInstrumentGen4Schedules *sourceObjList,
                                                            const char specifiedLabels[][31]);
    // can be static.
    RBRInstrumentGen4Error match_dataset_to_label(RBRInstrumentGen4Dataset **targetDataset,
                                                  RBRInstrumentGen4Datasets *datasets,
                                                  const char datasetLabel[]);

    RBRInstrumentGen4Error init_groupStructure(RBRInstrumentGen4 *instrument,
                                               const char newGroupLabel[],
                                               const char specifiedChannelLabels[][31],
                                               RBRInstrumentGen4Channels *channels,
                                               RBRInstrumentGen4Group *newGroup,
                                               RBRInstrumentGen4Groups *groups);
    // can be static.
    RBRInstrumentGen4Error init_scheduleStructure(RBRInstrumentGen4 *instrument,
                                                  const char newScheduleLabel[],
                                                  const char specifiedGroupLabels[][31],
                                                  RBRInstrumentGen4SamplingMode mode,
                                                  RBRInstrumentGen4Groups *groups,
                                                  RBRInstrumentGen4Schedule *newSchedule,
                                                  RBRInstrumentGen4Schedules *schedules);
    // can be static.
    RBRInstrumentGen4Error populate_schedule_continuous(RBRInstrumentGen4Schedule *targetSchedule,
                                                        RBRInstrumentGen4Period period,
                                                        bool castdetection);
    // can be static.
    RBRInstrumentGen4Error populate_schedule_regimes(RBRInstrumentGen4Schedule *targetSchedule,
                                                     RBRInstrumentGen4Regimes regimes);

    RBRInstrumentGen4Error init_schedule_regimes(RBRInstrumentGen4 *instrument,
                                                 const char newScheduleLabel[],
                                                 const char specifiedGroupLabels[][31],
                                                 RBRInstrumentGen4SamplingMode mode,
                                                 RBRInstrumentGen4Groups *groups,
                                                 RBRInstrumentGen4Regimes regimes,
                                                 RBRInstrumentGen4Schedule *newSchedule,
                                                 RBRInstrumentGen4Schedules *schedules);
    RBRInstrumentGen4Error init_schedule_continuous(RBRInstrumentGen4 *instrument,
                                                    const char newScheduleLabel[],
                                                    const char specifiedGroupLabels[][31],
                                                    RBRInstrumentGen4SamplingMode mode,
                                                    RBRInstrumentGen4Groups *groups,
                                                    RBRInstrumentGen4Period period,
                                                    bool castdetection,
                                                    RBRInstrumentGen4Schedule *newSchedule,
                                                    RBRInstrumentGen4Schedules *schedules);

    RBRInstrumentGen4Error init_configStructure(RBRInstrumentGen4 *instrument,
                                                const char configLabel[],
                                                const char scheduleLabels[][31],
                                                RBRInstrumentGen4Schedules *schedules,
                                                RBRInstrumentGen4Config *newConfig,
                                                RBRInstrumentGen4Configs *configs);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_POSIX_SHARED_H */
