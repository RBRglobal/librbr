/**
 * \file RBRParserGen4.h
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

#include "RBRInstrumentGen4.h"

/** \brief The maximum number of pieces of auxiliary data in an event. */
#define RBRINSTRUMENTGEN4_EVENT_AUXILIARY_DATA_MAX 4

struct RBRParserGen4;

/**
 * \brief Callback to provide a parsed sample to user code.
 *
 * The \a sample pointer will be the same as given via
 * RBRParserGen4Callbacks.sampleBuffer. That buffer will be overwritten every time
 * a parsed sample is returned via this callback. If you want to retain the use
 * of the sample after your callback has returned, make a copy of it.
 *
 * \param [in] parser the dataset parser which parsed the sample
 * \param [in] sample the parsed sample
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the sample data is successfully consumed
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR when an unrecoverable error occurs
 */
typedef RBRInstrumentGen4Error (*RBRParserGen4SampleCallback)(
    const struct RBRParserGen4 *parser,
    const struct RBRInstrumentGen4Sample *const sample);

/**
 * \brief Instrument event types.
 * Gen4 Todo: Currently under construction (FW-258)
 */
typedef enum RBRInstrumentGen4EventType
{
    RBRINSTRUMENTGEN4_EVENT_UNKNOWN_OR_UNRECOGNIZED_EVENT                                     = 0x00,
    RBRINSTRUMENTGEN4_EVENT_TIME_SYNCHRONIZATION_MARKER                                       = 0x01,
    RBRINSTRUMENTGEN4_EVENT_DISABLE_COMMAND_RECEIVED                                          = 0x02,
    RBRINSTRUMENTGEN4_EVENT_RUN_TIME_ERROR_ENCOUNTERED                                        = 0x03,
    RBRINSTRUMENTGEN4_EVENT_CPU_RESET_DETECTED                                                = 0x04,
    RBRINSTRUMENTGEN4_EVENT_ONE_OR_MORE_PARAMETERS_RECOVERED_AFTER_RESET                      = 0x05,
    RBRINSTRUMENTGEN4_EVENT_RESTART_FAILED_RTC_CALENDAR_CONTENTS_NOT_VALID                    = 0x06,
    RBRINSTRUMENTGEN4_EVENT_RESTART_FAILED_LOGGER_STATUS_NOT_VALID                            = 0x07,
    RBRINSTRUMENTGEN4_EVENT_RESTART_FAILED_PRIMARY_SCHEDULE_PARAMETERS_COULD_NOT_BE_RECOVERED = 0x08,
    RBRINSTRUMENTGEN4_EVENT_UNABLE_TO_LOAD_ALARM_TIME_FOR_NEXT_SAMPLE                         = 0x09,
    RBRINSTRUMENTGEN4_EVENT_SAMPLING_RESTARTED_AFTER_RESETTING_RTC                            = 0x0A,
    RBRINSTRUMENTGEN4_EVENT_PARAMETERS_RECOVERED_SAMPLING_RESTARTED_AFTER_RESETTING_RTC       = 0x0B,
    RBRINSTRUMENTGEN4_EVENT_SAMPLING_STOPPED_END_TIME_REACHED                                 = 0x0C,
    RBRINSTRUMENTGEN4_EVENT_START_OF_A_RECORDED_BURST                                         = 0x0D,
    RBRINSTRUMENTGEN4_EVENT_START_OF_A_WAVE_BURST                                             = 0x0E,
    RBRINSTRUMENTGEN4_EVENT_RESERVED1                                                         = 0x0F,
    RBRINSTRUMENTGEN4_EVENT_STREAMING_NOW_OFF_FOR_BOTH_PORTS                                  = 0x10,
    RBRINSTRUMENTGEN4_EVENT_STREAMING_ON_FOR_USB_OFF_FOR_SERIAL                               = 0x11,
    RBRINSTRUMENTGEN4_EVENT_STREAMING_OFF_FOR_USB_ON_FOR_SERIAL                               = 0x12,
    RBRINSTRUMENTGEN4_EVENT_STREAMING_NOW_ON_FOR_BOTH_PORTS                                   = 0x13,
    RBRINSTRUMENTGEN4_EVENT_SAMPLING_STARTED_THRESHOLD_CONDITION_SATISFIED                    = 0x14,
    RBRINSTRUMENTGEN4_EVENT_SAMPLING_PAUSED_THRESHOLD_CONDITION_NOT_MET                       = 0x15,
    RBRINSTRUMENTGEN4_EVENT_POWER_SOURCE_SWITCHED_TO_INTERNAL_BATTERY                         = 0x16,
    RBRINSTRUMENTGEN4_EVENT_POWER_SOURCE_SWITCHED_TO_EXTERNAL_BATTERY                         = 0x17,
    RBRINSTRUMENTGEN4_EVENT_TWIST_ACTIVATION_STARTED_SAMPLING                                 = 0x18,
    RBRINSTRUMENTGEN4_EVENT_TWIST_ACTIVATION_PAUSED_SAMPLING                                  = 0x19,
    RBRINSTRUMENTGEN4_EVENT_WIFI_MODULE_DETECTED_AND_ACTIVATED                                = 0x1A,
    RBRINSTRUMENTGEN4_EVENT_WIFI_MODULE_DEACTIVATED_REMOVED_OR_ACTIVITY_TIMEOUT               = 0x1B,
    RBRINSTRUMENTGEN4_EVENT_REGIMES_ENABLED_BUT_NOT_YET_IN_A_REGIME                           = 0x1C,
    RBRINSTRUMENTGEN4_EVENT_ENTERED_REGIME_1                                                  = 0x1D,
    RBRINSTRUMENTGEN4_EVENT_ENTERED_REGIME_2                                                  = 0x1E,
    RBRINSTRUMENTGEN4_EVENT_ENTERED_REGIME_3                                                  = 0x1F,
    RBRINSTRUMENTGEN4_EVENT_START_OF_REGIME_BIN                                               = 0x20,
    RBRINSTRUMENTGEN4_EVENT_BEGIN_PROFILING_UP_CAST                                           = 0x21,
    RBRINSTRUMENTGEN4_EVENT_BEGIN_PROFILING_DOWN_CAST                                         = 0x22,
    RBRINSTRUMENTGEN4_EVENT_END_OF_PROFILING_CAST                                             = 0x23,
    RBRINSTRUMENTGEN4_EVENT_BATTERY_FAILED_SCHEDULE_FINISHED                                  = 0x24,
    RBRINSTRUMENTGEN4_EVENT_DIRECTIONAL_DEPENDENT_SAMPLING_BEGINNING_OF_FAST_SAMPLING_MODE    = 0x25,
    RBRINSTRUMENTGEN4_EVENT_DIRECTIONAL_DEPENDENT_SAMPLING_BEGINNING_OF_SLOW_SAMPLING_MODE    = 0x26,
    RBRINSTRUMENTGEN4_EVENT_ENERGY_USED_MARKER_INTERNAL_BATTERY                               = 0x27,
    RBRINSTRUMENTGEN4_EVENT_ENERGY_USED_MARKER_EXTERNAL_POWER_SOURCE                          = 0x28
} RBRInstrumentGen4EventType;

/**
 * \brief Get a human-readable string name for an event type type.
 *
 * \param [in] type the event type
 * \return a string name for the event type
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4EventType_name(RBRInstrumentGen4EventType type);

/**
 * \brief An instrument event.
 *
 * \see https://docs.rbr-global.com/L3commandreference/format-of-stored-data/standard-rawbin00-format/standard-format-events-markers
 * \see https://docs.rbr-global.com/L3commandreference/format-of-stored-data/easyparse-calbin00-format/easyparse-format-events-markers
 */
typedef struct RBRInstrumentGen4Event
{
    /** \brief The type of the event. */
    RBRInstrumentGen4EventType type;
    /** \brief The timestamp of the event. */
    RBRInstrumentGen4DateTime timestamp;
    /**
     * \brief The number of populated entries in
     * RBRInstrumentGen4Event.auxiliaryData.
     *
     * For EasyParse events, this will be either 0 or 1. For standard events,
     * this may be up to RBRINSTRUMENTGEN4_EVENT_AUXILIARY_DATA_MAX depending on
     * the event type.
     */
    int32_t auxiliaryDataLength;
    /** \brief Auxiliary data for the event. */
    uint32_t auxiliaryData[RBRINSTRUMENTGEN4_EVENT_AUXILIARY_DATA_MAX];
} RBRInstrumentGen4Event;

/**
 * \brief Callback to provide a parsed event to user code.
 *
 * The \a event pointer will be the same as given via
 * RBRParserGen4Callbacks.eventBuffer. The event value will be overwritten every
 * time event parsing is attempted, which will be at least once per invocation
 * of RBRParserGen4_parse() for dataset RBRINSTRUMENTGEN4_DATASET_EASYPARSE_EVENTS
 * where the buffer is large enough. If you want to use the event after your
 * callback has returned, make a copy of it.
 *
 * \param [in] parser the dataset parser which parsed the event
 * \param [in] event the parsed event
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the event data is successfully consumed
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR when an unrecoverable error occurs
 */
typedef RBRInstrumentGen4Error (*RBRParserGen4EventCallback)(
    const struct RBRParserGen4 *parser,
    const struct RBRInstrumentGen4Event *const event);

/**
 * \brief A set of callbacks from parser to user code.
 *
 * All of the callback functions are optional; however, where any callback
 * function is provided, the corresponding buffer must also be provided, or
 * else RBRParserGen4_init() will return #RBRINSTRUMENTGEN4_MISSING_CALLBACK.
 */
typedef struct RBRParserGen4Callbacks
{
    /**
     * \brief Called when a sample has been parsed.
     *
     * Requires that RBRParserGen4Callbacks.sampleBuffer also be populated.
     */
    RBRParserGen4SampleCallback sample;

    /**
     * \brief Where to put sample data for consumption by the sample callback.
     *
     * Required only when RBRParserGen4Callbacks.sample is populated.
     */
    RBRInstrumentGen4Sample *sampleBuffer;

    /**
     * \brief Called when an event has been parsed.
     *
     * Requires that RBRParserGen4Callbacks.eventBuffer also be populated.
     */
    RBRParserGen4EventCallback event;

    /**
     * \brief Where to put event data for consumption by the event callback.
     *
     * Required only when RBRParserGen4Callbacks.event is populated.
     */
    RBRInstrumentGen4Event *eventBuffer;
} RBRParserGen4Callbacks;

/**
 * \brief EasyParse-specific parser configuration.
 *
 * \see RBRParserGen4Config
 */
typedef struct RBRParserGen4EasyParseConfig
{
    /**
     * \brief The number of instrument channels in each sample.
     *
     * If the value is less than or equal to 0 or exceeds
     * #RBRINSTRUMENTGEN4_CHANNEL_MAX, then RBRParserGen4_init() will return
     * #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE.
     */
    int32_t channels;
} RBRParserGen4EasyParseConfig;

/**
 * \brief Configuration for a RBRParserGen4.
 */
typedef struct RBRParserGen4Config
{

    /** \brief Format-specific configuration. */
    union
    {
        /** \brief EasyParse-specific parser configuration. */
        struct RBRParserGen4EasyParseConfig easyParse;
    } formatConfig;
} RBRParserGen4Config;

/**
 * \brief Parser context object.
 *
 * Users are strongly discouraged from accessing the fields of this structure
 * directly as layout and field availability maybe unstable from version to
 * version. Getter and setter functions are available for safely reading from
 * and writing to fields where necessary.
 *
 * \see RBRParserGen4_init() to initialize a parser
 * \see RBRParserGen4_destroy() to close a parser
 */
typedef struct RBRParserGen4
{
    /** \brief The parser configuration. */
    RBRParserGen4Config config;

    /** \brief The set of callbacks to be used by the parser. */
    RBRParserGen4Callbacks callbacks;

    /** \brief Arbitrary user data; useful in callbacks. */
    void *userData;

} RBRParserGen4;

/**
 * \brief Initialize a dataset parser.
 *
 * The use of the \a parser argument is the same as that of the \a instrument
 * argument to RBRInstrumentGen4_open(): when given as `NULL`, instance memory will
 * be allocated for you; otherwise, the pointer target will be used as instance
 * storage. See RBRInstrumentGen4_open() for “do”s and “don't”s inherent to this
 * approach.
 *
 * Again, as with the \a callbacks argument to RBRInstrumentGen4_open(), the
 * \a config and \a callbacks structures will be copied into the RBRParserGen4
 * structure and no references to them are retained.
 *
 * Currently, the only supported memory format is
 * RBRINSTRUMENTGEN4_MEMFORMAT_CALBIN00 (“EasyParse”). Requesting any other format
 * via RBRParserGen4Config will cause #RBRINSTRUMENTGEN4_UNSUPPORTED to be returned.
 *
 * Both callback functions are optional, but that probably isn't very useful:
 * after all, you won't receive any data that way. Still, the library won't
 * complain. If any buffer is not given for a callback function which _is_
 * given, or if \a callbacks itself is given as `NULL`, then
 * #RBRINSTRUMENTGEN4_MISSING_CALLBACK is returned and the parser instantiation
 * will not be completed.
 *
 * In the event of any return value other than #RBRINSTRUMENTGEN4_SUCCESS, any
 * memory allocated by this constructor is freed. That is, in the event of
 * failure, no cleanup of library resources is required. In the event of a
 * successful result, RBRParserGen4_destroy() should be used to release allocated
 * resources.
 *
 * \param [in,out] parser the context object to populate
 * \param [in] callbacks the set of callbacks to be used by the parser
 * \param [in] config the parser configuration
 * \param [in] userData arbitrary user data; useful in callbacks
 * \return #RBRINSTRUMENTGEN4_SUCCESS if the parser was instantiated successfully
 * \return #RBRINSTRUMENTGEN4_MISSING_CALLBACK if no callbacks were provided
 * \return #RBRINSTRUMENTGEN4_UNSUPPORTED if the memory format is unsupported
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE if the config is invalid
 * \see RBRParserGen4_destroy()
 */
RBRInstrumentGen4Error RBRParserGen4_init(RBRParserGen4 **parser,
                                  const RBRParserGen4Callbacks *callbacks,
                                  const RBRParserGen4Config *config,
                                  void *userData);

/**
 * \brief Release any resources held by the parser.
 *
 * Frees the buffer allocated by RBRParserGen4_init() if necessary.
 *
 * \param [in,out] parser the dataset parser to close
 * \return #RBRINSTRUMENTGEN4_SUCCESS if the parser was closed successfully
 * \see RBRParserGen4_init()
 */
RBRInstrumentGen4Error RBRParserGen4_destroy(RBRParserGen4 *parser);

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
void RBRParserGen4_getConfig(const RBRParserGen4 *parser, RBRParserGen4Config *config);

/**
 * \brief Get the pointer to arbitrary user data.
 *
 * Returns whatever arbitrary pointer the user has most recently provided,
 * either via RBRParserGen4_init() or RBRParserGen4_setUserData().
 *
 * \param [in] parser the dataset parser
 * \return the arbitrary user data pointer
 * \see RBRParserGen4_setUserData()
 */
void *RBRParserGen4_getUserData(const RBRParserGen4 *parser);

/**
 * \brief Change the arbitrary user data pointer.
 *
 * \param [in,out] parser the dataset parser
 * \param [in] userData the new user data
 * \see RBRParserGen4_getUserData()
 */
void RBRParserGen4_setUserData(RBRParserGen4 *parser, void *userData);

/**
 * \brief Parse a chunk of data.
 *
 * For a parser configured to parse RBRINSTRUMENTGEN4_MEMFORMAT_CALBIN00-format
 * data, \a dataset may be given as RBRINSTRUMENTGEN4_DATASET_EASYPARSE_EVENTS or
 * RBRINSTRUMENTGEN4_DATASET_EASYPARSE_SAMPLE_DATA. Any other value (including
 * RBRINSTRUMENTGEN4_DATASET_EASYPARSE_DEPLOYMENT_HEADER, which is currently
 * unsupported) will cause the function to return
 * #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE and no data will be parsed.
 *
 * Parsed values will be returned via the RBRParserGen4Callbacks provided to
 * RBRParserGen4_init(). The value at \a size after completion of parsing indicates
 * how much of the \a data was parsed.
 *
 * \param [in] parser the dataset parser
 * \param [in] dataset the dataset from which the chunk originated
 * \param [in] data the data to be parsed
 * \param [in,out] size initially, the size of the data given by \a data; set
 *                                 by the callback to the number of bytes
 *                                 actually parsed
 * \return #RBRINSTRUMENTGEN4_SUCCESS when no parsing errors occur
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when an invalid dataset is
 *                                                given, or when the parser
 *                                                configuration is incomplete
 *                                                or invalid
 */
RBRInstrumentGen4Error RBRParserGen4_parse(RBRParserGen4 *parser,
                                   RBRInstrumentGen4Dataset dataset,
                                   const void *const data,
                                   int32_t *size);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRPARSER_H */
