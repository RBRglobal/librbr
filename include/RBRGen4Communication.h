/**
 * \file RBRGen4Communication.h
 *
 * \brief Instrument commands and structures pertaining to the communication
 * interfaces of the instrument.
 *
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830218/Communications
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRGEN4COMMUNICATION_H
#define LIBRBR_RBRGEN4COMMUNICATION_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Instrument link types.
 *
 * \see RBRGen4Link
 * \see RBRGen4_getLink()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830279/link
 */
typedef enum RBRGen4LinkType
{
    /** USB CDC connectivity. */
    RBRGEN4_LINK_TYPE_USB,
    /** Serial connectivity. */
    RBRGEN4_LINK_TYPE_SERIAL,
    /** The number of specific link types. */
    RBRGEN4_LINK_TYPE_COUNT,
    /** An unknown or unrecognized link type. */
    RBRGEN4_UNKNOWN_LINK_TYPE
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
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830279/link
 */
typedef struct RBRGen4Link
{
    /** \brief The type of communication link carrying the connection. */
    RBRGen4LinkType type;
} RBRGen4Link;

/**
 * \brief Get the connectivity of the instrument connection.
 * \note Issues the `link` command.
 *
 * \param [in] conn the instrument connection
 * \param [out] link the link parameters
 * \return #RBRGEN4_SUCCESS when the setting is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830279/link
 */
RBRGen4Error RBRGen4_getLink(RBRGen4 *conn,
                                                RBRGen4Link *link);

/**
 * \brief Instrument serial baud rates.
 *
 * Not every instrument supports every baud rate. Consult
 * RBRGen4LinkSerial.availableBaudRates for the rates a given
 * instrument can use.
 *
 * \see RBRGen4LinkSerial
 * \see RBRGen4_getLinkSerial()
 * \see RBRGen4_setLinkSerial()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/97222817/serial
 */
typedef enum RBRGen4LinkSerialBaudRate
{
    /** An unrecognized baud rate, or none being set. */
    RBRGEN4_LINK_SERIAL_BAUD_NONE    =      0,
    /** 4,800 Bd */
    RBRGEN4_LINK_SERIAL_BAUD_4800    = 1 << 0,
    /** 9,600 Bd */
    RBRGEN4_LINK_SERIAL_BAUD_9600    = 1 << 1,
    /** 19,200 Bd */
    RBRGEN4_LINK_SERIAL_BAUD_19200   = 1 << 2,
    /** 38,400 Bd */
    RBRGEN4_LINK_SERIAL_BAUD_38400   = 1 << 3,
    /** 57,600 Bd */
    RBRGEN4_LINK_SERIAL_BAUD_57600   = 1 << 4,
    /** 115,200 Bd */
    RBRGEN4_LINK_SERIAL_BAUD_115200  = 1 << 5,
    /** 230,400 Bd */
    RBRGEN4_LINK_SERIAL_BAUD_230400  = 1 << 6,
    /** Corresponds to the largest baud rate enum value. */
    RBRGEN4_LINK_SERIAL_BAUD_MAX
        = RBRGEN4_LINK_SERIAL_BAUD_230400
} RBRGen4LinkSerialBaudRate;

/**
 * \brief Get a human-readable string name for a baud rate.
 *
 * \param [in] baud the baud rate
 * \return a string name for the baud rate
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4LinkSerialBaudRate_name(
    RBRGen4LinkSerialBaudRate baud);

/**
 * \brief Instrument serial modes.
 *
 * All modes are 8N1, use no flow control, and are full-duplex unless otherwise
 * noted.
 *
 * \see RBRGen4LinkSerial
 * \see RBRGen4_getLinkSerial()
 * \see RBRGen4_setLinkSerial()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/serial
 */
typedef enum RBRGen4LinkSerialMode
{
    /** An unrecognized serial mode, or none being set. */
    RBRGEN4_LINK_SERIAL_MODE_NONE          =      0,
    /** RS-232/EIA-232/TIA-232. */
    RBRGEN4_LINK_SERIAL_MODE_RS232         = 1 << 0,
    /** RS-485/EIA-485/TIA-485. */
    RBRGEN4_LINK_SERIAL_MODE_RS485F        = 1 << 1,
    /** 0-3.3V logic, idle high. */
    RBRGEN4_LINK_SERIAL_MODE_UART          = 1 << 2,
    /** 0-3.3V logic, idle low. */
    RBRGEN4_LINK_SERIAL_MODE_UART_IDLE_LOW = 1 << 3,
    /** Corresponds to the largest serial mode enum value. */
    RBRGEN4_LINK_SERIAL_MODE_MAX
        = RBRGEN4_LINK_SERIAL_MODE_UART_IDLE_LOW
} RBRGen4LinkSerialMode;

/**
 * \brief Get a human-readable string name for a serial mode.
 *
 * \param [in] mode the serial mode
 * \return a string name for the serial mode
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4LinkSerialMode_name(
    RBRGen4LinkSerialMode mode);

/**
 * \brief Instrument `link serial` command parameters.
 *
 * \see RBRGen4_getLinkSerial()
 * \see RBRGen4_setLinkSerial()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/97222817/serial
 */
typedef struct RBRGen4LinkSerial
{
    /** \brief The baud rate of the instrument. */
    RBRGen4LinkSerialBaudRate baudRate;
    /** \brief The serial mode of the instrument. */
    RBRGen4LinkSerialMode mode;
    /**
     * \brief Baud rates which the instrument can use.
     *
     * Treated as a bit field representation of available baud rates as
     * defined by RBRGen4LinkSerialBaudRate. For details, consult
     * the Working with Bit Fields page of the documentation.
     *
     * \readonly
     */
    RBRGen4LinkSerialBaudRate availableBaudRates;
    /**
     * \brief Serial modes which the instrument can use.
     *
     * Treated as a bit field representation of available modes as defined by
     * RBRGen4LinkSerialMode. For details, consult
     * the Working with Bit Fields page of the documentation.
     *
     * \readonly
     */
    RBRGen4LinkSerialMode availableModes;
} RBRGen4LinkSerial;

/**
 * \brief Retrieve the current and available serial baud rates and modes.
 * \note Issues the `link serial` command.
 *
 * The instrument reports the available baud rates and modes only when they are
 * requested by name, so all four parameters are requested explicitly.
 *
 * \param [in] conn the instrument connection
 * \param [out] serial the current and available serial parameters
 * \return #RBRGEN4_SUCCESS when the setting is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRGen4_setLinkSerial()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/97222817/serial
 */
RBRGen4Error RBRGen4_getLinkSerial(
    RBRGen4 *conn,
    RBRGen4LinkSerial *serial);

/**
 * \brief Reconfigure the instrument serial baud rate and mode.
 * \note Issues the `link serial` command.
 *
 * Every parameter of the command is sent, so \a serial must be fully
 * populated: read the current parameters with
 * RBRGen4_getLinkSerial() and modify them if only one is of
 * interest.
 *
 * A hardware error will occur if the baud rate or mode is unsupported by the
 * instrument. See RBRGen4LinkSerial.availableBaudRates and
 * RBRGen4LinkSerial.availableModes to determine supported
 * rates/modes.
 *
 * The new serial mode and/or baud rate will take effect immediately after the
 * response to this command has been produced. Make sure you alter the
 * configuration of your connection to the instrument correspondingly.
 *
 * \param [in] conn the instrument connection
 * \param [in] serial the new serial parameters
 * \return #RBRGEN4_SUCCESS when the setting is successfully written
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when a value is not supported, or another
 *                                      hardware error occurs
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the baud rate or
 *                                                   mode is not a real value
 * \see RBRGen4_getLinkSerial()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/97222817/serial
 */
RBRGen4Error RBRGen4_setLinkSerial(
    RBRGen4 *conn,
    const RBRGen4LinkSerial *serial);

/**
 * \brief Immediately shut down communications and implement any possible
 * power-saving measures.
 * \note Issues the `sleep` command.
 *
 * Any scheduled sampling activity is not affected.
 * The `sleep` command does not attempt to power down a USB link, because there
 * is always enough power available via USB to run the logger's basic functions;
 * sensor channels used for a `poll` command will still be shut down.
 *
 * \param [in] conn the instrument connection
 * \return #RBRGEN4_SUCCESS when the instrument has been put to sleep
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828543/sleep
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828337/Timeouts+output+blanking+and+power+saving
 */
RBRGen4Error RBRGen4_sleep(RBRGen4 *conn);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN4COMMUNICATION_H */
