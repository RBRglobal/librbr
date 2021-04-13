/**
 * \file RBRDynCorrectionPss78.h
 *
 * \brief Library for salinity dynamic correction (PSS-78 conversion)
 *
 * \copyright
 * Copyright (c) 2021 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */



#ifndef LIBRBR_DYNCORRECTION_PSS_78_H
#define LIBRBR_DYNCORRECTION_PSS_78_H


/** \brief parameters for PSS-78 conversion */
#define	PSS78_A0 0.0080f
#define	PSS78_A1 -0.1692f
#define	PSS78_A2 25.3851f
#define	PSS78_A3 14.0941f
#define	PSS78_A4 -7.0261f
#define	PSS78_A5 2.7081f
#define	PSS78_B0 0.0005f
#define	PSS78_B1 -0.0056f
#define	PSS78_B2 -0.0066f
#define	PSS78_B3 -0.0375f
#define	PSS78_B4 0.0636f
#define	PSS78_B5 -0.0144f
#define	PSS78_K  0.0162f
#define	PSS78_C0 0.6766097f
#define	PSS78_C1 2.00564e-2f
#define	PSS78_C2 1.104259e-4f
#define	PSS78_C3 -6.9698e-7f
#define	PSS78_C4 1.0031e-9f
#define	PSS78_D1 3.426e-2f
#define	PSS78_D2 4.464e-4f
#define	PSS78_D3 0.4215f
#define	PSS78_D4 -3.107e-3f
#define	PSS78_E1 2.070e-4f
#define	PSS78_E2 -6.370e-8f
#define	PSS78_E3 3.989e-12f
#define PSS78_C_REF 42.914f
#define PSS78_T_REF 15.0f


/**
 * \brief Apply the practical salinity 1978 (PSS-78) equation from C,T,P input.
 *
 * \param [in] C Conductivity
 * \param [in] T Temperature
 * \param [in] P Pressure
 * \return Salinity
 */
float RBRDynCorr_PSS78(float C, float T, float P);

#endif // LIBRBR_DYNCORRECTION_PSS_78_H
