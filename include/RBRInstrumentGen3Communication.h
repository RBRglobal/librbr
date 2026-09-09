/**
 * \file RBRInstrumentGen3Communication.h
 *
 * \brief Instrument commands and structures pertaining to the communication
 * interfaces of the instrument.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTGEN3COMMUNICATION_H
#define LIBRBR_RBRINSTRUMENTGEN3COMMUNICATION_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Instrument link types.
 *
 * \see RBRInstrumentGen3_getLink()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/link
 */
typedef enum RBRInstrumentGen3Link
{
    /** USB CDC connectivity. */
    RBRINSTRUMENTGEN3_LINK_USB,
    /** Serial connectivity. */
    RBRINSTRUMENTGEN3_LINK_SERIAL,
    /** Wi-Fi connectivity. */
    RBRINSTRUMENTGEN3_LINK_WIFI,
    /** The number of specific link types. */
    RBRINSTRUMENTGEN3_LINK_COUNT,
    /** An unknown or unrecognized link type. */
    RBRINSTRUMENTGEN3_UNKNOWN_LINK
} RBRInstrumentGen3Link;

/**
 * \brief Get a human-readable string name for a communication link.
 *
 * \param [in] link the communication link
 * \return a string name for the communication link
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen3Link_name(RBRInstrumentGen3Link link);

/**
 * \brief Get the type of connectivity for the instrument connection.
 *
 * \param [in] instrument the instrument connection
 * \param [out] link the link type
 * \return #RBRGEN3_SUCCESS when the setting is successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/link
 */
RBRGen3Error RBRInstrumentGen3_getLink(
    RBRGen3 *instrument,
    RBRInstrumentGen3Link *link);

/**
 * \brief Instrument serial baud rates.
 *
 * Most of these baud rates are unsupported by the instrument, but are included
 * for sake of completeness. Call RBRInstrumentGen3_getBaudRates() to determine
 * which rates are supported by a given instrument.
 *
 * \see RBRInstrumentGen3Serial
 * \see RBRInstrumentGen3_getSerial()
 * \see RBRInstrumentGen3_setSerial()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/serial
 */
typedef enum RBRInstrumentGen3SerialBaudRate
{
    /** None */
    RBRINSTRUMENTGEN3_SERIAL_BAUD_NONE   =       0,
    /** 300 Bd */
    RBRINSTRUMENTGEN3_SERIAL_BAUD_300    = 1 <<  0,
    /** 600 Bd */
    RBRINSTRUMENTGEN3_SERIAL_BAUD_600    = 1 <<  1,
    /** 1,200 Bd */
    RBRINSTRUMENTGEN3_SERIAL_BAUD_1200   = 1 <<  2,
    /** 2,400 Bd */
    RBRINSTRUMENTGEN3_SERIAL_BAUD_2400   = 1 <<  3,
    /** 4,800 Bd */
    RBRINSTRUMENTGEN3_SERIAL_BAUD_4800   = 1 <<  4,
    /** 9,600 Bd */
    RBRINSTRUMENTGEN3_SERIAL_BAUD_9600   = 1 <<  5,
    /** 19,200 Bd */
    RBRINSTRUMENTGEN3_SERIAL_BAUD_19200  = 1 <<  6,
    /** 28,800 Bd */
    RBRINSTRUMENTGEN3_SERIAL_BAUD_28800  = 1 <<  7,
    /** 38,400 Bd */
    RBRINSTRUMENTGEN3_SERIAL_BAUD_38400  = 1 <<  8,
    /** 57,600 Bd */
    RBRINSTRUMENTGEN3_SERIAL_BAUD_57600  = 1 <<  9,
    /** 115,200 Bd */
    RBRINSTRUMENTGEN3_SERIAL_BAUD_115200 = 1 << 10,
    /** 230,400 Bd */
    RBRINSTRUMENTGEN3_SERIAL_BAUD_230400 = 1 << 11,
    /** 460,800 Bd */
    RBRINSTRUMENTGEN3_SERIAL_BAUD_460800 = 1 << 12,
    /** 921,600 Bd */
    RBRINSTRUMENTGEN3_SERIAL_BAUD_921600 = 1 << 13,
    /** Corresponds to the largest baud rate enum value. */
    RBRINSTRUMENTGEN3_SERIAL_BAUD_MAX    = RBRINSTRUMENTGEN3_SERIAL_BAUD_921600
} RBRInstrumentGen3SerialBaudRate;

/**
 * \brief Get a human-readable string name for a baud rate.
 *
 * \param [in] baud the baud rate
 * \return a string name for the baud rate
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen3SerialBaudRate_name(RBRInstrumentGen3SerialBaudRate baud);

/**
 * \brief Instrument serial modes.
 *
 * All modes are 8N1, use no flow control, and are full-duplex unless otherwise
 * noted.
 *
 * \see RBRInstrumentGen3Serial
 * \see RBRInstrumentGen3_getSerial()
 * \see RBRInstrumentGen3_setSerial()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/serial
 */
typedef enum RBRInstrumentGen3SerialMode
{
    /** No serial mode */
    RBRINSTRUMENTGEN3_SERIAL_MODE_NONE          =      0,
    /** RS-232/EIA-232/TIA-232. */
    RBRINSTRUMENTGEN3_SERIAL_MODE_RS232         = 1 << 0,
    /** RS-485/EIA-485/TIA-485. */
    RBRINSTRUMENTGEN3_SERIAL_MODE_RS485F        = 1 << 1,
    /** RS-485/EIA-485/TIA-485 (half-duplex). Unimplemented by the logger. */
    RBRINSTRUMENTGEN3_SERIAL_MODE_RS485H        = 1 << 2,
    /** 0-3.3V logic, idle high. */
    RBRINSTRUMENTGEN3_SERIAL_MODE_UART          = 1 << 3,
    /** 0-3.3V logic, idle low. */
    RBRINSTRUMENTGEN3_SERIAL_MODE_UART_IDLE_LOW = 1 << 4,
    /** Corresponds to the largest UART mode enum value. */
    RBRINSTRUMENTGEN3_SERIAL_MODE_MAX = RBRINSTRUMENTGEN3_SERIAL_MODE_UART_IDLE_LOW
} RBRInstrumentGen3SerialMode;

/**
 * \brief Get a human-readable string name for a serial mode.
 *
 * \param [in] mode the serial mode
 * \return a string name for the serial mode
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen3SerialMode_name(RBRInstrumentGen3SerialMode mode);

/**
 * \brief Instrument `serial` command parameters.
 *
 * \see RBRInstrumentGen3_getSerial()
 * \see RBRInstrumentGen3_setSerial()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/serial
 */
typedef struct RBRInstrumentGen3Serial
{
    /** \brief The baud rate of the instrument. */
    RBRInstrumentGen3SerialBaudRate baudRate;
    /** \brief The serial mode of the instrument. */
    RBRInstrumentGen3SerialMode mode;
    /**
     * \brief Serial baud rates which the instrument can use.
     *
     * Treated as a bit field representation of available baud rates as defined
     * by RBRInstrumentGen3SerialBaudRate. For details, consult
     * [Working with Bit Fields](bitfields.md).
     *
     * \readonly
     *
     * The `serial availablebaudrates` command does not exist on Logger2
     * instruments. RBRInstrumentGen3_getSerial() will populate this field with the
     * baud rates supported by all Logger2 instruments.
     */
    const RBRInstrumentGen3SerialBaudRate availableBaudRates;
    /**
     * \brief Serial modes which the instrument can use.
     *
     * Treated as a bit field representation of available modes as defined by
     * RBRInstrumentGen3SerialMode. For details, consult
     * [Working with Bit Fields](bitfields.md).
     *
     * \readonly
     *
     * The `serial availablemodes` command does not exist on Logger2
     * instruments. RBRInstrumentGen3_getSerial() will populate this field with the
     * baud rates supported by all Logger2 instruments.
     */
    const RBRInstrumentGen3SerialMode availableModes;
} RBRInstrumentGen3Serial;

/**
 * \brief Retrieve the current and available serial baud rates and modes.
 *
 * \param [in] instrument the instrument connection
 * \param [out] serial the current and available serial parameters
 * \return #RBRGEN3_SUCCESS when the setting is successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen3_setSerial()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/serial
 */
RBRGen3Error RBRInstrumentGen3_getSerial(RBRGen3 *instrument,
                                           RBRInstrumentGen3Serial *serial);

/**
 * \brief Reconfigure the instrument serial baud rate and mode.
 *
 * A hardware error will occur if the baud rate or mode is unsupported by the
 * instrument. See RBRInstrumentGen3Serial.availableBaudRates and
 * RBRInstrumentGen3Serial.availableSerialModes to determine supported
 * rates/modes.
 *
 * The new serial mode and/or baud rate will take effect immediately after the
 * response to this command has been produced. Make sure you alter the
 * configuration of your connection to the instrument correspondingly.
 *
 * \param [in] instrument the instrument connection
 * \param [in] serial the new serial parameters
 * \return #RBRGEN3_SUCCESS when the setting is successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when a value is not supported
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when the baud/mode is invalid
 * \see RBRInstrumentGen3_getSerial()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/serial
 */
RBRGen3Error RBRInstrumentGen3_setSerial(RBRGen3 *instrument,
                                           const RBRInstrumentGen3Serial *serial);

/**
 * \brief Immediately shut down communications and implement any possible
 * power-saving measures.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRGEN3_SUCCESS when the instrument has been put to sleep
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/sleep
 */
RBRGen3Error RBRInstrumentGen3_sleep(RBRGen3 *instrument);

/**
 * \brief The state of the Wi-Fi connection.
 *
 * \see RBRInstrumentGen3WiFi
 */
typedef enum RBRInstrumentGen3WiFiState
{
    /** \brief The Wi-Fi connection is disabled. */
    RBRINSTRUMENTGEN3_WIFI_NA,
    /** \brief The Wi-Fi radio is powered up and ready to communicate. */
    RBRINSTRUMENTGEN3_WIFI_ON,
    /** \brief The Wi-Fi radio is powered down. */
    RBRINSTRUMENTGEN3_WIFI_OFF,
    /** The number of specific states. */
    RBRINSTRUMENTGEN3_WIFI_COUNT,
    /** An unknown or unrecognized state. */
    RBRINSTRUMENTGEN3_UNKNOWN_WIFI
} RBRInstrumentGen3WiFiState;

/**
 * \brief Get a human-readable string name for a Wi-Fi connection state.
 *
 * \param [in] state the Wi-Fi connection state
 * \return a string name for the Wi-Fi connection state
 * \see RBRGen3Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen3WiFiState_name(RBRInstrumentGen3WiFiState state);

/**
 * \brief Instrument `wifi` command parameters.
 *
 * \see RBRInstrumentGen3_getWiFi()
 * \see RBRInstrumentGen3_setWiFi()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/wifi
 */
typedef struct RBRInstrumentGen3WiFi
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
     * \nol2 Will be retrieved as #RBRINSTRUMENTGEN3_UNKNOWN_WIFI.
     */
    const RBRInstrumentGen3WiFiState state;
    /**
     * \brief How long the instrument will wait for a valid command after
     * first powering up the Wi-Fi radio before powering it back down.
     *
     * Specified in whole seconds expressed as milliseconds. Must be in the
     * range 5,000—600,000 (5 seconds to 10 minutes).
     */
    int32_t powerTimeout;
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
     * \nol2 Will be retrieved as #RBRINSTRUMENTGEN3_SERIAL_BAUD_NONE.
     */
    const RBRInstrumentGen3SerialBaudRate baudRate;
} RBRInstrumentGen3WiFi;

/**
 * \brief Retrieve the current instrument Wi-Fi settings.
 *
 * \param [in] instrument the instrument connection
 * \param [out] wifi the current Wi-Fi parameters
 * \return #RBRGEN3_SUCCESS when the setting is successfully read
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the feature is unavailable
 * \see RBRInstrumentGen3_setWiFi()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/wifi
 */
RBRGen3Error RBRInstrumentGen3_getWiFi(RBRGen3 *instrument,
                                         RBRInstrumentGen3WiFi *wifi);

/**
 * \brief Reconfigure the instrument Wi-Fi settings.
 *
 * For Logger3 instruments, this sends the values of RBRInstrumentGen3WiFi.enabled,
 * RBRInstrumentGen3WiFi.powerTimeout, and RBRInstrumentGen3WiFi.commandTimeout. For
 * Logger2 instruments, this sends only the values of
 * RBRInstrumentGen3WiFi.powerTimeout and RBRInstrumentGen3WiFi.commandTimeout as the
 * RBRInstrumentGen3WiFi.enabled parameter does not exist for that generation of
 * instruments.
 *
 * \param [in] instrument the instrument connection
 * \param [out] wifi the new Wi-Fi parameters
 * \return #RBRGEN3_SUCCESS when the setting is successfully written
 * \return #RBRGEN3_TIMEOUT when a timeout occurs
 * \return #RBRGEN3_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN3_HARDWARE_ERROR when the feature is unavailable
 * \return #RBRGEN3_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see RBRInstrumentGen3_getWifi()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/wifi
 */
RBRGen3Error RBRInstrumentGen3_setWiFi(RBRGen3 *instrument,
                                         const RBRInstrumentGen3WiFi *wifi);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTGEN3COMMUNICATION_H */
