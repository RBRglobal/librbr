/**
 * \file RBRInstrumentVehicle.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for memset, strcmp. */
#include <string.h>

#include "RBRInstrumentGen3.h"
#include "RBRInstrumentGen3Internal.h"

const char *RBRInstrumentDirection_name(RBRInstrumentDirection direction)
{
    switch (direction)
    {
    case RBRINSTRUMENT_DIRECTION_ASCENDING:
        return "ascending";
    case RBRINSTRUMENT_DIRECTION_DESCENDING:
        return "descending";
    case RBRINSTRUMENT_DIRECTION_COUNT:
        return "direction count";
    case RBRINSTRUMENT_UNKNOWN_DIRECTION:
    default:
        return "unknown direction";
    }
}

const char *RBRInstrumentRegimesReference_name(
    RBRInstrumentRegimesReference reference)
{
    switch (reference)
    {
    case RBRINSTRUMENT_REFERENCE_ABSOLUTE:
        return "absolute";
    case RBRINSTRUMENT_REFERENCE_SEAPRESSURE:
        return "seapressure";
    case RBRINSTRUMENT_REFERENCE_COUNT:
        return "regimes reference count";
    case RBRINSTRUMENT_UNKNOWN_REFERENCE:
    default:
        return "unknown regimes reference";
    }
}

RBRInstrumentError RBRInstrument_getRegimes(
    RBRInstrument *instrument,
    RBRInstrumentRegimes *regimes)
{
    memset(regimes, 0, sizeof(RBRInstrumentRegimes));
    regimes->direction = RBRINSTRUMENT_UNKNOWN_DIRECTION;
    regimes->reference = RBRINSTRUMENT_UNKNOWN_REFERENCE;

    RBR_TRY(RBRInstrument_converse(instrument, "regimes"));

    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    while (true)
    {
        RBRInstrument_parseResponse(instrument,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "direction") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENT_DIRECTION_COUNT; i++)
            {
                if (strcmp(RBRInstrumentDirection_name(i),
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
            for (int i = 0; i < RBRINSTRUMENT_REFERENCE_COUNT; i++)
            {
                if (strcmp(RBRInstrumentRegimesReference_name(i),
                           parameter.value) == 0)
                {
                    regimes->reference = i;
                    break;
                }
            }
        }
    }

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_setRegimes(
    RBRInstrument *instrument,
    const RBRInstrumentRegimes *regimes)
{
    if (regimes->direction < 0
        || regimes->direction >= RBRINSTRUMENT_DIRECTION_COUNT
        || regimes->count < 1
        || regimes->count > RBRINSTRUMENT_REGIME_MAX
        || regimes->reference < 0
        || regimes->reference >= RBRINSTRUMENT_REFERENCE_COUNT)
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrument_converse(
        instrument,
        "regimes direction = %s, count = %i, reference = %s",
        RBRInstrumentDirection_name(regimes->direction),
        regimes->count,
        RBRInstrumentRegimesReference_name(regimes->reference));
}

RBRInstrumentError RBRInstrument_getRegime(
    RBRInstrument *instrument,
    RBRInstrumentRegime *regime)
{
    RBRInstrumentRegimeIndex index = regime->index;

    if (index < 1 || index > RBRINSTRUMENT_REGIME_MAX)
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    memset(regime, 0, sizeof(RBRInstrumentRegime));

    RBR_TRY(RBRInstrument_converse(instrument, "regime %i", index));

    char *command = NULL;
    int32_t previousIndex = 0;
    RBRInstrumentResponseParameter parameter;
    while (true)
    {
        RBRInstrument_parseResponse(instrument,
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

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_setRegime(
    RBRInstrument *instrument,
    const RBRInstrumentRegime *regime)
{
    if (regime->index < 1
        || regime->index >= RBRINSTRUMENT_REGIME_MAX
        || regime->boundary < 0
        || regime->boundary > RBRINSTRUMENT_REGIME_BOUNDARY_MAX
        || regime->binSize < 0
        || regime->binSize > RBRINSTRUMENT_REGIME_BINSIZE_MAX
        || regime->samplingPeriod <= 0
        || regime->samplingPeriod > RBRINSTRUMENT_REGIME_SAMPLING_PERIOD_MAX
        || (regime->samplingPeriod >= 1000
            && regime->samplingPeriod % 1000 != 0))
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrument_converse(
        instrument,
        "regime %d boundary = %.0f, binsize = %0.1f, samplingperiod = %i",
        regime->index,
        (double) regime->boundary,
        (double) regime->binSize,
        regime->samplingPeriod);
}

RBRInstrumentError RBRInstrument_getDirectionDependentSampling(
    RBRInstrument *instrument,
    RBRInstrumentDirectionDependentSampling *ddsampling)
{
    memset(ddsampling, 0, sizeof(RBRInstrumentDirectionDependentSampling));
    ddsampling->direction = RBRINSTRUMENT_UNKNOWN_DIRECTION;

    RBR_TRY(RBRInstrument_converse(instrument, "ddsampling"));

    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    while (true)
    {
        RBRInstrument_parseResponse(instrument,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "direction") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENT_DIRECTION_COUNT; i++)
            {
                if (strcmp(RBRInstrumentDirection_name(i),
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

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_setDirectionDependentSampling(
    RBRInstrument *instrument,
    RBRInstrumentDirectionDependentSampling *ddsampling)
{
    if (ddsampling->direction < 0
        || ddsampling->direction >= RBRINSTRUMENT_DIRECTION_COUNT
        || ddsampling->fastPeriod >= ddsampling->slowPeriod
        || ddsampling->fastPeriod <= 0
        || ddsampling->fastPeriod > RBRINSTRUMENT_SAMPLING_PERIOD_MAX
        || (ddsampling->fastPeriod >= 1000
            && ddsampling->fastPeriod % 1000 != 0)
        || ddsampling->slowPeriod <= 0
        || ddsampling->slowPeriod > RBRINSTRUMENT_SAMPLING_PERIOD_MAX
        || (ddsampling->slowPeriod >= 1000
            && ddsampling->slowPeriod % 1000 != 0))
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrument_converse(
        instrument,
        "ddsampling direction = %s, fastperiod = %i, slowperiod = %i, "
        "fastthreshold = %0.1f, slowthreshold = %0.1f",
        RBRInstrumentDirection_name(ddsampling->direction),
        ddsampling->fastPeriod,
        ddsampling->slowPeriod,
        (double) ddsampling->fastThreshold,
        (double) ddsampling->slowThreshold);
}
