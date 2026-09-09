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
#include "RBRDynamicCorrectionGen3.h"

#define _AbsP_To_SeaP        10.132507

static RBRInstrumentGen3DateTime g_timeReference = 0;
static RBRInstrumentGen3Sample g_sample;

RBRInstrumentGen3Error instrumentSample(
    const struct RBRInstrumentGen3 *instrument,
    const struct RBRInstrumentGen3Sample *const sample)
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

    return RBRINSTRUMENTGEN3_SUCCESS;
}

RBRInstrumentGen3Error streamCTD(RBRInstrumentGen3 *instrument, int dynamicCorrection_channel[], bool _flagAbsP, RBRDynamicCorrectionGen3Measurement *meas)
{
    RBRInstrumentGen3Error err;
    /* if seapressure_00 channel is used, _flagAbsP will be false, isAbsolute = 0. 
     * Otherwise if pressure_00 channel is in use, it needs a conversion to sea pressure.
    */
    int isAbsolute = 0;

    /* if dynamicCorrection_channel[2] stores the channel index of pressure_00, then its value will be converted to sea pressure below */
    if(_flagAbsP == true){
        isAbsolute = 1;
    }
    
    err = RBRInstrumentGen3_readSample(instrument);
    if (err != RBRINSTRUMENTGEN3_SUCCESS)
    {
        fprintf(stderr, "Error: %s\n", RBRInstrumentGen3Error_name(err));
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
        meas->timestamp = g_sample.timestamp - g_timeReference; //in millisecond
        meas->conductivity = g_sample.readings[dynamicCorrection_channel[0]];
        meas->marineTemperature = g_sample.readings[dynamicCorrection_channel[1]];
        meas->pressure = g_sample.readings[dynamicCorrection_channel[2]]-isAbsolute*_AbsP_To_SeaP;
        meas->condTemperature = g_sample.readings[dynamicCorrection_channel[3]];
      }

    return RBRINSTRUMENTGEN3_SUCCESS;
}

RBRInstrumentGen3Error applyCorrection(RBRInstrumentGen3 *instrument, int dynamicCorrection_channel[], bool _flagAbsP, float Fs)
{
    RBRDynamicCorrectionGen3Params params;
    RBRDynamicCorrectionGen3Error status;
    RBRDynamicCorrectionGen3Measurement meas;
    RBRDynamicCorrectionGen3Result      corrResult;

    /* first step, initialiaze the algorithm using the proper sampling rate */
    status = RBRDynamicCorrectionGen3_init(&params, Fs, DCORR_T_DELAY, 
                                    DCORR_ALPHA_A, DCORR_ALPHA_E,
                                    DCORR_TAU_A, DCORR_TAU_E,
                                    DCORR_CT_COEFF_A, DCORR_CT_COEFF_E,
                                    DCORR_VP_MIN, DCORR_VP_MAX, DCORR_VP_FC);
    if ( status != RBRDYNAMICCORRECTIONGEN3_SUCCESS )
    {
        fprintf(stderr, "RBRDynamicCorrectionGen3_init() return error code %u\n", status);
        return RBRINSTRUMENTGEN3_UNKNOWN_ERROR;
    }

    while (1)
    {
        /* input to algorithm */
        streamCTD(instrument, dynamicCorrection_channel, _flagAbsP, &meas);
        
        /* feed the data into the correction algorithm */
        status = RBRDynamicCorrectionGen3_addMeasurement(&params, &meas, &corrResult);

        /* wait until sufficient sample feed into algorithm */
        if ( status == RBRDYNAMICCORRECTIONGEN3_NOT_VALID_YET )
        {
            continue;
        }

        if ( status != RBRDYNAMICCORRECTIONGEN3_SUCCESS )
        {
            /* timestamp and pressure, conductivity are not corrected,
            * so they should still be valid */
            corrResult.corrTemperature = NAN;
            corrResult.corrSalinity = NAN;
        }

        /* report the result */
        printf("timestamp(s) | T_cor(°C) | P_meas(sea pressure, dbar) | S_cor(PSU) | T_cond(°C): %.3f, %.8f, %.8f, %.8f, %.8f\n", 
                (double) corrResult.timestamp / 1000.0,
                (double) corrResult.corrTemperature,
                (double) corrResult.pressure,
                (double) corrResult.corrSalinity,
                (double) meas.condTemperature);
    }

    return RBRINSTRUMENTGEN3_SUCCESS;
}


int main(int argc, char *argv[])
{
    char *programName = argv[0];
    char *devicePath;

    int status = EXIT_SUCCESS;
    int instrumentFd;

    RBRInstrumentGen3Error err;
    RBRInstrumentGen3 *instrument = NULL;
    #ifdef RBR_LIB_NODYNAMICMEMORYALLOCATION
    RBRInstrumentGen3 instrumentSpace;
    instrument = &instrumentSpace;
    #endif

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
            "%s: Using %s v%s.\n",
            programName,
            RBRINSTRUMENTGEN3_LIB_NAME,
            RBRINSTRUMENTGEN3_LIB_VERSION);


    RBRInstrumentGen3Callbacks callbacks = {
        .time = instrumentTime, //in millisecond
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite,
        .sample = instrumentSample,
        .sampleBuffer = &g_sample
    };

    if ((err = RBRInstrumentGen3_open(
             &instrument,
             &callbacks,
             INSTRUMENT_COMMAND_TIMEOUT_MSEC,
             (void *) &instrumentFd)) != RBRINSTRUMENTGEN3_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to establish instrument connection: %s!\n",
                programName,
                RBRInstrumentGen3Error_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    /* query labels to check for CTD */
    RBRInstrumentGen3LabelsList labelList;
    err = RBRInstrumentGen3_getLabelsList(instrument, &labelList);
    if ( err != RBRINSTRUMENTGEN3_SUCCESS )
    {
            fprintf(stderr, "%s: Failed to query label list: %s!\n",
                    programName,
                    RBRInstrumentGen3Error_name(err));
            status = EXIT_FAILURE;
            goto fileCleanup;
    }

    bool isCtd = true;
    /* variables below are used in scannning labelsList to find C,T,D, T_for_cond_corr and store channel index in an array */
    int i = 0;
    int dynamicCorrection_channel[4];
    bool _flagAbsP = false; //if false, it means no absolute pressure channel detected.
    int _iSeaP = -1;
    int _iAbsP = -1;
    
    /* if total channels are less than 4, then it's impossible to have all 4 channels below. 
     * this will cause a warning message below */
    if (labelList.count < 4){
        isCtd = false;
    }
    /* otherwise channels list will be scanned to find C,T,D, T_for_cond_corr
     * and store channel index in an array dynamicCorrection_channel[] 
     */
    else if (labelList.count >= 4){
        for (int ch_id = 0; ch_id < labelList.count; ch_id++)
        {
            if (strcmp(labelList.labels[ch_id], "conductivity_00") == 0)
            {
                dynamicCorrection_channel[0] = ch_id;
                i++;
            }
            else if (strcmp(labelList.labels[ch_id], "temperature_00") == 0)
            {
                dynamicCorrection_channel[1] = ch_id;
                i++;
            }
            else if (strcmp(labelList.labels[ch_id], "pressure_00") == 0)
            {
                _iAbsP = ch_id;
                _flagAbsP = true;
                i++;
            }
            else if (strcmp(labelList.labels[ch_id], "seapressure_00") == 0)
            {
                _iSeaP = ch_id;
                i++;
            }
            else if (strcmp(labelList.labels[ch_id], "conductivitycelltemperature_00") == 0)
            {
                dynamicCorrection_channel[3] = ch_id;
                i++;
            }
        }

        /* if only absolute pressure channel detected, it will be marked 
        * to inform dynamiccorrection algorithm to convert it to sea pressure before use.
        * if neither absolute pressure channel nor sea pressure channel detected, it will not be treated as CTD instrument.
        */
        if (_iSeaP >= 0){
            //found sea pressure channel, use it
            dynamicCorrection_channel[2]= _iSeaP;
        }
        else{
            //didn't find sea pressure channel
            if(_flagAbsP == true){ //found absolute pressure channel, use it
                dynamicCorrection_channel[2]=_iAbsP;
            }
            else{ //didn't find absolute pressure channel
                isCtd = false;
            }
        }

        /* if not all 4 channels (C.T.D.Tcond) above are detected, it will not be treated as CTD instrument.*/
        if (i<4){
            isCtd = false;
        }
    }
    
    if ( isCtd == false ){
        fprintf(stderr, "Warning: Logger doesn't have all these channels ON:\n  conductivity_00, temperature_00, pressure_00|seapressure_00, conductivitycelltemperature_00\n");
        goto instrumentCleanup;
    }

    RBRInstrumentGen3Link link;
    RBRInstrumentGen3_getLink(instrument, &link);
    printf("Connected to the instrument via %s.\n",
           RBRInstrumentGen3Link_name(link));

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
            printf("Connected in %s mode at %s baud.\n",
                   RBRInstrumentGen3SerialMode_name(serial.mode),
                   RBRInstrumentGen3SerialBaudRate_name(serial.baudRate));

            RBRInstrumentGen3_setSerialStreamingState(instrument, true);
            break;
        }
    default:
        fprintf(stderr,
                "I don't know how I'm connected to the instrument, so I can't"
                " enable streaming. Giving up.\n");
        goto instrumentCleanup;
    }

    RBRInstrumentGen3Deployment deployment;
    RBRInstrumentGen3_getDeployment(instrument, &deployment);
    if (deployment.status != RBRINSTRUMENTGEN3_STATUS_LOGGING)
    {
        printf("%s: Instrument is %s, not logging. I'm going to start it.\n",
               programName,
               RBRInstrumentGen3DeploymentStatus_name(deployment.status));

        if ((err = instrumentStart(instrument)) != RBRINSTRUMENTGEN3_SUCCESS)
        {
            fprintf(stderr,
                    "%s: Failed to start instrument: %s!\n",
                    programName,
                    RBRInstrumentGen3Error_name(err));
            status = EXIT_FAILURE;
            goto instrumentCleanup;
        }
    }

    /* get sampling rate from instrument */
    RBRInstrumentGen3Sampling sampling;
    if ((err = RBRInstrumentGen3_getSampling(instrument, &sampling)) != RBRINSTRUMENTGEN3_SUCCESS)
    {
        fprintf(stderr,
                "%s: Failed to query 'sampling' from instrument: %s!\n",
                programName,
                RBRInstrumentGen3Error_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }
    /* sampling.period is in ms, samplingRate is in Hz */
    float samplingRate = 1000.0f / (float) sampling.period;

    if(isCtd == true) {
        err = applyCorrection(instrument, dynamicCorrection_channel, _flagAbsP, samplingRate);
        if (err != RBRINSTRUMENTGEN3_SUCCESS)
        {
            fprintf(stderr, "Unexpected termination\n");
        }
    }

instrumentCleanup:
    RBRInstrumentGen3_close(instrument);
fileCleanup:
    close(instrumentFd);

    return status;
}
