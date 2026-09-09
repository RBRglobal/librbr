/**
 * \file RBRGen3Parser.h
 *
 * \brief Interface for parsing datasets produced by RBR instruments.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRGEN3PARSER_H
#define LIBRBR_RBRGEN3PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdbool.h>

#include "RBRGen3.h"

/** \brief The maximum number of pieces of auxiliary data in an event. */
#define RBRGEN3_EVENT_AUXILIARY_DATA_MAX 4

struct RBRGen3Parser;

/**
 * \brief Callback to provide a parsed sample to user code.
 *
 * The \a sample pointer will be the same as given via
 * RBRGen3ParserCallbacks.sampleBuffer. That buffer will be overwritten every time
 * a parsed sample is returned via this callback. If you want to retain the use
 * of the sample after your callback has returned, make a copy of it.
 *
 * \param [in] parser the dataset parser which parsed the sample
 * \param [in] sample the parsed sample
 * \return #RBRGEN3_SUCCESS when the sample data is successfully consumed
 * \return #RBRGEN3_CALLBACK_ERROR when an unrecoverable error occurs
 */
typedef RBRGen3Error (*RBRGen3ParserSampleCallback)(
    const struct RBRGen3Parser *parser,
    const struct RBRGen3Sample *const sample);

/**
 * \brief Instrument event types.
 */
typedef enum RBRGen3EventType
{
    RBRGEN3_EVENT_UNKNOWN_OR_UNRECOGNIZED_EVENT                                     = 0x00,
    RBRGEN3_EVENT_TIME_SYNCHRONIZATION_MARKER                                       = 0x01,
    RBRGEN3_EVENT_DISABLE_COMMAND_RECEIVED                                          = 0x02,
    RBRGEN3_EVENT_RUN_TIME_ERROR_ENCOUNTERED                                        = 0x03,
    RBRGEN3_EVENT_CPU_RESET_DETECTED                                                = 0x04,
    RBRGEN3_EVENT_ONE_OR_MORE_PARAMETERS_RECOVERED_AFTER_RESET                      = 0x05,
    RBRGEN3_EVENT_RESTART_FAILED_RTC_CALENDAR_CONTENTS_NOT_VALID                    = 0x06,
    RBRGEN3_EVENT_RESTART_FAILED_LOGGER_STATUS_NOT_VALID                            = 0x07,
    RBRGEN3_EVENT_RESTART_FAILED_PRIMARY_SCHEDULE_PARAMETERS_COULD_NOT_BE_RECOVERED = 0x08,
    RBRGEN3_EVENT_UNABLE_TO_LOAD_ALARM_TIME_FOR_NEXT_SAMPLE                         = 0x09,
    RBRGEN3_EVENT_SAMPLING_RESTARTED_AFTER_RESETTING_RTC                            = 0x0A,
    RBRGEN3_EVENT_PARAMETERS_RECOVERED_SAMPLING_RESTARTED_AFTER_RESETTING_RTC       = 0x0B,
    RBRGEN3_EVENT_SAMPLING_STOPPED_END_TIME_REACHED                                 = 0x0C,
    RBRGEN3_EVENT_START_OF_A_RECORDED_BURST                                         = 0x0D,
    RBRGEN3_EVENT_START_OF_A_WAVE_BURST                                             = 0x0E,
    RBRGEN3_EVENT_RESERVED1                                                         = 0x0F,
    RBRGEN3_EVENT_STREAMING_NOW_OFF_FOR_BOTH_PORTS                                  = 0x10,
    RBRGEN3_EVENT_STREAMING_ON_FOR_USB_OFF_FOR_SERIAL                               = 0x11,
    RBRGEN3_EVENT_STREAMING_OFF_FOR_USB_ON_FOR_SERIAL                               = 0x12,
    RBRGEN3_EVENT_STREAMING_NOW_ON_FOR_BOTH_PORTS                                   = 0x13,
    RBRGEN3_EVENT_SAMPLING_STARTED_THRESHOLD_CONDITION_SATISFIED                    = 0x14,
    RBRGEN3_EVENT_SAMPLING_PAUSED_THRESHOLD_CONDITION_NOT_MET                       = 0x15,
    RBRGEN3_EVENT_POWER_SOURCE_SWITCHED_TO_INTERNAL_BATTERY                         = 0x16,
    RBRGEN3_EVENT_POWER_SOURCE_SWITCHED_TO_EXTERNAL_BATTERY                         = 0x17,
    RBRGEN3_EVENT_TWIST_ACTIVATION_STARTED_SAMPLING                                 = 0x18,
    RBRGEN3_EVENT_TWIST_ACTIVATION_PAUSED_SAMPLING                                  = 0x19,
    RBRGEN3_EVENT_WIFI_MODULE_DETECTED_AND_ACTIVATED                                = 0x1A,
    RBRGEN3_EVENT_WIFI_MODULE_DEACTIVATED_REMOVED_OR_ACTIVITY_TIMEOUT               = 0x1B,
    RBRGEN3_EVENT_REGIMES_ENABLED_BUT_NOT_YET_IN_A_REGIME                           = 0x1C,
    RBRGEN3_EVENT_ENTERED_REGIME_1                                                  = 0x1D,
    RBRGEN3_EVENT_ENTERED_REGIME_2                                                  = 0x1E,
    RBRGEN3_EVENT_ENTERED_REGIME_3                                                  = 0x1F,
    RBRGEN3_EVENT_START_OF_REGIME_BIN                                               = 0x20,
    RBRGEN3_EVENT_BEGIN_PROFILING_UP_CAST                                           = 0x21,
    RBRGEN3_EVENT_BEGIN_PROFILING_DOWN_CAST                                         = 0x22,
    RBRGEN3_EVENT_END_OF_PROFILING_CAST                                             = 0x23,
    RBRGEN3_EVENT_BATTERY_FAILED_SCHEDULE_FINISHED                                  = 0x24,
    RBRGEN3_EVENT_DIRECTIONAL_DEPENDENT_SAMPLING_BEGINNING_OF_FAST_SAMPLING_MODE    = 0x25,
    RBRGEN3_EVENT_DIRECTIONAL_DEPENDENT_SAMPLING_BEGINNING_OF_SLOW_SAMPLING_MODE    = 0x26,
    RBRGEN3_EVENT_ENERGY_USED_MARKER_INTERNAL_BATTERY                               = 0x27,
    RBRGEN3_EVENT_ENERGY_USED_MARKER_EXTERNAL_POWER_SOURCE                          = 0x28
} RBRGen3EventType;

/**
 * \brief Get a human-readable string name for an event type type.
 *
 * \param [in] type the event type
 * \return a string name for the event type
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRGen3EventType_name(RBRGen3EventType type);

/**
 * \brief An instrument event.
 *
 * \see https://docs.rbr-global.com/L3commandreference/format-of-stored-data/standard-rawbin00-format/standard-format-events-markers
 * \see https://docs.rbr-global.com/L3commandreference/format-of-stored-data/easyparse-calbin00-format/easyparse-format-events-markers
 */
typedef struct RBRGen3Event
{
    /** \brief The type of the event. */
    RBRGen3EventType type;
    /** \brief The timestamp of the event. */
    RBRGen3DateTime timestamp;
    /**
     * \brief The number of populated entries in
     * RBRGen3Event.auxiliaryData.
     *
     * For EasyParse events, this will be either 0 or 1. For standard events,
     * this may be up to RBRGEN3_EVENT_AUXILIARY_DATA_MAX depending on
     * the event type.
     */
    int32_t auxiliaryDataLength;
    /** \brief Auxiliary data for the event. */
    uint32_t auxiliaryData[RBRGEN3_EVENT_AUXILIARY_DATA_MAX];
} RBRGen3Event;

/**
 * \brief Callback to provide a parsed event to user code.
 *
 * The \a event pointer will be the same as given via
 * RBRGen3ParserCallbacks.eventBuffer. The event value will be overwritten every
 * time event parsing is attempted, which will be at least once per invocation
 * of RBRGen3Parser_parse() for dataset #RBRGEN3_DATASET_EASYPARSE_EVENTS
 * where the buffer is large enough. If you want to use the event after your
 * callback has returned, make a copy of it.
 *
 * \param [in] parser the dataset parser which parsed the event
 * \param [in] event the parsed event
 * \return #RBRGEN3_SUCCESS when the event data is successfully consumed
 * \return #RBRGEN3_CALLBACK_ERROR when an unrecoverable error occurs
 */
typedef RBRGen3Error (*RBRGen3ParserEventCallback)(
    const struct RBRGen3Parser *parser,
    const struct RBRGen3Event *const event);

/**
 * \brief A set of callbacks from parser to user code.
 *
 * All of the callback functions are optional; however, where any callback
 * function is provided, the corresponding buffer must also be provided, or
 * else RBRGen3Parser_init() will return #RBRGEN3_MISSING_CALLBACK.
 */
typedef struct RBRGen3ParserCallbacks
{
    /**
     * \brief Called when a sample has been parsed.
     *
     * Requires that RBRGen3ParserCallbacks.sampleBuffer also be populated.
     */
    RBRGen3ParserSampleCallback sample;

    /**
     * \brief Where to put sample data for consumption by the sample callback.
     *
     * Required only when RBRGen3ParserCallbacks.sample is populated.
     */
    RBRGen3Sample *sampleBuffer;

    /**
     * \brief Called when an event has been parsed.
     *
     * Requires that RBRGen3ParserCallbacks.eventBuffer also be populated.
     */
    RBRGen3ParserEventCallback event;

    /**
     * \brief Where to put event data for consumption by the event callback.
     *
     * Required only when RBRGen3ParserCallbacks.event is populated.
     */
    RBRGen3Event *eventBuffer;
} RBRGen3ParserCallbacks;

/**
 * \brief EasyParse-specific parser configuration.
 *
 * \see RBRGen3ParserConfig
 */
typedef struct RBRGen3ParserEasyParseConfig
{
    /**
     * \brief The number of instrument channels in each sample.
     *
     * If the value is less than or equal to 0 or exceeds
     * #RBRGEN3_CHANNEL_MAX, then RBRGen3Parser_init() will return
     * #RBRGEN3_INVALID_PARAMETER_VALUE.
     */
    int32_t channels;
} RBRGen3ParserEasyParseConfig;

/**
 * \brief Configuration for a RBRGen3Parser.
 */
typedef struct RBRGen3ParserConfig
{
    /** \brief The format of memory being parsed. */
    RBRGen3MemoryFormat format;

    /** \brief Format-specific configuration. */
    union
    {
        /** \brief EasyParse-specific parser configuration. */
        struct RBRGen3ParserEasyParseConfig easyParse;
    } formatConfig;
} RBRGen3ParserConfig;

/**
 * \brief Parser context object.
 *
 * Users are strongly discouraged from accessing the fields of this structure
 * directly as layout and field availability maybe unstable from version to
 * version. Getter and setter functions are available for safely reading from
 * and writing to fields where necessary.
 *
 * \see RBRGen3Parser_init() to initialize a parser
 * \see RBRGen3Parser_destroy() to close a parser
 */
typedef struct RBRGen3Parser
{
    /** \brief The parser configuration. */
    RBRGen3ParserConfig config;

    /** \brief The set of callbacks to be used by the parser. */
    RBRGen3ParserCallbacks callbacks;

    /** \brief Arbitrary user data; useful in callbacks. */
    void *userData;

    /**
     * \brief Whether the instance memory was dynamically allocated by the
     * constructor.
     */
    bool managedAllocation;
} RBRGen3Parser;

/**
 * \brief Initialize a dataset parser.
 *
 * The use of the \a parser argument is the same as that of the \a instrument
 * argument to RBRGen3_open(): when given as `NULL`, instance memory will
 * be allocated for you; otherwise, the pointer target will be used as instance
 * storage. See RBRGen3_open() for “do”s and “don't”s inherent to this
 * approach.
 *
 * Again, as with the \a callbacks argument to RBRGen3_open(), the
 * \a config and \a callbacks structures will be copied into the RBRGen3Parser
 * structure and no references to them are retained.
 *
 * Currently, the only supported memory format is
 * #RBRGEN3_MEMFORMAT_CALBIN00 (“EasyParse”). Requesting any other format
 * via RBRGen3ParserConfig will cause #RBRGEN3_UNSUPPORTED to be returned.
 *
 * Both callback functions are optional, but that probably isn't very useful:
 * after all, you won't receive any data that way. Still, the library won't
 * complain. If any buffer is not given for a callback function which _is_
 * given, or if \a callbacks itself is given as `NULL`, then
 * #RBRGEN3_MISSING_CALLBACK is returned and the parser instantiation
 * will not be completed.
 *
 * In the event of any return value other than #RBRGEN3_SUCCESS, any
 * memory allocated by this constructor is freed. That is, in the event of
 * failure, no cleanup of library resources is required. In the event of a
 * successful result, RBRGen3Parser_destroy() should be used to release allocated
 * resources.
 *
 * \param [in,out] parser the context object to populate
 * \param [in] callbacks the set of callbacks to be used by the parser
 * \param [in] config the parser configuration
 * \param [in] userData arbitrary user data; useful in callbacks
 * \return #RBRGEN3_SUCCESS if the parser was instantiated successfully
 * \return #RBRGEN3_ALLOCATION_FAILURE if memory allocation failed
 * \return #RBRGEN3_MISSING_CALLBACK if no callbacks were provided
 * \return #RBRGEN3_UNSUPPORTED if the memory format is unsupported
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE if the config is invalid
 * \see RBRGen3Parser_destroy()
 */
RBRGen3Error RBRGen3Parser_init(RBRGen3Parser **parser,
                                  const RBRGen3ParserCallbacks *callbacks,
                                  const RBRGen3ParserConfig *config,
                                  void *userData);

/**
 * \brief Release any resources held by the parser.
 *
 * Frees the buffer allocated by RBRGen3Parser_init() if necessary.
 *
 * \param [in,out] parser the dataset parser to close
 * \return #RBRGEN3_SUCCESS if the parser was closed successfully
 * \see RBRGen3Parser_init()
 */
RBRGen3Error RBRGen3Parser_destroy(RBRGen3Parser *parser);

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
void RBRGen3Parser_getConfig(const RBRGen3Parser *parser, RBRGen3ParserConfig *config);

/**
 * \brief Get the pointer to arbitrary user data.
 *
 * Returns whatever arbitrary pointer the user has most recently provided,
 * either via RBRGen3Parser_init() or RBRGen3Parser_setUserData().
 *
 * \param [in] parser the dataset parser
 * \return the arbitrary user data pointer
 * \see RBRGen3Parser_setUserData()
 */
void *RBRGen3Parser_getUserData(const RBRGen3Parser *parser);

/**
 * \brief Change the arbitrary user data pointer.
 *
 * \param [in,out] parser the dataset parser
 * \param [in] userData the new user data
 * \see RBRGen3Parser_getUserData()
 */
void RBRGen3Parser_setUserData(RBRGen3Parser *parser, void *userData);

/**
 * \brief Parse a chunk of data.
 *
 * For a parser configured to parse #RBRGEN3_MEMFORMAT_CALBIN00-format
 * data, \a dataset may be given as #RBRGEN3_DATASET_EASYPARSE_EVENTS or
 * #RBRGEN3_DATASET_EASYPARSE_SAMPLE_DATA. Any other value (including
 * #RBRGEN3_DATASET_EASYPARSE_DEPLOYMENT_HEADER, which is currently
 * unsupported) will cause the function to return
 * #RBRGEN3_INVALID_PARAMETER_VALUE and no data will be parsed.
 *
 * Parsed values will be returned via the RBRGen3ParserCallbacks provided to
 * RBRGen3Parser_init(). The value at \a size after completion of parsing indicates
 * how much of the \a data was parsed.
 *
 * \param [in] parser the dataset parser
 * \param [in] dataset the dataset from which the chunk originated
 * \param [in] data the data to be parsed
 * \param [in,out] size initially, the size of the data given by \a data; set
 *                                 by the callback to the number of bytes
 *                                 actually parsed
 * \return #RBRGEN3_SUCCESS when no parsing errors occur
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when an invalid dataset is
 *                                                given, or when the parser
 *                                                configuration is incomplete
 *                                                or invalid
 */
RBRGen3Error RBRGen3Parser_parse(RBRGen3Parser *parser,
                                   RBRGen3Dataset dataset,
                                   const void *const data,
                                   int32_t *size);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN3PARSER_H */
