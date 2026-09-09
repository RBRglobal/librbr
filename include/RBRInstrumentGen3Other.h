/**
 * \file RBRInstrumentGen3Other.h
 *
 * \brief Instrument commands and structures for miscellaneous commands.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTGEN3OTHER_H
#define LIBRBR_RBRINSTRUMENTGEN3OTHER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief The maximum number of characters in the type and revision of the CPU.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENTGEN3_HWREV_CPU_MAX 5

/**
 * \brief The maximum number of characters in an instrument part number.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENTGEN3_PART_NUMBER_MAX 255

/**
 * \brief Compare two firmware version strings.
 *
 * If either string is not a version string (format XXSYYY) then the result
 * will indicate that it is the lesser version. If neither is valid, then the
 * result will indicate equality.
 *
 * \param a the first firmware version as a null-terminated C string
 * \param b the second firmware version as a null-terminated C string
 * \return <0 if \a a is a lower version than \a b
 * \return 0 \a a and \a b are the same version
 * \return >0 if \a b is a lower version than \a b
 */
int RBRInstrumentGen3Version_compare(const char *a, const char *b);

/**
 * \brief Get identification information from the instrument.
 *
 * \param [in] instrument the instrument connection
 * \param [out] id the instrument information
 * \return #RBRGEN3_SUCCESS when the information is successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/id
 */
RBRGen3Error RBRInstrumentGen3_getId(RBRGen3 *instrument,
                                       RBRGen3Id *id);

/**
 * \brief Instrument `hwrev` command parameters.
 *
 * \see RBRInstrumentGen3_getHardwareRevision()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/hwrev
 */
typedef struct RBRInstrumentGen3HardwareRevision
{
    /** The revision of the CPU PCB. */
    char pcb;
    /** The part number and revision of the CPU. */
    char cpu[RBRINSTRUMENTGEN3_HWREV_CPU_MAX + 1];
    /** The revision of the CPU boot loader. */
    char bsl;
} RBRInstrumentGen3HardwareRevision;

/**
 * \brief Get instrument hardware revision information.
 *
 * \param [in] instrument the instrument connection
 * \param [out] hwrev the hardware revision information
 * \return #RBRGEN3_SUCCESS when the information is successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/hwrev
 */
RBRGen3Error RBRInstrumentGen3_getHardwareRevision(
    RBRGen3 *instrument,
    RBRInstrumentGen3HardwareRevision *hwrev);

/**
 * brief Possible instrument power sources.
 *
 * \see RBRInstrumentGen3Power
 */
typedef enum RBRInstrumentGen3PowerSource
{
    /** USB power. */
    RBRINSTRUMENTGEN3_POWER_SOURCE_USB,
    /** Internal (battery) power. */
    RBRINSTRUMENTGEN3_POWER_SOURCE_INTERNAL,
    /** External power. */
    RBRINSTRUMENTGEN3_POWER_SOURCE_EXTERNAL,
    /** The number of specific power sources. */
    RBRINSTRUMENTGEN3_POWER_SOURCE_COUNT,
    /** An unknown or unrecognized power source. */
    RBRINSTRUMENTGEN3_UNKNOWN_POWER_SOURCE
} RBRInstrumentGen3PowerSource;

/**
 * \brief Get a human-readable string name for a power source.
 *
 * \param [in] source the power source
 * \return a string name for the power source
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen3PowerSource_name(RBRInstrumentGen3PowerSource source);

/**
 * \brief Instrument `power` command parameters.
 *
 * \see RBRInstrumentGen3_getPower()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/power
 */
typedef struct RBRInstrumentGen3Power
{
    /** \brief The power source from which the instrument is running. */
    RBRInstrumentGen3PowerSource source;
    /**
     * \brief The measured voltage of a standard logger's internal battery.
     *
     * NAN for a short logger.
     */
    float internal;
    /** \brief The measured voltage of any external power source. */
    float external;
    /**
     * \brief The measured voltage of a short logger's internal voltage
     * regulator.
     *
     * NAN for a standard logger.
     */
    float regulator;
} RBRInstrumentGen3Power;

/**
 * \brief Get instrument power information.
 *
 * \param [in] instrument the instrument connection
 * \param [out] power the power information
 * \return #RBRGEN3_SUCCESS when the information is successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR if an error occurs reading voltages
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/power
 */
RBRGen3Error RBRInstrumentGen3_getPower(RBRGen3 *instrument,
                                          RBRInstrumentGen3Power *power);

/**
 * Internal battery types.
 *
 * \see RBRInstrumentGen3PowerInternal
 */
typedef enum RBRInstrumentGen3InternalBatteryType
{
    /** No internal battery */
    RBRINSTRUMENTGEN3_INTERNAL_BATTERY_NONE,
    /** Li-SOCl₂ */
    RBRINSTRUMENTGEN3_INTERNAL_BATTERY_LISOCL2,
    /** Li-FeS₂ */
    RBRINSTRUMENTGEN3_INTERNAL_BATTERY_LIFES2,
    /** Zn-MnO₂ */
    RBRINSTRUMENTGEN3_INTERNAL_BATTERY_ZNMNO2,
    /** Li-NiMnCo */
    RBRINSTRUMENTGEN3_INTERNAL_BATTERY_LINIMNCO,
    /** NiMH */
    RBRINSTRUMENTGEN3_INTERNAL_BATTERY_NIMH,
    /** The number of specific internal battery types. */
    RBRINSTRUMENTGEN3_INTERNAL_BATTERY_COUNT,
    /** An unknown or unrecognized internal battery type. */
    RBRINSTRUMENTGEN3_UNKNOWN_INTERNAL_BATTERY
} RBRInstrumentGen3InternalBatteryType;

/**
 * \brief Get a human-readable string name for an internal battery type.
 *
 * \param [in] type the battery type
 * \return a string name for the battery type
 * \see RBRGen3Error_name() for a description of the format of names
 * \see RBRInstrumentGen3InternalBatteryType_displayName() for display names
 */
const char *RBRInstrumentGen3InternalBatteryType_name(
    RBRInstrumentGen3InternalBatteryType type);

/**
 * \brief Get a human-readable display name for an internal battery type.
 *
 * Unlike the values returned by RBRInstrumentGen3InternalBatteryType_name(),
 * the names of battery types will be formatted appropriately for the cell
 * chemistry; e.g., “Li-SOCl₂”, not “lisocl2”. Values will be UTF-8-encoded.
 *
 * \param [in] type the battery type
 * \return a string name for the battery type
 * \see RBRInstrumentGen3InternalBatteryType_name() for instrument-equivalent names
 */
const char *RBRInstrumentGen3InternalBatteryType_displayName(
    RBRInstrumentGen3InternalBatteryType type);

/**
 * \brief Instrument `powerinternal` command parameters.
 *
 * \see RBRInstrumentGen3_getPowerInternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerinternal
 */
typedef struct RBRInstrumentGen3PowerInternal
{
    /** \brief The type of battery. */
    RBRInstrumentGen3InternalBatteryType batteryType;
    /**
     * \brief The capacity of the battery.
     *
     * \readonly
     */
    const float capacity;
    /**
     * \brief The accumulated energy used from the internal battery since the
     * value was last reset.
     */
    float used;
} RBRInstrumentGen3PowerInternal;

/**
 * \brief Get instrument internal power information.
 *
 * \nol2 Always returns #RBRGEN3_UNSUPPORTED.
 *
 * \param [in] instrument the instrument connection
 * \param [out] power the power information
 * \return #RBRGEN3_SUCCESS when the information is successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen3_setPowerInternalBatteryType()
 * \see RBRInstrumentGen3_resetPowerInternalUsed()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerinternal
 */
RBRGen3Error RBRInstrumentGen3_getPowerInternal(
    RBRGen3 *instrument,
    RBRInstrumentGen3PowerInternal *power);

/**
 * \brief Set the internal power battery type.
 *
 * \nol2 Always returns #RBRGEN3_UNSUPPORTED.
 *
 * \param [in] instrument the instrument connection
 * \param [in] type the battery type
 * \return #RBRGEN3_SUCCESS when the setting is successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the instrument is logging
 * \see RBRInstrumentGen3_getPowerInternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerinternal
 */
RBRGen3Error RBRInstrumentGen3_setPowerInternalBatteryType(
    RBRGen3 *instrument,
    RBRInstrumentGen3InternalBatteryType type);

/**
 * \brief Reset the counter of energy used from the internal battery.
 *
 * \nol2 Always returns #RBRGEN3_UNSUPPORTED.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRGEN3_SUCCESS when the setting is successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the instrument is logging
 * \see RBRInstrumentGen3_getPowerInternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerinternal
 */
RBRGen3Error RBRInstrumentGen3_resetPowerInternalUsed(
    RBRGen3 *instrument);

/**
 * External battery types.
 *
 * \see RBRInstrumentGen3PowerExternal
 */
typedef enum RBRInstrumentGen3ExternalBatteryType
{
    /** Other/unknown external battery type */
    RBRINSTRUMENTGEN3_EXTERNAL_BATTERY_OTHER,
    /** RBRfermata Li-SOCl₂ */
    RBRINSTRUMENTGEN3_EXTERNAL_BATTERY_FERMATA_LISOCL2,
    /** RBRfermata Zn-MnO₂ */
    RBRINSTRUMENTGEN3_EXTERNAL_BATTERY_FERMATA_ZNMNO2,
    /** RBRfermette Li-MnO₂ */
    RBRINSTRUMENTGEN3_EXTERNAL_BATTERY_FERMETTE_LIMNO2,
    /** RBRfermette³ Li-SOCl₂ */
    RBRINSTRUMENTGEN3_EXTERNAL_BATTERY_FERMETTE3_LISOCL2,
    /** RBRfermette³ Li-FeS₂ */
    RBRINSTRUMENTGEN3_EXTERNAL_BATTERY_FERMETTE3_LIFES2,
    /** RBRfermette³ Zn-MnO₂ */
    RBRINSTRUMENTGEN3_EXTERNAL_BATTERY_FERMETTE3_ZNMNO2,
    /** RBRfermette³ Li-NiMnCo */
    RBRINSTRUMENTGEN3_EXTERNAL_BATTERY_FERMETTE3_LINIMNCO,
    /** RBRfermette³ NiMH */
    RBRINSTRUMENTGEN3_EXTERNAL_BATTERY_FERMETTE3_NIMH,
    /** The number of specific external battery types. */
    RBRINSTRUMENTGEN3_EXTERNAL_BATTERY_COUNT,
    /** An unknown or unrecognized external battery type. */
    RBRINSTRUMENTGEN3_UNKNOWN_EXTERNAL_BATTERY
} RBRInstrumentGen3ExternalBatteryType;

/**
 * \brief Get a human-readable string name for an external battery type.
 *
 * \param [in] type the battery type
 * \return a string name for the battery type
 * \see RBRGen3Error_name() for a description of the format of names
 * \see RBRInstrumentGen3ExternalBatteryType_displayName() for display names
 */
const char *RBRInstrumentGen3ExternalBatteryType_name(
    RBRInstrumentGen3ExternalBatteryType type);

/**
 * \brief Get a human-readable display name for an external battery type.
 *
 * Unlike the values returned by RBRInstrumentGen3ExternalBatteryType_name(),
 * RBRfermata/RBRfermette product names will be correctly capitalized, and the
 * names of battery types will be formatted appropriately for the cell
 * chemistry; e.g., “RBRfermette³ Li-SOCl₂”, not “rbrfermette3 lisocl2”. Values
 * will be UTF-8-encoded.
 *
 * \param [in] type the battery type
 * \return a string name for the battery type
 * \see RBRInstrumentGen3ExternalBatteryType_name() for instrument-equivalent names
 */
const char *RBRInstrumentGen3ExternalBatteryType_displayName(
    RBRInstrumentGen3ExternalBatteryType type);

/**
 * \brief Instrument `powerexternal` command parameters.
 *
 * \see RBRInstrumentGen3_getPowerExternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerexternal
 */
typedef struct RBRInstrumentGen3PowerExternal
{
    /** \brief The type of battery. */
    RBRInstrumentGen3ExternalBatteryType batteryType;
    /**
     * \brief The capacity of the battery.
     *
     * \readonly
     */
    const float capacity;
    /**
     * \brief The accumulated energy used from the external battery since the
     * value was last reset.
     */
    float used;
} RBRInstrumentGen3PowerExternal;

/**
 * \brief Get instrument external power information.
 *
 * \nol2 Always returns #RBRGEN3_UNSUPPORTED.
 *
 * \param [in] instrument the instrument connection
 * \param [out] power the power information
 * \return #RBRGEN3_SUCCESS when the information is successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen3_setPowerExternalBatteryType()
 * \see RBRInstrumentGen3_resetPowerExternalUsed()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerexternal
 */
RBRGen3Error RBRInstrumentGen3_getPowerExternal(
    RBRGen3 *instrument,
    RBRInstrumentGen3PowerExternal *power);

/**
 * \brief Set the external power battery type.
 *
 * \nol2 Always returns #RBRGEN3_UNSUPPORTED.
 *
 * \param [in] instrument the instrument connection
 * \param [in] type the battery type
 * \return #RBRGEN3_SUCCESS when the setting is successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen3_getPowerExternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerexternal
 */
RBRGen3Error RBRInstrumentGen3_setPowerExternalBatteryType(
    RBRGen3 *instrument,
    RBRInstrumentGen3ExternalBatteryType type);

/**
 * \brief Reset the counter of energy used from the external battery.
 *
 * \nol2 Always returns #RBRGEN3_UNSUPPORTED.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRGEN3_SUCCESS when the setting is successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen3_getPowerExternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerexternal
 */
RBRGen3Error RBRInstrumentGen3_resetPowerExternalUsed(
    RBRGen3 *instrument);

/**
 * \brief Instrument `info` command parameters.
 *
 * \see RBRInstrumentGen3_getInfo()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/info
 */
typedef struct RBRInstrumentGen3Info
{
    /** The RBR part number of the instrument. */
    char partNumber[RBRINSTRUMENTGEN3_PART_NUMBER_MAX + 1];
    /** Whether firmware upgrades are locked. */
    bool fwLock;
} RBRInstrumentGen3Info;

/**
 * \brief Get more information about the instrument.
 *
 * \nol2
 *
 * \param [in] instrument the instrument connection
 * \param [out] info the extended instrument information
 * \return #RBRGEN3_UNSUPPORTED for Logger2 instruments
 * \return #RBRGEN3_SUCCESS when the information is successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/info
 */
RBRGen3Error RBRInstrumentGen3_getInfo(
    RBRGen3 *instrument,
    RBRInstrumentGen3Info *info);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTGEN3OTHER_H */
