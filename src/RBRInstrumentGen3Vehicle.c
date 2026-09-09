/**
 * \file RBRInstrumentGen3Vehicle.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for memset, strcmp. */
#include <string.h>

#include "RBRGen3.h"
#include "RBRGen3Internal.h"

const char *RBRInstrumentGen3Direction_name(RBRInstrumentGen3Direction direction)
{
    switch (direction)
    {
    case RBRINSTRUMENTGEN3_DIRECTION_ASCENDING:
        return "ascending";
    case RBRINSTRUMENTGEN3_DIRECTION_DESCENDING:
        return "descending";
    case RBRINSTRUMENTGEN3_DIRECTION_COUNT:
        return "direction count";
    case RBRINSTRUMENTGEN3_UNKNOWN_DIRECTION:
    default:
        return "unknown direction";
    }
}

const char *RBRInstrumentGen3RegimesReference_name(
    RBRInstrumentGen3RegimesReference reference)
{
    switch (reference)
    {
    case RBRINSTRUMENTGEN3_REFERENCE_ABSOLUTE:
        return "absolute";
    case RBRINSTRUMENTGEN3_REFERENCE_SEAPRESSURE:
        return "seapressure";
    case RBRINSTRUMENTGEN3_REFERENCE_COUNT:
        return "regimes reference count";
    case RBRINSTRUMENTGEN3_UNKNOWN_REFERENCE:
    default:
        return "unknown regimes reference";
    }
}

RBRGen3Error RBRInstrumentGen3_getRegimes(
    RBRGen3 *instrument,
    RBRInstrumentGen3Regimes *regimes)
{
    memset(regimes, 0, sizeof(RBRInstrumentGen3Regimes));
    regimes->direction = RBRINSTRUMENTGEN3_UNKNOWN_DIRECTION;
    regimes->reference = RBRINSTRUMENTGEN3_UNKNOWN_REFERENCE;

    RBR_TRY(RBRGen3_converse(instrument, "regimes"));

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true)
    {
        RBRGen3_parseResponse(instrument,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "direction") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENTGEN3_DIRECTION_COUNT; i++)
            {
                if (strcmp(RBRInstrumentGen3Direction_name(i),
                           parameter.value) == 0)
                {
                    regimes->direction = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "count") == 0)
        {
            regimes->count = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "reference") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENTGEN3_REFERENCE_COUNT; i++)
            {
                if (strcmp(RBRInstrumentGen3RegimesReference_name(i),
                           parameter.value) == 0)
                {
                    regimes->reference = i;
                    break;
                }
            }
        }
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRInstrumentGen3_setRegimes(
    RBRGen3 *instrument,
    const RBRInstrumentGen3Regimes *regimes)
{
    if (regimes->direction < 0
        || regimes->direction >= RBRINSTRUMENTGEN3_DIRECTION_COUNT
        || regimes->count < 1
        || regimes->count > RBRINSTRUMENTGEN3_REGIME_MAX
        || regimes->reference < 0
        || regimes->reference >= RBRINSTRUMENTGEN3_REFERENCE_COUNT)
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    return RBRGen3_converse(
        instrument,
        "regimes direction = %s, count = %i, reference = %s",
        RBRInstrumentGen3Direction_name(regimes->direction),
        regimes->count,
        RBRInstrumentGen3RegimesReference_name(regimes->reference));
}

RBRGen3Error RBRInstrumentGen3_getRegime(
    RBRGen3 *instrument,
    RBRInstrumentGen3Regime *regime)
{
    RBRInstrumentGen3RegimeIndex index = regime->index;

    if (index < 1 || index > RBRINSTRUMENTGEN3_REGIME_MAX)
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    memset(regime, 0, sizeof(RBRInstrumentGen3Regime));

    RBR_TRY(RBRGen3_converse(instrument, "regime %i", index));

    char *command = NULL;
    int32_t previousIndex = 0;
    RBRGen3ResponseParameter parameter;
    while (true)
    {
        RBRGen3_parseResponse(instrument,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (parameter.index != previousIndex)
        {
            previousIndex = parameter.index;
            regime->index = strtol(parameter.indexValue, NULL, 10);
        }

        if (strcmp(parameter.key, "boundary") == 0)
        {
            regime->boundary = strtod(parameter.value, NULL);
        }
        else if (strcmp(parameter.key, "binsize") == 0)
        {
            regime->binSize = strtod(parameter.value, NULL);
        }
        else if (strcmp(parameter.key, "samplingperiod") == 0)
        {
            regime->samplingPeriod = strtol(parameter.value, NULL, 10);
        }
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRInstrumentGen3_setRegime(
    RBRGen3 *instrument,
    const RBRInstrumentGen3Regime *regime)
{
    if (regime->index < 1
        || regime->index >= RBRINSTRUMENTGEN3_REGIME_MAX
        || regime->boundary < 0
        || regime->boundary > RBRINSTRUMENTGEN3_REGIME_BOUNDARY_MAX
        || regime->binSize < 0
        || regime->binSize > RBRINSTRUMENTGEN3_REGIME_BINSIZE_MAX
        || regime->samplingPeriod <= 0
        || regime->samplingPeriod > RBRINSTRUMENTGEN3_REGIME_SAMPLING_PERIOD_MAX
        || (regime->samplingPeriod >= 1000
            && regime->samplingPeriod % 1000 != 0))
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    return RBRGen3_converse(
        instrument,
        "regime %d boundary = %.0f, binsize = %0.1f, samplingperiod = %i",
        regime->index,
        (double) regime->boundary,
        (double) regime->binSize,
        regime->samplingPeriod);
}

RBRGen3Error RBRInstrumentGen3_getDirectionDependentSampling(
    RBRGen3 *instrument,
    RBRInstrumentGen3DirectionDependentSampling *ddsampling)
{
    memset(ddsampling, 0, sizeof(RBRInstrumentGen3DirectionDependentSampling));
    ddsampling->direction = RBRINSTRUMENTGEN3_UNKNOWN_DIRECTION;

    RBR_TRY(RBRGen3_converse(instrument, "ddsampling"));

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true)
    {
        RBRGen3_parseResponse(instrument,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "direction") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENTGEN3_DIRECTION_COUNT; i++)
            {
                if (strcmp(RBRInstrumentGen3Direction_name(i),
                           parameter.value) == 0)
                {
                    ddsampling->direction = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "fastperiod") == 0)
        {
            ddsampling->fastPeriod = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "slowperiod") == 0)
        {
            ddsampling->slowPeriod = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "fastthreshold") == 0)
        {
            ddsampling->fastThreshold = strtod(parameter.value, NULL);
        }
        else if (strcmp(parameter.key, "slowthreshold") == 0)
        {
            ddsampling->slowThreshold = strtod(parameter.value, NULL);
        }
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRInstrumentGen3_setDirectionDependentSampling(
    RBRGen3 *instrument,
    RBRInstrumentGen3DirectionDependentSampling *ddsampling)
{
    if (ddsampling->direction < 0
        || ddsampling->direction >= RBRINSTRUMENTGEN3_DIRECTION_COUNT
        || ddsampling->fastPeriod >= ddsampling->slowPeriod
        || ddsampling->fastPeriod <= 0
        || ddsampling->fastPeriod > RBRINSTRUMENTGEN3_SAMPLING_PERIOD_MAX
        || (ddsampling->fastPeriod >= 1000
            && ddsampling->fastPeriod % 1000 != 0)
        || ddsampling->slowPeriod <= 0
        || ddsampling->slowPeriod > RBRINSTRUMENTGEN3_SAMPLING_PERIOD_MAX
        || (ddsampling->slowPeriod >= 1000
            && ddsampling->slowPeriod % 1000 != 0))
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    return RBRGen3_converse(
        instrument,
        "ddsampling direction = %s, fastperiod = %i, slowperiod = %i, "
        "fastthreshold = %0.1f, slowthreshold = %0.1f",
        RBRInstrumentGen3Direction_name(ddsampling->direction),
        ddsampling->fastPeriod,
        ddsampling->slowPeriod,
        (double) ddsampling->fastThreshold,
        (double) ddsampling->slowThreshold);
}
