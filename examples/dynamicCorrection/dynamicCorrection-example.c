/*
 * Copyright (c) 2021 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file dynamicCorrection-example.c
 * \brief Example of using the library to apply the dynamic correction to data
 * read from a CSV file.
 */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _DEBUG 1
#include "RBRDynamicCorrection.h"

/* ---- retrieve data from CSV ----- */

/* for testing.  We can put a restriction on test file
 * to no exceed a given length */
#define MAX_LINE_SIZE 256
#define MAX_CSV_SIZE  50000

/** @struct csvData_t
 *  This is a struct
 *
 *  @var csvData_t::size
 *  @var csvData_t::timestamp_sec[MAX_CSV_SIZE]
 *  @var csvData_t::P_meas[MAX_CSV_SIZE]
 *  @var csvData_t::T_meas[MAX_CSV_SIZE]
 *  @var csvData_t::C_meas[MAX_CSV_SIZE]
 *  @var csvData_t::T_cond[MAX_CSV_SIZE]
 */
typedef struct {
    int size;
    double timestamp_sec[MAX_CSV_SIZE]; /* time in second */
    float P_meas[MAX_CSV_SIZE];         /* P_meas is sea pressure. */
    float T_meas[MAX_CSV_SIZE];
    float C_meas[MAX_CSV_SIZE];
    float T_cond[MAX_CSV_SIZE];
} csvData_t;

/* just replay data all the data from CSV file and store
 * back result into another csv file for analysis */
void RBRDynamicCorrection_replayData(FILE *file, csvData_t *data, float Fs)
{
    RBRDynamicCorrectionParams params;
    RBRDynamicCorrectionError status;
    RBRDynamicCorrectionMeasurement meas;
    RBRDynamicCorrectionResult corrResult;
    int index;

    /* first step, initialiaze the algorithm */
    status = RBRDynamicCorrection_init(&params,
                                       Fs,
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
    if (status != RBRDYNAMICCORRECTION_SUCCESS) {
        fprintf(stderr, "RBRDynamicCorrection_init() return error code %u", status);
        return;
    }

    /* write an header */
    fprintf(file,
            "# timestamp(s), T_cor(°C), P_meas(sea pressure, dbar), S_cor(PSU), T_cond(°C)\n");

    for (index = 0; index < data->size; index++) {
        /* input to algorithm */
        meas.timestamp =
            (int64_t) llround(data->timestamp_sec[index] * 1000.0); /* time in millisecond */
        meas.conductivity = data->C_meas[index];
        meas.marineTemperature = data->T_meas[index];
        meas.condTemperature = data->T_cond[index];
        meas.pressure = data->P_meas[index];

        status = RBRDynamicCorrection_addMeasurement(&params, &meas, &corrResult);

        /* wait until sufficient sample feed into algorithm */
        if (status == RBRDYNAMICCORRECTION_NOT_VALID_YET) {
            continue;
        }

        if (status != RBRDYNAMICCORRECTION_SUCCESS) {
            /* timestamp and pressure, conductivity are not corrected,
             * so they should still be valid
             * here the pressure is sea pressure.
             */
            corrResult.corrTemperature = NAN;
            corrResult.corrSalinity = NAN;
        }
        /* here the pressure is sea pressure */
        fprintf(file,
                "%.3f, %.8f, %.8f, %.8f, %.8f\n",
                /* maintain output time as second */
                (double) corrResult.timestamp / 1000.0,
                (double) corrResult.corrTemperature,
                (double) corrResult.pressure,
                (double) corrResult.corrSalinity,
                (double) meas.condTemperature);
    }
}

/* parse a .csv with the following format
 *  # timestamp (s), C_meas (mS/cm), T_meas (°C), P_meas (sea pressure, dbar), T_cond (°C) */
int RBRDynamicCorrection_parseCsv(const char *filename, csvData_t *data)
{
    char line[MAX_LINE_SIZE];
    char entry[MAX_LINE_SIZE];
    char *ptrStart, *ptrEnd;
    int len;
    int index;
    int line_no = 0;
    double value;
    int n;
    FILE *file;

    /* clear to keep static analyzer happy */
    memset(data, 0x00, sizeof(csvData_t));

    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Unable to open file %s\r\n", filename);
        return EXIT_FAILURE;
    }

    /* skip header (unknown number of line)
     * assuming in csv file, all header lines start with '#'
     * also assuming column 0->4 corresponds to timestamp_sec, C_meas, T_meas, P_meas, T_cond
     * here the P_meas means sea pressure */
    do {
        fgets(line, MAX_LINE_SIZE, file);
        line_no++;
    } while ((!feof(file)) && (line[0] == '#'));

    index = 0;
    while ((index < MAX_CSV_SIZE) && (!feof(file))) {
        ptrStart = line;
        ptrEnd = strchr(ptrStart, ',');
        n = 0;

        while (n <= 4) {
            if (ptrEnd == NULL) {
                strcpy(entry, ptrStart);
            } else {
                len = ptrEnd - ptrStart + 1;
                strncpy(entry, ptrStart, len);
                entry[len - 1] = '\0';
            }

            if (strstr(entry, "Error-")) {
                char *error_number = entry + 6;
                value = nan(error_number);
            } else {
                value = strtod(entry, NULL);
            }

            /*as mentioned, assuming column 0->4 corresponds to timestamp_sec, C_meas, T_meas,
             *P_meas, T_cond here the P_meas is sea pressure */
            switch (n++) {
            case 0:
                data->timestamp_sec[index] = value;
                break;
            case 1:
                data->C_meas[index] = value;
                break;
            case 2:
                data->T_meas[index] = value;
                break;
            case 3:
                data->P_meas[index] = value;
                break;
            case 4:
                data->T_cond[index] = value;
                break;
            default:
                break;
            }

            if (ptrEnd == NULL) {
                break;
                if (n <= 4) {
                    fprintf(stderr, "Parsing error in %s [ line %d ]\r\n", filename, line_no);
                    break;
                }
            }
            ptrStart = ptrEnd + 1;
            ptrEnd = strchr(ptrStart, ',');
        }

        index++;

        /* next line */
        fgets(line, MAX_LINE_SIZE, file);
        line_no++;
    }

    data->size = index;

    fclose(file);

    if (data->size == 0) {
        fprintf(stderr, "Input file %s was empty!", filename);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void usage(void)
{
    printf("usage: <path>/dynamicCorrection-example <path_to_.csv_file>\r\n");
    printf("       %s\r\n",
           "e.g: ./dynamicCorrection-example ../sampledata/dynamiccorrection-sample.csv");
    return;
}

int main(int argc, char *argv[])
{
    FILE *file;
    char filename[MAX_LINE_SIZE - 20];
    char filenameOut[MAX_LINE_SIZE];
    csvData_t data;
    float Fs;
    if (argc <= 1) {
        fprintf(stderr, "command incomplete: %s\r\n", argv[0]);
        usage();
        return EXIT_SUCCESS;
    }

    if (strcmp(argv[1], "--help") == 0) {
        usage();
        return EXIT_SUCCESS;
    }

    strncpy(filename, argv[1], sizeof(filename) - 1);
    filename[sizeof(filename) - 1] = '\0';

    if (RBRDynamicCorrection_parseCsv(filename, &data) < 0) {
        fprintf(stderr, "ERROR: unable to parse csv %s\r\n", filename);
        exit(-1);
    }

    if (strlen(filename) > 4) {
        /* trim the .csv */
        filename[strlen(filename) - 4] = '\0';
    }
    snprintf(filenameOut, MAX_LINE_SIZE, "%s_corr.csv", filename);

    file = fopen(filenameOut, "w");
    if (file != NULL) {
        /* discover sampling rate using first two row of the data */
        double deltaT = data.timestamp_sec[1] - data.timestamp_sec[0];
        /* when sampling rate is 16Hz, 63 is specified instead of 62 */
        if (fabs(deltaT - 0.062) < 1e-6) {
            deltaT = 0.063;
        }
        Fs = (float) (1.0 / deltaT);
        printf("sampling rate is %.3f\n", (double) Fs);

        printf("Correction written to %s\n", filenameOut);
        RBRDynamicCorrection_replayData(file, &data, Fs);
        fclose(file);
    } else {
        fprintf(stderr, "Unable to write file %s\n", filenameOut);
    }

    return EXIT_SUCCESS;
}
