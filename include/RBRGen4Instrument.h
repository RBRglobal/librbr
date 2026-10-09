/*
 * Copyright (c) 2024 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file RBRGen4Instrument.h
 *
 * \brief Instrument commands and structures for miscellaneous commands.
 */

#ifndef LIBRBR_RBRGEN4INSTRUMENT_H
#define LIBRBR_RBRGEN4INSTRUMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RBRGen4.h"

/**
 * \brief Possible instrument states.
 * This tracks whether the deployment is running on the instrument.
 *
 * \see RBRGen4_getInstrument()
 * \see RBRGen4_enable()
 * \see RBRGen4_verify()
 * \see RBRGen4DeploymentStatus
 * \see RBRGen4Deployment
 */
typedef enum RBRGen4InstrumentState {
    /** Logging is not enabled. */
    RBRGEN4_INSTRUMENT_STATE_DISABLED,
    /** Logging for at least one deployment is enabled. */
    RBRGEN4_INSTRUMENT_STATE_ENABLED,
    /** The number of specific instrument states. */
    RBRGEN4_INSTRUMENT_STATE_COUNT,
    /** An unknown or unrecognized instrument state. */
    RBRGEN4_UNKNOWN_INSTRUMENT_STATE,
} RBRGen4InstrumentState;

/**
 * \brief Get a human-readable string name for a instrument state.
 *
 * \param [in] state the instrument state
 * \return a string name for the instrument state
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4InstrumentState_name(RBRGen4InstrumentState state);

/**
 * \brief The maximum number of characters in the instrument name.
 *
 * Does not include any null terminator.
 */
#define RBRGEN4_INSTRUMENT_NAME_MAX 32

/**
 * \brief Get identification information from the instrument.
 *
 * \command{id4}
 *
 * \param [in] conn the instrument connection
 * \param [out] id the instrument information
 * \return #RBRGEN4_SUCCESS when the information is successfully read
 * \return #RBRGEN4_COMMAND_TOO_LONG when the command does not fit the command buffer
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument reports a hardware error
 * \see RBRGen4_getInstrument()
 */
RBRGen4Error RBRGen4_getId4(RBRGen4 *conn, RBRGen4Id4 *id);

/**
 * \brief Possible instrument power sources.
 *
 * \see RBRGen4_getPowerSource()
 */
typedef enum RBRGen4PowerSource {
    /** USB power. */
    RBRGEN4_POWER_SOURCE_USB,
    /** Internal (battery) power. */
    RBRGEN4_POWER_SOURCE_INTERNAL,
    /** External power. */
    RBRGEN4_POWER_SOURCE_EXTERNAL,
    /** The number of specific power sources. */
    RBRGEN4_POWER_SOURCE_COUNT,
    /** An unknown or unrecognized power source. */
    RBRGEN4_UNKNOWN_POWER_SOURCE,
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
 *
 * \command{instrument power}
 *
 * \param [in] conn the instrument connection
 * \param [out] powerSource the power source from which the instrument is running
 * \return #RBRGEN4_SUCCESS when the information is successfully read
 * \return #RBRGEN4_COMMAND_TOO_LONG when the command does not fit the command buffer
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the information cannot be read, or another hardware error
 *         occurs
 * \see RBRGen4_getPowerInternal()
 * \see RBRGen4_getPowerExternal()
 */
RBRGen4Error RBRGen4_getPowerSource(RBRGen4 *conn, RBRGen4PowerSource *powerSource);

/**
 * \brief Internal battery types.
 *
 * \see RBRGen4PowerInternal
 */
typedef enum RBRGen4InternalBatteryType {
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
    RBRGEN4_UNKNOWN_INTERNAL_BATTERY,
} RBRGen4InternalBatteryType;

/**
 * \brief Get a human-readable string name for an internal battery type.
 *
 * \param [in] type the battery type
 * \return a string name for the battery type
 * \see RBRGen4Error_name() for a description of the format of names
 * \see RBRGen4InternalBatteryType_displayName() for display names
 */
const char *RBRGen4InternalBatteryType_name(RBRGen4InternalBatteryType type);

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
const char *RBRGen4InternalBatteryType_displayName(RBRGen4InternalBatteryType type);

/**
 * \brief Instrument `instrument power internal` command parameters.
 *
 * \see RBRGen4_getPowerInternal()
 * \see RBRGen4_setPowerInternalBatteryType()
 * \see RBRGen4_resetPowerInternalUsed()
 */
typedef struct RBRGen4PowerInternal {
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
 *
 * \command{instrument power internal}
 *
 * \param [in] conn the instrument connection
 * \param [out] power the power information
 * \return #RBRGEN4_SUCCESS when the information is successfully read
 * \return #RBRGEN4_COMMAND_TOO_LONG when the command does not fit the command buffer
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when an error occurs reading voltages, or another hardware error
 *         occurs
 * \see RBRGen4_setPowerInternalBatteryType()
 * \see RBRGen4_resetPowerInternalUsed()
 */
RBRGen4Error RBRGen4_getPowerInternal(RBRGen4 *conn, RBRGen4PowerInternal *power);

/**
 * \brief Set the internal power battery type.
 *
 * \command{instrument power internal}
 *
 * \param [in] conn the instrument connection
 * \param [in] type the battery type
 * \return #RBRGEN4_SUCCESS when the setting is successfully written
 * \return #RBRGEN4_COMMAND_TOO_LONG when the command does not fit the command buffer
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument is logging, or another hardware error occurs
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the battery type is unknown
 * \see RBRGen4_getPowerInternal()
 * \see RBRGen4_resetPowerInternalUsed()
 */
RBRGen4Error RBRGen4_setPowerInternalBatteryType(RBRGen4 *conn,
                                                 const RBRGen4InternalBatteryType type);

/**
 * \brief Reset the counter of energy used from the internal battery.
 *
 * \command{instrument power internal}
 *
 * \param [in] conn the instrument connection
 * \return #RBRGEN4_SUCCESS when the setting is successfully written
 * \return #RBRGEN4_COMMAND_TOO_LONG when the command does not fit the command buffer
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument is logging, or another hardware error occurs
 * \see RBRGen4_getPowerInternal()
 * \see RBRGen4_setPowerInternalBatteryType()
 */
RBRGen4Error RBRGen4_resetPowerInternalUsed(RBRGen4 *conn);

/**
 * \brief External battery types.
 *
 * \see RBRGen4PowerExternal
 */
typedef enum RBRGen4ExternalBatteryType {
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
    /** RBRfermata NiMH */
    RBRGEN4_EXTERNAL_BATTERY_FERMATA_NIMH,
    /** Other/unknown external battery type */
    RBRGEN4_EXTERNAL_BATTERY_OTHER,
    /** The number of specific external battery types. */
    RBRGEN4_EXTERNAL_BATTERY_COUNT,
    /** An unknown or unrecognized external battery type. */
    RBRGEN4_UNKNOWN_EXTERNAL_BATTERY,
} RBRGen4ExternalBatteryType;

/**
 * \brief Get a human-readable string name for an external battery type.
 *
 * \param [in] type the battery type
 * \return a string name for the battery type
 * \see RBRGen4Error_name() for a description of the format of names
 * \see RBRGen4ExternalBatteryType_displayName() for display names
 */
const char *RBRGen4ExternalBatteryType_name(RBRGen4ExternalBatteryType type);

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
const char *RBRGen4ExternalBatteryType_displayName(RBRGen4ExternalBatteryType type);

/**
 * \brief Instrument `instrument power external` command parameters.
 *
 * \see RBRGen4_getPowerExternal()
 * \see RBRGen4_setPowerExternalBatteryType()
 * \see RBRGen4_resetPowerExternalUsed()
 */
typedef struct RBRGen4PowerExternal {
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
 * \command{instrument power external}
 *
 * \param [in] conn the instrument connection
 * \param [out] power the power information
 * \return #RBRGEN4_SUCCESS when the information is successfully read
 * \return #RBRGEN4_COMMAND_TOO_LONG when the command does not fit the command buffer
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument reports a hardware error
 * \see RBRGen4_setPowerExternalBatteryType()
 * \see RBRGen4_resetPowerExternalUsed()
 */
RBRGen4Error RBRGen4_getPowerExternal(RBRGen4 *conn, RBRGen4PowerExternal *power);

/**
 * \brief Set the external power battery type.
 *
 * \command{instrument power external}
 *
 * \param [in] conn the instrument connection
 * \param [in] type the battery type
 * \return #RBRGEN4_SUCCESS when the setting is successfully written
 * \return #RBRGEN4_COMMAND_TOO_LONG when the command does not fit the command buffer
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument is logging, or another hardware error occurs
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the battery type is unknown
 * \see RBRGen4_getPowerExternal()
 * \see RBRGen4_resetPowerExternalUsed()
 */
RBRGen4Error RBRGen4_setPowerExternalBatteryType(RBRGen4 *conn,
                                                 const RBRGen4ExternalBatteryType type);

/**
 * \brief Reset the counter of energy used from the external battery.
 *
 * \command{instrument power external}
 *
 * \param [in] conn the instrument connection
 * \return #RBRGEN4_SUCCESS when the setting is successfully written
 * \return #RBRGEN4_COMMAND_TOO_LONG when the command does not fit the command buffer
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument is logging, or another hardware error occurs
 * \see RBRGen4_getPowerExternal()
 * \see RBRGen4_setPowerExternalBatteryType()
 */
RBRGen4Error RBRGen4_resetPowerExternalUsed(RBRGen4 *conn);

/**
 * \brief Instrument `instrument` command parameters.
 *
 * Distinct from #RBRGen4, which is the connection to an instrument.
 *
 * \see RBRGen4_getInstrument()
 */
typedef struct RBRGen4Instrument {
    /** \brief Whether a deployment is currently logging. */
    RBRGen4InstrumentState state;
    /** \brief The serial number of the instrument. */
    int32_t sn;
    /** \brief The instrument model. */
    char model[RBRGEN4_ID_MODEL_MAX + 1];
    /** \brief The RBR part number of the instrument. */
    char pn[RBRGEN4_PART_NUMBER_MAX + 1];
    /** \brief The instrument firmware version. */
    char fwVersion[RBRGEN4_ID_VERSION_MAX + 1];
    /**
     * \brief The instrument firmware version in Semantic Version form.
     *
     * For example, `2.0.0-rc3-14-g5e07a2c91`.
     */
    char semver[RBRGEN4_ID_SEMVER_MAX + 1];
    /** \brief The firmware type of the instrument. */
    int32_t fwType;
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
    /**
     * \brief The version of the Gen4 command API implemented by the firmware.
     *
     * For example, `2.1`. Empty when the parameter is not reported.
     */
    char apiVersion[RBRGEN4_ID_API_VERSION_MAX + 1];
} RBRGen4Instrument;

/**
 * \brief Get the instrument's identity and state.
 *
 * \command{instrument}
 *
 * All of the parameters the command reports are returned. They are read-only,
 * so there is no corresponding setter.
 *
 * \param [in] conn the instrument connection
 * \param [out] instrumentInfo the instrument information
 * \return #RBRGEN4_SUCCESS when the information is successfully read
 * \return #RBRGEN4_COMMAND_TOO_LONG when the command does not fit the command buffer
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument reports a hardware error
 * \see RBRGen4_getId4()
 */
RBRGen4Error RBRGen4_getInstrument(RBRGen4 *conn, RBRGen4Instrument *instrumentInfo);

/**
 * \brief Get the current output format.
 *
 * \command{instrument outputformat}
 *
 * On success, the library caches the output format and uses it to parse
 * subsequently received samples.
 *
 * \param [in] conn the instrument connection
 * \param [out] outputFormat the current output format
 * \return #RBRGEN4_SUCCESS when the settings are successfully read
 * \return #RBRGEN4_COMMAND_TOO_LONG when the command does not fit the command buffer
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument reports a hardware error
 * \see RBRGen4_setOutputFormat()
 */
RBRGen4Error RBRGen4_getOutputFormat(RBRGen4 *conn, RBRGen4OutputFormat *outputFormat);

/**
 * \brief Set the current output format.
 *
 * \command{instrument outputformat}
 *
 * Every parameter of the command is sent, so \a outputFormat must be fully
 * populated: read the current format with RBRGen4_getOutputFormat()
 * and modify it if only some parameters are of interest.
 *
 * On success, the library caches the output format and uses it to parse
 * subsequently received samples. On failure the cache is left unchanged and
 * may no longer match the instrument. Samples received while the cache is
 * stale are usually refused, but some mismatches go undetected and yield a
 * wrongly accepted sample, so call RBRGen4_getOutputFormat()
 * before relying on parsed samples again.
 *
 * \param [in] conn the instrument connection
 * \param [in] outputFormat the desired output format
 * \return #RBRGEN4_SUCCESS when the settings are successfully written
 * \return #RBRGEN4_COMMAND_TOO_LONG when the command does not fit the command buffer
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument refuses a value, or another hardware error
 *         occurs
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the datatype is not a real value
 * \see RBRGen4_getOutputFormat()
 */
RBRGen4Error RBRGen4_setOutputFormat(RBRGen4 *conn, const RBRGen4OutputFormat *outputFormat);

/**
 * \brief Return the instrument's configuration to its factory state.
 *
 * \command{instrument factory reset}
 *
 * \param [in] conn the instrument connection
 * \return #RBRGEN4_SUCCESS when the instrument has been reset
 * \return #RBRGEN4_COMMAND_TOO_LONG when the command does not fit the command buffer
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument refuses, or another hardware error occurs
 */
RBRGen4Error RBRGen4_factoryReset(RBRGen4 *conn);

/**
 * \brief Reset the instrument CPU.
 *
 * \command{instrument reboot}
 *
 * \param [in] conn the instrument connection
 * \param [in] delay time in milliseconds to wait before rebooting; zero omits the parameter,
 *                   rebooting without a delay
 * \return #RBRGEN4_SUCCESS when the reboot has been requested
 * \return #RBRGEN4_COMMAND_TOO_LONG when the command does not fit the command buffer
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 */
RBRGen4Error RBRGen4_reboot(RBRGen4 *conn, const int32_t delay);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN4INSTRUMENT_H */
