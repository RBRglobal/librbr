/**
 * \file RBRGen3Schedule.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for isnan, NAN. */
#include <math.h>
/* Required for strtod, strtol. */
#include <stdlib.h>
/* Required for memset, strcmp. */
#include <string.h>

#include "RBRGen3.h"
#include "RBRGen3Internal.h"

static RBRGen3Error RBRGen3_getClockL2(RBRGen3 *conn, RBRGen3Clock *clock)
{
    char *command;
    RBRGen3ResponseParameter parameter;

    RBRGen3Error err = RBRGen3_converse(conn, "settings offsetfromutc");
    /* Older Logger2 firmware didn't support the `offsetfromutc` setting, so it
     * will indicate an error when we go looking for it. Just swallow the
     * error and move on to retrieving the time. */
    if (err == RBRGEN3_HARDWARE_ERROR) {
        return RBRGEN3_SUCCESS;
    } else if (err != RBRGEN3_SUCCESS) {
        return err;
    } else {
        command = NULL;
        while (true) {
            RBRGen3_parseResponse(conn, &command, &parameter);

            if (parameter.key == NULL || parameter.value == NULL) {
                break;
            } else if (strcmp(parameter.key, "offsetfromutc") != 0) {
                continue;
            }

            if (strcmp(parameter.value, "unknown") != 0) {
                clock->offsetFromUtc = strtod(parameter.value, NULL);
            }
        }
    }

    /* Retrieve the time after the UTC offset so our return value is as close
     * as possible to the actual instrument time. */
    RBR_TRY(RBRGen3_converse(conn, "now"));

    command = NULL;
    while (true) {
        RBRGen3_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "now") != 0) {
            continue;
        }

        RBR_TRY(RBRGen3DateTime_parseScheduleTime(parameter.value, &clock->dateTime, NULL));
    }

    return RBRGEN3_SUCCESS;
}

static RBRGen3Error RBRGen3_getClockL3(RBRGen3 *conn, RBRGen3Clock *clock)
{
    RBR_TRY(RBRGen3_converse(conn, "clock"));

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true) {
        RBRGen3_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "datetime") == 0) {
            RBR_TRY(RBRGen3DateTime_parseScheduleTime(parameter.value, &clock->dateTime, NULL));
        } else if (strcmp(parameter.key, "offsetfromutc") == 0 &&
                   strcmp(parameter.value, "unknown") != 0) {
            clock->offsetFromUtc = strtod(parameter.value, NULL);
        }
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_getClock(RBRGen3 *conn, RBRGen3Clock *clock)
{
    clock->dateTime = 0;
    clock->offsetFromUtc = NAN;

    if (conn->generation == RBRGEN3_LOGGER2) {
        return RBRGen3_getClockL2(conn, clock);
    } else {
        return RBRGen3_getClockL3(conn, clock);
    }
}

static RBRGen3Error RBRGen3_setClockL2(RBRGen3 *conn, const char *dateTime, float offsetFromUtc)
{
    /* Set the clock as quickly as possible so that the hardware clock is as
     * close as possible to the provided value. */
    RBR_TRY(RBRGen3_converse(conn, "now = %s", dateTime));

    if (isnan(offsetFromUtc)) {
        return RBRGEN3_SUCCESS;
    }

    RBR_TRY(RBRGen3_permit(conn, "settings"));

    RBRGen3Error err;
    err = RBRGen3_converse(conn, "settings offsetfromutc = %02f", (double) offsetFromUtc);
    /* Older Logger2 firmware didn't support the `offsetfromutc` setting,
     * so it will indicate an error when we go looking for it. Just swallow
     * the error and move on to setting the time. */
    if (err == RBRGEN3_HARDWARE_ERROR) {
        err = RBRGEN3_SUCCESS;
    }
    return err;
}

static RBRGen3Error RBRGen3_setClockL3(RBRGen3 *conn, const char *dateTime, float offsetFromUtc)
{
    if (!isnan(offsetFromUtc)) {
        return RBRGen3_converse(
            conn, "clock datetime = %s, offsetfromutc = %02f", dateTime, (double) offsetFromUtc);
    } else {
        return RBRGen3_converse(conn, "clock datetime = %s", dateTime);
    }
}

RBRGen3Error RBRGen3_setClock(RBRGen3 *conn, const RBRGen3Clock *clock)
{
    if (clock->dateTime < RBRGEN3_DATETIME_MIN || clock->dateTime > RBRGEN3_DATETIME_MAX) {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    char dateTime[RBRGEN3_SCHEDULE_TIME_LEN + 1];
    RBRGen3DateTime_toScheduleTime(clock->dateTime, dateTime);

    if (conn->generation == RBRGEN3_LOGGER2) {
        return RBRGen3_setClockL2(conn, dateTime, clock->offsetFromUtc);
    } else {
        return RBRGen3_setClockL3(conn, dateTime, clock->offsetFromUtc);
    }
}

const char *RBRGen3SamplingMode_name(RBRGen3SamplingMode mode)
{
    switch (mode) {
    case RBRGEN3_SAMPLING_CONTINUOUS:
        return "continuous";
    case RBRGEN3_SAMPLING_BURST:
        return "burst";
    case RBRGEN3_SAMPLING_WAVE:
        return "wave";
    case RBRGEN3_SAMPLING_AVERAGE:
        return "average";
    case RBRGEN3_SAMPLING_TIDE:
        return "tide";
    case RBRGEN3_SAMPLING_REGIMES:
        return "regimes";
    case RBRGEN3_SAMPLING_DDSAMPLING:
        return "ddsampling";
    case RBRGEN3_SAMPLING_COUNT:
        return "sampling mode count";
    case RBRGEN3_UNKNOWN_SAMPLING:
    default:
        return "unknown sampling mode";
    }
}

const char *RBRGen3Gate_name(RBRGen3Gate gate)
{
    switch (gate) {
    case RBRGEN3_GATE_NONE:
        return "none";
    case RBRGEN3_GATE_THRESHOLDING:
        return "thresholding";
    case RBRGEN3_GATE_TWISTACTIVATION:
        return "twistactivation";
    case RBRGEN3_GATE_INVALID:
        return "invalid";
    case RBRGEN3_GATE_COUNT:
        return "gate count";
    case RBRGEN3_UNKNOWN_GATE:
    default:
        return "unknown gate";
    }
}

RBRGen3Error RBRGen3_getSampling(RBRGen3 *conn, RBRGen3Sampling *sampling)
{
    memset(sampling, 0, sizeof(RBRGen3Sampling));
    sampling->mode = RBRGEN3_UNKNOWN_SAMPLING;
    sampling->gate = RBRGEN3_UNKNOWN_GATE;
    /* Very old Logger2 instruments didn't show the userperiodlimit parameter,
     * so we'll set the default value of the field conservatively. */
    RBRGen3Period *userPeriodLimit = (RBRGen3Period *) &sampling->userPeriodLimit;
    *userPeriodLimit = 1000;

    RBRGen3Period *availableFastPeriods = (RBRGen3Period *) sampling->availableFastPeriods;

    /*
     * The `sampling` command format added support for the `all` parameter
     * between L2 and L3. It's necessary to get the `availablefastperiods`.
     * The `schedule` parameter also went away between generations, but we
     * don't care about that.
     *
     * The L2 command:
     *
     *     >> sampling
     *     << sampling schedule = 1, mode = continuous, period = 83, burstlength = 10, burstinterval
     * = 10000, gate = none, userperiodlimit = 83
     *
     * The L3 command:
     *
     *     >> sampling all
     *     << sampling mode = continuous, period = 63, burstlength = 10, burstinterval = 10000, gate
     * = none, userperiodlimit = 63, availablefastperiods = 500|250|125|63
     */
    const char *generationCommand;
    if (conn->generation == RBRGEN3_LOGGER2) {
        generationCommand = "sampling";
    } else {
        generationCommand = "sampling all";
    }

    RBR_TRY(RBRGen3_converse(conn, generationCommand));

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true) {
        RBRGen3_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "mode") == 0) {
            for (int i = 0; i < RBRGEN3_SAMPLING_COUNT; ++i) {
                if (strcmp(RBRGen3SamplingMode_name(i), parameter.value) == 0) {
                    sampling->mode = i;
                    break;
                }
            }
        } else if (strcmp(parameter.key, "period") == 0) {
            sampling->period = strtol(parameter.value, NULL, 10);
        } else if (strcmp(parameter.key, "burstlength") == 0) {
            sampling->burstLength = strtol(parameter.value, NULL, 10);
        } else if (strcmp(parameter.key, "burstinterval") == 0) {
            sampling->burstInterval = strtol(parameter.value, NULL, 10);
        } else if (strcmp(parameter.key, "gate") == 0) {
            for (int i = 0; i < RBRGEN3_GATE_COUNT; ++i) {
                if (strcmp(RBRGen3Gate_name(i), parameter.value) == 0) {
                    sampling->gate = i;
                    break;
                }
            }
        } else if (strcmp(parameter.key, "userperiodlimit") == 0) {
            *userPeriodLimit = strtol(parameter.value, NULL, 10);

            /* Logger3 will tell us available sampling rates, so we don't have
             * to guess them. */
            if (conn->generation != RBRGEN3_LOGGER2) {
                continue;
            }

            bool has3Hz5HzAvailable = false;
            /* 200/333 are only available on firmware type 100/up to
             * firmware version 1.360 on firmware type 103. */
            if (conn->id.fwtype == 100 ||
                (conn->id.fwtype == 103 &&
                 RBRGen3Version_compare(conn->id.version, "1.360") <= 0)) {
                has3Hz5HzAvailable = true;
            }

            int i = 0;
            switch (*userPeriodLimit) {
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
                if (has3Hz5HzAvailable) {
                    availableFastPeriods[i++] = 200;
                }
                availableFastPeriods[i++] = 250;
                if (has3Hz5HzAvailable) {
                    availableFastPeriods[i++] = 333;
                }
                availableFastPeriods[i++] = 500;
            }
        } else if (strcmp(parameter.key, "availablefastperiods") == 0) {
            int periodCount = 0;
            char *nextValue;
            do {
                if ((nextValue = strstr(parameter.value, "|")) != NULL) {
                    *nextValue = '\0';
                    ++nextValue;
                }

                availableFastPeriods[periodCount++] = strtol(parameter.value, NULL, 10);

                parameter.value = nextValue;
            } while (nextValue != NULL && periodCount < RBRGEN3_AVAILABLE_FAST_PERIODS_MAX);
        }
    }

    return RBRGEN3_SUCCESS;
}

static RBRGen3Error RBRGen3Sampling_validateSamplingPeriod(const RBRGen3Sampling *sampling)
{
    if (sampling->period <= 0 || sampling->period > RBRGEN3_SAMPLING_PERIOD_MAX ||
        (sampling->period >= 1000 && sampling->period % 1000 != 0) ||
        (sampling->userPeriodLimit > 0 && sampling->period < sampling->userPeriodLimit)) {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    /* If we're not doing fast sampling or we don't have any available fast
     * periods then we don't want to check for inclusion. */
    if (sampling->period < 1000 && sampling->availableFastPeriods[0] != 0) {
        bool hasFastPeriod = false;

        for (int i = 0;
             i < RBRGEN3_AVAILABLE_FAST_PERIODS_MAX && sampling->availableFastPeriods[i] != 0;
             ++i) {
            if (sampling->availableFastPeriods[i] == sampling->period) {
                hasFastPeriod = true;
                break;
            }
        }

        if (!hasFastPeriod) {
            return RBRGEN3_INVALID_PARAMETER_VALUE;
        }
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_setSampling(RBRGen3 *conn, const RBRGen3Sampling *sampling)
{
    RBR_TRY(RBRGen3Sampling_validateSamplingPeriod(sampling));

    if (sampling->mode < 0 || sampling->mode >= RBRGEN3_SAMPLING_COUNT) {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    return RBRGen3_converse(conn,
                            "sampling mode = %s, period = %d",
                            RBRGen3SamplingMode_name(sampling->mode),
                            sampling->period);
}

RBRGen3Error RBRGen3_setBurstSampling(RBRGen3 *conn, const RBRGen3Sampling *sampling)
{
    RBR_TRY(RBRGen3Sampling_validateSamplingPeriod(sampling));

    int32_t minBurstInterval = sampling->burstLength * sampling->period;

    if (sampling->burstLength < 2 || sampling->burstLength > 65535 ||
        sampling->burstInterval < 1000 || sampling->burstInterval > RBRGEN3_SAMPLING_PERIOD_MAX ||
        sampling->burstInterval % 1000 != 0 || sampling->burstInterval <= minBurstInterval) {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    return RBRGen3_converse(conn,
                            "sampling burstlength = %d, burstinterval = %d",
                            sampling->burstLength,
                            sampling->burstInterval);
}

const char *RBRGen3DeploymentStatus_name(RBRGen3DeploymentStatus status)
{
    switch (status) {
    case RBRGEN3_STATUS_DISABLED:
        return "disabled";
    case RBRGEN3_STATUS_PENDING:
        return "pending";
    case RBRGEN3_STATUS_LOGGING:
        return "logging";
    case RBRGEN3_STATUS_GATED:
        return "gated";
    case RBRGEN3_STATUS_FINISHED:
        return "finished";
    case RBRGEN3_STATUS_STOPPED:
        return "stopped";
    case RBRGEN3_STATUS_FULLANDSTOPPED:
        return "fullandstopped";
    case RBRGEN3_STATUS_FULL:
        return "full";
    case RBRGEN3_STATUS_FAILED:
        return "failed";
    case RBRGEN3_STATUS_NOTBLANK:
        return "notblank";
    case RBRGEN3_STATUS_UNKNOWN:
        return "unknown";
    case RBRGEN3_STATUS_COUNT:
        return "status count";
    case RBRGEN3_UNKNOWN_STATUS:
    default:
        return "unknown status";
    }
}

static RBRGen3Error RBRGen3_getDeploymentL2(RBRGen3 *conn, RBRGen3Deployment *deployment)
{
    char *command;
    RBRGen3ResponseParameter parameter;

    /* Logger2 doesn't have a deployment command; it has separate starttime/
     * endtime/status commands. We'll call and parse each one separately. */

    RBR_TRY(RBRGen3_converse(conn, "starttime"));
    command = NULL;
    while (true) {
        RBRGen3_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "starttime") != 0) {
            continue;
        }

        RBR_TRY(RBRGen3DateTime_parseScheduleTime(parameter.value, &deployment->startTime, NULL));
    }

    RBR_TRY(RBRGen3_converse(conn, "endtime"));
    command = NULL;
    while (true) {
        RBRGen3_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "endtime") != 0) {
            continue;
        }

        RBR_TRY(RBRGen3DateTime_parseScheduleTime(parameter.value, &deployment->endTime, NULL));
    }

    RBR_TRY(RBRGen3_converse(conn, "status"));
    command = NULL;
    while (true) {
        RBRGen3_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "status") != 0) {
            continue;
        }

        for (int i = 0; i < RBRGEN3_STATUS_COUNT; ++i) {
            if (strcmp(RBRGen3DeploymentStatus_name(i), parameter.value) == 0) {
                *(RBRGen3DeploymentStatus *) &deployment->status = i;
                break;
            }
        }
    }

    return RBRGEN3_SUCCESS;
}

static RBRGen3Error RBRGen3_getDeploymentL3(RBRGen3 *conn, RBRGen3Deployment *deployment)
{
    RBR_TRY(RBRGen3_converse(conn, "deployment"));

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true) {
        RBRGen3_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "starttime") == 0) {
            RBR_TRY(
                RBRGen3DateTime_parseScheduleTime(parameter.value, &deployment->startTime, NULL));
        } else if (strcmp(parameter.key, "endtime") == 0) {
            RBR_TRY(RBRGen3DateTime_parseScheduleTime(parameter.value, &deployment->endTime, NULL));
        } else if (strcmp(parameter.key, "status") == 0) {
            for (int i = 0; i < RBRGEN3_STATUS_COUNT; ++i) {
                if (strcmp(RBRGen3DeploymentStatus_name(i), parameter.value) == 0) {
                    *(RBRGen3DeploymentStatus *) &deployment->status = i;
                    break;
                }
            }
        }
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_getDeployment(RBRGen3 *conn, RBRGen3Deployment *deployment)
{
    memset(deployment, 0, sizeof(RBRGen3Deployment));

    *(RBRGen3DeploymentStatus *) &deployment->status = RBRGEN3_UNKNOWN_STATUS;

    if (conn->generation == RBRGEN3_LOGGER2) {
        return RBRGen3_getDeploymentL2(conn, deployment);
    } else {
        return RBRGen3_getDeploymentL3(conn, deployment);
    }
}

RBRGen3Error RBRGen3_setDeployment(RBRGen3 *conn, const RBRGen3Deployment *deployment)
{
    if (deployment->endTime <= deployment->startTime ||
        deployment->startTime < RBRGEN3_DATETIME_MIN ||
        deployment->startTime > RBRGEN3_DATETIME_MAX ||
        deployment->endTime < RBRGEN3_DATETIME_MIN || deployment->endTime > RBRGEN3_DATETIME_MAX) {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    char startTime[RBRGEN3_SCHEDULE_TIME_LEN + 1];
    RBRGen3DateTime_toScheduleTime(deployment->startTime, startTime);

    char endTime[RBRGEN3_SCHEDULE_TIME_LEN + 1];
    RBRGen3DateTime_toScheduleTime(deployment->endTime, endTime);

    /* As with reading deployment details, we'll have to call the starttime/
     * endtime commands each in turn for Logger2. */
    if (conn->generation == RBRGEN3_LOGGER2) {
        RBR_TRY(RBRGen3_converse(conn, "starttime = %s", startTime));
        RBR_TRY(RBRGen3_converse(conn, "endtime = %s", endTime));
    } else {
        RBR_TRY(
            RBRGen3_converse(conn, "deployment starttime = %s, endtime = %s", startTime, endTime));
    }
    return RBRGEN3_SUCCESS;
}
