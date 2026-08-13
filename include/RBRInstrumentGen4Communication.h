/**
 * \file RBRInstrumentGen4Communication.h
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

#ifndef LIBRBR_RBRINSTRUMENTGEN4COMMUNICATION_H
#define LIBRBR_RBRINSTRUMENTGEN4COMMUNICATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RBRInstrumentGen4.h"

/**
 * \brief Instrument link types.
 *
 * \see RBRInstrumentGen4Link
 * \see RBRInstrumentGen4_getLink()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830279/link
 */
typedef enum RBRInstrumentGen4LinkType
{
    /** USB CDC connectivity. */
    RBRINSTRUMENTGEN4_LINK_TYPE_USB,
    /** Serial connectivity. */
    RBRINSTRUMENTGEN4_LINK_TYPE_SERIAL,
    /** Wi-Fi connectivity. */
    /* RBRINSTRUMENTGEN4_LINK_TYPE_WIFI, */
    
    /** The number of specific link types. */
    RBRINSTRUMENTGEN4_LINK_TYPE_COUNT,
    /** An unknown or unrecognized link type. */
    RBRINSTRUMENTGEN4_UNKNOWN_LINK_TYPE
} RBRInstrumentGen4LinkType;

/**
 * \brief Get a human-readable string name for a type of communication link.
 *
 * \param [in] linkType the type of communication link
 * \return a string name for the communication link
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4LinkType_name(RBRInstrumentGen4LinkType linkType);

/**
 * \brief Instrument `link` command parameters.
 *
 * \see RBRInstrumentGen4_getLink()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830279/link
 */
typedef struct RBRInstrumentGen4Link
{
    /** \brief The type of communication link carrying the connection. */
    RBRInstrumentGen4LinkType type;
} RBRInstrumentGen4Link;

/**
 * \brief Get the connectivity of the instrument connection.
 * \note Issues the `link` command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] link the link parameters
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830279/link
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getLink(RBRInstrumentGen4 *instrument,
                                                RBRInstrumentGen4Link *link);

/**
 * \brief Instrument serial baud rates.
 *
 * Most of these baud rates are unsupported by the instrument, but are included
 * for sake of completeness. Call RBRInstrumentGen4_getBaudRates() to determine
 * which rates are supported by a given instrument.
 *
 * \see RBRInstrumentGen4Serial
 * \see RBRInstrumentGen4_getSerial()
 * \see RBRInstrumentGen4_setSerial()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/97222817/serial
 */
typedef enum RBRInstrumentGen4SerialBaudRate
{
    /** None */
    RBRINSTRUMENTGEN4_SERIAL_BAUD_NONE   =       0,
    /** 300 Bd */
    RBRINSTRUMENTGEN4_SERIAL_BAUD_300    = 1 <<  0,
    /** 600 Bd */
    RBRINSTRUMENTGEN4_SERIAL_BAUD_600    = 1 <<  1,
    /** 1,200 Bd */
    RBRINSTRUMENTGEN4_SERIAL_BAUD_1200   = 1 <<  2,
    /** 2,400 Bd */
    RBRINSTRUMENTGEN4_SERIAL_BAUD_2400   = 1 <<  3,
    /** 4,800 Bd */
    RBRINSTRUMENTGEN4_SERIAL_BAUD_4800   = 1 <<  4,
    /** 9,600 Bd */
    RBRINSTRUMENTGEN4_SERIAL_BAUD_9600   = 1 <<  5,
    /** 19,200 Bd */
    RBRINSTRUMENTGEN4_SERIAL_BAUD_19200  = 1 <<  6,
    /** 28,800 Bd */
    RBRINSTRUMENTGEN4_SERIAL_BAUD_28800  = 1 <<  7,
    /** 38,400 Bd */
    RBRINSTRUMENTGEN4_SERIAL_BAUD_38400  = 1 <<  8,
    /** 57,600 Bd */
    RBRINSTRUMENTGEN4_SERIAL_BAUD_57600  = 1 <<  9,
    /** 115,200 Bd */
    RBRINSTRUMENTGEN4_SERIAL_BAUD_115200 = 1 << 10,
    /** 230,400 Bd */
    RBRINSTRUMENTGEN4_SERIAL_BAUD_230400 = 1 << 11,
    /** 460,800 Bd */
    RBRINSTRUMENTGEN4_SERIAL_BAUD_460800 = 1 << 12,
    /** 921,600 Bd */
    RBRINSTRUMENTGEN4_SERIAL_BAUD_921600 = 1 << 13,
    /** Corresponds to the largest baud rate enum value. */
    RBRINSTRUMENTGEN4_SERIAL_BAUD_MAX    = RBRINSTRUMENTGEN4_SERIAL_BAUD_921600
} RBRInstrumentGen4SerialBaudRate;

/**
 * \brief Get a human-readable string name for a baud rate.
 *
 * \param [in] baud the baud rate
 * \return a string name for the baud rate
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4SerialBaudRate_name(RBRInstrumentGen4SerialBaudRate baud);

/**
 * \brief Instrument serial modes.
 *
 * All modes are 8N1, use no flow control, and are full-duplex unless otherwise
 * noted.
 *
 * \see RBRInstrumentGen4Serial
 * \see RBRInstrumentGen4_getSerial()
 * \see RBRInstrumentGen4_setSerial()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/serial
 */
typedef enum RBRInstrumentGen4SerialMode
{
    /** No serial mode */
    RBRINSTRUMENTGEN4_SERIAL_MODE_NONE          =      0,
    /** RS-232/EIA-232/TIA-232. */
    RBRINSTRUMENTGEN4_SERIAL_MODE_RS232         = 1 << 0,
    /** RS-485/EIA-485/TIA-485. */
    RBRINSTRUMENTGEN4_SERIAL_MODE_RS485F        = 1 << 1,
    /** RS-485/EIA-485/TIA-485 (half-duplex). Unimplemented by the logger. */
    RBRINSTRUMENTGEN4_SERIAL_MODE_RS485H        = 1 << 2,
    /** 0-3.3V logic, idle high. */
    RBRINSTRUMENTGEN4_SERIAL_MODE_UART          = 1 << 3,
    /** 0-3.3V logic, idle low. */
    RBRINSTRUMENTGEN4_SERIAL_MODE_UART_IDLE_LOW = 1 << 4,
    /** Corresponds to the largest UART mode enum value. */
    RBRINSTRUMENTGEN4_SERIAL_MODE_MAX = RBRINSTRUMENTGEN4_SERIAL_MODE_UART_IDLE_LOW
} RBRInstrumentGen4SerialMode;

/**
 * \brief Get a human-readable string name for a serial mode.
 *
 * \param [in] mode the serial mode
 * \return a string name for the serial mode
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4SerialMode_name(RBRInstrumentGen4SerialMode mode);

/**
 * \brief Instrument `serial` command parameters.
 *
 * \see RBRInstrumentGen4_getSerial()
 * \see RBRInstrumentGen4_setSerial()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/97222817/serial
 */
typedef struct RBRInstrumentGen4Serial
{
    /** \brief The baud rate of the instrument. */
    RBRInstrumentGen4SerialBaudRate baudRate;
    /** \brief The serial mode of the instrument. */
    RBRInstrumentGen4SerialMode mode;
    /**
     * \brief Serial baud rates which the instrument can use.
     *
     * Treated as a bit field representation of available baud rates as defined
     * by RBRInstrumentGen4SerialBaudRate. For details, consult
     * [Working with Bit Fields](bitfields.md).
     *
     * \readonly
     */
    const RBRInstrumentGen4SerialBaudRate availableBaudRates;
    /**
     * \brief Serial modes which the instrument can use.
     *
     * Treated as a bit field representation of available modes as defined by
     * RBRInstrumentGen4SerialMode. For details, consult
     * [Working with Bit Fields](bitfields.md).
     *
     * \readonly
     */
    const RBRInstrumentGen4SerialMode availableModes;
} RBRInstrumentGen4Serial;

/**
 * \brief Retrieve the current and available serial baud rates and modes.
 * \note Issues the `link serial` command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] serial the current and available serial parameters
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_setSerial()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/97222817/serial
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getSerial(RBRInstrumentGen4 *instrument,
                                           RBRInstrumentGen4Serial *serial);

/**
 * \brief Reconfigure the instrument serial baud rate and mode.
 * \note Issues the `link serial` command.
 *
 * A hardware error will occur if the baud rate or mode is unsupported by the
 * instrument. See RBRInstrumentGen4Serial.availableBaudRates and
 * RBRInstrumentGen4Serial.availableSerialModes to determine supported
 * rates/modes.
 *
 * The new serial mode and/or baud rate will take effect immediately after the
 * response to this command has been produced. Make sure you alter the
 * configuration of your connection to the instrument correspondingly.
 *
 * \param [in] instrument the instrument connection
 * \param [in] serial the new serial parameters
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when a value is not supported
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the baud/mode is invalid
 * \see RBRInstrumentGen4_getSerial()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/97222817/serial
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setSerial(RBRInstrumentGen4 *instrument,
                                            const RBRInstrumentGen4Serial *serial);

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
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the instrument has been put to sleep
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828543/sleep
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828337/Timeouts+output+blanking+and+power+saving
 */
RBRInstrumentGen4Error RBRInstrumentGen4_sleep(RBRInstrumentGen4 *instrument);

/**
 * L3.5/L4 WiFi interface is To Be Defined as of October 2024
 */

#if 0

/**
 * \brief The state of the Wi-Fi connection.
 *
 * \see RBRInstrumentGen4WiFi
 */
typedef enum RBRInstrumentGen4WiFiState
{
    /** \brief The Wi-Fi connection is disabled. */
    RBRINSTRUMENTGEN4_WIFI_NA,
    /** \brief The Wi-Fi radio is powered up and ready to communicate. */
    RBRINSTRUMENTGEN4_WIFI_ON,
    /** \brief The Wi-Fi radio is powered down. */
    RBRINSTRUMENTGEN4_WIFI_OFF,
    /** The number of specific states. */
    RBRINSTRUMENTGEN4_WIFI_COUNT,
    /** An unknown or unrecognized state. */
    RBRINSTRUMENTGEN4_UNKNOWN_WIFI
} RBRInstrumentGen4WiFiState;

/**
 * \brief Get a human-readable string name for a Wi-Fi connection state.
 *
 * \param [in] state the Wi-Fi connection state
 * \return a string name for the Wi-Fi connection state
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4WiFiState_name(RBRInstrumentGen4WiFiState state);

/**
 * \brief Instrument `wifi` command parameters.
 *
 * \see RBRInstrumentGen4_getWiFi()
 * \see RBRInstrumentGen4_setWiFi()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/wifi
 */
typedef struct RBRInstrumentGen4WiFi
{
    /**
     * \brief Enables or disables Wi-Fi connectivity.
     *
     * \nol2 Will be retrieved as `false`.
     */
    bool enabled;
    /**
     * \brief The state of the Wi-Fi radio.
     *
     * \readonly
     *
     * \nol2 Will be retrieved as #RBRINSTRUMENTGEN4_UNKNOWN_WIFI.
     */
    const RBRInstrumentGen4WiFiState state;
    /**
     * \brief How long the instrument will wait for a valid command after
     * first powering up the Wi-Fi radio before powering it back down.
     *
     * Specified in whole seconds expressed as milliseconds. Must be in the
     * range 5,000—600,000 (5 seconds to 10 minutes).
     */
    int32_t timeout;
    /**
     * \brief How long the instrument will wait between commands after the
     * first command before powering down the Wi-Fi radio.
     *
     * Specified in whole seconds expressed as milliseconds. Must be in the
     * range 5,000—600,000 (5 seconds to 10 minutes).
     */
    int32_t commandTimeout;
    /**
     * \brief The speed of the internal connection between the instrument's CPU
     * and the Wi-Fi radio.
     *
     * \readonly
     *
     * \nol2 Will be retrieved as #RBRINSTRUMENTGEN4_SERIAL_BAUD_NONE.
     */
    const RBRInstrumentGen4SerialBaudRate baudRate;
} RBRInstrumentGen4WiFi;

/**
 * \brief Retrieve the current instrument Wi-Fi settings. 
 * L3.5 won't support, L4 supports it.
 *
 * \param [in] instrument the instrument connection
 * \param [out] wifi the current Wi-Fi parameters
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the feature is unavailable
 * \see RBRInstrumentGen4_setWiFi()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/wifi
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getWiFi(RBRInstrumentGen4 *instrument,
                                         RBRInstrumentGen4WiFi *wifi);

/**
 * \brief Reconfigure the instrument Wi-Fi settings.
 * L3.5 won't support, L4 supports it.
 *
 * For Logger3 instruments, this sends the values of RBRInstrumentGen4WiFi.enabled,
 * RBRInstrumentGen4WiFi.timeout, and RBRInstrumentGen4WiFi.commandTimeout. For
 * Logger2 instruments, this sends only the values of
 * RBRInstrumentGen4WiFi.timeout and RBRInstrumentGen4WiFi.commandTimeout as the
 * RBRInstrumentGen4WiFi.enabled parameter does not exist for that generation of
 * instruments.
 *
 * \param [in] instrument the instrument connection
 * \param [out] wifi the new Wi-Fi parameters
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the feature is unavailable
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see RBRInstrumentGen4_getWifi()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/wifi
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setWiFi(RBRInstrumentGen4 *instrument,
                                         const RBRInstrumentGen4WiFi *wifi);

#endif

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTCOMMUNICATION_H */
