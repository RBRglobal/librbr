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

RBRGen3Error instrumentStart(RBRGen3 *conn)
{
    RBRGen3Error err;

    RBRGen3DeploymentStatus status;
    if ((err = RBRGen3_disable(conn, &status))
        != RBRGEN3_SUCCESS)
    {
        return err;
    }

    RBRGen3Sampling sampling;
    if ((err = RBRGen3_getSampling(conn, &sampling))
        != RBRGEN3_SUCCESS)
    {
        return err;
    }
    sampling.mode = RBRGEN3_SAMPLING_CONTINUOUS;
    sampling.period = sampling.userPeriodLimit;
    if ((err = RBRGen3_setSampling(conn, &sampling))
        != RBRGEN3_SUCCESS)
    {
        return err;
    }

    RBRGen3Deployment deployment = {
        .startTime = RBRGEN3_DATETIME_MIN,
        .endTime = RBRGEN3_DATETIME_MAX
    };
    if ((err = RBRGen3_setDeployment(conn, &deployment))
        != RBRGEN3_SUCCESS)
    {
        return err;
    }

    if ((err = RBRGen3_setNewMemoryFormat(
             conn,
             RBRGEN3_MEMFORMAT_CALBIN00))
        != RBRGEN3_SUCCESS)
    {
        return err;
    }

    RBRGen3Thresholding thresholding;
    err = RBRGen3_getThresholding(conn, &thresholding);
    if (err == RBRGEN3_SUCCESS && thresholding.enabled)
    {
        thresholding.enabled = false;
        RBRGen3_setThresholding(conn, &thresholding);
    }

    RBRGen3TwistActivation twistActivation;
    err = RBRGen3_getTwistActivation(conn, &twistActivation);
    if (err == RBRGEN3_SUCCESS && twistActivation.enabled)
    {
        twistActivation.enabled = false;
        RBRGen3_setTwistActivation(conn, &twistActivation);
    }

    if ((err = RBRGen3_enable(conn, true, &status))
        != RBRGEN3_SUCCESS)
    {
        return err;
    }

    return RBRGEN3_SUCCESS;
}
