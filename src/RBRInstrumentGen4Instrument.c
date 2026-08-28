/**
 * \file RBRInstrumentGen4Instrument.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for isspace. */
#include <ctype.h>
/* Required for NAN. */
#include <math.h>
/* Required for memcpy, memset, strchr, strcmp. */
#include <string.h>
/* Required for snprintf. */
#include <stdio.h>
/* Required for strtol. */
#include <stdlib.h>

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Internal.h"
#include "RBRInstrumentGen4Instrument.h"

const char *RBRInstrumentGen4InstrumentState_name(
    RBRInstrumentGen4InstrumentState state)
{
    switch (state)
    {
    case RBRINSTRUMENTGEN4_INSTRUMENT_STATE_DISABLED:
        return "disabled";
    case RBRINSTRUMENTGEN4_INSTRUMENT_STATE_ENABLED:
        return "enabled";
    case RBRINSTRUMENTGEN4_INSTRUMENT_STATE_COUNT:
        return "instrument state count";
    case RBRINSTRUMENTGEN4_UNKNOWN_INSTRUMENT_STATE:
    default:
        return "unknown instrument state";
    }
}

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

/*
 * The `id` command separates its parameters with commas and pads its
 * assignments with spaces (`id model = L4, version = 2.0.0`), so the Gen4
 * response tokenizer cannot be used on it.
 */
#define LEGACY_ID_PARAMETER_SEPARATOR ','
#define LEGACY_ID_ASSIGNMENT '='

/*
 * The longest `id` parameter name is “version”, at seven characters. One
 * character of headroom keeps an unrecognised longer name from truncating
 * onto a name we look for: anything which does not fit is cut to the full
 * eight characters, which matches none of them.
 */
#define LEGACY_ID_KEY_MAX 8

/**
 * \brief Copy a range of characters into a buffer, less any padding.
 *
 * Leading and trailing whitespace within the range is discarded. No more than
 * \a size characters are written, inclusive of the null terminator.
 *
 * \param [out] destination the destination buffer
 * \param [in] size the size of the destination buffer
 * \param [in] begin the first character of the range
 * \param [in] end one character past the end of the range
 */
static void RBRInstrumentGen4_copyTrimmed(char *destination,
                                          size_t size,
                                          const char *begin,
                                          const char *end)
{
    while (begin < end && isspace((unsigned char) *begin))
    {
        ++begin;
    }
    while (end > begin && isspace((unsigned char) *(end - 1)))
    {
        --end;
    }

    size_t length = (size_t) (end - begin);
    if (length > size - 1)
    {
        length = size - 1;
    }
    memcpy(destination, begin, length);
    destination[length] = '\0';
}

RBRInstrumentGen4Error RBRInstrumentGen4_getId(RBRInstrumentGen4 *instrument,
                                       RBRInstrumentGen4Id *id)
{
    memset(id, 0, sizeof(RBRInstrumentGen4Id));

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "id"));

    const char *cursor = instrument->response.response;
    if (cursor == NULL)
    {
        return RBRINSTRUMENTGEN4_SUCCESS;
    }

    /* Step over the command name which the instrument echoes back. */
    cursor = strchr(cursor, ' ');

    while (cursor != NULL)
    {
        char key[LEGACY_ID_KEY_MAX + 1];
        /* Long enough for any 32-bit decimal value and its sign. */
        char number[12];

        const char *assignment = strchr(cursor, LEGACY_ID_ASSIGNMENT);
        if (assignment == NULL)
        {
            break;
        }

        const char *end = strchr(assignment, LEGACY_ID_PARAMETER_SEPARATOR);
        if (end == NULL)
        {
            end = assignment + strlen(assignment);
        }

        RBRInstrumentGen4_copyTrimmed(key, sizeof(key), cursor, assignment);

        if (strcmp(key, "model") == 0)
        {
            RBRInstrumentGen4_copyTrimmed((char *) (id->model),
                                          sizeof(id->model),
                                          assignment + 1,
                                          end);
        }
        else if (strcmp(key, "version") == 0)
        {
            RBRInstrumentGen4_copyTrimmed((char *) (id->fwversion),
                                          sizeof(id->fwversion),
                                          assignment + 1,
                                          end);
        }
        else if (strcmp(key, "serial") == 0)
        {
            RBRInstrumentGen4_copyTrimmed(number,
                                          sizeof(number),
                                          assignment + 1,
                                          end);
            id->sn = strtol(number, NULL, 10);
        }
        else if (strcmp(key, "fwtype") == 0)
        {
            RBRInstrumentGen4_copyTrimmed(number,
                                          sizeof(number),
                                          assignment + 1,
                                          end);
            id->fwtype = strtol(number, NULL, 10);
        }

        cursor = *end == '\0' ? NULL : end + 1;
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getId4(RBRInstrumentGen4 *instrument,
                                       RBRInstrumentGen4Id4 *id)
{
    memset(id, 0, sizeof(RBRInstrumentGen4Id4));

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "id4"));
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
        else if (strcmp(parameter.key, "fwversion") == 0)
        {
            snprintf((char *)(id->fwversion),
                     sizeof(id->fwversion),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "semver") == 0)
        {
            snprintf((char *)(id->semver),
                     sizeof(id->semver),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "sn") == 0)
        {
            id->sn = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "fwtype") == 0)
        {
            id->fwtype = strtol(parameter.value, NULL, 10);
        }
    } while (true);
    if (id != &instrument->id)
    {
        memcpy(&instrument->id, id, sizeof(RBRInstrumentGen4Id4));
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getPcbaPool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4PcbaPool *pcbaPool)
{
    memset(pcbaPool, 0, sizeof(RBRInstrumentGen4PcbaPool));

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "pcba"));

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
        else if (strcmp(parameter.key, "count") == 0)
        {
            pcbaPool->count = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "list") == 0)
        {
            char *nextValue = NULL;
            for (int32_t pcba = 0; pcba < RBRINSTRUMENTGEN4_PCBA_COUNT_MAX; pcba++)
            {
                if ((nextValue = strstr(parameter.value, "|")) != NULL)
                {
                    *nextValue = '\0';
                    nextValue++;
                }

                snprintf((char *)(pcbaPool->pool[pcba].label),
                         sizeof(pcbaPool->pool[pcba].label),
                         "%s",
                         parameter.value);

                if (nextValue == NULL)
                {
                    break;
                }
                else
                {
                    parameter.value = nextValue;
                }
            }
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getPcba(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Pcba *pcba)
{
    /* The label selects the PCBA to read, so it has to outlive the reset of
     * the rest of the structure. */
    char label[sizeof(pcba->label)];
    snprintf(label, sizeof(label), "%s", pcba->label);

    memset(pcba, 0, sizeof(RBRInstrumentGen4Pcba));

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "pcba %s", label));

    snprintf(pcba->label, sizeof(pcba->label), "%s", label);

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
        else if (strcmp(parameter.key, "sn") == 0)
        {
            /* `na` is not a number, so it converts to the zero which stands
             * for it. */
            pcba->sn = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "pn") == 0)
        {
            snprintf(pcba->pn,
                     sizeof(pcba->pn),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "node") == 0)
        {
            snprintf(pcba->node,
                     sizeof(pcba->node),
                     "%s",
                     parameter.value);
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

RBRInstrumentGen4Error RBRInstrumentGen4_getPowerSource(RBRInstrumentGen4 *instrument,
                                          RBRInstrumentGen4PowerSource *powerSource)
{
    *powerSource = RBRINSTRUMENTGEN4_POWER_SOURCE_UNKNOWN;

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "instrument power"));

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
                    *powerSource = i;
                    break;
                }
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

const char *RBRInstrumentGen4InternalBatteryType_displayName(
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

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "instrument power internal"));

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
        else if (strcmp(parameter.key, "voltage") == 0)
        {
            *(float *) &power->voltage = strtod(parameter.value, NULL);
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
        "instrument power internal batterytype=%s",
        RBRInstrumentGen4InternalBatteryType_name(type));
}

RBRInstrumentGen4Error RBRInstrumentGen4_resetPowerInternalUsed(
    RBRInstrumentGen4 *instrument)
{
    return RBRInstrumentGen4_converse(instrument, "instrument power internal used=0");
}

const char *RBRInstrumentGen4ExternalBatteryType_name(
    RBRInstrumentGen4ExternalBatteryType type)
{
    switch (type)
    {
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_NONE:
        return "none";
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
    case RBRINSTRUMENTGEN4_EXTERNAL_BATTERY_NONE:
        return "none";
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
    memset(power, 0, sizeof(RBRInstrumentGen4PowerExternal));
    power->batteryType = RBRINSTRUMENTGEN4_UNKNOWN_EXTERNAL_BATTERY;

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "instrument power external"));

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
        else if (strcmp(parameter.key, "voltage") == 0)
        {
            *(float *) &power->voltage = strtod(parameter.value, NULL);
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
        "instrument power external batterytype=%s",
        RBRInstrumentGen4ExternalBatteryType_name(type));
}

RBRInstrumentGen4Error RBRInstrumentGen4_resetPowerExternalUsed(
    RBRInstrumentGen4 *instrument)
{
    return RBRInstrumentGen4_converse(instrument, "instrument power external used=0");
}

/**
 * \brief Resolve a `datatype` parameter value to its enum member.
 *
 * Matched against RBRInstrumentGen4DataType_name() so that the accepted
 * spellings cannot drift from the ones the library emits.
 *
 * \param [in] value the parameter value reported by the instrument
 * \return the corresponding data type
 * \return #RBRINSTRUMENTGEN4_UNKNOWN_DATATYPE when the value is unrecognized
 */
static RBRInstrumentGen4DataType RBRInstrumentGen4DataType_parse(
    const char *value)
{
    for (int32_t dataType = 0;
         dataType < RBRINSTRUMENTGEN4_DATATYPE_COUNT;
         ++dataType)
    {
        if (strcmp(value,
                   RBRInstrumentGen4DataType_name(
                       (RBRInstrumentGen4DataType) dataType)) == 0)
        {
            return (RBRInstrumentGen4DataType) dataType;
        }
    }

    return RBRINSTRUMENTGEN4_UNKNOWN_DATATYPE;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getInstrument(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Instrument *instrumentInfo)
{
    memset(instrumentInfo, 0, sizeof(RBRInstrumentGen4Instrument));
    instrumentInfo->state = RBRINSTRUMENTGEN4_UNKNOWN_INSTRUMENT_STATE;
    instrumentInfo->dataType = RBRINSTRUMENTGEN4_UNKNOWN_DATATYPE;

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "instrument"));

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
        else if (strcmp(parameter.key, "state") == 0)
        {
            if (strcmp(parameter.value, "disabled") == 0)
            {
                instrumentInfo->state
                    = RBRINSTRUMENTGEN4_INSTRUMENT_STATE_DISABLED;
            }
            else if (strcmp(parameter.value, "enabled") == 0)
            {
                instrumentInfo->state
                    = RBRINSTRUMENTGEN4_INSTRUMENT_STATE_ENABLED;
            }
        }
        else if (strcmp(parameter.key, "sn") == 0)
        {
            instrumentInfo->sn = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "model") == 0)
        {
            snprintf(instrumentInfo->model,
                     sizeof(instrumentInfo->model),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "pn") == 0)
        {
            snprintf(instrumentInfo->pn,
                     sizeof(instrumentInfo->pn),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "fwversion") == 0)
        {
            snprintf(instrumentInfo->fwversion,
                     sizeof(instrumentInfo->fwversion),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "semver") == 0)
        {
            snprintf(instrumentInfo->semver,
                     sizeof(instrumentInfo->semver),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "fwtype") == 0)
        {
            instrumentInfo->fwtype = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "fwlock") == 0)
        {
            instrumentInfo->fwLock = (strcmp(parameter.value, "on") == 0);
        }
        else if (strcmp(parameter.key, "datatype") == 0)
        {
            instrumentInfo->dataType = RBRInstrumentGen4DataType_parse(parameter.value);
        }
        else if (strcmp(parameter.key, "name") == 0)
        {
            snprintf(instrumentInfo->name,
                     sizeof(instrumentInfo->name),
                     "%s",
                     parameter.value);
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

/**
 * \brief Resolve an `encoding` parameter value to its enum member.
 *
 * \param [in] value the parameter value reported by the instrument
 * \return the corresponding encoding
 * \return #RBRINSTRUMENTGEN4_UNKNOWN_ENCODING when the value is unrecognized
 */
static RBRInstrumentGen4Encoding RBRInstrumentGen4Encoding_parse(
    const char *value)
{
    for (int32_t encoding = 0;
         encoding < RBRINSTRUMENTGEN4_ENCODING_COUNT;
         ++encoding)
    {
        if (strcmp(value,
                   RBRInstrumentGen4Encoding_name(
                       (RBRInstrumentGen4Encoding) encoding)) == 0)
        {
            return (RBRInstrumentGen4Encoding) encoding;
        }
    }

    return RBRINSTRUMENTGEN4_UNKNOWN_ENCODING;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getOutputFormat(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4OutputFormat *outputformat)
{
    memset(outputformat, 0, sizeof(RBRInstrumentGen4OutputFormat));
    outputformat->encoding = RBRINSTRUMENTGEN4_UNKNOWN_ENCODING;
    outputformat->dataType = RBRINSTRUMENTGEN4_UNKNOWN_DATATYPE;

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "instrument outputformat"));

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

        bool enabled = strcmp(parameter.value, "on") == 0;

        if (strcmp(parameter.key, "sn") == 0)
        {
            outputformat->sn = enabled;
        }
        else if (strcmp(parameter.key, "schedulelabel") == 0)
        {
            outputformat->scheduleLabel = enabled;
        }
        else if (strcmp(parameter.key, "datetime") == 0)
        {
            outputformat->dateTime = enabled;
        }
        else if (strcmp(parameter.key, "crc") == 0)
        {
            outputformat->crc = enabled;
        }
        else if (strcmp(parameter.key, "encoding") == 0)
        {
            outputformat->encoding
                = RBRInstrumentGen4Encoding_parse(parameter.value);
        }
        else if (strcmp(parameter.key, "datatype") == 0)
        {
            outputformat->dataType
                = RBRInstrumentGen4DataType_parse(parameter.value);
        }
    }

    /* The caller may have asked us to populate the cache itself. */
    if (outputformat != &instrument->outputFormat)
    {
        instrument->outputFormat = *outputformat;
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setOutputFormat(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4OutputFormat *outputformat)
{
    if (outputformat->encoding < 0
        || outputformat->encoding >= RBRINSTRUMENTGEN4_ENCODING_COUNT
        || outputformat->dataType < 0
        || outputformat->dataType >= RBRINSTRUMENTGEN4_DATATYPE_COUNT)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    RBR_TRY(RBRInstrumentGen4_converse(
                instrument,
                "instrument outputformat sn=%s schedulelabel=%s datetime=%s"
                " crc=%s encoding=%s datatype=%s",
                outputformat->sn ? "on" : "off",
                outputformat->scheduleLabel ? "on" : "off",
                outputformat->dateTime ? "on" : "off",
                outputformat->crc ? "on" : "off",
                RBRInstrumentGen4Encoding_name(outputformat->encoding),
                RBRInstrumentGen4DataType_name(outputformat->dataType)));

    instrument->outputFormat = *outputformat;

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_factoryReset(
    RBRInstrumentGen4 *instrument)
{
    return RBRInstrumentGen4_converse(instrument, "instrument factory reset");
}

RBRInstrumentGen4Error RBRInstrumentGen4_reboot(RBRInstrumentGen4 *instrument,
                                        const int32_t delay)
{
    if (delay == 0)
    {
        RBR_TRY(RBRInstrumentGen4_sendCommand(instrument, "instrument reboot"));
    }
    else
    {
        RBR_TRY(RBRInstrumentGen4_sendCommand(instrument,
                                              "instrument reboot delay=%"
                                              PRId32,
                                              delay));
    }

    instrument->lastActivityTime = RBRINSTRUMENTGEN4_NO_ACTIVITY;
    return RBRINSTRUMENTGEN4_SUCCESS;
}
