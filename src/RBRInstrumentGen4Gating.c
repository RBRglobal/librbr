/**
 * \file RBRInstrumentGen4Gating.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for snprintf. */
#include <stdio.h>
/* Required for memset, strcmp. */
#include <string.h>

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Internal.h"
#include "RBRInstrumentGen4Gating.h"

const char *RBRInstrumentGen4GatingState_name(RBRInstrumentGen4GatingState state)
{
    switch (state)
    {
    case RBRINSTRUMENTGEN4_GATING_NA:
        return "n/a";
    case RBRINSTRUMENTGEN4_GATING_PAUSED:
        return "paused";
    case RBRINSTRUMENTGEN4_GATING_RUNNING:
        return "running";
    case RBRINSTRUMENTGEN4_GATING_COUNT:
        return "gating state count";
    case RBRINSTRUMENTGEN4_UNKNOWN_GATING:
    default:
        return "unknown gating state";
    }
}

const char *RBRInstrumentGen4ThresholdingCondition_name(
    RBRInstrumentGen4ThresholdingCondition condition)
{
    switch (condition)
    {
    case RBRINSTRUMENTGEN4_THRESHOLDING_ABOVE:
        return "above";
    case RBRINSTRUMENTGEN4_THRESHOLDING_BELOW:
        return "below";
    case RBRINSTRUMENTGEN4_THRESHOLDING_COUNT:
        return "thresholding condition count";
    case RBRINSTRUMENTGEN4_UNKNOWN_THRESHOLDING:
    default:
        return "unknown thresholding condition";
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_getThresholding(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Thresholding *threshold)
{
        (void)instrument;
        (void)threshold;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setThresholding(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Thresholding *threshold)
{
        (void)instrument;
        (void)threshold;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getTwistActivation(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4TwistActivation *twistActivation)
{
        (void)instrument;
        (void)twistActivation;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setTwistActivation(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4TwistActivation *twistActivation)
{
        (void)instrument;
        (void)twistActivation;
    return RBRINSTRUMENTGEN4_SUCCESS;
}
