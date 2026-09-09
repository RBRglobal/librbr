/**
 * \file RBRGen3Vehicle.c
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

const char *RBRGen3Direction_name(RBRGen3Direction direction)
{
    switch (direction)
    {
    case RBRGEN3_DIRECTION_ASCENDING:
        return "ascending";
    case RBRGEN3_DIRECTION_DESCENDING:
        return "descending";
    case RBRGEN3_DIRECTION_COUNT:
        return "direction count";
    case RBRGEN3_UNKNOWN_DIRECTION:
    default:
        return "unknown direction";
    }
}

const char *RBRGen3RegimesReference_name(
    RBRGen3RegimesReference reference)
{
    switch (reference)
    {
    case RBRGEN3_REFERENCE_ABSOLUTE:
        return "absolute";
    case RBRGEN3_REFERENCE_SEAPRESSURE:
        return "seapressure";
    case RBRGEN3_REFERENCE_COUNT:
        return "regimes reference count";
    case RBRGEN3_UNKNOWN_REFERENCE:
    default:
        return "unknown regimes reference";
    }
}

RBRGen3Error RBRGen3_getRegimes(
    RBRGen3 *instrument,
    RBRGen3Regimes *regimes)
{
    memset(regimes, 0, sizeof(RBRGen3Regimes));
    regimes->direction = RBRGEN3_UNKNOWN_DIRECTION;
    regimes->reference = RBRGEN3_UNKNOWN_REFERENCE;

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
            for (int i = 0; i < RBRGEN3_DIRECTION_COUNT; i++)
            {
                if (strcmp(RBRGen3Direction_name(i),
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
            for (int i = 0; i < RBRGEN3_REFERENCE_COUNT; i++)
            {
                if (strcmp(RBRGen3RegimesReference_name(i),
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

RBRGen3Error RBRGen3_setRegimes(
    RBRGen3 *instrument,
    const RBRGen3Regimes *regimes)
{
    if (regimes->direction < 0
        || regimes->direction >= RBRGEN3_DIRECTION_COUNT
        || regimes->count < 1
        || regimes->count > RBRGEN3_REGIME_MAX
        || regimes->reference < 0
        || regimes->reference >= RBRGEN3_REFERENCE_COUNT)
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    return RBRGen3_converse(
        instrument,
        "regimes direction = %s, count = %i, reference = %s",
        RBRGen3Direction_name(regimes->direction),
        regimes->count,
        RBRGen3RegimesReference_name(regimes->reference));
}

RBRGen3Error RBRGen3_getRegime(
    RBRGen3 *instrument,
    RBRGen3Regime *regime)
{
    RBRGen3RegimeIndex index = regime->index;

    if (index < 1 || index > RBRGEN3_REGIME_MAX)
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    memset(regime, 0, sizeof(RBRGen3Regime));

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

RBRGen3Error RBRGen3_setRegime(
    RBRGen3 *instrument,
    const RBRGen3Regime *regime)
{
    if (regime->index < 1
        || regime->index >= RBRGEN3_REGIME_MAX
        || regime->boundary < 0
        || regime->boundary > RBRGEN3_REGIME_BOUNDARY_MAX
        || regime->binSize < 0
        || regime->binSize > RBRGEN3_REGIME_BINSIZE_MAX
        || regime->samplingPeriod <= 0
        || regime->samplingPeriod > RBRGEN3_REGIME_SAMPLING_PERIOD_MAX
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

RBRGen3Error RBRGen3_getDirectionDependentSampling(
    RBRGen3 *instrument,
    RBRGen3DirectionDependentSampling *ddsampling)
{
    memset(ddsampling, 0, sizeof(RBRGen3DirectionDependentSampling));
    ddsampling->direction = RBRGEN3_UNKNOWN_DIRECTION;

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
            for (int i = 0; i < RBRGEN3_DIRECTION_COUNT; i++)
            {
                if (strcmp(RBRGen3Direction_name(i),
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

RBRGen3Error RBRGen3_setDirectionDependentSampling(
    RBRGen3 *instrument,
    RBRGen3DirectionDependentSampling *ddsampling)
{
    if (ddsampling->direction < 0
        || ddsampling->direction >= RBRGEN3_DIRECTION_COUNT
        || ddsampling->fastPeriod >= ddsampling->slowPeriod
        || ddsampling->fastPeriod <= 0
        || ddsampling->fastPeriod > RBRGEN3_SAMPLING_PERIOD_MAX
        || (ddsampling->fastPeriod >= 1000
            && ddsampling->fastPeriod % 1000 != 0)
        || ddsampling->slowPeriod <= 0
        || ddsampling->slowPeriod > RBRGEN3_SAMPLING_PERIOD_MAX
        || (ddsampling->slowPeriod >= 1000
            && ddsampling->slowPeriod % 1000 != 0))
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    return RBRGen3_converse(
        instrument,
        "ddsampling direction = %s, fastperiod = %i, slowperiod = %i, "
        "fastthreshold = %0.1f, slowthreshold = %0.1f",
        RBRGen3Direction_name(ddsampling->direction),
        ddsampling->fastPeriod,
        ddsampling->slowPeriod,
        (double) ddsampling->fastThreshold,
        (double) ddsampling->slowThreshold);
}
