/**
 * \file RBRInstrumentGen3Schedule.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for isnan, NAN. */
#include <math.h>
/* Required for memset, strcmp. */
#include <string.h>

#include "RBRGen3.h"
#include "RBRGen3Internal.h"

static RBRGen3Error RBRInstrumentGen3_getClockL2(RBRGen3 *instrument,
                                                   RBRInstrumentGen3Clock *clock)
{
    char *command;
    RBRGen3ResponseParameter parameter;

    RBRGen3Error err = RBRGen3_converse(instrument,
                                                    "settings offsetfromutc");
    /* Older Logger2 firmware didn't support the `offsetfromutc` setting, so it
     * will indicate an error when we go looking for it. Just swallow the
     * error and move on to retrieving the time. */
    if (err == RBRGEN3_HARDWARE_ERROR)
    {
        return RBRGEN3_SUCCESS;
    }
    else if (err != RBRGEN3_SUCCESS)
    {
        return err;
    }
    else
    {
        command = NULL;
        while (true)
        {
            RBRGen3_parseResponse(instrument,
                                        &command,
                                        &parameter);

            if (parameter.key == NULL || parameter.value == NULL)
            {
                break;
            }
            else if (strcmp(parameter.key, "offsetfromutc") != 0)
            {
                continue;
            }

            if (strcmp(parameter.value, "unknown") != 0)
            {
                clock->offsetFromUtc = strtod(parameter.value, NULL);
            }
        }
    }

    /* Retrieve the time after the UTC offset so our return value is as close
     * as possible to the actual instrument time. */
    RBR_TRY(RBRGen3_converse(instrument, "now"));

    command = NULL;
    while (true)
    {
        RBRGen3_parseResponse(instrument,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "now") != 0)
        {
            continue;
        }

        RBR_TRY(RBRGen3DateTime_parseScheduleTime(parameter.value,
                                                        &clock->dateTime,
                                                        NULL));
    }

    return RBRGEN3_SUCCESS;
}

static RBRGen3Error RBRInstrumentGen3_getClockL3(RBRGen3 *instrument,
                                                   RBRInstrumentGen3Clock *clock)
{
    RBR_TRY(RBRGen3_converse(instrument, "clock"));

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
        else if (strcmp(parameter.key, "datetime") == 0)
        {
            RBR_TRY(RBRGen3DateTime_parseScheduleTime(parameter.value,
                                                            &clock->dateTime,
                                                            NULL));
        }
        else if (strcmp(parameter.key, "offsetfromutc") == 0
                 && strcmp(parameter.value, "unknown") != 0)
        {
            clock->offsetFromUtc = strtod(parameter.value, NULL);
        }
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRInstrumentGen3_getClock(RBRGen3 *instrument,
                                          RBRInstrumentGen3Clock *clock)
{
    clock->dateTime = 0;
    clock->offsetFromUtc = NAN;

    if (instrument->generation == RBRGEN3_LOGGER2)
    {
        return RBRInstrumentGen3_getClockL2(instrument, clock);
    }
    else
    {
        return RBRInstrumentGen3_getClockL3(instrument, clock);
    }
}

static RBRGen3Error RBRInstrumentGen3_setClockL2(
    RBRGen3 *instrument,
    const char *dateTime,
    float offsetFromUtc)
{
    /* Set the clock as quickly as possible so that the hardware clock is as
     * close as possible to the provided value. */
    RBR_TRY(RBRGen3_converse(instrument,
                                   "now = %s",
                                   dateTime));

    if (isnan(offsetFromUtc))
    {
        return RBRGEN3_SUCCESS;
    }

    RBR_TRY(RBRInstrumentGen3_permit(instrument, "settings"));

    RBRGen3Error err;
    err = RBRGen3_converse(instrument,
                                 "settings offsetfromutc = %02f",
                                 (double) offsetFromUtc);
    /* Older Logger2 firmware didn't support the `offsetfromutc` setting,
     * so it will indicate an error when we go looking for it. Just swallow
     * the error and move on to setting the time. */
    if (err == RBRGEN3_HARDWARE_ERROR)
    {
        err = RBRGEN3_SUCCESS;
    }
    return err;
}

static RBRGen3Error RBRInstrumentGen3_setClockL3(
    RBRGen3 *instrument,
    const char *dateTime,
    float offsetFromUtc)
{
    if (!isnan(offsetFromUtc))
    {
        return RBRGen3_converse(
            instrument,
            "clock datetime = %s, offsetfromutc = %02f",
            dateTime,
            (double) offsetFromUtc);
    }
    else
    {
        return RBRGen3_converse(instrument,
                                      "clock datetime = %s",
                                      dateTime);
    }
}

RBRGen3Error RBRInstrumentGen3_setClock(RBRGen3 *instrument,
                                          const RBRInstrumentGen3Clock *clock)
{
    if (clock->dateTime < RBRGEN3_DATETIME_MIN
        || clock->dateTime > RBRGEN3_DATETIME_MAX)
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    char dateTime[RBRGEN3_SCHEDULE_TIME_LEN + 1];
    RBRGen3DateTime_toScheduleTime(clock->dateTime, dateTime);

    if (instrument->generation == RBRGEN3_LOGGER2)
    {
        return RBRInstrumentGen3_setClockL2(instrument,
                                        dateTime,
                                        clock->offsetFromUtc);
    }
    else
    {
        return RBRInstrumentGen3_setClockL3(instrument,
                                        dateTime,
                                        clock->offsetFromUtc);
    }
}

const char *RBRInstrumentGen3SamplingMode_name(RBRInstrumentGen3SamplingMode mode)
{
    switch (mode)
    {
    case RBRINSTRUMENTGEN3_SAMPLING_CONTINUOUS:
        return "continuous";
    case RBRINSTRUMENTGEN3_SAMPLING_BURST:
        return "burst";
    case RBRINSTRUMENTGEN3_SAMPLING_WAVE:
        return "wave";
    case RBRINSTRUMENTGEN3_SAMPLING_AVERAGE:
        return "average";
    case RBRINSTRUMENTGEN3_SAMPLING_TIDE:
        return "tide";
    case RBRINSTRUMENTGEN3_SAMPLING_REGIMES:
        return "regimes";
    case RBRINSTRUMENTGEN3_SAMPLING_DDSAMPLING:
        return "ddsampling";
    case RBRINSTRUMENTGEN3_SAMPLING_COUNT:
        return "sampling mode count";
    case RBRINSTRUMENTGEN3_UNKNOWN_SAMPLING:
    default:
        return "unknown sampling mode";
    }
}

const char *RBRInstrumentGen3Gate_name(RBRInstrumentGen3Gate gate)
{
    switch (gate)
    {
    case RBRINSTRUMENTGEN3_GATE_NONE:
        return "none";
    case RBRINSTRUMENTGEN3_GATE_THRESHOLDING:
        return "thresholding";
    case RBRINSTRUMENTGEN3_GATE_TWISTACTIVATION:
        return "twistactivation";
    case RBRINSTRUMENTGEN3_GATE_INVALID:
        return "invalid";
    case RBRINSTRUMENTGEN3_GATE_COUNT:
        return "gate count";
    case RBRINSTRUMENTGEN3_UNKNOWN_GATE:
    default:
        return "unknown gate";
    }
}

RBRGen3Error RBRInstrumentGen3_getSampling(
    RBRGen3 *instrument,
    RBRInstrumentGen3Sampling *sampling)
{
    memset(sampling, 0, sizeof(RBRInstrumentGen3Sampling));
    sampling->mode = RBRINSTRUMENTGEN3_UNKNOWN_SAMPLING;
    sampling->gate = RBRINSTRUMENTGEN3_UNKNOWN_GATE;
    /* Very old Logger2 instruments didn't show the userperiodlimit parameter,
     * so we'll set the default value of the field conservatively. */
    RBRGen3Period *userPeriodLimit =
        (RBRGen3Period *) &sampling->userPeriodLimit;
    *userPeriodLimit = 1000;

    RBRGen3Period *availableFastPeriods =
        (RBRGen3Period *) sampling->availableFastPeriods;

    /*
     * The `sampling` command format added support for the `all` parameter
     * between L2 and L3. It's necessary to get the `availablefastperiods`.
     * The `schedule` parameter also went away between generations, but we
     * don't care about that.
     *
     * The L2 command:
     *
     *     >> sampling
     *     << sampling schedule = 1, mode = continuous, period = 83, burstlength = 10, burstinterval = 10000, gate = none, userperiodlimit = 83
     *
     * The L3 command:
     *
     *     >> sampling all
     *     << sampling mode = continuous, period = 63, burstlength = 10, burstinterval = 10000, gate = none, userperiodlimit = 63, availablefastperiods = 500|250|125|63
     */
    const char *generationCommand;
    if (instrument->generation == RBRGEN3_LOGGER2)
    {
        generationCommand = "sampling";
    }
    else
    {
        generationCommand = "sampling all";
    }

    RBR_TRY(RBRGen3_converse(instrument, generationCommand));

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
        else if (strcmp(parameter.key, "mode") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENTGEN3_SAMPLING_COUNT; ++i)
            {
                if (strcmp(RBRInstrumentGen3SamplingMode_name(i),
                           parameter.value) == 0)
                {
                    sampling->mode = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "period") == 0)
        {
            sampling->period = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "burstlength") == 0)
        {
            sampling->burstLength = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "burstinterval") == 0)
        {
            sampling->burstInterval = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "gate") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENTGEN3_GATE_COUNT; ++i)
            {
                if (strcmp(RBRInstrumentGen3Gate_name(i), parameter.value) == 0)
                {
                    sampling->gate = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "userperiodlimit") == 0)
        {
            *userPeriodLimit = strtol(parameter.value, NULL, 10);

            /* Logger3 will tell us available sampling rates, so we don't have
             * to guess them. */
            if (instrument->generation != RBRGEN3_LOGGER2)
            {
                continue;
            }

            bool has3Hz5HzAvailable = false;
            /* 200/333 are only available on firmware type 100/up to
             * firmware version 1.360 on firmware type 103. */
            if (instrument->id.fwtype == 100
                || (instrument->id.fwtype == 103
                    && RBRGen3Version_compare(instrument->id.version,
                                                    "1.360") <= 0))
            {
                has3Hz5HzAvailable = true;
            }

            int i = 0;
            switch (*userPeriodLimit)
            {
            case 31:
                availableFastPeriods[i++] = 31;
                availableFastPeriods[i++] = 42;
            /* Fallthrough. */
            case 63:
                availableFastPeriods[i++] = 63;
            /* Fallthrough. */
            case 83:
                availableFastPeriods[i++] = 83;
                availableFastPeriods[i++] = 125;
            /* Fallthrough. */
            case 167:
                availableFastPeriods[i++] = 167;
                if (has3Hz5HzAvailable)
                {
                    availableFastPeriods[i++] = 200;
                }
                availableFastPeriods[i++] = 250;
                if (has3Hz5HzAvailable)
                {
                    availableFastPeriods[i++] = 333;
                }
                availableFastPeriods[i++] = 500;
            }
        }
        else if (strcmp(parameter.key, "availablefastperiods") == 0)
        {
            int periodCount = 0;
            char *nextValue;
            do
            {
                if ((nextValue = strstr(parameter.value, "|")) != NULL)
                {
                    *nextValue = '\0';
                    ++nextValue;
                }

                availableFastPeriods[periodCount++] =
                    strtol(parameter.value, NULL, 10);

                parameter.value = nextValue;
            } while (nextValue != NULL
                     && periodCount
                     < RBRINSTRUMENTGEN3_AVAILABLE_FAST_PERIODS_MAX);
        }
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRInstrumentGen3Sampling_validateSamplingPeriod(
    const RBRInstrumentGen3Sampling *sampling)
{
    if (sampling->period <= 0
        || sampling->period > RBRINSTRUMENTGEN3_SAMPLING_PERIOD_MAX
        || (sampling->period >= 1000
            && sampling->period % 1000 != 0)
        || (sampling->userPeriodLimit > 0
            && sampling->period < sampling->userPeriodLimit))
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    /* If we're not doing fast sampling or we don't have any available fast
     * periods then we don't want to check for inclusion. */
    if (sampling->period < 1000 && sampling->availableFastPeriods[0] != 0)
    {
        bool hasFastPeriod = false;

        for (int i = 0;
             i < RBRINSTRUMENTGEN3_AVAILABLE_FAST_PERIODS_MAX
             && sampling->availableFastPeriods[i] != 0;
             ++i)
        {
            if (sampling->availableFastPeriods[i] == sampling->period)
            {
                hasFastPeriod = true;
                break;
            }
        }

        if (!hasFastPeriod)
        {
            return RBRGEN3_INVALID_PARAMETER_VALUE;
        }
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRInstrumentGen3_setSampling(
    RBRGen3 *instrument,
    const RBRInstrumentGen3Sampling *sampling)
{
    RBR_TRY(RBRInstrumentGen3Sampling_validateSamplingPeriod(sampling));

    if (sampling->mode < 0 || sampling->mode >= RBRINSTRUMENTGEN3_SAMPLING_COUNT)
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    return RBRGen3_converse(
        instrument,
        "sampling mode = %s, period = %d",
        RBRInstrumentGen3SamplingMode_name(sampling->mode),
        sampling->period);
}

RBRGen3Error RBRInstrumentGen3_setBurstSampling(
    RBRGen3 *instrument,
    const RBRInstrumentGen3Sampling *sampling)
{
    RBR_TRY(RBRInstrumentGen3Sampling_validateSamplingPeriod(sampling));

    int32_t minBurstInterval = sampling->burstLength * sampling->period;

    if (sampling->burstLength < 2
        || sampling->burstLength > 65535
        || sampling->burstInterval < 1000
        || sampling->burstInterval > RBRINSTRUMENTGEN3_SAMPLING_PERIOD_MAX
        || sampling->burstInterval % 1000 != 0
        || sampling->burstInterval <= minBurstInterval)
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    return RBRGen3_converse(
        instrument,
        "sampling burstlength = %d, burstinterval = %d",
        sampling->burstLength,
        sampling->burstInterval);
}

const char *RBRInstrumentGen3DeploymentStatus_name(
    RBRInstrumentGen3DeploymentStatus status)
{
    switch (status)
    {
    case RBRINSTRUMENTGEN3_STATUS_DISABLED:
        return "disabled";
    case RBRINSTRUMENTGEN3_STATUS_PENDING:
        return "pending";
    case RBRINSTRUMENTGEN3_STATUS_LOGGING:
        return "logging";
    case RBRINSTRUMENTGEN3_STATUS_GATED:
        return "gated";
    case RBRINSTRUMENTGEN3_STATUS_FINISHED:
        return "finished";
    case RBRINSTRUMENTGEN3_STATUS_STOPPED:
        return "stopped";
    case RBRINSTRUMENTGEN3_STATUS_FULLANDSTOPPED:
        return "fullandstopped";
    case RBRINSTRUMENTGEN3_STATUS_FULL:
        return "full";
    case RBRINSTRUMENTGEN3_STATUS_FAILED:
        return "failed";
    case RBRINSTRUMENTGEN3_STATUS_NOTBLANK:
        return "notblank";
    case RBRINSTRUMENTGEN3_STATUS_UNKNOWN:
        return "unknown";
    case RBRINSTRUMENTGEN3_STATUS_COUNT:
        return "status count";
    case RBRINSTRUMENTGEN3_UNKNOWN_STATUS:
    default:
        return "unknown status";
    }
}

static RBRGen3Error RBRInstrumentGen3_getDeploymentL2(
    RBRGen3 *instrument,
    RBRGen3Deployment *deployment)
{
    char *command;
    RBRGen3ResponseParameter parameter;

    /* Logger2 doesn't have a deployment command; it has separate starttime/
     * endtime/status commands. We'll call and parse each one separately. */

    RBR_TRY(RBRGen3_converse(instrument, "starttime"));
    command = NULL;
    while (true)
    {
        RBRGen3_parseResponse(instrument,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "starttime") != 0)
        {
            continue;
        }

        RBR_TRY(RBRGen3DateTime_parseScheduleTime(
                    parameter.value,
                    &deployment->startTime,
                    NULL));
    }

    RBR_TRY(RBRGen3_converse(instrument, "endtime"));
    command = NULL;
    while (true)
    {
        RBRGen3_parseResponse(instrument,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "endtime") != 0)
        {
            continue;
        }

        RBR_TRY(RBRGen3DateTime_parseScheduleTime(
                    parameter.value,
                    &deployment->endTime,
                    NULL));
    }

    RBR_TRY(RBRGen3_converse(instrument, "status"));
    command = NULL;
    while (true)
    {
        RBRGen3_parseResponse(instrument,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "status") != 0)
        {
            continue;
        }

        for (int i = 0; i < RBRINSTRUMENTGEN3_STATUS_COUNT; ++i)
        {
            if (strcmp(RBRInstrumentGen3DeploymentStatus_name(i),
                       parameter.value) == 0)
            {
                *(RBRInstrumentGen3DeploymentStatus *) &deployment->status = i;
                break;
            }
        }
    }

    return RBRGEN3_SUCCESS;
}

static RBRGen3Error RBRInstrumentGen3_getDeploymentL3(
    RBRGen3 *instrument,
    RBRGen3Deployment *deployment)
{
    RBR_TRY(RBRGen3_converse(instrument, "deployment"));

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
        else if (strcmp(parameter.key, "starttime") == 0)
        {
            RBR_TRY(RBRGen3DateTime_parseScheduleTime(
                        parameter.value,
                        &deployment->startTime,
                        NULL));
        }
        else if (strcmp(parameter.key, "endtime") == 0)
        {
            RBR_TRY(RBRGen3DateTime_parseScheduleTime(
                        parameter.value,
                        &deployment->endTime,
                        NULL));
        }
        else if (strcmp(parameter.key, "status") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENTGEN3_STATUS_COUNT; ++i)
            {
                if (strcmp(RBRInstrumentGen3DeploymentStatus_name(i),
                           parameter.value) == 0)
                {
                    *(RBRInstrumentGen3DeploymentStatus *) &deployment->status = i;
                    break;
                }
            }
        }
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRInstrumentGen3_getDeployment(
    RBRGen3 *instrument,
    RBRGen3Deployment *deployment)
{
    memset(deployment, 0, sizeof(RBRGen3Deployment));

    *(RBRInstrumentGen3DeploymentStatus *) &deployment->status =
        RBRINSTRUMENTGEN3_UNKNOWN_STATUS;

    if (instrument->generation == RBRGEN3_LOGGER2)
    {
        return RBRInstrumentGen3_getDeploymentL2(instrument, deployment);
    }
    else
    {
        return RBRInstrumentGen3_getDeploymentL3(instrument, deployment);
    }
}

RBRGen3Error RBRInstrumentGen3_setDeployment(
    RBRGen3 *instrument,
    const RBRGen3Deployment *deployment)
{
    if (deployment->endTime <= deployment->startTime
        || deployment->startTime < RBRGEN3_DATETIME_MIN
        || deployment->startTime > RBRGEN3_DATETIME_MAX
        || deployment->endTime < RBRGEN3_DATETIME_MIN
        || deployment->endTime > RBRGEN3_DATETIME_MAX)
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    char startTime[RBRGEN3_SCHEDULE_TIME_LEN + 1];
    RBRGen3DateTime_toScheduleTime(deployment->startTime, startTime);

    char endTime[RBRGEN3_SCHEDULE_TIME_LEN + 1];
    RBRGen3DateTime_toScheduleTime(deployment->endTime, endTime);

    /* As with reading deployment details, we'll have to call the starttime/
     * endtime commands each in turn for Logger2. */
    if (instrument->generation == RBRGEN3_LOGGER2)
    {
        RBR_TRY(RBRGen3_converse(instrument,
                                       "starttime = %s",
                                       startTime));
        RBR_TRY(RBRGen3_converse(instrument,
                                       "endtime = %s",
                                       endTime));
    }
    else
    {
        RBR_TRY(RBRGen3_converse(
                    instrument,
                    "deployment starttime = %s, endtime = %s",
                    startTime,
                    endTime));
    }
    return RBRGEN3_SUCCESS;
}
