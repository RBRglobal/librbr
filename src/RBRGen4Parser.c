/**
 * \file RBRGen4Parser.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for memcpy, memset. */
#include <string.h>

#include "RBRGen4.h"
#include "RBRGen4Internal.h"
#include "RBRGen4Parser.h"

const char *RBRGen4EventType_name(RBRGen4EventType type)
{
    switch (type)
    {
    case RBRGEN4_EVENT_UNKNOWN_OR_UNRECOGNIZED_EVENT:
    default:
        return "unknown or unrecognized event";
    case RBRGEN4_EVENT_TIME_SYNCHRONIZATION_MARKER:
        return "time synchronization marker";
    case RBRGEN4_EVENT_DISABLE_COMMAND_RECEIVED:
        return "disable command received";
    case RBRGEN4_EVENT_RUN_TIME_ERROR_ENCOUNTERED:
        return "run-time error encountered";
    case RBRGEN4_EVENT_CPU_RESET_DETECTED:
        return "CPU reset detected";
    case RBRGEN4_EVENT_ONE_OR_MORE_PARAMETERS_RECOVERED_AFTER_RESET:
        return "one or more parameters recovered after reset";
    case RBRGEN4_EVENT_RESTART_FAILED_RTC_CALENDAR_CONTENTS_NOT_VALID:
        return "restart failed: RTC/calendar contents not valid";
    case RBRGEN4_EVENT_RESTART_FAILED_LOGGER_STATUS_NOT_VALID:
        return "restart failed: logger status not valid";
    case RBRGEN4_EVENT_RESTART_FAILED_PRIMARY_SCHEDULE_PARAMETERS_COULD_NOT_BE_RECOVERED:
        return "restart failed: primary schedule parameters could not be recovered";
    case RBRGEN4_EVENT_UNABLE_TO_LOAD_ALARM_TIME_FOR_NEXT_SAMPLE:
        return "unable to load alarm time for next sample";
    case RBRGEN4_EVENT_SAMPLING_RESTARTED_AFTER_RESETTING_RTC:
        return "sampling restarted after resetting RTC";
    case RBRGEN4_EVENT_PARAMETERS_RECOVERED_SAMPLING_RESTARTED_AFTER_RESETTING_RTC:
        return "parameters recovered sampling restarted after resetting RTC";
    case RBRGEN4_EVENT_SAMPLING_STOPPED_END_TIME_REACHED:
        return "sampling stopped, end time reached";
    case RBRGEN4_EVENT_START_OF_A_RECORDED_BURST:
        return "start of a_recorded burst";
    case RBRGEN4_EVENT_START_OF_A_WAVE_BURST:
        return "start of a_wave burst";
    case RBRGEN4_EVENT_POWER_SOURCE_SWITCHED_TO_USB:
        return "power source switched to USB";
    case RBRGEN4_EVENT_STREAMING_NOW_OFF_FOR_BOTH_PORTS:
        return "streaming now OFF for both ports";
    case RBRGEN4_EVENT_STREAMING_ON_FOR_USB_OFF_FOR_SERIAL:
        return "streaming ON for USB, OFF for serial";
    case RBRGEN4_EVENT_STREAMING_OFF_FOR_USB_ON_FOR_SERIAL:
        return "streaming OFF for usb, ON for serial";
    case RBRGEN4_EVENT_STREAMING_NOW_ON_FOR_BOTH_PORTS:
        return "streaming now ON for both ports";
    case RBRGEN4_EVENT_SAMPLING_STARTED_THRESHOLD_CONDITION_SATISFIED:
        return "sampling started, threshold condition satisfied";
    case RBRGEN4_EVENT_SAMPLING_PAUSED_THRESHOLD_CONDITION_NOT_MET:
        return "sampling paused, threshold condition not met";
    case RBRGEN4_EVENT_POWER_SOURCE_SWITCHED_TO_INTERNAL_BATTERY:
        return "power source switched to internal battery";
    case RBRGEN4_EVENT_POWER_SOURCE_SWITCHED_TO_EXTERNAL_BATTERY:
        return "power source switched to external battery";
    case RBRGEN4_EVENT_TWIST_ACTIVATION_STARTED_SAMPLING:
        return "twist activation started sampling";
    case RBRGEN4_EVENT_TWIST_ACTIVATION_PAUSED_SAMPLING:
        return "twist activation paused sampling";
    case RBRGEN4_EVENT_WIFI_MODULE_DETECTED_AND_ACTIVATED:
        return "Wi-Fi module detected and activated";
    case RBRGEN4_EVENT_WIFI_MODULE_DEACTIVATED_REMOVED_OR_ACTIVITY_TIMEOUT:
        return "Wi-Fi module deactivated; removed or activity timeout";
    case RBRGEN4_EVENT_REGIMES_ENABLED_BUT_NOT_YET_IN_A_REGIME:
        return "regimes enabled, but not yet in a_regime";
    case RBRGEN4_EVENT_ENTERED_REGIME_1:
        return "entered regime 1";
    case RBRGEN4_EVENT_ENTERED_REGIME_2:
        return "entered regime 2";
    case RBRGEN4_EVENT_ENTERED_REGIME_3:
        return "entered regime 3";
    case RBRGEN4_EVENT_START_OF_REGIME_BIN:
        return "start of regime bin";
    case RBRGEN4_EVENT_BEGIN_PROFILING_UP_CAST:
        return "begin profiling 'up' cast";
    case RBRGEN4_EVENT_BEGIN_PROFILING_DOWN_CAST:
        return "begin profiling 'down' cast";
    case RBRGEN4_EVENT_END_OF_PROFILING_CAST:
        return "end of profiling cast";
    case RBRGEN4_EVENT_BATTERY_FAILED_SCHEDULE_FINISHED:
        return "battery failed, schedule finished";
    case RBRGEN4_EVENT_DIRECTIONAL_DEPENDENT_SAMPLING_BEGINNING_OF_FAST_SAMPLING_MODE:
        return "directional dependent sampling: beginning of fast sampling mode";
    case RBRGEN4_EVENT_DIRECTIONAL_DEPENDENT_SAMPLING_BEGINNING_OF_SLOW_SAMPLING_MODE:
        return "directional dependent sampling: beginning of slow sampling mode";
    case RBRGEN4_EVENT_ENERGY_USED_MARKER_INTERNAL_BATTERY:
        return "energy used marker, internal battery";
    case RBRGEN4_EVENT_ENERGY_USED_MARKER_EXTERNAL_POWER_SOURCE:
        return "energy used marker, external power source";
    case RBRGEN4_EVENT_DEVICE_CONTROL_ACTION_RESULT:
        return "device control action result";
    case RBRGEN4_EVENT_DEPLOYMENT_RESUMED:
        return "Paused deployment resumed by the resume command";
    case RBRGEN4_EVENT_DEPLOYMENT_PAUSED:
        return "Deployment paused using the pause command";
    case RBRGEN4_EVENT_REGIMES_PASSED_FINAL_BOUNDARY:
        return "Regimes; passed final boundary";
    }
}

RBRGen4Error RBRGen4Parser_init(RBRGen4Parser **parser,
                                  const RBRGen4ParserCallbacks *callbacks,
                                  const RBRGen4ParserConfig *config,
                                  void *userData)
{
    if (callbacks == NULL
        || (callbacks->sample != NULL && callbacks->sampleBuffer == NULL)
        || (callbacks->event != NULL && callbacks->eventBuffer == NULL))
    {
        return RBRGEN4_MISSING_CALLBACK;
    }

    if (config->channelCount <= 0
        || config->channelCount > RBRGEN4_CHANNEL_MAX)
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    memset(*parser, 0, sizeof(RBRGen4Parser));
    memcpy(&(*parser)->config, config, sizeof(RBRGen4ParserConfig));
    memcpy(&(*parser)->callbacks, callbacks, sizeof(RBRGen4ParserCallbacks));
    (*parser)->userData          = userData;

    return RBRGEN4_SUCCESS;
}

void RBRGen4Parser_getConfig(const RBRGen4Parser *parser, RBRGen4ParserConfig *config)
{
    memcpy(config, &parser->config, sizeof(RBRGen4ParserConfig));
}

void *RBRGen4Parser_getUserData(const RBRGen4Parser *parser)
{
    return parser->userData;
}

void RBRGen4Parser_setUserData(RBRGen4Parser *parser, void *userData)
{
    parser->userData = userData;
}

#define EP_EVENT_SIZE             16
#define EP_EVENT_CRC_OFFSET       0
#define EP_EVENT_TYPE_OFFSET      2
#define EP_EVENT_MARKER_OFFSET    3
#define EP_EVENT_TIMESTAMP_OFFSET 4
#define EP_EVENT_PAYLOAD_OFFSET   12

static RBRGen4Error RBRGen4Parser_parseEPEvents(
    RBRGen4Parser *parser,
    const uint8_t *const data,
    int32_t *size)
{
    int32_t maxSize = *size;
    *size = 0;

    RBRGen4Event *event = parser->callbacks.eventBuffer;
    if (event == NULL)
    {
        return RBRGEN4_SUCCESS;
    }

    for (; *size + EP_EVENT_SIZE <= maxSize; *size += EP_EVENT_SIZE)
    {
        memset(event, 0, sizeof(RBRGen4Event));

        event->type = *(uint8_t *) (data + *size + EP_EVENT_TYPE_OFFSET);
        event->timestamp =
            *(RBRGen4DateTime *) (data
                                        + *size
                                        + EP_EVENT_TIMESTAMP_OFFSET);
        switch (event->type)
        {
        case RBRGEN4_EVENT_START_OF_REGIME_BIN:
        case RBRGEN4_EVENT_BEGIN_PROFILING_UP_CAST:
        case RBRGEN4_EVENT_BEGIN_PROFILING_DOWN_CAST:
        case RBRGEN4_EVENT_END_OF_PROFILING_CAST:
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

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4Parser_parse(RBRGen4Parser *parser,
                                   RBRGen4Block block,
                                   const void *const data,
                                   int32_t *size)
{
    const uint8_t *d = (const uint8_t *const) data;

    (void)block;
    return RBRGen4Parser_parseEPEvents(parser, d, size);
}
