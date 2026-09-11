/**
 * \file RBRInstrumentGen4Instrument.h
 *
 * \brief Instrument commands and structures for miscellaneous commands.
 *
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/41582593/instrument 
 *
 * \copyright
 * Copyright (c) 2024 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTGEN4INSTRUMENT_H
#define LIBRBR_RBRINSTRUMENTGEN4INSTRUMENT_H

#include "RBRInstrumentGen4.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Possible instrument instrument states.
 * This tracks whether the deployment is running on the instrument.
 *
 * Returned by:
 * \see RBRInstrumentGen4_getInstrument()
 * \see RBRInstrumentGen4_enable()
 * \see RBRInstrumentGen4_verify()
 *
 * For the deployment state:
 * \see RBRInstrumentGen4DeploymentStatus
 * \see RBRInstrumentGen4Deployment
 */
typedef enum RBRInstrumentGen4InstrumentState
{
    /** Logging is not enabled. */
    RBRINSTRUMENTGEN4_INSTRUMENT_STATE_DISABLED,
    /** Logging for at least one deployment is enabled. */
    RBRINSTRUMENTGEN4_INSTRUMENT_STATE_ENABLED,
    /** The number of specific instrument states. */
    RBRINSTRUMENTGEN4_INSTRUMENT_STATE_COUNT,
    /** An unknown or unrecognized instrument state. */
    RBRINSTRUMENTGEN4_UNKNOWN_INSTRUMENT_STATE
} RBRInstrumentGen4InstrumentState;

/**
 * \brief Get a human-readable string name for a instrument state.
 *
 * \param [in] state the instrument state
 * \return a string name for the instrument state
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4InstrumentState_name(
    RBRInstrumentGen4InstrumentState status);

/**
 * \brief The maximum number of characters in the instrument name.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENTGEN4_INSTRUMENT_NAME_MAX 32

/**
 * \brief The maximum number of PCBAs in the instrument.
 */
#define RBRINSTRUMENTGEN4_PCBA_COUNT_MAX 12

/**
 * \brief Get identification information using the legacy `id` command.
 * \note Issues the `id` instrument command.
 *
 * `id` predates the Gen4 API and keeps its original grammar: parameters are
 * separated by commas and assignments are padded with spaces. It reports the
 * same information as `id4` less the Semantic Version; prefer
 * RBRInstrumentGen4_getId4() unless the legacy command is specifically
 * wanted.
 *
 * \param [in] instrument the instrument connection
 * \param [out] id the instrument information
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_getId4()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830290/id
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getId(RBRInstrumentGen4 *instrument,
                                       RBRInstrumentGen4Id *id);

/**
 * \brief Get identification information from the instrument.
 * \note Issues the `id4` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] id the instrument information
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_getInstrument();
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830290/id
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getId4(RBRInstrumentGen4 *instrument,
                                       RBRInstrumentGen4Id4 *id);

/**
 * \brief Instrument `pcba <pcba_label>` command parameters.
 *
 * \see RBRInstrumentGen4_getPcba()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/44761149/pcba
 */
typedef struct RBRInstrumentGen4Pcba
{
    /**
     * \brief PCBA label.
     *
     * Set by the caller to select the PCBA to read; see
     * RBRInstrumentGen4_getPcba().
     */
    char label[RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];
    /**
     * \brief PCBA serial number.
     *
     * Zero when the instrument has no serial number recorded for the PCBA,
     * which it reports as `na`.
     */
    int32_t sn;
    /**
     * \brief PCBA part number.
     *
     * Reported as `na` when unrecorded.
     */
    char pn[RBRINSTRUMENTGEN4_PART_NUMBER_MAX + 1];
    /** \brief The label of the node this PCBA belongs to. */
    char node[RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];
} RBRInstrumentGen4Pcba;

/**
 * \brief Instrument `pcba` command parameters.
 *
 * \see RBRInstrumentGen4_getPcbaPool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/44761149/pcba
 */
typedef struct RBRInstrumentGen4PcbaPool {
    /** \brief The number of PCBAs detected. */
    int32_t count;

    /** \brief The pool of PCBAs. */
    RBRInstrumentGen4Pcba pool[RBRINSTRUMENTGEN4_PCBA_COUNT_MAX];
} RBRInstrumentGen4PcbaPool ;

/**
 * \brief Populate the pool of the instrument's PCBAs.
 *
 * \param [in] instrument the instrument connection
 * \param [inout] pcbaPool the PCBAs of this instrument.
 * \return #RBRINSTRUMENTGEN4_SUCCESS when all PCBAs are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if a PCBA cannot be read
 * \see RBRInstrumentGen4_getPcba()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/44761149/pcba
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getPcbaPool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4PcbaPool *pcbaPool);

/**
 * \brief Get an instrument's PCBA's parameters.
 * \note Issues the `pcba <pcba_label>` instrument command.
 *
 * RBRInstrumentGen4Pcba.label must be populated by the caller to select the
 * PCBA to read; the remaining fields are overwritten. Labels can be
 * discovered with RBRInstrumentGen4_getPcbaPool().
 *
 * \param [in] instrument the instrument connection
 * \param [inout] pcba the label of the PCBA to read, and its information
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the information cannot be read
 * \see RBRInstrumentGen4_getPcbaPool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/44761149/pcba
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getPcba(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Pcba *pcba);

/**
 * \brief Possible instrument power sources.
 *
 * \see RBRInstrumentGen4_getPowerSource()
 */
typedef enum RBRInstrumentGen4PowerSource
{
    /** USB power. */
    RBRINSTRUMENTGEN4_POWER_SOURCE_USB,
    /** Internal (battery) power. */
    RBRINSTRUMENTGEN4_POWER_SOURCE_INTERNAL,
    /** External power. */
    RBRINSTRUMENTGEN4_POWER_SOURCE_EXTERNAL,
    /** The number of specific power sources. */
    RBRINSTRUMENTGEN4_POWER_SOURCE_COUNT,
    /** An unknown or unrecognized power source. */
    RBRINSTRUMENTGEN4_POWER_SOURCE_UNKNOWN
} RBRInstrumentGen4PowerSource;

/**
 * \brief Get a human-readable string name for a power source.
 *
 * \param [in] source the power source
 * \return a string name for the power source
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4PowerSource_name(RBRInstrumentGen4PowerSource source);

/**
 * \brief Get instrument power information.
 * \note Issues the `instrument power` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] powerSource the power source from which the instrument is running
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the information cannot be read
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830328/power
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getPowerSource(RBRInstrumentGen4 *instrument,
                                          RBRInstrumentGen4PowerSource *powerSource);

/**
 * \brief Internal battery types.
 *
 * \see RBRInstrumentGen4PowerInternal
 */
typedef enum RBRInstrumentGen4InternalBatteryType
{
    /** No internal battery */
    RBRINSTRUMENTGEN4_INTERNAL_BATTERY_NONE,
    /** Li-SOCl₂ */
    RBRINSTRUMENTGEN4_INTERNAL_BATTERY_LISOCL2,
    /** Li-FeS₂ */
    RBRINSTRUMENTGEN4_INTERNAL_BATTERY_LIFES2,
    /** Zn-MnO₂ */
    RBRINSTRUMENTGEN4_INTERNAL_BATTERY_ZNMNO2,
    /** Li-NiMnCo */
    RBRINSTRUMENTGEN4_INTERNAL_BATTERY_LINIMNCO,
    /** NiMH */
    RBRINSTRUMENTGEN4_INTERNAL_BATTERY_NIMH,
    /** The number of specific internal battery types. */
    RBRINSTRUMENTGEN4_INTERNAL_BATTERY_COUNT,
    /** An unknown or unrecognized internal battery type. */
    RBRINSTRUMENTGEN4_UNKNOWN_INTERNAL_BATTERY
} RBRInstrumentGen4InternalBatteryType;

/**
 * \brief Get a human-readable string name for an internal battery type.
 *
 * \param [in] type the battery type
 * \return a string name for the battery type
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 * \see RBRInstrumentGen4InternalBatteryType_displayName() for display names
 */
const char *RBRInstrumentGen4InternalBatteryType_name(
    RBRInstrumentGen4InternalBatteryType type);

/**
 * \brief Get a human-readable display name for an internal battery type.
 *
 * Unlike the values returned by RBRInstrumentGen4InternalBatteryType_name(),
 * the names of battery types will be formatted appropriately for the cell
 * chemistry; e.g., “Li-SOCl₂”, not “lisocl2”. Values will be UTF-8-encoded.
 *
 * \param [in] type the battery type
 * \return a string name for the battery type
 * \see RBRInstrumentGen4InternalBatteryType_name() for instrument-equivalent names
 */
const char *RBRInstrumentGen4InternalBatteryType_displayName(
    RBRInstrumentGen4InternalBatteryType type);

/**
 * \brief Instrument `instrument power internal` command parameters.
 *
 * \see RBRInstrumentGen4_getPowerInternal()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828548/internal
 */
typedef struct RBRInstrumentGen4PowerInternal
{
    /**
     * \brief The measured voltage of any internal power source.
     *
     * \readonly
     */
    float voltage;
    /** \brief The type of battery. */
    RBRInstrumentGen4InternalBatteryType batteryType;
    /**
     * \brief The accumulated energy used from the internal battery since the
     * value was last reset.
     */
    float used;
} RBRInstrumentGen4PowerInternal;

/**
 * \brief Get instrument internal power information.
 * \note Issues the `instrument power internal` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] power the power information
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if an error occurs reading voltages
 * \see RBRInstrumentGen4_setPowerInternalBatteryType()
 * \see RBRInstrumentGen4_resetPowerInternalUsed()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828548/internal
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getPowerInternal(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4PowerInternal *power);

/**
 * \brief Set the internal power battery type.
 * \note Issues the `instrument power internal` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] type the battery type
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the instrument is logging
 * \see RBRInstrumentGen4_getPowerInternal()
 * \see RBRInstrumentGen4_resetPowerInternalUsed()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828548/internal
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setPowerInternalBatteryType(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4InternalBatteryType type);

/**
 * \brief Reset the counter of energy used from the internal battery.
 * \note Issues the `instrument power internal` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the instrument is logging
 * \see RBRInstrumentGen4_getPowerInternal()
 * \see RBRInstrumentGen4_setPowerInternalBatteryType()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828548/internal
 */
RBRInstrumentGen4Error RBRInstrumentGen4_resetPowerInternalUsed(
    RBRInstrumentGen4 *instrument);

/**
 * \brief External battery types.
 *
 * \see RBRInstrumentGen4PowerExternal
 */
typedef enum RBRInstrumentGen4ExternalBatteryType
{
    /** No external battery */
    RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_NONE,
    /** RBRfermata Li-SOCl₂ */
    RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMATA_LISOCL2,
    /** RBRfermata Zn-MnO₂ */
    RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMATA_ZNMNO2,
    /** RBRfermette Li-MnO₂ */
    RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE_LIMNO2,
    /** RBRfermette³ Li-SOCl₂ */
    RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_LISOCL2,
    /** RBRfermette³ Li-FeS₂ */
    RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_LIFES2,
    /** RBRfermette³ Zn-MnO₂ */
    RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_ZNMNO2,
    /** RBRfermette³ Li-NiMnCo */
    RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_LINIMNCO,
    /** RBRfermette³ NiMH */
    RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_NIMH,
    /** RBRfermana NiMH - fw version 1.140 or later */
    RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMATA_NIMH,
    /** Other/unknown external battery type */
    RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_OTHER,
    /** The number of specific external battery types. */
    RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_COUNT,
    /** An unknown or unrecognized external battery type. */
    RBRINSTRUMENTGEN4_UNKNOWN_EXTERNAL_BATTERY
} RBRInstrumentGen4ExternalBatteryType;

/**
 * \brief Get a human-readable string name for an external battery type.
 *
 * \param [in] type the battery type
 * \return a string name for the battery type
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 * \see RBRInstrumentGen4ExternalBatteryType_displayName() for display names
 */
const char *RBRInstrumentGen4ExternalBatteryType_name(
    RBRInstrumentGen4ExternalBatteryType type);

/**
 * \brief Get a human-readable display name for an external battery type.
 *
 * Unlike the values returned by RBRInstrumentGen4ExternalBatteryType_name(),
 * RBRfermata/RBRfermette product names will be correctly capitalized, and the
 * names of battery types will be formatted appropriately for the cell
 * chemistry; e.g., “RBRfermette³ Li-SOCl₂”, not “rbrfermette3 lisocl2”. Values
 * will be UTF-8-encoded.
 *
 * \param [in] type the battery type
 * \return a string name for the battery type
 * \see RBRInstrumentGen4ExternalBatteryType_name() for instrument-equivalent names
 */
const char *RBRInstrumentGen4ExternalBatteryType_displayName(
    RBRInstrumentGen4ExternalBatteryType type);

/**
 * \brief Instrument `instrument power external` command parameters.
 *
 * \see RBRInstrumentGen4_getPowerExternal()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828551/external
 */
typedef struct RBRInstrumentGen4PowerExternal
{
    /**
     * \brief The measured voltage of any external power source.
     *
     * \readonly
     */
    float voltage;
    /** \brief The type of battery. */
    RBRInstrumentGen4ExternalBatteryType batteryType;
    /**
     * \brief The accumulated energy used from the external battery since the
     * value was last reset.
     */
    float used;
} RBRInstrumentGen4PowerExternal;

/**
 * \brief Get instrument external power information.
 *
 *
 * \param [in] instrument the instrument connection
 * \param [out] power the power information
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_setPowerExternalBatteryType()
 * \see RBRInstrumentGen4_resetPowerExternalUsed()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerexternal
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getPowerExternal(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4PowerExternal *power);

/**
 * \brief Set the external power battery type.
 * \note Issues the `instrument power external` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] type the battery type
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the instrument is logging
 * \see RBRInstrumentGen4_getPowerExternal()
 * \see RBRInstrumentGen4_resetPowerExternalUsed()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828551/external
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setPowerExternalBatteryType(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4ExternalBatteryType type);

/**
 * \brief Reset the counter of energy used from the external battery.
 * \note Issues the `instrument power external` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the instrument is logging
 * \see RBRInstrumentGen4_getPowerExternal()
 * \see RBRInstrumentGen4_setPowerExternalBatteryType()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828551/external
 */
RBRInstrumentGen4Error RBRInstrumentGen4_resetPowerExternalUsed(
    RBRInstrumentGen4 *instrument);

/**
 * \brief Instrument `instrument` command parameters.
 *
 * Distinct from #RBRInstrumentGen4, which is the connection to an instrument.
 *
 * Fields are declared in the order the instrument reports them.
 *
 * \see RBRInstrumentGen4_getInstrument()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/41582593/instrument
 */
typedef struct RBRInstrumentGen4Instrument
{
    /** \brief Whether a deployment is currently logging. */
    RBRInstrumentGen4InstrumentState state;
    /** \brief The serial number of the instrument. */
    int32_t sn;
    /** \brief The instrument model. */
    char model[RBRINSTRUMENTGEN4_ID_MODEL_MAX + 1];
    /** \brief The RBR part number of the instrument. */
    char pn[RBRINSTRUMENTGEN4_PART_NUMBER_MAX + 1];
    /** \brief The instrument firmware version. */
    char fwversion[RBRINSTRUMENTGEN4_ID_VERSION_MAX + 1];
    /**
     * \brief The instrument firmware version in Semantic Version form.
     *
     * For example, `2.0.0-rc1-10-g148bc5eb1`.
     */
    char semver[RBRINSTRUMENTGEN4_ID_SEMVER_MAX + 1];
    /** \brief The firmware type of the instrument. */
    int32_t fwtype;
    /** \brief Whether firmware upgrades are locked. */
    bool fwLock;
    /** \brief The data type used by the instrument's samples. */
    RBRInstrumentGen4DataType dataType;
    /**
     * \brief The extended model name of the instrument.
     *
     * For example, `RBRsolo^4_T.D!fast32`.
     */
    char name[RBRINSTRUMENTGEN4_INSTRUMENT_NAME_MAX + 1];
} RBRInstrumentGen4Instrument;

/**
 * \brief Get the instrument's identity and state.
 * \note Issues the `instrument` instrument command.
 *
 * All of the parameters the command reports are returned. They are read-only,
 * so there is no corresponding setter.
 *
 * \param [in] instrument the instrument connection
 * \param [out] instrumentInfo the instrument information
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_getId4()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/41582593/instrument
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getInstrument(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Instrument *instrumentInfo);

/**
 * \brief Get the current output format.
 * \note Issues the `instrument outputformat` command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] outputformat the current output format
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_setOutputFormat()
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getOutputFormat(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4OutputFormat *outputformat);

/**
 * \brief Set the current output format.
 * \note Issues the `instrument outputformat` command.
 *
 * Every parameter of the command is sent, so \a outputformat must be fully
 * populated: read the current format with RBRInstrumentGen4_getOutputFormat()
 * and modify it if only some parameters are of interest.
 *
 * \warning RBRParserGen4 reads only #RBRINSTRUMENTGEN4_ENCODING_ASCII.
 *          Selecting #RBRINSTRUMENTGEN4_ENCODING_BINARY will stop this library
 *          from being able to interpret samples or command responses.
 *
 * \param [in] instrument the instrument connection
 * \param [in] outputformat the desired output format
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the encoding or
 *                                                   datatype is not a real
 *                                                   value
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the instrument refuses a value
 * \see RBRInstrumentGen4_getOutputFormat()
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setOutputFormat(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4OutputFormat *outputformat);

/**
 * \brief Return the instrument's configuration to its factory state.
 * \note Issues the `instrument factory reset` command.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the instrument has been reset
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the instrument refuses
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/factory
 */
RBRInstrumentGen4Error RBRInstrumentGen4_factoryReset(
    RBRInstrumentGen4 *instrument);

/**
 * \brief Reset the instrument CPU.
 * \note Issues the `instrument reboot` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] delay time in milliseconds to wait before rebooting; zero omits
 *                   the parameter, rebooting without a delay. The command
 *                   has no default delay of its own.
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the reboot has been requested
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/security-and-interaction/reboot
 */
RBRInstrumentGen4Error RBRInstrumentGen4_reboot(RBRInstrumentGen4 *instrument,
                                        const int32_t delay);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTGEN4INSTRUMENT_H */
