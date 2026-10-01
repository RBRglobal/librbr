/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file RBRGen3Fetching.h
 *
 * \brief Instrument commands and structures pertaining to on-demand data
 * acquisition.
 */

#ifndef LIBRBR_RBRGEN3FETCHING_H
#define LIBRBR_RBRGEN3FETCHING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RBRGen3.h"
/* Required for RBRGen3LabelsList, RBRGen3Sample. */
#include "RBRGen3Streaming.h"

/**
 * \brief Requests an “on-demand” sample set from the logger.
 *
 * Unlike streaming data/RBRGen3_readSample(), fetched data is returned
 * directly to the caller (independent of any RBRGen3SampleCallback
 * defined in RBRGen3Environment.sample).
 *
 * Because fetched samples are indistinguishable from streamed samples, this
 * function may return a streamed sample, _not_ a fetched sample, if the
 * instrument is logging, streaming is enabled for the link over which the
 * library is communicating with the instrument, and a streamed sample is
 * produced by the instrument before the response to the `fetch` command.
 *
 * For Logger3 instruments, the \a channels argument can be used to control
 * which channels are fetched. This can be useful to limit the use of
 * power-hungry sensors. If \a channels is not given as `NULL`, then readings
 * will be requested from channels corresponding to the first
 * RBRGen3LabelsList.len labels from the list. Otherwise, and for
 * Logger2 instruments, readings will be fetched from all enabled channels.
 *
 * \param [in] conn the instrument connection
 * \param [in] channels the list of channels to be acquired (may be `NULL`); its length must not
 *             exceed its size; an empty list selects every channel, and a list
 *             RBRGen3_getLabelsList() returned as truncated selects only the labels it holds
 * \param [in] sleepAfter whether the instrument should sleep after fetching
 * \param [in,out] sample the fetched sample; RBRGen3Sample.readings and RBRGen3Sample.size must be
 *                 set by the caller
 * \return #RBRGEN3_SUCCESS when a sample is successfully read
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when \a sample has no readings storage, or \a channels
 *         has a negative length, more labels than storage, or labels but no label storage
 * \return #RBRGEN3_COMMAND_TOO_LONG when a channel label does not fit the command buffer; the
 *         command itself may be longer than the buffer
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when an invalid channel is requested, or another hardware error
 *         occurs
 * \see RBRGen3LabelsList
 * \see RBRGen3Sample
 * \see RBRGen3_readSample()
 */
RBRGen3Error RBRGen3_fetch(RBRGen3 *conn, RBRGen3LabelsList *channels, bool sleepAfter,
                           RBRGen3Sample *sample);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN3FETCHING_H */
