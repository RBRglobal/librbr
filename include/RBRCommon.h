/**
 * \file RBRCommon.h
 *
 * \brief Generation-independent helpers.
 *
 * Everything declared here is compiled into every build of the library,
 * whichever instrument generations are enabled, and depends on neither
 * generation API.
 *
 * \copyright
 * Copyright (c) 2026 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRCOMMON_H
#define LIBRBR_RBRCOMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/* Required for int32_t. */
#include <inttypes.h>

/** \brief Generations of RBR instruments. */
typedef enum RBRCommonGeneration {
    /** Logger1 (XR/XRX/TR/DR/TDR/HT). */
    RBRCOMMON_LOGGER1,
    /** Logger2 (RBRvirtuoso/duo/concerto/maestro/solo/duet/coda). */
    RBRCOMMON_LOGGER2,
    /** Logger3 (RBRvirtuoso³/duo³/concerto³/maestro³/solo³/duet³/coda³). */
    RBRCOMMON_LOGGER3,
    /** 4th generation instruments (RBRsolo⁴/duet⁴/coda⁴, etc.). */
    RBRCOMMON_LOGGER4,
    /** The number of known generations. */
    RBRCOMMON_GENERATION_COUNT,
    /** An unknown or unrecognized instrument generation. */
    RBRCOMMON_UNKNOWN_GENERATION
} RBRCommonGeneration;

/**
 * \brief Get a human-readable string name for a generation.
 *
 * Contrary to convention for values returned by other enum `_name` functions,
 * the generation names returned by this function are capitalized: “Logger3”
 * instead of “logger3”.
 *
 * \param [in] generation the generation
 * \return a string name for the generation
 */
const char *RBRCommonGeneration_name(RBRCommonGeneration generation);

/**
 * \brief Classify an instrument by the firmware type it reports.
 *
 * A firmware type of 0 is treated as Logger2: the concept of firmware type
 * was introduced part-way through Logger2, so instruments with very old
 * firmware do not report one.
 *
 * \param [in] fwtype the firmware type from the `id` (or `id4`) command
 * \return the generation the firmware type belongs to
 * \return #RBRCOMMON_UNKNOWN_GENERATION for a firmware type this library does
 *         not know
 */
RBRCommonGeneration RBRCommonGeneration_fromFwtype(int32_t fwtype);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRCOMMON_H */
