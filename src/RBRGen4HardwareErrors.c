/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file RBRGen4HardwareErrors.c
 *
 * \brief Library implementation.
 */

#include "RBRGen4.h"
#include "RBRGen4HardwareErrors.h"

const char *RBRGen4HardwareError_name(RBRGen4HardwareError error)
{
    switch (error) {
    case RBRGEN4_HARDWARE_ERROR_NONE:
        return "none";
    case RBRGEN4_HARDWARE_ERROR_COMMAND_PARSER_BUSY:
        return "command parser busy";
    case RBRGEN4_HARDWARE_ERROR_INVALID_COMMAND:
        return "invalid command";
    case RBRGEN4_HARDWARE_ERROR_FEATURE_NOT_YET_IMPLEMENTED:
        return "feature not yet implemented";
    case RBRGEN4_HARDWARE_ERROR_COMMAND_PROHIBITED_WHILE_LOGGING:
        return "command prohibited while logging";
    case RBRGEN4_HARDWARE_ERROR_COMMAND_PROHIBITED_DURING_POLLING_PERSISTENCE_WINDOW:
        return "command prohibited during polling persistence window";
    case RBRGEN4_HARDWARE_ERROR_EXPECTED_ARGUMENT_MISSING:
        return "expected argument missing";
    case RBRGEN4_HARDWARE_ERROR_INVALID_ARGUMENT_TO_COMMAND:
        return "invalid argument to command";
    case RBRGEN4_HARDWARE_ERROR_FEATURE_NOT_AVAILABLE:
        return "feature not available";
    case RBRGEN4_HARDWARE_ERROR_BUFFER_FULL:
        return "buffer full";
    case RBRGEN4_HARDWARE_ERROR_COMMAND_FAILED:
        return "command failed";
    case RBRGEN4_HARDWARE_ERROR_FEATURE_NOT_SUPPORTED_BY_HARDWARE:
        return "feature not supported by hardware";
    case RBRGEN4_HARDWARE_ERROR_SYNTAX_ERROR:
        return "syntax error";
    case RBRGEN4_HARDWARE_ERROR_PARSE_ERROR:
        return "parse error";
    case RBRGEN4_HARDWARE_ERROR_UNKNOWN_QUALIFIER:
        return "unknown qualifier";
    case RBRGEN4_HARDWARE_ERROR_INVALID_QUALIFIER:
        return "invalid qualifier";
    case RBRGEN4_HARDWARE_ERROR_MISSING_QUALIFIER:
        return "missing qualifier";
    case RBRGEN4_HARDWARE_ERROR_LABEL_ALREADY_IN_USE:
        return "label is already in use";
    case RBRGEN4_HARDWARE_ERROR_NO_QUALIFIED_OBJECTS:
        return "no qualified objects";
    case RBRGEN4_HARDWARE_ERROR_VALUE_IS_TOO_LONG:
        return "value is too long";
    case RBRGEN4_HARDWARE_ERROR_VALUE_IS_A_KEYWORD_AND_CANNOT_BE_USED_AS_A_VALUE:
        return "value is a keyword and cannot be used as a value";
    case RBRGEN4_HARDWARE_ERROR_VALUE_CANNOT_BE_SET_MULTIPLE_TIMES:
        return "value cannot be set multiple times";
    case RBRGEN4_HARDWARE_ERROR_PARAMETER_IS_READ_ONLY:
        return "parameter is read only";
    case RBRGEN4_HARDWARE_ERROR_ARGUMENTS_RESULT_IN_AMBIGUOUS_COMMAND:
        return "arguments result in ambiguous command";
    case RBRGEN4_HARDWARE_ERROR_NO_VALUE_HAS_BEEN_SET:
        return "no value has been set";
    case RBRGEN4_HARDWARE_ERROR_NO_MORE_ENTRIES_ALLOWED_IN_LIST:
        return "no more entries allowed in list";
    case RBRGEN4_HARDWARE_ERROR_LABEL_IS_A_KEYWORD_AND_CANNOT_BE_USED_AS_A_LABEL:
        return "label is a keyword and cannot be used as a label";
    case RBRGEN4_HARDWARE_ERROR_ILLEGAL_CHARACTER_IN_LABEL:
        return "illegal character in label";
    case RBRGEN4_HARDWARE_ERROR_MEMORY_ERASE_NOT_COMPLETED:
        return "memory erase not completed";
    case RBRGEN4_HARDWARE_ERROR_DOWNLOAD_TRACKING_FAILED:
        return "download tracking failed, specify all parameters";
    case RBRGEN4_HARDWARE_ERROR_STORAGE_ACCESS_MUST_BE_SET_TO_INSTRUMENT:
        return "storage access must be set to 'instrument' prior to being used";
    case RBRGEN4_HARDWARE_ERROR_DATASET_NOT_FOUND:
        return "dataset not found";
    case RBRGEN4_HARDWARE_ERROR_STORAGE_ACCESS_ALREADY_AT_SELECTED_LOCATION:
        return "storage access already at selected location";
    case RBRGEN4_HARDWARE_ERROR_EEPROM_MEMORY_ERROR:
        return "eeprom memory error";
    case RBRGEN4_HARDWARE_ERROR_STORAGE_MEDIA_ERROR:
        return "storage media error";
    case RBRGEN4_HARDWARE_ERROR_ESTIMATED_MEMORY_USAGE_EXCEEDS_CAPACITY:
        return "estimated memory usage exceeds capacity";
    case RBRGEN4_HARDWARE_ERROR_MEMORY_NOT_EMPTY_ERASE_FIRST:
        return "memory not empty erase first";
    case RBRGEN4_HARDWARE_ERROR_END_TIME_MUST_BE_AFTER_START_TIME:
        return "end time must be after start time";
    case RBRGEN4_HARDWARE_ERROR_END_TIME_MUST_BE_AFTER_CURRENT_TIME:
        return "end time must be after current time";
    case RBRGEN4_HARDWARE_ERROR_FAILED_TO_ENABLE_FOR_LOGGING:
        return "failed to enable for logging";
    case RBRGEN4_HARDWARE_ERROR_INSTRUMENT_WAS_ALREADY_ENABLED:
        return "instrument was already enabled";
    case RBRGEN4_HARDWARE_ERROR_NO_SAMPLING_CHANNELS_ACTIVE:
        return "no sampling channels active";
    case RBRGEN4_HARDWARE_ERROR_PERIOD_NOT_VALID_FOR_SELECTED_MODE:
        return "period not valid for selected mode";
    case RBRGEN4_HARDWARE_ERROR_BURST_PARAMETERS_INCONSISTENT:
        return "burst parameters inconsistent";
    case RBRGEN4_HARDWARE_ERROR_PERIOD_TOO_SHORT_FOR_SERIAL_STREAMING:
        return "period too short for serial streaming";
    case RBRGEN4_HARDWARE_ERROR_THRESHOLDING_INTERVAL_NOT_VALID:
        return "thresholding interval not valid";
    case RBRGEN4_HARDWARE_ERROR_MORE_THAN_ONE_GATING_CONDITION_IS_ENABLED:
        return "more than one gating condition is enabled";
    case RBRGEN4_HARDWARE_ERROR_WRONG_REGIMES_SETTING:
        return "wrong regimes setting";
    case RBRGEN4_HARDWARE_ERROR_NO_GATING_ALLOWED_WITH_REGIMES_MODE:
        return "no gating allowed with regimes mode";
    case RBRGEN4_HARDWARE_ERROR_CAST_DETECTION_NEEDS_A_PRESSURE_DEPTH_CHANNEL:
        return "cast detection needs a pressure/depth channel";
    case RBRGEN4_HARDWARE_ERROR_CALIBRATION_COEFFICIENTS_ARE_MISSING:
        return "calibration coefficients are missing";
    case RBRGEN4_HARDWARE_ERROR_REQUIRED_CHANNEL_IS_TURNED_OFF:
        return "required channel is turned off";
    case RBRGEN4_HARDWARE_ERROR_DDSAMPLING_MODE_NEEDS_PRESSURE_CHANNEL:
        return "ddsampling mode needs pressure channel";
    case RBRGEN4_HARDWARE_ERROR_WRONG_DDSAMPLING_SETTINGS:
        return "wrong ddsampling settings";
    case RBRGEN4_HARDWARE_ERROR_UNABLE_TO_ESTIMATE_MEMORY_USAGE:
        return "unable to estimate memory usage";
    case RBRGEN4_HARDWARE_ERROR_EMPTY_SCHEDULE_LIST_IN_CONFIGURATION:
        return "empty schedule list in configuration";
    case RBRGEN4_HARDWARE_ERROR_DATASET_LIMIT_REACHED:
        return "dataset limit reached, delete dataset(s) to make space";
    case RBRGEN4_HARDWARE_ERROR_NO_START_TIME_SPECIFIED_FOR_GATING_BY_TIME:
        return "no start time specified for gating by time";
    case RBRGEN4_HARDWARE_ERROR_STARTTIME_ACCESSIBLE_ONLY_IF_GATING_BY_TIME:
        return "starttime accessible only if gating by time";
    case RBRGEN4_HARDWARE_ERROR_INSTRUMENT_STATE_IS_ALREADY_DISABLED:
        return "instrument state is already disabled";
    case RBRGEN4_HARDWARE_ERROR_ALREADY_ENABLED_WITH_DIFFERENT_SETTINGS:
        return "instrument was already enabled with different settings";
    case RBRGEN4_HARDWARE_ERROR_INVALID_ARGUMENT_DATA_STORAGE_IS_NOT_AVAILABLE:
        return "invalid argument, data storage is not available";
    case RBRGEN4_HARDWARE_ERROR_CHANNELS_ON_SENSOR_SPLIT_ACROSS_SCHEDULES:
        return "channels on sensor split across schedules";
    case RBRGEN4_HARDWARE_ERROR_NO_SAMPLING_GROUPS_ACTIVE_IN_SCHEDULE:
        return "no sampling groups active in schedule";
    case RBRGEN4_HARDWARE_ERROR_INCOMPATIBLE_PERIODS_FOR_SCHEDULES_WITH_COMMON_CHANNELS:
        return "incompatible periods for schedules with common channels";
    case RBRGEN4_HARDWARE_ERROR_CONFIG_HAS_TOO_MANY_CHANNELS_SENSORS_FOR_GIVEN_SAMPLING_RATE:
        return "config has too many channels/sensors for given sampling rate(s)";
    case RBRGEN4_HARDWARE_ERROR_CONFIG_STREAMING_TOO_MUCH_DATA_FOR_CURRENT_BAUD_RATE:
        return "config streaming too much data for current baud rate";
    case RBRGEN4_HARDWARE_ERROR_CHANNEL_IS_MISCONFIGURED:
        return "channel is misconfigured";
    case RBRGEN4_HARDWARE_ERROR_PERIOD_TOO_SHORT_FOR_CHANNEL:
        return "period too short for channel";
    case RBRGEN4_HARDWARE_ERROR_ITEM_IS_NOT_CONFIGURED:
        return "item is not configured";
    case RBRGEN4_HARDWARE_ERROR_NO_CHANNELS_CONFIGURED:
        return "no channels configured";
    case RBRGEN4_HARDWARE_ERROR_SENSOR_HAS_NO_ROOT_CHANNELS:
        return "sensor has no root channels";
    case RBRGEN4_HARDWARE_ERROR_CHANNEL_DOES_NOT_BELONG_TO_A_PERIPHERAL:
        return "channel does not belong to a peripheral";
    case RBRGEN4_HARDWARE_ERROR_NO_CALIBRATION_FOR_CHANNEL:
        return "no calibration for channel";
    case RBRGEN4_HARDWARE_ERROR_DEVICE_ERROR:
        return "device error";
    case RBRGEN4_HARDWARE_ERROR_NO_DEVICES_CONFIGURED:
        return "no devices configured";
    case RBRGEN4_HARDWARE_ERROR_DEVICE_SCHEDULE_INCONSISTENT:
        return "device schedule inconsistent";
    case RBRGEN4_HARDWARE_ERROR_DEVICE_IS_NOT_ENABLED:
        return "device is not enabled";
    case RBRGEN4_HARDWARE_ERROR_MULTIPLE_OPERATIONS_NOT_SUPPORTED:
        return "multiple operations not supported";
    case RBRGEN4_HARDWARE_ERROR_DISCOVERY_INCOMPLETE:
        return "discovery incomplete";
    case RBRGEN4_HARDWARE_ERROR_UNSUPPORTED_INSTRUMENT:
        return "unsupported instrument";
    case RBRGEN4_HARDWARE_ERROR_UNSUPPORTED_FIRMWARE_VERSION:
        return "unsupported firmware version";
    case RBRGEN4_HARDWARE_ERROR_FAILED_TO_INITIALIZE_PERIPHERAL:
        return "failed to initialize peripheral";
    case RBRGEN4_HARDWARE_ERROR_PERIPHERAL_DOES_NOT_SUPPORT_PASSTHROUGH:
        return "peripheral does not support passthrough";
    case RBRGEN4_HARDWARE_ERROR_LEAVING_PASSTHROUGH:
        return "leaving passthrough";
    default:
        return "unknown hardware error";
    }
}
