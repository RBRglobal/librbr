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

RBRGen3Error instrumentStart(RBRGen3 *instrument)
{
    RBRGen3Error err;

    RBRInstrumentGen3DeploymentStatus status;
    if ((err = RBRGen3_disable(instrument, &status))
        != RBRGEN3_SUCCESS)
    {
        return err;
    }

    RBRInstrumentGen3Sampling sampling;
    if ((err = RBRInstrumentGen3_getSampling(instrument, &sampling))
        != RBRGEN3_SUCCESS)
    {
        return err;
    }
    sampling.mode = RBRINSTRUMENTGEN3_SAMPLING_CONTINUOUS;
    sampling.period = sampling.userPeriodLimit;
    if ((err = RBRInstrumentGen3_setSampling(instrument, &sampling))
        != RBRGEN3_SUCCESS)
    {
        return err;
    }

    RBRGen3Deployment deployment = {
        .startTime = RBRGEN3_DATETIME_MIN,
        .endTime = RBRGEN3_DATETIME_MAX
    };
    if ((err = RBRInstrumentGen3_setDeployment(instrument, &deployment))
        != RBRGEN3_SUCCESS)
    {
        return err;
    }

    if ((err = RBRGen3_setNewMemoryFormat(
             instrument,
             RBRGEN3_MEMFORMAT_CALBIN00))
        != RBRGEN3_SUCCESS)
    {
        return err;
    }

    RBRGen3Thresholding thresholding;
    err = RBRGen3_getThresholding(instrument, &thresholding);
    if (err == RBRGEN3_SUCCESS && thresholding.enabled)
    {
        thresholding.enabled = false;
        RBRGen3_setThresholding(instrument, &thresholding);
    }

    RBRGen3TwistActivation twistActivation;
    err = RBRGen3_getTwistActivation(instrument, &twistActivation);
    if (err == RBRGEN3_SUCCESS && twistActivation.enabled)
    {
        twistActivation.enabled = false;
        RBRGen3_setTwistActivation(instrument, &twistActivation);
    }

    if ((err = RBRGen3_enable(instrument, true, &status))
        != RBRGEN3_SUCCESS)
    {
        return err;
    }

    return RBRGEN3_SUCCESS;
}
