/**
 * \file RBRGen3Other.c
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

#include "RBRGen3.h"
#include "RBRGen3Internal.h"

/* The minimum length of a version string. */
#define VERSION_MIN 3

/* The maximum length of a version string. */
#define VERSION_MAX 7

int RBRGen3Version_compare(const char *inA, const char *inB)
{
    int lengthA = strlen(inA);
    int lengthB = strlen(inB);

    /* If one of the strings is too short or long to be a version, we can bail
     * out early. */
    bool validLengthA = lengthA >= VERSION_MIN && lengthA <= VERSION_MAX;
    bool validLengthB = lengthB >= VERSION_MIN && lengthB <= VERSION_MAX;

    if (!validLengthA && !validLengthB)
    {
        return 0;
    }
    else if (!validLengthA)
    {
        return -1;
    }
    else if (!validLengthB)
    {
        return 1;
    }

    /* Make copies of the version strings so we can modify them (to split at
     * the separator) with impunity. */
    char a[VERSION_MAX];
    char b[VERSION_MAX];
    snprintf(a, VERSION_MAX, "%s", inA);
    snprintf(b, VERSION_MAX, "%s", inB);

    /* '.' for production firmware releases; 'X' for developer versions. */
    char *separatorPosA = strchr(a, '.');
    if (separatorPosA == NULL)
    {
        separatorPosA = strchr(a, 'X');
    }

    char *separatorPosB = strchr(b, '.');
    if (separatorPosB == NULL)
    {
        separatorPosB = strchr(b, 'X');
    }

    /* The separators must be present, and there must be at least one character
     * before and after the separator. */
    bool validA = separatorPosA != NULL
                  && separatorPosA - a > 0
                  && separatorPosA - a < lengthA - 1;
    bool validB = separatorPosB != NULL
                  && separatorPosB - b > 0
                  && separatorPosB - b < lengthB - 1;

    if (!validA && !validB)
    {
        return 0;
    }
    else if (!validA)
    {
        return -1;
    }
    else if (!validB)
    {
        return 1;
    }

    /* Slap in a null terminator at the separator position so we can use strtol
     * on both sides. */
    char separatorA = *separatorPosA;
    *separatorPosA = '\0';
    char separatorB = *separatorPosB;
    *separatorPosB = '\0';

    int majorA = strtol(a, NULL, 10);
    int majorB = strtol(b, NULL, 10);
    int majorDelta = majorA - majorB;
    if (majorDelta != 0)
    {
        return majorDelta;
    }

    int minorA = strtol(separatorPosA + 1, NULL, 10);
    int minorB = strtol(separatorPosB + 1, NULL, 10);
    int minorDelta = minorA - minorB;
    if (minorDelta != 0)
    {
        return minorDelta;
    }

    /* Developer versions are always inferior to a production release with the
     * same major/minor version numbers. Assuming an ASCII character encoding,
     * 'X' comes after '.', so we can numerically determine the separator
     * ordering by subtracting then negating the result. */
    return -(separatorA - separatorB);
}

RBRGen3Error RBRGen3_getId(RBRGen3 *conn,
                                       RBRGen3Id *id)
{
    memset(id, 0, sizeof(RBRGen3Id));

    RBR_TRY(RBRGen3_converse(conn, "id"));

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    do
    {
        RBRGen3_parseResponse(conn,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        if (strcmp(parameter.key, "model") == 0)
        {
            snprintf(id->model,
                     sizeof(id->model),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "version") == 0)
        {
            snprintf(id->version,
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
        else if (strcmp(parameter.key, "mode") == 0)
        {
            snprintf(id->mode,
                     sizeof(id->mode),
                     "%s",
                     parameter.value);
        }
    } while (true);

    if (id != &conn->id)
    {
        memcpy(&conn->id, id, sizeof(RBRGen3Id));
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_getHardwareRevision(
    RBRGen3 *conn,
    RBRGen3HardwareRevision *hwrev)
{
    memset(hwrev, 0, sizeof(RBRGen3HardwareRevision));

    RBR_TRY(RBRGen3_converse(conn, "hwrev"));

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true)
    {
        RBRGen3_parseResponse(conn,
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

    return RBRGEN3_SUCCESS;
}

const char *RBRGen3PowerSource_name(RBRGen3PowerSource source)
{
    switch (source)
    {
    case RBRGEN3_POWER_SOURCE_USB:
        return "usb";
    case RBRGEN3_POWER_SOURCE_INTERNAL:
        return "int";
    case RBRGEN3_POWER_SOURCE_EXTERNAL:
        return "ext";
    case RBRGEN3_POWER_SOURCE_COUNT:
        return "power source count";
    case RBRGEN3_UNKNOWN_POWER_SOURCE:
    default:
        return "unknown power source";
    }
}

RBRGen3Error RBRGen3_getPower(RBRGen3 *conn,
                                          RBRGen3Power *power)
{
    memset(power, 0, sizeof(RBRGen3Power));
    power->source = RBRGEN3_UNKNOWN_POWER_SOURCE;
    power->internal = NAN;
    power->regulator = NAN;

    if (conn->generation == RBRGEN3_LOGGER2)
    {
        RBR_TRY(RBRGen3_converse(conn, "powerstatus"));
    }
    else
    {
        RBR_TRY(RBRGen3_converse(conn, "power"));
    }

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true)
    {
        RBRGen3_parseResponse(conn,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "source") == 0)
        {
            for (int i = 0; i < RBRGEN3_POWER_SOURCE_COUNT; i++)
            {
                if (strcmp(RBRGen3PowerSource_name(i),
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

    return RBRGEN3_SUCCESS;
}

const char *RBRGen3InternalBatteryType_name(
    RBRGen3InternalBatteryType type)
{
    switch (type)
    {
    case RBRGEN3_INTERNAL_BATTERY_NONE:
        return "none";
    case RBRGEN3_INTERNAL_BATTERY_LISOCL2:
        return "lisocl2";
    case RBRGEN3_INTERNAL_BATTERY_LIFES2:
        return "lifes2";
    case RBRGEN3_INTERNAL_BATTERY_ZNMNO2:
        return "znmno2";
    case RBRGEN3_INTERNAL_BATTERY_LINIMNCO:
        return "linimnco";
    case RBRGEN3_INTERNAL_BATTERY_NIMH:
        return "nimh";
    case RBRGEN3_INTERNAL_BATTERY_COUNT:
        return "internal battery type count";
    case RBRGEN3_UNKNOWN_INTERNAL_BATTERY:
    default:
        return "unknown internal battery type";
    }
}

const char *RBRGen3InternalBatteryType_displayName(
    RBRGen3InternalBatteryType type)
{
    switch (type)
    {
    case RBRGEN3_INTERNAL_BATTERY_NONE:
        return "none";
    case RBRGEN3_INTERNAL_BATTERY_LISOCL2:
        return "Li-SOCl₂";
    case RBRGEN3_INTERNAL_BATTERY_LIFES2:
        return "Li-FeS₂";
    case RBRGEN3_INTERNAL_BATTERY_ZNMNO2:
        return "Zn-MnO₂";
    case RBRGEN3_INTERNAL_BATTERY_LINIMNCO:
        return "Li-NiMnCo";
    case RBRGEN3_INTERNAL_BATTERY_NIMH:
        return "NiMH";
    case RBRGEN3_INTERNAL_BATTERY_COUNT:
        return "internal battery type count";
    case RBRGEN3_UNKNOWN_INTERNAL_BATTERY:
    default:
        return "unknown internal battery type";
    }
}

RBRGen3Error RBRGen3_getPowerInternal(
    RBRGen3 *conn,
    RBRGen3PowerInternal *power)
{
    if (conn->generation == RBRGEN3_LOGGER2)
    {
        return RBRGEN3_UNSUPPORTED;
    }

    memset(power, 0, sizeof(RBRGen3PowerInternal));
    power->batteryType = RBRGEN3_UNKNOWN_INTERNAL_BATTERY;

    RBR_TRY(RBRGen3_converse(conn, "powerinternal"));

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true)
    {
        RBRGen3_parseResponse(conn,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "batterytype") == 0)
        {
            for (int i = 0; i < RBRGEN3_INTERNAL_BATTERY_COUNT; i++)
            {
                if (strcmp(RBRGen3InternalBatteryType_name(i),
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

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_setPowerInternalBatteryType(
    RBRGen3 *conn,
    RBRGen3InternalBatteryType type)
{
    if (type < 0 || type >= RBRGEN3_INTERNAL_BATTERY_COUNT)
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    return RBRGen3_converse(
        conn,
        "powerinternal batterytype = %s",
        RBRGen3InternalBatteryType_name(type));
}

RBRGen3Error RBRGen3_resetPowerInternalUsed(
    RBRGen3 *conn)
{
    return RBRGen3_converse(conn, "powerinternal used = 0");
}

const char *RBRGen3ExternalBatteryType_name(
    RBRGen3ExternalBatteryType type)
{
    switch (type)
    {
    case RBRGEN3_EXTERNAL_BATTERY_OTHER:
        return "other";
    case RBRGEN3_EXTERNAL_BATTERY_FERMATA_LISOCL2:
        return "fermata_lisocl2";
    case RBRGEN3_EXTERNAL_BATTERY_FERMATA_ZNMNO2:
        return "fermata_znmno2";
    case RBRGEN3_EXTERNAL_BATTERY_FERMETTE_LIMNO2:
        return "fermette_limno2";
    case RBRGEN3_EXTERNAL_BATTERY_FERMETTE3_LISOCL2:
        return "fermette3_lisocl2";
    case RBRGEN3_EXTERNAL_BATTERY_FERMETTE3_LIFES2:
        return "fermette3_lifes2";
    case RBRGEN3_EXTERNAL_BATTERY_FERMETTE3_ZNMNO2:
        return "fermette3_znmno2";
    case RBRGEN3_EXTERNAL_BATTERY_FERMETTE3_LINIMNCO:
        return "fermette3_linimnco";
    case RBRGEN3_EXTERNAL_BATTERY_FERMETTE3_NIMH:
        return "fermette3_nimh";
    case RBRGEN3_EXTERNAL_BATTERY_COUNT:
        return "external battery type count";
    case RBRGEN3_UNKNOWN_EXTERNAL_BATTERY:
    default:
        return "unknown external battery type";
    }
}

const char *RBRGen3ExternalBatteryType_displayName(
    RBRGen3ExternalBatteryType type)
{
    switch (type)
    {
    case RBRGEN3_EXTERNAL_BATTERY_OTHER:
        return "other";
    case RBRGEN3_EXTERNAL_BATTERY_FERMATA_LISOCL2:
        return "RBRfermata Li-SOCl₂";
    case RBRGEN3_EXTERNAL_BATTERY_FERMATA_ZNMNO2:
        return "RBRfermata Zn-MnO₂";
    case RBRGEN3_EXTERNAL_BATTERY_FERMETTE_LIMNO2:
        return "RBRfermette Li-MnO₂";
    case RBRGEN3_EXTERNAL_BATTERY_FERMETTE3_LISOCL2:
        return "RBRfermette³ Li-SOCl₂";
    case RBRGEN3_EXTERNAL_BATTERY_FERMETTE3_LIFES2:
        return "RBRfermette³ Li-FeS₂";
    case RBRGEN3_EXTERNAL_BATTERY_FERMETTE3_ZNMNO2:
        return "RBRfermette³ Zn-MnO₂";
    case RBRGEN3_EXTERNAL_BATTERY_FERMETTE3_LINIMNCO:
        return "RBRfermette³ Li-NiMnCo";
    case RBRGEN3_EXTERNAL_BATTERY_FERMETTE3_NIMH:
        return "RBRfermette³ NiMH";
    case RBRGEN3_EXTERNAL_BATTERY_COUNT:
        return "external battery type count";
    case RBRGEN3_UNKNOWN_EXTERNAL_BATTERY:
    default:
        return "unknown external battery type";
    }
}

RBRGen3Error RBRGen3_getPowerExternal(
    RBRGen3 *conn,
    RBRGen3PowerExternal *power)
{
    if (conn->generation == RBRGEN3_LOGGER2)
    {
        return RBRGEN3_UNSUPPORTED;
    }

    memset(power, 0, sizeof(RBRGen3PowerExternal));
    power->batteryType = RBRGEN3_UNKNOWN_EXTERNAL_BATTERY;

    RBR_TRY(RBRGen3_converse(conn, "powerexternal"));

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true)
    {
        RBRGen3_parseResponse(conn,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "batterytype") == 0)
        {
            for (int i = 0; i < RBRGEN3_EXTERNAL_BATTERY_COUNT; i++)
            {
                if (strcmp(RBRGen3ExternalBatteryType_name(i),
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

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_setPowerExternalBatteryType(
    RBRGen3 *conn,
    RBRGen3ExternalBatteryType type)
{
    if (type < 0 || type >= RBRGEN3_EXTERNAL_BATTERY_COUNT)
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    return RBRGen3_converse(
        conn,
        "powerexternal batterytype = %s",
        RBRGen3ExternalBatteryType_name(type));
}

RBRGen3Error RBRGen3_resetPowerExternalUsed(
    RBRGen3 *conn)
{
    return RBRGen3_converse(conn, "powerexternal used = 0");
}

RBRGen3Error RBRGen3_getInfo(
    RBRGen3 *conn,
    RBRGen3Info *info)
{
    if (conn->generation == RBRGEN3_LOGGER2)
    {
        return RBRGEN3_UNSUPPORTED;
    }

    memset(info, 0, sizeof(RBRGen3Info));

    RBR_TRY(RBRGen3_converse(conn, "info"));

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true)
    {
        RBRGen3_parseResponse(conn,
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

    return RBRGEN3_SUCCESS;
}
