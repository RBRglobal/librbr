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

RBRInstrumentGen3Error instrumentStart(RBRInstrumentGen3 *instrument)
{
    RBRInstrumentGen3Error err;

    RBRInstrumentGen3DeploymentStatus status;
    if ((err = RBRInstrumentGen3_disable(instrument, &status))
        != RBRINSTRUMENTGEN3_SUCCESS)
    {
        return err;
    }

    RBRInstrumentGen3Sampling sampling;
    if ((err = RBRInstrumentGen3_getSampling(instrument, &sampling))
        != RBRINSTRUMENTGEN3_SUCCESS)
    {
        return err;
    }
    sampling.mode = RBRINSTRUMENTGEN3_SAMPLING_CONTINUOUS;
    sampling.period = sampling.userPeriodLimit;
    if ((err = RBRInstrumentGen3_setSampling(instrument, &sampling))
        != RBRINSTRUMENTGEN3_SUCCESS)
    {
        return err;
    }

    RBRInstrumentGen3Deployment deployment = {
        .startTime = RBRINSTRUMENTGEN3_DATETIME_MIN,
        .endTime = RBRINSTRUMENTGEN3_DATETIME_MAX
    };
    if ((err = RBRInstrumentGen3_setDeployment(instrument, &deployment))
        != RBRINSTRUMENTGEN3_SUCCESS)
    {
        return err;
    }

    if ((err = RBRInstrumentGen3_setNewMemoryFormat(
             instrument,
             RBRINSTRUMENTGEN3_MEMFORMAT_CALBIN00))
        != RBRINSTRUMENTGEN3_SUCCESS)
    {
        return err;
    }

    RBRInstrumentGen3Thresholding thresholding;
    err = RBRInstrumentGen3_getThresholding(instrument, &thresholding);
    if (err == RBRINSTRUMENTGEN3_SUCCESS && thresholding.enabled)
    {
        thresholding.enabled = false;
        RBRInstrumentGen3_setThresholding(instrument, &thresholding);
    }

    RBRInstrumentGen3TwistActivation twistActivation;
    err = RBRInstrumentGen3_getTwistActivation(instrument, &twistActivation);
    if (err == RBRINSTRUMENTGEN3_SUCCESS && twistActivation.enabled)
    {
        twistActivation.enabled = false;
        RBRInstrumentGen3_setTwistActivation(instrument, &twistActivation);
    }

    if ((err = RBRInstrumentGen3_enable(instrument, true, &status))
        != RBRINSTRUMENTGEN3_SUCCESS)
    {
        return err;
    }

    return RBRINSTRUMENTGEN3_SUCCESS;
}
