/*
 * Copyright (c) 2021 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file posix-parse-file-dynamiccorrection.c
 *
 * \brief Example of applying the dynamic correction to data parsed from a file.
 *
 * Data are parsed from an easyparse file;
 * a test file is provided as a .bin file in the ../examples/sampledata folder.
 * If you want to test with your own .bin file,
 * a few assumptions are made.
 * In the .bin file, the channels must be defined in the following order:
 *
 * - channel 1: C (mS/cm)
 * - channel 2: T meas (°C)
 * - channel 3: P (sea pressure, dbar)
 * - channel 4: T cond (°C)
 */

/* Required for errno. */
#include <errno.h>
/* Required for NAN, isnan. */
#include <math.h>
/* Required for open. */
#include <fcntl.h>
/* Required for EXIT_FAILURE, EXIT_SUCCESS, strtol. */
#include <stdlib.h>
/* Required for open. */
#include <sys/stat.h>
/* Required for fprintf, printf. */
#include <stdio.h>
/* Required for strerror. */
#include <string.h>
/* Required for close. */
#include <unistd.h>

#include "posix-shared.h"
#include "RBRGen3Parser.h"
#include "RBRDynamicCorrection.h"

/* Readings storage for as many channels as this application expects. */
#define CHANNEL_MAX 32

/* CSV column assignement */
/* channel id from 1 to 4 corresponds to C(mS/cm), T meas(°C), P meas(dbar), T cond(°C) */
#define CHANNEL_COND   1
#define CHANNEL_T_MEAS 2
#define CHANNEL_P_MEAS 3
#define CHANNEL_T_COND 4

/* definition for sampling rate for input data
 * (this value is not stored within the binary data, example given is 2 sample/sec) */
#define SAMPLING_RATE 2.0f

RBRGen3DateTime g_timeReference = 0;

RBRDynamicCorrectionParams dynamicCorrParams;

RBRGen3Error parserSample(const struct RBRGen3Parser *parser,
                          const struct RBRGen3Sample *const sample)
{
    /* struct for dynamic correction */
    RBRDynamicCorrectionError status;
    RBRDynamicCorrectionMeasurement meas;
    RBRDynamicCorrectionResult corrResult;
    static bool firstCall = true; /* warning: this example is not designed to be reentrant */
    /* Unused. */
    (void) parser;

    /* retrieve first timestamp */
    if (firstCall) {
        g_timeReference = sample->timestamp;
        firstCall = false;
    }

    /* The channels to be defined in the following order (for this example) */
    /* channel id from 1 to 4 corresponds to C(mS/cm), T meas(°C), P meas(dbar), T cond(°C) */
    /* here the P meas means sea pressure */
    meas.timestamp = sample->timestamp - g_timeReference; /* in millisecond */
    meas.conductivity = sample->readings[CHANNEL_COND - 1];
    meas.marineTemperature = sample->readings[CHANNEL_T_MEAS - 1];
    meas.pressure = sample->readings[CHANNEL_P_MEAS - 1];
    meas.condTemperature = sample->readings[CHANNEL_T_COND - 1];

    /* feed the data into the correction algorithm */
    status = RBRDynamicCorrection_addMeasurement(&dynamicCorrParams, &meas, &corrResult);

    /* wait until sufficient sample feed into algorithm */
    if (status == RBRDYNAMICCORRECTION_NOT_VALID_YET) {
        return RBRGEN3_SUCCESS;
    }

    if (status != RBRDYNAMICCORRECTION_SUCCESS) {
        /* timestamp and sea pressure are not corrected,
         * so they should still be valid */
        corrResult.corrTemperature = NAN;
        corrResult.corrSalinity = NAN;
    }

    /* report the result. here pressure is sea pressure*/
    printf("%.3f, %.8f, %.8f, %.8f, %.8f\n",
           /* output timestamp in seconds */
           (double) corrResult.timestamp / 1000.0,
           (double) corrResult.corrTemperature,
           (double) corrResult.pressure,
           (double) corrResult.corrSalinity,
           (double) meas.condTemperature);

    return RBRGEN3_SUCCESS;
}

int main(int argc, char *argv[])
{
    char *programName = argv[0];
    char *filePath;
    int32_t channels;

    int status = EXIT_SUCCESS;
    int datasetFd;

    if (argc < 3) {
        fprintf(stderr, "command incomplete: %s\r\n", argv[0]);
        printf("usage: <path>/posix-parse-file-dynamiccorrection <path_to_.bin_file> <number of "
               "channels>\r\n");
        printf("       %s\r\n",
               "e.g: ./posix-parse-file-dynamiccorrection "
               "../sampledata/dynamiccorrection-sample.bin 4");
        return EXIT_FAILURE;
    }

    filePath = argv[1];
    channels = strtol(argv[2], NULL, 10);

    if ((datasetFd = open(filePath, O_RDONLY)) < 0) {
        fprintf(stderr, "%s: Failed to open file: %s!\n", programName, strerror(errno));
        return EXIT_FAILURE;
    }

    fprintf(stderr, "%s: Using %s v%s.\n", programName, RBRGEN3_LIB_NAME, RBRGEN3_LIB_VERSION);

    printf("warning: this example works for data file with a fixed sampling rate of %.1f Hz.\n",
           (double) SAMPLING_RATE);
    /* write an header */
    printf("-----------------------------------------------------------------------------------\n");
    printf("timestamp(s) | T_cor(°C) | P_meas(sea pressure, dbar) | S_cor(PSU) | T_cond(°C)\n");
    printf("-----------------------------------------------------------------------------------\n");

    RBRGen3Parser parser;

    double sampleReadings[CHANNEL_MAX];
    RBRGen3Sample sampleBuffer = {.size = CHANNEL_MAX, .readings = sampleReadings};
    RBRGen3ParserCallbacks parserCallbacks = {
        .sample = parserSample,
        .sampleBuffer = &sampleBuffer,
    };

    RBRGen3ParserConfig parserConfig = {
        .format = RBRGEN3_MEMFORMAT_CALBIN00,
        .formatConfig =
            {
                .easyParse =
                    {
                        .channels = channels,
                    },
            },
    };

    RBRDynamicCorrectionError dynamicCorrStatus;
    dynamicCorrStatus = RBRDynamicCorrection_init(&dynamicCorrParams,
                                                  SAMPLING_RATE,
                                                  DCORR_T_DELAY,
                                                  DCORR_ALPHA_A,
                                                  DCORR_ALPHA_E,
                                                  DCORR_TAU_A,
                                                  DCORR_TAU_E,
                                                  DCORR_CT_COEFF_A,
                                                  DCORR_CT_COEFF_E,
                                                  DCORR_VP_MIN,
                                                  DCORR_VP_MAX,
                                                  DCORR_VP_FC);
    if (dynamicCorrStatus != RBRDYNAMICCORRECTION_SUCCESS) {
        fprintf(stderr,
                "%s: Failed to initialize dynamic correction library: err code %u!\n",
                programName,
                dynamicCorrStatus);
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    RBRGen3Error err;
    if ((err = RBRGen3Parser_init(&parser, &parserCallbacks, &parserConfig, NULL)) !=
        RBRGEN3_SUCCESS) {
        fprintf(
            stderr, "%s: Failed to initialize parser: %s!\n", programName, RBRGen3Error_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    uint8_t buf[1024];
    int32_t bufSize = 0;
    int32_t readSize;
    int32_t parsedSize;

    while (true) {
        readSize = read(datasetFd, buf + bufSize, sizeof(buf) - bufSize);
        if (readSize < 0 && errno == EAGAIN) {
            fprintf(stderr, "\nRetrying...\n");
            continue;
        } else if (readSize < 0) {
            /* unknown error (just report errno) */
            fprintf(stderr, "\nReading error: errno = %s", strerror(errno));
            break;
        } else if (readSize == 0) {
            break;
        }

        bufSize += readSize;
        parsedSize = bufSize;
        RBRGen3Parser_parse(&parser,
                            RBRGEN3_DATASET_EASYPARSE_SAMPLE_DATA,
                            buf,
                            &parsedSize); /* parserSample() gets called and prints the sample. */
        bufSize -= parsedSize;
        memmove(buf, buf + parsedSize, bufSize);
    }

    RBRGen3Parser_destroy(&parser);
fileCleanup:
    close(datasetFd);

    return status;
}
