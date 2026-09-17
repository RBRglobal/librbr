/**
 * \file RBRGen4Communication.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for strcmp. */
#include <string.h>

#include "RBRGen4.h"
#include "RBRGen4Internal.h"
#include "RBRGen4Communication.h"

const char *RBRGen4LinkType_name(RBRGen4LinkType linkType)
{
    switch (linkType)
    {
    case RBRGEN4_LINK_TYPE_USB:
        return "usb";
    case RBRGEN4_LINK_TYPE_SERIAL:
        return "serial";
    /* case RBRGEN4_LINK_TYPE_WIFI:
        return "wifi"; */
    case RBRGEN4_LINK_TYPE_COUNT:
        return "link type count";
    case RBRGEN4_UNKNOWN_LINK_TYPE:
    default:
        return "unknown link type";
    }
}

RBRGen4Error RBRGen4_getLink(RBRGen4 *conn,
                                                RBRGen4Link *link)
{
    memset(link, 0, sizeof(RBRGen4Link));
    link->type = RBRGEN4_UNKNOWN_LINK_TYPE;

    RBR_TRY(RBRGen4_converse(conn, "link"));

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(conn,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "type") == 0)
        {
            for (int i = 0; i < RBRGEN4_LINK_TYPE_COUNT; i++)
            {
                if (strcmp(RBRGen4LinkType_name(i),
                           parameter.value) == 0)
                {
                    link->type = i;
                    break;
                }
            }
        }
    }

    return RBRGEN4_SUCCESS;
}

const char *RBRGen4LinkSerialBaudRate_name(
    RBRGen4LinkSerialBaudRate baud)
{
    switch (baud)
    {
    case RBRGEN4_LINK_SERIAL_BAUD_4800:
        return "4800";
    case RBRGEN4_LINK_SERIAL_BAUD_9600:
        return "9600";
    case RBRGEN4_LINK_SERIAL_BAUD_19200:
        return "19200";
    case RBRGEN4_LINK_SERIAL_BAUD_38400:
        return "38400";
    case RBRGEN4_LINK_SERIAL_BAUD_57600:
        return "57600";
    case RBRGEN4_LINK_SERIAL_BAUD_115200:
        return "115200";
    case RBRGEN4_LINK_SERIAL_BAUD_230400:
        return "230400";
    case RBRGEN4_LINK_SERIAL_BAUD_NONE:
    default:
        return "none";
    }
}

/**
 * \brief Find the baud rate a response value names.
 *
 * \param [in] value the response value
 * \return the baud rate, or #RBRGEN4_LINK_SERIAL_BAUD_NONE
 */
static RBRGen4LinkSerialBaudRate
RBRGen4LinkSerialBaudRate_parse(
    const char *value)
{
    for (int i = RBRGEN4_LINK_SERIAL_BAUD_NONE + 1;
         i <= RBRGEN4_LINK_SERIAL_BAUD_MAX;
         i <<= 1)
    {
        if (strcmp(RBRGen4LinkSerialBaudRate_name(i), value) == 0)
        {
            return i;
        }
    }

    return RBRGEN4_LINK_SERIAL_BAUD_NONE;
}

const char *RBRGen4LinkSerialMode_name(
    RBRGen4LinkSerialMode mode)
{
    switch (mode)
    {
    case RBRGEN4_LINK_SERIAL_MODE_RS232:
        return "rs232";
    case RBRGEN4_LINK_SERIAL_MODE_RS485F:
        return "rs485f";
    case RBRGEN4_LINK_SERIAL_MODE_UART:
        return "uart";
    case RBRGEN4_LINK_SERIAL_MODE_UART_IDLE_LOW:
        return "uart_idlelow";
    case RBRGEN4_LINK_SERIAL_MODE_NONE:
    default:
        return "none";
    }
}

/**
 * \brief Find the serial mode a response value names.
 *
 * \param [in] value the response value
 * \return the serial mode, or #RBRGEN4_LINK_SERIAL_MODE_NONE
 */
static RBRGen4LinkSerialMode RBRGen4LinkSerialMode_parse(
    const char *value)
{
    for (int i = RBRGEN4_LINK_SERIAL_MODE_NONE + 1;
         i <= RBRGEN4_LINK_SERIAL_MODE_MAX;
         i <<= 1)
    {
        if (strcmp(RBRGen4LinkSerialMode_name(i), value) == 0)
        {
            return i;
        }
    }

    return RBRGEN4_LINK_SERIAL_MODE_NONE;
}

RBRGen4Error RBRGen4_getLinkSerial(
    RBRGen4 *conn,
    RBRGen4LinkSerial *serial)
{
    memset(serial, 0, sizeof(RBRGen4LinkSerial));

    RBR_TRY(RBRGen4_converse(
        conn,
        "link serial baudrate mode availablebaudrates availablemodes"
        ));

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(conn,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "baudrate") == 0)
        {
            serial->baudRate
                = RBRGen4LinkSerialBaudRate_parse(parameter.value);
        }
        else if (strcmp(parameter.key, "mode") == 0)
        {
            serial->mode
                = RBRGen4LinkSerialMode_parse(parameter.value);
        }
        else if (strcmp(parameter.key, "availablebaudrates") == 0)
        {
            char *value = parameter.value;
            while (value != NULL)
            {
                char *nextValue = RBRGen4_splitListValue(value);
                serial->availableBaudRates
                    |= RBRGen4LinkSerialBaudRate_parse(value);

                value = nextValue;
            }
        }
        else if (strcmp(parameter.key, "availablemodes") == 0)
        {
            char *value = parameter.value;
            while (value != NULL)
            {
                char *nextValue = RBRGen4_splitListValue(value);
                serial->availableModes
                    |= RBRGen4LinkSerialMode_parse(value);

                value = nextValue;
            }
        }
    }
    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_setLinkSerial(
    RBRGen4 *conn,
    const RBRGen4LinkSerial *serial)
{
    /* The command takes one baud rate and one mode, so a field carrying
     * several flags is as invalid as one carrying none. */
    if (serial->baudRate <= RBRGEN4_LINK_SERIAL_BAUD_NONE
        || serial->baudRate > RBRGEN4_LINK_SERIAL_BAUD_MAX
        || (serial->baudRate & (serial->baudRate - 1)) != 0
        || serial->mode <= RBRGEN4_LINK_SERIAL_MODE_NONE
        || serial->mode > RBRGEN4_LINK_SERIAL_MODE_MAX
        || (serial->mode & (serial->mode - 1)) != 0)
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGen4_converse(
        conn,
        "link serial baudrate=%s mode=%s",
        RBRGen4LinkSerialBaudRate_name(serial->baudRate),
        RBRGen4LinkSerialMode_name(serial->mode));
}

RBRGen4Error RBRGen4_sleep(RBRGen4 *conn)
{
    RBR_TRY(RBRGen4_sendCommand(conn, "sleep"));
    conn->lastActivityTime = RBRGEN4_NO_ACTIVITY;
    return RBRGEN4_SUCCESS;
}

#if 0

const char *RBRGen4WiFiState_name(RBRGen4WiFiState state)
{
    switch (state)
    {
    case RBRGEN4_WIFI_NA:
        return "n/a";
    case RBRGEN4_WIFI_ON:
        return "on";
    case RBRGEN4_WIFI_OFF:
        return "off";
    case RBRGEN4_WIFI_COUNT:
        return "state count";
    case RBRGEN4_UNKNOWN_WIFI:
    default:
        return "unknown state";
    }
}

RBRGen4Error RBRGen4_getWiFi(RBRGen4 *conn,
                                         RBRGen4WiFi *wifi)
{
     memset(wifi, 0, sizeof(RBRGen4WiFi));

    wifi->state = RBRGEN4_UNKNOWN_WIFI;

    RBR_TRY(RBRGen4_converse(conn, "wifi"));

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(conn,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "enabled") == 0)
        {
            wifi->enabled = (strcmp(parameter.value, "true") == 0);
        }
        else if (strcmp(parameter.key, "state") == 0)
        {
            for (int i = RBRGEN4_WIFI_NA;
                 i < RBRGEN4_WIFI_COUNT;
                 i++)
            {
                if (strcmp(RBRGen4WiFiState_name(i),
                           parameter.value) == 0)
                {
                    wifi->state = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "timeout") == 0)
        {
            wifi->timeout = strtol(parameter.value, NULL, 10) * 1000;
        }
        else if (strcmp(parameter.key, "commandtimeout") == 0)
        {
            wifi->commandTimeout = strtol(parameter.value, NULL, 10) * 1000;
        }
        else if (strcmp(parameter.key, "baudrate") == 0)
        {
            for (int i = RBRGEN4_LINK_SERIAL_BAUD_NONE + 1;
                 i <= RBRGEN4_LINK_SERIAL_BAUD_MAX;
                 i <<= 1)
            {
                if (strcmp(RBRGen4LinkSerialBaudRate_name(i),
                           parameter.value) == 0)
                {
                    wifi->baudRate = i;
                    break;
                }
            }
        }
    }
    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_setWiFi(RBRGen4 *conn,
                                         const RBRGen4WiFi *wifi)
{
    if (wifi->timeout < 5000
        || wifi->timeout > 600000
        || wifi->timeout % 1000 != 0
        || wifi->commandTimeout < 5000
        || wifi->commandTimeout > 600000
        || wifi->commandTimeout % 1000 != 0)
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGen4_converse(
            conn,
            "wifi enabled=%s timeout=%d commandtimeout=%d",
            wifi->enabled ? "true" : "false",
            wifi->timeout / 1000,
            wifi->commandTimeout / 1000);
}

#endif
