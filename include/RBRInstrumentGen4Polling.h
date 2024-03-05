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

/**
 * \brief Requests an “on-demand” sample of one channel from the logger.
 *
 * Unlike streaming data/RBRInstrumentGen4_readSample(), polled data is returned
 * directly to the caller (independent of any RBRInstrumentGen4SampleCallback
 * defined in RBRInstrumentGen4Callbacks.sample).
 *
 * Because polled samples are indistinguishable from streamed samples, this
 * function may return a streamed sample, _not_ a polled sample, if the
 * instrument is logging, streaming is enabled for the link over which the
 * library is communicating with the instrument, and a streamed sample is
 * produced by the instrument before the response to the `poll` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] channellabel specifies one channel to sample
 * \param [in] sleepafter determines if the power-down delay will be infoked follwing
     * completion of the polling operation
 * \param [out] sample the polled sample
 * \return #RBRINSTRUMENTGEN4_SUCCESS when a sample is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when an invalid channel is requested
 * \see https://docs.rbr-global.com/L3commandreference/commands/data-sample/poll
 */
RBRInstrumentGen4Error RBRInstrumentGen4_pollOneChannel(RBRInstrumentGen4 *instrument,
                                       const char *channellabel,
                                       const bool sleepafter, 
                                       RBRInstrumentGen4Sample *sample);

/**
 * \brief Requests an “on-demand” sample set of a group from the logger.
 *
 * Unlike streaming data/RBRInstrumentGen4_readSample(), polled data is returned
 * directly to the caller (independent of any RBRInstrumentGen4SampleCallback
 * defined in RBRInstrumentGen4Callbacks.sample).
 *
 * Because polled samples are indistinguishable from streamed samples, this
 * function may return a streamed sample, _not_ a polled sample, if the
 * instrument is logging, streaming is enabled for the link over which the
 * library is communicating with the instrument, and a streamed sample is
 * produced by the instrument before the response to the `poll` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] grouplabel specifies one group to sample
 * \param [in] sleepafter determines if the power-down delay will be infoked follwing
     * completion of the polling operation
 * \param [out] sample the polled sample
 * \return #RBRINSTRUMENTGEN4_SUCCESS when a sample is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when an invalid channel is requested
 * \see https://docs.rbr-global.com/L3commandreference/commands/data-sample/poll
 */
RBRInstrumentGen4Error RBRInstrumentGen4_pollOneGroup(RBRInstrumentGen4 *instrument,
                                       const char *grouplabel,
                                       const bool sleepafter,
                                       RBRInstrumentGen4Sample *sample);

/**
 * \brief Requests an “on-demand” sample set of all channels from the logger.
 *
 * Unlike streaming data/RBRInstrumentGen4_readSample(), polled data is returned
 * directly to the caller (independent of any RBRInstrumentGen4SampleCallback
 * defined in RBRInstrumentGen4Callbacks.sample).
 *
 * Because polled samples are indistinguishable from streamed samples, this
 * function may return a streamed sample, _not_ a polled sample, if the
 * instrument is logging, streaming is enabled for the link over which the
 * library is communicating with the instrument, and a streamed sample is
 * produced by the instrument before the response to the `poll` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] sleepafter determines if the power-down delay will be infoked follwing
     * completion of the polling operation
 * \param [out] sample the polled sample
 * \return #RBRINSTRUMENTGEN4_SUCCESS when a sample is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when an invalid channel is requested
 * \see https://docs.rbr-global.com/L3commandreference/commands/data-sample/poll
 */
RBRInstrumentGen4Error RBRInstrumentGen4_pollAllChannels(RBRInstrumentGen4 *instrument,
                                       const bool sleepafter, 
                                       RBRInstrumentGen4Sample *sample);
                            

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTPOLLING_H */
