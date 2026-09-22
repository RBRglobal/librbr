/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file RBRGen4.h
 *
 * \brief Interface for simplified communication with RBR instruments.
 *
 * This file contains declarations and constants relevant throughout the
 * library. Command-specific declarations are stored in categorical headers,
 * each of which includes this one and can be included on its own. To include
 * every command header at once, include RBRGen4Commands.h.
 */

#ifndef LIBRBR_RBRGEN4_H
#define LIBRBR_RBRGEN4_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdbool.h>

#include "RBRCommon.h"
#include "RBRGen4HardwareErrors.h"

/**
 * \brief The library name.
 *
 * As shipped by RBR, this builds with the value “libRBR”. Project forks might
 * like to change this at build time to easily identify which library variant
 * is in use. See the Makefile for details.
 */
extern const char *RBRGEN4_LIB_NAME;
/**
 * \brief The library version.
 *
 * As shipped by RBR, this builds with a value based on the contents of the
 * VERSION file. Project forks might also like to override this at build time.
 */
extern const char *RBRGEN4_LIB_VERSION;
/**
 * \brief The library build date.
 *
 * Stored in ISO 8601 format (“YYYY-mm-ddTHH:MM:SS±hhmm”).
 */
extern const char *RBRGEN4_LIB_BUILD_DATE;

/**
 * \brief The library's default size for the buffer storing commands destined
 *        for the instrument.
 *
 * This is a starting point, not a limit or a recommendation. The caller
 * supplies the command buffer and its capacity through RBRGen4Environment and
 * should size it for the commands the application sends: the length of a
 * command varies widely with the number and length of the labels it carries,
 * so an application which configures many channels, groups, or schedules may
 * need considerably more, and one which only polls may need much less. A
 * command which does not fit is refused with #RBRGEN4_COMMAND_TOO_LONG.
 *
 * The label lists sent by the configuration setters and the poll functions
 * are currently assembled in library storage of this size before being placed
 * in the command buffer, so those commands are limited to this length
 * regardless of the buffer supplied.
 */
#define RBRGEN4_COMMAND_BUFFER_DEFAULT 256

/**
 * \brief The library's default size for the buffer storing instrument
 *        responses.
 *
 * This is a starting point, not a limit or a recommendation. The caller
 * supplies the response buffer and its capacity through RBRGen4Environment and
 * should size it for the longest response the application expects, which
 * grows with the number of channels and labels the instrument reports and
 * with the number of channels in a streamed or polled sample. A line which
 * does not fit is discarded without being reported. This does not include
 * download data, which is read directly into a user-managed buffer.
 */
#define RBRGEN4_RESPONSE_BUFFER_DEFAULT 1024

/**
 * \brief The maximum number of characters in a channel type (e.g., “temp09”).
 *
 * Does not include any null terminator.
 */
#define RBRGEN4_CHANNEL_TYPE_MAX 15

/**
 * \brief The maximum number of characters in a channel unit name (e.g., “C”).
 *
 * Does not include any null terminator.
 */
#define RBRGEN4_CHANNEL_UNIT_MAX 15

/**
 * \brief The minimum date and time which the instrument can handle.
 *
 * Specified in milliseconds since the Unix epoch (1970-01-01T00:00:00.000Z).
 * Represents 2000-01-01T00:00:00.000Z.
 */
#define RBRGEN4_DATETIME_MIN 946684800000LL

/**
 * \brief The maximum date and time which the instrument can handle.
 *
 * Specified in milliseconds since the Unix epoch (1970-01-01T00:00:00.000Z).
 * Represents 2099-12-31T23:59:59.000Z.
 */
#define RBRGEN4_DATETIME_MAX 4102444799000LL

/**
 * \brief The maximum number of characters in an instrument part number.
 *
 * Does not include any null terminator.
 */
#define RBRGEN4_PART_NUMBER_MAX 255

/**
 * \brief The maximum number of characters in the instrument model name.
 *
 * Does not include any null terminator.
 */
#define RBRGEN4_ID_MODEL_MAX 14

/**
 * \brief The maximum number of characters in a firmware version.
 *
 * Does not include any null terminator.
 */
#define RBRGEN4_ID_VERSION_MAX 35

/**
 * \brief The maximum number of characters in a firmware Semantic Version.
 *
 * Does not include any null terminator.
 */
#define RBRGEN4_ID_SEMVER_MAX 35

/**
 * \brief The maximum number of characters in a Gen4 command API version.
 *
 * Does not include any null terminator.
 */
#define RBRGEN4_ID_APIVERSION_MAX 8

/**
 * \brief The maximum number of characters in a label.
 *
 * Does not include any null terminator.
 */
#define RBRGEN4_LABEL_NAME_MAX 31

/**
 * \brief One label in a list of labels.
 *
 * Gives the list helpers a single type to work in. Structure members are
 * declared with their own label constants rather than this type; a label type
 * wider than this makes the call a type error rather than a silent misread.
 */
typedef char RBRGen4Label[RBRGEN4_LABEL_NAME_MAX + 1];

/**
 * \brief A list of labels stored in a user provided buffer (#labels).
 *
 * Library functions which read or send a list of labels (a group's channels,
 * a schedule's groups, ...) take one of these instead of storing the list in
 * the object structure. The user sizes #labels for the lists it needs, and
 * may reuse one array across objects and commands.
 */
typedef struct RBRGen4LabelList {
    /** \brief The number of labels #labels can hold. */
    int32_t size;

    /** \brief The number of labels stored in #labels. Never exceeds #size. */
    int32_t len;

    /** \brief User provided array of labels. */
    RBRGen4Label *labels;
} RBRGen4LabelList;

/**
 * A date and time in milliseconds since the Unix epoch
 * (1970-01-01T00:00:00.000Z). Instrument functions operating on time (e.g.,
 * RBRGen4_getClock(), RBRGen4_setClock()) will automatically
 * convert to and from the instrument's string time representation.
 *
 * The valid range for any instrument date/time parameter is
 * 2000-01-01T00:00:00.000Z to 2099-12-31T23:59:59.000Z, inclusive. Passing a
 * value outside of this range will be detected by the library and will cause a
 * #RBRGEN4_INVALID_PARAMETER_VALUE error, not a hardware error.
 */
typedef int64_t RBRGen4DateTime;

/**
 * \brief A periodic parameter.
 *
 * Specified in milliseconds. Generally, parameters of this type must be
 * greater than 0, may not be greater than 86,400,000 (24 hours), and must be
 * multiples of 1,000 when greater than 1,000. See specific parameter
 * documentation for details.
 */
typedef int32_t RBRGen4Period;

/**
 * \brief Errors which can be returned from library functions.
 *
 * Generally speaking, library functions will return error codes in lieu of
 * data values; data will be passed back to the caller via out pointers. This
 * allows for predictable and consistent error checking by the caller.
 */
typedef enum RBRGen4Error {
    /** No error. */
    RBRGEN4_SUCCESS,
    /** A caller-supplied output buffer, such as a download buffer, was too small. */
    RBRGEN4_BUFFER_TOO_SMALL,
    /** The outbound command does not fit the command buffer. */
    RBRGEN4_COMMAND_TOO_LONG,
    /** A response from the instrument did not fit the response buffer. */
    RBRGEN4_RESPONSE_TOO_LONG,
    /** A required callback function was not provided. */
    RBRGEN4_MISSING_CALLBACK,
    /** An unrecoverable error from within a user callback function. */
    RBRGEN4_CALLBACK_ERROR,
    /** A timeout occurred. */
    RBRGEN4_TIMEOUT,
    /** The instrument or command is unsupported by the library. */
    RBRGEN4_UNSUPPORTED,
    /**
     * The physical instrument reported a warning or error.
     *
     * \see RBRGen4_getLastHardwareError()
     */
    RBRGEN4_HARDWARE_ERROR,
    /** A CRC check failed. */
    RBRGEN4_CHECKSUM_ERROR,
    /** The given value is out of bounds or otherwise unsuitable. */
    RBRGEN4_INVALID_PARAMETER_VALUE,
    /**
     * The command succeeded but the user provided buffer could not hold
     * everything the instrument reported; only what fits was stored.
     */
    RBRGEN4_TRUNCATED,
    /**
     * Used internally when the parser encounters a sample.
     *
     * \see RBRGen4_poll()
     * \see RBRGen4_readSample()
     */
    RBRGEN4_SAMPLE,
    /** Communication error. */
    RBRGEN4_COMMUNICATION_ERROR,
    /** The number of specific errors. Should not be used as an error value. */
    RBRGEN4_ERROR_COUNT,
    /** An unknown or unrecognized error. */
    RBRGEN4_UNKNOWN_ERROR
} RBRGen4Error;

/**
 * \brief Get a human-readable string name for a library error.
 *
 * Names are a “friendlier” version of the error enum constant names: they have
 * the `RBRGEN4_` prefix removed, are converted to lower-case, and words
 * are space-separated instead of underscore-separated.
 *
 * For example:
 *
 * ~~~{.c}
 * RBRGen4Error error = ...;
 * if (error != RBRGEN4_SUCCESS)
 * {
 *     fprintf(stderr,
 *             "Encountered an error: %s!\n",
 *             RBRGen4Error_name(error));
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
const char *RBRGen4Error_name(RBRGen4Error error);

/**
 * \brief Possible instrument dataType.
 * dataType is the numeric format used to store data values in the memory for
 * all channels. Options include float32|float64|calfloat64.
 */
typedef enum RBRGen4DataType {
    /** IEEE single precision floating point.
     * Most instruments will use this dataType. */
    RBRGEN4_DATA_TYPE_FLOAT32,
    /** IEEE double precision floating point. Instruments with very high
     * precision may use this format to maintain the necessary level
     * of resolution.  */
    RBRGEN4_DATA_TYPE_FLOAT64,
    /** Same as Float64, but no calibration equation applied. It is presented
     * as a ratio compared to nominal full-scale, so the expected range is
     * nominally 0.0 to 1.0. The full thoretical range is -2.0 to +2.0, but the
     * output of most channels will remain within or close to the expected
     * nominal range.
     */
    RBRGEN4_DATA_TYPE_CALFLOAT64,
    /** The number of specific datatypes. */
    RBRGEN4_DATA_TYPE_COUNT,
    /** An unknown or unrecognized dataType. */
    RBRGEN4_UNKNOWN_DATA_TYPE
} RBRGen4DataType;

/**
 * \brief Get a human-readable string name for a dataType.
 *
 * \param [in] dataType the dataType
 * \return a string name for the dataType
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4DataType_name(RBRGen4DataType dataType);

/**
 * \brief Instrument `instrument outputformat` command parameters.
 *
 * \see RBRGen4_getOutputFormat()
 * \see RBRGen4_setOutputFormat()
 */
typedef struct RBRGen4OutputFormat {
    /**
     * \brief Whether the output begins with “RBR” followed by the
     * instrument's 6-digit serial number.
     *
     * The `sn` parameter. Retrieved as `false` when the instrument does not
     * report it.
     */
    bool sn;
    /**
     * \brief Whether the schedule label appears before the timestamp.
     *
     * The `schedulelabel` parameter. Retrieved as `false` when the instrument
     * does not report it.
     */
    bool scheduleLabel;
    /**
     * \brief Whether a timestamp appears before the data.
     *
     * The `datetime` parameter. Retrieved as `false` when the instrument does
     * not report it.
     */
    bool dateTime;
    /**
     * \brief Whether a cyclic redundancy check appears after the data and
     * immediately before the terminating `\r\n`.
     *
     * The `crc` parameter. Retrieved as `false` when the instrument does not
     * report it.
     */
    bool crc;
    /** \brief The numeric format used to report data values. */
    RBRGen4DataType dataType;
} RBRGen4OutputFormat;

/**
 * \brief The output format assumed before the instrument has been asked.
 *
 * \see RBRGen4_open()
 */
#define RBRGEN4_DEFAULT_OUTPUT_FORMAT              \
    ((RBRGen4OutputFormat) {.sn = false,           \
                            .scheduleLabel = true, \
                            .dateTime = true,      \
                            .crc = false,          \
                            .dataType = RBRGEN4_DATA_TYPE_FLOAT32})

/**
 * \brief Instrument `id4` command parameters.
 *
 * \see RBRGen4_getId4()
 */
typedef struct RBRGen4Id4 {
    /**
     * \brief The instrument model.
     *
     * \readonly
     */
    char model[RBRGEN4_ID_MODEL_MAX + 1];
    /**
     * \brief The instrument firmware version.
     *
     * \readonly
     */
    char fwversion[RBRGEN4_ID_VERSION_MAX + 1];
    /**
     * \brief The instrument firmware version in Semantic Version form.
     *
     * For example, `2.0.0-rc1-10-g148bc5eb1`.
     *
     * \readonly
     */
    char semver[RBRGEN4_ID_SEMVER_MAX + 1];
    /**
     * \brief The version of the Gen4 command API implemented by the firmware.
     *
     * For example, `2.1`. Empty when the parameter is not reported.
     *
     * \readonly
     */
    char apiversion[RBRGEN4_ID_APIVERSION_MAX + 1];
    /** The serial number of the instrument. */
    int32_t sn;
    /** The firmware type of the instrument. */
    int32_t fwtype;
} RBRGen4Id4;

struct RBRGen4;

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
 * \return #RBRGEN4_SUCCESS when the time is successfully retrieved
 * \return #RBRGEN4_CALLBACK_ERROR when an unrecoverable error occurs
 * \see RBRGen4ReadCallback() for details on how the values returned from
 *                                  user callback functions are used
 */
typedef RBRGen4Error (*RBRGen4TimeCallback)(const struct RBRGen4 *conn, RBRGen4DateTime *time);

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
 * \return #RBRGEN4_SUCCESS when the time is successfully retrieved
 * \return #RBRGEN4_CALLBACK_ERROR when an unrecoverable error occurs
 * \see RBRGen4ReadCallback() for details on how the values returned from
 *                                  user callback functions are used
 */
typedef RBRGen4Error (*RBRGen4SleepCallback)(const struct RBRGen4 *conn, RBRGen4DateTime time);

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
 * #RBRGEN4_SUCCESS, the value of \a size should be updated by the
 * callback to reflect the number of bytes written to \a data. When a value
 * other than #RBRGEN4_SUCCESS is returned, any new value of \a size
 * is ignored, as is any data written to \a data.
 *
 * The function should return #RBRGEN4_SUCCESS when data is successfully
 * read from the instrument. In the event of any other value being returned,
 * the calling library function will treat that value as indicative of an
 * error, immediately perform any necessary cleanup, and then return that same
 * value to its caller. On Posix systems, the library will avoid doing anything
 * which might disturb the value of `errno` before returning to user code. It
 * is strongly suggested that #RBRGEN4_TIMEOUT be returned in the event
 * of a timeout and that #RBRGEN4_CALLBACK_ERROR be returned under any
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
 * \param [in,out] size initially, the maximum amount of data which can be
 *                      written to \a data; set by the callback to the number
 *                      of bytes actually written
 * \return #RBRGEN4_SUCCESS when data is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR when an unrecoverable error occurs
 */
typedef RBRGen4Error (*RBRGen4ReadCallback)(const struct RBRGen4 *conn, void *data, int32_t *size);

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
 * \return #RBRGEN4_SUCCESS when the data is successfully written
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR when an unrecoverable error occurs
 * \see RBRGen4ReadCallback() for details on how the values returned from
 *                                  user callback functions are used
 */
typedef RBRGen4Error (*RBRGen4WriteCallback)(const struct RBRGen4 *conn, const void *const data,
                                             int32_t size);

struct RBRGen4Sample;

/**
 * \brief Callback to feed streaming sample data into user code.
 *
 * Library functions will call this user code when a streaming sample has been
 * received.
 *
 * The \a sample pointer will be the same as given via
 * RBRGen4Environment.sampleBuffer. The sample value will be overwritten
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
 * \return #RBRGEN4_SUCCESS when the sample data is successfully consumed
 * \return #RBRGEN4_CALLBACK_ERROR when an unrecoverable error occurs
 */
typedef RBRGen4Error (*RBRGen4SampleCallback)(const struct RBRGen4 *conn,
                                              const struct RBRGen4Sample *const sample);

/**
 * \brief The caller-supplied callbacks and working buffers of a connection.
 *
 * RBRGen4_open() requires all callbacks to be populated except for
 * RBRGen4Environment.sample, which may be `NULL` when undesired.
 *
 * The library never allocates memory. The command and response buffers used
 * by a connection are provided by the caller when the connection is opened
 * and must remain valid until it is closed or they are replaced with
 * RBRGen4_setCommandBuffer() or RBRGen4_setResponseBuffer().
 *
 * The two buffers must not overlap. The command buffer must have room for
 * the longest command, its terminator, and a trailing null byte; a command
 * which does not fit is refused with #RBRGEN4_COMMAND_TOO_LONG.
 *
 * The command buffer holds nothing between commands and may be shared freely
 * between connections which are never used at the same time.
 *
 * The response buffer carries state between commands, so it belongs to one
 * connection at a time. Connections used one at a time may still share it,
 * provided each one calls RBRGen4_resetResponseBuffer() before its first
 * command after another connection has used the buffer. Sharing costs
 * nothing for a connection which only exchanges commands. It does cost a
 * connection which streams: any samples received but not yet delivered when
 * the buffer changes hands are lost. A streaming connection whose every
 * sample matters should own its response buffer.
 *
 * \see RBRGen4_open()
 * \see RBRGen4_setCommandBuffer()
 * \see RBRGen4_setResponseBuffer()
 * \see RBRGEN4_COMMAND_BUFFER_DEFAULT for a starting command buffer size
 * \see RBRGEN4_RESPONSE_BUFFER_DEFAULT for a starting response buffer size
 */
typedef struct RBRGen4Environment {
    /** \brief Callback to get the current platform time in milliseconds. */
    RBRGen4TimeCallback time;

    /** \brief Callback to suspend activity for a fixed amount of time. */
    RBRGen4SleepCallback sleep;

    /** \brief Called to read data from the physical instrument. */
    RBRGen4ReadCallback read;

    /** \brief Called to write data to the physical instrument. */
    RBRGen4WriteCallback write;

    /**
     * \brief Called when streaming sample data has been received.
     *
     * Optional, but requires that RBRGen4Environment.sampleBuffer also be
     * populated.
     */
    RBRGen4SampleCallback sample;

    /**
     * \brief Where to put sample data for consumption by the sample callback.
     *
     * Required only when RBRGen4Environment.sample is populated. The sample's
     * RBRGen4Sample.readings and RBRGen4Sample.size must be set by the
     * caller; they are never changed by the library.
     */
    struct RBRGen4Sample *sampleBuffer;

    /**
     * \brief Storage for commands destined for the instrument.
     *
     * Intentionally not a `char` pointer to discourage the use of `str`
     * functions. Commands may contain binary data and should not be assumed
     * to be null-terminated.
     */
    uint8_t *command;

    /** \brief The capacity of RBRGen4Environment.command in bytes. */
    int32_t commandCapacity;

    /**
     * \brief Storage for data received from the instrument.
     *
     * Intentionally not a `char` pointer to discourage the use of `str`
     * functions. Responses may contain binary data and should not be
     * assumed to be null-terminated. \ref RBRGen4Response.response, when
     * non-`NULL`, provides null-terminated, C-string access to the response.
     */
    uint8_t *response;

    /** \brief The capacity of RBRGen4Environment.response in bytes. */
    int32_t responseCapacity;
} RBRGen4Environment;

/**
 * \brief The types of responses returned by the instrument.
 *
 * Used by RBRGen4Response.
 */
typedef enum RBRGen4ResponseType {
    /** A success indicator or informational response. */
    RBRGEN4_RESPONSE_INFO,
    /** Typically indicates that the command succeeded but with caveats. */
    RBRGEN4_RESPONSE_WARNING,
    /** A command failure. */
    RBRGEN4_RESPONSE_ERROR,
    /** The number of specific types. */
    RBRGEN4_RESPONSE_TYPE_COUNT,
    /** The response has been incorrectly or incompletely populated. */
    RBRGEN4_RESPONSE_UNKNOWN_TYPE
} RBRGen4ResponseType;

/**
 * \brief Get a human-readable string name for a response type.
 *
 * \param [in] type the response type
 * \return a string name for the response type
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4ResponseType_name(RBRGen4ResponseType type);

/**
 * \brief A command response returned by the instrument.
 */
typedef struct RBRGen4Response {
    /**
     * \brief The type of this response: informational, warning, or error.
     *
     * Successful commands, as indicated by the command having returned
     * #RBRGEN4_SUCCESS, may yield informational or warning responses
     * (types #RBRGEN4_RESPONSE_INFO and #RBRGEN4_RESPONSE_WARNING,
     * respectively). Commands having resulted in a hardware error will return
     * #RBRGEN4_HARDWARE_ERROR and yield an error response (type
     * #RBRGEN4_RESPONSE_ERROR). In any other case, the response is
     * unpopulated and its contents are irrelevant (type
     * #RBRGEN4_RESPONSE_UNKNOWN_TYPE).
     *
     * - Informational responses will provide only a response (number as `0`).
     * - Warnings and errors will provide a number and occasionally a response.
     * - Otherwise, the response number will be `0`, and the response `NULL`.
     */
    RBRGen4ResponseType type;
    /**
     * \brief The instrument warning or error number, if applicable.
     *
     * Will be `0` for informational responses. Otherwise, will include the
     * error number indicated by the instrument; e.g., for “ERR-109 feature not
     * available”, this field will contain `109`, aka
     * RBRGEN4_HARDWARE_ERROR_FEATURE_NOT_AVAILABLE.
     */
    RBRGen4HardwareError error;
    /**
     * \brief The response, if available.
     *
     * Will be `NULL` when absent (_not_ a pointer to a 0-length string).
     * Otherwise points to a null-terminated C string.
     */
    char *response;
} RBRGen4Response;

/**
 * \brief Core library context object.
 *
 * Users are strongly discouraged from accessing the fields of this structure
 * directly as layout and field availability maybe unstable from version to
 * version. Getter and setter functions are available for safely reading from
 * and writing to fields where necessary.
 *
 * \see RBRGen4_open() to open an instrument connection
 * \see RBRGen4_close() to close an instrument connection
 */
typedef struct RBRGen4 {
    /**
     * \brief The instrument identifier.
     *
     * \note Cached every time RBRGen4_getId4() is called.
     * \see RBRGen4_getId4()
     */
    RBRGen4Id4 id;

    /**
     * \brief The generation of the instrument.
     *
     * \note Detected while establishing the instrument connection.
     * \see RBRGen4_getGeneration()
     */
    RBRCommonGeneration generation;

    /** \brief The callbacks and buffers used by the connection. */
    RBRGen4Environment environment;

    /**
     * \brief The command timeout in milliseconds.
     *
     * See the Timeouts page of the documentation for details on how the
     * library handles timeouts.
     * \see RBRGen4_getCommandTimeout()
     * \see RBRGen4_setCommandTimeout()
     */
    RBRGen4DateTime commandTimeout;

    /**
     * \brief The poll timeout in milliseconds.
     *
     * RBRGen4_open() sets it to twice the command timeout. See the Timeouts
     * page of the documentation for details on how the library handles
     * timeouts.
     * \see RBRGen4_getPollTimeout()
     * \see RBRGen4_setPollTimeout()
     */
    RBRGen4DateTime pollTimeout;

    /**
     * \brief Arbitrary user data; useful in callbacks.
     * \see RBRGen4_getUserData()
     * \see RBRGen4_setUserData()
     */
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
    RBRGen4DateTime lastActivityTime;

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
    RBRGen4Response response;

    /**
     * \brief The format of the instrument's polled and streamed samples.
     */
    RBRGen4OutputFormat outputFormat;

} RBRGen4;

/**
 * \brief Establish a connection with an instrument and initialize the context.
 * \note Issues the `id4` and `instrument outputformat` commands.
 *
 * “connection” in this library means purely the state tracking
 * and management of an instrument: the underlying physical communication with
 * that instrument (via serial, TCP/IP socket, RFC 1149, whatever) must be
 * managed externally and exposed to the library via callbacks.
 *
 * The library never allocates memory: the caller provides the RBRGen4
 * instance (statically, on the stack, or from a heap of its choosing) and the
 * constructor initializes it in place. Any prior contents are discarded. The
 * caller likewise provides the command and response buffers through
 * \a environment. If either buffer is
 * `NULL`, the command buffer has a capacity of zero or less, or the response
 * buffer cannot hold more than a line terminator, the connection is not
 * opened and #RBRGEN4_INVALID_PARAMETER_VALUE is returned. A command buffer
 * too small for the opening command yields #RBRGEN4_COMMAND_TOO_LONG.
 *
 * The \a environment structure will be copied into the RBRGen4 structure;
 * no reference to it is retained, so any subsequent modifications will not
 * affect the connection. The buffers it points to, however, must remain valid
 * until the connection is closed or they are replaced with
 * RBRGen4_setCommandBuffer() or RBRGen4_setResponseBuffer(). The same
 * applies to RBRGen4Environment.sampleBuffer.
 *
 * All callbacks must be given except for RBRGen4Environment.sample.
 * If any others are given as null pointers, the instrument connection will
 * not be opened, and #RBRGEN4_MISSING_CALLBACK is returned.
 * If RBRGen4Environment.sample is given, then
 * RBRGen4Environment.sampleBuffer must also be given; if it is not,
 * #RBRGEN4_MISSING_CALLBACK is returned.
 *
 * Whenever callbacks are called, the data passed to them should be handled
 * immediately. The pointers passed will coincide with the caller-supplied
 * buffers, and may be overwritten as soon as the callback returns.
 *
 * This constructor supports only 4th-generation RBR instruments. If the
 * constructor detects an unsupported instrument during connection,
 * #RBRGEN4_UNSUPPORTED is returned; a callback failure or a buffer too small
 * for the identification exchange is reported as itself.
 *
 * Until this function has read the instrument's output format, the library
 * assumes it to be #RBRGEN4_DEFAULT_OUTPUT_FORMAT. Samples streamed
 * in any other format while the connection is being opened are not
 * recognised as samples: they are discarded rather than passed to
 * RBRGen4Environment.sample.
 * \see RBRGen4_setOutputFormat()
 *
 * In the event of any return value other than #RBRGEN4_SUCCESS, no cleanup of
 * library resources is required. In the event of a successful result,
 * RBRGen4_close() should be used to terminate the instrument connection.
 *
 * \param [out] conn the context object to populate
 * \param [in] environment the callbacks and buffers to be used by the
 *                         connection
 * \param [in] commandTimeout the command timeout in milliseconds
 * \param [in] userData arbitrary user data; useful in callbacks
 * \return #RBRGEN4_SUCCESS if the instrument was opened successfully
 * \return #RBRGEN4_MISSING_CALLBACK if \a environment or a callback was not
 *         provided
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE if a buffer is missing or empty,
 *         or RBRGen4Environment.sampleBuffer has no readings storage
 * \return #RBRGEN4_COMMAND_TOO_LONG if the command buffer cannot hold the
 *         opening command
 * \return #RBRGEN4_TIMEOUT if an instrument communication timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR if the instrument rejects the opening
 *         command
 * \return #RBRGEN4_UNSUPPORTED if the instrument is unsupported
 * \see RBRGen4_close()
 */
RBRGen4Error RBRGen4_open(RBRGen4 *conn, const RBRGen4Environment *environment,
                          const RBRGen4DateTime commandTimeout, void *userData);

/**
 * \brief Terminate the instrument connection.
 *
 * Clears the connection state. Does not release the caller-provided instance
 * memory and does not perform any communication with the instrument.
 *
 * \param [in,out] conn the instrument connection to terminate
 * \return #RBRGEN4_SUCCESS if the instrument was closed successfully
 * \see RBRGen4_open()
 */
RBRGen4Error RBRGen4_close(RBRGen4 *conn);

/**
 * \brief Replace the command buffer of a connection.
 *
 * The command buffer given to RBRGen4_open() is used until this function
 * replaces it. Nothing is carried over, so this function should be called
 * only between commands. The sizing rules of RBRGen4Environment apply, and
 * the connection keeps its current buffer when the new one is refused.
 *
 * Does not communicate with the instrument.
 *
 * \param [in,out] conn the instrument connection
 * \param [in] command storage for commands destined for the instrument
 * \param [in] capacity the capacity of \a command in bytes
 * \return #RBRGEN4_SUCCESS when the buffer is replaced
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE if the buffer is missing or empty
 * \see RBRGen4Environment for the rules on sizing and sharing buffers
 * \see RBRGen4_setResponseBuffer()
 */
RBRGen4Error RBRGen4_setCommandBuffer(RBRGen4 *conn, uint8_t *command, int32_t capacity);

/**
 * \brief Replace the response buffer of a connection.
 *
 * The response buffer given to RBRGen4_open() is used until this function
 * replaces it. Nothing is carried over: any buffered response data is
 * discarded as by RBRGen4_resetResponseBuffer(), so this function should be
 * called only between commands. The sizing rules of RBRGen4Environment
 * apply, and the connection keeps its current buffer when the new one is
 * refused.
 *
 * Does not communicate with the instrument.
 *
 * \param [in,out] conn the instrument connection
 * \param [in] response storage for data received from the instrument
 * \param [in] capacity the capacity of \a response in bytes
 * \return #RBRGEN4_SUCCESS when the buffer is replaced
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE if the buffer is missing or cannot
 *         hold more than a line terminator
 * \see RBRGen4Environment for the rules on sizing and sharing buffers
 * \see RBRGen4_setCommandBuffer()
 */
RBRGen4Error RBRGen4_setResponseBuffer(RBRGen4 *conn, uint8_t *response, int32_t capacity);

/**
 * \brief Discard any buffered instrument response data.
 *
 * This function must be called after a response buffer has been shared with
 * another RBRGenX connection instance.
 *
 * The response buffer is not always fully consumed during a command-response
 * interaction with an instrument. Unread data can include the prompt or
 * terminator after the last response, data read past the end
 * of that response, and, on a streaming instrument, samples not yet delivered
 * to RBRGen4Environment.sample. Once a response buffer has been shared with
 * another connection, that data may be overwritten, so the response state of
 * the original connection must be reset with this function before trying to
 * converse with an instrument. Otherwise, the next command would parse the
 * other connection's output as its own reply, its own sample, or its own
 * hardware error.
 *
 * Does not communicate with the instrument.
 *
 * \param [in,out] conn the instrument connection
 * \see RBRGen4Environment for the rules on sharing buffers between connections
 */
void RBRGen4_resetResponseBuffer(RBRGen4 *conn);

/**
 * \brief Get the generation of an instrument.
 *
 * \param [in] conn the instrument connection
 * \return the instrument generation
 */
RBRCommonGeneration RBRGen4_getGeneration(const RBRGen4 *conn);

/**
 * \brief Get the command timeout.
 *
 * \param [in] conn the instrument connection
 * \return the command timeout
 * \see RBRGen4_setCommandTimeout()
 */
RBRGen4DateTime RBRGen4_getCommandTimeout(const RBRGen4 *conn);

/**
 * \brief Set the command timeout.
 *
 * \param [in] conn the instrument connection
 * \param [in] commandTimeout the new command timeout
 * \see RBRGen4_getCommandTimeout()
 */
void RBRGen4_setCommandTimeout(RBRGen4 *conn, const RBRGen4DateTime commandTimeout);

/**
 * \brief Get the poll timeout.
 *
 * \param [in] conn the instrument connection
 * \return the poll timeout
 * \see RBRGen4_setPollTimeout()
 */
RBRGen4DateTime RBRGen4_getPollTimeout(const RBRGen4 *conn);

/**
 * \brief Set the poll timeout.
 *
 * \param [in] conn the instrument connection
 * \param [in] pollTimeout the new poll timeout
 * \see RBRGen4_getPollTimeout()
 */
void RBRGen4_setPollTimeout(RBRGen4 *conn, const RBRGen4DateTime pollTimeout);

/**
 * \brief Get the pointer to arbitrary user data.
 *
 * Returns whatever arbitrary pointer the user has most recently provided,
 * either via RBRGen4_open() or RBRGen4_setUserData().
 *
 * \param [in] conn the instrument connection
 * \return the arbitrary user data pointer
 * \see RBRGen4_setUserData()
 */
void *RBRGen4_getUserData(const RBRGen4 *conn);

/**
 * \brief Change the arbitrary user data pointer.
 *
 * \param [in] conn the instrument connection
 * \param [in] userData the new user data
 * \see RBRGen4_getUserData()
 */
void RBRGen4_setUserData(RBRGen4 *conn, void *userData);

/**
 * \brief Get the error which resulted from the last instrument command, if
 *        applicable.
 *
 * If the instrument responded with an error or a warning to the last command,
 * this function returns that error. Otherwise, and before any commands have
 * been issued to the instrument, it returns RBRGEN4_HARDWARE_ERROR_NONE.
 *
 * Note that this information is _not_ recorded by the instrument: it is
 * recorded by the library as responses are parsed. Accordingly, the value will
 * not persist across instrument connections.
 *
 * Error messages can be quite long: in particular, errors ERR-102 (“invalid
 * command '<unknown-command-name>'”) and ERR-108 (“invalid argument to command:
 * '<invalid-argument>'”) both include user-provided data. Make sure you
 * perform bounds-checking as necessary when consuming them.
 *
 * \param [in] conn the instrument connection
 * \return the last error
 * \see RBRGen4_getLastHardwareErrorMessage() for the error message
 */
RBRGen4HardwareError RBRGen4_getLastHardwareError(const RBRGen4 *conn);

/**
 * \brief Get the error message which resulted from the last instrument
 *        command, if applicable.
 *
 * If the last instrument command returned #RBRGEN4_HARDWARE_ERROR and an
 * error message is available, this function returns the verbatim error
 * message. Otherwise, and before any commands have been issued to the
 * instrument, it returns `NULL`.
 *
 * This function differs from RBRGen4HardwareError_name() in that it
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
 * Error messages can be quite long: in particular, errors ERR-102 (“invalid
 * command '<unknown-command-name>'”) and ERR-108 (“invalid argument to command:
 * '<invalid-argument>'”) both include user-provided data. Make sure you
 * perform bounds-checking as necessary when consuming them.
 *
 * \param [in] conn the instrument connection
 * \return the last error message
 * \see RBRGen4_getLastHardwareError() for the error number/presence
 */
const char *RBRGen4_getLastHardwareErrorMessage(const RBRGen4 *conn);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN4_H */
