/**
 * \file RBRInstrumentFetching.h
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

#ifndef LIBRBR_RBRINSTRUMENTFETCHING_H
#define LIBRBR_RBRINSTRUMENTFETCHING_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Requests an “on-demand” sample set from the logger.
 *
 * Unlike streaming data/RBRInstrument_readSample(), fetched data is returned
 * directly to the caller (independent of any RBRInstrumentSampleCallback
 * defined in RBRInstrumentCallbacks.sample).
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
 * RBRInstrumentLabelsList.count labels from the list. Otherwise, and for
 * Logger2 instruments, readings will be fetched from all enabled channels.
 *
 * \param [in] instrument the instrument connection
 * \param [in] channels the list of channels to be acquired (may be `NULL`)
 * \param [in] sleepAfter whether the instrument should sleep after fetching
 * \param [in,out] sample the fetched sample
 * \return #RBRINSTRUMENT_SUCCESS when a sample is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when an invalid channel is requested
 * \see https://docs.rbr-global.com/L3commandreference/commands/data-sample/fetch
 */
RBRInstrumentError RBRInstrument_fetch(RBRInstrument *instrument,
                                       RBRInstrumentLabelsList *channels,
                                       bool sleepAfter,
                                       RBRInstrumentSample *sample);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTFETCHING_H */
