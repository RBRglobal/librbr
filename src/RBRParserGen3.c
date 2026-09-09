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

#include "RBRParserGen3.h"
/* Required for RBR_TRY. */
#include "RBRInstrumentGen3Internal.h"

const char *RBRInstrumentGen3EventType_name(RBRInstrumentGen3EventType type)
{
    switch (type)
    {
    case RBRINSTRUMENTGEN3_EVENT_UNKNOWN_OR_UNRECOGNIZED_EVENT:
    default:
        return "unknown or unrecognized event";
    case RBRINSTRUMENTGEN3_EVENT_TIME_SYNCHRONIZATION_MARKER:
        return "time synchronization marker";
    case RBRINSTRUMENTGEN3_EVENT_DISABLE_COMMAND_RECEIVED:
        return "disable command received";
    case RBRINSTRUMENTGEN3_EVENT_RUN_TIME_ERROR_ENCOUNTERED:
        return "run-time error encountered";
    case RBRINSTRUMENTGEN3_EVENT_CPU_RESET_DETECTED:
        return "CPU reset detected";
    case RBRINSTRUMENTGEN3_EVENT_ONE_OR_MORE_PARAMETERS_RECOVERED_AFTER_RESET:
        return "one or more parameters recovered after reset";
    case RBRINSTRUMENTGEN3_EVENT_RESTART_FAILED_RTC_CALENDAR_CONTENTS_NOT_VALID:
        return "restart failed: RTC/calendar contents not valid";
    case RBRINSTRUMENTGEN3_EVENT_RESTART_FAILED_LOGGER_STATUS_NOT_VALID:
        return "restart failed: logger status not valid";
    case RBRINSTRUMENTGEN3_EVENT_RESTART_FAILED_PRIMARY_SCHEDULE_PARAMETERS_COULD_NOT_BE_RECOVERED:
        return "restart failed: primary schedule parameters could not be recovered";
    case RBRINSTRUMENTGEN3_EVENT_UNABLE_TO_LOAD_ALARM_TIME_FOR_NEXT_SAMPLE:
        return "unable to load alarm time for next sample";
    case RBRINSTRUMENTGEN3_EVENT_SAMPLING_RESTARTED_AFTER_RESETTING_RTC:
        return "sampling restarted after resetting RTC";
    case RBRINSTRUMENTGEN3_EVENT_PARAMETERS_RECOVERED_SAMPLING_RESTARTED_AFTER_RESETTING_RTC:
        return "parameters recovered sampling restarted after resetting RTC";
    case RBRINSTRUMENTGEN3_EVENT_SAMPLING_STOPPED_END_TIME_REACHED:
        return "sampling stopped, end time reached";
    case RBRINSTRUMENTGEN3_EVENT_START_OF_A_RECORDED_BURST:
        return "start of a_recorded burst";
    case RBRINSTRUMENTGEN3_EVENT_START_OF_A_WAVE_BURST:
        return "start of a_wave burst";
    case RBRINSTRUMENTGEN3_EVENT_RESERVED1:
        return "reserved";
    case RBRINSTRUMENTGEN3_EVENT_STREAMING_NOW_OFF_FOR_BOTH_PORTS:
        return "streaming now OFF for both ports";
    case RBRINSTRUMENTGEN3_EVENT_STREAMING_ON_FOR_USB_OFF_FOR_SERIAL:
        return "streaming ON for USB, OFF for serial";
    case RBRINSTRUMENTGEN3_EVENT_STREAMING_OFF_FOR_USB_ON_FOR_SERIAL:
        return "streaming OFF for usb, ON for serial";
    case RBRINSTRUMENTGEN3_EVENT_STREAMING_NOW_ON_FOR_BOTH_PORTS:
        return "streaming now ON for both ports";
    case RBRINSTRUMENTGEN3_EVENT_SAMPLING_STARTED_THRESHOLD_CONDITION_SATISFIED:
        return "sampling started, threshold condition satisfied";
    case RBRINSTRUMENTGEN3_EVENT_SAMPLING_PAUSED_THRESHOLD_CONDITION_NOT_MET:
        return "sampling paused, threshold condition not met";
    case RBRINSTRUMENTGEN3_EVENT_POWER_SOURCE_SWITCHED_TO_INTERNAL_BATTERY:
        return "power source switched to internal battery";
    case RBRINSTRUMENTGEN3_EVENT_POWER_SOURCE_SWITCHED_TO_EXTERNAL_BATTERY:
        return "power source switched to external battery";
    case RBRINSTRUMENTGEN3_EVENT_TWIST_ACTIVATION_STARTED_SAMPLING:
        return "twist activation started sampling";
    case RBRINSTRUMENTGEN3_EVENT_TWIST_ACTIVATION_PAUSED_SAMPLING:
        return "twist activation paused sampling";
    case RBRINSTRUMENTGEN3_EVENT_WIFI_MODULE_DETECTED_AND_ACTIVATED:
        return "Wi-Fi module detected and activated";
    case RBRINSTRUMENTGEN3_EVENT_WIFI_MODULE_DEACTIVATED_REMOVED_OR_ACTIVITY_TIMEOUT:
        return "Wi-Fi module deactivated; removed or activity timeout";
    case RBRINSTRUMENTGEN3_EVENT_REGIMES_ENABLED_BUT_NOT_YET_IN_A_REGIME:
        return "regimes enabled, but not yet in a_regime";
    case RBRINSTRUMENTGEN3_EVENT_ENTERED_REGIME_1:
        return "entered regime 1";
    case RBRINSTRUMENTGEN3_EVENT_ENTERED_REGIME_2:
        return "entered regime 2";
    case RBRINSTRUMENTGEN3_EVENT_ENTERED_REGIME_3:
        return "entered regime 3";
    case RBRINSTRUMENTGEN3_EVENT_START_OF_REGIME_BIN:
        return "start of regime bin";
    case RBRINSTRUMENTGEN3_EVENT_BEGIN_PROFILING_UP_CAST:
        return "begin profiling 'up' cast";
    case RBRINSTRUMENTGEN3_EVENT_BEGIN_PROFILING_DOWN_CAST:
        return "begin profiling 'down' cast";
    case RBRINSTRUMENTGEN3_EVENT_END_OF_PROFILING_CAST:
        return "end of profiling cast";
    case RBRINSTRUMENTGEN3_EVENT_BATTERY_FAILED_SCHEDULE_FINISHED:
        return "battery failed, schedule finished";
    case RBRINSTRUMENTGEN3_EVENT_DIRECTIONAL_DEPENDENT_SAMPLING_BEGINNING_OF_FAST_SAMPLING_MODE:
        return "directional dependent sampling: beginning of fast sampling mode";
    case RBRINSTRUMENTGEN3_EVENT_DIRECTIONAL_DEPENDENT_SAMPLING_BEGINNING_OF_SLOW_SAMPLING_MODE:
        return "directional dependent sampling: beginning of slow sampling mode";
    case RBRINSTRUMENTGEN3_EVENT_ENERGY_USED_MARKER_INTERNAL_BATTERY:
        return "energy used marker, internal battery";
    case RBRINSTRUMENTGEN3_EVENT_ENERGY_USED_MARKER_EXTERNAL_POWER_SOURCE:
        return "energy used marker, external power source";
    }
}

RBRInstrumentGen3Error RBRParser_init(RBRParser **parser,
                                  const RBRParserCallbacks *callbacks,
                                  const RBRParserConfig *config,
                                  void *userData)
{
    if (callbacks == NULL
        || (callbacks->sample != NULL && callbacks->sampleBuffer == NULL)
        || (callbacks->event != NULL && callbacks->eventBuffer == NULL))
    {
        return RBRINSTRUMENTGEN3_MISSING_CALLBACK;
    }

    if (config->format != RBRINSTRUMENTGEN3_MEMFORMAT_CALBIN00)
    {
        return RBRINSTRUMENTGEN3_UNSUPPORTED;
    }

    if (config->formatConfig.easyParse.channels <= 0
        || config->formatConfig.easyParse.channels > RBRINSTRUMENTGEN3_CHANNEL_MAX)
    {
        return RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE;
    }

    bool allocated = false;
    if (*parser == NULL)
    {
        allocated = true;
        #ifndef RBR_LIB_NODYNAMICMEMORYALLOCATION
        if ((*parser = malloc(sizeof(RBRParser))) == NULL)
        {
        #endif
            return RBRINSTRUMENTGEN3_ALLOCATION_FAILURE;
        #ifndef RBR_LIB_NODYNAMICMEMORYALLOCATION
        }
        #endif
    }

    memset(*parser, 0, sizeof(RBRParser));
    memcpy(&(*parser)->config, config, sizeof(RBRParserConfig));
    memcpy(&(*parser)->callbacks, callbacks, sizeof(RBRParserCallbacks));
    (*parser)->userData          = userData;
    (*parser)->managedAllocation = allocated;

    return RBRINSTRUMENTGEN3_SUCCESS;
}


RBRInstrumentGen3Error RBRParser_destroy(RBRParser *parser)
{
    if (parser->managedAllocation)
    {
        #ifndef RBR_LIB_NODYNAMICMEMORYALLOCATION
        free(parser);
        #endif
    }

    return RBRINSTRUMENTGEN3_SUCCESS;
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

static RBRInstrumentGen3Error RBRParser_parseEPEvents(
    RBRParser *parser,
    const uint8_t *const data,
    int32_t *size)
{
    int32_t maxSize = *size;
    *size = 0;

    RBRInstrumentGen3Event *event = parser->callbacks.eventBuffer;
    if (event == NULL)
    {
        return RBRINSTRUMENTGEN3_SUCCESS;
    }

    for (; *size + EP_EVENT_SIZE <= maxSize; *size += EP_EVENT_SIZE)
    {
        memset(event, 0, sizeof(RBRInstrumentGen3Event));

        event->type = *(uint8_t *) (data + *size + EP_EVENT_TYPE_OFFSET);
        event->timestamp =
            *(RBRInstrumentGen3DateTime *) (data
                                        + *size
                                        + EP_EVENT_TIMESTAMP_OFFSET);
        switch (event->type)
        {
        case RBRINSTRUMENTGEN3_EVENT_START_OF_REGIME_BIN:
        case RBRINSTRUMENTGEN3_EVENT_BEGIN_PROFILING_UP_CAST:
        case RBRINSTRUMENTGEN3_EVENT_BEGIN_PROFILING_DOWN_CAST:
        case RBRINSTRUMENTGEN3_EVENT_END_OF_PROFILING_CAST:
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

    return RBRINSTRUMENTGEN3_SUCCESS;
}

#define EP_SAMPLE_TIMESTAMP_SIZE ((int32_t) sizeof(RBRInstrumentGen3DateTime))
#define EP_SAMPLE_READING_SIZE ((int32_t) sizeof(float))

static RBRInstrumentGen3Error RBRParser_parseEPSamples(
    RBRParser *parser,
    const uint8_t *const data,
    int32_t *size)
{
    int32_t maxSize = *size;
    *size = 0;

    RBRInstrumentGen3Sample *sample = parser->callbacks.sampleBuffer;
    if (sample == NULL)
    {
        return RBRINSTRUMENTGEN3_SUCCESS;
    }

    int32_t channels = parser->config.formatConfig.easyParse.channels;
    int32_t sampleSize = EP_SAMPLE_TIMESTAMP_SIZE
                         + EP_SAMPLE_READING_SIZE * channels;
    for (; *size + sampleSize <= maxSize; *size += sampleSize)
    {
        memset(sample, 0, sizeof(RBRInstrumentGen3Sample));

        sample->timestamp = *(RBRInstrumentGen3DateTime *) (data + *size);
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

    return RBRINSTRUMENTGEN3_SUCCESS;
}

RBRInstrumentGen3Error RBRParser_parse(RBRParser *parser,
                                   RBRInstrumentGen3Dataset dataset,
                                   const void *const data,
                                   int32_t *size)
{
    const uint8_t *d = (const uint8_t *const) data;

    switch (dataset)
    {
    case RBRINSTRUMENTGEN3_DATASET_EASYPARSE_EVENTS:
        return RBRParser_parseEPEvents(parser, d, size);
    case RBRINSTRUMENTGEN3_DATASET_EASYPARSE_SAMPLE_DATA:
        return RBRParser_parseEPSamples(parser, d, size);
    case RBRINSTRUMENTGEN3_DATASET_EASYPARSE_DEPLOYMENT_HEADER:
    default:
        return RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE;
    }
}
