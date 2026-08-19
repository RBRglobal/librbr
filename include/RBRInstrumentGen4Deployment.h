/**
 * \file RBRInstrumentGen4Deployment.h
 *
 * \brief Instrument commands and structures pertaining to deployments.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/deployments
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTGEN4DEPLOYMENT_H
#define LIBRBR_RBRINSTRUMENTGEN4DEPLOYMENT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Configuration.h"
#include "RBRInstrumentGen4Memory.h"
#include "RBRInstrumentGen4Schedule.h"

/**
 * \brief Performs all the same deployment consistency checks which the
 * enable command performs.It then reports the same response which the enable
 * command would produce, However, it does not actually enable the logger for
 * sampling.
 * \note Issues the `verify` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] config specifies the configuration which will define this 
 * deployment.  The configuration must be valid.
 * \param [in] datasetLabel new dataset label for this deployment. It needs to be new name.
 * \param [out] state the state which would be assumed by logger if the enable
 * command were issued. If a warning or an error occurs, this parameter not modified.
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if an error or a warning would occur when enabling logging
 * \see RBRInstrumentGen4_enable()
 * \see https://docs.rbr-global.com/L3commandreference/commands/deployments/verify
 */
RBRInstrumentGen4Error RBRInstrumentGen4_verify(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Config *config, 
    const char datasetLabel[],
    RBRInstrumentGen4InstrumentState *state);

/** \brief Possible storage modes in `enable` command.
 * \see RBRInstrumentGen4DeploymentEnable
 * \see RBRInstrumentGen4_enable()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828472/verify
 * \see meminfo dataType command for more details.
 */
typedef enum RBRInstrumentGen4DeploymentStoragemode
{
    /** Calibration equations will be applied to all channel data. */
    RBRINSTRUMENTGEN4_STORAGEMODE_NORMAL,
    /** All data values are stored as IEEE double precision floating point
     *  numbers in the nominal range 0.0 to 1.0
     */
    RBRINSTRUMENTGEN4_STORAGEMODE_CALIBRATION,
    /** The number of specific storage modes. */
    RBRINSTRUMENTGEN4_STORAGEMODE_COUNT,
    /** An unknown or unrecognized storage mode. */
    RBRINSTRUMENTGEN4_UNKNOWN_STORAGEMODE,
} RBRInstrumentGen4DeploymentStoragemode;

/**
 * \brief Get a human-readable string name for a deployment storageMode.
 *
 * \param [in] storageMode the deployment storage mode
 * \return a string name for the deployment storage mode
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4DeploymentStoragemode_name(
    RBRInstrumentGen4DeploymentStoragemode storageMode);
 
/**
 * \brief Enable the instrument to sample according to the programmed schedule.
 * \note Issues the `enable` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] config specifies the configuration which will define this deployment.
 * \param [in] datasetLabel the new datasetLabel.
 * \param [in] storageMode (opional) default to normal.
 * determines whether calibration equations will 
 * be applied to all channel data (normal), or not (calibration).  The 
 * setting applies only to the current deployment.  
 * When storageMode = calibration, all data 
 * values are stored as IEEE double precision floating point numbers in 
 * the nominal range 0.0 to 1.0, regardless of the normal storage format used. 
 * \param [inout] datasetPool datasetPool.
 * \param [out] newDataset the new dataset.
 * \param [out] state the status of the logger.
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when an error occurs enabling logging
 * \see RBRInstrumentGen4_disable()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828476/enable
 */
RBRInstrumentGen4Error RBRInstrumentGen4_enable(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Config *config,
    const char datasetLabel[],
    const RBRInstrumentGen4DeploymentStoragemode storageMode,
    RBRInstrumentGen4DatasetPool *datasetPool,
    RBRInstrumentGen4Dataset **newDataset,
    RBRInstrumentGen4InstrumentState *state);

/**
 * \brief If the instrument is logging, terminate the current deployment.
 * \note Issues the `disable` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [inout] state the status of logger
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_enable()
 * \see https://docs.rbr-global.com/L3commandreference/commands/deployments/disable
 */
RBRInstrumentGen4Error RBRInstrumentGen4_disable(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4InstrumentState *state);

/**
 * \brief Instrument `simulation` command parameters.
 *
 * \see RBRInstrumentGen4_getSimulation()
 * \see RBRInstrumentGen4_setSimulation()
 * \see https://docs.rbr-global.com/L3commandreference/commands/deployments/simulation
 */
typedef struct RBRInstrumentGen4Simulation
{
    /** \brief factory default is off.
     * It determines whether the channels specified in the channelList 
     * option will be simulated (on), or report true measured data (off).  
     * The default setting as shipped from the Factory is off.  When changed, 
     * the setting is persistent; it will remain in force by default from one 
     * deployment to the next.  However this default setting can be overridden 
     * on a per deployment basis with `enable` command.
     * Change of this parameter is protected, meaning it needs `permit command=simulation`
     * to succeed.
     */
    bool state;

    /**
     * \brief The period of each simulated profile.
     *
     * Specified in milliseconds. Must be greater than 0.
     */
    RBRInstrumentGen4Period period;

    /** \brief specifies which channels will be simulated. */
    RBRInstrumentGen4Channel *channelList[RBRINSTRUMENTGEN4_CHANNEL_MAX];
} RBRInstrumentGen4Simulation;

/**
 * \brief Get the instrument simulation settings.
 * \note Issues the `simulation` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] channelPool the channel pool to associate channel labels with
 * \param [out] simulation the simulation parameters
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the feature is unavailable
 * \see RBRInstrumentGen4_setSimulation()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828483/simulation
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getSimulation(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4ChannelPool *channelPool,
    RBRInstrumentGen4Simulation *simulation);

/**
 * \brief Set the instrument simulation settings.
 * \note Issues the `simulation` instrument command.
 *
 * Hardware errors may occur if:
 *
 * - simulations are not available for the instrument (either due to its
 *   firmware version or channel configuration)
 * - the instrument is logging
 * - you set an out-of-bounds parameter the library fails to detect
 *
 * \param [in] instrument the instrument connection
 * \param [in] simulation the simulation parameters
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when an out-of-bounds
 *                                                simulation period is
 *                                                requested
 * \see RBRInstrumentGen4_getSimulation()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828483/simulation
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setSimulation(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Simulation *simulation);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTDEPLOYMENT_H */
