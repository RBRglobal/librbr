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

#include <RBRInstrumentGen3.h>

#include "instrument.h"
#include "io.h"
#include "time.h"

LOG_MODULE_REGISTER(main, CONFIG_MAIN_LOG_LEVEL);

const static struct device *instrumentUart = DEVICE_DT_GET(DT_CHOSEN(rbr_instrument));

RBRInstrumentGen3 instrumentBuffer;
RBRInstrumentGen3Sample sampleBuffer;

ZephyrRBRInstrumentGen3IO io;

RBRInstrumentGen3Error instrumentSample(
    const struct RBRInstrumentGen3 *instrument,
    const struct RBRInstrumentGen3Sample *const sample)
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

    return RBRINSTRUMENTGEN3_SUCCESS;
}

int main(void)
{
    RBRInstrumentGen3Error err;
    RBRInstrumentGen3 *instrument = &instrumentBuffer;

    err = ZephyrRBRInstrumentGen3IO_init(&io, instrumentUart);
    if (err != RBRINSTRUMENTGEN3_SUCCESS)
    {
        LOG_ERR("initializing UART: %s", RBRInstrumentGen3Error_name(err));
        return 0;
    }

    LOG_INF("using %s v%s",
            RBRINSTRUMENTGEN3_LIB_NAME,
            RBRINSTRUMENTGEN3_LIB_VERSION);

    RBRInstrumentGen3Callbacks callbacks = {
        .time = ZephyrRBRInstrumentGen3Time_get,
        .sleep = ZephyrRBRInstrumentGen3Time_sleep,
        .read = ZephyrRBRInstrumentGen3IO_read,
        .write = ZephyrRBRInstrumentGen3IO_write,
        .sample = instrumentSample,
        .sampleBuffer = &sampleBuffer,
    };

    err = RBRInstrumentGen3_open(
        &instrument,
        &callbacks,
        CONFIG_INSTRUMENT_COMMAND_TIMEOUT_MSEC,
        (void *) &io);
    if (err != RBRINSTRUMENTGEN3_SUCCESS)
    {
        LOG_ERR("opening instrument: %s",
                RBRInstrumentGen3Error_name(err));
        return 0;
    }

    RBRInstrumentGen3Link link;
    RBRInstrumentGen3_getLink(instrument, &link);
    LOG_INF("connected via %s", RBRInstrumentGen3Link_name(link));

    switch (link)
    {
    case RBRINSTRUMENTGEN3_LINK_USB:
        RBRInstrumentGen3_setUSBStreamingState(instrument, true);
        break;
    case RBRINSTRUMENTGEN3_LINK_SERIAL:
    case RBRINSTRUMENTGEN3_LINK_WIFI:
        {
            RBRInstrumentGen3Serial serial;
            RBRInstrumentGen3_getSerial(instrument, &serial);
            LOG_INF("connected in %s mode at %s baud",
                    RBRInstrumentGen3SerialMode_name(serial.mode),
                    RBRInstrumentGen3SerialBaudRate_name(serial.baudRate));

            RBRInstrumentGen3_setSerialStreamingState(instrument, true);
            break;
        }
    default:
        LOG_ERR("I don't know how I'm connected to the instrument, so I can't"
                " enable streaming");
        return 0;
    }

    RBRInstrumentGen3Deployment deployment;
    RBRInstrumentGen3_getDeployment(instrument, &deployment);
    if (deployment.status != RBRINSTRUMENTGEN3_STATUS_LOGGING)
    {
        LOG_INF("instrument is %s, not logging; I'm going to start it",
                RBRInstrumentGen3DeploymentStatus_name(deployment.status));

        if ((err = instrumentStart(instrument)) != RBRINSTRUMENTGEN3_SUCCESS)
        {
            LOG_ERR("starting instrument: %s",
                    RBRInstrumentGen3Error_name(err));
            return 0;
        }
    }

    while (true)
    {
        if ((err = RBRInstrumentGen3_readSample(instrument)) != RBRINSTRUMENTGEN3_SUCCESS)
        {
            LOG_ERR("%s", RBRInstrumentGen3Error_name(err));
        }
    }
}
