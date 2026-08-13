/**
 * \file RBRInstrumentGen4Communication.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for strcmp. */
#include <string.h>

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Internal.h"
#include "RBRInstrumentGen4Communication.h"

const char *RBRInstrumentGen4LinkType_name(RBRInstrumentGen4LinkType linkType)
{
    switch (linkType)
    {
    case RBRINSTRUMENTGEN4_LINK_TYPE_USB:
        return "usb";
    case RBRINSTRUMENTGEN4_LINK_TYPE_SERIAL:
        return "serial";
    /* case RBRINSTRUMENTGEN4_LINK_TYPE_WIFI:
        return "wifi"; */
    case RBRINSTRUMENTGEN4_LINK_TYPE_COUNT:
        return "link type count";
    case RBRINSTRUMENTGEN4_UNKNOWN_LINK_TYPE:
    default:
        return "unknown link type";
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_getLink(RBRInstrumentGen4 *instrument,
                                                RBRInstrumentGen4Link *link)
{
    memset(link, 0, sizeof(RBRInstrumentGen4Link));
    link->type = RBRINSTRUMENTGEN4_UNKNOWN_LINK_TYPE;

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "link"));

    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "type") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENTGEN4_LINK_TYPE_COUNT; i++)
            {
                if (strcmp(RBRInstrumentGen4LinkType_name(i),
                           parameter.value) == 0)
                {
                    link->type = i;
                    break;
                }
            }
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

const char *RBRInstrumentGen4SerialBaudRate_name(RBRInstrumentGen4SerialBaudRate baud)
{
    switch (baud)
    {
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_NONE:
        return "none";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_300:
        return "300";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_600:
        return "600";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_1200:
        return "1200";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_2400:
        return "2400";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_4800:
        return "4800";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_9600:
        return "9600";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_19200:
        return "19200";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_28800:
        return "28800";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_38400:
        return "38400";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_57600:
        return "57600";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_115200:
        return "115200";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_230400:
        return "230400";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_460800:
        return "460800";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_921600:
        return "921600";
    default:
        return "unknown baud";
    }
}

const char *RBRInstrumentGen4SerialMode_name(RBRInstrumentGen4SerialMode mode)
{
    switch (mode)
    {
    case RBRINSTRUMENTGEN4_SERIAL_MODE_NONE:
        return "none";
    case RBRINSTRUMENTGEN4_SERIAL_MODE_RS232:
        return "rs232";
    case RBRINSTRUMENTGEN4_SERIAL_MODE_RS485F:
        return "rs485f";
    case RBRINSTRUMENTGEN4_SERIAL_MODE_RS485H:
        return "rs485h";
    case RBRINSTRUMENTGEN4_SERIAL_MODE_UART:
        return "uart";
    case RBRINSTRUMENTGEN4_SERIAL_MODE_UART_IDLE_LOW:
        return "uart_idlelow";
    default:
        return "unknown serial mode";
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_getSerial(RBRInstrumentGen4 *instrument,
                                                   RBRInstrumentGen4Serial *serial)
{
    memset(serial, 0, sizeof(RBRInstrumentGen4Serial));

    RBRInstrumentGen4SerialBaudRate *availableBaudRates =
        (RBRInstrumentGen4SerialBaudRate *) &serial->availableBaudRates;
    RBRInstrumentGen4SerialMode *availableModes =
        (RBRInstrumentGen4SerialMode *) &serial->availableModes;

    *availableBaudRates = RBRINSTRUMENTGEN4_SERIAL_BAUD_NONE;
    *availableModes = RBRINSTRUMENTGEN4_SERIAL_MODE_NONE;

    RBR_TRY(RBRInstrumentGen4_converse(
        instrument,
        "link serial baudrate mode availablebaudrates availablemodes"
        ));

    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "baudrate") == 0)
        {
            for (int i = RBRINSTRUMENTGEN4_SERIAL_BAUD_NONE + 1;
                 i <= RBRINSTRUMENTGEN4_SERIAL_BAUD_MAX;
                 i <<= 1)
            {
                if (strcmp(RBRInstrumentGen4SerialBaudRate_name(i),
                           parameter.value) == 0)
                {
                    serial->baudRate = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "mode") == 0)
        {
            for (int i = RBRINSTRUMENTGEN4_SERIAL_MODE_NONE + 1;
                 i <= RBRINSTRUMENTGEN4_SERIAL_MODE_MAX;
                 i <<= 1)
            {
                if (strcmp(RBRInstrumentGen4SerialMode_name(i),
                           parameter.value) == 0)
                {
                    serial->mode = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "availablebaudrates") == 0)
        {
            char *nextValue;
            do
            {
                if ((nextValue = strstr(parameter.value, "|")) != NULL)
                {
                    *nextValue = '\0';
                    nextValue++;
                }

                for (int i = RBRINSTRUMENTGEN4_SERIAL_BAUD_NONE + 1;
                     i <= RBRINSTRUMENTGEN4_SERIAL_BAUD_MAX;
                     i <<= 1)
                {
                    if (strcmp(RBRInstrumentGen4SerialBaudRate_name(i),
                               parameter.value) == 0)
                    {
                        *availableBaudRates |= i;
                    }
                }

                parameter.value = nextValue;
            } while (nextValue != NULL);
        }
        else if (strcmp(parameter.key, "availablemodes") == 0)
        {
            char *nextValue;
            do
            {
                if ((nextValue = strstr(parameter.value, "|")) != NULL)
                {
                    *nextValue = '\0';
                    nextValue++;
                }

                for (int i = RBRINSTRUMENTGEN4_SERIAL_MODE_NONE + 1;
                     i <= RBRINSTRUMENTGEN4_SERIAL_MODE_MAX;
                     i <<= 1)
                {
                    if (strcmp(RBRInstrumentGen4SerialMode_name(i),
                               parameter.value) == 0)
                    {
                        *availableModes |= i;
                    }
                }

                parameter.value = nextValue;
            } while (nextValue != NULL);
        }
    }
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setSerial(RBRInstrumentGen4 *instrument,
                                                   const RBRInstrumentGen4Serial *serial)
{
    if (serial->baudRate < 0 || serial->baudRate > RBRINSTRUMENTGEN4_SERIAL_BAUD_MAX || serial->mode < 0 || serial->mode > RBRINSTRUMENTGEN4_SERIAL_MODE_MAX)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrumentGen4_converse(
        instrument,
        "link serial baudrate=%s mode=%s",
        RBRInstrumentGen4SerialBaudRate_name(serial->baudRate),
        RBRInstrumentGen4SerialMode_name(serial->mode));
}

RBRInstrumentGen4Error RBRInstrumentGen4_sleep(RBRInstrumentGen4 *instrument)
{
    RBR_TRY(RBRInstrumentGen4_sendCommand(instrument, "sleep"));
    instrument->lastActivityTime = RBRINSTRUMENTGEN4_NO_ACTIVITY;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

/**
 * L3.5/L4 WiFi interface is To Be Defined as of October 2024.
*/

#if 0

const char *RBRInstrumentGen4WiFiState_name(RBRInstrumentGen4WiFiState state)
{
    switch (state)
    {
    case RBRINSTRUMENTGEN4_WIFI_NA:
        return "n/a";
    case RBRINSTRUMENTGEN4_WIFI_ON:
        return "on";
    case RBRINSTRUMENTGEN4_WIFI_OFF:
        return "off";
    case RBRINSTRUMENTGEN4_WIFI_COUNT:
        return "state count";
    case RBRINSTRUMENTGEN4_UNKNOWN_WIFI:
    default:
        return "unknown state";
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_getWiFi(RBRInstrumentGen4 *instrument,
                                         RBRInstrumentGen4WiFi *wifi)
{
     memset(wifi, 0, sizeof(RBRInstrumentGen4WiFi));

    RBRInstrumentGen4WiFiState *state = (RBRInstrumentGen4WiFiState *)&wifi->state;
    *state = RBRINSTRUMENTGEN4_UNKNOWN_WIFI;

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "wifi"));

    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
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
            for (int i = RBRINSTRUMENTGEN4_WIFI_NA;
                 i < RBRINSTRUMENTGEN4_WIFI_COUNT;
                 i++)
            {
                if (strcmp(RBRInstrumentGen4WiFiState_name(i),
                           parameter.value) == 0)
                {
                    *state = i;
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
            for (int i = RBRINSTRUMENTGEN4_SERIAL_BAUD_NONE + 1;
                 i <= RBRINSTRUMENTGEN4_SERIAL_BAUD_MAX;
                 i <<= 1)
            {
                if (strcmp(RBRInstrumentGen4SerialBaudRate_name(i),
                           parameter.value) == 0)
                {
                    *(RBRInstrumentGen4SerialBaudRate *) &wifi->baudRate = i;
                    break;
                }
            }
        }
    }
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setWiFi(RBRInstrumentGen4 *instrument,
                                         const RBRInstrumentGen4WiFi *wifi)
{
    if (wifi->timeout < 5000
        || wifi->timeout > 600000
        || wifi->timeout % 1000 != 0
        || wifi->commandTimeout < 5000
        || wifi->commandTimeout > 600000
        || wifi->commandTimeout % 1000 != 0)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrumentGen4_converse(
            instrument,
            "wifi enabled=%s timeout=%d commandtimeout=%d",
            wifi->enabled ? "true" : "false",
            wifi->timeout / 1000,
            wifi->commandTimeout / 1000);
}

#endif
