/**
 * \file RBRGen3HardwareErrors.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include "RBRGen3.h"

const char *RBRGen3HardwareError_name(RBRGen3HardwareError error)
{
    switch (error)
    {
    case RBRGEN3_HARDWARE_ERROR_NONE:
        return "none";
    case RBRGEN3_HARDWARE_ERROR_COMMAND_PARSER_BUSY:
        return "command parser busy";
    case RBRGEN3_HARDWARE_ERROR_INVALID_COMMAND:
        return "invalid command";
    case RBRGEN3_HARDWARE_ERROR_PROTECTED_COMMAND:
        return "protected command";
    case RBRGEN3_HARDWARE_ERROR_FEATURE_NOT_YET_IMPLEMENTED:
        return "feature not yet implemented";
    case RBRGEN3_HARDWARE_ERROR_COMMAND_PROHIBITED_WHILE_LOGGING:
        return "command prohibited while logging";
    case RBRGEN3_HARDWARE_ERROR_EXPECTED_ARGUMENT_MISSING:
        return "expected argument missing";
    case RBRGEN3_HARDWARE_ERROR_INVALID_ARGUMENT_TO_COMMAND:
        return "invalid argument to command";
    case RBRGEN3_HARDWARE_ERROR_FEATURE_NOT_AVAILABLE:
        return "feature not available";
    case RBRGEN3_HARDWARE_ERROR_BUFFER_FULL:
        return "buffer full";
    case RBRGEN3_HARDWARE_ERROR_COMMAND_FAILED:
        return "command failed";
    case RBRGEN3_HARDWARE_ERROR_EXPECTED_DATA_MISSING:
        return "expected data missing";
    case RBRGEN3_HARDWARE_ERROR_INVALID_DATA:
        return "invalid data";
    case RBRGEN3_HARDWARE_ERROR_FEATURE_NOT_SUPPORTED_BY_HARDWARE:
        return "feature not supported by hardware";
    case RBRGEN3_HARDWARE_ERROR_UNKNOWN_ERROR3:
    case RBRGEN3_HARDWARE_ERROR_UNKNOWN_ERROR4:
    case RBRGEN3_HARDWARE_ERROR_UNKNOWN_ERROR5:
    case RBRGEN3_HARDWARE_ERROR_UNKNOWN_ERROR6:
        return "unknown error";
    case RBRGEN3_HARDWARE_ERROR_MEMORY_ERASE_NOT_COMPLETED:
        return "memory erase not completed";
    case RBRGEN3_HARDWARE_ERROR_ESTIMATED_MEMORY_USAGE_EXCEEDS_CAPACITY:
        return "estimated memory usage exceeds capacity";
    case RBRGEN3_HARDWARE_ERROR_MEMORY_NOT_EMPTY_ERASE_FIRST:
        return "memory not empty erase first";
    case RBRGEN3_HARDWARE_ERROR_END_TIME_MUST_BE_AFTER_START_TIME:
        return "end time must be after start time";
    case RBRGEN3_HARDWARE_ERROR_END_TIME_MUST_BE_AFTER_CURRENT_TIME:
        return "end time must be after current time";
    case RBRGEN3_HARDWARE_ERROR_FAILED_TO_ENABLE_FOR_LOGGING:
        return "failed to enable for logging";
    case RBRGEN3_HARDWARE_ERROR_NOT_LOGGING:
        return "not logging";
    case RBRGEN3_HARDWARE_ERROR_LOGGING_ALREADY_ACTIVE:
        return "logging already active";
    case RBRGEN3_HARDWARE_ERROR_UNCLEARED_ERROR_USE_ERRORLOG:
        return "uncleared error, use 'errorlog'";
    case RBRGEN3_HARDWARE_ERROR_NO_SAMPLING_CHANNELS_ACTIVE:
        return "no sampling channels active";
    case RBRGEN3_HARDWARE_ERROR_PERIOD_NOT_VALID_FOR_SELECTED_MODE:
        return "period not valid for selected mode";
    case RBRGEN3_HARDWARE_ERROR_BURST_PARAMETERS_INCONSISTENT:
        return "burst parameters inconsistent";
    case RBRGEN3_HARDWARE_ERROR_PERIOD_TOO_SHORT_FOR_SERIAL_STREAMING:
        return "period too short for serial streaming";
    case RBRGEN3_HARDWARE_ERROR_THRESHOLDING_INTERVAL_NOT_VALID:
        return "thresholding interval not valid";
    case RBRGEN3_HARDWARE_ERROR_MORE_THAN_ONE_GATING_CONDITION_IS_ENABLED:
        return "more than one gating condition is enabled";
    case RBRGEN3_HARDWARE_ERROR_WRONG_REGIMES_SETTING:
        return "wrong regimes setting";
    case RBRGEN3_HARDWARE_ERROR_NO_GATING_ALLOWED_WITH_REGIMES_MODE:
        return "no gating allowed with regimes mode";
    case RBRGEN3_HARDWARE_ERROR_CAST_DETECTION_NEEDS_A_PRESSURE_DEPTH_CHANNEL:
        return "cast detection needs a pressure/depth channel";
    case RBRGEN3_HARDWARE_ERROR_CALIBRATION_COEFFICIENTS_ARE_MISSING:
        return "calibration coefficients are missing";
    case RBRGEN3_HARDWARE_ERROR_REQUIRED_CHANNEL_IS_TURNED_OFF:
        return "required channel is turned off";
    case RBRGEN3_HARDWARE_ERROR_RAW_OUTPUT_FORMAT_NOT_ALLOWED:
        return "raw output format not allowed";
    case RBRGEN3_HARDWARE_ERROR_AUX1_NOT_AVAILABLE_IN_CURRENT_SERIAL_MODE:
        return "AUX1 not available in current serial mode";
    case RBRGEN3_HARDWARE_ERROR_WRONG_DDSAMPLING_SETTINGS:
        return "wrong ddsampling settings";
    case RBRGEN3_HARDWARE_ERROR_ITEM_IS_NOT_CONFIGURED:
        return "item is not configured";
    case RBRGEN3_HARDWARE_ERROR_CONFIGURATION_FAILED:
        return "configuration failed";
    case RBRGEN3_HARDWARE_ERROR_ALL_AVAILABLE_CHANNELS_ASSIGNED:
        return "all available channels assigned";
    case RBRGEN3_HARDWARE_ERROR_ADDRESS_ALREADY_IN_USE:
        return "address already in use";
    case RBRGEN3_HARDWARE_ERROR_NO_CHANNELS_CONFIGURED:
        return "no channels configured";
    case RBRGEN3_HARDWARE_ERROR_CAN_NOT_CREATE_CALIBRATION_ENTRY:
        return "can not create calibration entry";
    case RBRGEN3_HARDWARE_ERROR_CAN_NOT_DELETE_CALIBRATION_ENTRY:
        return "can not delete calibration entry";
    case RBRGEN3_HARDWARE_ERROR_NO_CALIBRATION_FOR_CHANNEL:
        return "no calibration for channel";
    default:
        return "unknown hardware error";
    }
}
