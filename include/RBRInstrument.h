/**
 * \file RBRInstrument.h
 *
 * \brief Interface for simplified communication with RBR instruments.
 *
 * This file contains declarations and constants relevant throughout the
 * library. Command-specific declarations are stored in categorical headers
 * which are included by this one. As the end user, your include directives
 * need reference only this file.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENT_H
#define LIBRBR_RBRINSTRUMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#include "RBRInstrumentHardwareErrors.h"

/**
 * \brief The library name.
 *
 * As shipped by RBR, this builds with the value “libRBR”. Project forks might
 * like to change this at build time to easily identify which library variant
 * is in use. See the Makefile for details.
 */
extern const char *RBRINSTRUMENT_LIB_NAME;
/**
 * \brief The library version.
 *
 * As shipped by RBR, this builds with a value based on the contents of the
 * VERSION file. Project forks might also like to override this at build time.
 */
extern const char *RBRINSTRUMENT_LIB_VERSION;
/**
 * \brief The library build date.
 *
 * Stored in ISO 8601 format (“YYYY-mm-ddTHH:MM:SS±hhmm”).
 */
extern const char *RBRINSTRUMENT_LIB_BUILD_DATE;

/**
 * \brief The size of the buffer storing commands destined for the instrument.
 *
 * Must be large enough to hold the largest command you will want to send to
 * the instrument plus the trailing line termination (`\r\n\0`).
 *
 * A buffer of this size is included in RBRInstrument. Whether you let
 * RBRInstrument_open() perform its own allocation or you perform your own
 * allocation based on `sizeof(RBRInstrument)`, a buffer of this size is
 * included.
 */
#ifndef RBRINSTRUMENT_COMMAND_BUFFER_MAX
#define RBRINSTRUMENT_COMMAND_BUFFER_MAX 120
#endif

/**
 * \brief The size of the buffer storing instrument responses.
 *
 * Must be large enough to hold the largest command response you will want to
 * receive. This does not include download data, which is read directly into a
 * user-managed buffer.
 *
 * A buffer of this size is included in RBRInstrument. Whether you let
 * RBRInstrument_open() perform its own allocation or you perform your own
 * allocation based on `sizeof(RBRInstrument)`, a buffer of this size is
 * included.
 */
#ifndef RBRINSTRUMENT_RESPONSE_BUFFER_MAX
#define RBRINSTRUMENT_RESPONSE_BUFFER_MAX 1024
#endif

/**
 * \brief The maximum number of channels present on an instrument.
 *
 * The default maximum of 32 channels is reflective of the maximum number of
 * channels supported by RBR instruments, but most instruments have far fewer.
 * Adjusting this value will dramatically affect the size of some structures;
 * notably RBRInstrumentSample, but also RBRInstrumentChannels (used by
 * RBRInstrument_getChannels()) and RBRInstrumentChannelsList (used by
 * RBRInstrument_getChannelsList()).
 */
#ifndef RBRINSTRUMENT_CHANNEL_MAX
#define RBRINSTRUMENT_CHANNEL_MAX 32
#endif

/** \brief Stringize the result of macro expansion. */
#define xstr(s)                       str(s)
/** \brief Stringize the macro argument. */
#define str(s)                        #s
/** \brief The string length of the maximum number of instrument channels. */
#define RBRINSTRUMENT_CHANNEL_MAX_LEN sizeof(xstr(RBRINSTRUMENT_CHANNEL_MAX))

/**
 * \brief The maximum number of characters in a channel name (e.g.,
 * “Temperature”).
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENT_CHANNEL_NAME_MAX 31

/**
 * \brief The maximum number of characters in a channel type (e.g., “temp09”).
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENT_CHANNEL_TYPE_MAX 11

/**
 * \brief The maximum number of characters in a channel unit name (e.g., “C”).
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENT_CHANNEL_UNIT_MAX 7

/**
 * \brief The maximum number of characters in a channel label.
 *
 * Does not include any null terminator. Must be at least 4 (to hold “none”,
 * the default value).
 */
#define RBRINSTRUMENT_CHANNEL_LABEL_MAX 31

/**
 * \brief The minimum date and time which the instrument can handle.
 *
 * Specified in milliseconds since the Unix epoch (1970-01-01T00:00:00.000Z).
 * Represents 2000-01-01T00:00:00.000Z.
 */
#define RBRINSTRUMENT_DATETIME_MIN 946684800000LL

/**
 * \brief The minimum date and time which the instrument can handle.
 *
 * Specified in milliseconds since the Unix epoch (1970-01-01T00:00:00.000Z).
 * Represents 2099-12-31T23:59:59.000Z.
 */
#define RBRINSTRUMENT_DATETIME_MAX 4102444799000LL

/**
 * \brief The maximum number of characters in the instrument model name.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENT_ID_MODEL_MAX 14

/**
 * \brief The maximum number of characters in the instrument firmware version.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENT_ID_VERSION_MAX 7

/**
 * \brief The maximum number of characters in the instrument mode.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENT_ID_MODE_MAX 10

/**
 * A date and time in milliseconds since the Unix epoch
 * (1970-01-01T00:00:00.000Z). Instrument functions operating on time (e.g.,
 * RBRInstrument_getClock(), RBRInstrument_setClock()) will automatically
 * convert to and from the instrument's string time representation.
 *
 * The valid range for any instrument date/time parameter is
 * 2000-01-01T00:00:00.000Z to 2099-12-31T23:59:59.000Z, inclusive. Passing a
 * value outside of this range will be detected by the library and will cause a
 * #RBRINSTRUMENT_INVALID_PARAMETER_VALUE error, not a hardware error.
 */
typedef int64_t RBRInstrumentDateTime;

/**
 * \brief A periodic parameter.
 *
 * Specified in milliseconds. Generally, parameters of this type must be
 * greater than 0, may not be greater than 86,400,000 (24 hours), and must be
 * multiples of 1,000 when greater than 1,000. See specific parameter
 * documentation for details.
 */
typedef int32_t RBRInstrumentPeriod;

/**
 * \brief Errors which can be returned from library functions.
 *
 * Generally speaking, library functions will return error codes in lieu of
 * data values; data will be passed back to the caller via out pointers. This
 * allows for predictable and consistent error checking by the caller.
 */
typedef enum RBRInstrumentError
{
    /** No error. */
    RBRINSTRUMENT_SUCCESS,
    /** An error occurred while allocating memory. This is typically fatal. */
    RBRINSTRUMENT_ALLOCATION_FAILURE,
    /** The command buffer was too small to hold the outbound command. */
    RBRINSTRUMENT_BUFFER_TOO_SMALL,
    /** A required callback function was not provided. */
    RBRINSTRUMENT_MISSING_CALLBACK,
    /** An unrecoverable error from within a user callback function. */
    RBRINSTRUMENT_CALLBACK_ERROR,
    /** A timeout occurred. */
    RBRINSTRUMENT_TIMEOUT,
    /** The instrument or command is unsupported by the library. */
    RBRINSTRUMENT_UNSUPPORTED,
    /**
     * The physical instrument reported a warning or error.
     *
     * \see RBRInstrument_getLastHardwareError()
     */
    RBRINSTRUMENT_HARDWARE_ERROR,
    /** A CRC check failed. */
    RBRINSTRUMENT_CHECKSUM_ERROR,
    /** The given value is out of bounds or otherwise unsuitable. */
    RBRINSTRUMENT_INVALID_PARAMETER_VALUE,
    /**
     * Used internally when the parser encounters a sample.
     *
     * \see RBRInstrument_fetch()
     * \see RBRInstrument_readSample()
     */
    RBRINSTRUMENT_SAMPLE,
    /** Communication error. */
    RBRINSTRUMENT_COMMUNICATION_ERROR,
    /** The number of specific errors. Should not be used as an error value. */
    RBRINSTRUMENT_ERROR_COUNT,
    /** An unknown or unrecognized error. */
    RBRINSTRUMENT_UNKNOWN_ERROR
} RBRInstrumentError;

/**
 * \brief Get a human-readable string name for a library error.
 *
 * Names are a “friendlier” version of the error enum constant names: they have
 * the `RBRINSTRUMENT_` prefix removed, are converted to lower-case, and words
 * are space-separated instead of underscore-separated.
 *
 * For example:
 *
 * ~~~{.c}
 * RBRInstrumentError error = ...;
 * if (error != RBRINSTRUMENT_SUCCESS)
 * {
 *     fprintf(stderr,
 *             "Encountered an error: %s!\n",
 *             RBRInstrumentError_name(error));
 * }
 * ~~~
 *
 * ...might print something like:
 *
 * ~~~{.txt}
 * Encountered an error: timeout!
 * ~~~
 *
 * \param [in] error the error
 * \return a string name for the error
 */
const char *RBRInstrumentError_name(RBRInstrumentError error);

/**
 * \brief Instrument `id` command parameters.
 *
 * Externalized from RBRInstrumentOther.h to facilitate inclusion by
 * RBRInstrument.
 *
 * \see RBRInstrument_getId()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/id
 */
typedef struct RBRInstrumentId
{
    /** The instrument model. */
    char model[RBRINSTRUMENT_ID_MODEL_MAX + 1];
    /** The instrument firmware version. */
    char version[RBRINSTRUMENT_ID_VERSION_MAX + 1];
    /** The serial number of the instrument. */
    uint32_t serial;
    /** The firmware type of the instrument. */
    uint16_t fwtype;
    /** The instrument mode. */
    char mode[RBRINSTRUMENT_ID_MODE_MAX + 1];
} RBRInstrumentId;

/** \brief Generations of RBR instruments. */
typedef enum RBRInstrumentGeneration
{
    /** Logger1 (XR/XRX/TR/DR/TDR/HT). */
    RBRINSTRUMENT_LOGGER1,
    /** Logger2 (RBRvirtuoso/duo/concerto/maestro/solo/duet/coda). */
    RBRINSTRUMENT_LOGGER2,
    /** Logger3 (RBRvirtuoso³/duo³/concerto³/maestro³/solo³/duet³/coda³). */
    RBRINSTRUMENT_LOGGER3,
    /** The number of known generations. */
    RBRINSTRUMENT_GENERATION_COUNT,
    /** An unknown or unrecognized instrument generation. */
    RBRINSTRUMENT_UNKNOWN_GENERATION
} RBRInstrumentGeneration;

/**
 * \brief Get a human-readable string name for a generation.
 *
 * Contrary to convention for values returned by other enum `_name` functions,
 * the generation names returned by this function are capitalized: “Logger3”
 * instead of “logger3”.
 *
 * \param [in] generation the generation
 * \return a string name for the generation
 * \see RBRInstrumentError_name() for a description of the format of names
 */
const char *RBRInstrumentGeneration_name(RBRInstrumentGeneration generation);

struct RBRInstrument;

/**
 * \brief Callback to get the current platform time in milliseconds.
 *
 * Return values must be positive and monotonically increasing.
 *
 * Library functions will call this user code to determine whether an
 * instrument has likely gone to sleep (based on time of last communication).
 * The time returned should be independent of any instrument (i.e., a real
 * system time) and, while it must return a number of milliseconds, that number
 * need be relative only to other values returned by the callback (i.e.,
 * doesn't need to be an RTC time). On POSIX systems, the value can easily be
 * based on CLOCK_BOOTTIME (or CLOCK_MONOTONIC on older systems where
 * CLOCK_BOOTTIME is unavailable).
 *
 * \param [in] instrument the instrument for which the time is being requested
 * \param [out] time the current platform time in milliseconds
 * \return #RBRINSTRUMENT_SUCCESS when the time is successfully retrieved
 * \return #RBRINSTRUMENT_CALLBACK_ERROR when an unrecoverable error occurs
 * \see RBRInstrumentReadCallback() for details on how the values returned from
 *                                  user callback functions are used
 */
typedef RBRInstrumentError (*RBRInstrumentTimeCallback)(const struct RBRInstrument *instrument,
                                                        RBRInstrumentDateTime *time);

/**
 * \brief Callback to suspend instrument activity for a fixed amount of time.
 *
 * Library functions will call this user code when they know the instrument
 * will be unavailable particularly when waking the instrument from sleep.
 *
 * Library functions will call this user code to suspend activity for a fixed amount
 * of time.
 *
 * \param [in] instrument the instrument for which sleep is being requested
 * \param [in] time the duration for which a sleep is requested in milliseconds
 * \return #RBRINSTRUMENT_SUCCESS when the time is successfully retrieved
 * \return #RBRINSTRUMENT_CALLBACK_ERROR when an unrecoverable error occurs
 * \see RBRInstrumentReadCallback() for details on how the values returned from
 *                                  user callback functions are used
 */
typedef RBRInstrumentError (*RBRInstrumentSleepCallback)(const struct RBRInstrument *instrument,
                                                         RBRInstrumentDateTime time);

/**
 * \brief Callback to read data from the physical instrument.
 *
 * Library functions will call this user code to read data from the instrument.
 * This function should block until data is available (even if only a single
 * byte) or until the timeout has elapsed, then return. It will be called
 * multiple times in quick succession if the library requires more data.
 *
 * The library will provide a destination for data read from the instrument via
 * the \a data argument. The maximum amount of data which can be written to
 * this location is given by the \a size argument. Before returning
 * #RBRINSTRUMENT_SUCCESS, the value of \a size should be updated by the
 * callback to reflect the number of bytes written to \a data. When a value
 * other than #RBRINSTRUMENT_SUCCESS is returned, any new value of \a size
 * is ignored, as is any data written to \a data.
 *
 * The function should return #RBRINSTRUMENT_SUCCESS when data is successfully
 * read from the instrument. In the event of any other value being returned,
 * the calling library function will treat that value as indicative of an
 * error, immediately perform any necessary cleanup, and then return that same
 * value to its caller. On Posix systems, the library will avoid doing anything
 * which might disturb the value of `errno` before returning to user code. It
 * is strongly suggested that #RBRINSTRUMENT_TIMEOUT be returned in the event
 * of a timeout and that #RBRINSTRUMENT_CALLBACK_ERROR be returned under any
 * other circumstance; that way a clear distinction can be made between errors
 * occurring in user code versus library code.
 *
 * Because communication is handled by user code, and because communication
 * only occurs at the behest of the user, it is up to the user to define the
 * semantics of communication timeouts and to implement them. This could be a
 * constant read/write timeout, or a per-connection timeout tied to the
 * instrument by the user data pointer; the library is unopinionated.
 *
 * \param [in] instrument the instrument for which data is being requested
 * \param [in,out] data where up to \a size bytes of data can be written
 * \param [in,out] size initially, the maximum amount of data which can be
 *                      written to \a data; set by the callback to the number
 *                      of bytes actually written
 * \return #RBRINSTRUMENT_SUCCESS when data is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR when an unrecoverable error occurs
 */
typedef RBRInstrumentError (*RBRInstrumentReadCallback)(const struct RBRInstrument *instrument,
                                                        void *data, int32_t *size);

/**
 * \brief Callback to write data to the physical instrument.
 *
 * Library functions will call this user code to write data to the instrument.
 * This function should block until all data has been written or until the
 * timeout has elapsed.
 *
 * The \a data pointer should not be used after the callback returns. Do not
 * store copies of it; if you want to use it after your callback has returned
 * (perhaps for logging), copy the data instead.
 *
 * The library will attempt to call this function only for complete commands
 * (i.e., will try to avoid segmenting a single command over multiple callback
 * invocations). Depending on the transport layer, this may be useful (e.g.,
 * for a IP transport, to know that each callback payload could be sent as an
 * individual packet).
 *
 * \param [in] instrument the instrument for which data is being sent
 * \param [in] data the data to be written to the instrument
 * \param [in] size the size of the data given by \a data
 * \return #RBRINSTRUMENT_SUCCESS when the data is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR when an unrecoverable error occurs
 * \see RBRInstrumentReadCallback() for details on how the values returned from
 *                                  user callback functions are used
 */
typedef RBRInstrumentError (*RBRInstrumentWriteCallback)(const struct RBRInstrument *instrument,
                                                         const void *const data, int32_t size);

struct RBRInstrumentSample;

/**
 * \brief Callback to feed streaming sample data into user code.
 *
 * Library functions will call this user code when a streaming sample has been
 * received.
 *
 * The \a sample pointer will be the same as given via
 * RBRInstrumentCallbacks.sampleBuffer. The sample value will be overwritten
 * every time sample parsing is attempted, which will be at least once per
 * command exchanged with the instrument. If you want to use the sample after
 * your callback has returned, make a copy of it.
 *
 * This function is typically called as a side effect of parsing an instrument
 * response to some other command, and may be called several times in
 * succession if multiple samples have been received since the last instrument
 * activity. As such, this callback should execute quickly to avoid blocking
 * anything else.
 *
 * \param [in] instrument the instrument from which the sample was received
 * \param [in] sample the sample received from the instrument
 * \return #RBRINSTRUMENT_SUCCESS when the sample data is successfully consumed
 * \return #RBRINSTRUMENT_CALLBACK_ERROR when an unrecoverable error occurs
 */
typedef RBRInstrumentError (*RBRInstrumentSampleCallback)(
    const struct RBRInstrument *instrument, const struct RBRInstrumentSample *const sample);

/**
 * \brief A set of callbacks from library to user code.
 *
 * RBRInstrument_open() requires all callbacks to be populated except for
 * RBRInstrumentCallbacks.sample, which may be `NULL` when undesired.
 */
typedef struct RBRInstrumentCallbacks
{
    /** \brief Callback to get the current platform time in milliseconds. */
    RBRInstrumentTimeCallback time;

    /** \brief Callback to suspend activity for a fixed amount of time. */
    RBRInstrumentSleepCallback sleep;

    /** \brief Called to read data from the physical instrument. */
    RBRInstrumentReadCallback read;

    /** \brief Called to write data to the physical instrument. */
    RBRInstrumentWriteCallback write;

    /**
     * \brief Called when streaming sample data has been received.
     *
     * Optional, but requires that RBRInstrumentCallbacks.sampleBuffer also be
     * populated.
     */
    RBRInstrumentSampleCallback sample;

    /**
     * \brief Where to put sample data for consumption by the sample callback.
     *
     * Required only when RBRInstrumentCallbacks.sample is populated.
     */
    struct RBRInstrumentSample *sampleBuffer;
} RBRInstrumentCallbacks;

/**
 * \brief The types of responses returned by the instrument.
 *
 * Used by RBRInstrumentResponse.
 */
typedef enum RBRInstrumentResponseType
{
    /** A success indicator or informational response. */
    RBRINSTRUMENT_RESPONSE_INFO,
    /** Typically indicates that the command succeeded but with caveats. */
    RBRINSTRUMENT_RESPONSE_WARNING,
    /** A command failure. */
    RBRINSTRUMENT_RESPONSE_ERROR,
    /** The number of specific types. */
    RBRINSTRUMENT_RESPONSE_TYPE_COUNT,
    /** The response has been incorrectly or incompletely populated. */
    RBRINSTRUMENT_RESPONSE_UNKNOWN_TYPE
} RBRInstrumentResponseType;

/**
 * \brief Get a human-readable string name for a response type.
 *
 * \param [in] type the response type
 * \return a string name for the response type
 * \see RBRInstrumentError_name() for a description of the format of names
 */
const char *RBRInstrumentResponseType_name(RBRInstrumentResponseType type);

/**
 * \brief A command response returned by the instrument.
 */
typedef struct RBRInstrumentResponse
{
    /**
     * \brief The type of this response: informational, warning, or error.
     *
     * Successful commands, as indicated by the command having returned
     * #RBRINSTRUMENT_SUCCESS, may yield informational or warning responses
     * (types #RBRINSTRUMENT_RESPONSE_INFO and #RBRINSTRUMENT_RESPONSE_WARNING,
     * respectively). Commands having resulted in a hardware error will return
     * #RBRINSTRUMENT_HARDWARE_ERROR and yield an error response (type
     * #RBRINSTRUMENT_RESPONSE_ERROR). In any other case, the response is
     * unpopulated and its contents are irrelevant (type
     * #RBRINSTRUMENT_RESPONSE_UNKNOWN_TYPE).
     *
     * - Informational responses will provide only a response (number as `0`).
     * - Warnings and errors will provide a number and occasionally a response.
     * - Otherwise, the response number will be `0`, and the response `NULL`.
     */
    RBRInstrumentResponseType type;
    /**
     * \brief The instrument warning or error number, if applicable.
     *
     * Will be `0` for informational responses. Otherwise, will include the
     * error number indicated by the instrument; e.g., for “E0109: feature not
     * available”, this field will contain `109`, aka
     * RBRINSTRUMENT_HARDWARE_ERROR_FEATURE_NOT_AVAILABLE.
     */
    RBRInstrumentHardwareError error;
    /**
     * \brief The response, if available.
     *
     * Will be `NULL` when absent (_not_ a pointer to a 0-length string).
     * Otherwise points to a null-terminated C string.
     */
    char *response;
} RBRInstrumentResponse;

/**
 * \brief Core library context object.
 *
 * Users are strongly discouraged from accessing the fields of this structure
 * directly as layout and field availability maybe unstable from version to
 * version. Getter and setter functions are available for safely reading from
 * and writing to fields where necessary.
 *
 * \see RBRInstrument_open() to open an instrument connection
 * \see RBRInstrument_close() to close an instrument connection
 */
typedef struct RBRInstrument
{
    /**
     * \brief The instrument identifier.
     *
     * Cached every time RBRInstrument_getId() is called.
     */
    struct RBRInstrumentId id;

    /**
     * \brief The generation of the instrument.
     *
     * Detected while establishing the instrument connection.
     */
    RBRInstrumentGeneration generation;

    /** \brief The set of callbacks to be used by the connection. */
    RBRInstrumentCallbacks callbacks;

    /**
     * \brief The command timeout in milliseconds.
     *
     * See [Timeouts](timeouts.md) for details on how the library handles
     * timeouts.
     */
    RBRInstrumentDateTime commandTimeout;

    /** \brief Arbitrary user data; useful in callbacks. */
    void *userData;

    /** \brief The number of used bytes in the command buffer. */
    int32_t commandBufferLength;

    /** \brief The number of used bytes in the response buffer. */
    int32_t responseBufferLength;

    /**
     * \brief The time at which instrument communication last occurred.
     *
     * Used to determine whether the instrument needs to be woken before
     * further commands are sent.
     */
    RBRInstrumentDateTime lastActivityTime;

    /**
     * \brief The length in bytes of the most recent response.
     *
     * Used when moving the last response out of the parse buffer before
     * receiving new data.
     */
    int32_t lastResponseLength;

    /**
     * \brief The next command to be sent to the instrument.
     *
     * Intentionally not a `char` array to discourage the use of `str`
     * functions. Commands may contain binary data and should not be assumed to
     * be null-terminated.
     */
    uint8_t commandBuffer[RBRINSTRUMENT_COMMAND_BUFFER_MAX];

    /**
     * \brief Data received from the instrument.
     *
     * Intentionally not a `char` array to discourage the use of `str`
     * functions. Responses may contain binary data and should not be assumed
     * to be null-terminated. \ref RBRInstrumentResponse.response, when
     * non-`NULL`, provides null-terminated, C-string access to the response.
     */
    uint8_t responseBuffer[RBRINSTRUMENT_RESPONSE_BUFFER_MAX];

    /**
     * \brief The most recent response received from the instrument.
     *
     * After the response parser identifies and terminates a command response,
     * attributes of the response and its beginning position within the
     * response buffer are recorded within this struct.
     */
    RBRInstrumentResponse response;

    /**
     * \brief Whether the instance memory was dynamically allocated by the
     * constructor.
     */
    bool managedAllocation;
} RBRInstrument;

/**
 * \brief Establish a connection with an instrument and initialize the context.
 *
 * What this library calls a “connection” concerns purely the state tracking
 * and management of an instrument: the underlying physical communication with
 * that instrument (via serial, TCP/IP socket, RFC 1149, whatever) must be
 * managed externally and exposed to the library via callbacks.
 *
 * If the instrument pointer is `NULL`, enough memory will be allocated (via
 * `malloc(3)`) to satisfy the requirements of all instrument communications.
 *
 * For example:
 *
 * ~~~{.c}
 * RBRInstrument *instrument = NULL;
 * RBRInstrument_open(&instrument, ...);
 * ~~~
 *
 * *Do not* pass an uninitialized pointer to this constructor! It will be
 * dereferenced, which is undefined behaviour; and if a non-`NULL` value is
 * read, the constructor will think memory has been preallocated and will
 * attempt to write through the pointer.
 *
 * ~~~{.c}
 * /​* Don't do this! *​/
 * RBRInstrument *instrument;
 * RBRInstrument_open(&instrument, ...);
 * ~~~
 *
 * If you would rather allocate this memory yourself (perhaps statically), you
 * can use the size of RBRInstrument to inform your allocation then pass a
 * pointer to that memory.
 *
 * For example:
 *
 * ~~~{.c}
 * RBRInstrument instrumentBuf;
 * RBRInstrument *instrument = &instrumentBuf;
 * RBRInstrument_open(&instrument, ...);
 * ~~~
 *
 * If you pass pre-allocated memory, its contents will be discarded.
 *
 * The \a callbacks structure will be copied into the RBRInstrument structure;
 * no reference to it is retained, so any subsequent modifications will not
 * affect the connection. All callbacks must be given except for
 * RBRInstrumentCallbacks.sample. If any others are given as null pointers,
 * #RBRINSTRUMENT_MISSING_CALLBACK is returned and the instrument connection
 * will not be opened. RBRInstrumentCallbacks.sample is given, then
 * RBRInstrumentCallbacks.sampleBuffer must also be given; if it is not,
 * #RBRINSTRUMENT_MISSING_CALLBACK is returned.
 *
 * Whenever callbacks are called, the data passed to them should be handled
 * immediately. The pointers passed will coincide with buffers within the
 * RBRInstrument instance, and may be overwritten as soon as the callback
 * returns.
 *
 * This library supports 3rd-generation RBR instruments and, to a lesser
 * extent, 2nd-generation instruments. 1st-generation and third-party
 * instruments are not supported. If the library detects an unsupported
 * instrument during connection, #RBRINSTRUMENT_UNSUPPORTED is returned.
 *
 * In the event of any return value other than #RBRINSTRUMENT_SUCCESS, any
 * memory allocated by this constructor is freed. That is, in the event of
 * failure, no cleanup of library resources is required. In the event of a
 * successful result, RBRInstrument_close() should be used to terminate the
 * instrument connection.
 *
 * \param [in,out] instrument the context object to populate
 * \param [in] callbacks the set of callbacks to be used by the connection
 * \param [in] commandTimeout the command timeout in milliseconds
 * \param [in] userData arbitrary user data; useful in callbacks
 * \return #RBRINSTRUMENT_SUCCESS if the instrument was opened successfully
 * \return #RBRINSTRUMENT_ALLOCATION_FAILURE if memory allocation failed
 * \return #RBRINSTRUMENT_MISSING_CALLBACK if a callback was not provided
 * \return #RBRINSTRUMENT_TIMEOUT if an instrument communication timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_UNSUPPORTED if the instrument is unsupported
 * \see RBRInstrument_close()
 */
RBRInstrumentError RBRInstrument_open(RBRInstrument **instrument,
                                      const RBRInstrumentCallbacks *callbacks,
                                      RBRInstrumentDateTime commandTimeout, void *userData);

/**
 * \brief Terminate the instrument connection and release any held resources.
 *
 * Frees the buffer allocated by RBRInstrument_open() if necessary. Does not
 * perform any communication with the instrument.
 *
 * \param [in,out] instrument the instrument connection to terminate
 * \return #RBRINSTRUMENT_SUCCESS if the instrument was closed successfully
 * \see RBRInstrument_open()
 */
RBRInstrumentError RBRInstrument_close(RBRInstrument *instrument);

/**
 * \brief Get the generation of an instrument.
 *
 * \param [in] instrument the instrument connection
 * \return the instrument generation
 */
RBRInstrumentGeneration RBRInstrument_getGeneration(const RBRInstrument *instrument);

/**
 * \brief Get the command timeout.
 *
 * \param [in] instrument the instrument connection
 * \return the command timeout
 * \see RBRInstrument_setCommandTimeout()
 */
RBRInstrumentDateTime RBRInstrument_getCommandTimeout(const RBRInstrument *instrument);

/**
 * \brief Set the command timeout.
 *
 * \param [in,out] instrument the instrument connection
 * \param [in] commandTimeout the new command timeout
 * \see RBRInstrument_getCommandTimeout()
 */
void RBRInstrument_setCommandTimeout(RBRInstrument *instrument,
                                     RBRInstrumentDateTime commandTimeout);

/**
 * \brief Get the pointer to arbitrary user data.
 *
 * Returns whatever arbitrary pointer the user has most recently provided,
 * either via RBRInstrument_open() or RBRInstrument_setUserData().
 *
 * \param [in] instrument the instrument connection
 * \return the arbitrary user data pointer
 * \see RBRInstrument_setUserData()
 */
void *RBRInstrument_getUserData(const RBRInstrument *instrument);

/**
 * \brief Change the arbitrary user data pointer.
 *
 * \param [in,out] instrument the instrument connection
 * \param [in] userData the new user data
 * \see RBRInstrument_getUserData()
 */
void RBRInstrument_setUserData(RBRInstrument *instrument, void *userData);

/**
 * \brief Get the error which resulted from the last instrument command, if
 *        applicable.
 *
 * If the instrument responded with an error or a warning to the last command,
 * this function returns that error. Otherwise, and before any commands have
 * been issued to the instrument, it returns RBRINSTRUMENT_HARDWARE_ERROR_NONE.
 *
 * Note that this information is _not_ recorded by the instrument: it is
 * recorded by the library as responses are parsed. Accordingly, the value will
 * not persist across instrument connections.
 *
 * Error messages can be quite long: in particular, errors E0102 (“invalid
 * command '<unknown-command-name>'”) and E0108 (“invalid argument to command:
 * '<invalid-argument>'”) both include user-provided data. Make sure you
 * perform bounds-checking as necessary when consuming them.
 *
 * \param [in] instrument the instrument connection
 * \return the last error
 * \see RBRInstrument_getLastHardwareErrorMessage() for the error message
 */
RBRInstrumentHardwareError RBRInstrument_getLastHardwareError(const RBRInstrument *instrument);

/**
 * \brief Get the error message which resulted from the last instrument
 *        command, if applicable.
 *
 * If the last instrument command returned #RBRINSTRUMENT_HARDWARE_ERROR and an
 * error message is available, this function returns the verbatim error
 * message. Otherwise, and before any commands have been issued to the
 * instrument, it returns `NULL`.
 *
 * This function differs from RBRInstrumentHardwareError_name() in that it
 * returns the literal message produced by the instrument. This may include
 * instance-specific error details (e.g., in the case of an invalid parameter,
 * exactly which parameter was invalid). However, the enum name is a good
 * fallback for cases where no message is available (e.g., warnings).
 *
 * Note that this information is _not_ recorded by the instrument: it is
 * recorded by the library as responses are parsed. Accordingly, the value will
 * not persist across instrument connections.
 *
 * The buffer into which the return value points will change whenever
 * instrument communication occurs. The message should be considered invalid
 * after making any subsequent calls to the same instrument instance. If you
 * need to retain a copy of the message, you should `strcpy()` it to your own
 * buffer.
 *
 * Error messages can be quite long: in particular, errors E0102 (“invalid
 * command '<unknown-command-name>'”) and E0108 (“invalid argument to command:
 * '<invalid-argument>'”) both include user-provided data. Make sure you
 * perform bounds-checking as necessary when consuming them.
 *
 * \param [in] instrument the instrument connection
 * \return the last error message
 * \see RBRInstrument_getLastHardwareError() for the error number/presence
 */
const char *RBRInstrument_getLastHardwareErrorMessage(const RBRInstrument *instrument);

/* To help keep declarations and documentation organized and discoverable,
 * instrument commands and structures are broken out into individual
 * categorical headers. */
#include "RBRInstrumentCommands.h"

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENT_H */
