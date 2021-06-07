/**
 * \file posix-stream-dynamiccorrection.c
 *
 * \brief Example of using the library to use the dynamic correction.
 *        Data are streamed from logger and the correction is applied.
 *
 * \copyright
 * Copyright (c) 2021 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Prerequisite for gmtime_r in time.h. */
#define _POSIX_C_SOURCE 200112L

/* Required for errno. */
#include <errno.h>
/* Required for NAN, isnan. */
#include <math.h>
/* Required for fprintf, printf, snprintf. */
#include <stdio.h>
/* Required for strerror. */
#include <string.h>
/* Required for gmtime_r, time_t, strftime. */
#include <time.h>
/* Required for close, sleep. */
#include <unistd.h>

#include "posix-shared.h"
#include "RBRDynamicCorrection.h"

static RBRInstrumentDateTime g_timeReference = 0;

static RBRInstrumentSample g_sample;


RBRInstrumentError instrumentSample(
    const struct RBRInstrument *instrument,
    const struct RBRInstrumentSample *const sample)
{
    /* Unused. */
    (void) instrument;

    char ftime[128];
    time_t sampleSeconds = (time_t) (sample->timestamp / 1000);
    struct tm sampleTime;
    gmtime_r(&sampleSeconds, &sampleTime);
    strftime(ftime, sizeof(ftime), "%F %T", &sampleTime);

    printf("%s.%03" PRIi64, ftime, sample->timestamp % 1000);
    for (int32_t i = 0; i < sample->channels; i++)
    {
        printf(", %lf", sample->readings[i]);
    }
    printf("\n");

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError streamCTD(RBRInstrument *instrument, int dynamicCorrection_channel[], RBRDynamicCorrectionMeasurement *meas)
{
    RBRInstrumentError err;
    
    err = RBRInstrument_readSample(instrument);
    if (err != RBRINSTRUMENT_SUCCESS)
    {
        fprintf(stderr, "Error: %s\n", RBRInstrumentError_name(err));
        return err;
    }
    else {
        /* check first timestamp obtained for the stream */
        if ( g_timeReference == 0 )
        {
            g_timeReference = g_sample.timestamp;
        }


        /* we already pre-validated the channels to be
         * defined in the following order */
        meas->timestamp = (g_sample.timestamp - g_timeReference) / 1000.0f;
        meas->conductivity = g_sample.readings[dynamicCorrection_channel[0]];
        meas->marineTemperature = g_sample.readings[dynamicCorrection_channel[1]];
        meas->pressure = g_sample.readings[dynamicCorrection_channel[2]];
        meas->condTemperature = g_sample.readings[dynamicCorrection_channel[3]];
    }

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError applyCorrection(RBRInstrument *instrument, int dynamicCorrection_channel[], float Fs)
{
    RBRDynamicCorrectionParams params;
    RBRDynamicCorrectionError status;
    RBRDynamicCorrectionMeasurement meas;
    RBRDynamicCorrectionResult      corrResult;

    /* first step, initialiaze the algorithm using the proper sampling rate */
    status = RBRDynamicCorrection_init(&params, Fs);
    if ( status != RBR_DCORR_SUCCESS )
    {
        fprintf(stderr, "RBRDynamicCorrection_init() return error code %u\n", status);
        return RBRINSTRUMENT_UNKNOWN_ERROR;
    }

    /* write an header */
    printf("timestamp(s), C_cor(mS/cm), T_cor (celcius), P_meas(sea pressure, dbar), S_cor\n");
    printf("-----------------------------------------------------------------------------------\n");

    while (1)
    {
        /* input to algorithm */
        streamCTD(instrument, dynamicCorrection_channel, &meas);
        
        /* feed the data into the correction algorithm */
        status = RBRDynamicCorrection_addMeasurement(&params, &meas, &corrResult);

        /* wait until sufficient sample feed into algorithm */
        if ( status == RBR_DCORR_NOT_VALID_YET )
        {
            continue;
        }

        if ( status != RBR_DCORR_SUCCESS )
        {
            /* timestamp and pressure are not corrected,
            * so they should still be valid */
            corrResult.corrConductivity = NAN;
            corrResult.corrTemperature = NAN;
            corrResult.corrSalinity = NAN;
        }

        /* report the result */
        printf("timestamp: %.3f, C_cor: %.8f, T_cor: %.8f, P_meas: %.8f, S_cor: %.8f\n", 
                corrResult.timestamp,
                corrResult.corrConductivity,
                corrResult.corrTemperature,
                corrResult.pressure,
                corrResult.corrSalinity);
    }

    return RBRINSTRUMENT_SUCCESS;
}


int main(int argc, char *argv[])
{
    char *programName = argv[0];
    char *devicePath;

    int status = EXIT_SUCCESS;
    int instrumentFd;

    RBRInstrumentError err;
    RBRInstrument *instrument = NULL;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s device\n", argv[0]);
        return EXIT_FAILURE;
    }

    devicePath = argv[1];

    if ((instrumentFd = openSerialFd(devicePath)) < 0)
    {
        fprintf(stderr, "%s: Failed to open serial device: %s!\n",
                programName,
                strerror(errno));
        return EXIT_FAILURE;
    }

    fprintf(stderr,
            "%s: Using %s v%s (built %s).\n",
            programName,
            RBRINSTRUMENT_LIB_NAME,
            RBRINSTRUMENT_LIB_VERSION,
            RBRINSTRUMENT_LIB_BUILD_DATE);


    RBRInstrumentCallbacks callbacks = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite,
        .sample = instrumentSample,
        .sampleBuffer = &g_sample
    };

    if ((err = RBRInstrument_open(
             &instrument,
             &callbacks,
             INSTRUMENT_COMMAND_TIMEOUT_MSEC,
             (void *) &instrumentFd)) != RBRINSTRUMENT_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to establish instrument connection: %s!\n",
                programName,
                RBRInstrumentError_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    /* query labels to check for CTD */
    RBRInstrumentLabelsList labelList;
    err = RBRInstrument_getLabelsList(instrument, &labelList);
    if ( err != RBRINSTRUMENT_SUCCESS )
    {
            fprintf(stderr, "%s: Failed to query label list: %s!\n",
                    programName,
                    RBRInstrumentError_name(err));
            status = EXIT_FAILURE;
            goto fileCleanup;
    }

    /* scan labelsList to find C,T,D, T_for_cond_corr and store channel index in an array */
    bool isCtd = true;
    int i = 0;
    int dynamicCorrection_channel[4];
    for (int ch_id = 0; ch_id < labelList.count; ch_id++)
    {
                if ( strcmp(labelList.labels[ch_id],"conductivity_00") == 0)
                {
                   dynamicCorrection_channel[0]=ch_id;
                   i ++;
                }
                else if ( strcmp(labelList.labels[ch_id],"temperature_00")==0 )
                {
                    dynamicCorrection_channel[1]=ch_id;
                    i ++;
                }
                else if ( strcmp(labelList.labels[ch_id],"seapressure_00")==0 )
                {
                    dynamicCorrection_channel[2]=ch_id;
                    i ++;
                }
                else if (strcmp(labelList.labels[ch_id],"conductivitycelltemperature_00")==0)
                {
                    dynamicCorrection_channel[3]=ch_id;
                    i ++;
                }
    }

    if ( labelList.count < 4 || i<4)
    {
        isCtd = false;
    }

    if ( isCtd == false )
    {
        fprintf(stderr, "Warning: Logger not configured as a CTD\n");
        goto instrumentCleanup;
    }

    RBRInstrumentLink link;
    RBRInstrument_getLink(instrument, &link);
    printf("Connected to the instrument via %s.\n",
           RBRInstrumentLink_name(link));

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
            printf("Connected in %s mode at %s baud.\n",
                   RBRInstrumentSerialMode_name(serial.mode),
                   RBRInstrumentSerialBaudRate_name(serial.baudRate));

            RBRInstrument_setSerialStreamingState(instrument, true);
            break;
        }
    default:
        fprintf(stderr,
                "I don't know how I'm connected to the instrument, so I can't"
                " enable streaming. Giving up.\n");
        goto instrumentCleanup;
    }

    RBRInstrumentDeployment deployment;
    RBRInstrument_getDeployment(instrument, &deployment);
    if (deployment.status != RBRINSTRUMENT_STATUS_LOGGING)
    {
        printf("%s: Instrument is %s, not logging. I'm going to start it.\n",
               programName,
               RBRInstrumentDeploymentStatus_name(deployment.status));

        if ((err = instrumentStart(instrument)) != RBRINSTRUMENT_SUCCESS)
        {
            fprintf(stderr,
                    "%s: Failed to start instrument: %s!\n",
                    programName,
                    RBRInstrumentError_name(err));
            status = EXIT_FAILURE;
            goto instrumentCleanup;
        }
    }

    /* get sampling rate from instrument */
    RBRInstrumentSampling sampling;
    if ((err = RBRInstrument_getSampling(instrument, &sampling)) != RBRINSTRUMENT_SUCCESS)
    {
        fprintf(stderr,
                "%s: Failed to query 'sampling' from instrument: %s!\n",
                programName,
                RBRInstrumentError_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }
    /* sampling.period is in ms, samplingRate is in Hz */
    float samplingRate = 1000.0 / (float)sampling.period;

    err = applyCorrection(instrument, dynamicCorrection_channel, samplingRate);
    if (err != RBRINSTRUMENT_SUCCESS)
    {
        fprintf(stderr, "Unexpected termination\n");
    }

instrumentCleanup:
    RBRInstrument_close(instrument);
fileCleanup:
    close(instrumentFd);

    return status;
}



