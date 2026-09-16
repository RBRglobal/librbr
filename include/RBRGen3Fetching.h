/**
 * \file RBRGen3Fetching.h
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

#ifndef LIBRBR_RBRGEN3FETCHING_H
#define LIBRBR_RBRGEN3FETCHING_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Requests an “on-demand” sample set from the logger.
 *
 * Unlike streaming data/RBRGen3_readSample(), fetched data is returned
 * directly to the caller (independent of any RBRGen3SampleCallback
 * defined in RBRGen3Callbacks.sample).
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
 * RBRGen3LabelsList.count labels from the list. Otherwise, and for
 * Logger2 instruments, readings will be fetched from all enabled channels.
 *
 * \param [in] conn the instrument connection
 * \param [in] channels the list of channels to be acquired (may be `NULL`)
 * \param [in] sleepAfter whether the instrument should sleep after fetching
 * \param [in,out] sample the fetched sample
 * \return #RBRGEN3_SUCCESS when a sample is successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when an invalid channel is requested, or
 *                                 another hardware error occurs
 * \see https://docs.rbr-global.com/L3commandreference/commands/data-sample/fetch
 */
RBRGen3Error RBRGen3_fetch(RBRGen3 *conn,
                                       RBRGen3LabelsList *channels,
                                       bool sleepAfter,
                                       RBRGen3Sample *sample);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN3FETCHING_H */
