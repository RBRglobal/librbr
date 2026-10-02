/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file RBRGen3.h
 *
 * \brief Interface for simplified communication with RBR instruments.
 *
 * This file contains declarations and constants relevant throughout the
 * library. Command-specific declarations are stored in categorical headers,
 * each of which includes this one and can be included on its own. To include
 * every command header at once, include RBRGen3Commands.h.
 */

#ifndef LIBRBR_RBRGEN3_H
#define LIBRBR_RBRGEN3_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdbool.h>

#include "RBRCommon.h"
#include "RBRGen3HardwareErrors.h"

/**
 * \brief The library name.
 *
 * As shipped by RBR, this builds with the value “libRBR”. Project forks might
 * like to change this at build time to easily identify which library variant
 * is in use. See the Makefile for details.
 */
extern const char *RBRGEN3_LIB_NAME;
/**
 * \brief The library version.
 *
 * As shipped by RBR, this builds with a value based on the output of the `git
 * describe --dirty` command where possible, or else the contents of the
 * VERSION file. Project forks might also like to override this at build time.
 *
 * For tagged, released library versions, this will be a string like “1.3.0”.
 *
 * For development versions where Git and the library's .git directory are
 * available, this could be a string like “1.3.0-67-g0a1b2c3-dirty”. See `man
 * git-describe` for details, particularly the EXAMPLES section. Note that any
 * leading “v” is stripped from the tag name for consistency with the contents
 * of the VERSION file.
 */
extern const char *RBRGEN3_LIB_VERSION;
/**
 * \brief The library build date.
 *
 * \deprecated As of libRBR v1.3.0, this builds with the value “unknown” to
 *             support deterministic builds.
 */
extern const char *RBRGEN3_LIB_BUILD_DATE;

/**
 * \brief A reasonable size for the buffer storing commands destined for the instrument.
 *
 * \note See RBRGen3Environment.commandCapacity for how to size the buffer
 */
#define RBRGEN3_COMMAND_BUFFER_DEFAULT 120

/**
 * \brief A reasonable size for the buffer storing instrument responses.
 *
 * \note See RBRGen3Environment.responseCapacity for how to size the buffer
 */
#define RBRGEN3_RESPONSE_BUFFER_DEFAULT 1024

/**
 * \brief The maximum number of characters in a channel name (e.g.,
 * “Temperature”).
 *
 * Does not include any null terminator.
 */
#define RBRGEN3_CHANNEL_NAME_MAX 31

/**
 * \brief The maximum number of characters in a channel type (e.g., “temp09”).
 *
 * Does not include any null terminator.
 */
#define RBRGEN3_CHANNEL_TYPE_MAX 11

/**
 * \brief The maximum number of characters in a channel unit name (e.g., “C”).
 *
 * Does not include any null terminator.
 */
#define RBRGEN3_CHANNEL_UNIT_MAX 12

/**
 * \brief The maximum number of characters in a channel label.
 *
 * Does not include any null terminator. Must be at least 4 (to hold “none”,
 * the default value).
 */
#define RBRGEN3_CHANNEL_LABEL_MAX 31

/** \brief A channel label as a null-terminated C string. */
typedef char RBRGen3Label[RBRGEN3_CHANNEL_LABEL_MAX + 1];

/**
 * \brief The minimum date and time which the instrument can handle.
 *
 * Specified in milliseconds since the Unix epoch (1970-01-01T00:00:00.000Z).
 * Represents 2000-01-01T00:00:00.000Z.
 */
#define RBRGEN3_DATETIME_MIN 946684800000LL

/**
 * \brief The minimum date and time which the instrument can handle.
 *
 * Specified in milliseconds since the Unix epoch (1970-01-01T00:00:00.000Z).
 * Represents 2099-12-31T23:59:59.000Z.
 */
#define RBRGEN3_DATETIME_MAX 4102444799000LL

/**
 * \brief The maximum number of characters in the instrument model name.
 *
 * Does not include any null terminator.
 */
#define RBRGEN3_ID_MODEL_MAX 14

/**
 * \brief The maximum number of characters in the instrument firmware version.
 *
 * Does not include any null terminator.
 */
#define RBRGEN3_ID_VERSION_MAX 7

/**
 * \brief The maximum number of characters in the instrument mode.
 *
 * Does not include any null terminator.
 */
#define RBRGEN3_ID_MODE_MAX 10

/**
 * A date and time in milliseconds since the Unix epoch
 * (1970-01-01T00:00:00.000Z). Instrument functions operating on time (e.g.,
 * RBRGen3_getClock(), RBRGen3_setClock()) will automatically
 * convert to and from the instrument's string time representation.
 *
 * The valid range for any instrument date/time parameter is
 * 2000-01-01T00:00:00.000Z to 2099-12-31T23:59:59.000Z, inclusive. Passing a
 * value outside of this range will be detected by the library and will cause a
 * #RBRGEN3_INVALID_PARAMETER_VALUE error, not a hardware error.
 */
typedef int64_t RBRGen3DateTime;

/**
 * \brief A periodic parameter.
 *
 * Specified in milliseconds. Generally, parameters of this type must be
 * greater than 0, may not be greater than 86,400,000 (24 hours), and must be
 * multiples of 1,000 when greater than 1,000. See specific parameter
 * documentation for details.
 */
typedef int32_t RBRGen3Period;

/**
 * \brief Errors which can be returned from library functions.
 *
 * Generally speaking, library functions will return error codes in lieu of
 * data values; data will be passed back to the caller via out pointers. This
 * allows for predictable and consistent error checking by the caller.
 */
typedef enum RBRGen3Error {
    /** No error. */
    RBRGEN3_SUCCESS,
    /**
     * A caller-supplied output buffer was too small. Unused by Gen3 today;
     * kept for parity with #RBRGEN4_BUFFER_TOO_SMALL.
     */
    RBRGEN3_BUFFER_TOO_SMALL,
    /** The outbound command does not fit the command buffer. */
    RBRGEN3_COMMAND_TOO_LONG,
    /** A response from the instrument did not fit the response buffer. */
    RBRGEN3_RESPONSE_TOO_LONG,
    /** A required callback function was not provided. */
    RBRGEN3_MISSING_CALLBACK,
    /** An unrecoverable error from within a user callback function. */
    RBRGEN3_CALLBACK_ERROR,
    /** A timeout occurred. */
    RBRGEN3_TIMEOUT,
    /** The instrument or command is unsupported by the library. */
    RBRGEN3_UNSUPPORTED,
    /**
     * The physical instrument reported a warning or error.
     *
     * \see RBRGen3_getLastHardwareError()
     */
    RBRGEN3_HARDWARE_ERROR,
    /** A CRC check failed. */
    RBRGEN3_CHECKSUM_ERROR,
    /** The given value is out of bounds or otherwise unsuitable. */
    RBRGEN3_INVALID_PARAMETER_VALUE,
    /**
     * A caller-supplied list was too small to hold everything the instrument
     * reported; only what fits was stored.
     */
    RBRGEN3_TRUNCATED,
    /**
     * Used internally when the parser encounters a sample.
     *
     * \see RBRGen3_fetch()
     * \see RBRGen3_readSample()
     */
    RBRGEN3_SAMPLE,
    /** Communication error. */
    RBRGEN3_COMMUNICATION_ERROR,
    /** The number of specific errors. Should not be used as an error value. */
    RBRGEN3_ERROR_COUNT,
    /** An unknown or unrecognized error. */
    RBRGEN3_UNKNOWN_ERROR,
} RBRGen3Error;

/**
 * \brief Get a human-readable string name for a library error.
 *
 * Names are a “friendlier” version of the error enum constant names: they have
 * the `RBRGEN3_` prefix removed, are converted to lower-case, and words
 * are space-separated instead of underscore-separated.
 *
 * For example:
 *
 * ~~~{.c}
 * RBRGen3Error error = ...;
 * if (error != RBRGEN3_SUCCESS)
 * {
 *     fprintf(stderr,
 *             "Encountered an error: %s!\n",
 *             RBRGen3Error_name(error));
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
const char *RBRGen3Error_name(RBRGen3Error error);

/**
 * \brief Instrument `id` command parameters.
 *
 * Externalized from RBRGen3Other.h to facilitate inclusion by
 * RBRGen3.
 *
 * \see RBRGen3_getId()
 */
typedef struct RBRGen3Id {
    /** The instrument model. */
    char model[RBRGEN3_ID_MODEL_MAX + 1];
    /** The instrument firmware version. */
    char version[RBRGEN3_ID_VERSION_MAX + 1];
    /** The serial number of the instrument. */
    uint32_t serial;
    /** The firmware type of the instrument. */
    uint16_t fwType;
    /** The instrument mode. */
    char mode[RBRGEN3_ID_MODE_MAX + 1];
} RBRGen3Id;

struct RBRGen3;

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
 * \param [in] conn the instrument for which the time is being requested
 * \param [out] time the current platform time in milliseconds
 * \return #RBRGEN3_SUCCESS when the time is successfully retrieved
 * \return #RBRGEN3_CALLBACK_ERROR when an unrecoverable error occurs
 * \see RBRGen3ReadCallback() for details on how the values returned from user callback functions
 *      are used
 */
typedef RBRGen3Error (*RBRGen3TimeCallback)(const struct RBRGen3 *conn, RBRGen3DateTime *time);

/**
 * \brief Callback to suspend instrument activity for a fixed amount of time.
 *
 * Library functions will call this user code when they know the instrument
 * will be unavailable particularly when waking the instrument from sleep.
 *
 * Library functions will call this user code to suspend activity for a fixed amount
 * of time.
 *
 * \param [in] conn the instrument for which sleep is being requested
 * \param [in] time the duration for which a sleep is requested in milliseconds
 * \return #RBRGEN3_SUCCESS when the time is successfully retrieved
 * \return #RBRGEN3_CALLBACK_ERROR when an unrecoverable error occurs
 * \see RBRGen3ReadCallback() for details on how the values returned from user callback functions
 *      are used
 */
typedef RBRGen3Error (*RBRGen3SleepCallback)(const struct RBRGen3 *conn, RBRGen3DateTime time);

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
 * #RBRGEN3_SUCCESS, the value of \a size should be updated by the
 * callback to reflect the number of bytes written to \a data. When a value
 * other than #RBRGEN3_SUCCESS is returned, any new value of \a size
 * is ignored, as is any data written to \a data.
 *
 * The function should return #RBRGEN3_SUCCESS when data is successfully
 * read from the instrument. In the event of any other value being returned,
 * the calling library function will treat that value as indicative of an
 * error, immediately perform any necessary cleanup, and then return that same
 * value to its caller. On Posix systems, the library will avoid doing anything
 * which might disturb the value of `errno` before returning to user code. It
 * is strongly suggested that #RBRGEN3_TIMEOUT be returned in the event
 * of a timeout and that #RBRGEN3_CALLBACK_ERROR be returned under any
 * other circumstance; that way a clear distinction can be made between errors
 * occurring in user code versus library code.
 *
 * Because communication is handled by user code, and because communication
 * only occurs at the behest of the user, it is up to the user to define the
 * semantics of communication timeouts and to implement them. This could be a
 * constant read/write timeout, or a per-connection timeout tied to the
 * instrument by the user data pointer; the library is unopinionated.
 *
 * \param [in] conn the instrument for which data is being requested
 * \param [in,out] data where up to \a size bytes of data can be written
 * \param [in,out] size initially, the maximum amount of data which can be written to \a data; set
 *                      by the callback to the number of bytes actually written
 * \return #RBRGEN3_SUCCESS when data is successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR when an unrecoverable error occurs
 */
typedef RBRGen3Error (*RBRGen3ReadCallback)(const struct RBRGen3 *conn, void *data, int32_t *size);

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
 * \param [in] conn the instrument for which data is being sent
 * \param [in] data the data to be written to the instrument
 * \param [in] size the size of the data given by \a data
 * \return #RBRGEN3_SUCCESS when the data is successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR when an unrecoverable error occurs
 * \see RBRGen3ReadCallback() for details on how the values returned from user callback functions
 *      are used
 */
typedef RBRGen3Error (*RBRGen3WriteCallback)(const struct RBRGen3 *conn, const void *const data,
                                             int32_t size);

struct RBRGen3Sample;

/**
 * \brief Callback to feed streaming sample data into user code.
 *
 * Library functions will call this user code when a streaming sample has been
 * received.
 *
 * The \a sample pointer will be the same as given via
 * RBRGen3Environment.sampleBuffer. The sample value will be overwritten
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
 * \param [in] conn the instrument from which the sample was received
 * \param [in] sample the sample received from the instrument
 * \return #RBRGEN3_SUCCESS when the sample data is successfully consumed
 * \return #RBRGEN3_CALLBACK_ERROR when an unrecoverable error occurs
 */
typedef RBRGen3Error (*RBRGen3SampleCallback)(const struct RBRGen3 *conn,
                                              const struct RBRGen3Sample *const sample);

/**
 * \brief The caller-supplied callbacks and working buffers of a connection.
 *
 * RBRGen3_open() requires all callbacks to be populated except for
 * RBRGen3Environment.sample, which may be `NULL` when undesired.
 *
 * The library never allocates memory. The command and response buffers used
 * by a connection are provided by the caller when the connection is opened
 * and must remain valid until it is closed or they are replaced with
 * RBRGen3_setCommandBuffer() or RBRGen3_setResponseBuffer().
 *
 * The two buffers must not overlap. See RBRGen3Environment.commandCapacity and
 * RBRGen3Environment.responseCapacity for how to size each.
 *
 * The command buffer holds nothing between commands and may be shared freely
 * between connections which are never used at the same time.
 *
 * The response buffer carries state between commands, so it belongs to one
 * connection at a time. Connections used one at a time may still share it,
 * provided each one calls RBRGen3_resetResponseBuffer() before its first
 * command after another connection has used the buffer. Sharing costs
 * nothing for a connection which only exchanges commands. It does cost a
 * connection which streams: any samples received but not yet delivered when
 * the buffer changes hands are lost. A streaming connection whose every
 * sample matters should own its response buffer.
 *
 * \see RBRGen3_open()
 * \see RBRGen3_setCommandBuffer()
 * \see RBRGen3_setResponseBuffer()
 * \see RBRGEN3_COMMAND_BUFFER_DEFAULT
 * \see RBRGEN3_RESPONSE_BUFFER_DEFAULT
 */
typedef struct RBRGen3Environment {
    /** \brief Callback to get the current platform time in milliseconds. */
    RBRGen3TimeCallback time;

    /** \brief Callback to suspend activity for a fixed amount of time. */
    RBRGen3SleepCallback sleep;

    /** \brief Called to read data from the physical instrument. */
    RBRGen3ReadCallback read;

    /** \brief Called to write data to the physical instrument. */
    RBRGen3WriteCallback write;

    /**
     * \brief Called when streaming sample data has been received.
     *
     * Optional, but requires that RBRGen3Environment.sampleBuffer also be
     * populated.
     */
    RBRGen3SampleCallback sample;

    /**
     * \brief Where to put sample data for consumption by the sample callback.
     *
     * Required only when RBRGen3Environment.sample is populated. Its
     * RBRGen3Sample.readings and RBRGen3Sample.size must be set by the
     * caller.
     */
    struct RBRGen3Sample *sampleBuffer;

    /**
     * \brief Storage for commands destined for the instrument.
     *
     * Intentionally not a `char` pointer to discourage the use of `str`
     * functions. Commands may contain binary data and should not be assumed
     * to be null-terminated.
     */
    uint8_t *command;

    /**
     * \brief The capacity of RBRGen3Environment.command in bytes.
     *
     * The buffer must have room for the longest command the application will
     * send, its terminator, and a trailing null byte. A command which
     * does not fit is refused with #RBRGEN3_COMMAND_TOO_LONG before anything is
     * sent. RBRGen3_fetch() and RBRGen3_setPostprocessing() send their channel
     * lists in several writes when they do not fit, so for them each label,
     * with its separator, need only fit on its own.
     *
     * \see RBRGEN3_COMMAND_BUFFER_DEFAULT for a reasonable size
     */
    int32_t commandCapacity;

    /**
     * \brief Storage for data received from the instrument.
     *
     * Intentionally not a `char` pointer to discourage the use of `str`
     * functions. Responses may contain binary data and should not be
     * assumed to be null-terminated. \ref RBRGen3Response.response, when
     * non-`NULL`, provides null-terminated, C-string access to the response.
     */
    uint8_t *response;

    /**
     * \brief The capacity of RBRGen3Environment.response in bytes.
     *
     * The buffer must be large enough to hold the largest command response the
     * application will want to receive, and the longest streamed sample line
     * if the instrument streams. libRBR functions that converse with the
     * instrument fail with #RBRGEN3_RESPONSE_TOO_LONG when the response buffer
     * isn't large enough. An application that wants a clean link after such a
     * failure should flush its transport before the next command.
     *
     * The response buffer does not need to be sized for downloading data, as
     * downloads use their own dedicated buffer.
     *
     * \see RBRGEN3_RESPONSE_BUFFER_DEFAULT for a reasonable size
     */
    int32_t responseCapacity;
} RBRGen3Environment;

/**
 * \brief The types of responses returned by the instrument.
 *
 * Used by RBRGen3Response.
 */
typedef enum RBRGen3ResponseType {
    /** A success indicator or informational response. */
    RBRGEN3_RESPONSE_INFO,
    /** Typically indicates that the command succeeded but with caveats. */
    RBRGEN3_RESPONSE_WARNING,
    /** A command failure. */
    RBRGEN3_RESPONSE_ERROR,
    /** The number of specific types. */
    RBRGEN3_RESPONSE_TYPE_COUNT,
    /** The response has been incorrectly or incompletely populated. */
    RBRGEN3_RESPONSE_UNKNOWN_TYPE,
} RBRGen3ResponseType;

/**
 * \brief Get a human-readable string name for a response type.
 *
 * \param [in] type the response type
 * \return a string name for the response type
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRGen3ResponseType_name(RBRGen3ResponseType type);

/**
 * \brief A command response returned by the instrument.
 */
typedef struct RBRGen3Response {
    /**
     * \brief The type of this response: informational, warning, or error.
     *
     * Successful commands, as indicated by the command having returned
     * #RBRGEN3_SUCCESS, may yield informational or warning responses
     * (types #RBRGEN3_RESPONSE_INFO and #RBRGEN3_RESPONSE_WARNING,
     * respectively). Commands having resulted in a hardware error will return
     * #RBRGEN3_HARDWARE_ERROR and yield an error response (type
     * #RBRGEN3_RESPONSE_ERROR). In any other case, the response is
     * unpopulated and its contents are irrelevant (type
     * #RBRGEN3_RESPONSE_UNKNOWN_TYPE).
     *
     * - Informational responses will provide only a response (number as `0`).
     * - Warnings and errors will provide a number and occasionally a response.
     * - Otherwise, the response number will be `0`, and the response `NULL`.
     */
    RBRGen3ResponseType type;
    /**
     * \brief The instrument warning or error number, if applicable.
     *
     * Will be `0` for informational responses. Otherwise, will include the
     * error number indicated by the instrument; e.g., for “E0109: feature not
     * available”, this field will contain `109`, aka
     * RBRGEN3_HARDWARE_ERROR_FEATURE_NOT_AVAILABLE.
     */
    RBRGen3HardwareError error;
    /**
     * \brief The response, if available.
     *
     * Will be `NULL` when absent (_not_ a pointer to a 0-length string).
     * Otherwise points to a null-terminated C string.
     */
    char *response;
} RBRGen3Response;

/**
 * \brief Core library context object.
 *
 * Users are strongly discouraged from accessing the fields of this structure
 * directly as layout and field availability maybe unstable from version to
 * version. Getter and setter functions are available for safely reading from
 * and writing to fields where necessary.
 *
 * \see RBRGen3_open() to open an instrument connection
 * \see RBRGen3_close() to close an instrument connection
 */
typedef struct RBRGen3 {
    /**
     * \brief The instrument identifier.
     *
     * Cached every time RBRGen3_getId() is called.
     */
    RBRGen3Id id;

    /**
     * \brief The generation of the instrument.
     *
     * Detected while establishing the instrument connection.
     */
    RBRCommonGeneration generation;

    /** \brief The callbacks and buffers used by the connection. */
    RBRGen3Environment environment;

    /**
     * \brief The command timeout in milliseconds.
     *
     * See the Timeouts page of the documentation for details on how the
     * library handles timeouts.
     */
    RBRGen3DateTime commandTimeout;

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
    RBRGen3DateTime lastActivityTime;

    /**
     * \brief The length in bytes of the most recent response.
     *
     * Used when moving the last response out of the parse buffer before
     * receiving new data.
     */
    int32_t lastResponseLength;

    /**
     * \brief The most recent response received from the instrument.
     *
     * After the response parser identifies and terminates a command response,
     * attributes of the response and its beginning position within the
     * response buffer are recorded within this struct.
     */
    RBRGen3Response response;
} RBRGen3;

/**
 * \brief Establish a connection with an instrument and initialize the context.
 *
 * \command{id}
 *
 * What this library calls a “connection” concerns purely the state tracking
 * and management of an instrument: the underlying physical communication with
 * that instrument (via serial, TCP/IP socket, RFC 1149, whatever) must be
 * managed externally and exposed to the library via callbacks.
 *
 * The library never allocates memory: the caller provides the RBRGen3
 * instance (statically, on the stack, or from a heap of its choosing) and the
 * constructor initializes it in place. Any prior contents are discarded. The
 * caller likewise provides the command and response buffers through
 * \a environment. If either buffer is `NULL`, the command buffer has a
 * capacity of zero or less, or the response buffer cannot hold more than a
 * line terminator, the connection is not opened and
 * #RBRGEN3_INVALID_PARAMETER_VALUE is returned. A buffer too small for the
 * opening exchange yields #RBRGEN3_COMMAND_TOO_LONG or
 * #RBRGEN3_RESPONSE_TOO_LONG.
 *
 * The \a environment structure will be copied into the RBRGen3 structure;
 * no reference to it is retained, so any subsequent modifications will not
 * affect the connection. The buffers it points to, however, must remain valid
 * until the connection is closed or they are replaced with
 * RBRGen3_setCommandBuffer() or RBRGen3_setResponseBuffer(). The same
 * applies to RBRGen3Environment.sampleBuffer.
 *
 * All callbacks must be given except for RBRGen3Environment.sample. If any
 * others are given as null pointers, #RBRGEN3_MISSING_CALLBACK is returned and
 * the instrument connection will not be opened. If
 * RBRGen3Environment.sample is given, then RBRGen3Environment.sampleBuffer
 * must also be given; if it is not, #RBRGEN3_MISSING_CALLBACK is returned.
 *
 * Whenever callbacks are called, the data passed to them should be handled
 * immediately. The pointers passed will coincide with the caller-supplied
 * buffers, and may be overwritten as soon as the callback returns.
 *
 * This constructor supports 3rd-generation RBR instruments and, to a lesser
 * extent, 2nd-generation instruments. 1st-generation and third-party
 * instruments are not supported. 4th-generation instruments can be identified,
 * but full support is left to the Gen4 side of the library; see the note below.
 * If the constructor detects an unsupported instrument during connection,
 * #RBRGEN3_UNSUPPORTED is returned.
 *
 * In the event of any return value other than #RBRGEN3_SUCCESS, no cleanup of
 * library resources is required. In the event of a successful result,
 * RBRGen3_close() should be used to terminate the instrument connection.
 *
 * \note When #RBRGEN3_UNSUPPORTED is returned, the connection is not open, but
 * RBRGen3_getGeneration() reports the generation that was detected (e.g., #RBRCOMMON_LOGGER4 for a
 * 4th-generation instrument, or #RBRCOMMON_UNKNOWN_GENERATION if none could be identified).
 * Applications built with both APIs can therefore call RBRGen3_open() and fall back to
 * RBRGen4_open() on #RBRGEN3_UNSUPPORTED. Only RBRGen3_getGeneration() and RBRGen3_close() may be
 * used on the connection in that state.
 *
 * \param [out] conn the context object to populate
 * \param [in] environment the callbacks and buffers to be used by the connection
 * \param [in] commandTimeout the command timeout in milliseconds
 * \param [in] userData arbitrary user data; useful in callbacks
 * \return #RBRGEN3_SUCCESS when the instrument was opened successfully
 * \return #RBRGEN3_MISSING_CALLBACK when \a environment or a callback was not provided
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when a buffer is missing or empty, or
 *         RBRGen3Environment.sampleBuffer has no readings storage
 * \return #RBRGEN3_COMMAND_TOO_LONG when the command buffer cannot hold the opening command
 * \return #RBRGEN3_RESPONSE_TOO_LONG when the response buffer cannot hold the instrument's reply
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_UNSUPPORTED when the instrument is unsupported, does not answer, or answers with
 *         an error
 * \see RBRGen3_close()
 */
RBRGen3Error RBRGen3_open(RBRGen3 *conn, const RBRGen3Environment *environment,
                          RBRGen3DateTime commandTimeout, void *userData);

/**
 * \brief Terminate the instrument connection.
 *
 * Clears the connection state. Does not release the caller-provided instance
 * memory and does not perform any communication with the instrument.
 *
 * \param [in] conn the instrument connection to terminate
 * \return #RBRGEN3_SUCCESS when the instrument was closed successfully
 * \see RBRGen3_open()
 */
RBRGen3Error RBRGen3_close(RBRGen3 *conn);

/**
 * \brief Replace the command buffer of a connection.
 *
 * The command buffer given to RBRGen3_open() is used until this function
 * replaces it. Nothing is carried over, so this function should be called
 * only between commands. The sizing rules of RBRGen3Environment apply, and
 * the connection keeps its current buffer when the new one is refused.
 *
 * Does not communicate with the instrument.
 *
 * \param [in] conn the instrument connection
 * \param [out] command storage for commands destined for the instrument
 * \param [in] capacity the capacity of \a command in bytes
 * \return #RBRGEN3_SUCCESS when the buffer is replaced
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when the buffer is missing or empty
 * \see RBRGen3Environment for the rules on sizing and sharing buffers
 * \see RBRGen3_setResponseBuffer()
 */
RBRGen3Error RBRGen3_setCommandBuffer(RBRGen3 *conn, uint8_t *command, int32_t capacity);

/**
 * \brief Replace the response buffer of a connection.
 *
 * The response buffer given to RBRGen3_open() is used until this function
 * replaces it. Nothing is carried over: any buffered response data is
 * discarded as by RBRGen3_resetResponseBuffer(), so this function should be
 * called only between commands. The sizing rules of RBRGen3Environment
 * apply, and the connection keeps its current buffer when the new one is
 * refused.
 *
 * Does not communicate with the instrument.
 *
 * \param [in] conn the instrument connection
 * \param [out] response storage for data received from the instrument
 * \param [in] capacity the capacity of \a response in bytes
 * \return #RBRGEN3_SUCCESS when the buffer is replaced
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when the buffer is missing or cannot hold more than a
 *         line terminator
 * \see RBRGen3Environment for the rules on sizing and sharing buffers
 * \see RBRGen3_setCommandBuffer()
 */
RBRGen3Error RBRGen3_setResponseBuffer(RBRGen3 *conn, uint8_t *response, int32_t capacity);

/**
 * \brief Discard any buffered instrument response data.
 *
 * This function must be called after a response buffer has been shared with
 * another RBRGen3 connection instance.
 *
 * The response buffer is not always fully consumed during a command-response
 * interaction with an instrument. Unread data can include the prompt or
 * terminator after the last response, data read past the end
 * of that response, and, on a streaming instrument, samples not yet delivered
 * to RBRGen3Environment.sample. Once a response buffer has been shared with
 * another connection, that data may be overwritten, so the response state of
 * the original connection must be reset with this function before trying to
 * converse with an instrument. Otherwise, the next command would parse the
 * other connection's output as its own reply, its own sample, or its own
 * hardware error.
 *
 * Does not communicate with the instrument.
 *
 * \param [in] conn the instrument connection
 * \see RBRGen3Environment for the rules on sharing buffers between connections
 */
void RBRGen3_resetResponseBuffer(RBRGen3 *conn);

/**
 * \brief Get the generation of an instrument.
 *
 * \note Also reports the detected generation after RBRGen3_open() returns #RBRGEN3_UNSUPPORTED.
 *
 * \param [in] conn the instrument connection
 * \return the instrument generation
 */
RBRCommonGeneration RBRGen3_getGeneration(const RBRGen3 *conn);

/**
 * \brief Get the command timeout.
 *
 * \param [in] conn the instrument connection
 * \return the command timeout
 * \see RBRGen3_setCommandTimeout()
 */
RBRGen3DateTime RBRGen3_getCommandTimeout(const RBRGen3 *conn);

/**
 * \brief Set the command timeout.
 *
 * \param [in] conn the instrument connection
 * \param [in] commandTimeout the new command timeout
 * \see RBRGen3_getCommandTimeout()
 */
void RBRGen3_setCommandTimeout(RBRGen3 *conn, RBRGen3DateTime commandTimeout);

/**
 * \brief Get the pointer to arbitrary user data.
 *
 * Returns whatever arbitrary pointer the user has most recently provided,
 * either via RBRGen3_open() or RBRGen3_setUserData().
 *
 * \param [in] conn the instrument connection
 * \return the arbitrary user data pointer
 * \see RBRGen3_setUserData()
 */
void *RBRGen3_getUserData(const RBRGen3 *conn);

/**
 * \brief Change the arbitrary user data pointer.
 *
 * \param [in] conn the instrument connection
 * \param [in] userData the new user data
 * \see RBRGen3_getUserData()
 */
void RBRGen3_setUserData(RBRGen3 *conn, void *userData);

/**
 * \brief Get the error which resulted from the last instrument command, if applicable.
 *
 * If the instrument responded with an error or a warning to the last command,
 * this function returns that error. Otherwise, and before any commands have
 * been issued to the instrument, it returns RBRGEN3_HARDWARE_ERROR_NONE.
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
 * \param [in] conn the instrument connection
 * \return the last error
 * \see RBRGen3_getLastHardwareErrorMessage() for the error message
 */
RBRGen3HardwareError RBRGen3_getLastHardwareError(const RBRGen3 *conn);

/**
 * \brief Get the error message which resulted from the last instrument command, if applicable.
 *
 * If the last instrument command returned #RBRGEN3_HARDWARE_ERROR and an
 * error message is available, this function returns the verbatim error
 * message. Otherwise, and before any commands have been issued to the
 * instrument, it returns `NULL`.
 *
 * This function differs from RBRGen3HardwareError_name() in that it
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
 * \param [in] conn the instrument connection
 * \return the last error message
 * \see RBRGen3_getLastHardwareError() for the error number/presence
 */
const char *RBRGen3_getLastHardwareErrorMessage(const RBRGen3 *conn);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN3_H */
