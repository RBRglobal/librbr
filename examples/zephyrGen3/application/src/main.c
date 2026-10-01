/*
 * Copyright (c) 2025 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file main.c
 *
 * \brief Example of using libRBR with Zephyr.
 */

#include <stdio.h>
#include <time.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <RBRGen3.h>
#include <RBRGen3Commands.h>

#include "instrument.h"
#include "io.h"
#include "time.h"

/* Readings storage for as many channels as this application expects. */
#define CHANNEL_MAX 32

LOG_MODULE_REGISTER(main, CONFIG_MAIN_LOG_LEVEL);

const static struct device *instrumentUart = DEVICE_DT_GET(DT_CHOSEN(rbr_instrument));

RBRGen3 instrumentBuffer;
static uint8_t instrumentCommandBuffer[RBRGEN3_COMMAND_BUFFER_DEFAULT];
static uint8_t instrumentResponseBuffer[RBRGEN3_RESPONSE_BUFFER_DEFAULT];
static double sampleReadings[CHANNEL_MAX];
RBRGen3Sample sampleBuffer = {.size = CHANNEL_MAX, .readings = sampleReadings};

ZephyrRBRGen3IO io;

RBRGen3Error instrumentSample(const struct RBRGen3 *conn, const struct RBRGen3Sample *const sample)
{
    /* Unused. */
    (void) conn;

    char ftime[128];
    time_t sampleSeconds = (time_t) (sample->timestamp / 1000);
    struct tm *sampleTime = gmtime(&sampleSeconds);
    strftime(ftime, sizeof(ftime), "%F %T", sampleTime);

    printf("%s.%03" PRIi64, ftime, sample->timestamp % 1000);
    for (int32_t i = 0; i < sample->channelCount; i++) {
        printf(", %lf", sample->readings[i]);
    }
    printf("\n");

    return RBRGEN3_SUCCESS;
}

int main(void)
{
    RBRGen3Error err;
    RBRGen3 *conn = &instrumentBuffer;

    err = ZephyrRBRGen3IO_init(&io, instrumentUart);
    if (err != RBRGEN3_SUCCESS) {
        LOG_ERR("initializing UART: %s", RBRGen3Error_name(err));
        return 0;
    }

    LOG_INF("using %s v%s", RBRGEN3_LIB_NAME, RBRGEN3_LIB_VERSION);

    RBRGen3Environment environment = {
        .time = ZephyrRBRGen3Time_get,
        .sleep = ZephyrRBRGen3Time_sleep,
        .read = ZephyrRBRGen3IO_read,
        .write = ZephyrRBRGen3IO_write,
        .sample = instrumentSample,
        .sampleBuffer = &sampleBuffer,
        .command = instrumentCommandBuffer,
        .commandCapacity = sizeof(instrumentCommandBuffer),
        .response = instrumentResponseBuffer,
        .responseCapacity = sizeof(instrumentResponseBuffer),
    };

    err = RBRGen3_open(conn, &environment, CONFIG_INSTRUMENT_COMMAND_TIMEOUT_MSEC, (void *) &io);
    if (err != RBRGEN3_SUCCESS) {
        LOG_ERR("opening instrument: %s", RBRGen3Error_name(err));
        return 0;
    }

    RBRGen3Link link;
    RBRGen3_getLink(conn, &link);
    LOG_INF("connected via %s", RBRGen3Link_name(link));

    switch (link) {
    case RBRGEN3_LINK_USB:
        RBRGen3_setUSBStreamingState(conn, true);
        break;
    case RBRGEN3_LINK_SERIAL:
    case RBRGEN3_LINK_WIFI: {
        RBRGen3Serial serial;
        RBRGen3_getSerial(conn, &serial);
        LOG_INF("connected in %s mode at %s baud",
                RBRGen3SerialMode_name(serial.mode),
                RBRGen3SerialBaudRate_name(serial.baudRate));

        RBRGen3_setSerialStreamingState(conn, true);
        break;
    }
    default:
        LOG_ERR("I don't know how I'm connected to the instrument, so I can't"
                " enable streaming");
        return 0;
    }

    RBRGen3Deployment deployment;
    RBRGen3_getDeployment(conn, &deployment);
    if (deployment.status != RBRGEN3_STATUS_LOGGING) {
        LOG_INF("instrument is %s, not logging; I'm going to start it",
                RBRGen3DeploymentStatus_name(deployment.status));

        if ((err = instrumentStart(conn)) != RBRGEN3_SUCCESS) {
            LOG_ERR("starting instrument: %s", RBRGen3Error_name(err));
            return 0;
        }
    }

    while (true) {
        if ((err = RBRGen3_readSample(conn)) != RBRGEN3_SUCCESS) {
            LOG_ERR("%s", RBRGen3Error_name(err));
        }
    }
}
