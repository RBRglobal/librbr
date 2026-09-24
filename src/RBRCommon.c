/**
 * \file RBRCommon.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2026 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
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

RBRCommonGeneration RBRCommonGeneration_fromFwtype(int32_t fwtype)
{
    /* Logger2 instruments report a firmware type of 100–103 (compact and
     * standard loggers) or 200 (the RBRcoda T.ODO), or none at all on early
     * firmware. */
    if (fwtype == 0 || (fwtype >= 100 && fwtype <= 103) || fwtype == 200) {
        return RBRCOMMON_LOGGER2;
    } else if ((fwtype >= 104 && fwtype <= 110) || (fwtype >= 202 && fwtype <= 205)) {
        return RBRCOMMON_LOGGER3;
    } else if (fwtype == 130 || fwtype == 131 || fwtype == 150) {
        /* RBRsolo⁴/duet⁴ and Logger4 respectively. */
        return RBRCOMMON_LOGGER4;
    } else {
        return RBRCOMMON_UNKNOWN_GENERATION;
    }
}
