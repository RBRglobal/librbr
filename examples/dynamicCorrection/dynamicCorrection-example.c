/**
 * \file dynamiccorrection_example.c
 * \brief Library for salinity dynamic correction (example / test)
 *
 * \copyright
 * Copyright (c) 2021 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define _DEBUG 1
#include "RBRDynamicCorrection.h"



/* ---- retrieve data from CSV ----- */

/* for testing.  We can put a restriction on test file
 * to no exceed a given length */
#define MAX_LINE_SIZE   256
#define MAX_CSV_SIZE    50000


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
typedef struct
{
    int size;
    float timestamp_sec[MAX_CSV_SIZE];
    float P_meas[MAX_CSV_SIZE];
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
    RBRDynamicCorrectionResult      corrResult;
    int index;
    
    /* first step, initialiaze the algorithm */
    status = RBRDynamicCorrection_init(&params, Fs);
    if ( status != RBR_DCORR_SUCCESS )
    {
        fprintf(stderr, "RBRDynamicCorrection_init() return error code %u", status);
        return;
    }

    /* write an header */
    fprintf(file, "# COMPILATION " __DATE__ " , " __TIME__ "\n");
    fprintf(file, "# timestamp(s), C_cor(mS/cm), T_cor (celcius), P_meas (sea pressure, dbar), S_cor\n");

    for (index = 0; index < data->size; index++)
    {
        /* input to algorithm */
        meas.timestamp = data->timestamp_sec[index];
        meas.conductivity = data->C_meas[index];
        meas.marineTemperature = data->T_meas[index];
        meas.condTemperature = data->T_cond[index];
        meas.pressure = data->P_meas[index];
        
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

        fprintf(file, "%.3f, %.8f, %.8f, %.8f, %.8f\n", 
                corrResult.timestamp,
                corrResult.corrConductivity,
                corrResult.corrTemperature,
                corrResult.pressure,
                corrResult.corrSalinity);

    }
}

/* parse a .csv with the following format
 *  # timestamp (sec), C_meas (mS/cm), T_meas (celcius), P_meas (sea pressure, dbar), T_cond (celcius) */
int RBRDynamicCorrection_parseCsv(const char *filename, csvData_t *data)
{
    char line[MAX_LINE_SIZE];
    char entry[MAX_LINE_SIZE];
    char *ptrStart, *ptrEnd;
    int len;
    int index;
    int line_no = 0;
    float value;
    int n;
    FILE *file;
    

    /* clear to keep static analyzer happy */
    memset(data, 0x00, sizeof(csvData_t));

    file = fopen(filename, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Unable to open file %s\r\n", filename);
        return -1;
    }

    /* skip header (unknown number of line) */
    /* assuming in csv file, all header lines start with # */
    /* also assuming column 0->4 corresponds to timestamp_sec, C_meas, T_meas, P_meas, T_cond*/
    do {
        fgets(line, MAX_LINE_SIZE, file);
        line_no++;
    } while ( (!feof(file)) && (line[0] == '#') );

    index = 0;
    while ( (index < MAX_CSV_SIZE) && (!feof(file)) )
    {
        ptrStart = line;
        ptrEnd = strchr(ptrStart, ',');
        n = 0;
        
        while ( n <= 4 )
        {
            if ( ptrEnd == NULL )
            {
                strcpy(entry, ptrStart);
            }
            else
            {
                len = ptrEnd - ptrStart + 1;
                strncpy(entry, ptrStart, len);
                entry[len-1] = '\0';
            }

            value = strtof(entry, NULL);
            
            /*as mentioned, assuming column 0->4 corresponds to timestamp_sec, C_meas, T_meas, P_meas, T_cond*/
            switch (n++)
            {
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

            if (ptrEnd == NULL)
            {
                break;
                if (n <=4)
                {
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

    if ( data->size == 0 )
    {
        fprintf(stderr, "Input file %s was empty!", filename);
        return -1;
    }

    return 0;
}



void usage()
{
    printf("xxx csv_file\r\n");
    printf("\r\n");
    printf("    --help       : Show help\r\n");

    return;
}

int main(int argc, char *argv[])
{
    FILE *file;
    char filename[MAX_LINE_SIZE-20];
    char filenameOut[MAX_LINE_SIZE];
    csvData_t data;
    float Fs;
    
    if ( argc <= 1 )
    {
        printf("Provide input arguments\r\n");
        printf("\r\n");
        
        usage();
        return 0;
    }

    if ( strcmp(argv[1], "--help") == 0 )
    {
        usage();
        return 0;
    }


    strncpy(filename, argv[1], MAX_LINE_SIZE-1);
    filename[MAX_LINE_SIZE-1] = '\0';

    if ( RBRDynamicCorrection_parseCsv(filename, &data) < 0 )
    {
        fprintf(stderr, "ERROR: unable to parse csv %s\r\n", filename);
        exit(-1);
    }
    
    if ( strlen(filename) > 4 )
    {
        /* trim the .csv */
        filename[strlen(filename)-4] = '\0';
    }
    snprintf(filenameOut, MAX_LINE_SIZE, "%s_corr.csv", filename);

    file = fopen(filenameOut, "w");
    if (file != NULL)
    {
        /* discover sampling rate using first two row of the data */
        Fs = 1.0f / (data.timestamp_sec[1] - data.timestamp_sec[0]);
        printf("sampling rate is %.3f\n", Fs);

        printf("Correction written to %s\n", filenameOut);
        RBRDynamicCorrection_replayData(file, &data, Fs);
    }
    else
    {
        fprintf(stderr, "Unable to write file %s\n", filenameOut);
    }

    return 0;
}
