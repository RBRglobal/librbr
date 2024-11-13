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

    RBRInstrumentGen4ConfigPool configPool;
    printf("%ld, %ld\n", sizeof(configPool), sizeof(configPool.pool[0]));

    RBRInstrumentGen4SchedulePool schedulePool;
    printf("%ld, %ld\n", sizeof(schedulePool), sizeof(schedulePool.pool[0]));

    RBRInstrumentGen4GroupPool groupPool;
    printf("%ld, %ld\n", sizeof(groupPool), sizeof(groupPool.pool[0]));

    RBRInstrumentGen4ChannelPool channelPool;
    printf("%ld, %ld, %ld\n", sizeof(channelPool), sizeof(channelPool.pool[0]), sizeof(channelPool.pool[0].calibration));

    RBRInstrumentGen4DatasetPool datasetPool;
    printf("%ld, %ld\n", sizeof(datasetPool), sizeof(datasetPool.pool[0]));

}
