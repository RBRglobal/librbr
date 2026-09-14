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

#include "../../include/RBRGen4.h"
#include "../../include/RBRGen4Commands.h"

#define INSTRUMENT_CHARACTER_TIMEOUT_MSEC 4000
#define INSTRUMENT_COMMAND_TIMEOUT_MSEC 10000

    int openSerialFd(char *devicePath);

    /**
     * \brief Callback to get the current time.
     * \see RBRGen4Callbacks
     * \see RBRGen4TimeCallback
     */
    RBRGen4Error instrumentTime(const struct RBRGen4 *instrument,
                                          RBRGen4DateTime *time);

    /**
     * \brief Callback to run when the instrument goes to sleep.
     * \see RBRGen4Callbacks
     * \see RBRGen4TimeCallback
     */
    RBRGen4Error instrumentSleep(const struct RBRGen4 *instrument,
                                           RBRGen4DateTime time);

    /**
     * \brief Callback to read from the instrument.
     * \see RBRGen4Callbacks
     * \see RBRGen4ReadCallback
     */
    RBRGen4Error instrumentRead(const struct RBRGen4 *instrument,
                                          void *data,
                                          int32_t *size);

    /**
     * \brief Callback to write to the instrument.
     * \see RBRGen4Callbacks
     * \see RBRGen4WriteCallback
     */
    RBRGen4Error instrumentWrite(const struct RBRGen4 *instrument,
                                           const void *const data,
                                           int32_t size);

    /**
     * \brief Parse a schedule-format time string ("YYYYMMDDhhmmss", UTC)
     * into a millisecond Unix timestamp.
     *
     * On invalid input, prints an error and sets \a targetDatetime to 0.
     */
    void str_to_deploymentDatetime(RBRGen4DateTime *targetDatetime,
                                   const char *sourceDatetime);

    /**
     * \brief Copy an array of pointers to channel structs.
     * Can be static.
     * \param target destination array
     * \param source source array
     * \param count the number of elements to copy
     */

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
     * \brief Fill \a channelList with the \a channelPool channels whose labels
     *        match \a specifiedChannelLabels.
     * \param channelList destination list
     * \param channelPool pool of channels to match to \a specifiedChannelLabels
     * \param specifiedChannelLabels array of labels to match to \a channelPool
     * \param specifiedChannelLabelCnt number of labels in \a specifiedChannelLabels
     * \return RBRGEN4_SUCCESS when all labels are found and the target is set
     * \return RBRGEN4_INVALID_PARAMETER_VALUE if any label is not found
     * \return RBRGEN4_BUFFER_TOO_SMALL if the target is too small for the requested number of children
     * \return RBRGEN4_TRUNCATED if \a channelPool was too small to
     *         hold every channel
     */
    RBRGen4Error RBRInstrumentGen4_populateGroupChannels(
        RBRGen4LabelList *channelList,
        RBRInstrumentGen4ChannelPool *channelPool,
        const RBRGen4Label specifiedChannelLabels[],
        int32_t specifiedChannelLabelCnt);

    /**
     * \brief Fill \a groupList with \a specifiedGroupLabels.
     * \param groupList destination list
     * \param specifiedGroupLabels array of labels to copy
     * \param specifiedGroupLabelCnt number of labels in \a specifiedGroupLabels
     * \return RBRGEN4_SUCCESS when the labels are copied
     * \return RBRGEN4_BUFFER_TOO_SMALL if the list cannot hold the
     *         labels
     */
    RBRGen4Error RBRInstrumentGen4_populateScheduleGroups(
        RBRGen4LabelList *groupList,
        const RBRGen4Label specifiedGroupLabels[],
        int32_t specifiedGroupLabelCnt);

    /**
     * \brief Fill \a scheduleList with \a specifiedScheduleLabels.
     * \param scheduleList destination list
     * \param specifiedScheduleLabels array of labels to copy
     * \param specifiedScheduleLabelCnt number of labels in \a specifiedScheduleLabels
     * \return RBRGEN4_SUCCESS when the labels are copied
     * \return RBRGEN4_BUFFER_TOO_SMALL if the list cannot hold the
     *         labels
     */
    RBRGen4Error RBRInstrumentGen4_populateConfigSchedules(
        RBRGen4LabelList *scheduleList,
        const RBRGen4Label specifiedScheduleLabels[],
        int32_t specifiedScheduleLabelCnt);

    /**
     * \brief Set target to the first struct in the pool with a matching label.
     * \param targetDataset target
     * \param datasetPool pool to search in
     * \param datasetLabel label to search for
     * \return RBRGEN4_SUCCESS when the label is found and the target is set
     * \return RBRGEN4_INVALID_PARAMETER_VALUE if the label is not found
     * \return RBRGEN4_TRUNCATED if \a datasetPool was too small
     *         to hold every dataset
     */
    RBRGen4Error RBRInstrumentGen4_getDatasetFromPool(
        RBRInstrumentGen4Dataset **targetDataset,
        RBRInstrumentGen4DatasetPool *datasetPool,
        const char datasetLabel[]);

    /**
     * \brief Set target to the first struct in the pool with a matching label.
     * \param targetDataset target
     * \param datasetPool pool to search in
     * \param datasetLabel label to search for
     * \return RBRGEN4_SUCCESS when the label is found and the target is set
     * \return RBRGEN4_INVALID_PARAMETER_VALUE if the label is not found
     */
    RBRGen4Error RBRInstrumentGen4_getChannelFromPool(
        RBRInstrumentGen4Channel **targetChannel,
        RBRInstrumentGen4ChannelPool *channelPool,
        const char channelLabel[]);

    /**
     * \brief Set target to the first struct in the pool with a matching label.
     * \param targetGroup target
     * \param groupPool pool to search in
     * \param groupLabel label to search for
     * \return RBRGEN4_SUCCESS when the label is found and the target is set
     * \return RBRGEN4_INVALID_PARAMETER_VALUE if the label is not found
     */
    RBRGen4Error RBRInstrumentGen4_getGroupFromPool(
        RBRInstrumentGen4Group **targetGroup,
        RBRInstrumentGen4GroupPool *groupPool,
        const char groupLabel[]);

    /**
     * \brief Set target to the first struct in the pool with a matching label.
     * \param targetSchedule target
     * \param schedulePool pool to search in
     * \param scheduleLabel label to search for
     * \return RBRGEN4_SUCCESS when the label is found and the target is set
     * \return RBRGEN4_INVALID_PARAMETER_VALUE if the label is not found
     */
    RBRGen4Error RBRInstrumentGen4_getScheduleFromPool(
        RBRInstrumentGen4Schedule **targetSchedule,
        RBRInstrumentGen4SchedulePool *schedulePool,
        const char scheduleLabel[]);

    /**
     * \brief Set target to the first struct in the pool with a matching label.
     * \param targetConfig target
     * \param configPool pool to search in
     * \param configLabel label to search for
     */
    RBRGen4Error RBRInstrumentGen4_getConfigFromPool(
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
     * \param [out] newGroup the new group
     */
    RBRGen4Error RBRInstrumentGen4_initNewGroup(
        RBRGen4 *instrument,
        const char newGroupLabel[],
        const RBRGen4Label specifiedChannelLabels[],
        int32_t specifiedChannelLabelCnt,
        RBRInstrumentGen4ChannelPool *channelPool,
        RBRInstrumentGen4Group *newGroup);

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
     * \return #RBRGEN4_SUCCESS when the parent is successfully created and populated
     * \return #RBRGEN4_TIMEOUT when a timeout occurs
     * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
     * \return #RBRGEN4_HARDWARE_ERROR if the parent cannot be created or populated
     * \see RBRInstrumentGen4_create<Parent>()
     * \see RBRInstrumentGen4_set<Parent>()
     */
    RBRGen4Error RBRInstrumentGen4_initNewSchedule(
        RBRGen4 *instrument,
        const char newScheduleLabel[],
        const RBRGen4Label specifiedGroupLabels[],
        int32_t specifiedGroupLabelCnt,
        RBRInstrumentGen4ScheduleMode mode,
        RBRGen4LabelList *groupList,
        RBRInstrumentGen4Schedule *newSchedule);

    /**
     * \brief Configure \a targetSchedule to sample continuously.
     * \param [inout] targetSchedule the target schedule
     * \param [in] period the sample period in milliseconds
     * \param [in] castDetection enable cast detection when true
     * \return #RBRGEN4_SUCCESS when the schedule is successfully configured
     * \return #RBRGEN4_TIMEOUT when a timeout occurs
     * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
     * \return #RBRGEN4_HARDWARE_ERROR if the schedule cannot be configured
     * \see RBRInstrumentGen4_setSchedule()
     */
    RBRGen4Error RBRInstrumentGen4_populateScheduleContinuous(
        RBRInstrumentGen4Schedule *targetSchedule,
        RBRGen4Period period,
        bool castDetection);

    /**
     * \brief Create and populate a new schedule configured for continous sampling.
     * \note Issues the `schedule create` and `schedule <schedule_label>` instrument commands.
     * \param [in] instrument the instrument connection
     * \param [in] newScheduleLabel the label to give the parent
     * \param [in] specifiedGroupLabels the labels of the children to give the parent
     * \param [in] specifiedGroupLabelCnt the number of children to give the parent
     * \param [in] period the sample period in milliseconds
     * \param [in] castDetection enable cast detection when true
     * \param [out] groupList the groups given to the parent
     * \param [out] newSchedule the new parent
     * \return #RBRGEN4_SUCCESS when the parent is successfully created and populated
     * \return #RBRGEN4_TIMEOUT when a timeout occurs
     * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
     * \return #RBRGEN4_HARDWARE_ERROR if the parent cannot be created or populated
     * \see RBRInstrumentGen4_createSchedule()
     * \see RBRInstrumentGen4_setSchedule()
     */
    RBRGen4Error RBRInstrumentGen4_initNewScheduleContinuous(
        RBRGen4 *instrument,
        const char newScheduleLabel[],
        const RBRGen4Label specifiedGroupLabels[],
        int32_t specifiedGroupLabelCnt,
        RBRInstrumentGen4ScheduleMode mode,
        RBRGen4Period period,
        bool castDetection,
        RBRGen4LabelList *groupList,
        RBRInstrumentGen4Schedule *newSchedule);

    /**
     * \brief Create and populate a new parent.
     * \note Issues the `config create` and `config <config_label>` instrument commands.
     * \param [in] instrument the instrument connection
     * \param [in] newConfigLabel the label to give the config
     * \param [in] specifiedScheduleLabels the labels of the schedules to give the parent
     * \param [in] specifiedScheduleLabelCnt the number of children to give the parent
     * \param [out] scheduleList the schedules given to the parent
     * \param [out] newConfig the new parent
     * \return #RBRGEN4_SUCCESS when the parent is successfully created and populated
     * \return #RBRGEN4_TIMEOUT when a timeout occurs
     * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
     * \return #RBRGEN4_HARDWARE_ERROR if the parent cannot be created or populated
     * \see RBRInstrumentGen4_createConfig()
     * \see RBRInstrumentGen4_setConfig()
     */
    RBRGen4Error RBRInstrumentGen4_initNewConfig(
        RBRGen4 *instrument,
        const char newConfigLabel[],
        const RBRGen4Label specifiedScheduleLabels[],
        int32_t specifiedScheduleLabelCnt,
        RBRGen4LabelList *scheduleList,
        RBRInstrumentGen4Config *newConfig);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_POSIX_SHARED_H */
