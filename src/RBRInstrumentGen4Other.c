/**
 * \file RBRInstrumentGen4Other.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for NAN. */
#include <math.h>
/* Required for memcpy, memset, strcmp. */
#include <string.h>
/* Required for snprintf. */
#include <stdio.h>
/* Required for strtol. */
#include <stdlib.h>

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Internal.h"
#include "RBRInstrumentGen4Other.h"

/* The minimum length of a version string. */
#define VERSION_MIN 3

/* The maximum length of a version string. */
#define VERSION_MAX_LENGTH RBRINSTRUMENTGEN4_ID_VERSION_MAX

#define VERSION_NUM_MAJOR 0
#define VERSION_NUM_MINOR 1
#define VERSION_NUM_PATCH 2
#define VERSION_NUM_COUNT 3

bool scan_version(const char *version, uint32_t *version_arr)
{
    char _versionCpy[VERSION_MAX_LENGTH];
    snprintf(_versionCpy, VERSION_MAX_LENGTH, "%s", version);
    
    char *separatorPos = strchr(_versionCpy, '+');
    if (separatorPos != NULL){
        *separatorPos = '\0';
    }
    //in case this is a dev version: 0.0.0-dev.dbg+20231015092
    separatorPos = strchr(_versionCpy, '-');
    if (separatorPos != NULL){
        *separatorPos = '\0';
    }

    return (sscanf(_versionCpy, 
                   "%u.%u.%u",
                   &(version_arr[VERSION_NUM_MAJOR]),
                   &(version_arr[VERSION_NUM_MINOR]),
                   &(version_arr[VERSION_NUM_PATCH])) == VERSION_NUM_COUNT) ? true : false;
}

//used in RBRInstrumentGen4_setPostprocessing function.
int RBRInstrumentGen4Version_compare(const char *version1, const char *version2)
{
    uint32_t version1_arr[VERSION_NUM_COUNT];
    uint32_t version2_arr[VERSION_NUM_COUNT];

    if (!scan_version(version1, version1_arr))
    {
        //printf("Failed to read first version input: '%s'\n", version1);
        return RBRINSTRUMENTGEN4_FW_INVALID;
    }

    if (!scan_version(version2, version2_arr))
    {
        //printf("Failed to read second version input: '%s'\n", version2);
        return RBRINSTRUMENTGEN4_FW_INVALID;
    }

    for (int i = 0; i < VERSION_NUM_COUNT; i++)
    {
        if (version1_arr[i] < version2_arr[i])
        {
            return RBRINSTRUMENTGEN4_FW_LESS_THAN;
        }
        else if (version1_arr[i] > version2_arr[i])
        {
            return RBRINSTRUMENTGEN4_FW_GREATER_THAN;
        }
    }
    return RBRINSTRUMENTGEN4_FW_EQUAL;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getId(RBRInstrumentGen4 *instrument,
                                       RBRInstrumentGen4Id *id)
{
    memset(id, 0, sizeof(RBRInstrumentGen4Id));

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "id"));
    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    do
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                    &command,
                                    &parameter);
        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        if (strcmp(parameter.key, "model") == 0)
        {
            snprintf((char *)(id->model),
                     sizeof(id->model),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "version") == 0)
        {
            snprintf((char *)(id->version),
                     sizeof(id->version),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "serial") == 0)
        {
            id->serial = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "fwtype") == 0)
        {
            id->fwtype = strtol(parameter.value, NULL, 10);
        }
    } while (true);
    if (id != &instrument->id)
    {
        memcpy(&instrument->id, id, sizeof(RBRInstrumentGen4Id));
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getHardwareRevision(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4HardwareRevision *hwrev)
{
    memset(hwrev, 0, sizeof(RBRInstrumentGen4HardwareRevision));

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "hwrev"));

    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "pcb") == 0)
        {
            hwrev->pcb = *parameter.value;
        }
        else if (strcmp(parameter.key, "cpu") == 0)
        {
            snprintf(hwrev->cpu,
                     sizeof(hwrev->cpu),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "bsl") == 0)
        {
            hwrev->bsl = *parameter.value;
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

const char *RBRInstrumentGen4PowerSource_name(RBRInstrumentGen4PowerSource source)
{
    switch (source)
    {
    case RBRINSTRUMENTGEN4_POWER_SOURCE_USB:
        return "usb";
    case RBRINSTRUMENTGEN4_POWER_SOURCE_INTERNAL:
        return "int";
    case RBRINSTRUMENTGEN4_POWER_SOURCE_EXTERNAL:
        return "ext";
    case RBRINSTRUMENTGEN4_POWER_SOURCE_COUNT:
        return "power source count";
    // case RBRINSTRUMENTGEN4_POWER_SOURCE_UNKNOWN:
    default:
        return "unknown power source";
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_getPower(RBRInstrumentGen4 *instrument,
                                          RBRInstrumentGen4Power *power)
{
    memset(power, 0, sizeof(RBRInstrumentGen4Power));
    power->source = RBRINSTRUMENTGEN4_POWER_SOURCE_UNKNOWN;
    power->internal = NAN;
    power->regulator = NAN;

    if (instrument->generation == RBRINSTRUMENTGEN4_LOGGER2)
    {
        RBR_TRY(RBRInstrumentGen4_converse(instrument, "powerstatus"));
    }
    else
    {
        RBR_TRY(RBRInstrumentGen4_converse(instrument, "power"));
    }

    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "source") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENTGEN4_POWER_SOURCE_COUNT; i++)
            {
                if (strcmp(RBRInstrumentGen4PowerSource_name(i),
                           parameter.value) == 0)
                {
                    power->source = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "int") == 0)
        {
            if (strcmp(parameter.value, "n/a") != 0)
            {
                power->internal = strtod(parameter.value, NULL);
            }
        }
        else if (strcmp(parameter.key, "ext") == 0)
        {
            if (strcmp(parameter.value, "n/a") != 0)
            {
                power->external = strtod(parameter.value, NULL);
            }
        }
        else if (strcmp(parameter.key, "reg") == 0)
        {
            if (strcmp(parameter.value, "n/a") != 0)
            {
                power->regulator = strtod(parameter.value, NULL);
            }
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

const char *RBRInstrumentGen4InternalBatteryType_name(
    RBRInstrumentGen4InternalBatteryType type)
{
    switch (type)
    {
    case RBRINSTRUMENTGEN4_INTERNAL_BATTERY_NONE:
        return "none";
    case RBRINSTRUMENTGEN4_INTERNAL_BATTERY_LISOCL2:
        return "lisocl2";
    case RBRINSTRUMENTGEN4_INTERNAL_BATTERY_LIFES2:
        return "lifes2";
    case RBRINSTRUMENTGEN4_INTERNAL_BATTERY_ZNMNO2:
        return "znmno2";
    case RBRINSTRUMENTGEN4_INTERNAL_BATTERY_LINIMNCO:
        return "linimnco";
    case RBRINSTRUMENTGEN4_INTERNAL_BATTERY_NIMH:
        return "nimh";
    case RBRINSTRUMENTGEN4_INTERNAL_BATTERY_COUNT:
        return "internal battery type count";
    // case RBRINSTRUMENTGEN4_UNKNOWN_INTERNAL_BATTERY:
    default:
        return "unknown internal battery type";
    }
}

const char *RBRInstrumentGen4InternalBatteryType_dispalyName(
    RBRInstrumentGen4InternalBatteryType type)
{
    switch (type)
    {
    case RBRINSTRUMENTGEN4_INTERNAL_BATTERY_NONE:
        return "none";
    case RBRINSTRUMENTGEN4_INTERNAL_BATTERY_LISOCL2:
        return "Li-SOCl₂";
    case RBRINSTRUMENTGEN4_INTERNAL_BATTERY_LIFES2:
        return "Li-FeS₂";
    case RBRINSTRUMENTGEN4_INTERNAL_BATTERY_ZNMNO2:
        return "Zn-MnO₂";
    case RBRINSTRUMENTGEN4_INTERNAL_BATTERY_LINIMNCO:
        return "Li-NiMnCo";
    case RBRINSTRUMENTGEN4_INTERNAL_BATTERY_NIMH:
        return "NiMH";
    case RBRINSTRUMENTGEN4_INTERNAL_BATTERY_COUNT:
        return "internal battery type count";
    // case RBRINSTRUMENTGEN4_UNKNOWN_INTERNAL_BATTERY:
    default:
        return "unknown internal battery type";
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_getPowerInternal(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4PowerInternal *power)
{
    if (instrument->generation == RBRINSTRUMENTGEN4_LOGGER2)
    {
        return RBRINSTRUMENTGEN4_UNSUPPORTED;
    }

    memset(power, 0, sizeof(RBRInstrumentGen4PowerInternal));
    power->batteryType = RBRINSTRUMENTGEN4_UNKNOWN_INTERNAL_BATTERY;

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "powerinternal"));

    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "batterytype") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENTGEN4_INTERNAL_BATTERY_COUNT; i++)
            {
                if (strcmp(RBRInstrumentGen4InternalBatteryType_name(i),
                           parameter.value) == 0)
                {
                    power->batteryType = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "capacity") == 0)
        {
            *(float *) &power->capacity = strtod(parameter.value, NULL);
        }
        else if (strcmp(parameter.key, "used") == 0)
        {
            power->used = strtod(parameter.value, NULL);
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setPowerInternalBatteryType(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4InternalBatteryType type)
{
    if (type < 0 || type >= RBRINSTRUMENTGEN4_INTERNAL_BATTERY_COUNT)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrumentGen4_converse(
        instrument,
        "powerinternal batterytype = %s",
        RBRInstrumentGen4InternalBatteryType_name(type));
}

RBRInstrumentGen4Error RBRInstrumentGen4_resetPowerInternalUsed(
    RBRInstrumentGen4 *instrument)
{
    return RBRInstrumentGen4_converse(instrument, "powerinternal used = 0");
}

const char *RBRInstrumentGen4ExternalBatteryType_name(
    RBRInstrumentGen4ExternalBatteryType type)
{
    switch (type)
    {
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMATA_LISOCL2:
        return "fermata_lisocl2";
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMATA_ZNMNO2:
        return "fermata_znmno2";
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE_LIMNO2:
        return "fermette_limno2";
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_LISOCL2:
        return "fermette3_lisocl2";
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_LIFES2:
        return "fermette3_lifes2";
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_ZNMNO2:
        return "fermette3_znmno2";
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_LINIMNCO:
        return "fermette3_linimnco";
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_NIMH:
        return "fermette3_nimh";
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMATA_NIMH:
        return "fermata_nimh";
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_OTHER:
        return "other";
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_COUNT:
        return "external battery type count";
    // case RBRINSTRUMENTGEN4_UNKNOWN_EXTERNAL_BATTERY:
    default:
        return "unknown external battery type";
    }
}

const char *RBRInstrumentGen4ExternalBatteryType_displayName(
    RBRInstrumentGen4ExternalBatteryType type)
{
    switch (type)
    {

    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMATA_LISOCL2:
        return "RBRfermata Li-SOCl₂";
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMATA_ZNMNO2:
        return "RBRfermata Zn-MnO₂";
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE_LIMNO2:
        return "RBRfermette Li-MnO₂";
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_LISOCL2:
        return "RBRfermette³ Li-SOCl₂";
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_LIFES2:
        return "RBRfermette³ Li-FeS₂";
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_ZNMNO2:
        return "RBRfermette³ Zn-MnO₂";
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_LINIMNCO:
        return "RBRfermette³ Li-NiMnCo";
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMETTE3_NIMH:
        return "RBRfermette³ NiMH";
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_FERMATA_NIMH:
        return "RBRfermata_nimh";
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_OTHER:
        return "other";
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_COUNT:
        return "external battery type count";
    // case RBRINSTRUMENTGEN4_UNKNOWN_EXTERNAL_BATTERY:
    default:
        return "unknown external battery type";
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_getPowerExternal(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4PowerExternal *power)
{
    if (instrument->generation == RBRINSTRUMENTGEN4_LOGGER2)
    {
        return RBRINSTRUMENTGEN4_UNSUPPORTED;
    }

    memset(power, 0, sizeof(RBRInstrumentGen4PowerExternal));
    power->batteryType = RBRINSTRUMENTGEN4_UNKNOWN_EXTERNAL_BATTERY;

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "powerexternal"));

    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "batterytype") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_COUNT; i++)
            {
                if (strcmp(RBRInstrumentGen4ExternalBatteryType_name(i),
                           parameter.value) == 0)
                {
                    power->batteryType = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "capacity") == 0)
        {
            *(float *) &power->capacity = strtod(parameter.value, NULL);
        }
        else if (strcmp(parameter.key, "used") == 0)
        {
            power->used = strtod(parameter.value, NULL);
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setPowerExternalBatteryType(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4ExternalBatteryType type)
{
    if (type < 0 || type >= RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_COUNT)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrumentGen4_converse(
        instrument,
        "powerexternal batterytype = %s",
        RBRInstrumentGen4ExternalBatteryType_name(type));
}

RBRInstrumentGen4Error RBRInstrumentGen4_resetPowerExternalUsed(
    RBRInstrumentGen4 *instrument)
{
    return RBRInstrumentGen4_converse(instrument, "powerexternal used = 0");
}

RBRInstrumentGen4Error RBRInstrumentGen4_getInfo(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Info *info)
{
    if (instrument->generation == RBRINSTRUMENTGEN4_LOGGER2)
    {
        return RBRINSTRUMENTGEN4_UNSUPPORTED;
    }

    memset(info, 0, sizeof(RBRInstrumentGen4Info));

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "info"));

    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "pn") == 0)
        {
            snprintf(info->partNumber,
                     sizeof(info->partNumber),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "fwlock") == 0)
        {
            info->fwLock = (strcmp(parameter.value, "on") == 0);
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}
