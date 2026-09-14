/**
 * \file RBRGen4Instrument.h
 *
 * \brief Instrument commands and structures for miscellaneous commands.
 *
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/41582593/instrument 
 *
 * \copyright
 * Copyright (c) 2024 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRGEN4INSTRUMENT_H
#define LIBRBR_RBRGEN4INSTRUMENT_H

#include "RBRGen4.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Possible instrument instrument states.
 * This tracks whether the deployment is running on the instrument.
 *
 * Returned by:
 * \see RBRGen4_getInstrument()
 * \see RBRGen4_enable()
 * \see RBRGen4_verify()
 *
 * For the deployment state:
 * \see RBRGen4DeploymentStatus
 * \see RBRGen4Deployment
 */
typedef enum RBRGen4InstrumentState
{
    /** Logging is not enabled. */
    RBRGEN4_INSTRUMENT_STATE_DISABLED,
    /** Logging for at least one deployment is enabled. */
    RBRGEN4_INSTRUMENT_STATE_ENABLED,
    /** The number of specific instrument states. */
    RBRGEN4_INSTRUMENT_STATE_COUNT,
    /** An unknown or unrecognized instrument state. */
    RBRGEN4_UNKNOWN_INSTRUMENT_STATE
} RBRGen4InstrumentState;

/**
 * \brief Get a human-readable string name for a instrument state.
 *
 * \param [in] state the instrument state
 * \return a string name for the instrument state
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4InstrumentState_name(
    RBRGen4InstrumentState status);

/**
 * \brief The maximum number of characters in the instrument name.
 *
 * Does not include any null terminator.
 */
#define RBRGEN4_INSTRUMENT_NAME_MAX 32

/**
 * \brief The maximum number of PCBAs in the instrument.
 */
#define RBRGEN4_PCBA_COUNT_MAX 12

/**
 * \brief Get identification information using the legacy `id` command.
 * \note Issues the `id` instrument command.
 *
 * `id` predates the Gen4 API and keeps its original grammar: parameters are
 * separated by commas and assignments are padded with spaces. It reports the
 * same information as `id4` less the Semantic Version; prefer
 * RBRGen4_getId4() unless the legacy command is specifically
 * wanted.
 *
 * \param [in] instrument the instrument connection
 * \param [out] id the instrument information
 * \return #RBRGEN4_SUCCESS when the information is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRGen4_getId4()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830290/id
 */
RBRGen4Error RBRGen4_getId(RBRGen4 *instrument,
                                       RBRGen4Id *id);

/**
 * \brief Get identification information from the instrument.
 * \note Issues the `id4` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] id the instrument information
 * \return #RBRGEN4_SUCCESS when the information is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRGen4_getInstrument();
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830290/id
 */
RBRGen4Error RBRGen4_getId4(RBRGen4 *instrument,
                                       RBRGen4Id4 *id);

/**
 * \brief Instrument `pcba <pcba_label>` command parameters.
 *
 * \see RBRGen4_getPcba()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/44761149/pcba
 */
typedef struct RBRGen4Pcba
{
    /**
     * \brief PCBA label.
     *
     * Set by the caller to select the PCBA to read; see
     * RBRGen4_getPcba().
     */
    char label[RBRGEN4_LABEL_NAME_MAX + 1];
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
    char pn[RBRGEN4_PART_NUMBER_MAX + 1];
    /** \brief The label of the node this PCBA belongs to. */
    char node[RBRGEN4_LABEL_NAME_MAX + 1];
} RBRGen4Pcba;

/**
 * \brief Instrument `pcba` command parameters.
 *
 * \see RBRGen4_getPcbaPool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/44761149/pcba
 */
typedef struct RBRGen4PcbaPool {
    /** \brief The number of PCBAs detected. */
    int32_t count;

    /** \brief The pool of PCBAs. */
    RBRGen4Pcba pool[RBRGEN4_PCBA_COUNT_MAX];
} RBRGen4PcbaPool ;

/**
 * \brief Populate the pool of the instrument's PCBAs.
 *
 * \param [in] instrument the instrument connection
 * \param [inout] pcbaPool the PCBAs of this instrument.
 * \return #RBRGEN4_SUCCESS when all PCBAs are successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR if a PCBA cannot be read
 * \see RBRGen4_getPcba()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/44761149/pcba
 */
RBRGen4Error RBRGen4_getPcbaPool(
    RBRGen4 *instrument,
    RBRGen4PcbaPool *pcbaPool);

/**
 * \brief Get an instrument's PCBA's parameters.
 * \note Issues the `pcba <pcba_label>` instrument command.
 *
 * RBRGen4Pcba.label must be populated by the caller to select the
 * PCBA to read; the remaining fields are overwritten. Labels can be
 * discovered with RBRGen4_getPcbaPool().
 *
 * \param [in] instrument the instrument connection
 * \param [inout] pcba the label of the PCBA to read, and its information
 * \return #RBRGEN4_SUCCESS when the information is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR if the information cannot be read
 * \see RBRGen4_getPcbaPool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/44761149/pcba
 */
RBRGen4Error RBRGen4_getPcba(
    RBRGen4 *instrument,
    RBRGen4Pcba *pcba);

/**
 * \brief Possible instrument power sources.
 *
 * \see RBRGen4_getPowerSource()
 */
typedef enum RBRGen4PowerSource
{
    /** USB power. */
    RBRGEN4_POWER_SOURCE_USB,
    /** Internal (battery) power. */
    RBRGEN4_POWER_SOURCE_INTERNAL,
    /** External power. */
    RBRGEN4_POWER_SOURCE_EXTERNAL,
    /** The number of specific power sources. */
    RBRGEN4_POWER_SOURCE_COUNT,
    /** An unknown or unrecognized power source. */
    RBRGEN4_POWER_SOURCE_UNKNOWN
} RBRGen4PowerSource;

/**
 * \brief Get a human-readable string name for a power source.
 *
 * \param [in] source the power source
 * \return a string name for the power source
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4PowerSource_name(RBRGen4PowerSource source);

/**
 * \brief Get instrument power information.
 * \note Issues the `instrument power` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] powerSource the power source from which the instrument is running
 * \return #RBRGEN4_SUCCESS when the information is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR if the information cannot be read
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830328/power
 */
RBRGen4Error RBRGen4_getPowerSource(RBRGen4 *instrument,
                                          RBRGen4PowerSource *powerSource);

/**
 * \brief Internal battery types.
 *
 * \see RBRGen4PowerInternal
 */
typedef enum RBRGen4InternalBatteryType
{
    /** No internal battery */
    RBRGEN4_INTERNAL_BATTERY_NONE,
    /** Li-SOCl₂ */
    RBRGEN4_INTERNAL_BATTERY_LISOCL2,
    /** Li-FeS₂ */
    RBRGEN4_INTERNAL_BATTERY_LIFES2,
    /** Zn-MnO₂ */
    RBRGEN4_INTERNAL_BATTERY_ZNMNO2,
    /** Li-NiMnCo */
    RBRGEN4_INTERNAL_BATTERY_LINIMNCO,
    /** NiMH */
    RBRGEN4_INTERNAL_BATTERY_NIMH,
    /** The number of specific internal battery types. */
    RBRGEN4_INTERNAL_BATTERY_COUNT,
    /** An unknown or unrecognized internal battery type. */
    RBRGEN4_UNKNOWN_INTERNAL_BATTERY
} RBRGen4InternalBatteryType;

/**
 * \brief Get a human-readable string name for an internal battery type.
 *
 * \param [in] type the battery type
 * \return a string name for the battery type
 * \see RBRGen4Error_name() for a description of the format of names
 * \see RBRGen4InternalBatteryType_displayName() for display names
 */
const char *RBRGen4InternalBatteryType_name(
    RBRGen4InternalBatteryType type);

/**
 * \brief Get a human-readable display name for an internal battery type.
 *
 * Unlike the values returned by RBRGen4InternalBatteryType_name(),
 * the names of battery types will be formatted appropriately for the cell
 * chemistry; e.g., “Li-SOCl₂”, not “lisocl2”. Values will be UTF-8-encoded.
 *
 * \param [in] type the battery type
 * \return a string name for the battery type
 * \see RBRGen4InternalBatteryType_name() for instrument-equivalent names
 */
const char *RBRGen4InternalBatteryType_displayName(
    RBRGen4InternalBatteryType type);

/**
 * \brief Instrument `instrument power internal` command parameters.
 *
 * \see RBRGen4_getPowerInternal()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828548/internal
 */
typedef struct RBRGen4PowerInternal
{
    /**
     * \brief The measured voltage of any internal power source.
     *
     * \readonly
     */
    float voltage;
    /** \brief The type of battery. */
    RBRGen4InternalBatteryType batteryType;
    /**
     * \brief The accumulated energy used from the internal battery since the
     * value was last reset.
     */
    float used;
} RBRGen4PowerInternal;

/**
 * \brief Get instrument internal power information.
 * \note Issues the `instrument power internal` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] power the power information
 * \return #RBRGEN4_SUCCESS when the information is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR if an error occurs reading voltages
 * \see RBRGen4_setPowerInternalBatteryType()
 * \see RBRGen4_resetPowerInternalUsed()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828548/internal
 */
RBRGen4Error RBRGen4_getPowerInternal(
    RBRGen4 *instrument,
    RBRGen4PowerInternal *power);

/**
 * \brief Set the internal power battery type.
 * \note Issues the `instrument power internal` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] type the battery type
 * \return #RBRGEN4_SUCCESS when the setting is successfully written
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument is logging
 * \see RBRGen4_getPowerInternal()
 * \see RBRGen4_resetPowerInternalUsed()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828548/internal
 */
RBRGen4Error RBRGen4_setPowerInternalBatteryType(
    RBRGen4 *instrument,
    const RBRGen4InternalBatteryType type);

/**
 * \brief Reset the counter of energy used from the internal battery.
 * \note Issues the `instrument power internal` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRGEN4_SUCCESS when the setting is successfully written
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument is logging
 * \see RBRGen4_getPowerInternal()
 * \see RBRGen4_setPowerInternalBatteryType()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828548/internal
 */
RBRGen4Error RBRGen4_resetPowerInternalUsed(
    RBRGen4 *instrument);

/**
 * \brief External battery types.
 *
 * \see RBRGen4PowerExternal
 */
typedef enum RBRGen4ExternalBatteryType
{
    /** No external battery */
    RBRGEN4_EXTERNAL_BATTERY_NONE,
    /** RBRfermata Li-SOCl₂ */
    RBRGEN4_EXTERNAL_BATTERY_FERMATA_LISOCL2,
    /** RBRfermata Zn-MnO₂ */
    RBRGEN4_EXTERNAL_BATTERY_FERMATA_ZNMNO2,
    /** RBRfermette Li-MnO₂ */
    RBRGEN4_EXTERNAL_BATTERY_FERMETTE_LIMNO2,
    /** RBRfermette³ Li-SOCl₂ */
    RBRGEN4_EXTERNAL_BATTERY_FERMETTE3_LISOCL2,
    /** RBRfermette³ Li-FeS₂ */
    RBRGEN4_EXTERNAL_BATTERY_FERMETTE3_LIFES2,
    /** RBRfermette³ Zn-MnO₂ */
    RBRGEN4_EXTERNAL_BATTERY_FERMETTE3_ZNMNO2,
    /** RBRfermette³ Li-NiMnCo */
    RBRGEN4_EXTERNAL_BATTERY_FERMETTE3_LINIMNCO,
    /** RBRfermette³ NiMH */
    RBRGEN4_EXTERNAL_BATTERY_FERMETTE3_NIMH,
    /** RBRfermana NiMH - fw version 1.140 or later */
    RBRGEN4_EXTERNAL_BATTERY_FERMATA_NIMH,
    /** Other/unknown external battery type */
    RBRGEN4_EXTERNAL_BATTERY_OTHER,
    /** The number of specific external battery types. */
    RBRGEN4_EXTERNAL_BATTERY_COUNT,
    /** An unknown or unrecognized external battery type. */
    RBRGEN4_UNKNOWN_EXTERNAL_BATTERY
} RBRGen4ExternalBatteryType;

/**
 * \brief Get a human-readable string name for an external battery type.
 *
 * \param [in] type the battery type
 * \return a string name for the battery type
 * \see RBRGen4Error_name() for a description of the format of names
 * \see RBRGen4ExternalBatteryType_displayName() for display names
 */
const char *RBRGen4ExternalBatteryType_name(
    RBRGen4ExternalBatteryType type);

/**
 * \brief Get a human-readable display name for an external battery type.
 *
 * Unlike the values returned by RBRGen4ExternalBatteryType_name(),
 * RBRfermata/RBRfermette product names will be correctly capitalized, and the
 * names of battery types will be formatted appropriately for the cell
 * chemistry; e.g., “RBRfermette³ Li-SOCl₂”, not “rbrfermette3 lisocl2”. Values
 * will be UTF-8-encoded.
 *
 * \param [in] type the battery type
 * \return a string name for the battery type
 * \see RBRGen4ExternalBatteryType_name() for instrument-equivalent names
 */
const char *RBRGen4ExternalBatteryType_displayName(
    RBRGen4ExternalBatteryType type);

/**
 * \brief Instrument `instrument power external` command parameters.
 *
 * \see RBRGen4_getPowerExternal()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828551/external
 */
typedef struct RBRGen4PowerExternal
{
    /**
     * \brief The measured voltage of any external power source.
     *
     * \readonly
     */
    float voltage;
    /** \brief The type of battery. */
    RBRGen4ExternalBatteryType batteryType;
    /**
     * \brief The accumulated energy used from the external battery since the
     * value was last reset.
     */
    float used;
} RBRGen4PowerExternal;

/**
 * \brief Get instrument external power information.
 *
 *
 * \param [in] instrument the instrument connection
 * \param [out] power the power information
 * \return #RBRGEN4_SUCCESS when the information is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRGen4_setPowerExternalBatteryType()
 * \see RBRGen4_resetPowerExternalUsed()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerexternal
 */
RBRGen4Error RBRGen4_getPowerExternal(
    RBRGen4 *instrument,
    RBRGen4PowerExternal *power);

/**
 * \brief Set the external power battery type.
 * \note Issues the `instrument power external` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] type the battery type
 * \return #RBRGEN4_SUCCESS when the setting is successfully written
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument is logging
 * \see RBRGen4_getPowerExternal()
 * \see RBRGen4_resetPowerExternalUsed()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828551/external
 */
RBRGen4Error RBRGen4_setPowerExternalBatteryType(
    RBRGen4 *instrument,
    const RBRGen4ExternalBatteryType type);

/**
 * \brief Reset the counter of energy used from the external battery.
 * \note Issues the `instrument power external` instrument command.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRGEN4_SUCCESS when the setting is successfully written
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument is logging
 * \see RBRGen4_getPowerExternal()
 * \see RBRGen4_setPowerExternalBatteryType()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828551/external
 */
RBRGen4Error RBRGen4_resetPowerExternalUsed(
    RBRGen4 *instrument);

/**
 * \brief Instrument `instrument` command parameters.
 *
 * Distinct from #RBRGen4, which is the connection to an instrument.
 *
 * Fields are declared in the order the instrument reports them.
 *
 * \see RBRGen4_getInstrument()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/41582593/instrument
 */
typedef struct RBRGen4Instrument
{
    /** \brief Whether a deployment is currently logging. */
    RBRGen4InstrumentState state;
    /** \brief The serial number of the instrument. */
    int32_t sn;
    /** \brief The instrument model. */
    char model[RBRGEN4_ID_MODEL_MAX + 1];
    /** \brief The RBR part number of the instrument. */
    char pn[RBRGEN4_PART_NUMBER_MAX + 1];
    /** \brief The instrument firmware version. */
    char fwversion[RBRGEN4_ID_VERSION_MAX + 1];
    /**
     * \brief The instrument firmware version in Semantic Version form.
     *
     * For example, `2.0.0-rc1-10-g148bc5eb1`.
     */
    char semver[RBRGEN4_ID_SEMVER_MAX + 1];
    /** \brief The firmware type of the instrument. */
    int32_t fwtype;
    /** \brief Whether firmware upgrades are locked. */
    bool fwLock;
    /** \brief The data type used by the instrument's samples. */
    RBRGen4DataType dataType;
    /**
     * \brief The extended model name of the instrument.
     *
     * For example, `RBRsolo^4_T.D!fast32`.
     */
    char name[RBRGEN4_INSTRUMENT_NAME_MAX + 1];
} RBRGen4Instrument;

/**
 * \brief Get the instrument's identity and state.
 * \note Issues the `instrument` instrument command.
 *
 * All of the parameters the command reports are returned. They are read-only,
 * so there is no corresponding setter.
 *
 * \param [in] instrument the instrument connection
 * \param [out] instrumentInfo the instrument information
 * \return #RBRGEN4_SUCCESS when the information is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRGen4_getId4()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/41582593/instrument
 */
RBRGen4Error RBRGen4_getInstrument(
    RBRGen4 *instrument,
    RBRGen4Instrument *instrumentInfo);

/**
 * \brief Get the current output format.
 * \note Issues the `instrument outputformat` command.
 *
 * On success, the library caches the output format and uses it to parse
 * subsequently received samples.
 *
 * \param [in] instrument the instrument connection
 * \param [out] outputformat the current output format
 * \return #RBRGEN4_SUCCESS when the settings are successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRGen4_setOutputFormat()
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 */
RBRGen4Error RBRGen4_getOutputFormat(
    RBRGen4 *instrument,
    RBRGen4OutputFormat *outputformat);

/**
 * \brief Set the current output format.
 * \note Issues the `instrument outputformat` command.
 *
 * Every parameter of the command is sent, so \a outputformat must be fully
 * populated: read the current format with RBRGen4_getOutputFormat()
 * and modify it if only some parameters are of interest.
 *
 * \warning RBRParserGen4 reads only #RBRGEN4_ENCODING_ASCII.
 *          Selecting #RBRGEN4_ENCODING_BINARY will stop this library
 *          from being able to interpret samples or command responses.
 *
 * On success, the library caches the output format and uses it to parse
 * subsequently received samples. On failure the cache is left unchanged and
 * may no longer match the instrument. Samples received while the cache is
 * stale are usually refused, but some mismatches go undetected and yield a
 * wrongly accepted sample, so call RBRGen4_getOutputFormat()
 * before relying on parsed samples again.
 *
 * \param [in] instrument the instrument connection
 * \param [in] outputformat the desired output format
 * \return #RBRGEN4_SUCCESS when the settings are successfully written
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the encoding or
 *                                                   datatype is not a real
 *                                                   value
 * \return #RBRGEN4_HARDWARE_ERROR if the instrument refuses a value
 * \see RBRGen4_getOutputFormat()
 * \see https://docs.rbr-global.com/L3commandreference/commands/real-time-data/outputformat
 */
RBRGen4Error RBRGen4_setOutputFormat(
    RBRGen4 *instrument,
    const RBRGen4OutputFormat *outputformat);

/**
 * \brief Return the instrument's configuration to its factory state.
 * \note Issues the `instrument factory reset` command.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRGEN4_SUCCESS when the instrument has been reset
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR if the instrument refuses
 * \see https://docs.rbr-global.com/L3commandreference/commands/configuration-information-and-calibration/factory
 */
RBRGen4Error RBRGen4_factoryReset(
    RBRGen4 *instrument);

/**
 * \brief Reset the instrument CPU.
 * \note Issues the `instrument reboot` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] delay time in milliseconds to wait before rebooting; zero omits
 *                   the parameter, rebooting without a delay. The command
 *                   has no default delay of its own.
 * \return #RBRGEN4_SUCCESS when the reboot has been requested
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/security-and-interaction/reboot
 */
RBRGen4Error RBRGen4_reboot(RBRGen4 *instrument,
                                        const int32_t delay);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN4INSTRUMENT_H */
