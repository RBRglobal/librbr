/**
 * \file RBRInstrumentOther.h
 *
 * \brief Instrument commands and structures for miscellaneous commands.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTOTHER_H
#define LIBRBR_RBRINSTRUMENTOTHER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief The maximum number of characters in the type and revision of the CPU.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENT_HWREV_CPU_MAX 5

/**
 * \brief The maximum number of characters in an instrument part number.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENT_PART_NUMBER_MAX 255

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
int RBRInstrumentVersion_compare(const char *a, const char *b);

/**
 * \brief Get identification information from the instrument.
 *
 * \param [in] instrument the instrument connection
 * \param [out] id the instrument information
 * \return #RBRINSTRUMENT_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/id
 */
RBRInstrumentError RBRInstrument_getId(RBRInstrument *instrument,
                                       RBRInstrumentId *id);

/**
 * \brief Instrument `hwrev` command parameters.
 *
 * \see RBRInstrument_getHardwareRevision()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/hwrev
 */
typedef struct RBRInstrumentHardwareRevision
{
    /** The revision of the CPU PCB. */
    char pcb;
    /** The part number and revision of the CPU. */
    char cpu[RBRINSTRUMENT_HWREV_CPU_MAX + 1];
    /** The revision of the CPU boot loader. */
    char bsl;
} RBRInstrumentHardwareRevision;

/**
 * \brief Get instrument hardware revision information.
 *
 * \param [in] instrument the instrument connection
 * \param [out] hwrev the hardware revision information
 * \return #RBRINSTRUMENT_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/hwrev
 */
RBRInstrumentError RBRInstrument_getHardwareRevision(
    RBRInstrument *instrument,
    RBRInstrumentHardwareRevision *hwrev);

/**
 * brief Possible instrument power sources.
 *
 * \see RBRInstrumentPower
 */
typedef enum RBRInstrumentPowerSource
{
    /** USB power. */
    RBRINSTRUMENT_POWER_SOURCE_USB,
    /** Internal (battery) power. */
    RBRINSTRUMENT_POWER_SOURCE_INTERNAL,
    /** External power. */
    RBRINSTRUMENT_POWER_SOURCE_EXTERNAL,
    /** The number of specific power sources. */
    RBRINSTRUMENT_POWER_SOURCE_COUNT,
    /** An unknown or unrecognized power source. */
    RBRINSTRUMENT_UNKNOWN_POWER_SOURCE
} RBRInstrumentPowerSource;

/**
 * \brief Get a human-readable string name for a power source.
 *
 * \param [in] source the power source
 * \return a string name for the power source
 * \see RBRInstrumentError_name() for a description of the format of names
 */
const char *RBRInstrumentPowerSource_name(RBRInstrumentPowerSource source);

/**
 * \brief Instrument `power` command parameters.
 *
 * \see RBRInstrument_getPower()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/power
 */
typedef struct RBRInstrumentPower
{
    /** \brief The power source from which the instrument is running. */
    RBRInstrumentPowerSource source;
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
} RBRInstrumentPower;

/**
 * \brief Get instrument power information.
 *
 * \param [in] instrument the instrument connection
 * \param [out] power the power information
 * \return #RBRINSTRUMENT_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if an error occurs reading voltages
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/power
 */
RBRInstrumentError RBRInstrument_getPower(RBRInstrument *instrument,
                                          RBRInstrumentPower *power);

/**
 * Internal battery types.
 *
 * \see RBRInstrumentPowerInternal
 */
typedef enum RBRInstrumentInternalBatteryType
{
    /** No internal battery */
    RBRINSTRUMENT_INTERNAL_BATTERY_NONE,
    /** Li-SOCl₂ */
    RBRINSTRUMENT_INTERNAL_BATTERY_LISOCL2,
    /** Li-FeS₂ */
    RBRINSTRUMENT_INTERNAL_BATTERY_LIFES2,
    /** Zn-MnO₂ */
    RBRINSTRUMENT_INTERNAL_BATTERY_ZNMNO2,
    /** Li-NiMnCo */
    RBRINSTRUMENT_INTERNAL_BATTERY_LINIMNCO,
    /** NiMH */
    RBRINSTRUMENT_INTERNAL_BATTERY_NIMH,
    /** The number of specific internal battery types. */
    RBRINSTRUMENT_INTERNAL_BATTERY_COUNT,
    /** An unknown or unrecognized internal battery type. */
    RBRINSTRUMENT_UNKNOWN_INTERNAL_BATTERY
} RBRInstrumentInternalBatteryType;

/**
 * \brief Get a human-readable string name for an internal battery type.
 *
 * \param [in] type the battery type
 * \return a string name for the battery type
 * \see RBRInstrumentError_name() for a description of the format of names
 * \see RBRInstrumentInternalBatteryType_displayName() for display names
 */
const char *RBRInstrumentInternalBatteryType_name(
    RBRInstrumentInternalBatteryType type);

/**
 * \brief Get a human-readable display name for an internal battery type.
 *
 * Unlike the values returned by RBRInstrumentInternalBatteryType_name(),
 * the names of battery types will be formatted appropriately for the cell
 * chemistry; e.g., “Li-SOCl₂”, not “lisocl2”. Values will be UTF-8-encoded.
 *
 * \param [in] type the battery type
 * \return a string name for the battery type
 * \see RBRInstrumentInternalBatteryType_name() for instrument-equivalent names
 */
const char *RBRInstrumentInternalBatteryType_displayName(
    RBRInstrumentInternalBatteryType type);

/**
 * \brief Instrument `powerinternal` command parameters.
 *
 * \see RBRInstrument_getPowerInternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerinternal
 */
typedef struct RBRInstrumentPowerInternal
{
    /** \brief The type of battery. */
    RBRInstrumentInternalBatteryType batteryType;
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
} RBRInstrumentPowerInternal;

/**
 * \brief Get instrument internal power information.
 *
 * \nol2 Always returns #RBRINSTRUMENT_UNSUPPORTED.
 *
 * \param [in] instrument the instrument connection
 * \param [out] power the power information
 * \return #RBRINSTRUMENT_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_setPowerInternalBatteryType()
 * \see RBRInstrument_resetPowerInternalUsed()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerinternal
 */
RBRInstrumentError RBRInstrument_getPowerInternal(
    RBRInstrument *instrument,
    RBRInstrumentPowerInternal *power);

/**
 * \brief Set the internal power battery type.
 *
 * \nol2 Always returns #RBRINSTRUMENT_UNSUPPORTED.
 *
 * \param [in] instrument the instrument connection
 * \param [in] type the battery type
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the instrument is logging
 * \see RBRInstrument_getPowerInternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerinternal
 */
RBRInstrumentError RBRInstrument_setInternalBatteryType(
    RBRInstrument *instrument,
    RBRInstrumentInternalBatteryType type);

/**
 * \brief Reset the counter of energy used from the internal battery.
 *
 * \nol2 Always returns #RBRINSTRUMENT_UNSUPPORTED.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the instrument is logging
 * \see RBRInstrument_getPowerInternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerinternal
 */
RBRInstrumentError RBRInstrument_resetPowerInternalUsed(
    RBRInstrument *instrument);

/**
 * External battery types.
 *
 * \see RBRInstrumentPowerExternal
 */
typedef enum RBRInstrumentExternalBatteryType
{
    /** Other/unknown external battery type */
    RBRINSTRUMENT_EXTERNAL_BATTERY_OTHER,
    /** RBRfermata Li-SOCl₂ */
    RBRINSTRUMENT_EXTERNAL_BATTERY_FERMATA_LISOCL2,
    /** RBRfermata Zn-MnO₂ */
    RBRINSTRUMENT_EXTERNAL_BATTERY_FERMATA_ZNMNO2,
    /** RBRfermette Li-MnO₂ */
    RBRINSTRUMENT_EXTERNAL_BATTERY_FERMETTE_LIMNO2,
    /** RBRfermette³ Li-SOCl₂ */
    RBRINSTRUMENT_EXTERNAL_BATTERY_FERMETTE3_LISOCL2,
    /** RBRfermette³ Li-FeS₂ */
    RBRINSTRUMENT_EXTERNAL_BATTERY_FERMETTE3_LIFES2,
    /** RBRfermette³ Zn-MnO₂ */
    RBRINSTRUMENT_EXTERNAL_BATTERY_FERMETTE3_ZNMNO2,
    /** RBRfermette³ Li-NiMnCo */
    RBRINSTRUMENT_EXTERNAL_BATTERY_FERMETTE3_LINIMNCO,
    /** RBRfermette³ NiMH */
    RBRINSTRUMENT_EXTERNAL_BATTERY_FERMETTE3_NIMH,
    /** The number of specific external battery types. */
    RBRINSTRUMENT_EXTERNAL_BATTERY_COUNT,
    /** An unknown or unrecognized external battery type. */
    RBRINSTRUMENT_UNKNOWN_EXTERNAL_BATTERY
} RBRInstrumentExternalBatteryType;

/**
 * \brief Get a human-readable string name for an external battery type.
 *
 * \param [in] type the battery type
 * \return a string name for the battery type
 * \see RBRInstrumentError_name() for a description of the format of names
 * \see RBRInstrumentExternalBatteryType_displayName() for display names
 */
const char *RBRInstrumentExternalBatteryType_name(
    RBRInstrumentExternalBatteryType type);

/**
 * \brief Get a human-readable display name for an external battery type.
 *
 * Unlike the values returned by RBRInstrumentExternalBatteryType_name(),
 * RBRfermata/RBRfermette product names will be correctly capitalized, and the
 * names of battery types will be formatted appropriately for the cell
 * chemistry; e.g., “RBRfermette³ Li-SOCl₂”, not “rbrfermette3 lisocl2”. Values
 * will be UTF-8-encoded.
 *
 * \param [in] type the battery type
 * \return a string name for the battery type
 * \see RBRInstrumentExternalBatteryType_name() for instrument-equivalent names
 */
const char *RBRInstrumentExternalBatteryType_displayName(
    RBRInstrumentExternalBatteryType type);

/**
 * \brief Instrument `powerexternal` command parameters.
 *
 * \see RBRInstrument_getPowerExternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerexternal
 */
typedef struct RBRInstrumentPowerExternal
{
    /** \brief The type of battery. */
    RBRInstrumentExternalBatteryType batteryType;
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
} RBRInstrumentPowerExternal;

/**
 * \brief Get instrument external power information.
 *
 * \nol2 Always returns #RBRINSTRUMENT_UNSUPPORTED.
 *
 * \param [in] instrument the instrument connection
 * \param [out] power the power information
 * \return #RBRINSTRUMENT_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_setPowerExternalBatteryType()
 * \see RBRInstrument_resetPowerExternalUsed()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerexternal
 */
RBRInstrumentError RBRInstrument_getPowerExternal(
    RBRInstrument *instrument,
    RBRInstrumentPowerExternal *power);

/**
 * \brief Set the external power battery type.
 *
 * \nol2 Always returns #RBRINSTRUMENT_UNSUPPORTED.
 *
 * \param [in] instrument the instrument connection
 * \param [in] type the battery type
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_getPowerExternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerexternal
 */
RBRInstrumentError RBRInstrument_setPowerExternalBatteryType(
    RBRInstrument *instrument,
    RBRInstrumentExternalBatteryType type);

/**
 * \brief Reset the counter of energy used from the external battery.
 *
 * \nol2 Always returns #RBRINSTRUMENT_UNSUPPORTED.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_getPowerExternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerexternal
 */
RBRInstrumentError RBRInstrument_resetPowerExternalUsed(
    RBRInstrument *instrument);

/**
 * \brief Instrument `info` command parameters.
 *
 * \see RBRInstrument_getInfo()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/info
 */
typedef struct RBRInstrumentInfo
{
    /** The RBR part number of the instrument. */
    char partNumber[RBRINSTRUMENT_PART_NUMBER_MAX + 1];
    /** Whether firmware upgrades are locked. */
    bool fwLock;
} RBRInstrumentInfo;

/**
 * \brief Get more information about the instrument.
 *
 * \nol2
 *
 * \param [in] instrument the instrument connection
 * \param [out] info the extended instrument information
 * \return #RBRINSTRUMENT_UNSUPPORTED for Logger2 instruments
 * \return #RBRINSTRUMENT_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/info
 */
RBRInstrumentError RBRInstrument_getInfo(
    RBRInstrument *instrument,
    RBRInstrumentInfo *info);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTOTHER_H */
