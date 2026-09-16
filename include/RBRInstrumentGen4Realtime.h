/**
 * \file RBRInstrumentGen4Realtime.h
 *
 * \brief Instrument commands and structures pertaining to on-demand data
 * acquisition.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/data-sample
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTGEN4REALTIME_H
#define LIBRBR_RBRINSTRUMENTGEN4REALTIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Streaming.h"

/**
 * \brief Requests an “on-demand” sample of every channel from the
 * instrument.
 *
 * Sends a bare `poll` command.
 *
 * Unlike streaming data/RBRInstrumentGen4_readSample(), polled data is
 * returned directly to the caller (independent of any
 * RBRInstrumentGen4SampleCallback defined in
 * RBRInstrumentGen4Callbacks.sample).
 *
 * With \a requireLabel set, only a sample labelled `polling` is returned;
 * any streamed samples read while waiting for it are passed to
 * RBRInstrumentGen4Callbacks.sample instead. With \a requireLabel unset, the
 * first sample read is returned, which may be a streamed sample, not a
 * polled sample, if the instrument is streaming over this link.
 *
 * \param [in] instrument the instrument connection
 * \param [in] requireLabel whether to require and wait for a sample
 *                          labelled `polling`
 * \param [out] sample the polled sample
 * \return #RBRINSTRUMENTGEN4_SUCCESS when a sample is successfully read
 * \return #RBRINSTRUMENTGEN4_UNSUPPORTED when \a requireLabel is set but
 *         instrument.outputformat.scheduleLabel is false
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs, or when no
 *         polled sample arrives within RBRInstrumentGen4.pollTimeout
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/data-sample/poll
 */
RBRInstrumentGen4Error RBRInstrumentGen4_poll(
    RBRInstrumentGen4 *instrument,
    bool requireLabel,
    RBRInstrumentGen4Sample *sample);

/**
 * \brief Requests an “on-demand” sample of the given channels from the
 * instrument.
 *
 * Sends the `poll channellist=` command. \a channelList is sent verbatim as
 * the parameter value; see the command documentation for the list format.
 *
 * Unlike streaming data/RBRInstrumentGen4_readSample(), polled data is
 * returned directly to the caller (independent of any
 * RBRInstrumentGen4SampleCallback defined in
 * RBRInstrumentGen4Callbacks.sample).
 *
 * With \a requireLabel set, only a sample labelled `polling` is returned;
 * any streamed samples read while waiting for it are passed to
 * RBRInstrumentGen4Callbacks.sample instead. With \a requireLabel unset, the
 * first sample read is returned, which may be a streamed sample, not a
 * polled sample, if the instrument is streaming over this link.
 *
 * \param [in] instrument the instrument connection
 * \param [in] requireLabel whether to require and wait for a sample
 *                          labelled `polling`
 * \param [in] channelList the channels to sample
 * \param [out] sample the polled sample
 * \return #RBRINSTRUMENTGEN4_SUCCESS when a sample is successfully read
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the channel list
 *         is too long to send
 * \return #RBRINSTRUMENTGEN4_UNSUPPORTED when \a requireLabel is set but
 *         instrument.outputformat.scheduleLabel is false
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs, or when no
 *         polled sample arrives within RBRInstrumentGen4.pollTimeout
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when an invalid channel is
 *         requested
 * \see https://docs.rbr-global.com/L3commandreference/commands/data-sample/poll
 */
RBRInstrumentGen4Error RBRInstrumentGen4_pollChannels(
    RBRInstrumentGen4 *instrument,
    bool requireLabel,
    const char *channelList,
    RBRInstrumentGen4Sample *sample);

/**
 * \brief Requests an “on-demand” sample of the given groups of channels from
 * the instrument.
 *
 * Sends the `poll grouplist=` command. \a groupList is sent verbatim as the
 * parameter value; see the command documentation for the list format.
 *
 * Unlike streaming data/RBRInstrumentGen4_readSample(), polled data is
 * returned directly to the caller (independent of any
 * RBRInstrumentGen4SampleCallback defined in
 * RBRInstrumentGen4Callbacks.sample).
 *
 * With \a requireLabel set, only a sample labelled `polling` is returned;
 * any streamed samples read while waiting for it are passed to
 * RBRInstrumentGen4Callbacks.sample instead. With \a requireLabel unset, the
 * first sample read is returned, which may be a streamed sample, not a
 * polled sample, if the instrument is streaming over this link.
 *
 * \param [in] instrument the instrument connection
 * \param [in] requireLabel whether to require and wait for a sample
 *                          labelled `polling`
 * \param [in] groupList the groups of channels to sample
 * \param [out] sample the polled sample
 * \return #RBRINSTRUMENTGEN4_SUCCESS when a sample is successfully read
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the group list is
 *         too long to send
 * \return #RBRINSTRUMENTGEN4_UNSUPPORTED when \a requireLabel is set but
 *         instrument.outputformat.scheduleLabel is false
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs, or when no
 *         polled sample arrives within RBRInstrumentGen4.pollTimeout
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when an invalid group is
 *         requested
 * \see https://docs.rbr-global.com/L3commandreference/commands/data-sample/poll
 */
RBRInstrumentGen4Error RBRInstrumentGen4_pollGroups(
    RBRInstrumentGen4 *instrument,
    bool requireLabel,
    const char *groupList,
    RBRInstrumentGen4Sample *sample);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTGEN4REALTIME_H */
