/**
 * \file RBRParser.h
 *
 * \brief Interface for parsing datasets produced by RBR instruments.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRPARSER_H
#define LIBRBR_RBRPARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdbool.h>

#include "RBRInstrument.h"

/** \brief The maximum number of pieces of auxiliary data in an event. */
#define RBRINSTRUMENT_EVENT_AUXILIARY_DATA_MAX 4

struct RBRParser;

/**
 * \brief Callback to provide a parsed sample to user code.
 *
 * The \a sample pointer will be the same as given via
 * RBRParserCallbacks.sampleBuffer. That buffer will be overwritten every time
 * a parsed sample is returned via this callback. If you want to retain the use
 * of the sample after your callback has returned, make a copy of it.
 *
 * \param [in] parser the dataset parser which parsed the sample
 * \param [in] sample the parsed sample
 * \return #RBRINSTRUMENT_SUCCESS when the sample data is successfully consumed
 * \return #RBRINSTRUMENT_CALLBACK_ERROR when an unrecoverable error occurs
 */
typedef RBRInstrumentError (*RBRParserSampleCallback)(
    const struct RBRParser *parser, const struct RBRInstrumentSample *const sample);

/**
 * \brief Instrument event types.
 */
typedef enum RBRInstrumentEventType
{
    RBRINSTRUMENT_EVENT_UNKNOWN_OR_UNRECOGNIZED_EVENT = 0x00,
    RBRINSTRUMENT_EVENT_TIME_SYNCHRONIZATION_MARKER = 0x01,
    RBRINSTRUMENT_EVENT_DISABLE_COMMAND_RECEIVED = 0x02,
    RBRINSTRUMENT_EVENT_RUN_TIME_ERROR_ENCOUNTERED = 0x03,
    RBRINSTRUMENT_EVENT_CPU_RESET_DETECTED = 0x04,
    RBRINSTRUMENT_EVENT_ONE_OR_MORE_PARAMETERS_RECOVERED_AFTER_RESET = 0x05,
    RBRINSTRUMENT_EVENT_RESTART_FAILED_RTC_CALENDAR_CONTENTS_NOT_VALID = 0x06,
    RBRINSTRUMENT_EVENT_RESTART_FAILED_LOGGER_STATUS_NOT_VALID = 0x07,
    RBRINSTRUMENT_EVENT_RESTART_FAILED_PRIMARY_SCHEDULE_PARAMETERS_COULD_NOT_BE_RECOVERED = 0x08,
    RBRINSTRUMENT_EVENT_UNABLE_TO_LOAD_ALARM_TIME_FOR_NEXT_SAMPLE = 0x09,
    RBRINSTRUMENT_EVENT_SAMPLING_RESTARTED_AFTER_RESETTING_RTC = 0x0A,
    RBRINSTRUMENT_EVENT_PARAMETERS_RECOVERED_SAMPLING_RESTARTED_AFTER_RESETTING_RTC = 0x0B,
    RBRINSTRUMENT_EVENT_SAMPLING_STOPPED_END_TIME_REACHED = 0x0C,
    RBRINSTRUMENT_EVENT_START_OF_A_RECORDED_BURST = 0x0D,
    RBRINSTRUMENT_EVENT_START_OF_A_WAVE_BURST = 0x0E,
    RBRINSTRUMENT_EVENT_RESERVED1 = 0x0F,
    RBRINSTRUMENT_EVENT_STREAMING_NOW_OFF_FOR_BOTH_PORTS = 0x10,
    RBRINSTRUMENT_EVENT_STREAMING_ON_FOR_USB_OFF_FOR_SERIAL = 0x11,
    RBRINSTRUMENT_EVENT_STREAMING_OFF_FOR_USB_ON_FOR_SERIAL = 0x12,
    RBRINSTRUMENT_EVENT_STREAMING_NOW_ON_FOR_BOTH_PORTS = 0x13,
    RBRINSTRUMENT_EVENT_SAMPLING_STARTED_THRESHOLD_CONDITION_SATISFIED = 0x14,
    RBRINSTRUMENT_EVENT_SAMPLING_PAUSED_THRESHOLD_CONDITION_NOT_MET = 0x15,
    RBRINSTRUMENT_EVENT_POWER_SOURCE_SWITCHED_TO_INTERNAL_BATTERY = 0x16,
    RBRINSTRUMENT_EVENT_POWER_SOURCE_SWITCHED_TO_EXTERNAL_BATTERY = 0x17,
    RBRINSTRUMENT_EVENT_TWIST_ACTIVATION_STARTED_SAMPLING = 0x18,
    RBRINSTRUMENT_EVENT_TWIST_ACTIVATION_PAUSED_SAMPLING = 0x19,
    RBRINSTRUMENT_EVENT_WIFI_MODULE_DETECTED_AND_ACTIVATED = 0x1A,
    RBRINSTRUMENT_EVENT_WIFI_MODULE_DEACTIVATED_REMOVED_OR_ACTIVITY_TIMEOUT = 0x1B,
    RBRINSTRUMENT_EVENT_REGIMES_ENABLED_BUT_NOT_YET_IN_A_REGIME = 0x1C,
    RBRINSTRUMENT_EVENT_ENTERED_REGIME_1 = 0x1D,
    RBRINSTRUMENT_EVENT_ENTERED_REGIME_2 = 0x1E,
    RBRINSTRUMENT_EVENT_ENTERED_REGIME_3 = 0x1F,
    RBRINSTRUMENT_EVENT_START_OF_REGIME_BIN = 0x20,
    RBRINSTRUMENT_EVENT_BEGIN_PROFILING_UP_CAST = 0x21,
    RBRINSTRUMENT_EVENT_BEGIN_PROFILING_DOWN_CAST = 0x22,
    RBRINSTRUMENT_EVENT_END_OF_PROFILING_CAST = 0x23,
    RBRINSTRUMENT_EVENT_BATTERY_FAILED_SCHEDULE_FINISHED = 0x24,
    RBRINSTRUMENT_EVENT_DIRECTIONAL_DEPENDENT_SAMPLING_BEGINNING_OF_FAST_SAMPLING_MODE = 0x25,
    RBRINSTRUMENT_EVENT_DIRECTIONAL_DEPENDENT_SAMPLING_BEGINNING_OF_SLOW_SAMPLING_MODE = 0x26,
    RBRINSTRUMENT_EVENT_ENERGY_USED_MARKER_INTERNAL_BATTERY = 0x27,
    RBRINSTRUMENT_EVENT_ENERGY_USED_MARKER_EXTERNAL_POWER_SOURCE = 0x28
} RBRInstrumentEventType;

/**
 * \brief Get a human-readable string name for an event type type.
 *
 * \param [in] type the event type
 * \return a string name for the event type
 * \see RBRInstrumentError_name() for a description of the format of names
 */
const char *RBRInstrumentEventType_name(RBRInstrumentEventType type);

/**
 * \brief An instrument event.
 *
 * \see
 * https://docs.rbr-global.com/L3commandreference/format-of-stored-data/standard-rawbin00-format/standard-format-events-markers
 * \see
 * https://docs.rbr-global.com/L3commandreference/format-of-stored-data/easyparse-calbin00-format/easyparse-format-events-markers
 */
typedef struct RBRInstrumentEvent
{
    /** \brief The type of the event. */
    RBRInstrumentEventType type;
    /** \brief The timestamp of the event. */
    RBRInstrumentDateTime timestamp;
    /**
     * \brief The number of populated entries in
     * RBRInstrumentEvent.auxiliaryData.
     *
     * For EasyParse events, this will be either 0 or 1. For standard events,
     * this may be up to RBRINSTRUMENT_EVENT_AUXILIARY_DATA_MAX depending on
     * the event type.
     */
    int32_t auxiliaryDataLength;
    /** \brief Auxiliary data for the event. */
    uint32_t auxiliaryData[RBRINSTRUMENT_EVENT_AUXILIARY_DATA_MAX];
} RBRInstrumentEvent;

/**
 * \brief Callback to provide a parsed event to user code.
 *
 * The \a event pointer will be the same as given via
 * RBRParserCallbacks.eventBuffer. The event value will be overwritten every
 * time event parsing is attempted, which will be at least once per invocation
 * of RBRParser_parse() for dataset #RBRINSTRUMENT_DATASET_EASYPARSE_EVENTS
 * where the buffer is large enough. If you want to use the event after your
 * callback has returned, make a copy of it.
 *
 * \param [in] parser the dataset parser which parsed the event
 * \param [in] event the parsed event
 * \return #RBRINSTRUMENT_SUCCESS when the event data is successfully consumed
 * \return #RBRINSTRUMENT_CALLBACK_ERROR when an unrecoverable error occurs
 */
typedef RBRInstrumentError (*RBRParserEventCallback)(const struct RBRParser *parser,
                                                     const struct RBRInstrumentEvent *const event);

/**
 * \brief A set of callbacks from parser to user code.
 *
 * All of the callback functions are optional; however, where any callback
 * function is provided, the corresponding buffer must also be provided, or
 * else RBRParser_init() will return #RBRINSTRUMENT_MISSING_CALLBACK.
 */
typedef struct RBRParserCallbacks
{
    /**
     * \brief Called when a sample has been parsed.
     *
     * Requires that RBRParserCallbacks.sampleBuffer also be populated.
     */
    RBRParserSampleCallback sample;

    /**
     * \brief Where to put sample data for consumption by the sample callback.
     *
     * Required only when RBRParserCallbacks.sample is populated.
     */
    RBRInstrumentSample *sampleBuffer;

    /**
     * \brief Called when an event has been parsed.
     *
     * Requires that RBRParserCallbacks.eventBuffer also be populated.
     */
    RBRParserEventCallback event;

    /**
     * \brief Where to put event data for consumption by the event callback.
     *
     * Required only when RBRParserCallbacks.event is populated.
     */
    RBRInstrumentEvent *eventBuffer;
} RBRParserCallbacks;

/**
 * \brief EasyParse-specific parser configuration.
 *
 * \see RBRParserConfig
 */
typedef struct RBRParserEasyParseConfig
{
    /**
     * \brief The number of instrument channels in each sample.
     *
     * If the value is less than or equal to 0 or exceeds
     * #RBRINSTRUMENT_CHANNEL_MAX, then RBRParser_init() will return
     * #RBRINSTRUMENT_INVALID_PARAMETER_VALUE.
     */
    int32_t channels;
} RBRParserEasyParseConfig;

/**
 * \brief Configuration for a RBRParser.
 */
typedef struct RBRParserConfig
{
    /** \brief The format of memory being parsed. */
    RBRInstrumentMemoryFormat format;

    /** \brief Format-specific configuration. */
    union
    {
        /** \brief EasyParse-specific parser configuration. */
        struct RBRParserEasyParseConfig easyParse;
    } formatConfig;
} RBRParserConfig;

/**
 * \brief Parser context object.
 *
 * Users are strongly discouraged from accessing the fields of this structure
 * directly as layout and field availability maybe unstable from version to
 * version. Getter and setter functions are available for safely reading from
 * and writing to fields where necessary.
 *
 * \see RBRParser_init() to initialize a parser
 * \see RBRParser_destroy() to close a parser
 */
typedef struct RBRParser
{
    /** \brief The parser configuration. */
    RBRParserConfig config;

    /** \brief The set of callbacks to be used by the parser. */
    RBRParserCallbacks callbacks;

    /** \brief Arbitrary user data; useful in callbacks. */
    void *userData;

    /**
     * \brief Whether the instance memory was dynamically allocated by the
     * constructor.
     */
    bool managedAllocation;
} RBRParser;

/**
 * \brief Initialize a dataset parser.
 *
 * The use of the \a parser argument is the same as that of the \a instrument
 * argument to RBRInstrument_open(): when given as `NULL`, instance memory will
 * be allocated for you; otherwise, the pointer target will be used as instance
 * storage. See RBRInstrument_open() for “do”s and “don't”s inherent to this
 * approach.
 *
 * Again, as with the \a callbacks argument to RBRInstrument_open(), the
 * \a config and \a callbacks structures will be copied into the RBRParser
 * structure and no references to them are retained.
 *
 * Currently, the only supported memory format is
 * #RBRINSTRUMENT_MEMFORMAT_CALBIN00 (“EasyParse”). Requesting any other format
 * via RBRParserConfig will cause #RBRINSTRUMENT_UNSUPPORTED to be returned.
 *
 * Both callback functions are optional, but that probably isn't very useful:
 * after all, you won't receive any data that way. Still, the library won't
 * complain. If any buffer is not given for a callback function which _is_
 * given, or if \a callbacks itself is given as `NULL`, then
 * #RBRINSTRUMENT_MISSING_CALLBACK is returned and the parser instantiation
 * will not be completed.
 *
 * In the event of any return value other than #RBRINSTRUMENT_SUCCESS, any
 * memory allocated by this constructor is freed. That is, in the event of
 * failure, no cleanup of library resources is required. In the event of a
 * successful result, RBRParser_destroy() should be used to release allocated
 * resources.
 *
 * \param [in,out] parser the context object to populate
 * \param [in] callbacks the set of callbacks to be used by the parser
 * \param [in] config the parser configuration
 * \param [in] userData arbitrary user data; useful in callbacks
 * \return #RBRINSTRUMENT_SUCCESS if the parser was instantiated successfully
 * \return #RBRINSTRUMENT_ALLOCATION_FAILURE if memory allocation failed
 * \return #RBRINSTRUMENT_MISSING_CALLBACK if no callbacks were provided
 * \return #RBRINSTRUMENT_UNSUPPORTED if the memory format is unsupported
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE if the config is invalid
 * \see RBRParser_destroy()
 */
RBRInstrumentError RBRParser_init(RBRParser **parser, const RBRParserCallbacks *callbacks,
                                  const RBRParserConfig *config, void *userData);

/**
 * \brief Release any resources held by the parser.
 *
 * Frees the buffer allocated by RBRParser_init() if necessary.
 *
 * \param [in,out] parser the dataset parser to close
 * \return #RBRINSTRUMENT_SUCCESS if the parser was closed successfully
 * \see RBRParser_init()
 */
RBRInstrumentError RBRParser_destroy(RBRParser *parser);

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
void RBRParser_getConfig(const RBRParser *parser, RBRParserConfig *config);

/**
 * \brief Get the pointer to arbitrary user data.
 *
 * Returns whatever arbitrary pointer the user has most recently provided,
 * either via RBRParser_init() or RBRParser_setUserData().
 *
 * \param [in] parser the dataset parser
 * \return the arbitrary user data pointer
 * \see RBRParser_setUserData()
 */
void *RBRParser_getUserData(const RBRParser *parser);

/**
 * \brief Change the arbitrary user data pointer.
 *
 * \param [in,out] parser the dataset parser
 * \param [in] userData the new user data
 * \see RBRParser_getUserData()
 */
void RBRParser_setUserData(RBRParser *parser, void *userData);

/**
 * \brief Parse a chunk of data.
 *
 * For a parser configured to parse #RBRINSTRUMENT_MEMFORMAT_CALBIN00-format
 * data, \a dataset may be given as #RBRINSTRUMENT_DATASET_EASYPARSE_EVENTS or
 * #RBRINSTRUMENT_DATASET_EASYPARSE_SAMPLE_DATA. Any other value (including
 * #RBRINSTRUMENT_DATASET_EASYPARSE_DEPLOYMENT_HEADER, which is currently
 * unsupported) will cause the function to return
 * #RBRINSTRUMENT_INVALID_PARAMETER_VALUE and no data will be parsed.
 *
 * Parsed values will be returned via the RBRParserCallbacks provided to
 * RBRParser_init(). The value at \a size after completion of parsing indicates
 * how much of the \a data was parsed.
 *
 * \param [in] parser the dataset parser
 * \param [in] dataset the dataset from which the chunk originated
 * \param [in] data the data to be parsed
 * \param [in,out] size initially, the size of the data given by \a data; set
 *                                 by the callback to the number of bytes
 *                                 actually parsed
 * \return #RBRINSTRUMENT_SUCCESS when no parsing errors occur
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when an invalid dataset is
 *                                                given, or when the parser
 *                                                configuration is incomplete
 *                                                or invalid
 */
RBRInstrumentError RBRParser_parse(RBRParser *parser, RBRInstrumentDataset dataset,
                                   const void *const data, int32_t *size);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRPARSER_H */
