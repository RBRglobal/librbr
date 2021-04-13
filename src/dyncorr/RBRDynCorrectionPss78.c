/**
 * \file RBRDynCorrectionPss78.c
 *
 * \brief Library for salinity dynamic correction (PSS-78 conversion)
 *
 * \copyright
 * Copyright (c) 2021 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */


#include <stdio.h>
#include <math.h>
#include <stdint.h>

#include "RBRDynCorrectionPSS78.h"



float RBRDynCorr_PSS78(float C, float T, float P)
{
    float pressure;
    float T_its68;
    float Rp_num, Rp_den;
    float R, Rp, rT;
    float RT, RT_sqrt;
    float S_1, S_2;
    float S;

    // hydrostatic pressure in bars
    pressure=P*0.1f;

    // temperature in ITS68...
    T_its68=T*1.00024f;

    // convert conductivity to a ratio
    R= C/PSS78_C_REF;

    // rT & Rp
    //rT = PSS78_C0 + PSS78_C1*T_its68 + PSS78_C2*(T_its68*T_its68) + PSS78_C3*T_its68*(T_its68*T_its68) + 
    //                        PSS78_C4*(T_its68*T_its68)*(T_its68*T_its68);
    rT = PSS78_C3 + PSS78_C4*T_its68;
    rT = PSS78_C2 + rT*T_its68;
    rT = PSS78_C1 + rT*T_its68;
    rT = PSS78_C0 + rT*T_its68;

    //Rp_num = PSS78_E1*pressure + PSS78_E2*(pressure*pressure) + PSS78_E3*pressure*(pressure*pressure);
    Rp_num = PSS78_E2 + PSS78_E3*pressure;
    Rp_num = (PSS78_E1 + Rp_num*pressure)*pressure;

    Rp_den = 1.0f + PSS78_D1*T_its68 + PSS78_D2*(T_its68*T_its68) + PSS78_D3*R + PSS78_D4*(T_its68*R);
    Rp = 1.0f + Rp_num/Rp_den;

    // R_T
    RT = R/(Rp*rT);

    // sqrt(RT)
    RT_sqrt = sqrtf(RT);

    // S_1 = PSS78_A0 + PSS78_A1*RT_sqrt + PSS78_A2*RT + PSS78_A3*RT_sqrt*RT + PSS78_A4*(RT*RT) + PSS78_A5*(RT*RT)*RT_sqrt;
    // S_2 = PSS78_B0 + PSS78_B1*RT_sqrt + PSS78_B2*RT + PSS78_B3*RT_sqrt*RT + PSS78_B4*(RT*RT) + PSS78_B5*(RT*RT)*RT_sqrt;

    S_1 = PSS78_A4 + PSS78_A5*RT_sqrt;
    S_1 = PSS78_A3 + S_1*RT_sqrt;
    S_1 = PSS78_A2 + S_1*RT_sqrt;
    S_1 = PSS78_A1 + S_1*RT_sqrt;
    S_1 = PSS78_A0 + S_1*RT_sqrt;

    S_2 = PSS78_B4 + PSS78_B5*RT_sqrt;
    S_2 = PSS78_B3 + S_2*RT_sqrt;
    S_2 = PSS78_B2 + S_2*RT_sqrt;
    S_2 = PSS78_B1 + S_2*RT_sqrt;
    S_2 = PSS78_B0 + S_2*RT_sqrt;

    S = S_1 + ((T_its68 - PSS78_T_REF)/(1.0f + PSS78_K*(T_its68 - PSS78_T_REF)))*S_2;

    return S;
}

void RBRDynCorr_PSS78_unitTest()
{
    float C, T, P;
    float S;

    C = 110.0f;
    T = 5.0f;
    P = 2500.0f;
    S = RBRDynCorr_PSS78(C, T, P);

    printf("CTP: %.4f %.4f %.4f --> S = %.8f (138.626)\r\n", C, T, P, S);

    C = 55.0f;
    S = RBRDynCorr_PSS78(C, T, P);
    printf("CTP: %.4f %.4f %.4f --> S = %.8f (59.4009)\r\n", C, T, P, S);

    T = 21.0f;
    S = RBRDynCorr_PSS78(C, T, P);
    printf("CTP: %.4f %.4f %.4f --> S = %.8f (39.0323)\r\n", C, T, P, S);

    P = 100.0f;
    S = RBRDynCorr_PSS78(C, T, P);
    printf("CTP: %.4f %.4f %.4f --> S = %.8f (39.8831)\r\n", C, T, P, S);
}