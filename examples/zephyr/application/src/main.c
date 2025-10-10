/**
 * \file main.c
 *
 * \brief Example of using libRBR with Zephyr.
 *
 * \copyright
 * Copyright (c) 2025 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include <stdio.h>
#include <time.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <RBRInstrument.h>

#include "instrument.h"
#include "io.h"
#include "time.h"

LOG_MODULE_REGISTER(main, CONFIG_MAIN_LOG_LEVEL);

const static struct device *instrumentUart = DEVICE_DT_GET(DT_CHOSEN(rbr_instrument));

RBRInstrument instrumentBuffer;
RBRInstrumentSample sampleBuffer;

ZephyrRBRInstrumentIO io;

RBRInstrumentError instrumentSample(
    const struct RBRInstrument *instrument,
    const struct RBRInstrumentSample *const sample)
{
    /* Unused. */
    (void) instrument;

    char ftime[128];
    time_t sampleSeconds = (time_t) (sample->timestamp / 1000);
    struct tm *sampleTime = gmtime(&sampleSeconds);
    strftime(ftime, sizeof(ftime), "%F %T", sampleTime);

    printf("%s.%03" PRIi64, ftime, sample->timestamp % 1000);
    for (int32_t i = 0; i < sample->channels; i++)
    {
        printf(", %lf", sample->readings[i]);
    }
    printf("\n");

    return RBRINSTRUMENT_SUCCESS;
}

int main(void)
{
    RBRInstrumentError err;
    RBRInstrument *instrument = &instrumentBuffer;

    err = ZephyrRBRInstrumentIO_init(&io, instrumentUart);
    if (err != RBRINSTRUMENT_SUCCESS)
    {
        LOG_ERR("initializing UART: %s", RBRInstrumentError_name(err));
        return 0;
    }

    LOG_INF("using %s v%s (built %s)",
            RBRINSTRUMENT_LIB_NAME,
            RBRINSTRUMENT_LIB_VERSION,
            RBRINSTRUMENT_LIB_BUILD_DATE);

    RBRInstrumentCallbacks callbacks = {
        .time = ZephyrRBRInstrumentTime_get,
        .sleep = ZephyrRBRInstrumentTime_sleep,
        .read = ZephyrRBRInstrumentIO_read,
        .write = ZephyrRBRInstrumentIO_write,
        .sample = instrumentSample,
        .sampleBuffer = &sampleBuffer,
    };

    err = RBRInstrument_open(
        &instrument,
        &callbacks,
        CONFIG_INSTRUMENT_COMMAND_TIMEOUT_MSEC,
        (void *) &io);
    if (err != RBRINSTRUMENT_SUCCESS)
    {
        LOG_ERR("opening instrument: %s",
                RBRInstrumentError_name(err));
        return 0;
    }

    RBRInstrumentLink link;
    RBRInstrument_getLink(instrument, &link);
    LOG_INF("connected via %s", RBRInstrumentLink_name(link));

    switch (link)
    {
    case RBRINSTRUMENT_LINK_USB:
        RBRInstrument_setUSBStreamingState(instrument, true);
        break;
    case RBRINSTRUMENT_LINK_SERIAL:
    case RBRINSTRUMENT_LINK_WIFI:
        {
            RBRInstrumentSerial serial;
            RBRInstrument_getSerial(instrument, &serial);
            LOG_INF("connected in %s mode at %s baud",
                    RBRInstrumentSerialMode_name(serial.mode),
                    RBRInstrumentSerialBaudRate_name(serial.baudRate));

            RBRInstrument_setSerialStreamingState(instrument, true);
            break;
        }
    default:
        LOG_ERR("I don't know how I'm connected to the instrument, so I can't"
                " enable streaming");
        return 0;
    }

    RBRInstrumentDeployment deployment;
    RBRInstrument_getDeployment(instrument, &deployment);
    if (deployment.status != RBRINSTRUMENT_STATUS_LOGGING)
    {
        LOG_INF("instrument is %s, not logging; I'm going to start it",
                RBRInstrumentDeploymentStatus_name(deployment.status));

        if ((err = instrumentStart(instrument)) != RBRINSTRUMENT_SUCCESS)
        {
            LOG_ERR("starting instrument: %s",
                    RBRInstrumentError_name(err));
            return 0;
        }
    }

    while (true)
    {
        if ((err = RBRInstrument_readSample(instrument)) != RBRINSTRUMENT_SUCCESS)
        {
            LOG_ERR("%s", RBRInstrumentError_name(err));
        }
    }
}
