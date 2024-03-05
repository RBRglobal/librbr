/**
 * \file posix-footprint.c
 *
 * \brief Get the footprint for different structures.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for errno. */
#include <errno.h>
/* Required for isnan. */
#include <math.h>
/* Required for fprintf, printf, snprintf. */
#include <stdio.h>
/* Required for strerror. */
#include <string.h>
/* Required for close. */
#include <unistd.h>

#include "posix-shared.h"

int main()
{
    RBRInstrumentGen4 instrument;
    printf("%ld\n", sizeof(instrument)); //in bytes

    RBRInstrumentGen4Configs configs;
    RBRInstrumentGen4Configlist configlist;
    RBRInstrumentGen4Config config;
    printf("%ld, %ld, %ld\n", sizeof(configs), sizeof(configlist), sizeof(config));

    RBRInstrumentGen4Schedules schedules;
    RBRInstrumentGen4Schedulelist schedulelist;
    RBRInstrumentGen4Schedule schedule;
    printf("%ld, %ld, %ld\n", sizeof(schedules), sizeof(schedulelist), sizeof(schedule));

    RBRInstrumentGen4Groups groups;
    RBRInstrumentGen4Grouplist grouplist;
    RBRInstrumentGen4Group group;
    printf("%ld, %ld, %ld\n", sizeof(groups), sizeof(grouplist), sizeof(group));

    RBRInstrumentGen4Channels channels;
    RBRInstrumentGen4Channellist channellist;
    RBRInstrumentGen4Channel channel;
    RBRInstrumentGen4Calibration calibration;
    printf("%ld, %ld, %ld, %ld\n", sizeof(channels), sizeof(channellist), sizeof(channel), sizeof(calibration));

    RBRInstrumentGen4Datasets datasets;
    RBRInstrumentGen4Datasetlist datasetlist;
    RBRInstrumentGen4Dataset dataset;
    printf("%ld, %ld, %ld\n", sizeof(datasets), sizeof(datasetlist), sizeof(dataset));

}