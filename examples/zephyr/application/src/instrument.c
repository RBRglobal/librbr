/**
 * \file instrument.c
 *
 * \brief Instrument management helper implementations.
 *
 * \copyright
 * Copyright (c) 2025 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include "instrument.h"

RBRInstrumentError instrumentStart(RBRInstrument *instrument)
{
    RBRInstrumentError err;

    RBRInstrumentDeploymentStatus status;
    if ((err = RBRInstrument_disable(instrument, &status))
        != RBRINSTRUMENT_SUCCESS)
    {
        return err;
    }

    RBRInstrumentSampling sampling;
    if ((err = RBRInstrument_getSampling(instrument, &sampling))
        != RBRINSTRUMENT_SUCCESS)
    {
        return err;
    }
    sampling.mode = RBRINSTRUMENT_SAMPLING_CONTINUOUS;
    sampling.period = sampling.userPeriodLimit;
    if ((err = RBRInstrument_setSampling(instrument, &sampling))
        != RBRINSTRUMENT_SUCCESS)
    {
        return err;
    }

    RBRInstrumentDeployment deployment = {
        .startTime = RBRINSTRUMENT_DATETIME_MIN,
        .endTime = RBRINSTRUMENT_DATETIME_MAX
    };
    if ((err = RBRInstrument_setDeployment(instrument, &deployment))
        != RBRINSTRUMENT_SUCCESS)
    {
        return err;
    }

    if ((err = RBRInstrument_setNewMemoryFormat(
             instrument,
             RBRINSTRUMENT_MEMFORMAT_CALBIN00))
        != RBRINSTRUMENT_SUCCESS)
    {
        return err;
    }

    RBRInstrumentThresholding thresholding;
    err = RBRInstrument_getThresholding(instrument, &thresholding);
    if (err == RBRINSTRUMENT_SUCCESS && thresholding.enabled)
    {
        thresholding.enabled = false;
        RBRInstrument_setThresholding(instrument, &thresholding);
    }

    RBRInstrumentTwistActivation twistActivation;
    err = RBRInstrument_getTwistActivation(instrument, &twistActivation);
    if (err == RBRINSTRUMENT_SUCCESS && twistActivation.enabled)
    {
        twistActivation.enabled = false;
        RBRInstrument_setTwistActivation(instrument, &twistActivation);
    }

    if ((err = RBRInstrument_enable(instrument, true, &status))
        != RBRINSTRUMENT_SUCCESS)
    {
        return err;
    }

    return RBRINSTRUMENT_SUCCESS;
}
