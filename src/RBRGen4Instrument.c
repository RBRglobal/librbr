/**
 * \file RBRGen4Instrument.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for isspace. */
#include <ctype.h>
/* Required for memcpy, memset, strchr, strcmp, strlen, strstr. */
#include <string.h>
/* Required for snprintf. */
#include <stdio.h>
/* Required for strtod, strtol. */
#include <stdlib.h>

#include "RBRGen4.h"
#include "RBRGen4Internal.h"
#include "RBRGen4Instrument.h"

const char *RBRGen4InstrumentState_name(RBRGen4InstrumentState state)
{
    switch (state) {
    case RBRGEN4_INSTRUMENT_STATE_DISABLED:
        return "disabled";
    case RBRGEN4_INSTRUMENT_STATE_ENABLED:
        return "enabled";
    case RBRGEN4_INSTRUMENT_STATE_COUNT:
        return "instrument state count";
    case RBRGEN4_UNKNOWN_INSTRUMENT_STATE:
    default:
        return "unknown instrument state";
    }
}

/*
 * The `id` command separates its parameters with commas and pads its
 * assignments with spaces (`id model = L4, version = 2.0.0`), so the Gen4
 * response tokenizer cannot be used on it.
 */
#define LEGACY_ID_PARAMETER_SEPARATOR ','
#define LEGACY_ID_ASSIGNMENT          '='

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
static void RBRGen4_copyTrimmed(char *destination, size_t size, const char *begin, const char *end)
{
    while (begin < end && isspace((unsigned char) *begin)) {
        ++begin;
    }
    while (end > begin && isspace((unsigned char) *(end - 1))) {
        --end;
    }

    size_t length = (size_t) (end - begin);
    if (length > size - 1) {
        length = size - 1;
    }
    memcpy(destination, begin, length);
    destination[length] = '\0';
}

RBRGen4Error RBRGen4_getId(RBRGen4 *conn, RBRGen4Id *id)
{
    memset(id, 0, sizeof(RBRGen4Id));

    RBR_TRY(RBRGen4_converse(conn, "id"));

    const char *cursor = conn->response.response;
    if (cursor == NULL) {
        return RBRGEN4_SUCCESS;
    }

    /* Step over the command name which the instrument echoes back. */
    cursor = strchr(cursor, ' ');

    while (cursor != NULL) {
        char key[LEGACY_ID_KEY_MAX + 1];
        /* Long enough for any 32-bit decimal value and its sign. */
        char number[12];

        const char *assignment = strchr(cursor, LEGACY_ID_ASSIGNMENT);
        if (assignment == NULL) {
            break;
        }

        const char *end = strchr(assignment, LEGACY_ID_PARAMETER_SEPARATOR);
        if (end == NULL) {
            end = assignment + strlen(assignment);
        }

        RBRGen4_copyTrimmed(key, sizeof(key), cursor, assignment);

        if (strcmp(key, "model") == 0) {
            RBRGen4_copyTrimmed(id->model, sizeof(id->model), assignment + 1, end);
        } else if (strcmp(key, "version") == 0) {
            RBRGen4_copyTrimmed(id->fwversion, sizeof(id->fwversion), assignment + 1, end);
        } else if (strcmp(key, "serial") == 0) {
            RBRGen4_copyTrimmed(number, sizeof(number), assignment + 1, end);
            id->sn = strtol(number, NULL, 10);
        } else if (strcmp(key, "fwtype") == 0) {
            RBRGen4_copyTrimmed(number, sizeof(number), assignment + 1, end);
            id->fwtype = strtol(number, NULL, 10);
        }

        cursor = *end == '\0' ? NULL : end + 1;
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_getId4(RBRGen4 *conn, RBRGen4Id4 *id)
{
    memset(id, 0, sizeof(RBRGen4Id4));

    RBR_TRY(RBRGen4_converse(conn, "id4"));
    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    do {
        RBRGen4_parseResponse(conn, &command, &parameter);
        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        }
        if (strcmp(parameter.key, "model") == 0) {
            snprintf(id->model, sizeof(id->model), "%s", parameter.value);
        } else if (strcmp(parameter.key, "fwversion") == 0) {
            snprintf(id->fwversion, sizeof(id->fwversion), "%s", parameter.value);
        } else if (strcmp(parameter.key, "semver") == 0) {
            snprintf(id->semver, sizeof(id->semver), "%s", parameter.value);
        } else if (strcmp(parameter.key, "sn") == 0) {
            id->sn = strtol(parameter.value, NULL, 10);
        } else if (strcmp(parameter.key, "fwtype") == 0) {
            id->fwtype = strtol(parameter.value, NULL, 10);
        }
    } while (true);
    if (id != &conn->id) {
        memcpy(&conn->id, id, sizeof(RBRGen4Id4));
    }

    return RBRGEN4_SUCCESS;
}

const char *RBRGen4PowerSource_name(RBRGen4PowerSource source)
{
    switch (source) {
    case RBRGEN4_POWER_SOURCE_USB:
        return "usb";
    case RBRGEN4_POWER_SOURCE_INTERNAL:
        return "int";
    case RBRGEN4_POWER_SOURCE_EXTERNAL:
        return "ext";
    case RBRGEN4_POWER_SOURCE_COUNT:
        return "power source count";
    default:
        return "unknown power source";
    }
}

RBRGen4Error RBRGen4_getPowerSource(RBRGen4 *conn, RBRGen4PowerSource *powerSource)
{
    *powerSource = RBRGEN4_POWER_SOURCE_UNKNOWN;

    RBR_TRY(RBRGen4_converse(conn, "instrument power"));

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true) {
        RBRGen4_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "source") == 0) {
            for (int i = 0; i < RBRGEN4_POWER_SOURCE_COUNT; i++) {
                if (strcmp(RBRGen4PowerSource_name(i), parameter.value) == 0) {
                    *powerSource = i;
                    break;
                }
            }
        }
    }

    return RBRGEN4_SUCCESS;
}

const char *RBRGen4InternalBatteryType_name(RBRGen4InternalBatteryType type)
{
    switch (type) {
    case RBRGEN4_INTERNAL_BATTERY_NONE:
        return "none";
    case RBRGEN4_INTERNAL_BATTERY_LISOCL2:
        return "lisocl2";
    case RBRGEN4_INTERNAL_BATTERY_LIFES2:
        return "lifes2";
    case RBRGEN4_INTERNAL_BATTERY_ZNMNO2:
        return "znmno2";
    case RBRGEN4_INTERNAL_BATTERY_LINIMNCO:
        return "linimnco";
    case RBRGEN4_INTERNAL_BATTERY_NIMH:
        return "nimh";
    case RBRGEN4_INTERNAL_BATTERY_COUNT:
        return "internal battery type count";
    default:
        return "unknown internal battery type";
    }
}

const char *RBRGen4InternalBatteryType_displayName(RBRGen4InternalBatteryType type)
{
    switch (type) {
    case RBRGEN4_INTERNAL_BATTERY_NONE:
        return "none";
    case RBRGEN4_INTERNAL_BATTERY_LISOCL2:
        return "Li-SOCl₂";
    case RBRGEN4_INTERNAL_BATTERY_LIFES2:
        return "Li-FeS₂";
    case RBRGEN4_INTERNAL_BATTERY_ZNMNO2:
        return "Zn-MnO₂";
    case RBRGEN4_INTERNAL_BATTERY_LINIMNCO:
        return "Li-NiMnCo";
    case RBRGEN4_INTERNAL_BATTERY_NIMH:
        return "NiMH";
    case RBRGEN4_INTERNAL_BATTERY_COUNT:
        return "internal battery type count";
    default:
        return "unknown internal battery type";
    }
}

RBRGen4Error RBRGen4_getPowerInternal(RBRGen4 *conn, RBRGen4PowerInternal *power)
{
    memset(power, 0, sizeof(RBRGen4PowerInternal));
    power->batteryType = RBRGEN4_UNKNOWN_INTERNAL_BATTERY;

    RBR_TRY(RBRGen4_converse(conn, "instrument power internal"));

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true) {
        RBRGen4_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "voltage") == 0) {
            power->voltage = strtod(parameter.value, NULL);
        } else if (strcmp(parameter.key, "batterytype") == 0) {
            for (int i = 0; i < RBRGEN4_INTERNAL_BATTERY_COUNT; i++) {
                if (strcmp(RBRGen4InternalBatteryType_name(i), parameter.value) == 0) {
                    power->batteryType = i;
                    break;
                }
            }
        } else if (strcmp(parameter.key, "used") == 0) {
            power->used = strtod(parameter.value, NULL);
        }
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_setPowerInternalBatteryType(RBRGen4 *conn,
                                                 const RBRGen4InternalBatteryType type)
{
    if (type < 0 || type >= RBRGEN4_INTERNAL_BATTERY_COUNT) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGen4_converse(
        conn, "instrument power internal batterytype=%s", RBRGen4InternalBatteryType_name(type));
}

RBRGen4Error RBRGen4_resetPowerInternalUsed(RBRGen4 *conn)
{
    return RBRGen4_converse(conn, "instrument power internal used=0");
}

const char *RBRGen4ExternalBatteryType_name(RBRGen4ExternalBatteryType type)
{
    switch (type) {
    case RBRGEN4_EXTERNAL_BATTERY_NONE:
        return "none";
    case RBRGEN4_EXTERNAL_BATTERY_FERMATA_LISOCL2:
        return "fermata_lisocl2";
    case RBRGEN4_EXTERNAL_BATTERY_FERMATA_ZNMNO2:
        return "fermata_znmno2";
    case RBRGEN4_EXTERNAL_BATTERY_FERMETTE_LIMNO2:
        return "fermette_limno2";
    case RBRGEN4_EXTERNAL_BATTERY_FERMETTE3_LISOCL2:
        return "fermette3_lisocl2";
    case RBRGEN4_EXTERNAL_BATTERY_FERMETTE3_LIFES2:
        return "fermette3_lifes2";
    case RBRGEN4_EXTERNAL_BATTERY_FERMETTE3_ZNMNO2:
        return "fermette3_znmno2";
    case RBRGEN4_EXTERNAL_BATTERY_FERMETTE3_LINIMNCO:
        return "fermette3_linimnco";
    case RBRGEN4_EXTERNAL_BATTERY_FERMETTE3_NIMH:
        return "fermette3_nimh";
    case RBRGEN4_EXTERNAL_BATTERY_FERMATA_NIMH:
        return "fermata_nimh";
    case RBRGEN4_EXTERNAL_BATTERY_OTHER:
        return "other";
    case RBRGEN4_EXTERNAL_BATTERY_COUNT:
        return "external battery type count";
    default:
        return "unknown external battery type";
    }
}

const char *RBRGen4ExternalBatteryType_displayName(RBRGen4ExternalBatteryType type)
{
    switch (type) {
    case RBRGEN4_EXTERNAL_BATTERY_NONE:
        return "none";
    case RBRGEN4_EXTERNAL_BATTERY_FERMATA_LISOCL2:
        return "RBRfermata Li-SOCl₂";
    case RBRGEN4_EXTERNAL_BATTERY_FERMATA_ZNMNO2:
        return "RBRfermata Zn-MnO₂";
    case RBRGEN4_EXTERNAL_BATTERY_FERMETTE_LIMNO2:
        return "RBRfermette Li-MnO₂";
    case RBRGEN4_EXTERNAL_BATTERY_FERMETTE3_LISOCL2:
        return "RBRfermette³ Li-SOCl₂";
    case RBRGEN4_EXTERNAL_BATTERY_FERMETTE3_LIFES2:
        return "RBRfermette³ Li-FeS₂";
    case RBRGEN4_EXTERNAL_BATTERY_FERMETTE3_ZNMNO2:
        return "RBRfermette³ Zn-MnO₂";
    case RBRGEN4_EXTERNAL_BATTERY_FERMETTE3_LINIMNCO:
        return "RBRfermette³ Li-NiMnCo";
    case RBRGEN4_EXTERNAL_BATTERY_FERMETTE3_NIMH:
        return "RBRfermette³ NiMH";
    case RBRGEN4_EXTERNAL_BATTERY_FERMATA_NIMH:
        return "RBRfermata NiMH";
    case RBRGEN4_EXTERNAL_BATTERY_OTHER:
        return "other";
    case RBRGEN4_EXTERNAL_BATTERY_COUNT:
        return "external battery type count";
    default:
        return "unknown external battery type";
    }
}

RBRGen4Error RBRGen4_getPowerExternal(RBRGen4 *conn, RBRGen4PowerExternal *power)
{
    memset(power, 0, sizeof(RBRGen4PowerExternal));
    power->batteryType = RBRGEN4_UNKNOWN_EXTERNAL_BATTERY;

    RBR_TRY(RBRGen4_converse(conn, "instrument power external"));

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true) {
        RBRGen4_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "voltage") == 0) {
            power->voltage = strtod(parameter.value, NULL);
        } else if (strcmp(parameter.key, "batterytype") == 0) {
            for (int i = 0; i < RBRGEN4_EXTERNAL_BATTERY_COUNT; i++) {
                if (strcmp(RBRGen4ExternalBatteryType_name(i), parameter.value) == 0) {
                    power->batteryType = i;
                    break;
                }
            }
        } else if (strcmp(parameter.key, "used") == 0) {
            power->used = strtod(parameter.value, NULL);
        }
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_setPowerExternalBatteryType(RBRGen4 *conn,
                                                 const RBRGen4ExternalBatteryType type)
{
    if (type < 0 || type >= RBRGEN4_EXTERNAL_BATTERY_COUNT) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGen4_converse(
        conn, "instrument power external batterytype=%s", RBRGen4ExternalBatteryType_name(type));
}

RBRGen4Error RBRGen4_resetPowerExternalUsed(RBRGen4 *conn)
{
    return RBRGen4_converse(conn, "instrument power external used=0");
}

/**
 * \brief Resolve a `datatype` parameter value to its enum member.
 *
 * Matched against RBRGen4DataType_name() so that the accepted
 * spellings cannot drift from the ones the library emits.
 *
 * \param [in] value the parameter value reported by the instrument
 * \return the corresponding data type
 * \return #RBRGEN4_UNKNOWN_DATA_TYPE when the value is unrecognized
 */
static RBRGen4DataType RBRGen4DataType_parse(const char *value)
{
    for (int32_t dataType = 0; dataType < RBRGEN4_DATA_TYPE_COUNT; ++dataType) {
        if (strcmp(value, RBRGen4DataType_name((RBRGen4DataType) dataType)) == 0) {
            return (RBRGen4DataType) dataType;
        }
    }

    return RBRGEN4_UNKNOWN_DATA_TYPE;
}

RBRGen4Error RBRGen4_getInstrument(RBRGen4 *conn, RBRGen4Instrument *instrumentInfo)
{
    memset(instrumentInfo, 0, sizeof(RBRGen4Instrument));
    instrumentInfo->state = RBRGEN4_UNKNOWN_INSTRUMENT_STATE;
    instrumentInfo->dataType = RBRGEN4_UNKNOWN_DATA_TYPE;

    RBR_TRY(RBRGen4_converse(conn, "instrument"));

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true) {
        RBRGen4_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "state") == 0) {
            if (strcmp(parameter.value, "disabled") == 0) {
                instrumentInfo->state = RBRGEN4_INSTRUMENT_STATE_DISABLED;
            } else if (strcmp(parameter.value, "enabled") == 0) {
                instrumentInfo->state = RBRGEN4_INSTRUMENT_STATE_ENABLED;
            }
        } else if (strcmp(parameter.key, "sn") == 0) {
            instrumentInfo->sn = strtol(parameter.value, NULL, 10);
        } else if (strcmp(parameter.key, "model") == 0) {
            snprintf(instrumentInfo->model, sizeof(instrumentInfo->model), "%s", parameter.value);
        } else if (strcmp(parameter.key, "pn") == 0) {
            snprintf(instrumentInfo->pn, sizeof(instrumentInfo->pn), "%s", parameter.value);
        } else if (strcmp(parameter.key, "fwversion") == 0) {
            snprintf(instrumentInfo->fwversion,
                     sizeof(instrumentInfo->fwversion),
                     "%s",
                     parameter.value);
        } else if (strcmp(parameter.key, "semver") == 0) {
            snprintf(instrumentInfo->semver, sizeof(instrumentInfo->semver), "%s", parameter.value);
        } else if (strcmp(parameter.key, "fwtype") == 0) {
            instrumentInfo->fwtype = strtol(parameter.value, NULL, 10);
        } else if (strcmp(parameter.key, "fwlock") == 0) {
            instrumentInfo->fwLock = (strcmp(parameter.value, "on") == 0);
        } else if (strcmp(parameter.key, "datatype") == 0) {
            instrumentInfo->dataType = RBRGen4DataType_parse(parameter.value);
        } else if (strcmp(parameter.key, "name") == 0) {
            snprintf(instrumentInfo->name, sizeof(instrumentInfo->name), "%s", parameter.value);
        }
    }

    return RBRGEN4_SUCCESS;
}

/**
 * \brief Resolve an `encoding` parameter value to its enum member.
 *
 * \param [in] value the parameter value reported by the instrument
 * \return the corresponding encoding
 * \return #RBRGEN4_UNKNOWN_ENCODING when the value is unrecognized
 */
static RBRGen4Encoding RBRGen4Encoding_parse(const char *value)
{
    for (int32_t encoding = 0; encoding < RBRGEN4_ENCODING_COUNT; ++encoding) {
        if (strcmp(value, RBRGen4Encoding_name((RBRGen4Encoding) encoding)) == 0) {
            return (RBRGen4Encoding) encoding;
        }
    }

    return RBRGEN4_UNKNOWN_ENCODING;
}

RBRGen4Error RBRGen4_getOutputFormat(RBRGen4 *conn, RBRGen4OutputFormat *outputFormat)
{
    memset(outputFormat, 0, sizeof(RBRGen4OutputFormat));
    outputFormat->encoding = RBRGEN4_UNKNOWN_ENCODING;
    outputFormat->dataType = RBRGEN4_UNKNOWN_DATA_TYPE;

    RBR_TRY(RBRGen4_converse(conn, "instrument outputformat"));

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true) {
        RBRGen4_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        }

        bool enabled = strcmp(parameter.value, "on") == 0;

        if (strcmp(parameter.key, "sn") == 0) {
            outputFormat->sn = enabled;
        } else if (strcmp(parameter.key, "schedulelabel") == 0) {
            outputFormat->scheduleLabel = enabled;
        } else if (strcmp(parameter.key, "datetime") == 0) {
            outputFormat->dateTime = enabled;
        } else if (strcmp(parameter.key, "crc") == 0) {
            outputFormat->crc = enabled;
        } else if (strcmp(parameter.key, "encoding") == 0) {
            outputFormat->encoding = RBRGen4Encoding_parse(parameter.value);
        } else if (strcmp(parameter.key, "datatype") == 0) {
            outputFormat->dataType = RBRGen4DataType_parse(parameter.value);
        }
    }

    /* The caller may have asked us to populate the cache itself. */
    if (outputFormat != &conn->outputFormat) {
        conn->outputFormat = *outputFormat;
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_setOutputFormat(RBRGen4 *conn, const RBRGen4OutputFormat *outputFormat)
{
    if (outputFormat->encoding < 0 || outputFormat->encoding >= RBRGEN4_ENCODING_COUNT ||
        outputFormat->dataType < 0 || outputFormat->dataType >= RBRGEN4_DATA_TYPE_COUNT) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    RBR_TRY(RBRGen4_converse(conn,
                             "instrument outputformat sn=%s schedulelabel=%s datetime=%s"
                             " crc=%s encoding=%s datatype=%s",
                             outputFormat->sn ? "on" : "off",
                             outputFormat->scheduleLabel ? "on" : "off",
                             outputFormat->dateTime ? "on" : "off",
                             outputFormat->crc ? "on" : "off",
                             RBRGen4Encoding_name(outputFormat->encoding),
                             RBRGen4DataType_name(outputFormat->dataType)));

    conn->outputFormat = *outputFormat;

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_factoryReset(RBRGen4 *conn)
{
    return RBRGen4_converse(conn, "instrument factory reset");
}

RBRGen4Error RBRGen4_reboot(RBRGen4 *conn, const int32_t delay)
{
    if (delay == 0) {
        RBR_TRY(RBRGen4_sendCommand(conn, "instrument reboot"));
    } else {
        RBR_TRY(RBRGen4_sendCommand(conn, "instrument reboot delay=%" PRId32, delay));
    }

    conn->lastActivityTime = RBRGEN4_NO_ACTIVITY;
    return RBRGEN4_SUCCESS;
}
