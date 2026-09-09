/**
 * \file RBRInstrumentGen3Communication.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for strcmp. */
#include <string.h>

#include "RBRGen3.h"
#include "RBRGen3Internal.h"

const char *RBRInstrumentGen3Link_name(RBRInstrumentGen3Link link)
{
    switch (link)
    {
    case RBRINSTRUMENTGEN3_LINK_USB:
        return "usb";
    case RBRINSTRUMENTGEN3_LINK_SERIAL:
        return "serial";
    case RBRINSTRUMENTGEN3_LINK_WIFI:
        return "wifi";
    case RBRINSTRUMENTGEN3_LINK_COUNT:
        return "link count";
    case RBRINSTRUMENTGEN3_UNKNOWN_LINK:
    default:
        return "unknown link";
    }
}

RBRGen3Error RBRInstrumentGen3_getLink(RBRGen3 *instrument,
                                         RBRInstrumentGen3Link *link)
{
    *link = RBRINSTRUMENTGEN3_UNKNOWN_LINK;

    RBR_TRY(RBRGen3_converse(instrument, "link"));

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true)
    {
        RBRGen3_parseResponse(instrument,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "link") == 0
                 || strcmp(parameter.key, "type") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENTGEN3_LINK_COUNT; i++)
            {
                if (strcmp(RBRInstrumentGen3Link_name(i),
                           parameter.value) == 0)
                {
                    *link = i;
                    break;
                }
            }
        }
    }

    return RBRGEN3_SUCCESS;
}

const char *RBRInstrumentGen3SerialBaudRate_name(RBRInstrumentGen3SerialBaudRate baud)
{
    switch (baud)
    {
    case RBRINSTRUMENTGEN3_SERIAL_BAUD_NONE:
        return "none";
    case RBRINSTRUMENTGEN3_SERIAL_BAUD_300:
        return "300";
    case RBRINSTRUMENTGEN3_SERIAL_BAUD_600:
        return "600";
    case RBRINSTRUMENTGEN3_SERIAL_BAUD_1200:
        return "1200";
    case RBRINSTRUMENTGEN3_SERIAL_BAUD_2400:
        return "2400";
    case RBRINSTRUMENTGEN3_SERIAL_BAUD_4800:
        return "4800";
    case RBRINSTRUMENTGEN3_SERIAL_BAUD_9600:
        return "9600";
    case RBRINSTRUMENTGEN3_SERIAL_BAUD_19200:
        return "19200";
    case RBRINSTRUMENTGEN3_SERIAL_BAUD_28800:
        return "28800";
    case RBRINSTRUMENTGEN3_SERIAL_BAUD_38400:
        return "38400";
    case RBRINSTRUMENTGEN3_SERIAL_BAUD_57600:
        return "57600";
    case RBRINSTRUMENTGEN3_SERIAL_BAUD_115200:
        return "115200";
    case RBRINSTRUMENTGEN3_SERIAL_BAUD_230400:
        return "230400";
    case RBRINSTRUMENTGEN3_SERIAL_BAUD_460800:
        return "460800";
    case RBRINSTRUMENTGEN3_SERIAL_BAUD_921600:
        return "921600";
    default:
        return "unknown baud";
    }
}

const char *RBRInstrumentGen3SerialMode_name(RBRInstrumentGen3SerialMode mode)
{
    switch (mode)
    {
    case RBRINSTRUMENTGEN3_SERIAL_MODE_NONE:
        return "none";
    case RBRINSTRUMENTGEN3_SERIAL_MODE_RS232:
        return "rs232";
    case RBRINSTRUMENTGEN3_SERIAL_MODE_RS485F:
        return "rs485f";
    case RBRINSTRUMENTGEN3_SERIAL_MODE_RS485H:
        return "rs485h";
    case RBRINSTRUMENTGEN3_SERIAL_MODE_UART:
        return "uart";
    case RBRINSTRUMENTGEN3_SERIAL_MODE_UART_IDLE_LOW:
        return "uart_idlelow";
    default:
        return "unknown serial mode";
    }
}

RBRGen3Error RBRInstrumentGen3_getSerial(RBRGen3 *instrument,
                                           RBRInstrumentGen3Serial *serial)
{
    memset(serial, 0, sizeof(RBRInstrumentGen3Serial));

    RBRInstrumentGen3SerialBaudRate *availableBaudRates =
        (RBRInstrumentGen3SerialBaudRate *) &serial->availableBaudRates;
    RBRInstrumentGen3SerialMode *availableModes =
        (RBRInstrumentGen3SerialMode *) &serial->availableModes;

    if (instrument->generation == RBRGEN3_LOGGER2)
    {
        *availableBaudRates = RBRINSTRUMENTGEN3_SERIAL_BAUD_1200
                              | RBRINSTRUMENTGEN3_SERIAL_BAUD_2400
                              | RBRINSTRUMENTGEN3_SERIAL_BAUD_4800
                              | RBRINSTRUMENTGEN3_SERIAL_BAUD_9600
                              | RBRINSTRUMENTGEN3_SERIAL_BAUD_19200
                              | RBRINSTRUMENTGEN3_SERIAL_BAUD_115200;
        *availableModes = RBRINSTRUMENTGEN3_SERIAL_MODE_RS232
                          | RBRINSTRUMENTGEN3_SERIAL_MODE_RS485F
                          | RBRINSTRUMENTGEN3_SERIAL_MODE_UART
                          | RBRINSTRUMENTGEN3_SERIAL_MODE_UART_IDLE_LOW;

        RBR_TRY(RBRGen3_converse(instrument, "serial"));
    }
    else
    {
        *availableBaudRates = RBRINSTRUMENTGEN3_SERIAL_BAUD_NONE;
        *availableModes = RBRINSTRUMENTGEN3_SERIAL_MODE_NONE;

        RBR_TRY(RBRGen3_converse(instrument, "serial all"));
    }

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true)
    {
        RBRGen3_parseResponse(instrument,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "baudrate") == 0)
        {
            for (int i = RBRINSTRUMENTGEN3_SERIAL_BAUD_NONE + 1;
                 i <= RBRINSTRUMENTGEN3_SERIAL_BAUD_MAX;
                 i <<= 1)
            {
                if (strcmp(RBRInstrumentGen3SerialBaudRate_name(i),
                           parameter.value) == 0)
                {
                    serial->baudRate = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "mode") == 0)
        {
            for (int i = RBRINSTRUMENTGEN3_SERIAL_MODE_NONE + 1;
                 i <= RBRINSTRUMENTGEN3_SERIAL_MODE_MAX;
                 i <<= 1)
            {
                if (strcmp(RBRInstrumentGen3SerialMode_name(i),
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

                for (int i = RBRINSTRUMENTGEN3_SERIAL_BAUD_NONE + 1;
                     i <= RBRINSTRUMENTGEN3_SERIAL_BAUD_MAX;
                     i <<= 1)
                {
                    if (strcmp(RBRInstrumentGen3SerialBaudRate_name(i),
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

                for (int i = RBRINSTRUMENTGEN3_SERIAL_MODE_NONE + 1;
                     i <= RBRINSTRUMENTGEN3_SERIAL_MODE_MAX;
                     i <<= 1)
                {
                    if (strcmp(RBRInstrumentGen3SerialMode_name(i),
                               parameter.value) == 0)
                    {
                        *availableModes |= i;
                    }
                }

                parameter.value = nextValue;
            } while (nextValue != NULL);
        }
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRInstrumentGen3_setSerial(RBRGen3 *instrument,
                                           const RBRInstrumentGen3Serial *serial)
{
    if (serial->baudRate < 0
        || serial->baudRate > RBRINSTRUMENTGEN3_SERIAL_BAUD_MAX
        || serial->mode < 0
        || serial->mode > RBRINSTRUMENTGEN3_SERIAL_MODE_MAX)
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    return RBRGen3_converse(
        instrument,
        "serial baudrate = %s, mode = %s",
        RBRInstrumentGen3SerialBaudRate_name(serial->baudRate),
        RBRInstrumentGen3SerialMode_name(serial->mode));
}

RBRGen3Error RBRInstrumentGen3_sleep(RBRGen3 *instrument)
{
    RBR_TRY(RBRGen3_sendCommand(instrument, "sleep"));

    instrument->lastActivityTime = RBRGEN3_NO_ACTIVITY;
    return RBRGEN3_SUCCESS;
}

const char *RBRInstrumentGen3WiFiState_name(RBRInstrumentGen3WiFiState state)
{
    switch (state)
    {
    case RBRINSTRUMENTGEN3_WIFI_NA:
        return "n/a";
    case RBRINSTRUMENTGEN3_WIFI_ON:
        return "on";
    case RBRINSTRUMENTGEN3_WIFI_OFF:
        return "off";
    case RBRINSTRUMENTGEN3_WIFI_COUNT:
        return "state count";
    case RBRINSTRUMENTGEN3_UNKNOWN_WIFI:
    default:
        return "unknown state";
    }
}

RBRGen3Error RBRInstrumentGen3_getWiFi(RBRGen3 *instrument,
                                         RBRInstrumentGen3WiFi *wifi)
{
    memset(wifi, 0, sizeof(RBRInstrumentGen3WiFi));

    RBRInstrumentGen3WiFiState *state =
        (RBRInstrumentGen3WiFiState *) &wifi->state;
    *state = RBRINSTRUMENTGEN3_UNKNOWN_WIFI;

    RBR_TRY(RBRGen3_converse(instrument, "wifi"));

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true)
    {
        RBRGen3_parseResponse(instrument,
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
            for (int i = RBRINSTRUMENTGEN3_WIFI_NA;
                 i < RBRINSTRUMENTGEN3_WIFI_COUNT;
                 i++)
            {
                if (strcmp(RBRInstrumentGen3WiFiState_name(i),
                           parameter.value) == 0)
                {
                    *state = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "timeout") == 0)
        {
            wifi->powerTimeout = strtol(parameter.value, NULL, 10) * 1000;
        }
        else if (strcmp(parameter.key, "commandtimeout") == 0)
        {
            wifi->commandTimeout = strtol(parameter.value, NULL, 10) * 1000;
        }
        else if (strcmp(parameter.key, "baudrate") == 0)
        {
            for (int i = RBRINSTRUMENTGEN3_SERIAL_BAUD_NONE + 1;
                 i <= RBRINSTRUMENTGEN3_SERIAL_BAUD_MAX;
                 i <<= 1)
            {
                if (strcmp(RBRInstrumentGen3SerialBaudRate_name(i),
                           parameter.value) == 0)
                {
                    *(RBRInstrumentGen3SerialBaudRate *) &wifi->baudRate = i;
                    break;
                }
            }
        }
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRInstrumentGen3_setWiFi(RBRGen3 *instrument,
                                         const RBRInstrumentGen3WiFi *wifi)
{
    if (wifi->powerTimeout < 5000
        || wifi->powerTimeout > 600000
        || wifi->powerTimeout % 1000 != 0
        || wifi->commandTimeout < 5000
        || wifi->commandTimeout > 600000
        || wifi->commandTimeout % 1000 != 0)
    {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    if (instrument->generation == RBRGEN3_LOGGER2)
    {
        return RBRGen3_converse(
            instrument,
            "wifi timeout = %d, commandtimeout = %d",
            wifi->powerTimeout / 1000,
            wifi->commandTimeout / 1000);
    }
    else
    {
        return RBRGen3_converse(
            instrument,
            "wifi enabled = %s, timeout = %d, commandtimeout = %d",
            wifi->enabled ? "true" : "false",
            wifi->powerTimeout / 1000,
            wifi->commandTimeout / 1000);
    }
}
