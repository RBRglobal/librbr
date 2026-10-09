/*
 * Copyright (c) 2026 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file RBRCommon.c
 *
 * \brief Library implementation.
 */

#include "RBRCommon.h"

const char *RBRCommonGeneration_name(RBRCommonGeneration generation)
{
    switch (generation) {
    case RBRCOMMON_LOGGER1:
        return "Logger1";
    case RBRCOMMON_LOGGER2:
        return "Logger2";
    case RBRCOMMON_LOGGER3:
        return "Logger3";
    case RBRCOMMON_LOGGER4:
        return "Logger4";
    case RBRCOMMON_GENERATION_COUNT:
        return "generation count";
    case RBRCOMMON_UNKNOWN_GENERATION:
    default:
        return "unknown generation";
    }
}

RBRCommonGeneration RBRCommonGeneration_fromFwType(int32_t fwType)
{
    /* Logger2 instruments report a firmware type of 100–103 (compact and
     * standard loggers) or 200 (the RBRcoda T.ODO), or none at all on early
     * firmware. */
    if (fwType == 0 || (fwType >= 100 && fwType <= 103) || fwType == 200) {
        return RBRCOMMON_LOGGER2;
    } else if ((fwType >= 104 && fwType <= 110) || (fwType >= 202 && fwType <= 205)) {
        return RBRCOMMON_LOGGER3;
    } else if (fwType == 130 || fwType == 131 || fwType == 150) {
        /* RBRsolo⁴/duet⁴ and Logger4 respectively. */
        return RBRCOMMON_LOGGER4;
    } else {
        return RBRCOMMON_UNKNOWN_GENERATION;
    }
}
