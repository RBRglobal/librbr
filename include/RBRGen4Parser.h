/**
 * \file RBRGen4Parser.h
 *
 * \brief Interface for parsing datasetPool produced by RBR instruments.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRGEN4PARSER_H
#define LIBRBR_RBRGEN4PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdbool.h>

#include "RBRGen4.h"

/**
 * \brief The number of bytes of auxiliary data in an event.
 */
#define RBRGEN4_EVENT_AUXILIARY_DATA_MAX 8

struct RBRGen4Parser;

/**
 * \brief Callback to provide a parsed sample to user code.
 *
 * The \a sample pointer will be the same as given via
 * RBRGen4ParserCallbacks.sampleBuffer. That buffer will be overwritten every time
 * a parsed sample is returned via this callback. If you want to retain the use
 * of the sample after your callback has returned, make a copy of it.
 *
 * \param [in] parser the dataset parser which parsed the sample
 * \param [in] sample the parsed sample
 * \return #RBRGEN4_SUCCESS when the sample data is successfully consumed
 * \return #RBRGEN4_CALLBACK_ERROR when an unrecoverable error occurs
 */
typedef RBRGen4Error (*RBRGen4ParserSampleCallback)(const struct RBRGen4Parser *parser,
                                                    const struct RBRGen4Sample *const sample);

/**
 * \brief Instrument event types.
 */
typedef enum RBRGen4EventType {
    RBRGEN4_EVENT_UNKNOWN_OR_UNRECOGNIZED_EVENT = 0x00,
    RBRGEN4_EVENT_TIME_SYNCHRONIZATION_MARKER = 0x01, /* Reserved in L3.5 and L4 */
    RBRGEN4_EVENT_DISABLE_COMMAND_RECEIVED = 0x02,
    RBRGEN4_EVENT_RUN_TIME_ERROR_ENCOUNTERED = 0x03,
    RBRGEN4_EVENT_CPU_RESET_DETECTED = 0x04,
    RBRGEN4_EVENT_ONE_OR_MORE_PARAMETERS_RECOVERED_AFTER_RESET = 0x05,
    RBRGEN4_EVENT_RESTART_FAILED_RTC_CALENDAR_CONTENTS_NOT_VALID = 0x06,
    RBRGEN4_EVENT_RESTART_FAILED_LOGGER_STATUS_NOT_VALID = 0x07,
    RBRGEN4_EVENT_RESTART_FAILED_PRIMARY_SCHEDULE_PARAMETERS_COULD_NOT_BE_RECOVERED = 0x08,
    RBRGEN4_EVENT_UNABLE_TO_LOAD_ALARM_TIME_FOR_NEXT_SAMPLE = 0x09,
    RBRGEN4_EVENT_SAMPLING_RESTARTED_AFTER_RESETTING_RTC = 0x0A,
    RBRGEN4_EVENT_PARAMETERS_RECOVERED_SAMPLING_RESTARTED_AFTER_RESETTING_RTC = 0x0B,
    RBRGEN4_EVENT_SAMPLING_STOPPED_END_TIME_REACHED = 0x0C,
    RBRGEN4_EVENT_START_OF_A_RECORDED_BURST = 0x0D, /* Reserved in L3.5 */
    RBRGEN4_EVENT_START_OF_A_WAVE_BURST = 0x0E,     /* Reserved in L3.5 */
    RBRGEN4_EVENT_POWER_SOURCE_SWITCHED_TO_USB = 0x0F,
    RBRGEN4_EVENT_STREAMING_NOW_OFF_FOR_BOTH_PORTS = 0x10, /* Reserved in L3.5 and L4, used in L3 */
    RBRGEN4_EVENT_STREAMING_ON_FOR_USB_OFF_FOR_SERIAL =
        0x11, /* Reserved in L3.5 and L4, used in L3 */
    RBRGEN4_EVENT_STREAMING_OFF_FOR_USB_ON_FOR_SERIAL =
        0x12,                                             /* Reserved in L3.5 and L4, used in L3 */
    RBRGEN4_EVENT_STREAMING_NOW_ON_FOR_BOTH_PORTS = 0x13, /* Reserved in L3.5 and L4, used in L3 */
    RBRGEN4_EVENT_SAMPLING_STARTED_THRESHOLD_CONDITION_SATISFIED = 0x14, /* Reserved in L3.5 */
    RBRGEN4_EVENT_SAMPLING_PAUSED_THRESHOLD_CONDITION_NOT_MET = 0x15,    /* Reserved in L3.5 */
    RBRGEN4_EVENT_POWER_SOURCE_SWITCHED_TO_INTERNAL_BATTERY = 0x16,
    RBRGEN4_EVENT_POWER_SOURCE_SWITCHED_TO_EXTERNAL_BATTERY = 0x17,
    RBRGEN4_EVENT_TWIST_ACTIVATION_STARTED_SAMPLING = 0x18,                   /* Reserved in L3.5 */
    RBRGEN4_EVENT_TWIST_ACTIVATION_PAUSED_SAMPLING = 0x19,                    /* Reserved in L3.5 */
    RBRGEN4_EVENT_WIFI_MODULE_DETECTED_AND_ACTIVATED = 0x1A,                  /* Reserved in L3.5 */
    RBRGEN4_EVENT_WIFI_MODULE_DEACTIVATED_REMOVED_OR_ACTIVITY_TIMEOUT = 0x1B, /* Reserved in L3.5 */
    RBRGEN4_EVENT_REGIMES_ENABLED_BUT_NOT_YET_IN_A_REGIME = 0x1C,
    RBRGEN4_EVENT_ENTERED_REGIME_1 = 0x1D,
    RBRGEN4_EVENT_ENTERED_REGIME_2 = 0x1E,
    RBRGEN4_EVENT_ENTERED_REGIME_3 = 0x1F,
    RBRGEN4_EVENT_START_OF_REGIME_BIN = 0x20,
    RBRGEN4_EVENT_BEGIN_PROFILING_UP_CAST = 0x21,   /* Reserved in L3.5 */
    RBRGEN4_EVENT_BEGIN_PROFILING_DOWN_CAST = 0x22, /* Reserved in L3.5 */
    RBRGEN4_EVENT_END_OF_PROFILING_CAST = 0x23,     /* Reserved in L3.5 */
    RBRGEN4_EVENT_BATTERY_FAILED_SCHEDULE_FINISHED = 0x24,
    RBRGEN4_EVENT_DIRECTIONAL_DEPENDENT_SAMPLING_BEGINNING_OF_FAST_SAMPLING_MODE =
        0x25, /* Reserved in L3.5 */
    RBRGEN4_EVENT_DIRECTIONAL_DEPENDENT_SAMPLING_BEGINNING_OF_SLOW_SAMPLING_MODE =
        0x26, /* Reserved in L3.5 */
    RBRGEN4_EVENT_ENERGY_USED_MARKER_INTERNAL_BATTERY =
        0x27, /* Reserved in L4 and L3.5, used in L3 */
    RBRGEN4_EVENT_ENERGY_USED_MARKER_EXTERNAL_POWER_SOURCE =
        0x28,                                          /* Reserved in L4 and L3.5, used in L3 */
    RBRGEN4_EVENT_DEVICE_CONTROL_ACTION_RESULT = 0x29, /* Reserved in L3.5 */
    RBRGEN4_EVENT_DEPLOYMENT_RESUMED = 0x2A,           /* Reserved in L3.5 */
    RBRGEN4_EVENT_DEPLOYMENT_PAUSED = 0x2B,            /* Reserved in L3.5 */
    RBRGEN4_EVENT_REGIMES_PASSED_FINAL_BOUNDARY = 0x2D /* Reserved in L4 */
} RBRGen4EventType;

/**
 * \brief Get a human-readable string name for an event type type.
 *
 * \param [in] type the event type
 * \return a string name for the event type
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4EventType_name(RBRGen4EventType type);

/**
 * \brief An instrument event.
 */
typedef struct RBRGen4Event {
    /** \brief The type of the event. */
    RBRGen4EventType type;

    /** \brief The schedule(s) that this event belongs to. */
    RBRGen4Schedule *schedules[16];

    /** \brief The timestamp of the event. */
    RBRGen4DateTime timestamp;

    /**
     * \brief The size of the complete event in bytes.
     */
    int32_t auxiliaryDataLength;

    /** \brief Auxiliary data for the event. */
    uint8_t auxiliaryData[RBRGEN4_EVENT_AUXILIARY_DATA_MAX];
} RBRGen4Event;

/**
 * \brief Callback to provide a parsed event to user code.
 *
 * The \a event pointer will be the same as given via
 * RBRGen4ParserCallbacks.eventBuffer. The event value will be overwritten every
 * time event parsing is attempted, which will be at least once per invocation
 * of RBRGen4Parser_parse() for dataset RBRGEN4_DATASET_EASYPARSE_EVENTS
 * where the buffer is large enough. If you want to use the event after your
 * callback has returned, make a copy of it.
 *
 * \param [in] parser the dataset parser which parsed the event
 * \param [in] event the parsed event
 * \return #RBRGEN4_SUCCESS when the event data is successfully consumed
 * \return #RBRGEN4_CALLBACK_ERROR when an unrecoverable error occurs
 */
typedef RBRGen4Error (*RBRGen4ParserEventCallback)(const struct RBRGen4Parser *parser,
                                                   const struct RBRGen4Event *const event);

/**
 * \brief A set of callbacks from parser to user code.
 *
 * All of the callback functions are optional; however, where any callback
 * function is provided, the corresponding buffer must also be provided, or
 * else RBRGen4Parser_init() will return #RBRGEN4_MISSING_CALLBACK.
 */
typedef struct RBRGen4ParserCallbacks {
    /**
     * \brief Called when a sample has been parsed.
     *
     * Requires that RBRGen4ParserCallbacks.sampleBuffer also be populated.
     */
    RBRGen4ParserSampleCallback sample;

    /**
     * \brief Where to put sample data for consumption by the sample callback.
     *
     * Required only when RBRGen4ParserCallbacks.sample is populated.
     */
    RBRGen4Sample *sampleBuffer;

    /**
     * \brief Called when an event has been parsed.
     *
     * Requires that RBRGen4ParserCallbacks.eventBuffer also be populated.
     */
    RBRGen4ParserEventCallback event;

    /**
     * \brief Where to put event data for consumption by the event callback.
     *
     * Required only when RBRGen4ParserCallbacks.event is populated.
     */
    RBRGen4Event *eventBuffer;
} RBRGen4ParserCallbacks;

/**
 * \brief Configuration for a RBRGen4Parser.
 */
typedef struct RBRGen4ParserConfig {
    /**
     * \brief The number of instrument channels in each sample.
     *
     * If the value is less than or equal to 0 or exceeds
     * #RBRGEN4_CHANNEL_MAX, then RBRGen4Parser_init() will return
     * #RBRGEN4_INVALID_PARAMETER_VALUE.
     */
    int32_t channelCount;

    /**
     * \brief The numeric format used to store data values in the memory for
     * all channels.
     *
     * For normal deployments this will be either float32 (IEEE single precision
     * floating point) or float64 (IEEE double precision floating point). This
     * setting is factory configured; most instruments will use float32, but an
     * instrument with very high precision sensors may use float64 to maintain
     * the necessary level of resolution.
     */
    RBRGen4DataType dataType;
} RBRGen4ParserConfig;

/**
 * \brief Parser context object.
 *
 * Users are strongly discouraged from accessing the fields of this structure
 * directly as layout and field availability maybe unstable from version to
 * version. Getter and setter functions are available for safely reading from
 * and writing to fields where necessary.
 *
 * \see RBRGen4Parser_init() to initialize a parser
 * \see RBRGen4Parser_destroy() to close a parser
 */
typedef struct RBRGen4Parser {
    /** \brief The parser configuration. */
    RBRGen4ParserConfig config;

    /** \brief The set of callbacks to be used by the parser. */
    RBRGen4ParserCallbacks callbacks;

    /** \brief Arbitrary user data; useful in callbacks. */
    void *userData;

} RBRGen4Parser;

/**
 * \brief Initialize a dataset parser.
 *
 * As with the \a conn argument to RBRGen4_open(), the caller provides the
 * RBRGen4Parser instance and it is initialized in place; the library never
 * allocates memory.
 *
 * As with the \a callbacks argument to RBRGen4_open(), the
 * \a config and \a callbacks structures will be copied into the RBRGen4Parser
 * structure and no references to them are retained.
 *
 * Currently, the only supported memory format is
 * RBRGEN4_MEMFORMAT_CALBIN00 (“EasyParse”). Requesting any other format
 * via RBRGen4ParserConfig will cause #RBRGEN4_UNSUPPORTED to be returned.
 *
 * Both callback functions are optional, but that probably isn't very useful:
 * after all, you won't receive any data that way. Still, the library won't
 * complain. If any buffer is not given for a callback function which _is_
 * given, or if \a callbacks itself is given as `NULL`, then
 * #RBRGEN4_MISSING_CALLBACK is returned and the parser instantiation
 * will not be completed.
 *
 * In the event of any return value other than #RBRGEN4_SUCCESS, no cleanup of
 * library resources is required. In the event of a successful result,
 * RBRGen4Parser_destroy() should be used to close the parser.
 *
 * \param [out] parser the context object to populate
 * \param [in] callbacks the set of callbacks to be used by the parser
 * \param [in] config the parser configuration
 * \param [in] userData arbitrary user data; useful in callbacks
 * \return #RBRGEN4_SUCCESS if the parser was instantiated successfully
 * \return #RBRGEN4_MISSING_CALLBACK if no callbacks were provided
 * \return #RBRGEN4_UNSUPPORTED if the memory format is unsupported
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE if the config is invalid
 * \see RBRGen4Parser_destroy()
 */
RBRGen4Error RBRGen4Parser_init(RBRGen4Parser *parser, const RBRGen4ParserCallbacks *callbacks,
                                const RBRGen4ParserConfig *config, void *userData);

/**
 * \brief Close the parser.
 *
 * Clears the parser state. Does not release the caller-provided instance
 * memory.
 *
 * \param [in,out] parser the dataset parser to close
 * \return #RBRGEN4_SUCCESS if the parser was closed successfully
 * \see RBRGen4Parser_init()
 */
RBRGen4Error RBRGen4Parser_destroy(RBRGen4Parser *parser);

/**
 * \brief Get the parser configuration.
 *
 * This function copies into \a config a copy of the parser configuration
 * reflective of the current state of the parser.
 *
 * Because the configuration is integral to parser behaviour, it may only be
 * configured during instantiation; there is no corresponding function to
 * reconfigure a parser.
 *
 * \param [in] parser the dataset parser
 * \param [out] config the parser configuration
 */
void RBRGen4Parser_getConfig(const RBRGen4Parser *parser, RBRGen4ParserConfig *config);

/**
 * \brief Get the pointer to arbitrary user data.
 *
 * Returns whatever arbitrary pointer the user has most recently provided,
 * either via RBRGen4Parser_init() or RBRGen4Parser_setUserData().
 *
 * \param [in] parser the dataset parser
 * \return the arbitrary user data pointer
 * \see RBRGen4Parser_setUserData()
 */
void *RBRGen4Parser_getUserData(const RBRGen4Parser *parser);

/**
 * \brief Change the arbitrary user data pointer.
 *
 * \param [in,out] parser the dataset parser
 * \param [in] userData the new user data
 * \see RBRGen4Parser_getUserData()
 */
void RBRGen4Parser_setUserData(RBRGen4Parser *parser, void *userData);

/**
 * \brief Parse a chunk of data.
 *
 * Parsed values will be returned via the RBRGen4ParserCallbacks provided to
 * RBRGen4Parser_init(). The value at \a size after completion of parsing indicates
 * how much of the \a data was parsed.
 *
 * \param [in] parser the dataset parser
 * \param [in] block the block from which the chunk originated
 * \param [in] data the data to be parsed
 * \param [in,out] size initially, the size of the data given by \a data; set
 *                                 by the callback to the number of bytes
 *                                 actually parsed
 * \return #RBRGEN4_SUCCESS when no parsing errors occur
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when an invalid dataset is
 *                                                given, or when the parser
 *                                                configuration is incomplete
 *                                                or invalid
 */
RBRGen4Error RBRGen4Parser_parse(RBRGen4Parser *parser, RBRGen4Block block, const void *const data,
                                 int32_t *size);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN4PARSER_H */
