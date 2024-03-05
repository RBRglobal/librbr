/**
 * \file RBRInstrumentGen4Communication.h
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

#ifndef LIBRBR_RBRINSTRUMENTGEN4COMMUNICATION_H
#define LIBRBR_RBRINSTRUMENTGEN4COMMUNICATION_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Instrument link types.
 *
 * \see RBRInstrumentGen4_getLink()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/link
 */
typedef enum RBRInstrumentGen4Link
{
    /** USB CDC connectivity. */
    RBRINSTRUMENTGEN4_LINK_USB,
    /** Serial connectivity. */
    RBRINSTRUMENTGEN4_LINK_SERIAL,
    /** Wi-Fi connectivity. */
    RBRINSTRUMENTGEN4_LINK_WIFI,
    /** The number of specific link types. */
    RBRINSTRUMENTGEN4_LINK_COUNT,
    /** An unknown or unrecognized link type. */
    RBRINSTRUMENTGEN4_UNKNOWN_LINK
} RBRInstrumentGen4Link;

/**
 * \brief Get a human-readable string name for a communication link.
 *
 * \param [in] link the communication link
 * \return a string name for the communication link
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4Link_name(RBRInstrumentGen4Link link);

/**
 * \brief Get the type of connectivity for the instrument connection.
 *
 * \param [in] instrument the instrument connection
 * \param [out] link the link type
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/link
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getLink(
    RBRInstrumentGen4 *instrument,
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
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/serial
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
 * \brief Possible levels of the auxiliary output signal during the setup time,
 * data transmission, and hold time.
 *
 * \see RBRInstrument4Aux1
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/serial
 */
typedef enum RBRInstrumentGen4Aux1ActiveState
{
    /** Signal actively driven high. */
    RBRINSTRUMENTGEN4_AUX1ACTIVE_HIGH,
    /** Signal actively driven low. */
    RBRINSTRUMENTGEN4_AUX1ACTIVE_LOW,
    /** The number of active output levels. */
    RBRINSTRUMENTGEN4_AUX1ACTIVE_COUNT,
    /** An unknown or unrecognized active output level. */
    RBRINSTRUMENTGEN4_UNKNOWN_AUX1ACTIVE
} RBRInstrumentGen4Aux1ActiveState;

/**
 * \brief Get a human-readable string name for a signal level of an active
 * auxiliary output.
 *
 * \param [in] aux1_active the signal level
 * \return a string name for the signal level
 * \see RBRInstrumentError_name() for a description of the format of names
 */
const char *RBRInstrumentGen4Aux1ActiveState_name(
    RBRInstrumentGen4Aux1ActiveState aux1_active);

/**
 * \brief The level of the AUX1 signal seen by the external device while the 
 * logger is asleep.
 *
 * \see RBRInstrumentGen4Serial
 * \see RBRInstrumentGen4_getSerial()
 * \see RBRInstrumentGen4_setSerial()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/serial
 */
typedef enum RBRInstrumentGen4Aux1SleepState{
    /** High impedance on AUX1 when logger is asleep. */
    RBRINSTRUMENTGEN4_AUX1SLEEP_TRISTATE,
    /** +5V on AUX1 when logger is asleep. */
    RBRINSTRUMENTGEN4_AUX1SLEEP_HIGH,
    /** -5V on AUX1 when logger is asleep. */
    RBRINSTRUMENTGEN4_AUX1SLEEP_LOW,
    /** THe number of sleep output levels. */
    RBRINSTRUMENTGEN4_AUX1SLEEP_COUNT,
    /** An unknown or unrecognized sleep output level. */
    RBRINSTRUMENTGEN4_UNKNOWN_AUX1SLEEP
}RBRInstrumentGen4Aux1SleepState;

/**
 * \brief Get a human-readable string name for a serial mode.
 *
 * \param [in] aux1_sleep the aux1_sleep state
 * \return a string name for the aux1_sleep state
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4Aux1SleepState_name(RBRInstrumentGen4Aux1SleepState aux1_sleep);

/** \brief configure the behavior of an auxiliary RS-232 output signal AUX1,
 * if the logger is configured to support it. The signal can be used to control
 * an external device such as a modem, but is intended only for the purpose of
 * transmitting streamed data.It is not intended to be, and should not be used
 * as a general purpose flow control signal.
*/
typedef struct RBRInstrumentGen4Aux1{
    /** reports the state of the feature, or optionally enables or disables the 
     * feature as required. When the feature is disabled, the remaining aux1_... 
     * parameters have no effect. The default setting as shipped from the factory 
     * is off.
     */
    bool aux1_state;
    /** available only if in rs232 mode.
     * reports or sets the AUX1 signal set-up time, in milliseconds. When the 
     * logger is sampling and is about to stream data over the serial link, 
     * this is the time for which AUX1 will be set to the active level before 
     * the streaming transmission begins. The valid range of values is 
     * 10...120000 (10ms to 2 minutes); the default value as shipped from the 
     * factory is 1000ms.*/
    RBRInstrumentGen4Period aux1_setup;
    /** available only if in rs232 mode.
     * reports or sets the AUX1 signal hold time, in milliseconds. This is the 
     * time for which AUX1 will be held at the active level after the serial 
     * streaming transmission has finished. The valid range of values is 
     * 10...120000 (10ms to 2 minutes); the default value as shipped from the 
     * factory is 1000ms.
     */
    RBRInstrumentGen4Period aux1_hold;
    /** available only if in rs232 mode.
     * reports or sets the active level of the AUX1 signal seen by the external
     * device throughout the setup, data transmission, and hold phases. The high 
     * and low signal levels are approximately +5V and –5V respectively, 
     * compatible with the RS-232 specification. The default setting as shipped 
     * from the factory is high.
     */
    RBRInstrumentGen4Aux1ActiveState aux1_active;
    /** available only if in rs232 mode.
     * reports or sets the level of the AUX1 signal seen by the external device 
     * while the logger is asleep. In the high and low states the signal is 
     * actively driven to the appropriate level by the logger, which may be 
     * necessary for some external devices. The high and low signal levels are 
     * approximately +5V and –5V respectively,
     * However, these two options cause a large increase in the logger's sleep 
     * current, and will severely impact the available deployment lifetime when 
     * using the logger's internal batteries. In the tristate condition, the 
     * signal is not actively driven, but becomes high impedance. This allows 
     * the logger to maintain a very low sleep current
     */
    RBRInstrumentGen4Aux1SleepState aux1_sleep;
}RBRInstrumentGen4Aux1;

/**
 * \brief Get the behaviour of an auxiliary RS-232 output signal AUX1.
 *
 * \param [in] instrument the instrument connection
 * \param [out] aux1 the auxiliary output signal parameters
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/serial
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getAux1(RBRInstrumentGen4 *instrument,
                                           RBRInstrumentGen4Aux1 *aux1);

/**
 * \brief Configure the behaviour of an auxiliary RS-232 output signal AUX1.
 *
 * \param [in] instrument the instrument connection
 * \param [inout] aux1 the auxiliary output signal parameters
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/serial
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setAux1(RBRInstrumentGen4 *instrument,
                                           RBRInstrumentGen4Aux1 *aux1);                                           

/**
 * \brief Instrument `serial` command parameters.
 *
 * \see RBRInstrumentGen4_getSerial()
 * \see RBRInstrumentGen4_setSerial()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/serial
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
     *
     * The `serial availablemodes` command does not exist on Logger2
     * instruments. RBRInstrumentGen4_getSerial() will populate this field with the
     * baud rates supported by all Logger2 instruments.
     */
    const RBRInstrumentGen4SerialMode availableModes;
} RBRInstrumentGen4Serial;

/**
 * \brief Retrieve the current and available serial baud rates and modes.
 *
 * \param [in] instrument the instrument connection
 * \param [out] serial the current and available serial parameters
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/serial
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getSerial(RBRInstrumentGen4 *instrument,
                                           RBRInstrumentGen4Serial *serial);

/**
 * \brief Reconfigure the instrument serial baud rate and mode.
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
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/serial
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setSerial(RBRInstrumentGen4 *instrument,
                                            const RBRInstrumentGen4Serial *serial);

/**
 * \brief Immediately shut down communications and implement any possible
 * power-saving measures.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the instrument has been put to sleep
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/sleep
 */
RBRInstrumentGen4Error RBRInstrumentGen4_sleep(RBRInstrumentGen4 *instrument);

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



#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTCOMMUNICATION_H */
