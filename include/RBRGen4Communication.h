/*
 * Copyright (c) 2018 RBR Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file RBRGen4Communication.h
 *
 * \brief Instrument commands and structures pertaining to the communication
 * interfaces of the instrument.
 */

#ifndef LIBRBR_RBRGEN4COMMUNICATION_H
#define LIBRBR_RBRGEN4COMMUNICATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RBRGen4.h"

/**
 * \brief Instrument link types.
 *
 * \see RBRGen4Link
 * \see RBRGen4_getLink()
 */
typedef enum RBRGen4LinkType {
    /** USB CDC connectivity. */
    RBRGEN4_LINK_TYPE_USB,
    /** Serial connectivity. */
    RBRGEN4_LINK_TYPE_SERIAL,
    /** The number of specific link types. */
    RBRGEN4_LINK_TYPE_COUNT,
    /** An unknown or unrecognized link type. */
    RBRGEN4_UNKNOWN_LINK_TYPE,
} RBRGen4LinkType;

/**
 * \brief Get a human-readable string name for a type of communication link.
 *
 * \param [in] linkType the type of communication link
 * \return a string name for the communication link
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4LinkType_name(RBRGen4LinkType linkType);

/**
 * \brief Instrument `link` command parameters.
 *
 * \see RBRGen4_getLink()
 */
typedef struct RBRGen4Link {
    /** \brief The type of communication link carrying the connection. */
    RBRGen4LinkType type;
} RBRGen4Link;

/**
 * \brief Get the connectivity of the instrument connection.
 *
 * \command{link}
 *
 * \param [in] conn the instrument connection
 * \param [out] link the link parameters
 * \return #RBRGEN4_SUCCESS when the setting is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the
 *         response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument reports a hardware error
 * \see RBRGen4_getLinkSerial()
 */
RBRGen4Error RBRGen4_getLink(RBRGen4 *conn, RBRGen4Link *link);

/**
 * \brief Instrument serial baud rates.
 *
 * \see RBRGen4LinkSerial
 * \see RBRGen4_getLinkSerial()
 * \see RBRGen4_setLinkSerial()
 */
typedef enum RBRGen4LinkSerialBaudRate {
    /** An unrecognized baud rate, or none being set. */
    RBRGEN4_LINK_SERIAL_BAUD_NONE = 0,
    /** 4,800 Bd */
    RBRGEN4_LINK_SERIAL_BAUD_4800 = 1 << 0,
    /** 9,600 Bd */
    RBRGEN4_LINK_SERIAL_BAUD_9600 = 1 << 1,
    /** 19,200 Bd */
    RBRGEN4_LINK_SERIAL_BAUD_19200 = 1 << 2,
    /** 38,400 Bd */
    RBRGEN4_LINK_SERIAL_BAUD_38400 = 1 << 3,
    /** 57,600 Bd */
    RBRGEN4_LINK_SERIAL_BAUD_57600 = 1 << 4,
    /** 115,200 Bd */
    RBRGEN4_LINK_SERIAL_BAUD_115200 = 1 << 5,
    /** 230,400 Bd */
    RBRGEN4_LINK_SERIAL_BAUD_230400 = 1 << 6,
    /** Corresponds to the largest baud rate enum value. */
    RBRGEN4_LINK_SERIAL_BAUD_MAX = RBRGEN4_LINK_SERIAL_BAUD_230400,
} RBRGen4LinkSerialBaudRate;

/**
 * \brief Get a human-readable string name for a baud rate.
 *
 * \param [in] baud the baud rate
 * \return a string name for the baud rate
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4LinkSerialBaudRate_name(RBRGen4LinkSerialBaudRate baud);

/**
 * \brief Instrument serial modes.
 *
 * All modes are 8N1, use no flow control, and are full-duplex unless otherwise
 * noted.
 *
 * \see RBRGen4LinkSerial
 * \see RBRGen4_getLinkSerial()
 * \see RBRGen4_setLinkSerial()
 */
typedef enum RBRGen4LinkSerialMode {
    /** An unrecognized serial mode, or none being set. */
    RBRGEN4_LINK_SERIAL_MODE_NONE = 0,
    /** RS-232/EIA-232/TIA-232. */
    RBRGEN4_LINK_SERIAL_MODE_RS232 = 1 << 0,
    /** RS-485/EIA-485/TIA-485. */
    RBRGEN4_LINK_SERIAL_MODE_RS485F = 1 << 1,
    /** 0-3.3V logic, idle high. */
    RBRGEN4_LINK_SERIAL_MODE_UART = 1 << 2,
    /** 0-3.3V logic, idle low. */
    RBRGEN4_LINK_SERIAL_MODE_UART_IDLE_LOW = 1 << 3,
    /** Corresponds to the largest serial mode enum value. */
    RBRGEN4_LINK_SERIAL_MODE_MAX = RBRGEN4_LINK_SERIAL_MODE_UART_IDLE_LOW,
} RBRGen4LinkSerialMode;

/**
 * \brief Get a human-readable string name for a serial mode.
 *
 * \param [in] mode the serial mode
 * \return a string name for the serial mode
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4LinkSerialMode_name(RBRGen4LinkSerialMode mode);

/**
 * \brief Instrument `link serial` command parameters.
 *
 * \see RBRGen4_getLinkSerial()
 * \see RBRGen4_setLinkSerial()
 */
typedef struct RBRGen4LinkSerial {
    /** \brief The baud rate of the instrument. */
    RBRGen4LinkSerialBaudRate baudRate;
    /** \brief The serial mode of the instrument. */
    RBRGen4LinkSerialMode mode;
} RBRGen4LinkSerial;

/**
 * \brief Retrieve the current serial baud rate and mode.
 *
 * \command{link serial}
 *
 * \param [in] conn the instrument connection
 * \param [out] serial the current serial parameters
 * \return #RBRGEN4_SUCCESS when the setting is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the
 *         response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument reports a hardware error
 * \see RBRGen4_setLinkSerial()
 */
RBRGen4Error RBRGen4_getLinkSerial(RBRGen4 *conn, RBRGen4LinkSerial *serial);

/**
 * \brief Reconfigure the instrument serial baud rate and mode.
 *
 * \command{link serial}
 *
 * Every parameter of the command is sent, so \a serial must be fully
 * populated: read the current parameters with
 * RBRGen4_getLinkSerial() and modify them if only one is of
 * interest.
 *
 * A hardware error will occur if the baud rate or mode is unsupported by the
 * instrument.
 *
 * The new serial mode and/or baud rate will take effect immediately after the
 * response to this command has been produced. Make sure you alter the
 * configuration of your connection to the instrument correspondingly.
 *
 * \param [in] conn the instrument connection
 * \param [in] serial the new serial parameters
 * \return #RBRGEN4_SUCCESS when the setting is successfully written
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_RESPONSE_TOO_LONG when a response does not fit the
 *         response buffer
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when a value is not supported, or another
 *                                      hardware error occurs
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the baud rate or
 *                                                   mode is not a real value
 * \see RBRGen4_getLinkSerial()
 */
RBRGen4Error RBRGen4_setLinkSerial(RBRGen4 *conn, const RBRGen4LinkSerial *serial);

/**
 * \brief Immediately shut down communications and implement any possible
 * power-saving measures.
 *
 * \command{sleep}
 *
 * Any scheduled sampling activity is not affected.
 * The `sleep` command does not attempt to power down a USB link, because there
 * is always enough power available via USB to run the logger's basic functions;
 * sensor channels used for a `poll` command will still be shut down.
 *
 * \param [in] conn the instrument connection
 * \return #RBRGEN4_SUCCESS when the instrument has been put to sleep
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 */
RBRGen4Error RBRGen4_sleep(RBRGen4 *conn);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN4COMMUNICATION_H */
