/*
 * Copyright (c) 2018 RBR Ltd.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "RBRInstrument.h"
#include "RBRParser.h"

#ifdef CONFIG_LIBRBR_DYNAMIC_CORRECTION
#include "RBRDynamicCorrection.h"
#endif

LOG_MODULE_REGISTER(rbr_sample, CONFIG_LIBRBR_LOG_LEVEL);

int main(void)
{
    LOG_INF("RBR Library Sample Application");
    LOG_INF("Library Version: %s", RBR_LIB_VERSION);
    LOG_INF("Build Date: %s", RBR_LIB_BUILD_DATE);
    
    /* Initialize RBR instrument structure */
    RBRInstrument instrument;
    RBRInstrumentError err;
    
    /* Example: Initialize instrument with default settings */
    err = RBRInstrument_open(&instrument, NULL);
    if (err != RBRINSTRUMENT_SUCCESS) {
        LOG_ERR("Failed to initialize RBR instrument: %d", err);
        return -1;
    }
    
    LOG_INF("RBR instrument initialized successfully");
    
    /* Example: Get instrument information */
    char info[256];
    err = RBRInstrument_getInfo(&instrument, info, sizeof(info));
    if (err == RBRINSTRUMENT_SUCCESS) {
        LOG_INF("Instrument info: %s", info);
    } else {
        LOG_WRN("Could not retrieve instrument info: %d", err);
    }
    
#ifdef CONFIG_LIBRBR_DYNAMIC_CORRECTION
    LOG_INF("Dynamic correction library is available");
    
    /* Example dynamic correction usage would go here */
    RBRDynamicCorrectionConfig dcConfig;
    RBRDynamicCorrectionError dcErr;
    
    dcErr = RBRDynamicCorrection_initConfig(&dcConfig);
    if (dcErr == RBRDYNAMICCORRECTION_SUCCESS) {
        LOG_INF("Dynamic correction config initialized");
    }
#endif
    
    /* Close instrument */
    RBRInstrument_close(&instrument);
    
    LOG_INF("Sample application completed");
    
    return 0;
}