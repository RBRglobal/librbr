/**
 * \file RBRInstrumentGen4Other.h
 *
 * \brief Instrument commands and structures for miscellaneous commands.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTGEN4OTHER_H
#define LIBRBR_RBRINSTRUMENTGEN4OTHER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief The maximum number of characters in the type and revision of the CPU.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENTGEN4_HWREV_CPU_MAX 5

/**
 * \brief The maximum number of characters in an instrument part number.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENTGEN4_PART_NUMBER_MAX 255

/**
 * \brief Possible results when comparing two firmware version strings.
 *
 * \see RBRInstrumentGen4Version_compare()
 */
typedef enum RBRInstrumentGen4FWVersionCompareResult
{
    /** Invalid string for FW version found in at least one of the inputs 
     * for RBRInstrumentGen4Version_compare().
     */
    RBRINSTRUMENTGEN4_FW_INVALID = -2,
    /** First FW version input is less than the second input. */
    RBRINSTRUMENTGEN4_FW_LESS_THAN = -1,
    /** First FW version input is greater than the second input. */
    RBRINSTRUMENTGEN4_FW_GREATER_THAN = 1,
    /** Two FW version inputs equal. */
    RBRINSTRUMENTGEN4_FW_EQUAL = 0
} RBRInstrumentGen4FWVersionCompareResult;

/**
 * 
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
int RBRInstrumentGen4Version_compare(const char *a, const char *b);

/**
 * \brief Get identification information from the instrument.
 *
 * \param [in] instrument the instrument connection
 * \param [out] id the instrument information
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/id
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getId(RBRInstrumentGen4 *instrument,
                                       RBRInstrumentGen4Id *id);

/**
 * \brief Instrument `hwrev` command parameters.
 *
 * \see RBRInstrumentGen4_getHardwareRevision()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/hwrev
 */
typedef struct RBRInstrumentGen4HardwareRevision
{
    /** The revision of the CPU PCB. */
    char pcb;
    /** The part number and revision of the CPU. */
    char cpu[RBRINSTRUMENTGEN4_HWREV_CPU_MAX + 1];
    /** The revision of the CPU boot loader. */
    char bsl;
} RBRInstrumentGen4HardwareRevision;

/**
 * \brief Get instrument hardware revision information.
 *
 * \param [in] instrument the instrument connection
 * \param [out] hwrev the hardware revision information
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/hwrev
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getHardwareRevision(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4HardwareRevision *hwrev);

/**
 * \brief Possible instrument power sources.
 *
 * \see RBRInstrumentGen4Power
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
 * \brief Instrument `power` command parameters.
 *
 * \see RBRInstrumentGen4_getPower()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/power
 */
typedef struct RBRInstrumentGen4Power
{
    /** \brief The power source from which the instrument is running. */
    RBRInstrumentGen4PowerSource source;
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
} RBRInstrumentGen4Power;

/**
 * \brief Get instrument power information.
 *
 * \param [in] instrument the instrument connection
 * \param [out] power the power information
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if an error occurs reading voltages
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/power
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getPower(RBRInstrumentGen4 *instrument,
                                          RBRInstrumentGen4Power *power);

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
 * \brief Instrument `powerinternal` command parameters.
 *
 * \see RBRInstrumentGen4_getPowerInternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerinternal
 */
typedef struct RBRInstrumentGen4PowerInternal
{
    /** \brief The type of battery. */
    RBRInstrumentGen4InternalBatteryType batteryType;
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
} RBRInstrumentGen4PowerInternal;

/**
 * \brief Get instrument internal power information.
 *
 * \nol2 Always returns #RBRINSTRUMENTGEN4_UNSUPPORTED.
 *
 * \param [in] instrument the instrument connection
 * \param [out] power the power information
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_setPowerInternalBatteryType()
 * \see RBRInstrumentGen4_resetPowerInternalUsed()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerinternal
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getPowerInternal(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4PowerInternal *power);

/**
 * \brief Set the internal power battery type.
 *
 * \nol2 Always returns #RBRINSTRUMENTGEN4_UNSUPPORTED.
 *
 * \param [in] instrument the instrument connection
 * \param [in] type the battery type
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the instrument is logging
 * \see RBRInstrumentGen4_getPowerInternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerinternal
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setPowerInternalBatteryType(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4InternalBatteryType type);

/**
 * \brief Reset the counter of energy used from the internal battery.
 *
 * \nol2 Always returns #RBRINSTRUMENTGEN4_UNSUPPORTED.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the instrument is logging
 * \see RBRInstrumentGen4_getPowerInternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerinternal
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
 * \brief Instrument `powerexternal` command parameters.
 *
 * \see RBRInstrumentGen4_getPowerExternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerexternal
 */
typedef struct RBRInstrumentGen4PowerExternal
{
    /** \brief The type of battery. */
    RBRInstrumentGen4ExternalBatteryType batteryType;
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
} RBRInstrumentGen4PowerExternal;

/**
 * \brief Get instrument external power information.
 *
 * \nol2 Always returns #RBRINSTRUMENTGEN4_UNSUPPORTED.
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
 *
 * \nol2 Always returns #RBRINSTRUMENTGEN4_UNSUPPORTED.
 *
 * \param [in] instrument the instrument connection
 * \param [in] type the battery type
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_getPowerExternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerexternal
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setPowerExternalBatteryType(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4ExternalBatteryType type);

/**
 * \brief Reset the counter of energy used from the external battery.
 *
 * \nol2 Always returns #RBRINSTRUMENTGEN4_UNSUPPORTED.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_getPowerExternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerexternal
 */
RBRInstrumentGen4Error RBRInstrumentGen4_resetPowerExternalUsed(
    RBRInstrumentGen4 *instrument);

/**
 * \brief Instrument `info` command parameters.
 *
 * \see RBRInstrumentGen4_getInfo()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/info
 */
typedef struct RBRInstrumentGen4Info
{
    /** The RBR part number of the instrument. */
    char partNumber[RBRINSTRUMENTGEN4_PART_NUMBER_MAX + 1];
    /** Whether firmware upgrades are locked. */
    bool fwLock;
} RBRInstrumentGen4Info;

/**
 * \brief Get more information about the instrument.
 *
 * \nol2
 *
 * \param [in] instrument the instrument connection
 * \param [out] info the extended instrument information
 * \return #RBRINSTRUMENTGEN4_UNSUPPORTED for Logger2 instruments
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/info
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getInfo(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Info *info);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTOTHER_H */
