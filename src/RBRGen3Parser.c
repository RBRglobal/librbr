/**
 * \file RBRGen3Parser.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for NAN. */
#include <math.h>
/* Required for memcpy, memset. */
#include <string.h>

#include <stdio.h>
#include <stddef.h>

#include "RBRGen3Parser.h"
/* Required for RBR_TRY. */
#include "RBRGen3Internal.h"
#include "RBRGen3Memory.h"
#include "RBRGen3Streaming.h"

const char *RBRGen3EventType_name(RBRGen3EventType type)
{
    switch (type) {
    case RBRGEN3_EVENT_UNKNOWN_OR_UNRECOGNIZED_EVENT:
    default:
        return "unknown or unrecognized event";
    case RBRGEN3_EVENT_TIME_SYNCHRONIZATION_MARKER:
        return "time synchronization marker";
    case RBRGEN3_EVENT_DISABLE_COMMAND_RECEIVED:
        return "disable command received";
    case RBRGEN3_EVENT_RUN_TIME_ERROR_ENCOUNTERED:
        return "run-time error encountered";
    case RBRGEN3_EVENT_CPU_RESET_DETECTED:
        return "CPU reset detected";
    case RBRGEN3_EVENT_ONE_OR_MORE_PARAMETERS_RECOVERED_AFTER_RESET:
        return "one or more parameters recovered after reset";
    case RBRGEN3_EVENT_RESTART_FAILED_RTC_CALENDAR_CONTENTS_NOT_VALID:
        return "restart failed: RTC/calendar contents not valid";
    case RBRGEN3_EVENT_RESTART_FAILED_LOGGER_STATUS_NOT_VALID:
        return "restart failed: logger status not valid";
    case RBRGEN3_EVENT_RESTART_FAILED_PRIMARY_SCHEDULE_PARAMETERS_COULD_NOT_BE_RECOVERED:
        return "restart failed: primary schedule parameters could not be recovered";
    case RBRGEN3_EVENT_UNABLE_TO_LOAD_ALARM_TIME_FOR_NEXT_SAMPLE:
        return "unable to load alarm time for next sample";
    case RBRGEN3_EVENT_SAMPLING_RESTARTED_AFTER_RESETTING_RTC:
        return "sampling restarted after resetting RTC";
    case RBRGEN3_EVENT_PARAMETERS_RECOVERED_SAMPLING_RESTARTED_AFTER_RESETTING_RTC:
        return "parameters recovered sampling restarted after resetting RTC";
    case RBRGEN3_EVENT_SAMPLING_STOPPED_END_TIME_REACHED:
        return "sampling stopped, end time reached";
    case RBRGEN3_EVENT_START_OF_A_RECORDED_BURST:
        return "start of a_recorded burst";
    case RBRGEN3_EVENT_START_OF_A_WAVE_BURST:
        return "start of a_wave burst";
    case RBRGEN3_EVENT_RESERVED1:
        return "reserved";
    case RBRGEN3_EVENT_STREAMING_NOW_OFF_FOR_BOTH_PORTS:
        return "streaming now OFF for both ports";
    case RBRGEN3_EVENT_STREAMING_ON_FOR_USB_OFF_FOR_SERIAL:
        return "streaming ON for USB, OFF for serial";
    case RBRGEN3_EVENT_STREAMING_OFF_FOR_USB_ON_FOR_SERIAL:
        return "streaming OFF for usb, ON for serial";
    case RBRGEN3_EVENT_STREAMING_NOW_ON_FOR_BOTH_PORTS:
        return "streaming now ON for both ports";
    case RBRGEN3_EVENT_SAMPLING_STARTED_THRESHOLD_CONDITION_SATISFIED:
        return "sampling started, threshold condition satisfied";
    case RBRGEN3_EVENT_SAMPLING_PAUSED_THRESHOLD_CONDITION_NOT_MET:
        return "sampling paused, threshold condition not met";
    case RBRGEN3_EVENT_POWER_SOURCE_SWITCHED_TO_INTERNAL_BATTERY:
        return "power source switched to internal battery";
    case RBRGEN3_EVENT_POWER_SOURCE_SWITCHED_TO_EXTERNAL_BATTERY:
        return "power source switched to external battery";
    case RBRGEN3_EVENT_TWIST_ACTIVATION_STARTED_SAMPLING:
        return "twist activation started sampling";
    case RBRGEN3_EVENT_TWIST_ACTIVATION_PAUSED_SAMPLING:
        return "twist activation paused sampling";
    case RBRGEN3_EVENT_WIFI_MODULE_DETECTED_AND_ACTIVATED:
        return "Wi-Fi module detected and activated";
    case RBRGEN3_EVENT_WIFI_MODULE_DEACTIVATED_REMOVED_OR_ACTIVITY_TIMEOUT:
        return "Wi-Fi module deactivated; removed or activity timeout";
    case RBRGEN3_EVENT_REGIMES_ENABLED_BUT_NOT_YET_IN_A_REGIME:
        return "regimes enabled, but not yet in a_regime";
    case RBRGEN3_EVENT_ENTERED_REGIME_1:
        return "entered regime 1";
    case RBRGEN3_EVENT_ENTERED_REGIME_2:
        return "entered regime 2";
    case RBRGEN3_EVENT_ENTERED_REGIME_3:
        return "entered regime 3";
    case RBRGEN3_EVENT_START_OF_REGIME_BIN:
        return "start of regime bin";
    case RBRGEN3_EVENT_BEGIN_PROFILING_UP_CAST:
        return "begin profiling 'up' cast";
    case RBRGEN3_EVENT_BEGIN_PROFILING_DOWN_CAST:
        return "begin profiling 'down' cast";
    case RBRGEN3_EVENT_END_OF_PROFILING_CAST:
        return "end of profiling cast";
    case RBRGEN3_EVENT_BATTERY_FAILED_SCHEDULE_FINISHED:
        return "battery failed, schedule finished";
    case RBRGEN3_EVENT_DIRECTIONAL_DEPENDENT_SAMPLING_BEGINNING_OF_FAST_SAMPLING_MODE:
        return "directional dependent sampling: beginning of fast sampling mode";
    case RBRGEN3_EVENT_DIRECTIONAL_DEPENDENT_SAMPLING_BEGINNING_OF_SLOW_SAMPLING_MODE:
        return "directional dependent sampling: beginning of slow sampling mode";
    case RBRGEN3_EVENT_ENERGY_USED_MARKER_INTERNAL_BATTERY:
        return "energy used marker, internal battery";
    case RBRGEN3_EVENT_ENERGY_USED_MARKER_EXTERNAL_POWER_SOURCE:
        return "energy used marker, external power source";
    }
}

#define EP_SAMPLE_TIMESTAMP_SIZE ((int32_t) sizeof(RBRGen3DateTime))
#define EP_SAMPLE_READING_SIZE   ((int32_t) sizeof(float))

RBRGen3Error RBRGen3Parser_init(RBRGen3Parser *parser, const RBRGen3ParserCallbacks *callbacks,
                                const RBRGen3ParserConfig *config, void *userData)
{
    if (callbacks == NULL || (callbacks->sample != NULL && callbacks->sampleBuffer == NULL) ||
        (callbacks->event != NULL && callbacks->eventBuffer == NULL)) {
        return RBRGEN3_MISSING_CALLBACK;
    }

    if (config->format != RBRGEN3_MEMFORMAT_CALBIN00) {
        return RBRGEN3_UNSUPPORTED;
    }

    if (config->formatConfig.easyParse.channels <= 0 ||
        config->formatConfig.easyParse.channels > RBRGEN3_EASYPARSE_CHANNELS_MAX) {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }
    if (callbacks->sampleBuffer != NULL &&
        (callbacks->sampleBuffer->readings == NULL || callbacks->sampleBuffer->size <= 0)) {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    memset(parser, 0, sizeof(RBRGen3Parser));
    memcpy(&parser->config, config, sizeof(RBRGen3ParserConfig));
    memcpy(&parser->callbacks, callbacks, sizeof(RBRGen3ParserCallbacks));
    parser->userData = userData;

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3Parser_destroy(RBRGen3Parser *parser)
{
    /* The library holds no resources, so there is nothing to release. This
     * function is kept so that callers pair every init with a destroy and so
     * that resource management can be added later without an API change. */
    memset(parser, 0, sizeof(RBRGen3Parser));
    return RBRGEN3_SUCCESS;
}

void RBRGen3Parser_getConfig(const RBRGen3Parser *parser, RBRGen3ParserConfig *config)
{
    memcpy(config, &parser->config, sizeof(RBRGen3ParserConfig));
}

void *RBRGen3Parser_getUserData(const RBRGen3Parser *parser)
{
    return parser->userData;
}

void RBRGen3Parser_setUserData(RBRGen3Parser *parser, void *userData)
{
    parser->userData = userData;
}

#define EP_EVENT_SIZE             16
#define EP_EVENT_CRC_OFFSET       0
#define EP_EVENT_TYPE_OFFSET      2
#define EP_EVENT_MARKER_OFFSET    3
#define EP_EVENT_TIMESTAMP_OFFSET 4
#define EP_EVENT_PAYLOAD_OFFSET   12

static RBRGen3Error RBRGen3Parser_parseEPEvents(RBRGen3Parser *parser, const uint8_t *const data,
                                                int32_t *size)
{
    int32_t maxSize = *size;
    *size = 0;

    RBRGen3Event *event = parser->callbacks.eventBuffer;
    if (event == NULL) {
        return RBRGEN3_SUCCESS;
    }

    for (; EP_EVENT_SIZE <= maxSize - *size; *size += EP_EVENT_SIZE) {
        memset(event, 0, sizeof(RBRGen3Event));

        event->type = *(uint8_t *) (data + *size + EP_EVENT_TYPE_OFFSET);
        event->timestamp = *(RBRGen3DateTime *) (data + *size + EP_EVENT_TIMESTAMP_OFFSET);
        switch (event->type) {
        case RBRGEN3_EVENT_START_OF_REGIME_BIN:
        case RBRGEN3_EVENT_BEGIN_PROFILING_UP_CAST:
        case RBRGEN3_EVENT_BEGIN_PROFILING_DOWN_CAST:
        case RBRGEN3_EVENT_END_OF_PROFILING_CAST:
            event->auxiliaryDataLength = 1;
            event->auxiliaryData[0] = *(uint32_t *) (data + *size + EP_EVENT_PAYLOAD_OFFSET);
            break;
        default:
            event->auxiliaryDataLength = 0;
        }

        if (parser->callbacks.event != NULL) {
            RBR_TRY(parser->callbacks.event(parser, event));
        }
    }

    return RBRGEN3_SUCCESS;
}

static RBRGen3Error RBRGen3Parser_parseEPSamples(RBRGen3Parser *parser, const uint8_t *const data,
                                                 int32_t *size)
{
    int32_t maxSize = *size;
    *size = 0;

    RBRGen3Sample *sample = parser->callbacks.sampleBuffer;
    if (sample == NULL) {
        return RBRGEN3_SUCCESS;
    }

    int32_t channels = parser->config.formatConfig.easyParse.channels;
    int32_t sampleSize = EP_SAMPLE_TIMESTAMP_SIZE + EP_SAMPLE_READING_SIZE * channels;
    /* Readings past the caller's storage are dropped and flagged. */
    int32_t stored = channels < sample->size ? channels : sample->size;
    for (; sampleSize <= maxSize - *size; *size += sampleSize) {
        memset(sample->readings, 0, (size_t) sample->size * sizeof(*sample->readings));

        sample->timestamp = *(RBRGen3DateTime *) (data + *size);
        sample->channelCount = stored;
        sample->readingsDropped = stored < channels;
        for (int32_t channel = 0; channel < stored; ++channel) {
            sample->readings[channel] =
                (double) *(float *) (data + *size + EP_SAMPLE_TIMESTAMP_SIZE +
                                     channel * EP_SAMPLE_READING_SIZE);
        }

        if (parser->callbacks.sample != NULL) {
            RBR_TRY(parser->callbacks.sample(
                parser, sample)); /* calls the parser-> callback.sample function. */
        }
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3Parser_parse(RBRGen3Parser *parser, RBRGen3Dataset dataset,
                                 const void *const data, int32_t *size)
{
    const uint8_t *d = (const uint8_t *const) data;

    switch (dataset) {
    case RBRGEN3_DATASET_EASYPARSE_EVENTS:
        return RBRGen3Parser_parseEPEvents(parser, d, size);
    case RBRGEN3_DATASET_EASYPARSE_SAMPLE_DATA:
        return RBRGen3Parser_parseEPSamples(parser, d, size);
    case RBRGEN3_DATASET_EASYPARSE_DEPLOYMENT_HEADER:
    default:
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }
}
