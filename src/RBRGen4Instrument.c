/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file RBRGen4Instrument.c
 *
 * \brief Library implementation.
 */

/* Required for memcpy, memset, strcmp, strstr. */
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
            snprintf(id->fwVersion, sizeof(id->fwVersion), "%s", parameter.value);
        } else if (strcmp(parameter.key, "semver") == 0) {
            snprintf(id->semver, sizeof(id->semver), "%s", parameter.value);
        } else if (strcmp(parameter.key, "apiversion") == 0) {
            snprintf(id->apiVersion, sizeof(id->apiVersion), "%s", parameter.value);
        } else if (strcmp(parameter.key, "sn") == 0) {
            id->sn = strtol(parameter.value, NULL, 10);
        } else if (strcmp(parameter.key, "fwtype") == 0) {
            id->fwType = strtol(parameter.value, NULL, 10);
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
            snprintf(instrumentInfo->fwVersion,
                     sizeof(instrumentInfo->fwVersion),
                     "%s",
                     parameter.value);
        } else if (strcmp(parameter.key, "semver") == 0) {
            snprintf(instrumentInfo->semver, sizeof(instrumentInfo->semver), "%s", parameter.value);
        } else if (strcmp(parameter.key, "fwtype") == 0) {
            instrumentInfo->fwType = strtol(parameter.value, NULL, 10);
        } else if (strcmp(parameter.key, "fwlock") == 0) {
            instrumentInfo->fwLock = (strcmp(parameter.value, "on") == 0);
        } else if (strcmp(parameter.key, "datatype") == 0) {
            instrumentInfo->dataType = RBRGen4DataType_parse(parameter.value);
        } else if (strcmp(parameter.key, "name") == 0) {
            snprintf(instrumentInfo->name, sizeof(instrumentInfo->name), "%s", parameter.value);
        } else if (strcmp(parameter.key, "apiversion") == 0) {
            snprintf(instrumentInfo->apiVersion,
                     sizeof(instrumentInfo->apiVersion),
                     "%s",
                     parameter.value);
        }
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_getOutputFormat(RBRGen4 *conn, RBRGen4OutputFormat *outputFormat)
{
    memset(outputFormat, 0, sizeof(RBRGen4OutputFormat));
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
    if (outputFormat->dataType < 0 || outputFormat->dataType >= RBRGEN4_DATA_TYPE_COUNT) {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    RBR_TRY(RBRGen4_converse(conn,
                             "instrument outputformat sn=%s schedulelabel=%s datetime=%s"
                             " crc=%s datatype=%s",
                             outputFormat->sn ? "on" : "off",
                             outputFormat->scheduleLabel ? "on" : "off",
                             outputFormat->dateTime ? "on" : "off",
                             outputFormat->crc ? "on" : "off",
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
