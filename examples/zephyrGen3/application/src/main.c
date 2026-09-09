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

#include <RBRGen3.h>

#include "instrument.h"
#include "io.h"
#include "time.h"

LOG_MODULE_REGISTER(main, CONFIG_MAIN_LOG_LEVEL);

const static struct device *instrumentUart = DEVICE_DT_GET(DT_CHOSEN(rbr_instrument));

RBRGen3 instrumentBuffer;
RBRGen3Sample sampleBuffer;

ZephyrRBRInstrumentGen3IO io;

RBRGen3Error instrumentSample(
    const struct RBRGen3 *instrument,
    const struct RBRGen3Sample *const sample)
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

    return RBRGEN3_SUCCESS;
}

int main(void)
{
    RBRGen3Error err;
    RBRGen3 *instrument = &instrumentBuffer;

    err = ZephyrRBRInstrumentGen3IO_init(&io, instrumentUart);
    if (err != RBRGEN3_SUCCESS)
    {
        LOG_ERR("initializing UART: %s", RBRGen3Error_name(err));
        return 0;
    }

    LOG_INF("using %s v%s",
            RBRGEN3_LIB_NAME,
            RBRGEN3_LIB_VERSION);

    RBRGen3Callbacks callbacks = {
        .time = ZephyrRBRInstrumentGen3Time_get,
        .sleep = ZephyrRBRInstrumentGen3Time_sleep,
        .read = ZephyrRBRInstrumentGen3IO_read,
        .write = ZephyrRBRInstrumentGen3IO_write,
        .sample = instrumentSample,
        .sampleBuffer = &sampleBuffer,
    };

    err = RBRGen3_open(
        &instrument,
        &callbacks,
        CONFIG_INSTRUMENT_COMMAND_TIMEOUT_MSEC,
        (void *) &io);
    if (err != RBRGEN3_SUCCESS)
    {
        LOG_ERR("opening instrument: %s",
                RBRGen3Error_name(err));
        return 0;
    }

    RBRGen3Link link;
    RBRGen3_getLink(instrument, &link);
    LOG_INF("connected via %s", RBRGen3Link_name(link));

    switch (link)
    {
    case RBRGEN3_LINK_USB:
        RBRInstrumentGen3_setUSBStreamingState(instrument, true);
        break;
    case RBRGEN3_LINK_SERIAL:
    case RBRGEN3_LINK_WIFI:
        {
            RBRGen3Serial serial;
            RBRGen3_getSerial(instrument, &serial);
            LOG_INF("connected in %s mode at %s baud",
                    RBRGen3SerialMode_name(serial.mode),
                    RBRGen3SerialBaudRate_name(serial.baudRate));

            RBRInstrumentGen3_setSerialStreamingState(instrument, true);
            break;
        }
    default:
        LOG_ERR("I don't know how I'm connected to the instrument, so I can't"
                " enable streaming");
        return 0;
    }

    RBRGen3Deployment deployment;
    RBRInstrumentGen3_getDeployment(instrument, &deployment);
    if (deployment.status != RBRINSTRUMENTGEN3_STATUS_LOGGING)
    {
        LOG_INF("instrument is %s, not logging; I'm going to start it",
                RBRInstrumentGen3DeploymentStatus_name(deployment.status));

        if ((err = instrumentStart(instrument)) != RBRGEN3_SUCCESS)
        {
            LOG_ERR("starting instrument: %s",
                    RBRGen3Error_name(err));
            return 0;
        }
    }

    while (true)
    {
        if ((err = RBRInstrumentGen3_readSample(instrument)) != RBRGEN3_SUCCESS)
        {
            LOG_ERR("%s", RBRGen3Error_name(err));
        }
    }
}
