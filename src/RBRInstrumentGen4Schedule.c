/**
 * \file RBRInstrumentGen4Schedule.c
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
#include <stdlib.h>
#include <string.h>

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Configuration.h"
#include "RBRInstrumentGen4Internal.h"
#include "RBRInstrumentGen4Schedule.h"

RBRInstrumentGen4Error RBRInstrumentGen4_getClock(RBRInstrumentGen4 *instrument,
                                          RBRInstrumentGen4Clock *clock)
{
    clock->dateTime = 0;
    clock->offsetFromUtc = NAN;

RBR_TRY(RBRInstrumentGen4_converse(instrument, "clock"));

    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "datetime") == 0)
        {
            RBR_TRY(RBRInstrumentGen4DateTime_parseScheduleTime(parameter.value,
                                                            &clock->dateTime,
                                                            NULL));
        }
        else if (strcmp(parameter.key, "offsetfromutc") == 0
                 && strcmp(parameter.value, "unknown") != 0)
        {
            clock->offsetFromUtc = strtod(parameter.value, NULL);
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setClock(RBRInstrumentGen4 *instrument,
                                          const RBRInstrumentGen4Clock *clock)
{
    if (clock->dateTime < RBRINSTRUMENTGEN4_DATETIME_MIN
        || clock->dateTime > RBRINSTRUMENTGEN4_DATETIME_MAX)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    char dateTime[RBRINSTRUMENTGEN4_SCHEDULE_TIME_LEN + 1];
    RBRInstrumentGen4DateTime_toScheduleTime(clock->dateTime, dateTime);

    if (instrument->generation == RBRINSTRUMENTGEN4_LOGGER4)
    {
        float offsetFromUtc = (clock->offsetFromUtc);
        if (!isnan(offsetFromUtc))
        {
            return RBRInstrumentGen4_converse(
                instrument,
                "clock datetime=%s offsetfromutc=%02f",
                dateTime,
                (double) offsetFromUtc);
        }
        else
        {
            return RBRInstrumentGen4_converse(instrument,
                                        "clock datetime=%s",
                                        dateTime);
        }
    }
    else
    {
        return RBRINSTRUMENTGEN4_UNSUPPORTED;
    }
}

const char *RBRInstrumentGen4InstrumentState_name(
    RBRInstrumentGen4InstrumentState state)
{
    switch (state)
    {
    case RBRINSTRUMENTGEN4_INSTRUMENT_STATE_DISABLED:
        return "disabled";
    case RBRINSTRUMENTGEN4_INSTRUMENT_STATE_ENABLED:
        return "enabled";
    case RBRINSTRUMENTGEN4_INSTRUMENT_STATE_COUNT:
        return "instrument state count";
    case RBRINSTRUMENTGEN4_UNKNOWN_INSTRUMENT_STATE:
    default:
        return "unknown instrument state";
    }
}

const char *RBRInstrumentGen4DeploymentStatus_name(
    RBRInstrumentGen4DeploymentStatus status)
{
    switch (status)
    {
    case RBRINSTRUMENTGEN4_STATUS_SAMPLING:
        return "sampling";
    case RBRINSTRUMENTGEN4_STATUS_GATED:
        return "gated";
    case RBRINSTRUMENTGEN4_STATUS_PAUSED:
        return "paused";
    case RBRINSTRUMENTGEN4_STATUS_INACTIVE:
        return "inactive";
    case RBRINSTRUMENTGEN4_STATUS_UNKNOWN:
        return "unknown";
    case RBRINSTRUMENTGEN4_STATUS_COUNT:
        return "status count";
    case RBRINSTRUMENTGEN4_UNKNOWN_STATUS:
    default:
        return "unknown status";
    }
}

const char *RBRInstrumentGen4Gate_name(RBRInstrumentGen4Gate gate)
{
    switch (gate)
    {
    case RBRINSTRUMENTGEN4_GATE_NONE:
        return "none";
    case RBRINSTRUMENTGEN4_GATE_THRESHOLDING:
        return "thresholding";
    case RBRINSTRUMENTGEN4_GATE_TWISTACTIVATION:
        return "twistactivation";
    case RBRINSTRUMENTGEN4_GATE_INVALID:
        return "invalid";
    case RBRINSTRUMENTGEN4_GATE_COUNT:
        return "gate count";
    case RBRINSTRUMENTGEN4_UNKNOWN_GATE:
    default:
        return "unknown gate";
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_getDeployment(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Deployment *deployment)
{
    (void)instrument;
    (void)deployment;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setDeployment(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Deployment *deployment)
{
    if (deployment->startTime < RBRINSTRUMENTGEN4_DATETIME_MIN
        || deployment->startTime > RBRINSTRUMENTGEN4_DATETIME_MAX)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    char startTime[RBRINSTRUMENTGEN4_SCHEDULE_TIME_LEN + 1];
    RBRInstrumentGen4DateTime_toScheduleTime(deployment->startTime, startTime);

    if (instrument->generation == RBRINSTRUMENTGEN4_LOGGER4)
    {
        RBR_TRY(RBRInstrumentGen4_converse(
                    instrument,
                    "deployment starttime=%s",
                    startTime));
        return RBRINSTRUMENTGEN4_SUCCESS;
    }
    else
    {
        return RBRINSTRUMENTGEN4_UNSUPPORTED;
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_pause(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4DeploymentStatus *status)
{
    (void)instrument;
    (void)status;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_resume(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4DeploymentStatus *status)
{
    (void)instrument;
    (void)status;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

/******************************************************************/
const char *RBRInstrumentGen4SamplingMode_name(RBRInstrumentGen4SamplingMode mode)
{
    switch (mode)
    {
    case RBRINSTRUMENTGEN4_SAMPLING_CONTINUOUS:
        return "continuous";
    case RBRINSTRUMENTGEN4_SAMPLING_BURST:
        return "burst";
    case RBRINSTRUMENTGEN4_SAMPLING_WAVE:
        return "wave";
    case RBRINSTRUMENTGEN4_SAMPLING_AVERAGE:
        return "average";
    case RBRINSTRUMENTGEN4_SAMPLING_TIDE:
        return "tide";
    case RBRINSTRUMENTGEN4_SAMPLING_REGIMES:
        return "regimes";
    case RBRINSTRUMENTGEN4_SAMPLING_DDSAMPLING:
        return "ddsampling";
    case RBRINSTRUMENTGEN4_SAMPLING_COUNT:
        return "sampling mode count";
    case RBRINSTRUMENTGEN4_UNKNOWN_SAMPLING:
    default:
        return "unknown sampling mode";
    }
}

const char *RBRInstrumentGen4Direction_name(RBRInstrumentGen4Direction direction)
{
    switch (direction)
    {
    case RBRINSTRUMENTGEN4_DIRECTION_ASCENDING:
        return "ascending";
    case RBRINSTRUMENTGEN4_DIRECTION_DESCENDING:
        return "descending";
    case RBRINSTRUMENTGEN4_DIRECTION_COUNT:
        return "direction count";
    case RBRINSTRUMENTGEN4_UNKNOWN_DIRECTION:
    default:
        return "unknown direction";
    }
}

const char *RBRInstrumentGen4RegimesReference_name(
    RBRInstrumentGen4RegimesReference reference)
{
    switch (reference)
    {
    case RBRINSTRUMENTGEN4_REFERENCE_ABSOLUTE:
        return "absolute";
    case RBRINSTRUMENTGEN4_REFERENCE_SEAPRESSURE:
        return "seapressure";
    case RBRINSTRUMENTGEN4_REFERENCE_COUNT:
        return "regimes reference count";
    case RBRINSTRUMENTGEN4_UNKNOWN_REFERENCE:
    default:
        return "unknown regimes reference";
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_getSchedulePool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4SchedulePool *schedulePool)
{
    //GEN4 todo: add logic about it.
    (void)instrument;
    (void)schedulePool;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getSchedule(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4GroupPool *groupPool,
    RBRInstrumentGen4Schedule *schedule)
{
    (void)instrument;
    (void)groupPool;
    (void)schedule;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setSchedule(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Schedule *schedule)
{
    //GEN4 todo: Question: do we want to validate period with deployment values?
    //RBR_TRY(RBRInstrumentGen4Schedule_validateSchedulePeriod(schedule, scheduleLabel, deployment));

    if (schedule->mode < 0 || schedule->mode >= RBRINSTRUMENTGEN4_SAMPLING_COUNT)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    //GEN4 todo: generate a command which is mode-dependent.
    const char *setScheduleCommand=NULL;
    return RBRInstrumentGen4_converse(
        instrument,
        setScheduleCommand);
}

RBRInstrumentGen4Error RBRInstrumentGen4_createSchedule(
    RBRInstrumentGen4 *instrument,
    const char *newScheduleLabel,
    RBRInstrumentGen4SchedulePool *schedulePool,
    RBRInstrumentGen4Schedule **newSchedule){
        //GEN4 todo: add logic about it.
    (void)instrument;
    (void)newScheduleLabel;
    (void)schedulePool;
    (void)newSchedule;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteSchedule(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Schedule *schedule){
        //GEN4 todo: add logic about it.
    (void)instrument;
    (void)schedule;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteScheduleAll(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4SchedulePool *schedulePool){
        //GEN4 todo: add logic about it.
        (void)instrument;
        (void)schedulePool;
        return RBRINSTRUMENTGEN4_SUCCESS;
}
