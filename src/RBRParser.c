/**
 * \file RBRParser.c
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

#include "RBRParser.h"
/* Required for RBR_TRY. */
#include "RBRInstrumentInternal.h"

const char *RBRInstrumentEventType_name(RBRInstrumentEventType type)
{
    switch (type)
    {
    case RBRINSTRUMENT_EVENT_UNKNOWN_OR_UNRECOGNIZED_EVENT:
    default:
        return "unknown or unrecognized event";
    case RBRINSTRUMENT_EVENT_TIME_SYNCHRONIZATION_MARKER:
        return "time synchronization marker";
    case RBRINSTRUMENT_EVENT_DISABLE_COMMAND_RECEIVED:
        return "disable command received";
    case RBRINSTRUMENT_EVENT_RUN_TIME_ERROR_ENCOUNTERED:
        return "run-time error encountered";
    case RBRINSTRUMENT_EVENT_CPU_RESET_DETECTED:
        return "CPU reset detected";
    case RBRINSTRUMENT_EVENT_ONE_OR_MORE_PARAMETERS_RECOVERED_AFTER_RESET:
        return "one or more parameters recovered after reset";
    case RBRINSTRUMENT_EVENT_RESTART_FAILED_RTC_CALENDAR_CONTENTS_NOT_VALID:
        return "restart failed: RTC/calendar contents not valid";
    case RBRINSTRUMENT_EVENT_RESTART_FAILED_LOGGER_STATUS_NOT_VALID:
        return "restart failed: logger status not valid";
    case RBRINSTRUMENT_EVENT_RESTART_FAILED_PRIMARY_SCHEDULE_PARAMETERS_COULD_NOT_BE_RECOVERED:
        return "restart failed: primary schedule parameters could not be recovered";
    case RBRINSTRUMENT_EVENT_UNABLE_TO_LOAD_ALARM_TIME_FOR_NEXT_SAMPLE:
        return "unable to load alarm time for next sample";
    case RBRINSTRUMENT_EVENT_SAMPLING_RESTARTED_AFTER_RESETTING_RTC:
        return "sampling restarted after resetting RTC";
    case RBRINSTRUMENT_EVENT_PARAMETERS_RECOVERED_SAMPLING_RESTARTED_AFTER_RESETTING_RTC:
        return "parameters recovered sampling restarted after resetting RTC";
    case RBRINSTRUMENT_EVENT_SAMPLING_STOPPED_END_TIME_REACHED:
        return "sampling stopped, end time reached";
    case RBRINSTRUMENT_EVENT_START_OF_A_RECORDED_BURST:
        return "start of a_recorded burst";
    case RBRINSTRUMENT_EVENT_START_OF_A_WAVE_BURST:
        return "start of a_wave burst";
    case RBRINSTRUMENT_EVENT_RESERVED1:
        return "reserved";
    case RBRINSTRUMENT_EVENT_STREAMING_NOW_OFF_FOR_BOTH_PORTS:
        return "streaming now OFF for both ports";
    case RBRINSTRUMENT_EVENT_STREAMING_ON_FOR_USB_OFF_FOR_SERIAL:
        return "streaming ON for USB, OFF for serial";
    case RBRINSTRUMENT_EVENT_STREAMING_OFF_FOR_USB_ON_FOR_SERIAL:
        return "streaming OFF for usb, ON for serial";
    case RBRINSTRUMENT_EVENT_STREAMING_NOW_ON_FOR_BOTH_PORTS:
        return "streaming now ON for both ports";
    case RBRINSTRUMENT_EVENT_SAMPLING_STARTED_THRESHOLD_CONDITION_SATISFIED:
        return "sampling started, threshold condition satisfied";
    case RBRINSTRUMENT_EVENT_SAMPLING_PAUSED_THRESHOLD_CONDITION_NOT_MET:
        return "sampling paused, threshold condition not met";
    case RBRINSTRUMENT_EVENT_POWER_SOURCE_SWITCHED_TO_INTERNAL_BATTERY:
        return "power source switched to internal battery";
    case RBRINSTRUMENT_EVENT_POWER_SOURCE_SWITCHED_TO_EXTERNAL_BATTERY:
        return "power source switched to external battery";
    case RBRINSTRUMENT_EVENT_TWIST_ACTIVATION_STARTED_SAMPLING:
        return "twist activation started sampling";
    case RBRINSTRUMENT_EVENT_TWIST_ACTIVATION_PAUSED_SAMPLING:
        return "twist activation paused sampling";
    case RBRINSTRUMENT_EVENT_WIFI_MODULE_DETECTED_AND_ACTIVATED:
        return "Wi-Fi module detected and activated";
    case RBRINSTRUMENT_EVENT_WIFI_MODULE_DEACTIVATED_REMOVED_OR_ACTIVITY_TIMEOUT:
        return "Wi-Fi module deactivated; removed or activity timeout";
    case RBRINSTRUMENT_EVENT_REGIMES_ENABLED_BUT_NOT_YET_IN_A_REGIME:
        return "regimes enabled, but not yet in a_regime";
    case RBRINSTRUMENT_EVENT_ENTERED_REGIME_1:
        return "entered regime 1";
    case RBRINSTRUMENT_EVENT_ENTERED_REGIME_2:
        return "entered regime 2";
    case RBRINSTRUMENT_EVENT_ENTERED_REGIME_3:
        return "entered regime 3";
    case RBRINSTRUMENT_EVENT_START_OF_REGIME_BIN:
        return "start of regime bin";
    case RBRINSTRUMENT_EVENT_BEGIN_PROFILING_UP_CAST:
        return "begin profiling 'up' cast";
    case RBRINSTRUMENT_EVENT_BEGIN_PROFILING_DOWN_CAST:
        return "begin profiling 'down' cast";
    case RBRINSTRUMENT_EVENT_END_OF_PROFILING_CAST:
        return "end of profiling cast";
    case RBRINSTRUMENT_EVENT_BATTERY_FAILED_SCHEDULE_FINISHED:
        return "battery failed, schedule finished";
    case RBRINSTRUMENT_EVENT_DIRECTIONAL_DEPENDENT_SAMPLING_BEGINNING_OF_FAST_SAMPLING_MODE:
        return "directional dependent sampling: beginning of fast sampling mode";
    case RBRINSTRUMENT_EVENT_DIRECTIONAL_DEPENDENT_SAMPLING_BEGINNING_OF_SLOW_SAMPLING_MODE:
        return "directional dependent sampling: beginning of slow sampling mode";
    case RBRINSTRUMENT_EVENT_ENERGY_USED_MARKER_INTERNAL_BATTERY:
        return "energy used marker, internal battery";
    case RBRINSTRUMENT_EVENT_ENERGY_USED_MARKER_EXTERNAL_POWER_SOURCE:
        return "energy used marker, external power source";
    }
}

RBRInstrumentError RBRParser_init(RBRParser **parser,
                                  const RBRParserCallbacks *callbacks,
                                  const RBRParserConfig *config,
                                  void *userData)
{
    if (callbacks == NULL
        || (callbacks->sample != NULL && callbacks->sampleBuffer == NULL)
        || (callbacks->event != NULL && callbacks->eventBuffer == NULL))
    {
        return RBRINSTRUMENT_MISSING_CALLBACK;
    }

    if (config->format != RBRINSTRUMENT_MEMFORMAT_CALBIN00)
    {
        return RBRINSTRUMENT_UNSUPPORTED;
    }

    if (config->formatConfig.easyParse.channels <= 0
        || config->formatConfig.easyParse.channels > RBRINSTRUMENT_CHANNEL_MAX)
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    bool allocated = false;
    if (*parser == NULL)
    {
        allocated = true;
        #ifndef RBR_LIB_NODYNAMICMEMORYALLOCATION
        if ((*parser = malloc(sizeof(RBRParser))) == NULL)
        {
        #endif
            return RBRINSTRUMENT_ALLOCATION_FAILURE;
        #ifndef RBR_LIB_NODYNAMICMEMORYALLOCATION
        }
        #endif
    }

    memset(*parser, 0, sizeof(RBRParser));
    memcpy(&(*parser)->config, config, sizeof(RBRParserConfig));
    memcpy(&(*parser)->callbacks, callbacks, sizeof(RBRParserCallbacks));
    (*parser)->userData          = userData;
    (*parser)->managedAllocation = allocated;

    return RBRINSTRUMENT_SUCCESS;
}


RBRInstrumentError RBRParser_destroy(RBRParser *parser)
{
    if (parser->managedAllocation)
    {
        #ifndef RBR_LIB_NODYNAMICMEMORYALLOCATION
        free(parser);
        #endif
    }

    return RBRINSTRUMENT_SUCCESS;
}

void RBRParser_getConfig(const RBRParser *parser, RBRParserConfig *config)
{
    memcpy(config, &parser->config, sizeof(RBRParserConfig));
}

void *RBRParser_getUserData(const RBRParser *parser)
{
    return parser->userData;
}

void RBRParser_setUserData(RBRParser *parser, void *userData)
{
    parser->userData = userData;
}

#define EP_EVENT_SIZE             16
#define EP_EVENT_CRC_OFFSET       0
#define EP_EVENT_TYPE_OFFSET      2
#define EP_EVENT_MARKER_OFFSET    3
#define EP_EVENT_TIMESTAMP_OFFSET 4
#define EP_EVENT_PAYLOAD_OFFSET   12

static RBRInstrumentError RBRParser_parseEPEvents(
    RBRParser *parser,
    const uint8_t *const data,
    int32_t *size)
{
    int32_t maxSize = *size;
    *size = 0;

    RBRInstrumentEvent *event = parser->callbacks.eventBuffer;
    if (event == NULL)
    {
        return RBRINSTRUMENT_SUCCESS;
    }

    for (; *size + EP_EVENT_SIZE <= maxSize; *size += EP_EVENT_SIZE)
    {
        memset(event, 0, sizeof(RBRInstrumentEvent));

        event->type = *(uint8_t *) (data + *size + EP_EVENT_TYPE_OFFSET);
        event->timestamp =
            *(RBRInstrumentDateTime *) (data
                                        + *size
                                        + EP_EVENT_TIMESTAMP_OFFSET);
        switch (event->type)
        {
        case RBRINSTRUMENT_EVENT_START_OF_REGIME_BIN:
        case RBRINSTRUMENT_EVENT_BEGIN_PROFILING_UP_CAST:
        case RBRINSTRUMENT_EVENT_BEGIN_PROFILING_DOWN_CAST:
        case RBRINSTRUMENT_EVENT_END_OF_PROFILING_CAST:
            event->auxiliaryDataLength = 1;
            event->auxiliaryData[0] =
                *(uint32_t *) (data + *size + EP_EVENT_PAYLOAD_OFFSET);
            break;
        default:
            event->auxiliaryDataLength = 0;
        }

        if (parser->callbacks.event != NULL)
        {
            RBR_TRY(parser->callbacks.event(parser, event));
        }
    }

    return RBRINSTRUMENT_SUCCESS;
}

#define EP_SAMPLE_TIMESTAMP_SIZE ((int32_t) sizeof(RBRInstrumentDateTime))
#define EP_SAMPLE_READING_SIZE ((int32_t) sizeof(float))

static RBRInstrumentError RBRParser_parseEPSamples(
    RBRParser *parser,
    const uint8_t *const data,
    int32_t *size)
{
    int32_t maxSize = *size;
    *size = 0;

    RBRInstrumentSample *sample = parser->callbacks.sampleBuffer;
    if (sample == NULL)
    {
        return RBRINSTRUMENT_SUCCESS;
    }

    int32_t channels = parser->config.formatConfig.easyParse.channels;
    int32_t sampleSize = EP_SAMPLE_TIMESTAMP_SIZE
                         + EP_SAMPLE_READING_SIZE * channels;
    for (; *size + sampleSize <= maxSize; *size += sampleSize)
    {
        memset(sample, 0, sizeof(RBRInstrumentSample));

        sample->timestamp = *(RBRInstrumentDateTime *) (data + *size);
        sample->channels = channels;
        for (int32_t channel = 0; channel < channels; ++channel)
        {
            sample->readings[channel] =
                (double)
                *(float *) (data
                            + *size
                            + EP_SAMPLE_TIMESTAMP_SIZE
                            + channel * EP_SAMPLE_READING_SIZE);
        }

        if (parser->callbacks.sample != NULL)
        {
            RBR_TRY(parser->callbacks.sample(parser, sample)); //calls the parser-> callback.sample function.
        }
    }

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRParser_parse(RBRParser *parser,
                                   RBRInstrumentDataset dataset,
                                   const void *const data,
                                   int32_t *size)
{
    const uint8_t *d = (const uint8_t *const) data;

    switch (dataset)
    {
    case RBRINSTRUMENT_DATASET_EASYPARSE_EVENTS:
        return RBRParser_parseEPEvents(parser, d, size);
    case RBRINSTRUMENT_DATASET_EASYPARSE_SAMPLE_DATA:
        return RBRParser_parseEPSamples(parser, d, size);
    case RBRINSTRUMENT_DATASET_EASYPARSE_DEPLOYMENT_HEADER:
    default:
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }
}
