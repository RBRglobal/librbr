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

int main(void)
{
    RBRInstrumentGen4 instrument;
    printf("%zu\n", sizeof(instrument)); //in bytes

    RBRInstrumentGen4ConfigPool configPool;
    printf("%zu, %zu\n", sizeof(configPool), sizeof(configPool.pool[0]));

    RBRInstrumentGen4SchedulePool schedulePool;
    printf("%zu, %zu\n", sizeof(schedulePool), sizeof(schedulePool.pool[0]));

    RBRInstrumentGen4GroupPool groupPool;
    printf("%zu, %zu\n", sizeof(groupPool), sizeof(groupPool.pool[0]));

    RBRInstrumentGen4ChannelPool channelPool;
    printf("%zu, %zu, %zu\n", sizeof(channelPool), sizeof(channelPool.pool[0]), sizeof(channelPool.pool[0].calibration));

    RBRInstrumentGen4DatasetPool datasetPool;
    printf("%zu, %zu\n", sizeof(datasetPool), sizeof(datasetPool.pool[0]));

}
