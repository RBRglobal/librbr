/**
 * \file RBRParserGen3.h
 *
 * \brief Interface for parsing datasets produced by RBR instruments.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRPARSERGEN3_H
#define LIBRBR_RBRPARSERGEN3_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdbool.h>

#include "RBRInstrumentGen3.h"

/** \brief The maximum number of pieces of auxiliary data in an event. */
#define RBRINSTRUMENTGEN3_EVENT_AUXILIARY_DATA_MAX 4

struct RBRParserGen3;

/**
 * \brief Callback to provide a parsed sample to user code.
 *
 * The \a sample pointer will be the same as given via
 * RBRParserGen3Callbacks.sampleBuffer. That buffer will be overwritten every time
 * a parsed sample is returned via this callback. If you want to retain the use
 * of the sample after your callback has returned, make a copy of it.
 *
 * \param [in] parser the dataset parser which parsed the sample
 * \param [in] sample the parsed sample
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the sample data is successfully consumed
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR when an unrecoverable error occurs
 */
typedef RBRInstrumentGen3Error (*RBRParserGen3SampleCallback)(
    const struct RBRParserGen3 *parser,
    const struct RBRInstrumentGen3Sample *const sample);

/**
 * \brief Instrument event types.
 */
typedef enum RBRInstrumentGen3EventType
{
    RBRINSTRUMENTGEN3_EVENT_UNKNOWN_OR_UNRECOGNIZED_EVENT                                     = 0x00,
    RBRINSTRUMENTGEN3_EVENT_TIME_SYNCHRONIZATION_MARKER                                       = 0x01,
    RBRINSTRUMENTGEN3_EVENT_DISABLE_COMMAND_RECEIVED                                          = 0x02,
    RBRINSTRUMENTGEN3_EVENT_RUN_TIME_ERROR_ENCOUNTERED                                        = 0x03,
    RBRINSTRUMENTGEN3_EVENT_CPU_RESET_DETECTED                                                = 0x04,
    RBRINSTRUMENTGEN3_EVENT_ONE_OR_MORE_PARAMETERS_RECOVERED_AFTER_RESET                      = 0x05,
    RBRINSTRUMENTGEN3_EVENT_RESTART_FAILED_RTC_CALENDAR_CONTENTS_NOT_VALID                    = 0x06,
    RBRINSTRUMENTGEN3_EVENT_RESTART_FAILED_LOGGER_STATUS_NOT_VALID                            = 0x07,
    RBRINSTRUMENTGEN3_EVENT_RESTART_FAILED_PRIMARY_SCHEDULE_PARAMETERS_COULD_NOT_BE_RECOVERED = 0x08,
    RBRINSTRUMENTGEN3_EVENT_UNABLE_TO_LOAD_ALARM_TIME_FOR_NEXT_SAMPLE                         = 0x09,
    RBRINSTRUMENTGEN3_EVENT_SAMPLING_RESTARTED_AFTER_RESETTING_RTC                            = 0x0A,
    RBRINSTRUMENTGEN3_EVENT_PARAMETERS_RECOVERED_SAMPLING_RESTARTED_AFTER_RESETTING_RTC       = 0x0B,
    RBRINSTRUMENTGEN3_EVENT_SAMPLING_STOPPED_END_TIME_REACHED                                 = 0x0C,
    RBRINSTRUMENTGEN3_EVENT_START_OF_A_RECORDED_BURST                                         = 0x0D,
    RBRINSTRUMENTGEN3_EVENT_START_OF_A_WAVE_BURST                                             = 0x0E,
    RBRINSTRUMENTGEN3_EVENT_RESERVED1                                                         = 0x0F,
    RBRINSTRUMENTGEN3_EVENT_STREAMING_NOW_OFF_FOR_BOTH_PORTS                                  = 0x10,
    RBRINSTRUMENTGEN3_EVENT_STREAMING_ON_FOR_USB_OFF_FOR_SERIAL                               = 0x11,
    RBRINSTRUMENTGEN3_EVENT_STREAMING_OFF_FOR_USB_ON_FOR_SERIAL                               = 0x12,
    RBRINSTRUMENTGEN3_EVENT_STREAMING_NOW_ON_FOR_BOTH_PORTS                                   = 0x13,
    RBRINSTRUMENTGEN3_EVENT_SAMPLING_STARTED_THRESHOLD_CONDITION_SATISFIED                    = 0x14,
    RBRINSTRUMENTGEN3_EVENT_SAMPLING_PAUSED_THRESHOLD_CONDITION_NOT_MET                       = 0x15,
    RBRINSTRUMENTGEN3_EVENT_POWER_SOURCE_SWITCHED_TO_INTERNAL_BATTERY                         = 0x16,
    RBRINSTRUMENTGEN3_EVENT_POWER_SOURCE_SWITCHED_TO_EXTERNAL_BATTERY                         = 0x17,
    RBRINSTRUMENTGEN3_EVENT_TWIST_ACTIVATION_STARTED_SAMPLING                                 = 0x18,
    RBRINSTRUMENTGEN3_EVENT_TWIST_ACTIVATION_PAUSED_SAMPLING                                  = 0x19,
    RBRINSTRUMENTGEN3_EVENT_WIFI_MODULE_DETECTED_AND_ACTIVATED                                = 0x1A,
    RBRINSTRUMENTGEN3_EVENT_WIFI_MODULE_DEACTIVATED_REMOVED_OR_ACTIVITY_TIMEOUT               = 0x1B,
    RBRINSTRUMENTGEN3_EVENT_REGIMES_ENABLED_BUT_NOT_YET_IN_A_REGIME                           = 0x1C,
    RBRINSTRUMENTGEN3_EVENT_ENTERED_REGIME_1                                                  = 0x1D,
    RBRINSTRUMENTGEN3_EVENT_ENTERED_REGIME_2                                                  = 0x1E,
    RBRINSTRUMENTGEN3_EVENT_ENTERED_REGIME_3                                                  = 0x1F,
    RBRINSTRUMENTGEN3_EVENT_START_OF_REGIME_BIN                                               = 0x20,
    RBRINSTRUMENTGEN3_EVENT_BEGIN_PROFILING_UP_CAST                                           = 0x21,
    RBRINSTRUMENTGEN3_EVENT_BEGIN_PROFILING_DOWN_CAST                                         = 0x22,
    RBRINSTRUMENTGEN3_EVENT_END_OF_PROFILING_CAST                                             = 0x23,
    RBRINSTRUMENTGEN3_EVENT_BATTERY_FAILED_SCHEDULE_FINISHED                                  = 0x24,
    RBRINSTRUMENTGEN3_EVENT_DIRECTIONAL_DEPENDENT_SAMPLING_BEGINNING_OF_FAST_SAMPLING_MODE    = 0x25,
    RBRINSTRUMENTGEN3_EVENT_DIRECTIONAL_DEPENDENT_SAMPLING_BEGINNING_OF_SLOW_SAMPLING_MODE    = 0x26,
    RBRINSTRUMENTGEN3_EVENT_ENERGY_USED_MARKER_INTERNAL_BATTERY                               = 0x27,
    RBRINSTRUMENTGEN3_EVENT_ENERGY_USED_MARKER_EXTERNAL_POWER_SOURCE                          = 0x28
} RBRInstrumentGen3EventType;

/**
 * \brief Get a human-readable string name for an event type type.
 *
 * \param [in] type the event type
 * \return a string name for the event type
 * \see RBRInstrumentGen3Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen3EventType_name(RBRInstrumentGen3EventType type);

/**
 * \brief An instrument event.
 *
 * \see https://docs.rbr-global.com/L3commandreference/format-of-stored-data/standard-rawbin00-format/standard-format-events-markers
 * \see https://docs.rbr-global.com/L3commandreference/format-of-stored-data/easyparse-calbin00-format/easyparse-format-events-markers
 */
typedef struct RBRInstrumentGen3Event
{
    /** \brief The type of the event. */
    RBRInstrumentGen3EventType type;
    /** \brief The timestamp of the event. */
    RBRInstrumentGen3DateTime timestamp;
    /**
     * \brief The number of populated entries in
     * RBRInstrumentGen3Event.auxiliaryData.
     *
     * For EasyParse events, this will be either 0 or 1. For standard events,
     * this may be up to RBRINSTRUMENTGEN3_EVENT_AUXILIARY_DATA_MAX depending on
     * the event type.
     */
    int32_t auxiliaryDataLength;
    /** \brief Auxiliary data for the event. */
    uint32_t auxiliaryData[RBRINSTRUMENTGEN3_EVENT_AUXILIARY_DATA_MAX];
} RBRInstrumentGen3Event;

/**
 * \brief Callback to provide a parsed event to user code.
 *
 * The \a event pointer will be the same as given via
 * RBRParserGen3Callbacks.eventBuffer. The event value will be overwritten every
 * time event parsing is attempted, which will be at least once per invocation
 * of RBRParserGen3_parse() for dataset #RBRINSTRUMENTGEN3_DATASET_EASYPARSE_EVENTS
 * where the buffer is large enough. If you want to use the event after your
 * callback has returned, make a copy of it.
 *
 * \param [in] parser the dataset parser which parsed the event
 * \param [in] event the parsed event
 * \return #RBRINSTRUMENTGEN3_SUCCESS when the event data is successfully consumed
 * \return #RBRINSTRUMENTGEN3_CALLBACK_ERROR when an unrecoverable error occurs
 */
typedef RBRInstrumentGen3Error (*RBRParserGen3EventCallback)(
    const struct RBRParserGen3 *parser,
    const struct RBRInstrumentGen3Event *const event);

/**
 * \brief A set of callbacks from parser to user code.
 *
 * All of the callback functions are optional; however, where any callback
 * function is provided, the corresponding buffer must also be provided, or
 * else RBRParserGen3_init() will return #RBRINSTRUMENTGEN3_MISSING_CALLBACK.
 */
typedef struct RBRParserGen3Callbacks
{
    /**
     * \brief Called when a sample has been parsed.
     *
     * Requires that RBRParserGen3Callbacks.sampleBuffer also be populated.
     */
    RBRParserGen3SampleCallback sample;

    /**
     * \brief Where to put sample data for consumption by the sample callback.
     *
     * Required only when RBRParserGen3Callbacks.sample is populated.
     */
    RBRInstrumentGen3Sample *sampleBuffer;

    /**
     * \brief Called when an event has been parsed.
     *
     * Requires that RBRParserGen3Callbacks.eventBuffer also be populated.
     */
    RBRParserGen3EventCallback event;

    /**
     * \brief Where to put event data for consumption by the event callback.
     *
     * Required only when RBRParserGen3Callbacks.event is populated.
     */
    RBRInstrumentGen3Event *eventBuffer;
} RBRParserGen3Callbacks;

/**
 * \brief EasyParse-specific parser configuration.
 *
 * \see RBRParserGen3Config
 */
typedef struct RBRParserGen3EasyParseConfig
{
    /**
     * \brief The number of instrument channels in each sample.
     *
     * If the value is less than or equal to 0 or exceeds
     * #RBRINSTRUMENTGEN3_CHANNEL_MAX, then RBRParserGen3_init() will return
     * #RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE.
     */
    int32_t channels;
} RBRParserGen3EasyParseConfig;

/**
 * \brief Configuration for a RBRParserGen3.
 */
typedef struct RBRParserGen3Config
{
    /** \brief The format of memory being parsed. */
    RBRInstrumentGen3MemoryFormat format;

    /** \brief Format-specific configuration. */
    union
    {
        /** \brief EasyParse-specific parser configuration. */
        struct RBRParserGen3EasyParseConfig easyParse;
    } formatConfig;
} RBRParserGen3Config;

/**
 * \brief Parser context object.
 *
 * Users are strongly discouraged from accessing the fields of this structure
 * directly as layout and field availability maybe unstable from version to
 * version. Getter and setter functions are available for safely reading from
 * and writing to fields where necessary.
 *
 * \see RBRParserGen3_init() to initialize a parser
 * \see RBRParserGen3_destroy() to close a parser
 */
typedef struct RBRParserGen3
{
    /** \brief The parser configuration. */
    RBRParserGen3Config config;

    /** \brief The set of callbacks to be used by the parser. */
    RBRParserGen3Callbacks callbacks;

    /** \brief Arbitrary user data; useful in callbacks. */
    void *userData;

    /**
     * \brief Whether the instance memory was dynamically allocated by the
     * constructor.
     */
    bool managedAllocation;
} RBRParserGen3;

/**
 * \brief Initialize a dataset parser.
 *
 * The use of the \a parser argument is the same as that of the \a instrument
 * argument to RBRInstrumentGen3_open(): when given as `NULL`, instance memory will
 * be allocated for you; otherwise, the pointer target will be used as instance
 * storage. See RBRInstrumentGen3_open() for “do”s and “don't”s inherent to this
 * approach.
 *
 * Again, as with the \a callbacks argument to RBRInstrumentGen3_open(), the
 * \a config and \a callbacks structures will be copied into the RBRParserGen3
 * structure and no references to them are retained.
 *
 * Currently, the only supported memory format is
 * #RBRINSTRUMENTGEN3_MEMFORMAT_CALBIN00 (“EasyParse”). Requesting any other format
 * via RBRParserGen3Config will cause #RBRINSTRUMENTGEN3_UNSUPPORTED to be returned.
 *
 * Both callback functions are optional, but that probably isn't very useful:
 * after all, you won't receive any data that way. Still, the library won't
 * complain. If any buffer is not given for a callback function which _is_
 * given, or if \a callbacks itself is given as `NULL`, then
 * #RBRINSTRUMENTGEN3_MISSING_CALLBACK is returned and the parser instantiation
 * will not be completed.
 *
 * In the event of any return value other than #RBRINSTRUMENTGEN3_SUCCESS, any
 * memory allocated by this constructor is freed. That is, in the event of
 * failure, no cleanup of library resources is required. In the event of a
 * successful result, RBRParserGen3_destroy() should be used to release allocated
 * resources.
 *
 * \param [in,out] parser the context object to populate
 * \param [in] callbacks the set of callbacks to be used by the parser
 * \param [in] config the parser configuration
 * \param [in] userData arbitrary user data; useful in callbacks
 * \return #RBRINSTRUMENTGEN3_SUCCESS if the parser was instantiated successfully
 * \return #RBRINSTRUMENTGEN3_ALLOCATION_FAILURE if memory allocation failed
 * \return #RBRINSTRUMENTGEN3_MISSING_CALLBACK if no callbacks were provided
 * \return #RBRINSTRUMENTGEN3_UNSUPPORTED if the memory format is unsupported
 * \return #RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE if the config is invalid
 * \see RBRParserGen3_destroy()
 */
RBRInstrumentGen3Error RBRParserGen3_init(RBRParserGen3 **parser,
                                  const RBRParserGen3Callbacks *callbacks,
                                  const RBRParserGen3Config *config,
                                  void *userData);

/**
 * \brief Release any resources held by the parser.
 *
 * Frees the buffer allocated by RBRParserGen3_init() if necessary.
 *
 * \param [in,out] parser the dataset parser to close
 * \return #RBRINSTRUMENTGEN3_SUCCESS if the parser was closed successfully
 * \see RBRParserGen3_init()
 */
RBRInstrumentGen3Error RBRParserGen3_destroy(RBRParserGen3 *parser);

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
void RBRParserGen3_getConfig(const RBRParserGen3 *parser, RBRParserGen3Config *config);

/**
 * \brief Get the pointer to arbitrary user data.
 *
 * Returns whatever arbitrary pointer the user has most recently provided,
 * either via RBRParserGen3_init() or RBRParserGen3_setUserData().
 *
 * \param [in] parser the dataset parser
 * \return the arbitrary user data pointer
 * \see RBRParserGen3_setUserData()
 */
void *RBRParserGen3_getUserData(const RBRParserGen3 *parser);

/**
 * \brief Change the arbitrary user data pointer.
 *
 * \param [in,out] parser the dataset parser
 * \param [in] userData the new user data
 * \see RBRParserGen3_getUserData()
 */
void RBRParserGen3_setUserData(RBRParserGen3 *parser, void *userData);

/**
 * \brief Parse a chunk of data.
 *
 * For a parser configured to parse #RBRINSTRUMENTGEN3_MEMFORMAT_CALBIN00-format
 * data, \a dataset may be given as #RBRINSTRUMENTGEN3_DATASET_EASYPARSE_EVENTS or
 * #RBRINSTRUMENTGEN3_DATASET_EASYPARSE_SAMPLE_DATA. Any other value (including
 * #RBRINSTRUMENTGEN3_DATASET_EASYPARSE_DEPLOYMENT_HEADER, which is currently
 * unsupported) will cause the function to return
 * #RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE and no data will be parsed.
 *
 * Parsed values will be returned via the RBRParserGen3Callbacks provided to
 * RBRParserGen3_init(). The value at \a size after completion of parsing indicates
 * how much of the \a data was parsed.
 *
 * \param [in] parser the dataset parser
 * \param [in] dataset the dataset from which the chunk originated
 * \param [in] data the data to be parsed
 * \param [in,out] size initially, the size of the data given by \a data; set
 *                                 by the callback to the number of bytes
 *                                 actually parsed
 * \return #RBRINSTRUMENTGEN3_SUCCESS when no parsing errors occur
 * \return #RBRINSTRUMENTGEN3_INVALID_PARAMETER_VALUE when an invalid dataset is
 *                                                given, or when the parser
 *                                                configuration is incomplete
 *                                                or invalid
 */
RBRInstrumentGen3Error RBRParserGen3_parse(RBRParserGen3 *parser,
                                   RBRInstrumentGen3Dataset dataset,
                                   const void *const data,
                                   int32_t *size);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRPARSERGEN3_H */
