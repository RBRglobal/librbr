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
#include "../../include/RBRInstrumentGen4Commands.h"

#define INSTRUMENT_CHARACTER_TIMEOUT_MSEC 4000
#define INSTRUMENT_COMMAND_TIMEOUT_MSEC 10000

    int openSerialFd(char *devicePath);

    /**
     * \brief Callback to get the current time.
     * \see RBRInstrumentGen4Callbacks
     * \see RBRInstrumentGen4TimeCallback
     */
    RBRInstrumentGen4Error instrumentTime(const struct RBRInstrumentGen4 *instrument,
                                          RBRInstrumentGen4DateTime *time);

    /**
     * \brief Callback to run when the instrument goes to sleep.
     * \see RBRInstrumentGen4Callbacks
     * \see RBRInstrumentGen4TimeCallback
     */
    RBRInstrumentGen4Error instrumentSleep(const struct RBRInstrumentGen4 *instrument,
                                           RBRInstrumentGen4DateTime time);

    /**
     * \brief Callback to read from the instrument.
     * \see RBRInstrumentGen4Callbacks
     * \see RBRInstrumentGen4ReadCallback
     */
    RBRInstrumentGen4Error instrumentRead(const struct RBRInstrumentGen4 *instrument,
                                          void *data,
                                          int32_t *size);

    /**
     * \brief Callback to write to the instrument.
     * \see RBRInstrumentGen4Callbacks
     * \see RBRInstrumentGen4WriteCallback
     */
    RBRInstrumentGen4Error instrumentWrite(const struct RBRInstrumentGen4 *instrument,
                                           const void *const data,
                                           int32_t size);

    // RBRInstrumentGen4DateTime_toScheduleTime() is available in L3 _setDeployment() function.
    // however it lives in RBRInstrumentInternal.c
    void str_to_deploymentDatetime(RBRInstrumentGen4DateTime *targetDatetime,
                                   const char *sourceDatetime);

    /**
     * \brief Copy an array of pointers to channel structs.
     * Can be static.
     * \param target destination array
     * \param source source array
     * \param count the number of elements to copy
     */
    void cpy_ptrArray_forChannel(RBRInstrumentGen4Channel *target[],
                                 RBRInstrumentGen4Channel *source[],
                                 int count);

    /**
     * \brief Copy an array of pointers to group structs.
     * Can be static.
     * \param target destination array
     * \param source source array
     * \param count the number of elements to copy
     */
    void cpy_ptrArray_forGroup(RBRInstrumentGen4Group *target[],
                               RBRInstrumentGen4Group *source[],
                               int count);

    /**
     * \brief Copy an array of pointers to schedule structs.
     * Can be static.
     * \param target destination array
     * \param source source array
     * \param count the number of elements to copy
     */
    void cpy_ptrArray_forSchedule(RBRInstrumentGen4Schedule *target[],
                                  RBRInstrumentGen4Schedule *source[],
                                  int count);

    /**
     * \brief Set \a group pointers to \a channelPool channels with labels
     *        that match \a specifiedChannelLabels.
     * \param group destination group
     * \param channelPool pool of channels to match to \a specifiedChannelLabels
     * \param specifiedChannelLabels array of labels to match to \a channelPool
     * \param specifiedChannelLabelCnt number of labels in \a specifiedChannelLabels
     * \return RBRINSTRUMENTGEN4_SUCCESS when all labels are found and the target is set
     * \return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE if any label is not found
     * \return RBRINSTRUMENTGEN4_BUFFER_TOO_SMALL if the target is too small for the requested number of children
     */
    RBRInstrumentGen4Error RBRInstrumentGen4_populateGroupChannels(
        RBRInstrumentGen4Group *group,
        RBRInstrumentGen4ChannelPool *channelPool,
        const char specifiedChannelLabels[][RBRINSTRUMENTGEN4_CHANNEL_LABEL_MAX],
        int32_t specifiedChannelLabelCnt);

    /**
     * \brief Set \a schedule pointers to \a groupPool groups with labels
     *        that match \a specifiedGroupLabels.
     * \param schedule destination schedule
     * \param groupPool pool of groups to match to \a specifiedGroupLabels
     * \param specifiedGroupLabels array of labels to match to \a groupPool
     * \param specifiedGroupLabelCnt number of labels in \a specifiedGroupLabels
     * \return RBRINSTRUMENTGEN4_SUCCESS when all labels are found and the target is set
     * \return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE if any label is not found
     * \return RBRINSTRUMENTGEN4_BUFFER_TOO_SMALL if the target is too small for the requested number of children
     */
    RBRInstrumentGen4Error RBRInstrumentGen4_populateScheduleGroups(
        RBRInstrumentGen4Schedule *schedule,
        RBRInstrumentGen4GroupPool *groupPool,
        const char specifiedGroupLabels[][RBRINSTRUMENTGEN4_LABEL_NAME_MAX],
        int32_t specifiedGroupLabelCnt);

    /**
     * \brief Set \a config pointers to \a schedulePool schedules with labels
     *        that match \a specifiedScheduleLabels.
     * \param config destination config
     * \param schedulePool pool of schedules to match to \a specifiedScheduleLabels
     * \param specifiedScheduleLabels array of labels to match to \a schedulePool
     * \param specifiedScheduleLabelCnt number of labels in \a specifiedScheduleLabels
     * \return RBRINSTRUMENTGEN4_SUCCESS when all labels are found and the target is set
     * \return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE if any label is not found
     * \return RBRINSTRUMENTGEN4_BUFFER_TOO_SMALL if the target is too small for the requested number of children
     */
    RBRInstrumentGen4Error RBRInstrumentGen4_populateConfigSchedules(
        RBRInstrumentGen4Config *config,
        RBRInstrumentGen4SchedulePool *schedulePool,
        const char specifiedScheduleLabels[][RBRINSTRUMENTGEN4_LABEL_NAME_MAX],
        int32_t specifiedScheduleLabelCnt);

    /**
     * \brief Set target to the first struct in the pool with a matching label.
     * \param targetDataset target
     * \param datasetPool pool to search in
     * \param datasetLabel label to search for
     * \return RBRINSTRUMENTGEN4_SUCCESS when the label is found and the target is set
     * \return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE if the label is not found
     */
    RBRInstrumentGen4Error RBRInstrumentGen4_getDatasetFromPool(
        RBRInstrumentGen4Dataset **targetDataset,
        RBRInstrumentGen4DatasetPool *datasetPool,
        const char datasetLabel[]);

    /**
     * \brief Set target to the first struct in the pool with a matching label.
     * \param targetDataset target
     * \param datasetPool pool to search in
     * \param datasetLabel label to search for
     * \return RBRINSTRUMENTGEN4_SUCCESS when the label is found and the target is set
     * \return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE if the label is not found
     */
    RBRInstrumentGen4Error RBRInstrumentGen4_getChannelFromPool(
        RBRInstrumentGen4Channel **targetChannel,
        RBRInstrumentGen4ChannelPool *channelPool,
        const char channelLabel[]);

    /**
     * \brief Set target to the first struct in the pool with a matching label.
     * \param targetGroup target
     * \param groupPool pool to search in
     * \param groupLabel label to search for
     * \return RBRINSTRUMENTGEN4_SUCCESS when the label is found and the target is set
     * \return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE if the label is not found
     */
    RBRInstrumentGen4Error RBRInstrumentGen4_getGroupFromPool(
        RBRInstrumentGen4Group **targetGroup,
        RBRInstrumentGen4GroupPool *groupPool,
        const char groupLabel[]);

    /**
     * \brief Set target to the first struct in the pool with a matching label.
     * \param targetSchedule target
     * \param schedulePool pool to search in
     * \param scheduleLabel label to search for
     * \return RBRINSTRUMENTGEN4_SUCCESS when the label is found and the target is set
     * \return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE if the label is not found
     */
    RBRInstrumentGen4Error RBRInstrumentGen4_getScheduleFromPool(
        RBRInstrumentGen4Schedule **targetSchedule,
        RBRInstrumentGen4SchedulePool *schedulePool,
        const char scheduleLabel[]);

    /**
     * \brief Set target to the first struct in the pool with a matching label.
     * \param targetConfig target
     * \param configPool pool to search in
     * \param configLabel label to search for
     */
    RBRInstrumentGen4Error RBRInstrumentGen4_getConfigFromPool(
        RBRInstrumentGen4Config **targetConfig,
        RBRInstrumentGen4ConfigPool *configPool,
        const char configLabel[]);

    /**
     * \brief Create and populate a new group.
     * \note Issues the `group create` and `group <group_label>` instrument commands.
     * \param [in] instrument the instrument connection
     * \param [in] newGroupLabel the label to give the group
     * \param [in] specifiedChannelLabels the labels of the channels to include in the group
     * \param [in] specifiedChannelLabelCnt the number of channels to include in the group
     * \param [in] channelPool the pool to search in
     * \param [inout] groupPool the pool to add to
     * \param [out] newGroup the new group
     */
    RBRInstrumentGen4Error RBRInstrumentGen4_initNewGroup(
        RBRInstrumentGen4 *instrument,
        const char newGroupLabel[],
        const char specifiedChannelLabels[][RBRINSTRUMENTGEN4_CHANNEL_LABEL_MAX],
        int32_t specifiedChannelLabelCnt,
        RBRInstrumentGen4ChannelPool *channelPool,
        RBRInstrumentGen4GroupPool *groupPool,
        RBRInstrumentGen4Group **newGroup);

    /**
     * \brief Create and populate a new parent.
     * \note Issues the `<parent> create` and `<parent> <<parent>_label>` instrument commands.
     * \param [in] instrument the instrument connection
     * \param [in] newGroupLabel the label to give the parent
     * \param [in] specifiedChannelLabels the labels of the children to give the parent
     * \param [in] specifiedChannelLabelCnt the number of children to give the parent
     * \param [in] channelPool the pool to search for children in
     * \param [inout] groupPool the pool to add the parent to
     * \param [out] newGroup the new parent
     * \return #RBRINSTRUMENTGEN4_SUCCESS when the parent is successfully created and populated
     * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
     * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
     * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the parent cannot be created or populated
     * \see RBRInstrumentGen4_create<Parent>()
     * \see RBRInstrumentGen4_set<Parent>()
     */
    RBRInstrumentGen4Error RBRInstrumentGen4_initNewSchedule(
        RBRInstrumentGen4 *instrument,
        const char newScheduleLabel[],
        const char specifiedGroupLabels[][RBRINSTRUMENTGEN4_LABEL_NAME_MAX],
        int32_t specifiedGroupLabelCnt,
        RBRInstrumentGen4SamplingMode mode,
        RBRInstrumentGen4GroupPool *groupPool,
        RBRInstrumentGen4SchedulePool *schedulePool,
        RBRInstrumentGen4Schedule **newSchedule);

    /**
     * \brief Configure \a targetSchedule to sample continuously.
     * \param [inout] targetSchedule the target schedule
     * \param [in] period the sample period in milliseconds
     * \param [in] castDetection enable cast detection when true
     * \return #RBRINSTRUMENTGEN4_SUCCESS when the schedule is successfully configured
     * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
     * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
     * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the schedule cannot be configured
     * \see RBRInstrumentGen4_setSchedule()
     */
    RBRInstrumentGen4Error RBRInstrumentGen4_populateScheduleContinuous(
        RBRInstrumentGen4Schedule *targetSchedule,
        RBRInstrumentGen4Period period,
        bool castDetection);

    /**
     * \brief Configure \a targetSchedule to sample by pressure regimes.
     * \param [inout] targetSchedule the target schedule
     * \param [in] regimes the regimes to configure the schedule with
     * \return #RBRINSTRUMENTGEN4_SUCCESS when the schedule is successfully configured
     * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
     * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
     * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the schedule cannot be configured
     * \see RBRInstrumentGen4_setSchedule()
     */
    RBRInstrumentGen4Error RBRInstrumentGen4_populateScheduleRegimes(
        RBRInstrumentGen4Schedule *targetSchedule,
        RBRInstrumentGen4Regimes regimes);

    /**
     * \brief Create and populate a new schedule configured for regime sampling.
     * \note Issues the `schedule create` and `schedule <schedule_label>` instrument commands.
     * \param [in] instrument the instrument connection
     * \param [in] newScheduleLabel the label to give the parent
     * \param [in] specifiedGroupLabels the labels of the children to give the parent
     * \param [in] specifiedGroupLabelCnt the number of children to give the parent
     * \param [in] groupPool the pool to search for children in
     * \param [in] regimes the regimes to configure the schedule with
     * \param [inout] groupPool the pool to add the parent to
     * \param [out] newSchedule the new parent
     * \return #RBRINSTRUMENTGEN4_SUCCESS when the parent is successfully created and populated
     * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
     * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
     * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the parent cannot be created or populated
     * \see RBRInstrumentGen4_createSchedule()
     * \see RBRInstrumentGen4_setSchedule()
     */
    RBRInstrumentGen4Error RBRInstrumentGen4_initNewScheduleRegimes(
        RBRInstrumentGen4 *instrument,
        const char newScheduleLabel[],
        const char specifiedGroupLabels[][RBRINSTRUMENTGEN4_LABEL_NAME_MAX],
        int32_t specifiedGroupLabelCnt,
        RBRInstrumentGen4SamplingMode mode,
        RBRInstrumentGen4Regimes regimes,
        RBRInstrumentGen4GroupPool *groupPool,
        RBRInstrumentGen4SchedulePool *schedulePool,
        RBRInstrumentGen4Schedule **newSchedule);

    /**
     * \brief Create and populate a new schedule configured for continous sampling.
     * \note Issues the `schedule create` and `schedule <schedule_label>` instrument commands.
     * \param [in] instrument the instrument connection
     * \param [in] newScheduleLabel the label to give the parent
     * \param [in] specifiedGroupLabels the labels of the children to give the parent
     * \param [in] specifiedGroupLabelCnt the number of children to give the parent
     * \param [in] groupPool the pool to search for children in
     * \param [in] period the sample period in milliseconds
     * \param [in] castDetection enable cast detection when true
     * \param [inout] groupPool the pool to add the parent to
     * \param [out] newSchedule the new parent
     * \return #RBRINSTRUMENTGEN4_SUCCESS when the parent is successfully created and populated
     * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
     * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
     * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the parent cannot be created or populated
     * \see RBRInstrumentGen4_createSchedule()
     * \see RBRInstrumentGen4_setSchedule()
     */
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
        RBRInstrumentGen4Schedule **newSchedule);

    /**
     * \brief Create and populate a new parent.
     * \note Issues the `config create` and `config <config_label>` instrument commands.
     * \param [in] instrument the instrument connection
     * \param [in] newConfigLabel the label to give the config
     * \param [in] specifiedScheduleLabels the labels of the schedules to give the parent
     * \param [in] specifiedScheduleLabelCnt the number of children to give the parent
     * \param [in] schedulePool the pool to search for children in
     * \param [inout] configPool the pool to add the parent to
     * \param [out] newConfig the new parent
     * \return #RBRINSTRUMENTGEN4_SUCCESS when the parent is successfully created and populated
     * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
     * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
     * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the parent cannot be created or populated
     * \see RBRInstrumentGen4_createConfig()
     * \see RBRInstrumentGen4_setConfig()
     */
    RBRInstrumentGen4Error RBRInstrumentGen4_initNewConfig(
        RBRInstrumentGen4 *instrument,
        const char newConfigLabel[],
        const char specifiedScheduleLabels[][RBRINSTRUMENTGEN4_LABEL_NAME_MAX],
        int32_t specifiedScheduleLabelCnt,
        RBRInstrumentGen4SchedulePool *schedulePool,
        RBRInstrumentGen4ConfigPool *configPool,
        RBRInstrumentGen4Config **newConfig);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_POSIX_SHARED_H */
