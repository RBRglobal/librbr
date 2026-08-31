/**
 * \file RBRInstrumentGen4Polling.h
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

#ifndef LIBRBR_RBRINSTRUMENTGEN4POLLING_H
#define LIBRBR_RBRINSTRUMENTGEN4POLLING_H

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
 * Because polled samples are indistinguishable from streamed samples, this
 * function may return a streamed sample, _not_ a polled sample, if the
 * instrument is logging, streaming is enabled for the link over which the
 * library is communicating with the instrument, and a streamed sample is
 * produced by the instrument before the response to the `poll` command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] sample the polled sample
 * \return #RBRINSTRUMENTGEN4_SUCCESS when a sample is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/data-sample/poll
 */
RBRInstrumentGen4Error RBRInstrumentGen4_poll(
    RBRInstrumentGen4 *instrument,
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
 * Because polled samples are indistinguishable from streamed samples, this
 * function may return a streamed sample, _not_ a polled sample, if the
 * instrument is logging, streaming is enabled for the link over which the
 * library is communicating with the instrument, and a streamed sample is
 * produced by the instrument before the response to the `poll` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] channelList the channels to sample
 * \param [out] sample the polled sample
 * \return #RBRINSTRUMENTGEN4_SUCCESS when a sample is successfully read
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the channel list
 *         is too long to send
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when an invalid channel is
 *         requested
 * \see https://docs.rbr-global.com/L3commandreference/commands/data-sample/poll
 */
RBRInstrumentGen4Error RBRInstrumentGen4_pollChannels(
    RBRInstrumentGen4 *instrument,
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
 * Because polled samples are indistinguishable from streamed samples, this
 * function may return a streamed sample, _not_ a polled sample, if the
 * instrument is logging, streaming is enabled for the link over which the
 * library is communicating with the instrument, and a streamed sample is
 * produced by the instrument before the response to the `poll` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] groupList the groups of channels to sample
 * \param [out] sample the polled sample
 * \return #RBRINSTRUMENTGEN4_SUCCESS when a sample is successfully read
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the group list is
 *         too long to send
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when an invalid group is
 *         requested
 * \see https://docs.rbr-global.com/L3commandreference/commands/data-sample/poll
 */
RBRInstrumentGen4Error RBRInstrumentGen4_pollGroups(
    RBRInstrumentGen4 *instrument,
    const char *groupList,
    RBRInstrumentGen4Sample *sample);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTGEN4POLLING_H */
